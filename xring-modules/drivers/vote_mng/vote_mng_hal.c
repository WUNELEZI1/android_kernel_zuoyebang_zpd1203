// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/arm-smccc.h>
#include <soc/xring/vote_mng.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <soc/xring/xr_timestamp.h>
#include "vote_mng_internal.h"

#define CREATE_TRACE_POINTS
#include "vote_mng_trace.h"
#undef CREATE_TRACE_POINTS

#define VOTE_MNG_MODE1_MSG_SIZE	4

#define VOTE_MNG_MODE2_VOTE(data)	(BIT(16) | (0xFFFFU & (data)))

#define VOTE_MNG_MODE1_IDLE_TIMEOUT	30000
#define VOTE_MNG_MODE1_ACK_TIMEOUT	30000

#define VOTE_MNG_M3_ACK_TIMEOUT	3000

#define VOTE_MNG_MODE3_VOTE_ON_BM	0x10001
#define VOTE_MNG_MODE3_VOTE_OFF_BM	0x10000

#define VOTE_MNG_MODE3_IDLE		0

#define SMC_CALLER_MODE1		1
#define SMC_CALLER_MODE2		2
#define SMC_CALLER_MODE3		3

uint32_t vote_mng_mode1_ack_get(struct vote_mng_mode1_hw *hw, uint32_t ch)
{
	return (readl(hw->ack) & BIT(ch)) >> ch;
}

static void vote_mng_mode1_data_write(struct vote_mng_mode1_hw *hw,
				      uint32_t ch,
				      uint32_t *msg,
				      uint32_t size)
{
	uint32_t i;

	for (i = 0; i < size; i++)
		writel(msg[i], hw->msg + VOTE_MNG_MODE1_MSG_OFFSET(ch) + 4 * i);
}

static void vote_mng_mode1_data_read(struct vote_mng_mode1_hw *hw,
				     uint32_t ch,
				     uint32_t *msg,
				     uint32_t size)
{
	uint32_t i;

	for (i = 0; i < size; i++)
		msg[i] = readl(hw->msg + VOTE_MNG_MODE1_MSG_OFFSET(ch) + 4 * i);
}

int vote_mng_mode1_msg_send(void *hw_data,
			    uint32_t arb,
			    uint32_t *data,
			    uint32_t size,
			    uint32_t sync)
{
	uint32_t timeout;
	struct vote_mng_mode1_hw *hw = (struct vote_mng_mode1_hw *)hw_data;
	struct task_struct *task = current;

	if (!hw || size > VOTE_MNG_MODE1_MSG_SIZE)
		return -EINVAL;

	if (arb >= hw->arb_num)
		return -ENOMEM;

	timeout = VOTE_MNG_MODE1_IDLE_TIMEOUT;

	/* wait idle */
	while (timeout > 0) {
		timeout--;
		if (vote_mng_mode1_ack_get(hw, arb) != 0) {
			vote_mng_mode1_data_write(hw, arb, data, size);
			break;
		}
		usleep_range(10, 100);
	}
	if (timeout == 0)
		return -EBUSY;

	if (task != NULL && trace_vote_mng_mode1_enabled())
		trace_vote_mng_mode1(task->pid, task->comm, arb, 0, hw->msg, xr_timestamp_gettime());

	writel(BIT(arb), hw->en);

	if (sync == VOTE_MNG_MSG_ASYNC)
		return 0;

	/* wait ack */
	while (vote_mng_mode1_ack_get(hw, arb) == 0)
		usleep_range(10, 100);

	vote_mng_mode1_data_read(hw, arb, data, size);

	if (task != NULL && trace_vote_mng_mode1_enabled())
		trace_vote_mng_mode1(task->pid, task->comm, arb, 1, hw->msg, xr_timestamp_gettime());

	return 0;
}

int vote_mng_mode2_vote(void *hw_data,
			uint32_t arb,
			uint32_t ch,
			uint32_t data,
			uint32_t type)
{
	void __iomem *vote;
	struct task_struct *task = current;
	struct vote_mng_mode2_hw *hw = (struct vote_mng_mode2_hw *)hw_data;

	if (!hw)
		return -ENOENT;

	if (arb >= hw->arb_num || ch >= hw->ch_num)
		return -ENOMEM;

	switch (type) {
	case VOTE_MNG_MODE2_T_MAX:
		vote = hw->max;
		break;
	case VOTE_MNG_MODE2_T_MIN:
		vote = hw->min;
		break;
	case VOTE_MNG_MODE2_T_DMD:
		vote = hw->dmd;
		break;
	default:
		return -ENXIO;
	}

	writel(VOTE_MNG_MODE2_VOTE(data),
	       vote + VOTE_MNG_MODE2_HW_OFFSET(arb) + VOTE_MNG_MODE2_VOTE_OFFSET(ch));

	if (task != NULL && trace_vote_mng_mode2_enabled())
		trace_vote_mng_mode2(task->pid, task->comm, arb, ch, type, data, xr_timestamp_gettime());

	return 0;
}

static uint32_t vote_mng_mode3_ack_get(struct vote_mng_mode3_hw *hw,
				       uint32_t arb,
				       uint32_t ch)
{
	return (readl(hw->ack + VOTE_MNG_MODE3_HW_OFFSET(arb)) & BIT(ch)) >> ch;
}

int vote_mng_mode3_vote(void *hw_data, uint32_t arb, uint32_t ch, uint32_t flag)
{
	uint32_t timeout;
	struct arm_smccc_res res;
	struct task_struct *task = current;
	struct vote_mng_mode3_hw *hw = (struct vote_mng_mode3_hw *)hw_data;

	if (!hw)
		return -ENOENT;

	if (arb >= hw->arb_num || ch >= hw->ch_num)
		return -EINVAL;

	/* wait idle */
	timeout = VOTE_MNG_M3_ACK_TIMEOUT;
	while (timeout > 0) {
		timeout--;
		if (vote_mng_mode3_ack_get(hw, arb, ch))
			break;
		usleep_range(10, 100);
	}
	if (timeout == 0)
		return -EBUSY;

	if (task != NULL && trace_vote_mng_mode3_enabled())
		trace_vote_mng_mode3(task->pid, task->comm, arb, ch, flag, 0, xr_timestamp_gettime());

	/* write vote */
	if (!(flag & VOTE_MNG_ON) && !(flag & VOTE_MNG_OFF_SYNC)) {
		writel(VOTE_MNG_MODE3_VOTE_OFF_BM << ch,
		       hw->vote + VOTE_MNG_MODE3_HW_OFFSET(arb));
		return 0;
	}

	if (flag & VOTE_MNG_ON)
		writel(VOTE_MNG_MODE3_VOTE_ON_BM << ch,
		       hw->vote + VOTE_MNG_MODE3_HW_OFFSET(arb));
	else
		writel(VOTE_MNG_MODE3_VOTE_OFF_BM << ch,
		       hw->vote + VOTE_MNG_MODE3_HW_OFFSET(arb));

	/* wait ack */
	while (vote_mng_mode3_ack_get(hw, arb, ch) == 0)
		usleep_range(10, 100);

	if (task != NULL && trace_vote_mng_mode3_enabled())
		trace_vote_mng_mode3(task->pid, task->comm, arb, ch, flag, 1, xr_timestamp_gettime());

	if (!(flag & VOTE_MNG_ON))
		return 0;

	do {
		arm_smccc_smc(FID_BL31_VOTE_MNG_GET, hw->core, SMC_CALLER_MODE3,
			      arb, 0, 0, 0, 0, &res);
		usleep_range(10, 100);
	} while (!(res.a0 == VOTE_MNG_MODE3_IDLE && res.a1 == VOTE_MNG_ON));

	return 0;
}
