// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 */
#include "soc/xring/xrse/pwrmng.h"
#include "../xrse_internal.h"
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <dt-bindings/xring/platform-specific/xrse_platform.h>
#include <linux/kstrtox.h>
#include <linux/errno.h>
#include <linux/arm-smccc.h>
#include <linux/mutex.h>

static DEFINE_MUTEX(g_vote_mutex);
static uint32_t g_vote_cnt;


static int xrse_vote_on(void)
{
	int ret = -EINVAL;
	struct arm_smccc_res res = {0};

	mutex_lock(&g_vote_mutex);

	if (g_vote_cnt == XRSE_VOTE_CNT_MAX) {
		xrse_err("vote on cnt overflow\n");
		ret = -EOVERFLOW;
		goto exit;
	}

	g_vote_cnt++;
	if (g_vote_cnt != XRSE_VOTE_POWERON_CNT) {
		ret = 0;
		goto exit;
	}

	arm_smccc_smc(FID_BL31_XRSE_VOTE, true, 0, 0, 0, 0, 0, 0, &res);
	ret = res.a0;
	if (ret < 0) {
		g_vote_cnt--;
		xrse_err("smc vote xrse failed: %d\n", ret);
		goto exit;
	}

exit:
	if (ret)
		xrse_err("xrse vote on failed. cnt = %d\n", g_vote_cnt);
	else
		xrse_info("xrse vote on success. cnt = %d\n", g_vote_cnt);

	mutex_unlock(&g_vote_mutex);

	return ret;
}

static int xrse_vote_off(void)
{
	int ret = -EINVAL;
	struct arm_smccc_res res = {0};

	mutex_lock(&g_vote_mutex);

	if (g_vote_cnt == XRSE_VOTE_CNT_MIN) {
		xrse_err("vote off cnt overflow\n");
		ret = -EOVERFLOW;
		goto exit;
	}

	g_vote_cnt--;
	if (g_vote_cnt != XRSE_VOTE_POWERDOWN_CNT) {
		ret = 0;
		goto exit;
	}

	arm_smccc_smc(FID_BL31_XRSE_VOTE, false, 0, 0, 0, 0, 0, 0, &res);
	ret = res.a0;
	if (ret < 0) {
		g_vote_cnt++;
		xrse_err("smc vote xrse failed: %d\n", ret);
		goto exit;
	}

exit:
	if (ret)
		xrse_err("xrse vote off failed. cnt = %d\n", g_vote_cnt);
	else
		xrse_info("xrse vote off success. cnt = %d\n", g_vote_cnt);

	mutex_unlock(&g_vote_mutex);

	return ret;
}

int xrse_vote(bool on)
{
	int ret = -EINVAL;

	if (on)
		ret = xrse_vote_on();
	else
		ret = xrse_vote_off();

	return ret;
}
EXPORT_SYMBOL(xrse_vote);
