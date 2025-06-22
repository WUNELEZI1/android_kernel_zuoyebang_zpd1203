// SPDX-License-Identifier: GPL-2.0-or-later

#define pr_fmt(fmt)	"[xr_dfx][trace_hook]:%s:%d " fmt, __func__, __LINE__

#include <linux/err.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/smp.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/string.h>
#include <linux/version.h>
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
#include <trace/hooks/xring_maintain.h>
#endif
#include <trace/hooks/sched.h>
#include <trace/hooks/cpuidle.h>
#include <soc/xring/xr_timestamp.h>
#include <soc/xring/trace_hook_set.h>
#include <soc/xring/dfx_switch.h>
#include <dt-bindings/xring/platform-specific/dfx_memory_layout.h>
#include "hook_ringbuffer.h"
#include <trace/events/workqueue.h>
#include <linux/interrupt.h>
#include <trace/events/irq.h>

#define TASK_COMM_LEN			16
#define DIRECTION_IN			1
#define DIRECTION_OUT			0
#define HOOK_MAGIC			0xaabbccdd
#define PERCPU_TOTAL_RATIO		16
#define IRQ_NULL_MAGIC			0xdeadbeef
#define VERSION_OFFSET			(DFX_MEM_HOOK_STRUCT_SIZE - VERSION_LEN_MAX)

struct trace_hook_struct {
	u64 start_addr;
	u32 mem_size;
	void *mem;
};

struct trace_hook_info {
	u32 filedcnt;
	u32 buffer_size;
};

static struct trace_hook_struct *g_trace_hook_struct;
static struct trace_hook_root *g_trace_hook_root;
static struct percpu_buffer_info *g_hook_percpu_buffer[HOOK_PERCPU_TAG] = { NULL };

static atomic_t g_trace_hook_on[HOOK_MAX] = { ATOMIC_INIT(0) };

static enum hook_type g_def_open_hook[] = { HOOK_IRQ, HOOK_TASK };

const char *g_hook_trace_keys[HOOK_MAX] = {
	"irq_switch::name,time,vec,dir",
	"task_switch::time,stack,prepid,nextpid,nextname",
	"cpuidle::time,dir",
	"worker::time,action,pid,dir",
	"softirq::time,vec,dir",
	"",
	"clk::time,name,en_stat,old_rate,new_rate,ops,cpuid,peridvfs,mediadvfs",
	"dvfs::time,voter_name,cur_volt,target_volt,dvs_state,dvs_offset,cpuid,is_low_temp",
	"ufs::cpu,event,pid,clock,duration,tm,utp,uic,clk_gating,dev,clk_scaling,pm",
	"regulator::time,name,ops,ops_success,cpu,mode,use_cnt,open_cnt,min_uv,max_uv,uv_set,t_trace,t_ops",
	"sr_keypoint::time,keypoint",
	"regulator_1::time,name,ops,ops_success,cpu,mode,use_cnt,open_cnt,min_uv,max_uv,uv_set,t_trace,t_ops",
	"ipc::time,vc_id,irq_sta,dir",
};

static const char *g_version = "TraceHookV104";

unsigned int g_record_ratio[HOOK_PERCPU_TAG][HOOK_CPU_NUMBERS][HOOK_CPU_NUMBERS] = {
	{
		/* irq_record_ratio */
		{ 16, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 8, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 4, 4, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 4, 2, 2, 0, 0, 0, 0, 0, 0 },
		{ 8, 4, 2, 1, 1, 0, 0, 0, 0, 0 },
		{ 8, 4, 1, 1, 1, 1, 0, 0, 0, 0 },
		{ 6, 4, 2, 1, 1, 1, 1, 0, 0, 0 },
		{ 6, 4, 1, 1, 1, 1, 1, 1, 0, 0 },
		{ 6, 2, 2, 1, 1, 1, 1, 1, 1, 0 },
		{ 6, 2, 1, 1, 1, 1, 1, 1, 1, 1 }
	},
	{
		/* task_record_ratio */
		{ 16, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 8, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 4, 4, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 4, 2, 2, 0, 0, 0, 0, 0, 0 },
		{ 4, 4, 4, 2, 2, 0, 0, 0, 0, 0 },
		{ 4, 4, 2, 2, 2, 2, 0, 0, 0, 0 },
		{ 4, 4, 2, 2, 2, 1, 1, 0, 0, 0 },
		{ 4, 2, 2, 2, 2, 2, 1, 1, 0, 0 },
		{ 4, 2, 2, 2, 2, 1, 1, 1, 1, 0 },
		{ 4, 2, 2, 2, 1, 1, 1, 1, 1, 1 }
	},
	{
		/* cpuidle_record_ratio */
		{ 16, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 8, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 4, 4, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 4, 2, 2, 0, 0, 0, 0, 0, 0 },
		{ 4, 4, 4, 2, 2, 0, 0, 0, 0, 0 },
		{ 4, 4, 2, 2, 2, 2, 0, 0, 0, 0 },
		{ 4, 4, 2, 2, 2, 1, 1, 0, 0, 0 },
		{ 4, 3, 3, 2, 1, 1, 1, 1, 0, 0 },
		{ 4, 3, 2, 2, 1, 1, 1, 1, 1, 0 },
		{ 4, 3, 2, 1, 1, 1, 1, 1, 1, 1 }
	},
	{
		/* worker_record_ratio */
		{ 16, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 8, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 4, 4, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 4, 2, 2, 0, 0, 0, 0, 0, 0 },
		{ 4, 4, 4, 2, 2, 0, 0, 0, 0, 0 },
		{ 4, 4, 2, 2, 2, 2, 0, 0, 0, 0 },
		{ 4, 4, 2, 2, 2, 1, 1, 0, 0, 0 },
		{ 4, 3, 3, 2, 1, 1, 1, 1, 0, 0 },
		{ 4, 3, 2, 2, 1, 1, 1, 1, 1, 0 },
		{ 4, 3, 2, 1, 1, 1, 1, 1, 1, 1 }
	},
	{
		/* softirq_record_ratio */
		{ 16, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 8, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 4, 4, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 4, 2, 2, 0, 0, 0, 0, 0, 0 },
		{ 8, 4, 2, 1, 1, 0, 0, 0, 0, 0 },
		{ 8, 4, 1, 1, 1, 1, 0, 0, 0, 0 },
		{ 6, 4, 2, 1, 1, 1, 1, 0, 0, 0 },
		{ 6, 4, 1, 1, 1, 1, 1, 1, 0, 0 },
		{ 6, 2, 2, 1, 1, 1, 1, 1, 1, 0 },
		{ 6, 2, 1, 1, 1, 1, 1, 1, 1, 1 }
	}
};

struct trace_hook_info g_hook_info[HOOK_MAX] = {
	{sizeof(struct irq_hook_info), IRQ_HOOK_SIZE},
	{sizeof(struct task_hook_info), TASK_HOOK_SIZE},
	{sizeof(struct cpuidle_hook_info), CPUIDLE_HOOK_SIZE},
	{sizeof(struct worker_hook_info), WORKER_HOOK_SIZE},
	{sizeof(struct softirq_hook_info), SOFTIRQ_HOOK_SIZE},
	{0, 0},
	{sizeof(struct clk_hook_info), CLK_HOOK_SIZE},
	{sizeof(struct dvfs_hook_info), DVFS_HOOK_SIZE},
	{sizeof(struct ufs_hook_info), UFS_HOOK_SIZE},
	{sizeof(struct regulator_hook_info), REGULATOR_HOOK_SIZE},
	{sizeof(struct sr_keypoint_hook_info), SR_KEYPOINT_HOOK_SIZE},
	{sizeof(struct regulator_hook_info), REGULATOR_HOOK_1_SIZE},
	{sizeof(struct ipc_hook_info), IPC_HOOK_SIZE},
};

static struct trace_hook_struct *trace_hook_struct_get(void)
{
	return g_trace_hook_struct;
}

static void trace_hook_struct_set(struct trace_hook_struct *trace_hook_struct)
{
	if (trace_hook_struct != NULL)
		g_trace_hook_struct = trace_hook_struct;
}

struct trace_hook_root *trace_hook_root_get(void)
{
	return g_trace_hook_root;
}
EXPORT_SYMBOL(trace_hook_root_get);

static int single_buffer_init(unsigned char *addr, unsigned int size, enum hook_type hk,
			unsigned int fieldcnt)
{
	int ret;
	unsigned int min_size = sizeof(struct hook_ringbuffer) + fieldcnt;

	if (!addr) {
		pr_err("argument addr is invalid\n");
		return -EINVAL;
	}

	if ((hk <= HOOK_PERCPU_TAG) || (hk >= HOOK_MAX)) {
		pr_err("nonpercpu hook_type {%d} is invalid\n", hk);
		return -EINVAL;
	}

	if (size < min_size) {
		g_trace_hook_root->hook_buffer_addr[hk] = 0;
		pr_err("buffer_size {0x%x} is not enough\n", size);
		return 0;
	}
	g_trace_hook_root->hook_buffer_addr[hk] = addr;

	ret = ringbuffer_init((struct hook_ringbuffer *)addr, size, fieldcnt,
				g_hook_trace_keys[hk]);
	if (ret) {
		pr_err("hook {%d} ringbuffer init failed\n", hk);
		return ret;
	}

	return 0;
}

static int percpu_buffer_init(struct percpu_buffer_info *buffer_info, u32 ratio[][HOOK_CPU_NUMBERS],
		u32 cpu_num, u32 fieldcnt, const char *keys, u32 gap)
{
	unsigned int i;
	int ret;
	struct percpu_buffer_info *buffer = buffer_info;

	if (!keys) {
		pr_err("percpu_buffer argument keys is NULL\n");
		return -EINVAL;
	}

	if (!buffer) {
		pr_err("percpu_buffer info is null\n");
		return -EINVAL;
	}

	if (!buffer->buffer_addr) {
		pr_err("percpu_buffer_addr is NULL\n");
		return -EINVAL;
	}

	for (i = 0; i < cpu_num; i++) {
		buffer->percpu_length[i] = (buffer->buffer_size - cpu_num * gap)
			/ PERCPU_TOTAL_RATIO * ratio[cpu_num - 1][i];

		if (i == 0)
			buffer->percpu_addr[0] = buffer->buffer_addr;
		else
			buffer->percpu_addr[i] = buffer->percpu_addr[i - 1] +
				buffer->percpu_length[i - 1] + gap;

		ret = ringbuffer_init((struct hook_ringbuffer *) buffer->percpu_addr[i],
					   buffer->percpu_length[i], fieldcnt, keys);
		if (ret) {
			pr_err("cpu {%u} ringbuffer init failed\n", i);
			return ret;
		}
	}
	return 0;
}

static int hook_percpu_buffer_init(struct percpu_buffer_info *buffer_info,
		unsigned char *addr, u32 size, u32 fieldcnt,
		enum hook_type hook, u32 ratio[][HOOK_CPU_NUMBERS])
{
	u64 min_size;
	u32 cpu_num = HOOK_CPU_NUMBERS;

	if (IS_ERR_OR_NULL(addr) || IS_ERR_OR_NULL(buffer_info)) {
		pr_err("addr is null or buffer_info is null\n");
		return -EINVAL;
	}

	min_size = cpu_num * (sizeof(struct hook_ringbuffer) + PERCPU_TOTAL_RATIO *
		(u64)(unsigned int)fieldcnt);
	if (size < (u32)min_size) {
		g_trace_hook_root->hook_buffer_addr[hook] = 0;
		g_hook_percpu_buffer[hook] = buffer_info;
		g_hook_percpu_buffer[hook]->buffer_addr = 0;
		g_hook_percpu_buffer[hook]->buffer_size = 0;
		pr_err("buffer_size {0x%x} is not enough\n", size);
		return 0;
	}

	if (hook >= HOOK_PERCPU_TAG) {
		pr_err("hook_type {%d} is invalid\n", hook);
		return -EINVAL;
	}

	g_trace_hook_root->hook_buffer_addr[hook] = addr;
	g_hook_percpu_buffer[hook] = buffer_info;
	g_hook_percpu_buffer[hook]->buffer_addr = addr;
	g_hook_percpu_buffer[hook]->buffer_size = size;

	return percpu_buffer_init(buffer_info, ratio, cpu_num,
				  fieldcnt, g_hook_trace_keys[hook], 0);
}

int hook_buffer_alloc(void)
{
	int hook;
	int ret;

	pr_info("hook_buffer_init start\n");
	for (hook = HOOK_IRQ; hook < HOOK_PERCPU_TAG; hook++) {
		ret = hook_percpu_buffer_init(g_hook_percpu_buffer[hook],
			g_trace_hook_root->hook_buffer_addr[hook],
			g_trace_hook_root->hook_buffer_size[hook],
			g_hook_info[hook].filedcnt, hook, g_record_ratio[hook]);
		if (ret) {
			pr_err("%d buffer_init failed\n", hook);
			return ret;
		}
	}

	for (hook = HOOK_PERCPU_TAG + 1; hook < HOOK_MAX; hook++) {
		ret = single_buffer_init(g_trace_hook_root->hook_buffer_addr[hook],
			g_trace_hook_root->hook_buffer_size[hook],
			hook, g_hook_info[hook].filedcnt);
		if (ret) {
			pr_err("%d buffer_init failed\n", hook);
			return ret;
		}
	}

	return 0;
}

static int set_trace_hook_mem_size(void)
{
	u32 used_memory_size = 0;
	int hook;

	for (hook = HOOK_IRQ; hook < HOOK_MAX; hook++) {
		used_memory_size += g_hook_info[hook].buffer_size;
		if (used_memory_size > DFX_MEM_HOOK_SIZE) {
			pr_err("not enough memory allocated to hook type {%d}\n.", hook);
			return -ENOEXEC;
		}
		g_trace_hook_root->hook_buffer_size[hook] = g_hook_info[hook].buffer_size;
	}

	return 0;
}

static int set_trace_hook_mem_start(void)
{
	int i;
	struct trace_hook_struct *trace_hook_struct = trace_hook_struct_get();

	if (IS_ERR_OR_NULL(trace_hook_struct->mem)) {
		pr_err("trace_hook_struct->mem is err\n");
		return -EINVAL;
	}
	g_trace_hook_root->magic_num = HOOK_MAGIC;

	(void)strncpy((void *)g_trace_hook_root + VERSION_OFFSET, g_version, VERSION_LEN_MAX);
	pr_info("version: %s\n", g_version);

	for (i = 0; i < HOOK_PERCPU_TAG; i++)
		g_hook_percpu_buffer[i] = &(g_trace_hook_root->percpu_buffer[i]);

	g_trace_hook_root->hook_buffer_addr[0] = trace_hook_struct->mem +
		DFX_MEM_HOOK_STRUCT_SIZE;

	for (i = 1; i < HOOK_MAX; i++)
		g_trace_hook_root->hook_buffer_addr[i] =
			g_trace_hook_root->hook_buffer_addr[i - 1] +
			g_trace_hook_root->hook_buffer_size[i - 1];

	return 0;
}

int hook_install(int hook)
{
	if (hook >= HOOK_MAX) {
		pr_err("hoot type {%d} is invalid\n", hook);
		return -EINVAL;
	}

	if (g_trace_hook_root->hook_buffer_addr[hook])
		atomic_set(&g_trace_hook_on[hook], 1);

	return 0;
}

static bool is_def_open_hook(enum hook_type hk)
{
	int i;

	if (hk >= HOOK_MAX) {
		pr_err("invalid hoot type {%d}\n", hk);
		return false;
	}

	for (i = 0; i < ARRAY_SIZE(g_def_open_hook); i++) {
		if (g_def_open_hook[i] == hk)
			return true;
	}

	return false;
}

int trace_hook_install(void)
{
	int ret = 0;
	int hook;

	for (hook = HOOK_IRQ; hook < HOOK_MAX; hook++) {
		if (is_def_open_hook(hook)) {
			if (!is_switch_on(SWITCH_HOOK_DEFAULT))
				continue;
		} else {
			if (!is_switch_on(SWITCH_HOOK_OTHER))
				continue;
		}

		ret = hook_install(hook);
		if (ret) {
			pr_err("hook type {%d} install failed\n", hook);
			return ret;
		}
	}
	return ret;
}

static void irq_info_config(struct irq_hook_info *info, int irq)
{
	struct irq_data *data = NULL;
	struct irq_chip *chip = NULL;

	info->clock = xr_timestamp_gettime();

	data = irq_get_irq_data(irq);
	if (data != NULL) {
		info->irq = (u32)data->hwirq;
	} else {
		info->irq = IRQ_NULL_MAGIC;
		pr_err("get desc fail\n");
	}

	chip = irq_get_chip(irq);
	if ((chip == NULL) || (chip->name == NULL)) {
		pr_err("get chip fail\n");
		return;
	}

	if (strlen(chip->name) >= IRQ_NAME_LEN_MAX) {
		(void)strncpy(info->name, chip->name, IRQ_NAME_LEN_MAX);
		info->name[IRQ_NAME_LEN_MAX - 1] = '\0';
	} else {
		(void)strncpy(info->name, chip->name, strlen(chip->name));
		info->name[strlen(chip->name)] = '\0';
	}
}

static void irq_enter_handler(void *unused, int irq, struct irqaction *action)
{
	struct irq_hook_info info;
	u8 cpu;

	if (!atomic_read(&g_trace_hook_on[HOOK_IRQ]))
		return;

	irq_info_config(&info, irq);
	cpu = (u8)smp_processor_id();
	info.dir = DIRECTION_IN;

	ringbuffer_write((struct hook_ringbuffer *)
				g_hook_percpu_buffer[HOOK_IRQ]->percpu_addr[cpu],
				(u8 *)&info);
}

static void irq_exit_handler(void *unused, int irq, struct irqaction *action, int ret)
{
	struct irq_hook_info info;
	u8 cpu;

	if (!atomic_read(&g_trace_hook_on[HOOK_IRQ]))
		return;

	irq_info_config(&info, irq);
	cpu = (u8)smp_processor_id();
	info.dir = DIRECTION_OUT;

	ringbuffer_write((struct hook_ringbuffer *)
				g_hook_percpu_buffer[HOOK_IRQ]->percpu_addr[cpu],
				(u8 *)&info);
}

static void softirq_entry_handler(void *unused, u32 vec_nr)
{
	struct softirq_hook_info info;
	u8 cpu;

	if (!atomic_read(&g_trace_hook_on[HOOK_SOFTIRQ]))
		return;

	info.clock = xr_timestamp_gettime();
	info.irq = vec_nr;
	info.dir = DIRECTION_IN;
	cpu = (u8)smp_processor_id();

	ringbuffer_write((struct hook_ringbuffer *)
			g_hook_percpu_buffer[HOOK_SOFTIRQ]->percpu_addr[cpu],
			(u8 *)&info);
}

static void softirq_exit_handler(void *unused, u32 vec_nr)
{
	struct softirq_hook_info info;
	u8 cpu;

	if (!atomic_read(&g_trace_hook_on[HOOK_SOFTIRQ]))
		return;

	info.clock = xr_timestamp_gettime();
	info.irq = vec_nr;
	info.dir = DIRECTION_OUT;
	cpu = (u8)smp_processor_id();

	ringbuffer_write((struct hook_ringbuffer *)
			g_hook_percpu_buffer[HOOK_SOFTIRQ]->percpu_addr[cpu],
			(u8 *)&info);
}

static void android_rvh_task_switch_handler(void *unused, struct task_struct *prev,
		struct task_struct *next)
{
	struct task_hook_info info;
	u8 cpu;

	if (!prev || !next) {
		pr_err("task switch error:prev_task or next_task is NULL\n");
		return;
	}

	if (!atomic_read(&g_trace_hook_on[HOOK_TASK]))
		return;

	info.clock = xr_timestamp_gettime();
	cpu = (u8)smp_processor_id();
	info.prev_pid = (u32)prev->pid;
	info.next_pid = (u32)next->pid;
	info.stack = (uintptr_t)next->stack;
	(void)strncpy(info.comm, next->comm, sizeof(next->comm) - 1);
	info.comm[TASK_COMM_LEN - 1] = '\0';

	ringbuffer_write((struct hook_ringbuffer *)
				g_hook_percpu_buffer[HOOK_TASK]->percpu_addr[cpu],
				(u8 *)&info);
}

static void android_vh_cpu_idle_enter_handler(void *unused, int *state,
		struct cpuidle_device *dev)
{
	struct cpuidle_hook_info info;
	u8 cpu;

	if (!atomic_read(&g_trace_hook_on[HOOK_CPUIDLE]))
		return;

	info.clock = xr_timestamp_gettime();
	cpu = (u8)smp_processor_id();
	info.dir = DIRECTION_IN;

	ringbuffer_write((struct hook_ringbuffer *)
				g_hook_percpu_buffer[HOOK_CPUIDLE]->percpu_addr[cpu],
				(u8 *)&info);
}

static void android_vh_cpu_idle_exit_handler(void *unused, int state,
		struct cpuidle_device *dev)
{
	struct cpuidle_hook_info info;
	u8 cpu;

	if (!atomic_read(&g_trace_hook_on[HOOK_CPUIDLE]))
		return;

	info.clock = xr_timestamp_gettime();
	cpu = (u8)smp_processor_id();
	info.dir = DIRECTION_OUT;

	ringbuffer_write((struct hook_ringbuffer *)
				g_hook_percpu_buffer[HOOK_CPUIDLE]->percpu_addr[cpu],
				(u8 *)&info);
}

static void worker_enter_handler(void *unused, struct work_struct *work)
{
	struct worker_hook_info info;
	u8 cpu;

	if (!atomic_read(&g_trace_hook_on[HOOK_WORKER]))
		return;

	info.clock = xr_timestamp_gettime();
	cpu = (u8)smp_processor_id();
	info.action = (u64)work->func;
	info.pid = (u32)current->pid;
	info.dir = DIRECTION_IN;

	ringbuffer_write((struct hook_ringbuffer *)
				g_hook_percpu_buffer[HOOK_WORKER]->percpu_addr[cpu],
				(u8 *)&info);
}

static void worker_exit_handler(void *unused, struct work_struct *work, work_func_t function)
{
	struct worker_hook_info info;
	u8 cpu;

	if (!atomic_read(&g_trace_hook_on[HOOK_WORKER]))
		return;

	info.clock = xr_timestamp_gettime();
	cpu = (u8)smp_processor_id();
	info.action = (u64)function;
	info.pid = (u32)current->pid;
	info.dir = DIRECTION_OUT;

	ringbuffer_write((struct hook_ringbuffer *)
				g_hook_percpu_buffer[HOOK_WORKER]->percpu_addr[cpu],
				(u8 *)&info);
}

void set_sr_keypoint(u32 keypoint)
{
	struct sr_keypoint_hook_info info;

	if (!atomic_read(&g_trace_hook_on[HOOK_SR_KEYPOINT]))
		return;

	info.clock = xr_timestamp_gettime();
	info.keypoint = keypoint;

	ringbuffer_write((struct hook_ringbuffer *)
			g_trace_hook_root->hook_buffer_addr[HOOK_SR_KEYPOINT],
			(u8 *)&info);
}
EXPORT_SYMBOL(set_sr_keypoint);

static int irq_trace_hook_register(void)
{
	int ret;

	ret = register_trace_irq_handler_entry(irq_enter_handler, NULL);
	if (ret) {
		pr_err("register_trace_irq_handler_entry failed(ret = %d)\n", ret);
		return ret;
	}
	ret = register_trace_irq_handler_exit(irq_exit_handler, NULL);
	if (ret) {
		pr_err("register_trace_irq_handler_exit failed(ret = %d)\n", ret);
		return ret;
	}

	pr_info("irq_trace init success\n");

	return 0;
}

static int task_switch_hook_register(void)
{
	int ret;

	ret = register_trace_android_rvh_context_switch(android_rvh_task_switch_handler, NULL);
	if (ret) {
		pr_err("register_trace_android_rvh_task_switch failed(ret = %d)\n", ret);
		return ret;
	}

	pr_info("rvh_task_switch init success\n");

	return 0;
}

static int cpu_idle_stat_hook_register(void)
{
	int ret;

	ret = register_trace_android_vh_cpu_idle_enter(android_vh_cpu_idle_enter_handler, NULL);
	if (ret) {
		pr_err("register_trace_android_vh_cpu_idle_enter failed(ret = %d)\n", ret);
		return -ENOEXEC;
	}
	ret = register_trace_android_vh_cpu_idle_exit(android_vh_cpu_idle_exit_handler, NULL);
	if (ret) {
		pr_err("register_trace_android_vh_cpu_idle_exit failed(ret = %d)\n", ret);
		return -ENOEXEC;
	}

	pr_info("vh_cpuidle init success\n");

	return 0;
}

static int worker_hook_register(void)
{
	int ret;

	ret = register_trace_workqueue_execute_start(worker_enter_handler, NULL);
	if (ret) {
		pr_err("register_trace_workqueue_execute_start failed(ret = %d)\n", ret);
		return -ENOEXEC;
	}

	ret = register_trace_workqueue_execute_end(worker_exit_handler, NULL);
	if (ret) {
		pr_err("register_trace_workqueue_execute_end failed(ret = %d)\n", ret);
		return -ENOEXEC;
	}

	pr_info("vh_worker init success\n");

	return 0;
}

static int softirq_hook_register(void)
{
	int ret;

	ret = register_trace_softirq_entry(softirq_entry_handler, NULL);
	if (ret) {
		pr_err("register_trace_softirq_entry failed(ret = %d)\n", ret);
		return ret;
	}

	ret = register_trace_softirq_exit(softirq_exit_handler, NULL);
	if (ret) {
		pr_err("register_trace_softirq_exit failed(ret = %d)\n", ret);
		return ret;
	}

	pr_info("softirq init success\n");

	return 0;
}

static int trace_init(void)
{
	int ret = 0;

	g_trace_hook_root = (struct trace_hook_root *) g_trace_hook_struct->mem;

	ret = set_trace_hook_mem_size();
	if (ret) {
		pr_err("set trace hook memory size failed\n");
		return -ENOEXEC;
	}

	ret = set_trace_hook_mem_start();
	if (ret) {
		pr_err("set trace hook memory start failed\n");
		return -ENOEXEC;
	}

	ret = hook_buffer_alloc();
	if (ret) {
		pr_err("trace hook buffer alloc failed\n");
		return -ENOEXEC;
	}

	ret = trace_hook_install();
	if (ret) {
		pr_err("trace hook install failed\n");
		return -ENOEXEC;
	}

	ret = irq_trace_hook_register();
	if (ret) {
		pr_err("irq trace hook register failed\n");
		return -ENOEXEC;
	}

	ret = task_switch_hook_register();
	if (ret) {
		pr_err("task switch hook register failed\n");
		return -ENOEXEC;
	}

	ret = cpu_idle_stat_hook_register();
	if (ret) {
		pr_err("cpu idle stat hook register failed\n");
		return -ENOEXEC;
	}

	ret = worker_hook_register();
	if (ret) {
		pr_err("worker hook register failed\n");
		return -ENOEXEC;
	}

	ret = softirq_hook_register();
	if (ret) {
		pr_err("softirq hook register failed\n");
		return -ENOEXEC;
	}

	return 0;
}

int trace_hook_init(void)
{
	struct trace_hook_struct *trace_hook_struct = NULL;
	int ret = 0;

	pr_info("trace_hook driver loaded\n");
	trace_hook_struct = kzalloc(sizeof(struct trace_hook_struct), GFP_KERNEL);
	if (!trace_hook_struct) {
		pr_err("kzalloc memory for trace_hook_struct failed\n");
		return -ENOMEM;
	}

	trace_hook_struct->start_addr = DFX_MEM_HOOK_STRUCT_ADDR;
	trace_hook_struct->mem_size = DFX_MEM_HOOK_STRUCT_SIZE + DFX_MEM_HOOK_SIZE;
	trace_hook_struct->mem = ioremap_wt(trace_hook_struct->start_addr,
		trace_hook_struct->mem_size);
	if (!trace_hook_struct->mem) {
		pr_err("map device memory failed\n");
		ret = -ENOMEM;
		goto exit;
	}

	trace_hook_struct_set(trace_hook_struct);
	ret = trace_init();
	if (ret < 0) {
		pr_err("trace hook init failed\n");
		goto exit;
	}

exit:
	kfree(trace_hook_struct);
	g_trace_hook_struct = NULL;
	return ret;
}
