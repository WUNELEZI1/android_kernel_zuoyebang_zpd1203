// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * 1-wire busmaster driver for Xring w1 busmaster
 *
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * Use consistent with the GNU GPL is permitted,
 * provided that this copyright notice is
 * preserved in its entirety in all copies and derived works.
 */

#include <linux/clk.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/pm.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/mfd/core.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/reset.h>

#include <asm/io.h>

#include "../xr_w1.h"
#include <dt-bindings/xring/platform-specific/one_wire.h>
#include <dt-bindings/xring/platform-specific/peri_crg.h>

#define XR_1WM_TIMEOUT (HZ * 5)
#define S_TO_NS 1000000000
#define MS_TO_NS 1000000

#define XR_FUNC_PARA_ERR_MASK					(1 << 0)
#define XR_FUNC_CRC_ERR_MASK					(1 << 1)
#define XR_FUNC_RX_COMMU_ERR_MASK				(1 << 2)
#define XR_FUNC_TX_COMMU_ERR_MASK				(1 << 3)
#define XR_FUNC_CMD_UNSUPPORT_ERR_MASK			(1 << 4)
#define XR_FUNC_ACCESS_LIMT_ERR_MASK			(1 << 5)
#define XR_FUNC_EEROM_ERR_MASK					(1 << 6)
#define XR_FUNC_ACCESS_CMD_ERR_MASK				(1 << 7)
#define XR_FUNC_ECDSA_FAIL_ERR_MASK				(1 << 8)
#define XR_FUNC_CHIP_UNVALID_ERR_MASK			(1 << 9)
#define XR_FUNC_RX_CRC_ERR_MASK					(1 << 10)
#define XR_FUNC_SUCCESS_MASK					(1 << 11)
#define XR_ROM_SEARCH_DISCREPANCY_MASK			(1 << 12)
#define XR_ROM_SLV_NO_PRESENT_ERR_MASK			(1 << 13)
#define XR_ROM_TIMEOUT_WARNING_ERR_MASK			(1 << 14)
#define XR_ROM_SEARCH_SLV_NO_PRESENT_ERR_MASK	(1 << 15)

#define XR_BYTE_4	4
#define XR_INT_8_BIT	8
#define XR_INT_16_BIT	16
#define XR_INT_24_BIT	24
#define XR_INT_32_BIT	32
#define XR_INT_40_BIT	40
#define XR_INT_48_BIT	48
#define XR_INT_56_BIT	56

#define XR_BYTE_2	2

#define XR_TIME_OUT_SLAVE_COUNT	100
#define XR_CMD_ERR_SLAVE_COUNT	200
#define XR_MAX_SLAVE_COUNT		2
#define XR_DISCREPANCY_MASK_L	0x3f
#define XR_DISCREPANCY_MASK_H	0x40

struct xr_1wm_data {
	void __iomem *map;
	struct clk *clk;
	unsigned long clkrate;
	struct platform_device *pdev;
	struct mutex access_lock;
	int irq;
	void *cmd_complete;
	struct reset_control	*rst;
	struct reset_control	*prst;
};

static inline void xr_1wm_write_register(struct xr_1wm_data *xr_1wm_data, u32 addr,
					const u8 *val, u8 bc)
{
	u32 data = 0;

	if (bc > XR_BYTE_4) {
		dev_err(&xr_1wm_data->pdev->dev,
			"xr_1wm_write_register bc %d > 4  too long byte count\n", bc);
		return;
	}

	memcpy(&data, val, bc);

	dev_dbg(&xr_1wm_data->pdev->dev,
		" %s addr: 0x%x, 32 bit data:0x%x\n", __func__, addr, data);

	__raw_writel(data, xr_1wm_data->map + addr);
}

static inline void xr_1wm_read_register(struct xr_1wm_data *xr_1wm_data, u32 addr, u8 *val, u8 bc)
{
	u32 res = 0;

	if (bc > XR_BYTE_4)
		return;

	res = __raw_readl(xr_1wm_data->map + addr);

	dev_dbg(&xr_1wm_data->pdev->dev,
		" %s addr: 0x%x, 32 bit res:0x%x\n", __func__, addr, res);

	memcpy(val, &res, bc);
}

static void xr_1wm_write(struct xr_1wm_data *xr_1wm_data, u32 addr, u8 val)
{
	mutex_lock(&xr_1wm_data->access_lock);

	xr_1wm_write_register(xr_1wm_data, addr, &val, 1);

	mutex_unlock(&xr_1wm_data->access_lock);
}

static void xr_1wm_write_int(struct xr_1wm_data *xr_1wm_data, u32 addr, int val)
{
	u8 buf[XR_BYTE_4];

	memset(buf, 0, sizeof(buf));

	buf[0] = (val & 0xFF);
	buf[1] = ((val >> XR_INT_8_BIT) & 0xFF);
	buf[2] = ((val >> XR_INT_16_BIT) & 0xFF);
	buf[3] = ((val >> XR_INT_24_BIT) & 0xFF);

	mutex_lock(&xr_1wm_data->access_lock);

	xr_1wm_write_register(xr_1wm_data, addr, buf, sizeof(buf));

	mutex_unlock(&xr_1wm_data->access_lock);
}

static void xr_1wm_set_resp_delay(void *data, int val)
{
	struct xr_1wm_data *xr_1wm_data = data;
	int reg_val;

	reg_val = (val * MS_TO_NS) / (S_TO_NS / xr_1wm_data->clkrate);

	dev_dbg(&xr_1wm_data->pdev->dev, " %s, val:%d ms, reg_val:%d clkrate:%lu Hz\n",
		__func__, val, reg_val, xr_1wm_data->clkrate);

	xr_1wm_write_int(xr_1wm_data, ONE_WIRE_M_RESP_DELAY, reg_val);
}

static u8 xr_1wm_read(struct xr_1wm_data *xr_1wm_data, u32 addr)
{
	u8 res;

	mutex_lock(&xr_1wm_data->access_lock);

	xr_1wm_read_register(xr_1wm_data, addr, &res, 1);

	mutex_unlock(&xr_1wm_data->access_lock);

	return res;
}

static irqreturn_t xr_1wm_isr(int isr, void *data)
{
	struct xr_1wm_data *xr_1wm_data = data;
	u8 val = 1;

	if (!xr_1wm_data) {
		pr_err(" %s xr_1wm_data is null\n", __func__);
		return IRQ_NONE;
	}

	if (!xr_1wm_data->cmd_complete) {
		dev_err(&xr_1wm_data->pdev->dev,
			" %s cmd_complete is null\n", __func__);
		return IRQ_NONE;
	}

	complete(xr_1wm_data->cmd_complete);

	xr_1wm_write_register(xr_1wm_data, ONE_WIRE_INT_1_WIRE_IRQ_INT_RAW, &val, 1);

	return IRQ_HANDLED;
}
/* --------------------------------------------------------------------- */
/* w1 methods */
static u8 xr_1wm_read_byte_null(void *data)
{
	return 0;
}

static void xr_1wm_write_byte_null(void *data, u8 val)
{

}

static u8 xr_1wm_reset_bus(void *data)
{
	return 0;
}

static int xr_1wm_cmd_start_en(void *data)
{
	int ret = -1;
	u8 buf[XR_BYTE_2];
	struct xr_1wm_data *xr_1wm_data = data;
	unsigned long timeleft;

	DECLARE_COMPLETION_ONSTACK(cmd_done);

	xr_1wm_data->cmd_complete = &cmd_done;

	xr_1wm_write(xr_1wm_data, ONE_WIRE_M_CMD_START, 0);
	xr_1wm_write(xr_1wm_data, ONE_WIRE_M_CMD_START, 1);

	timeleft = wait_for_completion_timeout(&cmd_done, XR_1WM_TIMEOUT);
	xr_1wm_data->cmd_complete = NULL;
	if (!timeleft) {
		dev_err(&xr_1wm_data->pdev->dev, "cmd start en failed, timed out\n");
		return -ETIMEDOUT;
	}

	memset(buf, 0, sizeof(buf));

	mutex_lock(&xr_1wm_data->access_lock);

	xr_1wm_read_register(xr_1wm_data, ONE_WIRE_M_RPT_WARNING, buf, sizeof(buf));

	mutex_unlock(&xr_1wm_data->access_lock);

	ret = (buf[1] << XR_INT_8_BIT) | buf[0];

	return ret;
}

static void xr_1wm_skip(void *data, u8 val)
{
	struct xr_1wm_data *xr_1wm_data = data;

	xr_1wm_write(xr_1wm_data, ONE_WIRE_M_ROM_CMD, val);

}

static void xr_1wm_read_romid(void *data, u64 *romid)
{
	u8 buf[XR_BYTE_4];
	struct xr_1wm_data *xr_1wm_data = data;
	u64 tmp = 0;
	u8 crc;

	memset(buf, 0, sizeof(buf));

	xr_1wm_read_register(xr_1wm_data, ONE_WIRE_M_RX_ROMID0, buf, sizeof(buf));
	tmp |= ((u64)buf[0] << XR_INT_48_BIT) | ((u64)buf[1] << XR_INT_40_BIT) |
				((u64)buf[2] << XR_INT_32_BIT) | ((u64)buf[3] << XR_INT_24_BIT);

	memset(buf, 0, sizeof(buf));

	xr_1wm_read_register(xr_1wm_data, ONE_WIRE_M_RX_ROMID1, buf, sizeof(buf));
	tmp |= ((u64)buf[0] << XR_INT_16_BIT) | ((u64)buf[1] << XR_INT_8_BIT) |
				((u64)buf[2]);

	crc = w1_calc_crc8((u8 *)&tmp, 7);

	tmp |= ((u64)crc << XR_INT_56_BIT);

	*romid = tmp;
	return;

}

static int xr_1wm_read_rom(void *data, u8 val)
{
	struct xr_1wm_data *xr_1wm_data = data;

	xr_1wm_write(xr_1wm_data, ONE_WIRE_M_ROM_CMD, val);

	return 0;
}

static void xr_1wm_search(void *data, struct w1_master *master_dev,
			u8 search_type, w1_slave_found_callback slave_found)
{
	struct xr_1wm_data *xr_1wm_data = data;
	u8 tmp;
	u64 romid;
	int res = 0;
	static unsigned int slaves_found;
	static unsigned int search_count;
	static int second_search_flag;

	second_search_flag = 0;

	while (true) {
		if (slaves_found >= XR_MAX_SLAVE_COUNT || search_count >= XR_CMD_ERR_SLAVE_COUNT) {
			dev_dbg(&xr_1wm_data->pdev->dev,
				"slaves_found:%d, search_count:%d, search aborted\n",
				slaves_found, search_count);
			goto out;
		}
		xr_1wm_write(xr_1wm_data, ONE_WIRE_M_ROM_CMD, search_type);

		res = xr_1wm_cmd_start_en(xr_1wm_data);
		if (res < 0) {
			search_count++;
			dev_err(&xr_1wm_data->pdev->dev,
				"slaves_found:%d, search_count:%d, irq timeout res:%d\n",
				slaves_found, search_count, res);
			goto out;
		} else if ((res & XR_ROM_SEARCH_SLV_NO_PRESENT_ERR_MASK) ||
						(res & XR_ROM_TIMEOUT_WARNING_ERR_MASK) ||
						(res & XR_ROM_SLV_NO_PRESENT_ERR_MASK)) {
			search_count++;
			dev_err(&xr_1wm_data->pdev->dev,
				"slaves_found:%d, search_count:%d, err status:0x%x\n",
				slaves_found, search_count, res);
			goto out;
		} else if ((slaves_found == 0) || (second_search_flag == 1)) {/* search success */

			xr_1wm_read_romid(xr_1wm_data, &romid);
			slaves_found += 1;

			if (second_search_flag == 1)
				second_search_flag = 0;
			/* search discrepancy */
			if ((res & XR_ROM_SEARCH_DISCREPANCY_MASK) && (slaves_found == 1) &&
				(second_search_flag == 0)) {
				second_search_flag = 1;
				/* read discrepancy */
				tmp = xr_1wm_read(xr_1wm_data, ONE_WIRE_M_SEARCH_DISCREPANCY_LOC);
				/* user bit5:0 */
				tmp &= XR_DISCREPANCY_MASK_L;
				/* bit6=1 */
				tmp |= XR_DISCREPANCY_MASK_H;
				/* write discrepancy cfg bit6=1  bit5:0 */
				xr_1wm_write(xr_1wm_data, ONE_WIRE_M_SEARCH_DISCREPANCY_LOC_CFG,
					tmp);
			}

			dev_dbg(&xr_1wm_data->pdev->dev,
				"slaves_found: %d found %0#18llx\n", slaves_found, romid);

			slave_found(master_dev, romid);
		} else {
			search_count = XR_CMD_ERR_SLAVE_COUNT;
			dev_err(&xr_1wm_data->pdev->dev,
				"slaves_found:%d, search_count:%d, only found one\n",
				slaves_found, search_count);
		}
	} /* end while true */
out:
	dev_dbg(&xr_1wm_data->pdev->dev,
		"slaves_found: %d\n", slaves_found);
}

#define XR_ROM_CMD_LOC	0
#define XR_ROMID0_LOC	1
#define XR_ROMID1_LOC	5

static void xr_1wm_match(void *data, const u8 *buf, int len)
{
	struct xr_1wm_data *xr_1wm_data = data;

	mutex_lock(&xr_1wm_data->access_lock);

	xr_1wm_write_register(xr_1wm_data, ONE_WIRE_M_TX_ROMID0, &buf[XR_ROMID0_LOC], XR_BYTE_4);
	xr_1wm_write_register(xr_1wm_data, ONE_WIRE_M_TX_ROMID1, &buf[XR_ROMID1_LOC], XR_BYTE_4);

	mutex_unlock(&xr_1wm_data->access_lock);

	xr_1wm_write(xr_1wm_data, ONE_WIRE_M_ROM_CMD, buf[XR_ROM_CMD_LOC]);
}

#define XR_MIN_DATA_LEN		1
#define XR_MAX_DATA_LEN		64

static void xr_1wm_revert_endding(u8 *buf, int len)
{
	int i;
	u8 tmp;

	for (i = 0; i < len / 2; i++) {
		tmp = buf[i];
		buf[i] = buf[len - i - 1];
		buf[len - i - 1] = tmp;
	}
}

static void xr_1wm_read_block_reg(void *data, u8 *buf, int len)
{
	struct xr_1wm_data *xr_1wm_data = data;
	int cnt;
	int start = 0;

	if (len < XR_MIN_DATA_LEN || len > XR_MAX_DATA_LEN)
		return;

	cnt = len;

	mutex_lock(&xr_1wm_data->access_lock);

	while (cnt >= XR_MIN_DATA_LEN) {
		if (cnt / XR_BYTE_4) {
			xr_1wm_read_register(xr_1wm_data, ONE_WIRE_M_MEMORY_RDATA0 + start,
				&buf[start], XR_BYTE_4);
			cnt -= XR_BYTE_4;
			start += XR_BYTE_4;
		} else {
			if (cnt) {
				xr_1wm_read_register(xr_1wm_data, ONE_WIRE_M_MEMORY_RDATA0 + start,
					&buf[start], cnt);
				cnt = 0;
				start = 0;
			}
		}
	}

	if (start == len)
		xr_1wm_revert_endding(buf, len);

	mutex_unlock(&xr_1wm_data->access_lock);
}

#define XR_FUNC_CMD_LOC			0
#define	XR_FUNC_MSG_LOC			1
#define	XR_FUNC_PARA_LOC		2
#define	XR_FUNC_MIN_LEN			2
#define	XR_FUNC_PARA_MIN_LEN	3
#define	XR_FUNC_PARA_MAX_LEN	36

#define XR_WRITE_MIN_LEN	2

static int xr_1wm_write_block_reg(void *data, const u8 *buf, int len)
{

	struct xr_1wm_data *xr_1wm_data = data;
	int cnt;
	int start = 0;
	int ret = 0;
	u8 *send_buf = NULL;

	if (len < XR_FUNC_MIN_LEN)
		return -1;

	mutex_lock(&xr_1wm_data->access_lock);

	xr_1wm_write_register(xr_1wm_data, ONE_WIRE_M_FUN_CMD, &buf[XR_FUNC_CMD_LOC], 1);
	xr_1wm_write_register(xr_1wm_data, ONE_WIRE_M_CMD_MSG, &buf[XR_FUNC_MSG_LOC], 1);

	if (len > XR_WRITE_MIN_LEN && buf[1] > 0) {
		xr_1wm_write_register(xr_1wm_data, ONE_WIRE_M_FUN_PARA, &buf[XR_FUNC_PARA_LOC], 1);
		if (len > XR_FUNC_PARA_MIN_LEN && len < XR_FUNC_PARA_MAX_LEN &&
			buf[XR_FUNC_MSG_LOC] > 1) {
			cnt = len - XR_FUNC_PARA_MIN_LEN;

			send_buf = kmalloc_array(cnt, sizeof(char), GFP_KERNEL);
			if (send_buf == NULL)
				goto fail;
			memcpy(send_buf, &buf[start + XR_FUNC_PARA_MIN_LEN], cnt);

			while (cnt > 0) {
				if (cnt / XR_BYTE_4) {
					xr_1wm_write_register(xr_1wm_data,
						ONE_WIRE_M_MEMORY_WDATA0 + start,
						&send_buf[start], XR_BYTE_4);
					cnt -= XR_BYTE_4;
					start += XR_BYTE_4;
				} else {
					if (cnt) {
						xr_1wm_write_register(xr_1wm_data,
							ONE_WIRE_M_MEMORY_WDATA0 + start,
							&send_buf[start], cnt);
						cnt = 0;
						start = 0;
					}
				}
			}
		}
	}

	kfree(send_buf);

fail:
	mutex_unlock(&xr_1wm_data->access_lock);

	return ret;
}

static struct w1_bus_master xr_1wm_master = {
	.read_byte  = xr_1wm_read_byte_null,
	.write_byte = xr_1wm_write_byte_null,
	.reset_bus  = xr_1wm_reset_bus,
	.read_block_reg		= xr_1wm_read_block_reg,
	.write_block_reg	= xr_1wm_write_block_reg,
	.search	    = xr_1wm_search,
	.skip	    = xr_1wm_skip,
	.match	    = xr_1wm_match,
	.read_rom	= xr_1wm_read_rom,
	.cmd_start	= xr_1wm_cmd_start_en,
	.set_resp_delay = xr_1wm_set_resp_delay,
};

static int xr_1wm_crg_config(struct platform_device *pdev)
{
	int rc;
	u64 val;
	void __iomem *crg_addr;

	rc = device_property_read_u64(&pdev->dev, "fpga-crg-base", &val);
	if (rc) {
		dev_err(&pdev->dev, "%s failed to get fpga-crg-base, ret = %d\n",
			__func__, rc);
		return rc;
	}

	dev_dbg(&pdev->dev, "%s get fpga-crg-base = %llx\n", __func__, val);
	crg_addr = devm_ioremap(&pdev->dev, val, 4096);

	writel((BIT(PERI_CRG_CLKGT5_W1S_GT_PCLK_ONEWIRE2_SHIFT) |
		BIT(PERI_CRG_CLKGT5_W1S_GT_PCLK_ONEWIRE1_SHIFT) |
		BIT(PERI_CRG_CLKGT5_W1S_GT_PCLK_ONEWIRE0_SHIFT)), crg_addr + PERI_CRG_CLKGT5_W1S);

	writel((BIT(PERI_CRG_CLKGT8_W1S_GT_CLK_ONEWIRE2_SHIFT) |
		BIT(PERI_CRG_CLKGT8_W1S_GT_CLK_ONEWIRE1_SHIFT) |
		BIT(PERI_CRG_CLKGT8_W1S_GT_CLK_ONEWIRE0_SHIFT)), crg_addr + PERI_CRG_CLKGT8_W1S);

	return rc;
}

static int xr_1wm_parse_dt(void *data)
{
	int rc;
	int val;
	struct xr_1wm_data *xr_1wm_data = data;
	struct device_node *node = xr_1wm_data->pdev->dev.of_node;

	if (!node) {
		dev_err(&xr_1wm_data->pdev->dev, "device tree info missing\n");
		return -EINVAL;
	}

	/* write 1 low time */
	rc = of_property_read_u32(node, "xr,tw1l_time", &val);
	if (!rc)
		xr_1wm_write_int(xr_1wm_data, ONE_WIRE_M_TW1L_TIME, val);

	/* write 0 low time */
	rc = of_property_read_u32(node, "xr,tw0l_time", &val);
	if (!rc)
		xr_1wm_write_int(xr_1wm_data, ONE_WIRE_M_TW0L_TIME, val);

	/* cmd timeout config */
	rc = of_property_read_u32(node, "xr,timeout_cfg", &val);
	if (!rc)
		xr_1wm_write_int(xr_1wm_data, ONE_WIRE_M_TIMEOUT_CFG, val);

	/* write data bit width time */
	rc = of_property_read_u32(node, "xr,twslot_time", &val);
	if (!rc)
		xr_1wm_write_int(xr_1wm_data, ONE_WIRE_M_TWSLOT_TIME, val);

	/* read data output low time */
	rc = of_property_read_u32(node, "xr,trl_time", &val);
	if (!rc)
		xr_1wm_write_int(xr_1wm_data, ONE_WIRE_M_TRL_TIME, val);

	/* read data sampling time */
	rc = of_property_read_u32(node, "xr,trsamp_time", &val);
	if (!rc)
		xr_1wm_write_int(xr_1wm_data, ONE_WIRE_M_TRSAMP_TIME, val);

	/* read data bit width time */
	rc = of_property_read_u32(node, "xr,trslot_time", &val);
	if (!rc)
		xr_1wm_write_int(xr_1wm_data, ONE_WIRE_M_TRSLOT_TIME, val);

	/* response delay time */
	rc = of_property_read_u32(node, "xr,resp_delay", &val);
	if (!rc)
		xr_1wm_write_int(xr_1wm_data, ONE_WIRE_M_RESP_DELAY, val);

	/* reset output low level time */
	rc = of_property_read_u32(node, "xr,trsth_time", &val);
	if (!rc)
		xr_1wm_write_int(xr_1wm_data, ONE_WIRE_M_TRSTH_TIME, val);

	/* reset recive low level time */
	rc = of_property_read_u32(node, "xr,trstl_time", &val);
	if (!rc)
		xr_1wm_write_int(xr_1wm_data, ONE_WIRE_M_TRSTL_TIME, val);

	/* reset hold low level time */
	rc = of_property_read_u32(node, "xr,trstrec_time", &val);
	if (!rc)
		xr_1wm_write_int(xr_1wm_data, ONE_WIRE_M_TRSTREC_TIME, val);

	return 0;
}

static void xr_1wm_unmask_int(void *data)
{
	struct xr_1wm_data *xr_1wm_data = data;

	xr_1wm_write_int(xr_1wm_data, ONE_WIRE_INT_1_WIRE_IRQ_INT_MASK, 0);
}

static int xr_1wm_pm_resume(struct device *dev)
{
	int ret = 0;

	struct xr_1wm_data *xr_1wm_data = dev_get_drvdata(dev);

	if (xr_1wm_data == NULL || xr_1wm_data->pdev == NULL || xr_1wm_data->clk == NULL)
		return -ENOMEM;

	ret = xr_1wm_crg_config(xr_1wm_data->pdev);
	if (ret)
		goto err;

	ret = reset_control_deassert(xr_1wm_data->prst);
	if (ret < 0) {
		dev_err(&xr_1wm_data->pdev->dev, "Couldn't deassert prst control: %d in Resume\n", ret);
		return ret;
	}

	ret = reset_control_deassert(xr_1wm_data->rst);
	if (ret < 0) {
		dev_err(&xr_1wm_data->pdev->dev, "Couldn't deassert rst control: %d in Resume\n", ret);
		return ret;
	}

	ret = clk_prepare_enable(xr_1wm_data->clk);
	if (ret)
		goto out_clk;

	xr_1wm_data->clkrate = clk_get_rate(xr_1wm_data->clk);

	xr_1wm_parse_dt(xr_1wm_data);

err:
	return ret;

out_clk:
	clk_disable_unprepare(xr_1wm_data->clk);
	return ret;
}

static int xr_1wm_pm_suspend(struct device *dev)
{
	struct xr_1wm_data *xr_1wm_data = dev_get_drvdata(dev);

	if (xr_1wm_data == NULL || xr_1wm_data->clk == NULL)
		return -ENOMEM;

	clk_disable_unprepare(xr_1wm_data->clk);

	return 0;
}

static const struct dev_pm_ops xr_1wm_pm_ops = {
	.resume = xr_1wm_pm_resume,
	.suspend = xr_1wm_pm_suspend,
};

static int xr_1wm_probe(struct platform_device *pdev)
{
	struct xr_1wm_data *xr_1wm_data;
	struct resource *res;
	int ret;

	if (!pdev)
		return -ENODEV;

	xr_1wm_data = devm_kzalloc(&pdev->dev, sizeof(*xr_1wm_data), GFP_KERNEL);
	if (!xr_1wm_data)
		return -ENOMEM;

	ret = xr_1wm_crg_config(pdev);
	if (ret)
		goto err;

	xr_1wm_data->prst = devm_reset_control_get_exclusive(&pdev->dev, "prst");
	if (IS_ERR(xr_1wm_data->prst)) {
		dev_err(&pdev->dev, "Couldn't get prst control\n");
		return PTR_ERR(xr_1wm_data->prst);
	}

	xr_1wm_data->rst = devm_reset_control_get_exclusive(&pdev->dev, "rst");
	if (IS_ERR(xr_1wm_data->rst))  {
		dev_err(&pdev->dev, "Couldn't get rst control\n");
		return PTR_ERR(xr_1wm_data->rst);
	}

	ret = reset_control_deassert(xr_1wm_data->prst);
	if (ret < 0) {
		dev_err(&pdev->dev, "Couldn't deassert prst control: %d in Probe\n", ret);
		goto err;
	}

	ret = reset_control_deassert(xr_1wm_data->rst);
	if (ret < 0) {
		dev_err(&pdev->dev, "Couldn't deassert rst control: %d in Probe\n", ret);
		goto err;
	}

	platform_set_drvdata(pdev, xr_1wm_data);

	xr_1wm_data->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(xr_1wm_data->clk))
		return PTR_ERR(xr_1wm_data->clk);

	ret = clk_prepare_enable(xr_1wm_data->clk);
	if (ret)
		goto out_clk;

	xr_1wm_data->clkrate = clk_get_rate(xr_1wm_data->clk);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		ret = -ENXIO;
		goto out_clk;
	}

	xr_1wm_data->map = devm_ioremap(&pdev->dev, res->start,
				       resource_size(res));
	if (!xr_1wm_data->map) {
		ret = -ENOMEM;
		goto out_clk;
	}

	xr_1wm_data->pdev = pdev;

	mutex_init(&xr_1wm_data->access_lock);

	xr_1wm_parse_dt(xr_1wm_data);

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		ret = -ENXIO;
		goto out_clk;
	}

	xr_1wm_data->irq = res->start;

	if (res->flags & IORESOURCE_IRQ_HIGHEDGE)
		irq_set_irq_type(xr_1wm_data->irq, IRQ_TYPE_EDGE_RISING);
	if (res->flags & IORESOURCE_IRQ_LOWEDGE)
		irq_set_irq_type(xr_1wm_data->irq, IRQ_TYPE_EDGE_FALLING);
	if (res->flags & IORESOURCE_IRQ_HIGHLEVEL)
		irq_set_irq_type(xr_1wm_data->irq, IRQ_TYPE_LEVEL_HIGH);
	if (res->flags & IORESOURCE_IRQ_LOWLEVEL)
		irq_set_irq_type(xr_1wm_data->irq, IRQ_TYPE_LEVEL_LOW);

	ret = devm_request_irq(&pdev->dev, xr_1wm_data->irq, xr_1wm_isr,
			IRQF_SHARED, "xr1wm", xr_1wm_data);
	if (ret) {
		dev_err(&xr_1wm_data->pdev->dev,
			"devm_request_irq %d failed with errno %d\n",
			xr_1wm_data->irq, ret);
		goto out_clk;
	}

	xr_1wm_unmask_int(xr_1wm_data);

	xr_1wm_master.data = (void *)xr_1wm_data;

	ret = w1_add_master_device(&xr_1wm_master);
	if (ret) {
		dev_err(&xr_1wm_data->pdev->dev, "add master fail, ret:%d\n", ret);
		goto out_clk;
	}

	dev_dbg(&xr_1wm_data->pdev->dev, "debug xr_1wm: probe successful\n");
	return 0;

out_clk:
	clk_disable_unprepare(xr_1wm_data->clk);
err:
	reset_control_assert(xr_1wm_data->rst);
	reset_control_assert(xr_1wm_data->prst);
	return ret;
}

static int xr_1wm_remove(struct platform_device *pdev)
{
	struct xr_1wm_data *xr_1wm_data = dev_get_drvdata(&pdev->dev);

	if (xr_1wm_data == NULL || xr_1wm_data->pdev == NULL || xr_1wm_data->clk == NULL)
		return -ENOMEM;

	w1_remove_master_device(&xr_1wm_master);

	reset_control_assert(xr_1wm_data->rst);
	reset_control_assert(xr_1wm_data->prst);

	return 0;
}

static const struct of_device_id xr_w1_match_table[] = {
	{ .compatible = "xr,one-wire", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, xr_w1_match_table);

static struct platform_driver xr_1wm_driver = {
	.driver   = {
		.name = "xr1wm",
		.of_match_table = xr_w1_match_table,
		.pm = &xr_1wm_pm_ops,
	},
	.probe    = xr_1wm_probe,
	.remove   = xr_1wm_remove,
};

static int __init xr_1wm_init(void)
{
	pr_info("Xring w1 busmaster driver init\n");
	return platform_driver_register(&xr_1wm_driver);
}

static void __exit xr_1wm_exit(void)
{
	platform_driver_unregister(&xr_1wm_driver);
}

module_init(xr_1wm_init);
module_exit(xr_1wm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("XRing Technologies Co., Ltd");
MODULE_DESCRIPTION("Xring w1 busmaster driver");
MODULE_SOFTDEP("pre: xr_wire");
