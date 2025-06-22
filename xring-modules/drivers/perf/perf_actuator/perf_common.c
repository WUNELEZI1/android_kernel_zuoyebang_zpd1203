// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#include <uapi/linux/sched/types.h>
#include <linux/sched/cputime.h>
#include <linux/sched/prio.h>
#include <linux/sched.h>
#include <linux/arm-smccc.h>
#include <soc/xring/perf_actuator.h>
#include <soc/xring/xr-clk-provider.h>
#include <dt-bindings/xring/platform-specific/common/xhee_hvc/hvc_id.h>
#include <soc/xring/xhee_status.h>

/* use cap to track the difference of the ioctl cmd */
enum {
	CAP_AI_SCHED_COMM_CMD = 0,
	CAP_RTG_CMD,
	CAP_RT_TRACKER_CMD,
};

enum sg_xhee_task_type {
	SG_XHEE_GET_REGULATE_RES = 0,
	SG_XHEE_GET_DISCOUNT_COEF,
};

struct sched_prio {
	pid_t pid;
	int priority;
};

#define SG_REGULATE_ARGS_NUM	8

struct frame_regulate_info {
	uint64_t args[SG_REGULATE_ARGS_NUM];
};

#define CMD_GET_DEV_CAP		PERF_R_CMD(GET_DEV_CAP, unsigned long)
#define CMD_SET_SCHED_PRIO	PERF_W_CMD(SET_SCHED_PRIO, struct sched_prio)
#define CMD_GET_REGULATE_RES	PERF_R_CMD(GET_REGULATE_RES, struct frame_regulate_info)
#define CMD_GET_DISCOUNT_COEF	PERF_R_CMD(GET_DISCOUNT_COEF, struct frame_regulate_info)
#define CMD_SET_PERI_RATE	PERF_W_CMD(SET_PERI_RATE, unsigned int)

static int perf_actuator_get_dev_cap(void __user *uarg)
{
	unsigned long cap = 0;

	if (uarg == NULL)
		return -EINVAL;

	cap |= BIT(CAP_AI_SCHED_COMM_CMD);
	cap |= BIT(CAP_RTG_CMD);
	cap |= BIT(CAP_RT_TRACKER_CMD);

	if (copy_to_user(uarg, &cap, sizeof(unsigned long)))
		return -EFAULT;

	return 0;
}

static int perf_actuator_set_sched_prio(void __user *uarg)
{
	struct sched_prio sched_val;
	struct task_struct *task = NULL;
	struct sched_param param;
	int policy;
	int ret;
	int nice;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&sched_val, uarg, sizeof(struct sched_prio)))
		return -EFAULT;

	if (sched_val.priority >= 140 || sched_val.priority < 0)
		return -EFAULT;

	param.sched_priority = sched_val.priority;
	if (param.sched_priority >= 100) {
		nice = PRIO_TO_NICE(param.sched_priority);
		policy = SCHED_NORMAL;
		param.sched_priority = 0;
	} else {
		policy = SCHED_FIFO;
		param.sched_priority = 100 - param.sched_priority;
	}

	rcu_read_lock();
	task = find_task_by_vpid(sched_val.pid);
	if (task == NULL) {
		rcu_read_unlock();
		return -ENOENT;
	}

	ret = sched_setscheduler_nocheck(task, policy, &param);
	if (param.sched_priority == 0)
		set_user_nice(task, nice);
	rcu_read_unlock();

	return ret;
}

static int xhee_hvc_regulate(struct frame_regulate_info *frame_info_val, enum sg_xhee_task_type type)
{
	/* check whether xhee is enabled */
	if (!xhee_status_enabled()) {
		pr_err("perf_actuator: xhee is disabled, so regulator would not visit hvc\n");
		return -EINVAL;
	}

	struct arm_smccc_res res = { 0ULL };

	/* regulate in xhee */
	switch (type) {
	case SG_XHEE_GET_REGULATE_RES:
		arm_smccc_hvc(FID_XHEE_SCHEDGENIUS_REGULATE, frame_info_val->args[1], frame_info_val->args[2],
			frame_info_val->args[3], frame_info_val->args[4], frame_info_val->args[5],
			frame_info_val->args[6], frame_info_val->args[7], &res);
		break;
	case SG_XHEE_GET_DISCOUNT_COEF:
		arm_smccc_hvc(FID_XHEE_SCHEDGENIUS_DISCOUNT, frame_info_val->args[1], frame_info_val->args[2],
			frame_info_val->args[3], frame_info_val->args[4], frame_info_val->args[5],
			frame_info_val->args[6], frame_info_val->args[7], &res);
		break;
	default:
		pr_err("perf_actuator: hvc does not support this task type\n");
		return -EINVAL;
	}

	if (res.a0 != 0) {
		pr_err("perf_actuator: hvc does not return success value\n");
		return -EINVAL;
	}

	frame_info_val->args[1] = res.a1;
	frame_info_val->args[2] = res.a2;
	frame_info_val->args[3] = res.a3;

	return 0;
}

static int perf_actuator_sg_regulate(void __user *uarg)
{
	struct frame_regulate_info frame_info_val = { 0ULL };

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&frame_info_val, uarg, sizeof(struct frame_regulate_info)))
		return -EFAULT;

	int ret = xhee_hvc_regulate(&frame_info_val, SG_XHEE_GET_REGULATE_RES);

	if (ret != 0)
		return ret;

	if (copy_to_user(uarg, &frame_info_val, sizeof(struct frame_regulate_info)))
		ret = -EFAULT;

	return ret;
}

static int perf_actuator_get_discount_coef(void __user *uarg)
{
	struct frame_regulate_info picked_freq_comb_info = { 0ULL };

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&picked_freq_comb_info, uarg, sizeof(struct frame_regulate_info)))
		return -EFAULT;

	int ret = xhee_hvc_regulate(&picked_freq_comb_info, SG_XHEE_GET_DISCOUNT_COEF);

	if (ret != 0)
		return ret;

	if (copy_to_user(uarg, &picked_freq_comb_info, sizeof(struct frame_regulate_info)))
		ret = -EFAULT;

	return ret;
}

static int perf_actuator_set_peri_rate(void __user *uarg)
{
	unsigned int rate = 0;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&rate, uarg, sizeof(unsigned int)))
		return -EFAULT;

	return peribus_set_rate(rate);
}

void perf_actuator_common_init(void)
{
	register_perf_actuator(CMD_GET_DEV_CAP, perf_actuator_get_dev_cap);
	register_perf_actuator(CMD_SET_SCHED_PRIO, perf_actuator_set_sched_prio);
	if (xhee_status_enabled()) {
		register_perf_actuator(CMD_GET_REGULATE_RES, perf_actuator_sg_regulate);
		register_perf_actuator(CMD_GET_DISCOUNT_COEF, perf_actuator_get_discount_coef);
	}
	register_perf_actuator(CMD_SET_PERI_RATE, perf_actuator_set_peri_rate);
}
