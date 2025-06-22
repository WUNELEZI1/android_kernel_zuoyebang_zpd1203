// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "xr-dvs-ctrl.h"
#include <clk/xr-clk-common.h>
#include <dt-bindings/xring/platform-specific/dvs.h>
#include <dt-bindings/xring/xr-clock-base.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/of_address.h>
#include <soc/xring/xr-clk-provider.h>

struct dvs_dump_info {
	const char *info_name;
	unsigned int offset;
	unsigned int data;
};

static struct dvs_dump_info g_dumpinfo[] = {
	{"CUR_STATE", DVS_CUR_STATE, 0},
	{"VOLT_REQ_0", DVS_VOLT_REQ_0, 0},
	{"VOLT_REQ_1", DVS_VOLT_REQ_1, 0},
	{"VOLT_REQ_2", DVS_VOLT_REQ_2, 0},
	{"VOLT_REQ_3", DVS_VOLT_REQ_3, 0},
	{"VOLT_REQ_4", DVS_VOLT_REQ_4, 0},
	{"VOLT_REQ_5", DVS_VOLT_REQ_5, 0},
	{"SW_STATE_ACK", DVS_SW_STATE_ACK, 0},
	{"ENABLE", DVS_ENABLE, 0},
	{"TIMEOUT_IRQ_INT_RAW", DVS_INTR_TIMEOUT_IRQ_INT_RAW, 0},
	{"TIMEOUT_IRQ_INT_STATUS", DVS_INTR_TIMEOUT_IRQ_INT_STATUS, 0},
	{"SW_INITIAL_0", DVS_SW_INITIAL_0, 0},
	{"SW_INITIAL_1", DVS_SW_INITIAL_1, 0},
	{"STATE_1", DVS_STATE_1, 0},
};

static void dvs_excep_info_update(void __iomem *base)
{
	int i;
	int info_num = ARRAY_SIZE(g_dumpinfo);

	for (i = 0; i < info_num; i++)
		g_dumpinfo[i].data = clkread(base + g_dumpinfo[i].offset);
}

static void dvs_excep_dump_print(void)
{
	int i;
	int info_num = ARRAY_SIZE(g_dumpinfo);

	for (i = 0; i < info_num; i++)
		clkerr("%s\t : \t0x%x\n", g_dumpinfo[i].info_name, g_dumpinfo[i].data);
}

static void dvs_excep_dump(void __iomem *base)
{
	dvs_excep_info_update(base);
	dvs_excep_dump_print();
}

static void clr_dvs_intr(void __iomem *base)
{
	clkwrite(0x1, base + DVS_INTR_TIMEOUT_IRQ_INT_RAW);
}

static int __dvs_check_intr(void __iomem *base)
{
	void __iomem *timeout_intr_addr = NULL;
	unsigned int data;

	timeout_intr_addr = base + DVS_INTR_TIMEOUT_IRQ_INT_STATUS;
	data = clkread(timeout_intr_addr);
	if (data) {
		clkerr("dvs timeout happened, 0x%p, data 0x%x!\n", timeout_intr_addr, data);
		return -EDVS_TMOT_IRQ;
	}

	return 0;
}

static int __dvs_wait_ack(void __iomem *base, uint8_t ch)
{
	void __iomem *ack_addr = NULL;
	unsigned int data, ack_bit;
	int timeout = DVS_VLD_TIMEOUT;

	ack_addr = base + ((ch >= DVS_UNSEC_CH_NUM) ? DVS_SW_STATE_ACK_1 : DVS_SW_STATE_ACK);
	ack_bit = (ch >= DVS_UNSEC_CH_NUM) ? (ch - DVS_UNSEC_CH_NUM) : ch;
	/*
	 * In order to prevent dvs ack from not having time to pull down
	 * after voting and read false ack=1
	 */
	udelay(1);

	do {
		data = clkread(ack_addr);
		if ((data & BIT(ack_bit)) != 0)
			return 0;
		udelay(1);
		timeout--;
	} while (timeout >= 0);

	clkerr("dvs wait ack fail, data 0x%x!\n", data);
	return -EDVS_TMOT;
}

static int dvs_prepare(void __iomem *base, uint8_t ch)
{
	int ret;

	ret = __dvs_wait_ack(base, ch);
	if (ret)
		goto excep_out;

	ret = __dvs_check_intr(base);
	if (ret)
		goto excep_out;

	return 0;
excep_out:
	dvs_excep_dump(base);
	return ret;
}

static int dvs_write(struct dvs_hw *hw, uint8_t data)
{
	void __iomem *dvs_base = NULL;
	void __iomem *req_addr = NULL;
	union dvs_sw_reg reg_data1 = { 0 };
	union dvs_sw_reg reg_data2 = { 0 };
	unsigned int valid_mask, req_mask;
	int ret;

	dvs_base = get_xr_clk_base(hw->base);
	if (!dvs_base)
		return -EINVAL;

	ret = dvs_prepare(dvs_base, hw->ch);
	if (ret != 0) {
		clkerr("dvs prepare fail, base %u, ch %u, data 0x%x, ret %d!\n",
			hw->base, hw->ch, data, ret);
		return ret;
	}
	req_addr = dvs_base + DVS_CH_OFFSET(hw->ch);

	req_mask = DVS_VOLT_REQ_0_SW_VOLT_REQ_0_MASK >>
		   DVS_VOLT_REQ_0_SW_VOLT_REQ_0_SHIFT;
	valid_mask = DVS_VOLT_REQ_0_SW_VOLT_REQ_VALID_0_MASK >>
		     DVS_VOLT_REQ_0_SW_VOLT_REQ_VALID_0_SHIFT;

	reg_data1.mem.volt_req = data;
	reg_data1.mem.req_valid = 1;
	reg_data1.mem.volt_req_mask = req_mask;
	reg_data1.mem.req_valid_mask = valid_mask;
	clkwrite(reg_data1.data, req_addr);

	ret = dvs_prepare(dvs_base, hw->ch);
	if (ret != 0) {
		clkerr("dvs prepare fail, base %u, ch %u, data 0x%x, ret %d!\n",
			hw->base, hw->ch, data, ret);
		return ret;
	}

	reg_data2.mem.req_valid_mask = valid_mask;
	reg_data2.mem.req_valid = 0;
	clkwrite(reg_data2.data, req_addr);

	return 0;
}

int dvs_write_no_wait_ack(struct dvs_hw *hw, uint8_t data)
{
	int ret;

	if (hw == NULL || hw->ch >= DVS_CH_NUM) {
		clkerr("hw is null or hw channel invalid!\n");
		return -EINVAL;
	}

	ret = dvs_write(hw, data);

	return ret;
}
EXPORT_SYMBOL_GPL(dvs_write_no_wait_ack);

int dvs_wait_ack(struct dvs_hw *hw)
{
	void __iomem *dvs_base = NULL;
	int ret;

	if (hw == NULL || hw->ch >= DVS_CH_NUM) {
		clkerr("hw is null or hw channel invalid!\n");
		return -EINVAL;
	}

	dvs_base = get_xr_clk_base(hw->base);
	if (!dvs_base)
		return -EINVAL;

	ret = dvs_prepare(dvs_base, hw->ch);
	if (ret != 0) {
		clkerr("dvs prepare fail, base %u, ch %u, ret %d!\n",
			hw->base, hw->ch, ret);
		return ret;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(dvs_wait_ack);

int dvs_get_cur_volt(struct dvs_hw *hw, uint8_t *data)
{
	void __iomem *dvs_base = NULL;
	union dvs_cur_state_reg state_data;

	if (hw == NULL || hw->ch >= DVS_CH_NUM) {
		clkerr("hw is null or hw channel invalid!\n");
		return -EINVAL;
	}

	dvs_base = get_xr_clk_base(hw->base);
	if (!dvs_base)
		return -EINVAL;

	state_data.data = clkread(dvs_base + DVS_CUR_STATE);
	*data = state_data.mem.volt_cur;

	return 0;
}
EXPORT_SYMBOL_GPL(dvs_get_cur_volt);

int dvs_write_wait_ack(struct dvs_hw *hw, uint8_t data)
{
	int ret;

	if (hw == NULL || hw->ch >= DVS_CH_NUM) {
		clkerr("hw is null or hw channel invalid!\n");
		return -EINVAL;
	}

	ret = dvs_write(hw, data);
	if (ret)
		return ret;

	ret = dvs_wait_ack(hw);

	return ret;
}
EXPORT_SYMBOL_GPL(dvs_write_wait_ack);

irqreturn_t dvs_timeout_interrupt_handle(int irq, void *dev)
{
	void __iomem *dvs_base = NULL;
	unsigned int base_id = (unsigned int)(uintptr_t)dev;

	clkinfo("dvs irq handle base %u!\n", base_id);
	dvs_base = get_xr_clk_base(base_id);
	if (!dvs_base)
		goto out;

	dvs_excep_dump(dvs_base);
	clr_dvs_intr(dvs_base);

	return IRQ_HANDLED;
out:
	return IRQ_NONE;
}
EXPORT_SYMBOL_GPL(dvs_timeout_interrupt_handle);
