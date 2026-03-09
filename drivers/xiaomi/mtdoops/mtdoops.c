#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include "mtdoops.h"

struct PACK(mtdoops_header) first_mtdoops_header;
struct bootmonitor_context boot_cxt;
struct file *bdev_file = NULL;
struct mutex write_bm_mutex;
static char *mtd_buffer;

static char bm_devices_name[128] = {0};
module_param_string(devname, bm_devices_name, 128, 0644);

char boot_mode[16] = {0};
module_param_string(bootmode, boot_mode, 16, 0644);

char boot_index[10] = {0};
module_param_string(boot_index, boot_index, 10, 0644);

static int is_first_init = 1;
static int recovery_flag = 0;
static struct task_struct *retry_thread = NULL;

/*add device for write log to oops partition*/
static struct page *page_read(struct address_space *mapping, pgoff_t index)
{
	return read_mapping_page(mapping, index, NULL);
}

int _partition_bm_read(struct file *dev, loff_t from,
		size_t len, void *buf)
{
	struct page *page;
	pgoff_t index = from >> PAGE_SHIFT;
	int offset = from & (PAGE_SIZE-1);
	int cpylen;
	u_char *ubuf = (u_char *)buf;
	while (len) {
		if ((offset + len) > PAGE_SIZE)
			cpylen = PAGE_SIZE - offset;	// multiple pages
		else
			cpylen = len;	// this page
		len = len - cpylen;

		page = page_read(dev->f_mapping, index);
		if (IS_ERR(page))
			return PTR_ERR(page);

		memcpy(ubuf, page_address(page) + offset, cpylen);
		put_page(page);

		ubuf += cpylen;
		offset = 0;
		index++;
	}

	return 0;
}

static int _partition_write(struct file *dev, const void *buf,
		loff_t to, size_t len)
{
	struct page *page;
	struct address_space *mapping = dev->f_mapping;
	pgoff_t index = to >> PAGE_SHIFT;	// page index
	int offset = to & ~PAGE_MASK;	// page offset
	int cpylen;
	u_char *ubuf = (u_char *)buf;
	while (len) {
		if ((offset+len) > PAGE_SIZE)
			cpylen = PAGE_SIZE - offset;	// multiple pages
		else
			cpylen = len;			// this page
		len = len - cpylen;

		page = page_read(mapping, index);
		if (IS_ERR(page))
			return PTR_ERR(page);

		if (memcmp(page_address(page)+offset, ubuf, cpylen)) {
			lock_page(page);
			memcpy(page_address(page) + offset, ubuf, cpylen);
			set_page_dirty(page);
			unlock_page(page);
			balance_dirty_pages_ratelimited(mapping);
		}
		put_page(page);

		ubuf += cpylen;
		offset = 0;
		index++;
	}

	return 0;
}

int partition_bm_write(loff_t to, size_t len, const void *buf)
{
	int err;

	err = -1;
	if (IS_ERR(bdev_file) || bdev_file == NULL) {
		MTN_PRINT_ERR("%s-%d:failed:IS_ERR(bdev_file)\n", __func__, __LINE__);
		return err;
	}
	if (buf == NULL) {
		MTN_PRINT_ERR("%s-%d:buf is NULL\n", __func__, __LINE__);
		return err;
	}
	mutex_lock(&write_bm_mutex);
	err = _partition_write(bdev_file, buf, to, len);
	mutex_unlock(&write_bm_mutex);
	if (err > 0)
		err = 0;
	sync_blockdev(file_bdev(bdev_file));

	return err;
}

/*erase \n*/
static inline void kill_final_newline(char *str)
{
	char *newline = strrchr(str, '\n');
	if (newline && !newline[1])
		*newline = 0;
}

static int retry_add_device_thread_func(void* data);
static int mtdoops_init_header(struct bootmonitor_context *cxt);

/*get oops partion to write/read */
int get_bm_devices(void)
{
	int err;
	int i;
	const fmode_t mode = BLK_OPEN_READ | BLK_OPEN_WRITE;
	char buf[128];
	char *str = buf;

	err = -1;

	if (strnlen(bm_devices_name, sizeof(buf)) >= sizeof(buf)) {
		MTN_PRINT_ERR("%s-%d:parameter too long\n", __func__, __LINE__);
		return err;
	}

	strcpy(str, bm_devices_name);
	kill_final_newline(str);

	/* Get a handle on the device */
	bdev_file = bdev_file_open_by_path(str, mode, THIS_MODULE, NULL);
	MTN_PRINT_ERR("%s-%d:bm dev: str %s:%zu\n", __func__, __LINE__, str, strlen(str));

	/*
	 * We might not have the root device mounted at this point.
	 * Try to resolve the device name by other means.
	 */
	for (i = 0; IS_ERR(bdev_file) && i <= DEFAULT_TIMEOUT; i++) {
		/*if first boot mode is recovery, preventing extended startup times*/
		if (recovery_flag == 1 && is_first_init == 1) {
			MTN_PRINT_ERR("%s-%d:recovery mode and first\n", __func__, __LINE__);
			break;
		}
		MTN_PRINT_ERR("%s-%d:bm dev: i %d\n", __func__, __LINE__, i);
		if (i) {
			/*
			 * Calling wait_for_device_probe in the first loop
			 * was not enough, sleep for a bit in subsequent
			 * go-arounds.
			 */
			msleep(3000);
		}
		wait_for_device_probe();

		bdev_file = bdev_file_open_by_path(str, mode, THIS_MODULE, NULL);
		MTN_PRINT_ERR("%s-%d:blkdev_get_by_dev end !\n", __func__, __LINE__);
	}

	/*recovery mode and first*/
	if (is_first_init == 1 && recovery_flag == 1) {
		is_first_init = 0;
		retry_thread = kthread_run(retry_add_device_thread_func, NULL, "block2mtd_retry");
		return 0;
	}

	if (IS_ERR(bdev_file)) {
		MTN_PRINT_ERR("%s-%d: error: cannot open device %s\n",  __func__, __LINE__, str);
		return err;
	}

	/*dev wirte lock*/
	mutex_init(&write_bm_mutex);

	err = 0;
	return err;
}

/*add thread get device in recovery*/
static int retry_add_device_thread_func(void* data) {
	struct bootmonitor_context *cxt;
	int retry_count = 2;

	cxt = &boot_cxt;
	while (retry_count > 0) {
		if (!get_bm_devices()) {
			MTN_PRINT_ERR("%s-%d: get device ok! retry_count is %d", __func__, __LINE__, retry_count);
			/*get device fail*/
			if (IS_ERR(bdev_file) || bdev_file == NULL) {
				MTN_PRINT_ERR("%s-%d:bdev_file is NULL\n", __func__, __LINE__);
			} else {
				/*init oops header*/
				if (mtdoops_init_header(cxt)) {
					MTN_PRINT_ERR("%s-%d:mtdoops_init_header fail\n", __func__, __LINE__);
				}
			}
			break;
		} else {
			MTN_PRINT_ERR("%s-%d: sleep retry_count is %d", __func__, __LINE__, retry_count);
			msleep(2000);
		}
		retry_count--;
	}
	kthread_stop(retry_thread);
	return 0;
}

/*vmap pstore*/
static void *ram_vmap(phys_addr_t start, size_t size)
{
	struct page **pages;
	phys_addr_t page_start;
	unsigned int page_count;
	unsigned int i;
	void *vaddr;

	page_start = start - offset_in_page(start);
	page_count = DIV_ROUND_UP(size + offset_in_page(start), PAGE_SIZE);

	pages = kmalloc_array(page_count, sizeof(struct page *), GFP_KERNEL);
	if (!pages) {
		pr_err("%s: Failed to allocate array for %u pages\n",
		       __func__, page_count);
		return NULL;
	}

	for (i = 0; i < page_count; i++) {
		phys_addr_t addr = page_start + i * PAGE_SIZE;
		pages[i] = pfn_to_page(addr >> PAGE_SHIFT);
	}
	/*
	 * VM_IOREMAP used here to bypass this region during vread()
	 * and kmap_atomic() (i.e. kcore) to avoid __va() failures.
	 */
	vaddr = vmap(pages, page_count, VM_MAP | VM_IOREMAP, PAGE_KERNEL);
	kfree(pages);

	/*
	 * Since vmap() uses page granularity, we must add the offset
	 * into the page here, to get the byte granularity address
	 * into the mapping to represent the actual "start" location.
	 */
	return vaddr + offset_in_page(start);
}

/*get kmsg and pmsg*/
static void add_timeout_header_kmsg(char *pmsg_buf, enum mtdoops_log_type type, int reason, int flags) {
	char str_buf[MTDOOPS_HEADER_SIZE] = {0};
	int ret_len;
	unsigned long local_time;
	struct timespec64 now;
	struct tm ts;

	ret_len = 0;
	ktime_get_coarse_real_ts64(&now);
	/*set title time to UTC+8*/
	local_time = (unsigned long)(now.tv_sec + 8 * 60 * 60);
	time64_to_tm(local_time, 0, &ts);
	ret_len = snprintf(str_buf, MTDOOPS_HEADER_SIZE,
			"\n```\n##FLAGS:%d\n##VEISION:%s\n## BOOT_INDEX:%s\n##REBOOT REASON:%s\n## LOG TYPE:%s\n## BOOT MODE:%s\n"
			"##### %04ld-%02d-%02d %02d:%02d:%02d\n```c\n",
			flags, build_fingerprint, boot_index, kdump_reason[reason], log_type[type], boot_mode, \
			ts.tm_year+1900, ts.tm_mon + 1, ts.tm_mday, \
			ts.tm_hour, ts.tm_min, ts.tm_sec);

	if (ret_len >= sizeof(str_buf))
        	ret_len = sizeof(str_buf);

	memcpy(pmsg_buf, str_buf, ret_len);
}

static void add_timeout_header_pmsg(char *pmsg_buf, enum mtdoops_log_type type, int reason) {
	char str_buf[MTDOOPS_HEADER_SIZE] = {0};
	int ret_len;
	unsigned long local_time;
	struct timespec64 now;
	struct tm ts;

	ret_len = 0;
	ktime_get_coarse_real_ts64(&now);
	/*set title time to UTC+8*/
	local_time = (unsigned long)(now.tv_sec + 8 * 60 * 60);
	time64_to_tm(local_time, 0, &ts);
	ret_len = snprintf(str_buf, MTDOOPS_HEADER_SIZE,
			"\n```\n## LOG TYPE:%s\n## BOOT MODE:%s\n"
			"##### %04ld-%02d-%02d %02d:%02d:%02d\n```c\n",
			log_type[type], boot_mode, \
			ts.tm_year+1900, ts.tm_mon + 1, ts.tm_mday, \
			ts.tm_hour, ts.tm_min, ts.tm_sec);

	if (ret_len >= sizeof(str_buf))
        	ret_len = sizeof(str_buf);

	memcpy(pmsg_buf, str_buf, ret_len);
}

/*get kmsg pmsg*/
void monitor_get_kmsg(struct bootmonitor_context *cxt) {
	struct kmsg_dump_iter iter;
	size_t ret_len = 0;

	kmsg_dump_rewind(&iter);
	kmsg_dump_get_buffer(&iter, true,
			     cxt->dmesg_buf + MTDOOPS_HEADER_SIZE,
			     PARTITION_KMSG_SIZE - MTDOOPS_HEADER_SIZE, &ret_len);
}

void monitor_get_pmsg(struct bootmonitor_context *cxt) {
	char *pmsg_buffer_start = NULL;
	struct pmsg_buffer_hdr *p_hdr = NULL;
	phys_addr_t pstart;
	size_t dstart;

	pstart = (phys_addr_t)(cxt->monitor_data.mem_address + cxt->monitor_data.mem_size - cxt->monitor_data.pmsg_size);
	dstart = cxt->monitor_data.pmsg_size;

	if (!pstart || !dstart) {
		MTN_PRINT_ERR("pmsg_buffer_start is NULL\n");
		return;
	}

	pmsg_buffer_start = ram_vmap(pstart, dstart);
	if (!pmsg_buffer_start) {
		MTN_PRINT_ERR("pmsg_buffer_start is NULL\n");
		return;
	}

	p_hdr = (struct pmsg_buffer_hdr *)pmsg_buffer_start;

	if (p_hdr->sig == 0x43474244) {
		void *oopsbuf = cxt->pmsg_buf + MTDOOPS_HEADER_SIZE;
		uint8_t *p_buff_end = (uint8_t *)p_hdr->data + atomic_read(&p_hdr->size);
		int pmsg_cp_size = 0;
		int p_start = p_hdr->start.counter;
		int p_size = p_hdr->size.counter;

		/*8850:防止pstore被踩导致的异常访问*/
		if (p_start > cxt->monitor_data.pmsg_size || p_size > cxt->monitor_data.pmsg_size) {
			MTN_PRINT_ERR("pstore buffer overflow\n");
			return;
		}

		pmsg_cp_size = PARTITION_PMSG_SIZE - MTDOOPS_HEADER_SIZE;
		if (p_size <= pmsg_cp_size)
			pmsg_cp_size = p_size;

		if (p_start >= pmsg_cp_size)
			memcpy(oopsbuf, p_hdr->data, pmsg_cp_size);
		else {
			memcpy(oopsbuf, p_buff_end - (pmsg_cp_size - p_start),
					pmsg_cp_size - p_start);
			memcpy(oopsbuf + (pmsg_cp_size - p_start), p_hdr->data,
					p_start);
		}
	} else {
		MTN_PRINT_ERR("read pmsg failed sig\n");
	}
}

/*init header*/
static void initmtdoopsheader(void) {
	first_mtdoops_header.magic = LOG_TABLE_HEADER_MAGIC;
	first_mtdoops_header.flags = 0;
}

/*init oops patrition header*/
static int mtdoops_init_header(struct bootmonitor_context *cxt)
{
	int err;

	err = -1;
	if(IS_ERR(bdev_file) || bdev_file == NULL) {
		MTN_PRINT_ERR("%s-%d:bdev_file is NULL\n", __func__, __LINE__);
		return err;
	}

	err = _partition_bm_read(bdev_file, MTDOOPS_START_ADDR, sizeof(struct mtdoops_header), &first_mtdoops_header);
	if (err != 0) {
		MTN_PRINT_ERR("%s-%d:_partition_bm_read fail\n", __func__, __LINE__);
		return err;
	}
	if (first_mtdoops_header.magic == LOG_TABLE_HEADER_MAGIC) {
		MTN_PRINT_ERR("%s-%d:mtdoops_header magic \n", __func__, __LINE__);
		return err;
	}
	initmtdoopsheader();
	err = partition_bm_write(MTDOOPS_START_ADDR, sizeof(struct mtdoops_header), &first_mtdoops_header);
	if (err != 0) {
		MTN_PRINT_ERR("%s-%d:partition_bm_write fail\n", __func__, __LINE__);
		return err;
	}

	err = 0;
	return err;
}

/*get d task*/
static void mtdoops_show_state_filter_single(unsigned long state_filter)
{
	struct task_struct *g, *p;

#if BITS_PER_LONG == 32
	printk(KERN_INFO
		"  task 			   PC stack   pid father\n");
#else
	printk(KERN_INFO
		"  task 					   PC stack   pid father\n");
#endif
	rcu_read_lock();
	for_each_process_thread(g, p) {
		/*
		 * reset the NMI-timeout, listing all files on a slow
		 * console might take a lot of time:
		 * Also, reset softlockup watchdogs on all CPUs, because
		 * another CPU might be blocked waiting for us to process
		 * an IPI.
		 */
		touch_nmi_watchdog();
		//touch_all_softlockup_watchdogs();
		if (p->__state == state_filter)
			sched_show_task(p);
	}
	rcu_read_unlock();
}

/*write pmsg kmsg to oops*/
static int mtdoops_do_dump(struct kmsg_dumper *dumper,
	enum mtd_dump_reason reason)
{
	int err;
	struct bootmonitor_context *cxt;

	err = -1;
	cxt = &boot_cxt;

	if(IS_ERR(bdev_file) || bdev_file == NULL) {
		MTN_PRINT_ERR("%s-%d:bdev_file is NULL\n", __func__, __LINE__);
		return err;
	}
	if(!cxt) {
		MTN_PRINT_ERR("%s-%d:cxt is NULL\n", __func__, __LINE__);
		return err;
	}
	if (!cxt->pmsg_buf || !cxt->dmesg_buf) {
		MTN_PRINT_ERR("%s-%d:cxt->pmsg_buf or cxt->dmesg_buf is NULL\n", __func__, __LINE__);
		return err;
	}
	/*get flags*/
	err = _partition_bm_read(bdev_file, MTDOOPS_START_ADDR, sizeof(struct mtdoops_header), &first_mtdoops_header);
	if (err != 0) {
		MTN_PRINT_ERR("%s-%d:_partition_bm_read fail\n", __func__, __LINE__);
		return err;
	}
	if (first_mtdoops_header.magic != LOG_TABLE_HEADER_MAGIC) {
		MTN_PRINT_ERR("%s-%d:mtdoops_header magic \n", __func__, __LINE__);
		return err;
	}
	if (first_mtdoops_header.flags < 0 || first_mtdoops_header.flags > 7) {
		MTN_PRINT_ERR("%s-%d:first_mtdoops_header.flags error \n", __func__, __LINE__);
		return err;
	}
	/*write zero to clean partition*/
	err = partition_bm_write(first_mtdoops_header.flags * PARTITION_PART_SIZE + MTDOOPS_HEADER, PARTITION_KMSG_SIZE, cxt->dmesg_buf);
	if (err != 0) {
		MTN_PRINT_ERR("%s-%d:partition_bm_write error\n", __func__, __LINE__);
		return err;
	}
	err = partition_bm_write(first_mtdoops_header.flags * PARTITION_PART_SIZE + MTDOOPS_HEADER + PARTITION_KMSG_SIZE, PARTITION_PMSG_SIZE, cxt->pmsg_buf);
	if (err != 0) {
		MTN_PRINT_ERR("%s-%d:partition_bm_write error\n", __func__, __LINE__);
		return err;
	}
	/*8850:规避因串口打印问题导致的调度异常*/
	if (reason == MTD_DUMP_LONG_PRESS) {
		/*get d task print*/
		MTN_PRINT_ERR("%s-%d:mtdoops start D task info\n", __func__, __LINE__);
		mtdoops_show_state_filter_single(TASK_UNINTERRUPTIBLE);
		MTN_PRINT_ERR("%s-%d:mtdoops end D task info\n", __func__, __LINE__);
		/*get kernel log and logcat*/
	}
	monitor_get_kmsg(cxt);
	monitor_get_pmsg(cxt);
	add_timeout_header_kmsg(cxt->dmesg_buf, MTDOOPS_TYPE_DMESG, reason, first_mtdoops_header.flags);
	add_timeout_header_pmsg(cxt->pmsg_buf, MTDOOPS_TYPE_PMSG, reason);
	/*write kmsg pmsg*/
	err = partition_bm_write(first_mtdoops_header.flags * PARTITION_PART_SIZE + MTDOOPS_HEADER, PARTITION_KMSG_SIZE, cxt->dmesg_buf);
	if (err != 0) {
		MTN_PRINT_ERR("%s-%d:partition_bm_write error\n", __func__, __LINE__);
		return err;
	}
	err = partition_bm_write(first_mtdoops_header.flags * PARTITION_PART_SIZE + MTDOOPS_HEADER + PARTITION_KMSG_SIZE, PARTITION_PMSG_SIZE, cxt->pmsg_buf);
	if (err != 0) {
		MTN_PRINT_ERR("%s-%d:partition_bm_write error\n", __func__, __LINE__);
		return err;
	}
	/*update flags*/
	if (first_mtdoops_header.flags == 7) {
		first_mtdoops_header.flags = 0;
	} else {
		first_mtdoops_header.flags ++;
	}
	err = partition_bm_write(MTDOOPS_START_ADDR, sizeof(struct mtdoops_header), &first_mtdoops_header);
	if (err != 0) {
		MTN_PRINT_ERR("%s-%d:partition_bm_write fail\n", __func__, __LINE__);
		return err;
	}
	return err;
}

/*reboot powerkey panic handle*/
static int mtdoops_reboot_nb_handle(struct notifier_block *this,
	unsigned long event, void *ptr)
{
	enum mtd_dump_reason reason;
	struct bootmonitor_context *cxt = &boot_cxt;

	if (event == SYS_RESTART)
		reason = MTD_DUMP_RESTART;
	else if(event == SYS_POWER_OFF)
		reason = MTD_DUMP_POWEROFF;
	else
		return NOTIFY_OK;

	mtdoops_do_dump(&cxt->dump, reason);

	return NOTIFY_OK;
}

static int pwrkey_long_press_irq_event(struct notifier_block *this, unsigned long event, 
	void *ptr) {
	struct bootmonitor_context *cxt = &boot_cxt;

	mtdoops_do_dump(&cxt->dump,  MTD_DUMP_LONG_PRESS);
	return NOTIFY_DONE;
}

static int mtdoops_panic_nb_handle(struct notifier_block *this, unsigned long event, 
	void *ptr) {
	//struct bootmonitor_context *cxt = &boot_cxt;
	//mtdoops_do_dump(&cxt->dump,  MTD_DUMP_PANIC);
	MTN_PRINT_ERR("mtdoops_panic_nb_handle test\n");
	return NOTIFY_DONE;
}

/*get mtdoops_pmsg to dtsi*/
static int monitor_parse_dt_u32(struct platform_device *pdev,
				const char *propname,
				u32 default_value, u32 *value)
{
	u32 val32;
	int ret;

	ret = of_property_read_u32(pdev->dev.of_node, propname, &val32);
	if (ret == -EINVAL) {
		/* field is missing, use default value. */
		val32 = default_value;
	} else if (ret < 0) {
		MTN_PRINT_ERR("failed to parse property %s: %d\n",
			propname, ret);
		return ret;
	}
	/* Sanity check our results. */
	if (val32 > INT_MAX) {
		MTN_PRINT_ERR("%s:%u > INT_MAX\n", propname, val32);
		return -EOVERFLOW;
	}
	*value = val32;
	return 0;
}

static int boot_monitor_probe(struct platform_device *pdev)
{
	struct bootmonitor_context *cxt;
	struct resource *res;
	u32 value;
	int ret;
	cxt = &boot_cxt;
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		MTN_PRINT_ERR("failed to locate DT /reserved-memory resource\n");
		return -EINVAL;
	}

	cxt->monitor_data.mem_size = resource_size(res);
	cxt->monitor_data.mem_address = res->start;
	MTN_PRINT_INFO( "cxt->monitor_data.mem_size=0x%lx, cxt->monitor_data.mem_address=0x%llx",
			cxt->monitor_data.mem_size, cxt->monitor_data.mem_address);
#define parse_u32(name, field, default_value) {				\
		ret = monitor_parse_dt_u32(pdev, name, default_value,	\
					    &value);			\
		if (ret < 0)						\
			return ret;					\
		field = value;						\
	}

	parse_u32("console-size", cxt->monitor_data.console_size, 0);
	parse_u32("pmsg-size", cxt->monitor_data.pmsg_size, 0);
	MTN_PRINT_INFO( "console-size =0x%lx, pmsg-size =0x%lx\n",
			cxt->monitor_data.console_size, cxt->monitor_data.pmsg_size);
#undef parse_u32

	return 0;
}

static const struct of_device_id dt_match[] = {
	{ .compatible = "mtdoops" },
	{}
};

struct platform_driver boot_monitor_driver = {
	.probe		= boot_monitor_probe,
	.driver		= {
		.name		= "mtdoops_pmsg",
		.of_match_table	= dt_match,
	},
};

/*add mtd0 device*/
static ssize_t mtdoops_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
    ssize_t ret;

    if (*ppos >= MTD_SIZE)
        return 0;

    if (*ppos + len > MTD_SIZE)
        len = MTD_SIZE - *ppos;

    if (copy_to_user(buf, mtd_buffer + *ppos, len)) {
        MTN_PRINT_ERR("mtdoops_read: copy_to_user failed at offset %lld, len %zu\n", *ppos, len);
        return -EFAULT;
    }

    *ppos += len;
    ret = len;

    return ret;
}

static ssize_t mtdoops_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
    ssize_t ret;

    if (*ppos >= MTD_SIZE)
        return -ENOSPC;

    if (*ppos + len > MTD_SIZE)
        len = MTD_SIZE - *ppos;

    if (copy_from_user(mtd_buffer + *ppos, buf, len)) {
        MTN_PRINT_ERR("mtdoops_write: copy_from_user failed at offset %lld, len %zu\n", *ppos, len);
        return -EFAULT;
    }

    *ppos += len;
    ret = len;

    return ret;
}
static const struct file_operations mtd_fops = {
    .owner = THIS_MODULE,
    .read = mtdoops_read,
    .write = mtdoops_write,
    .llseek = default_llseek,
};

static struct miscdevice mtd_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &mtd_fops,
};

static int mtd_sim_init(void)
{
    int ret;

    mtd_buffer = vmalloc(MTD_SIZE);
    if (!mtd_buffer) {
        MTN_PRINT_ERR("mtd0: Failed to allocate memory.\n");
        return -ENOMEM;
    }

    ret = misc_register(&mtd_misc);
    if (ret) {
        MTN_PRINT_ERR("mtd0: Failed to register misc device.\n");
        vfree(mtd_buffer);
        return ret;
    }

    MTN_PRINT_INFO("mtd0: Initialized 16MB simulated MTD device.\n");
    return 0;
}

/*mtdoops init*/
static int __init mtdoops_init(void)
{
	struct bootmonitor_context *cxt;
	int ret;

	/*get recovery mode*/
	if (strcmp(boot_mode, "recovery") == 0) {
		recovery_flag = 1;
	}

	cxt = &boot_cxt;
	/*for restart and power off*/
	cxt->reboot_nb.notifier_call = mtdoops_reboot_nb_handle;
	cxt->reboot_nb.priority = INT_MIN;
	register_reboot_notifier(&cxt->reboot_nb);

	cxt->panic_nb.notifier_call = mtdoops_panic_nb_handle;
	cxt->panic_nb.priority = INT_MIN;
	atomic_notifier_chain_register(&panic_notifier_list, &cxt->panic_nb);

	cxt->pwrkey_long_press_nb.notifier_call = pwrkey_long_press_irq_event;
	cxt->pwrkey_long_press_nb.priority = INT_MIN;
	raw_notifier_chain_register(&pwrkey_irq_notifier_list,
			&cxt->pwrkey_long_press_nb);

	if (get_bm_devices()) {
		MTN_PRINT_ERR("%s get_bm_devices fail\n", __func__);
		return 0;
	}
	cxt->pmsg_buf = vzalloc(PARTITION_PMSG_SIZE);
	if (!cxt->pmsg_buf) {
		MTN_PRINT_ERR("%s-%d:cxt->pmsg_buf vzalloc fail\n", __func__, __LINE__);
		return 0;
	}
	cxt->dmesg_buf = vzalloc(PARTITION_KMSG_SIZE);
	if (!cxt->dmesg_buf) {
		MTN_PRINT_ERR("%s-%d:cxt->pmsg_buf vzalloc fail\n", __func__, __LINE__);
		if(cxt->pmsg_buf)
			vfree(cxt->pmsg_buf);
		return 0;
	}
	ret = platform_driver_register(&boot_monitor_driver);
	if (ret != 0) {
		MTN_PRINT_ERR("%s-%d:platform_driver_register fail\n", __func__, __LINE__);
		return 0;
	}

	ret = mtd_sim_init();
	if (ret != 0) {
		MTN_PRINT_ERR("%s-%d:mtd_sim_init fail\n", __func__, __LINE__);
		return 0;
	}

	if (IS_ERR(bdev_file) || bdev_file == NULL) {
		MTN_PRINT_ERR("%s-%d:bdev_file is NULL\n", __func__, __LINE__);
		return 0;
	} else {
		if (mtdoops_init_header(cxt)) {
			MTN_PRINT_ERR("%s-%d:mtdoops_init_header fail\n", __func__, __LINE__);
			return 0;
		}
	}
	return 0;
}

static void __exit mtdoops_exit(void)
{
	struct bootmonitor_context *cxt;
	cxt = &boot_cxt;
	MTN_PRINT_INFO("%s exit\n", __func__);
	if(cxt->pmsg_buf)
		vfree(cxt->pmsg_buf);
	if(cxt->dmesg_buf)
		vfree(cxt->dmesg_buf);

	misc_deregister(&mtd_misc);
	vfree(mtd_buffer);
	atomic_notifier_chain_unregister(&panic_notifier_list, &cxt->panic_nb);
	unregister_reboot_notifier(&cxt->reboot_nb);
	raw_notifier_chain_unregister(&pwrkey_irq_notifier_list, &cxt->pwrkey_long_press_nb);
	platform_driver_unregister(&boot_monitor_driver);
}

module_init(mtdoops_init);
module_exit(mtdoops_exit);
MODULE_LICENSE("GPL v2");
