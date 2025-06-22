// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include "mdr_print.h"
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/export.h>

#include "mdr_module_core.h"
#include "mdr_syserr_core.h"
#include "mdr_field_core.h"
#include "mdr.h"

static u64 socsubsys_core_list[] = {
	MDR_AUDIO, //0x8
	MDR_XRSE,  //0x40000
};

static u64 wcn_core_list[] = {
	MDR_WCN, //0x400000
};

/*******************************************************************************
 * Description:
 *   If the exception core is soc core.
 *
 * Input:
 *   @e_info: struct mdr_exception_info_s
 *
 * Return:
 *   true : is soc core
 *   false: isn't soc core
 ********************************************************************************/
static bool socsubsys_core(struct mdr_exception_info_s *e_info)
{
	u32 i;

	for (i = 0; i < ARRAY_SIZE(socsubsys_core_list); i++) {
		if (socsubsys_core_list[i] == e_info->e_from_core)
			return true;
	}

	return false;
}

/*******************************************************************************
 * Description:
 *   If the exception core is modem core.
 *
 * Input:
 *   @e_info: struct mdr_exception_info_s
 *
 * Return:
 *   true : is modem core
 *   false: isn't modem core
 ********************************************************************************/
static bool modem_core(struct mdr_exception_info_s *e_info)
{
	if (e_info->e_from_core == MDR_CP)
		return true;

	return false;
}

/*******************************************************************************
 * Description:
 *   If the exception core is WCN core.
 *
 * Input:
 *   @e_info: struct mdr_exception_info_s
 *
 * Return:
 *   true : is wcn core
 *   false: isn't wcn core
 ********************************************************************************/
static bool wcn_core(struct mdr_exception_info_s *e_info)
{
	u32 i;

	for (i = 0; i < ARRAY_SIZE(wcn_core_list); i++) {
		if (wcn_core_list[i] == e_info->e_from_core)
			return true;
	}

	return false;
}

/*******************************************************************************
 * Description:
 *   If the subsys exception and the allsys reset switch is switch on,
 *   perform a allsys reset.
 *
 * Input:
 *   @e_info: struct mdr_exception_info_s
 *
 * Return:
 *   void
 ********************************************************************************/
void mdr_subsys_reset(struct mdr_exception_info_s *e_info)
{
	bool soc_rst_switch_on = is_switch_on(SWITCH_SOCSUBSYS_RESET);
	bool modem_rst_switch_on = is_switch_on(SWITCH_MODEM_RESET);
	bool wcn_rst_switch_on = is_switch_on(SWITCH_WCN_RESET);

	pr_info("e_from_core: 0x%llx\n", e_info->e_from_core);

	if ((soc_rst_switch_on && socsubsys_core(e_info))
		|| (modem_rst_switch_on && modem_core(e_info))
		|| (wcn_rst_switch_on && wcn_core(e_info))) {
		record_exce_type(e_info);
		panic(SUBSYS_RESET);
	}
}

/*******************************************************************************
 * Description:
 *   Check whether the subsys exception is sys reset.
 *
 * Input:
 *   @e_info: exception info
 *
 * Return:
 *   true : is sys reset
 *   false: not sys reset
 ********************************************************************************/
bool check_subsys_sysrst_enable(struct mdr_exception_info_s *e_info)
{
	bool ret = false;

	if ((is_switch_on(SWITCH_SOCSUBSYS_RESET) && socsubsys_core(e_info))
		|| (is_switch_on(SWITCH_MODEM_RESET) && modem_core(e_info))
		|| (is_switch_on(SWITCH_WCN_RESET) && wcn_core(e_info)))
		ret = true;

	pr_info("coreid[0x%llx] sys_reset_enable[%s]\n", e_info->e_from_core, ret ? "true" : "false");
	return ret;
}
