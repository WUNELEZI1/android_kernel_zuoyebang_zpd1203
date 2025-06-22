// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/irq.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/time64.h>
#include <linux/ktime.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/sched/clock.h>
#include <soc/xring/xrisp/xrisp.h>
#include "soc/xring/xr_timestamp.h"
#include "cam_sysfs/xrisp_sysfs.h"

#include "xrisp_ramlog.h"
#include "xrisp_common.h"
#include "xrisp_ringbuffer.h"
#include "xrisp_rproc.h"
#include "xrisp_camctrl_ipcm.h"

#define PREFIX_MAX	48
#define CONSOLE_LOG_MAX 1024

DECLARE_WAIT_QUEUE_HEAD(log_wait);

static u64 syslog_seq;
static size_t syslog_partial;

static struct ramlog_ringbuffer *lrb;
static ktime_t ramlog_utc;
static u64 ramlog_stamp;
atomic_t rproc_stop;
atomic_t reader_stop;
static struct task_struct *user_task;
static spinlock_t global_task_lock;
static int default_ramlog_level;
static int default_ramlog_enable;

static size_t print_syslog(unsigned int level, char *buf)
{
	return sprintf(buf, "<%u>", level);
}

static size_t print_utc_time(u64 ts, char *buf)
{
	struct timespec64 time_spec;
	struct tm tm;

	if (ts > ramlog_stamp)
		time_spec = ktime_to_timespec64(ramlog_utc + (ts - ramlog_stamp) * 1000);
	else
		time_spec = ktime_to_timespec64(ramlog_utc - (ramlog_stamp - ts) * 1000);

	time64_to_tm(time_spec.tv_sec, 0, &tm);
	if (edr_time.tm_mday > 0)
		return sprintf(buf, "[%02d-%02d %02d:%02d:%02d.%03ld]", edr_time.tm_mon,
			       edr_time.tm_mday, edr_time.tm_hour, tm.tm_min, tm.tm_sec,
			       time_spec.tv_nsec / 1000000);
	else
		return sprintf(buf, "[%02d-%02d %02d:%02d:%02d.%03ld]", tm.tm_mon + 1, tm.tm_mday,
			       tm.tm_hour, tm.tm_min, tm.tm_sec, time_spec.tv_nsec / 1000000);
}

static void ramlog_time_init(void)
{
	struct timespec64 time_spec;
	struct tm tm;

	ramlog_stamp = xr_timestamp_gettime() / 1000;
	ramlog_utc = ktime_get_real();
	ramlog_debug("ramlog_stamp:%llu, ramlog_utc:%lld\n", ramlog_stamp, ramlog_utc);
	time_spec = ktime_to_timespec64(ramlog_utc);
	time64_to_tm(time_spec.tv_sec, 0, &tm);
	ramlog_info("current UTC: [%ld-%02d-%02d %02d:%02d:%02d.%ld]\n", tm.tm_year + 1900,
		    tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
		    time_spec.tv_nsec / 1000);
}

static size_t print_threadid(u32 id, char *buf)
{
	return sprintf(buf, "[%2u]", id);
}

static size_t info_print_prefix(const struct lrb_info *info, bool syslog, bool time, char *buf)
{
	size_t len = 0;

	if (syslog)
		len = print_syslog(info->level, buf);

	if (time)
		len += print_utc_time(info->ts_usec, buf + len);

	len += print_threadid(info->thread_id, buf + len);

	buf[len++] = ' ';
	buf[len] = '\0';

	return len;
}

static int ramlog_ipc_send_cmd(ramlog_ipc_t *msg)
{
	int ret = 0;
	u8 cmd[2] = { 0 };

	cmd[0] = msg->cmd;
	cmd[1] = msg->val;
	ret = xrisp_msg_send((void *)&cmd, 2, TOPIC_LOG);

	return ret;
}

/*
 * Prepare the record for printing. The text is shifted within the given
 * buffer to avoid a need for another one. The following operations are
 * done:
 *
 *   - Add prefix for each line.
 *   - Drop truncated lines that no longer fit into the buffer.
 *   - Add the trailing newline that has been removed in vprintk_store().
 *   - Add a string terminator.
 *
 * Since the produced string is always terminated, the maximum possible
 * return value is @r->text_buf_size - 1;
 *
 * Return: The length of the updated/prepared text, including the added
 * prefixes and the newline. The terminator is not counted. The dropped
 * line(s) are not counted.
 */
static size_t record_print_text(struct lrb_record *r, bool syslog, bool time)
{
	size_t text_len = r->info->text_len;
	size_t buf_size = r->text_buf_size;
	char *text = r->text_buf;
	char prefix[PREFIX_MAX];
	bool truncated = false;
	size_t prefix_len;
	size_t line_len;
	size_t len = 0;
	char *next;

	/*
	 * If the message was truncated because the buffer was not large
	 * enough, treat the available text as if it were the full text.
	 */
	if (text_len > buf_size)
		text_len = buf_size;

	prefix_len = info_print_prefix(r->info, syslog, time, prefix);

	/*
	 * @text_len: bytes of unprocessed text
	 * @line_len: bytes of current line _without_ newline
	 * @text:     pointer to beginning of current line
	 * @len:      number of bytes prepared in r->text_buf
	 */
	for (;;) {
		next = memchr(text, '\n', text_len);
		if (next) {
			line_len = next - text;
		} else {
			/* Drop truncated line(s). */
			if (truncated)
				break;
			line_len = text_len;
		}

		/*
		 * Truncate the text if there is not enough space to add the
		 * prefix and a trailing newline and a terminator.
		 */
		if (len + prefix_len + text_len + 1 + 1 > buf_size) {
			/* Drop even the current line if no space. */
			if (len + prefix_len + line_len + 1 + 1 > buf_size)
				break;

			text_len = buf_size - len - prefix_len - 1 - 1;
			truncated = true;
		}

		memmove(text + prefix_len, text, text_len);
		memcpy(text, prefix, prefix_len);

		/*
		 * Increment the prepared length to include the text and
		 * prefix that were just moved+copied. Also increment for the
		 * newline at the end of this line. If this is the last line,
		 * there is no newline, but it will be added immediately below.
		 */
		len += prefix_len + line_len + 1;
		if (text_len == line_len) {
			/*
			 * This is the last line. Add the trailing newline
			 * removed in vprintk_store().
			 */
			text[prefix_len + line_len] = '\n';
			break;
		}

		/*
		 * Advance beyond the added prefix and the related line with
		 * its newline.
		 */
		text += prefix_len + line_len + 1;

		/*
		 * The remaining text has only decreased by the line with its
		 * newline.
		 *
		 * Note that @text_len can become zero. It happens when @text
		 * ended with a newline (either due to truncation or the
		 * original string ending with "\n\n"). The loop is correctly
		 * repeated and (if not truncated) an empty line with a prefix
		 * will be prepared.
		 */
		text_len -= line_len + 1;
	}

	/*
	 * If a buffer was provided, it will be terminated. Space for the
	 * string terminator is guaranteed to be available. The terminator is
	 * not counted in the return value.
	 */
	if (buf_size > 0)
		r->text_buf[len] = 0;

	return len;
}

static size_t get_record_print_text_size(struct lrb_info *info, unsigned int line_count,
					 bool syslog, bool time)
{
	char prefix[PREFIX_MAX];
	size_t prefix_len;

	prefix_len = info_print_prefix(info, syslog, time, prefix);

	/*
	 * Each line will be preceded with a prefix. The intermediate
	 * newlines are already within the text, but a final trailing
	 * newline will be added.
	 */
	return ((prefix_len * line_count) + info->text_len + 1);
}

bool check_and_handle_signals(struct task_struct *task)
{
	sigset_t pending_signals = task->pending.signal;
#if RAMLOG_DEBUG
	ramlog_info("pending signal for task: %s\n", current->comm);
	for (int i = 1; i <= _NSIG; i++) {
		if (sigismember(&pending_signals, i))
			ramlog_info("signal: %d\n", i);
	}
#endif
	if (sigismember(&pending_signals, SIGINT) || sigismember(&pending_signals, SIGUSR2) ||
	    sigismember(&pending_signals, SIGKILL)) {
		ramlog_info("receive correct signal\n");
		return 1;
	}

	return 0;
}

/* The caller is responsible for making sure @size is greater than 0. */
static int ramlog_print(char __user *buf, int size)
{
	struct lrb_info info;
	struct lrb_record r;
	char *text;
	int len = 0;
	u64 seq;

	text = kmalloc(CONSOLE_LOG_MAX, GFP_KERNEL);
	if (!text)
		return -ENOMEM;

	lrb_rec_init_rd(&r, &info, text, CONSOLE_LOG_MAX);

	/*
	 * Wait for the @syslog_seq record to be available. @syslog_seq may
	 * change while waiting.
	 */
	seq = syslog_seq;
#if RAMLOG_DEBUG
	ramlog_debug("check seq valid, syslog_seq:%lx\n", syslog_seq);
#endif
	while (!lrb_read_valid_seq(lrb, &seq, NULL)) {
		if ((atomic_read(&rproc_stop) == 1) || !lrb) {
			kfree(text);
			return -1;
		}

		usleep_range(1000, 1001);

		if (signal_pending(current) && check_and_handle_signals(current)) {
			ramlog_info("receive signal, exit loop\n");
			kfree(text);
			return -1;
		}
	}

	syslog_seq = seq;
#if RAMLOG_DEBUG
	ramlog_debug("update syslog_seq:%lx\n", syslog_seq);
#endif
	/*
	 * Copy records that fit into the buffer. The above cycle makes sure
	 * that the first record is always available.
	 */
	do {
		size_t n;
		size_t skip;
		int err;

		if (signal_pending(current) && check_and_handle_signals(current)) {
			ramlog_info("receive signal, exit read\n");
			kfree(text);
			/* return error no*/
			return -1;
		}

		if (atomic_read(&rproc_stop) == 1) {
			kfree(text);
			return -1;
		}

		if (!lrb_read_valid(lrb, syslog_seq, &r))
			break;

		if (r.info->seq != syslog_seq) {
			/* message is gone, move to next valid one */
			syslog_seq = r.info->seq;
			syslog_partial = 0;
		}

		skip = syslog_partial;
		n = record_print_text(&r, false, true);
		if (n - syslog_partial <= size) {
			/* message fits into buffer, move forward */
			syslog_seq = r.info->seq + 1;
			n -= syslog_partial;
			syslog_partial = 0;
		} else if (!len) {
#if RAMLOG_DEBUG
			/* partial read(), remember position */
			ramlog_debug("partial read, n:0x%x, size:0x%x, syslog_partial:0x%x", n,
				     size, syslog_partial);
#endif
			n = size;
			syslog_partial += n;
		} else
			n = 0;

		if (!n)
			break;

		err = copy_to_user(buf, text + skip, n);

		if (err) {
			if (!len)
				len = -EFAULT;
			break;
		}

		len += n;
		size -= n;
		buf += n;
	} while (size);

	kfree(text);
	return len;
}

int do_xrisp_ramlog(int type, char __user *buf, int len, int source)
{
	struct lrb_info info;
	int error = 0;

	switch (type) {
	case RAMLOG_ACTION_CLOSE: /* Close log */
		spin_lock(&global_task_lock);
		if (user_task)
			user_task = NULL;
		spin_unlock(&global_task_lock);
		atomic_set(&reader_stop, 1);
		smp_mb(); //up reader_stop
		ramlog_info("ramlog reader close\n");
		break;
	case RAMLOG_ACTION_OPEN: /* Open log */
		if (!lrb) {
			ramlog_err("ISP FW is not start, open ramlog fail\n");
			return -EFAULT;
		}
		spin_lock(&global_task_lock);
		if (!user_task) {
			user_task = current;
			spin_unlock(&global_task_lock);
		} else {
			spin_unlock(&global_task_lock);
			ramlog_err("ramlog cannot be opened twice\n");
			return -EFAULT;
		}
		if (atomic_read(&rproc_stop) == 1)
			return -EFAULT;
		/* set syslog_seq */
		syslog_seq = lrb_get_first_seq(lrb);
		ramlog_info("syslog_seq set to:0x%llx\n", syslog_seq);

		break;
	case RAMLOG_ACTION_READ: /* Read from log */
		if (!buf || len < 0)
			return -EINVAL;
		if (!len)
			return 0;
		if (!access_ok(buf, len))
			return -EFAULT;
		if (atomic_read(&rproc_stop) == 1)
			return -EINVAL;
		error = ramlog_print(buf, len);
		break;
	case RAMLOG_ACTION_SIZE_UNREAD:
		if (!lrb_read_valid_info(lrb, syslog_seq, &info, NULL)) {
			/* No unread messages. */
			return 0;
		}

		if (info.seq != syslog_seq) {
			/* messages are gone, move to first one */
			ramlog_info("syslog_seq:%llu move to %lu\n", syslog_seq, info.seq);
			syslog_seq = info.seq;
		}

		if (source == RAMLOG_FROM_PROC) {
			/*
			 * Short-cut for poll(/"proc/kmsg") which simply checks
			 * for pending data, not the size; return the count of
			 * records, not the length.
			 */
			error = lrb_next_seq(lrb) - syslog_seq;
		} else {
			unsigned int line_count = 0;
			u64 seq = 0;

			lrb_for_each_info(syslog_seq, lrb, seq, &info, &line_count)
				error += get_record_print_text_size(&info, line_count, true, true);
			//TODO?
		}
		break;
	default:
		error = -EINVAL;
		break;
	}

	return error;
}

static int ramlog_open(struct inode *inode, struct file *file)
{
	return do_xrisp_ramlog(RAMLOG_ACTION_OPEN, NULL, 0, RAMLOG_FROM_PROC);
}

static int ramlog_release(struct inode *inode, struct file *file)
{
	(void)do_xrisp_ramlog(RAMLOG_ACTION_CLOSE, NULL, 0, RAMLOG_FROM_PROC);
	return 0;
}

static ssize_t ramlog_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	if ((file->f_flags & O_NONBLOCK) &&
	    !do_xrisp_ramlog(RAMLOG_ACTION_SIZE_UNREAD, NULL, 0, RAMLOG_FROM_PROC))
		return -EAGAIN;
	return do_xrisp_ramlog(RAMLOG_ACTION_READ, buf, count, RAMLOG_FROM_PROC);
}

static __poll_t ramlog_poll(struct file *file, poll_table *wait)
{
	poll_wait(file, &log_wait, wait);
	if (do_xrisp_ramlog(RAMLOG_ACTION_SIZE_UNREAD, NULL, 0, RAMLOG_FROM_PROC))
		return EPOLLIN | EPOLLRDNORM;
	return 0;
}

static const struct proc_ops ramlog_proc_ops = {
	.proc_flags = PROC_ENTRY_PERMANENT,
	.proc_read = ramlog_read,
	.proc_poll = ramlog_poll,
	.proc_open = ramlog_open,
	.proc_release = ramlog_release,
	.proc_lseek = generic_file_llseek,
};

static int __init proc_ramlog_init(void)
{
	proc_create("xrisp_ramlog", 0444, NULL, &ramlog_proc_ops);
	return 0;
}

static int proc_ramlog_exit(void)
{
	remove_proc_entry("xrisp_ramlog", NULL);
	return 0;
}

static void append_char(char **pp, char *e, char c)
{
	if (*pp < e)
		*(*pp)++ = c;
}

static ssize_t info_print_ext_header(char *buf, size_t size, struct lrb_info *info)
{
	u64 ts_usec = info->ts_usec;
	char caller[20];
	u32 id = info->thread_id;

	snprintf(caller, sizeof(caller), ",caller=%u", id);

	return scnprintf(buf, size, "%u,%lu,%llu,%s;", info->level, info->seq, ts_usec, caller);
}

static ssize_t msg_add_ext_text(char *buf, size_t size, const char *text, size_t text_len,
				unsigned char endc)
{
	char *p = buf, *e = buf + size;
	size_t i;

	/* escape non-printable characters */
	for (i = 0; i < text_len; i++) {
		unsigned char c = text[i];

		if (c < ' ' || c >= 127 || c == '\\')
			p += scnprintf(p, e - p, "\\x%02x", c);
		else
			append_char(&p, e, c);
	}
	append_char(&p, e, endc);

	return p - buf;
}

static ssize_t msg_print_ext_body(char *buf, size_t size, char *text, size_t text_len)
{
	ssize_t len;

	len = msg_add_ext_text(buf, size, text, text_len, '\n');

	return len;
}

/* /dev/xrisp-ramlog */
struct xrisp_ramlog_user {
	atomic64_t seq;
	struct ratelimit_state rs;
	struct mutex lock;
	char buf[CONSOLE_EXT_LOG_MAX];
	struct lrb_info info;
	char text_buf[CONSOLE_EXT_LOG_MAX];
	struct lrb_record record;
};

static ssize_t xrisp_ramlog_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	struct xrisp_ramlog_user *user = file->private_data;
	struct lrb_record *r = &user->record;
	size_t len;
	ssize_t ret;

	if (!user)
		return -EBADF;

	ret = mutex_lock_interruptible(&user->lock);
	if (ret)
		return ret;

	if (!lrb_read_valid(lrb, atomic64_read(&user->seq), r)) {
		if (file->f_flags & O_NONBLOCK) {
			ret = -EAGAIN;
			goto out;
		}

		/*
		 * Guarantee this task is visible on the waitqueue before
		 * checking the wake condition.
		 *
		 * The full memory barrier within set_current_state() of
		 * prepare_to_wait_event() pairs with the full memory barrier
		 * within wq_has_sleeper().
		 */
		ret = wait_event_interruptible(log_wait,
					       lrb_read_valid(lrb, atomic64_read(&user->seq), r));
		if (ret)
			goto out;
	}

	if (r->info->seq != atomic64_read(&user->seq)) {
		/* our last seen message is gone, return error and reset */
		atomic64_set(&user->seq, r->info->seq);
		ret = -EPIPE;
		goto out;
	}

	len = info_print_ext_header(user->buf, sizeof(user->buf), r->info);
	len += msg_print_ext_body(user->buf + len, sizeof(user->buf) - len, &r->text_buf[0],
				  r->info->text_len);

	atomic64_set(&user->seq, r->info->seq + 1);

	if (len > count) {
		ret = -EINVAL;
		goto out;
	}

	if (copy_to_user(buf, user->buf, len)) {
		ret = -EFAULT;
		goto out;
	}
	ret = len;
out:
	mutex_unlock(&user->lock);
	return ret;
}

/*
 * Be careful when modifying this function!!!
 *
 * Only few operations are supported because the device works only with the
 * entire variable length messages (records). Non-standard values are
 * returned in the other cases and has been this way for quite some time.
 * User space applications might depend on this behavior.
 */
static loff_t xrisp_ramlog_llseek(struct file *file, loff_t offset, int whence)
{
	struct xrisp_ramlog_user *user = file->private_data;
	loff_t ret = 0;

	if (!user)
		return -EBADF;
	if (offset)
		return -ESPIPE;

	switch (whence) {
	case SEEK_SET:
		/* the first record */
		atomic64_set(&user->seq, lrb_first_valid_seq(lrb));
		break;
	case SEEK_DATA:
		//atomic64_set(&user->seq, latched_seq_read_nolock(&clear_seq));
		break;
	case SEEK_END:
		/* after the last record */
		atomic64_set(&user->seq, lrb_next_seq(lrb));
		break;
	default:
		ret = -EINVAL;
	}
	return ret;
}

static __poll_t xrisp_ramlog_poll(struct file *file, poll_table *wait)
{
	struct xrisp_ramlog_user *user = file->private_data;
	struct lrb_info info;
	__poll_t ret = 0;

	if (!user)
		return EPOLLERR | EPOLLNVAL;

	poll_wait(file, &log_wait, wait);

	if (lrb_read_valid_info(lrb, atomic64_read(&user->seq), &info, NULL)) {
		/* return error when data has vanished underneath us */
		if (info.seq != atomic64_read(&user->seq))
			ret = EPOLLIN | EPOLLRDNORM | EPOLLERR | EPOLLPRI;
		else
			ret = EPOLLIN | EPOLLRDNORM;
	}

	return ret;
}

static int xrisp_ramlog_open(struct inode *inode, struct file *file)
{
	struct xrisp_ramlog_user *user;

	user = kmalloc(sizeof(struct xrisp_ramlog_user), GFP_KERNEL);
	if (!user)
		return -ENOMEM;

	mutex_init(&user->lock);

	lrb_rec_init_rd(&user->record, &user->info, &user->text_buf[0], sizeof(user->text_buf));

	atomic64_set(&user->seq, lrb_first_valid_seq(lrb));

	file->private_data = user;

	return 0;
}

static int xrisp_ramlog_release(struct inode *inode, struct file *file)
{
	struct xrisp_ramlog_user *user = file->private_data;

	if (!user)
		return 0;

	mutex_destroy(&user->lock);
	file->private_data = NULL;
	kfree(user);
	return 0;
}

const struct file_operations xrisp_ramlog_fops = {
	.open = xrisp_ramlog_open,
	.read = xrisp_ramlog_read,
	.llseek = xrisp_ramlog_llseek,
	.poll = xrisp_ramlog_poll,
	.release = xrisp_ramlog_release,
};

struct xrisp_memdev {
	const char *name;
	umode_t mode;
	const struct file_operations *fops;
	fmode_t fmode;
};

static const struct xrisp_memdev memdev = {
	.name = "xrisp_ramlog",
	.mode = 0644,
	.fops = &xrisp_ramlog_fops,
	.fmode = 0,
};

static void *xr_ramlog_mem_map;
static size_t xr_ramlog_mem_size;

int xring_edr_ramlog_start(void *priv)
{
	struct lrb_desc *descs = NULL;
	struct lrb_info *infos = NULL;
	char *data_rb = NULL;

	ramlog_debug("ramlog start\n");
	atomic_set(&rproc_stop, 0);
	atomic_set(&reader_stop, 0);

	// init user_task lock
	spin_lock_init(&global_task_lock);

	if (xrisp_rproc_ramlog_avail()) {
		xr_ramlog_mem_map = xrisp_rproc_ramlog_addr();
		if (!xr_ramlog_mem_map) {
			ramlog_err("get ramlog addr fail\n");
			return -1;
		}

		xr_ramlog_mem_size = xrisp_rproc_ramlog_size();
		if (!xr_ramlog_mem_size) {
			ramlog_err("get ramlog buf size fail\n");
			return -1;
		}
		ramlog_info("ramlog addr:0x%pK, size:0x%lx\n", xr_ramlog_mem_map,
			    xr_ramlog_mem_size);
	} else {
		ramlog_err("ramlog init fail, buffer not ready\n");
		return -1;
	}

	lrb = (struct ramlog_ringbuffer *)xr_ramlog_mem_map;
	descs = (struct lrb_desc *)(xr_ramlog_mem_map + DESCS_OFFSET);
	infos = (struct lrb_info *)(xr_ramlog_mem_map + INFOS_OFFSET);
	data_rb = (char *)(xr_ramlog_mem_map + DATA_RING_BUF_OFFSET);

	ramlog_time_init();
	xrisp_ramlog_ringbuffer_init(data_rb, descs, infos);
#ifdef CONFIG_XRING_ISP_EDR_TRACE
#define RAM_TRACE_BUF_OFFSET 0x600000
	ramtrace_buf = xr_ramlog_mem_map + RAM_TRACE_BUF_OFFSET;
#endif
	wdt_buf = xr_ramlog_mem_map + EDR_WDT_BUF_OFFSET;
	ramlog_info("wdt_buf addr:0x%pK", wdt_buf);

	return 0;
}

void xring_edr_ramlog_stop(void *priv, bool crashed)
{
	ramlog_info("ramlog stop\n");

	/*
	 * reader should exit if ramlog is opened
	 * send SIGUSR2 signal gently
	 */
#ifdef CONFIG_DEBUG_SPINLOCK
	if (READ_ONCE(global_task_lock.rlock.magic) != SPINLOCK_MAGIC) {
		lrb = NULL;
		return;
	}
#endif
	ramlog_info("rproc sta: [%d]", atomic_read(&rproc_stop));
	spin_lock(&global_task_lock);
	if (user_task) {
		send_sig(SIGUSR2, user_task, 0);
		spin_unlock(&global_task_lock);
		int timeout = 1000;

		while (timeout > 0 && (atomic_read(&reader_stop) == 0)) {
			timeout -= 1;
			usleep_range(1000, 1001);
		}
		ramlog_info("ramlog reader exit before rproc stop sta[%d][%d]",
			    atomic_read(&reader_stop), timeout);
		spin_lock(&global_task_lock);
		user_task = NULL;
		spin_unlock(&global_task_lock);
	} else
		spin_unlock(&global_task_lock);

	lrb = NULL;
}

static ssize_t ramlog_level_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	if (!lrb) {
		ramlog_err("read fail, ringbuffer not ready\n");
		return -EINVAL;
	}

	return sprintf(buf, "%d\n", lrb->log_level);
}

static ssize_t ramlog_level_store(struct kobject *kobj, struct kobj_attribute *attr,
				  const char *buf, size_t count)
{
	int err, value;
	ramlog_ipc_t msg;

	err = kstrtoint(buf, 0, &value);
	if (err)
		return err;

	if (value < 0 || value > 4)
		return -EINVAL;

	default_ramlog_level = value;

	if (!lrb) {
		ramlog_err("read fail, ringbuffer not ready\n");
		return -EINVAL;
	}

	// lrb->log_level = value;

	msg.cmd = RAMLOG_IPC_CMD_LOG_LEVEL;
	msg.val = (u8)value;
	err = ramlog_ipc_send_cmd(&msg);
	if (err)
		ramlog_err("send isp fw log level(%d) failed. %d", value, err);

	return count;
}

static ssize_t ramlog_log_enable_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	if (!lrb) {
		ramlog_err("read fail, ringbuffer not ready\n");
		return -EINVAL;
	}

	return sprintf(buf, "%d\n", lrb->log_enable);
}

static ssize_t ramlog_log_enable_store(struct kobject *kobj, struct kobj_attribute *attr,
				       const char *buf, size_t count)
{
	int err, value;
	ramlog_ipc_t msg;

	err = kstrtoint(buf, 0, &value);
	if (err)
		return err;

	if (value < 0)
		return -EINVAL;

	default_ramlog_enable = value;

	if (!lrb) {
		ramlog_err("read fail, ringbuffer not ready\n");
		return -EINVAL;
	}

	// lrb->log_enable = value;

	msg.cmd = RAMLOG_IPC_CMD_LOG_ENABLE;
	msg.val = (u8)value;
	err = ramlog_ipc_send_cmd(&msg);
	if (err)
		ramlog_err("send isp fw log enable(%d) failed. %d", value, err);

	return count;
}

/* Set ramlog config before fw boot */
int ramlog_set_default_cfg(void *vaddr)
{
	if (!vaddr)
		return -1;

	struct ramlog_ringbuffer *lrb = NULL;

	lrb = (struct ramlog_ringbuffer *)vaddr;
	if (!lrb) {
		ramlog_err("get ramlog addr fail");
		return -1;
	}

	/* DMA alloc buff no need to flush dcache */
	lrb->log_level = default_ramlog_level;
	lrb->log_enable = default_ramlog_enable;

	ramlog_info("Set isp ramlog cfg: level=%d, enable=%d.", lrb->log_level, lrb->log_enable);

	return 0;
}

void ramlog_loglevel_set(int level)
{
	int err;
	ramlog_ipc_t msg;

	default_ramlog_level = level;
	if (!lrb) {
		ramlog_debug("lrb not ready,exit");
		return;
	}

	msg.cmd = RAMLOG_IPC_CMD_LOG_LEVEL;
	msg.val = (u8)level;
	err = ramlog_ipc_send_cmd(&msg);
	if (err)
		ramlog_err("send isp fw log level(%d) failed. %d", level, err);
}

static struct kobj_attribute kobj_attr_xrisp_ramlog_level =
	__ATTR(xrisp_ramlog_level, 0664, ramlog_level_show, ramlog_level_store);
static struct kobj_attribute kobj_attr_xrisp_ramlog_log_enable =
	__ATTR(xrisp_ramlog_log_enable, 0664, ramlog_log_enable_show, ramlog_log_enable_store);

static struct attribute *xrisp_ramlog_attrs[] = {
	&kobj_attr_xrisp_ramlog_level.attr,
	&kobj_attr_xrisp_ramlog_log_enable.attr,
	NULL,
};
static const struct attribute_group xrisp_ramlog_group = {
	.attrs = xrisp_ramlog_attrs,
};

static struct xrisp_miscdev_entry ramlog_misc = {
	.name = "xrisp_ramlog",
	.fops = &xrisp_ramlog_fops,
};

static void *ramlog_priv;
static struct kobject *parent_kobj;
static struct kobject *ramlog_kobj;

int xring_edr_ramlog_init(void)
{
	int ret = 0;

	ramlog_info("ramlog init");

	default_ramlog_level = 4;
	default_ramlog_enable = 1;
	ret = xrisp_wait_sysfs_init_done();
	if (ret)
		return ret;

	ramlog_priv = kmalloc(10, GFP_KERNEL);
	if (!ramlog_priv)
		return -ENOMEM;

	ret = xrisp_register_ramlog_ops(ramlog_priv, xring_edr_ramlog_start, xring_edr_ramlog_stop);
	if (ret) {
		ramlog_err("register ramlog ops fail\n");
		goto free_ramlog_priv;
	}

	ret = xrisp_class_dev_register(&ramlog_misc);
	if (ret) {
		ramlog_err("register misc device fail\n");
		goto destory_ramlog_ops;
	}

	parent_kobj = xrisp_get_isp_kobj();
	if (!parent_kobj) {
		ret = -EINVAL;
		goto destory_device;
	}

	ramlog_kobj = kobject_create_and_add("xrisp_ramlog", parent_kobj);
	if (!ramlog_kobj) {
		ret = -EINVAL;
		goto put_isp_kobj;
	}
	ret = sysfs_create_group(ramlog_kobj, &xrisp_ramlog_group);
	if (ret)
		goto destory_kobj;

	proc_ramlog_init();
	ramlog_info("ramlog init success");
	return 0;

destory_kobj:
	kobject_put(ramlog_kobj);
put_isp_kobj:
	xrisp_put_isp_kobj();
destory_device:
	xrisp_class_dev_unregister(&ramlog_misc);
destory_ramlog_ops:
	xrisp_unregister_ramlog_ops();
free_ramlog_priv:
	kfree(ramlog_priv);
	return ret;
}

void xring_edr_ramlog_exit(void)
{
	xrisp_unregister_ramlog_ops();
	sysfs_remove_group(ramlog_kobj, &xrisp_ramlog_group);
	kobject_put(ramlog_kobj);
	xrisp_put_isp_kobj();
	xrisp_class_dev_unregister(&ramlog_misc);
	kfree(ramlog_priv);
	proc_ramlog_exit();
	ramlog_info("ramlog exit");
}

MODULE_AUTHOR("Qiang Wu <wuqiang9@xiaomi.com>");
MODULE_DESCRIPTION("x-ring isp ramlog");
MODULE_LICENSE("GPL v2");
