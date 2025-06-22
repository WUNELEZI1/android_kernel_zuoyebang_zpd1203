/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef __TRACE_HOOK_SET_H__
#define __TRACE_HOOK_SET_H__

#define MAX_CPU_NUM			10
#define HOOK_CPU_NUMBERS		10
#define IRQ_HOOK_SIZE			(512 * 1024)
#define TASK_HOOK_SIZE			(512 * 1024)
#define CPUIDLE_HOOK_SIZE		(256 * 1024)
#define WORKER_HOOK_SIZE		(64 * 1024)
#define CLK_HOOK_SIZE			(72 * 1024)
#define DVFS_HOOK_SIZE			(10 * 1024)
#define UFS_HOOK_SIZE			(32 * 1024)
#define REGULATOR_HOOK_SIZE		(64 * 1024)
#define REGULATOR_HOOK_1_SIZE		(32 * 1024)
#define SOFTIRQ_HOOK_SIZE		(16 * 1024)
#define SR_KEYPOINT_HOOK_SIZE		(4 * 1024)
#define IPC_HOOK_SIZE			(32 * 1024)

#define VERSION_LEN_MAX			32
#define IRQ_NAME_LEN_MAX		16
#define CLK_NAME_LEN_MAX		32
#define DVFS_VOTER_NAME_LEN_MAX		32
#define REGULATOR_NAME_LEN_MAX		28

enum hook_type {
	HOOK_IRQ = 0,
	HOOK_TASK,
	HOOK_CPUIDLE,
	HOOK_WORKER,
	HOOK_SOFTIRQ,
	HOOK_PERCPU_TAG,
 	HOOK_CLK,
	HOOK_DVFS,
	HOOK_UFS,
	HOOK_REGULATOR,
	HOOK_SR_KEYPOINT,
	HOOK_REGULATOR_1,
	HOOK_IPC,
	HOOK_MAX
};

struct percpu_buffer_info {
	unsigned char *buffer_addr;
	unsigned char *percpu_addr[MAX_CPU_NUM];
	unsigned int percpu_length[MAX_CPU_NUM];
	unsigned int buffer_size;
};

struct trace_hook_root {
	u64 magic_num;
	unsigned char *hook_buffer_addr[HOOK_MAX];
	u32 hook_buffer_size[HOOK_MAX];
	struct percpu_buffer_info percpu_buffer[HOOK_PERCPU_TAG];
};

struct irq_hook_info {
	char name[IRQ_NAME_LEN_MAX];
	u64 clock;
	u32 irq;
	u8 dir;
};

struct softirq_hook_info {
	u64 clock;
	u32 irq;
	u8 dir;
};

struct task_hook_info {
	u64 clock;
	u64 stack;
	u32 prev_pid;
	u32 next_pid;
	char comm[TASK_COMM_LEN];
};

struct cpuidle_hook_info {
	u64 clock;
	u8 dir;
};

struct worker_hook_info {
	u64 clock;
	u64 action;
	u32 pid;
	u8 dir;
};

struct dvfs_hook_info {
	u64 local_time;
	char voter_name[DVFS_VOTER_NAME_LEN_MAX];
	u32 cur_volt;
	u32 target_volt;
	u32 dvs_state;
	int16_t dvs_offset;
	u8 cpu_id;
	bool is_low_temp;
};

struct clk_hook_info {
	ktime_t local_time;
	char clk_name[CLK_NAME_LEN_MAX];
	unsigned int enable_state;
	unsigned long old_rate;
	unsigned long new_rate;
	unsigned short ops_type;
	unsigned short cpu_id;
	unsigned int peri_dvfs_vote;
	unsigned int media_dvfs_vote;
};

struct ipc_hook_info {
	u64 clock;
	u8 vc_id;
	u8 irq_sta;
	u8 dir;
};

/* ufs hook info struct */
enum cmd_hist_event {
	CMD_SEND = 0,
	CMD_COMPLETED,
	CMD_DEV_SEND,
	CMD_DEV_COMPLETED,
	CMD_TM_SEND,
	CMD_TM_COMPLETED,
	CMD_TM_COMPLETED_ERR,
	CMD_UIC_SEND,
	CMD_UIC_CMPL_GENERAL,
	CMD_UIC_CMPL_PWR_CTRL,
	CMD_CLK_GATING,
	CMD_CLK_SCALING,
	CMD_PM,
	CMD_UNKNOWN
};
struct tm_cmd_struct {
	u8 lun;
	u8 tag;
	u8 task_tag;
	u16 tm_func;
};

struct utp_cmd_struct {
	u8 opcode;
	u16 tag;
	u32 doorbell;
	u32 intr;
	int transfer_len;
	u64 lba;
};

struct uic_cmd_struct {
	u8 cmd;
	u32 arg1;
	u32 arg2;
	u32 arg3;
};

struct clk_gating_event_struct {
	u8 state;
};

struct dev_cmd_struct {
	u8 type;
	u8 opcode;
	u8 idn;
	u8 index;
	u8 selector;
	u16 tag;
};

struct clk_scaling_event_struct {
	u8 state;
	int err;
};

struct ufs_pm_struct {
	u8 state;
	int err;
	s64 time_us;
	int pwr_mode;
	int link_state;
};

struct ufs_hook_info {
	u8 cpu;
	enum cmd_hist_event event;
	pid_t pid;
	u64 clock;
	u64 duration;
	union {
		struct tm_cmd_struct tm;
		struct utp_cmd_struct utp;
		struct uic_cmd_struct uic;
		struct clk_gating_event_struct clk_gating;
		struct dev_cmd_struct dev;
		struct clk_scaling_event_struct clk_scaling;
		struct ufs_pm_struct pm;
	} cmd;
};

enum regulator_hook_ops_type {
	REGULATOR_HOOK_OPS_ENABLE = 0,
	REGULATOR_HOOK_OPS_DISABLE,
	REGULATOR_HOOK_OPS_FORCE_DISABLE,
	REGULATOR_HOOK_OPS_SET_VOLTAGE,
	REGULATOR_HOOK_OPS_SET_MODE,
	REGULATOR_HOOK_OPS_NUM,
};

/**
 * regulator hook record info
 * (size: 64 bytes)
 *
 * @local_time:    record the timestamp.
 * @ops_type:      regulator operation type, type: enum regulator_hook_ops_type.
 * @ops_success:   the regulator operation status,
 *                 1: sucess, 0: fail.
 * @cpu_id:        regulator ops executed on which cpu id.
 * @mode:          regulator operating mode.
 * @use_count:     the regulator dev is enabled by how many consumers.
 * @open_count:    the regulator dev is opened by how many consumers.
 * @min_uv_set:    record the regulator_set_voltage param max_uV,
 *                 only valid when ops_type is REGULATOR_HOOK_OPS_SET_VOLTAGE
 * @max_uv_set:    record the regulator_set_voltage param min_uV,
 *                 only valid when ops_type is REGULATOR_HOOK_OPS_SET_VOLTAGE
 * @t_last_trace_save:  the last same ops_type record save to DFX mem
 *                      cost time (ns).
 * @t_ops_call:    the ops_type call cost time (ns),
 */
struct regulator_hook_info {
	u64 local_time;
	char name[REGULATOR_NAME_LEN_MAX];
	u8 ops_type;
	u8 ops_success;
	u8 cpu_id;
	u8 mode;
	u16 use_count;
	u16 open_count;
	u32 min_uv_set;
	u32 max_uv_set;
	u32 uv_set;
	u32 t_last_trace_save;
	u32 t_ops_call;
};

struct sr_keypoint_hook_info {
	u64 clock;
	u32 keypoint;
};

struct trace_hook_root *trace_hook_root_get(void);
void set_sr_keypoint(u32 keypoint);

#endif /* __TRACE_HOOK_SET_H__ */
