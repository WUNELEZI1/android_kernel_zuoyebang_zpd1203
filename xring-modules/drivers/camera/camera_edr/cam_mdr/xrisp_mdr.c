// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "xrisp_common.h"
#include "xrisp_mdr.h"



void mdr_isp_dump(u32 modid, u32 etype, u64 coreid, char *log_path,
			pfn_cb_dump_done pfn_cb, void *data)
{
	char path[256] = {0};

	if (log_path == NULL)
		return;

	strscpy(path, log_path, sizeof(path) - 1);
	int len = strlen(path);

	if (path[len - 1] == '/')
		path[len - 1] = '\0';

	char *last_slash = strrchr(path, '/');

	if (last_slash != NULL) {
		strscpy(new_event.logpath, last_slash + 1, sizeof(new_event.logpath) - 1);
		new_event.logpath[sizeof(new_event.logpath) - 1] = '\0';
	} else {
		strscpy(new_event.logpath, path, sizeof(new_event.logpath) - 1);
		new_event.logpath[sizeof(new_event.logpath) - 1] = '\0';
	}

	edr_info("log_path [%s] path[%s]", log_path, new_event.logpath);

	wake_up_interruptible(&wq);
}

void mdr_isp_reset(u32 modid, u32 etype, u64 coreid, void *data)
{
	edr_info("mdr_isp_reset");
}

struct mdr_module_ops isp_ops = {
	.ops_dump = mdr_isp_dump,
	.ops_reset = mdr_isp_reset,
};

int xrisp_mdr_exception_register(void)
{
	int ret = 0;
	int i = 0;
	struct mdr_register_module_result info;

	for (i = 0; i < sizeof(g_isp_exception)/sizeof(struct mdr_exception_info_s); i++) {
		/* this function is implemented by mdr and return 0 when error */
		ret = mdr_register_exception(&g_isp_exception[i]);
		if (ret == 0) {
			edr_err("ISP exception register fail");
			return -EINVAL;
		}
	}

	/* this function is implemented by mdr and return negative value when error */
	ret = mdr_register_module_ops(MDR_ISP, &isp_ops, &info);
	if (ret < 0) {
		edr_err("ISP module register fail");
		return ret;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(xrisp_mdr_exception_register);

int xrisp_mdr_exception_unregister(void)
{
	int ret = 0;
	int i = 0;

	for (i = 0; i < sizeof(g_isp_exception)/sizeof(struct mdr_exception_info_s); i++) {
		ret = mdr_unregister_exception(g_isp_exception[i].e_modid);
		if (ret) {
			edr_err("ISP exception unregister fail");
			return -EINVAL;
		}
	}

	ret = mdr_unregister_module_ops(MDR_ISP);
	if (ret) {
		edr_err("ISP module unregister fail");
		return ret;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(xrisp_mdr_exception_unregister);

int xrisp_exception_to_mdr(uint16_t type, uint16_t subtype)
{
	uint32_t ret = MODID_ISP_EEPROM_CAM_PROBE_ERROR + 1;

	switch (type) {
	case 0:
		ret = MODID_ISP_FW_PANIC;
		break;
	case 1:
		ret = MODID_ISP_FW_WDT_HANG;
		break;
	case 2:
		ret = MODID_ISP_FW_TIMER_HANG;
		break;
	case 3:
		if (subtype == 51)
			ret = MODID_ISP_MIPI_TIMEOUT_ERROR;
		else if (subtype == 52)
			ret = MODID_ISP_MIPI_RX_ERROR;
		else if (subtype == 53)
			ret = MODID_ISP_MIPI_HSRX_TO_ERROR;
		else if (subtype == 54)
			ret = MODID_ISP_MIPI_DESKEW_ERROR;
		else if (subtype == 55)
			ret = MODID_ISP_MIPI_PHY01_ERROR;
		else if (subtype == 56)
			ret = MODID_ISP_MIPI_PHY02_ERROR;
		else if (subtype == 57)
			ret = MODID_ISP_MIPI_CSI2_ERROR;
		else if (subtype == 58)
			ret = MODID_ISP_MIPI_FRAME_ERROR;
		else if (subtype == 59)
			ret = MODID_ISP_MIPI_LINE_ERROR;
		else if (subtype == 60)
			ret = MODID_ISP_MIPI_SDI_HD_FIFO_ERROR;
		else if (subtype == 61)
			ret = MODID_ISP_MIPI_SDI_PLD_FIFO_ERROR;
		else if (subtype == 62)
			ret = MODID_ISP_MIPI_SDI_WC_ERROR;
		else if (subtype == 63)
			ret = MODID_ISP_MIPI_BUS_ERROR;
		else if (subtype == 64)
			ret = MODID_ISP_MIPI_AXI_ERROR;
		else if (subtype == 65)
			ret = MODID_ISP_MIPI_WR_ERROR;
		else if (subtype == 66)
			ret = MODID_ISP_MIPI_SDI_BUF_ERROR;
		break;
	case 4:
		if (subtype == 67)
			ret = MODID_ISP_FE_ROUTER_SDI0_VBUS_ERROR;
		else if (subtype == 68)
			ret = MODID_ISP_FE_ROUTER_SDI1_VBUS_ERROR;
		else if (subtype == 69)
			ret = MODID_ISP_FE_ROUTER_SDI2_VBUS_ERROR;
		else if (subtype == 70)
			ret = MODID_ISP_FE_ROUTER_SDI0_BUF_FULL;
		else if (subtype == 71)
			ret = MODID_ISP_FE_ROUTER_SDI1_BUF_FULL;
		else if (subtype == 72)
			ret = MODID_ISP_FE_ROUTER_SDI2_BUF_FULL;
		else if (subtype == 74)
			ret = MODID_ISP_FE_ROUTER_DATA_DMA_READ;
		else if (subtype == 75)
			ret = MODID_ISP_FE_ROUTER_DATA_DMA_WRITE;
		break;
	case 5:
		if (subtype == 76)
			ret = MODID_ISP_FE0_CMD_DMA_ERROR;
		else if (subtype == 75)
			ret = MODID_ISP_FE0_DATA_DMA_WRITE_ERROR;
		else if (subtype == 73)
			ret = MODID_ISP_FE0_IP_ERROR;
		break;
	case 6:
		if (subtype == 76)
			ret = MODID_ISP_FE1_CMD_DMA_ERROR;
		else if (subtype == 75)
			ret = MODID_ISP_FE1_DATA_DMA_WRITE_ERROR;
		else if (subtype == 73)
			ret = MODID_ISP_FE1_IP_ERROR;
		break;
	case 7:
		if (subtype == 76)
			ret = MODID_ISP_FE2_CMD_DMA_ERROR;
		else if (subtype == 75)
			ret = MODID_ISP_FE2_DATA_DMA_WRITE_ERROR;
		else if (subtype == 73)
			ret = MODID_ISP_FE2_IP_ERROR;
		break;
	case 8:
		if (subtype == 70)
			ret = MODID_ISP_BE_DATA_DMA_READ_ERROR;
		else if (subtype == 71)
			ret = MODID_ISP_BE_DATA_DMA_WRITE_ERROR;
		else if (subtype == 72)
			ret = MODID_ISP_BE_CMD_DMA_ERROR;
		break;
	case 9:
		if (subtype == 74)
			ret = MODID_ISP_PE_DATA_DMA_READ_ERROR;
		else if (subtype == 75)
			ret = MODID_ISP_PE_DATA_DMA_WRITE_ERROR;
		else if (subtype == 76)
			ret = MODID_ISP_PE_CMD_DMA_ERROR;
		else if (subtype == 77)
			ret = MODID_ISP_PE_WARPEIS_SYS_ERROR;
		else if (subtype == 78)
			ret = MODID_ISP_PE_WAPRTNR_SYS_ERROR;
		break;
	case 10:
		if (subtype == 73)
			ret = MODID_ISP_CVE_IP_ERROR;
		else if (subtype == 74)
			ret = MODID_ISP_CVE_DATA_DMA_READ_ERROR;
		else if (subtype == 76)
			ret = MODID_ISP_CVE_DATA_DMA_WRITE_ERROR;
		else if (subtype == 79)
			ret = MODID_ISP_CVE_ALGIN_MFNR_ERROR;
		else if (subtype == 80)
			ret = MODID_ISP_CVE_ALGIN1_CMD_DMA_ERROR;
		else if (subtype == 81)
			ret = MODID_ISP_CVE_MFNR_CMD_DMA_ERROR;
		break;
	case 16:
		ret = MODID_ISP_I2C_ERROR;
		break;
	case 17:
		ret = MODID_ISP_I3C_ERROR;
		break;
	case 18:
		if (subtype == 91)
			ret = MODID_ISP_OIS_CAM_PROBE_ERROR;
		else if (subtype == 92)
			ret = MODID_ISP_OIS_CAM_INIT_ERROR;
		break;
	case 19:
		ret = MODID_ISP_CIS_CAM_PROBE_ERROR;
		break;
	case 20:
		ret = MODID_ISP_EEPROM_CAM_PROBE_ERROR;
		break;
	default:
		break;
	}
	//edr_info("mdr id: %x", ret);
	return ret;
}
EXPORT_SYMBOL_GPL(xrisp_exception_to_mdr);

#define PROC_NAME "xrisp_mdr_test"

static int value;

static int proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", value);
	return 0;
}

static int proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_show, NULL);
}

static ssize_t mdr_test_proc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	char str[10];
	int len;

	len = snprintf(str, sizeof(str), "%d\n", value);
	if (*ppos >= len)
		return 0;

	if (copy_to_user(buf, str + *ppos, len - *ppos))
		return -EFAULT;

	*ppos = len;
	return len;
}

static ssize_t mdr_test_proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char str[10];
	int err;
	uint32_t mod_start;

	if (count >= sizeof(str))
		return -EINVAL;
	if (copy_from_user(str, buf, count))
		return -EFAULT;

	str[count] = '\0';
	err = kstrtoint(str, 10, &value);
	if (err)
		return err;
	switch (value) {
	case 0:
		//auto mode
		for (mod_start = MODID_ISP_FW_PANIC; mod_start < (MODID_ISP_EEPROM_CAM_PROBE_ERROR + 1); mod_start++)
			mdr_system_error(mod_start, 0, 0);
		break;
	case 1:
		mdr_system_error(MODID_ISP_FW_PANIC, 0, 0);
		break;
	case 2:
		mdr_system_error(MODID_ISP_FW_WDT_HANG, 0, 0);
		break;
	case 3:
		mdr_system_error(MODID_ISP_FW_TIMER_HANG, 0, 0);
		break;
	case 4:
		mdr_system_error(MODID_ISP_MIPI_TIMEOUT_ERROR, 0, 0);
		break;
	case 5:
		mdr_system_error(MODID_ISP_MIPI_RX_ERROR, 0, 0);
		break;
	case 6:
		mdr_system_error(MODID_ISP_MIPI_HSRX_TO_ERROR, 0, 0);
		break;
	case 7:
		mdr_system_error(MODID_ISP_MIPI_DESKEW_ERROR, 0, 0);
		break;
	case 8:
		mdr_system_error(MODID_ISP_MIPI_PHY01_ERROR, 0, 0);
		break;
	case 9:
		mdr_system_error(MODID_ISP_MIPI_PHY02_ERROR, 0, 0);
		break;
	case 10:
		mdr_system_error(MODID_ISP_MIPI_CSI2_ERROR, 0, 0);
		break;
	case 11:
		mdr_system_error(MODID_ISP_MIPI_FRAME_ERROR, 0, 0);
		break;
	case 12:
		mdr_system_error(MODID_ISP_MIPI_LINE_ERROR, 0, 0);
		break;
	case 13:
		mdr_system_error(MODID_ISP_MIPI_SDI_HD_FIFO_ERROR, 0, 0);
		break;
	case 14:
		mdr_system_error(MODID_ISP_MIPI_SDI_PLD_FIFO_ERROR, 0, 0);
		break;
	case 15:
		mdr_system_error(MODID_ISP_MIPI_SDI_WC_ERROR, 0, 0);
		break;
	case 16:
		mdr_system_error(MODID_ISP_MIPI_BUS_ERROR, 0, 0);
		break;
	case 17:
		mdr_system_error(MODID_ISP_MIPI_AXI_ERROR, 0, 0);
		break;
	case 18:
		mdr_system_error(MODID_ISP_MIPI_WR_ERROR, 0, 0);
		break;
	case 19:
		mdr_system_error(MODID_ISP_MIPI_SDI_BUF_ERROR, 0, 0);
		break;
	case 20:
		mdr_system_error(MODID_ISP_FE_ROUTER_SDI0_VBUS_ERROR, 0, 0);
		break;
	case 21:
		mdr_system_error(MODID_ISP_FE_ROUTER_SDI1_VBUS_ERROR, 0, 0);
		break;
	case 22:
		mdr_system_error(MODID_ISP_FE_ROUTER_SDI2_VBUS_ERROR, 0, 0);
		break;
	case 23:
		mdr_system_error(MODID_ISP_FE_ROUTER_SDI0_BUF_FULL, 0, 0);
		break;
	case 24:
		mdr_system_error(MODID_ISP_FE_ROUTER_SDI1_BUF_FULL, 0, 0);
		break;
	case 25:
		mdr_system_error(MODID_ISP_FE_ROUTER_SDI2_BUF_FULL, 0, 0);
		break;
	case 26:
		mdr_system_error(MODID_ISP_FE_ROUTER_DATA_DMA_READ, 0, 0);
		break;
	case 27:
		mdr_system_error(MODID_ISP_FE_ROUTER_DATA_DMA_WRITE, 0, 0);
		break;
	case 28:
		mdr_system_error(MODID_ISP_FE0_CMD_DMA_ERROR, 0, 0);
		break;
	case 29:
		mdr_system_error(MODID_ISP_FE0_DATA_DMA_WRITE_ERROR, 0, 0);
		break;
	case 30:
		mdr_system_error(MODID_ISP_FE0_IP_ERROR, 0, 0);
		break;
	case 31:
		mdr_system_error(MODID_ISP_FE1_CMD_DMA_ERROR, 0, 0);
		break;
	case 32:
		mdr_system_error(MODID_ISP_FE1_DATA_DMA_WRITE_ERROR, 0, 0);
		break;
	case 33:
		mdr_system_error(MODID_ISP_FE1_IP_ERROR, 0, 0);
		break;
	case 34:
		mdr_system_error(MODID_ISP_FE2_CMD_DMA_ERROR, 0, 0);
		break;
	case 35:
		mdr_system_error(MODID_ISP_FE2_DATA_DMA_WRITE_ERROR, 0, 0);
		break;
	case 36:
		mdr_system_error(MODID_ISP_FE2_IP_ERROR, 0, 0);
		break;
	case 37:
		mdr_system_error(MODID_ISP_BE_DATA_DMA_READ_ERROR, 0, 0);
		break;
	case 38:
		mdr_system_error(MODID_ISP_BE_DATA_DMA_WRITE_ERROR, 0, 0);
		break;
	case 39:
		mdr_system_error(MODID_ISP_BE_CMD_DMA_ERROR, 0, 0);
		break;
	case 40:
		mdr_system_error(MODID_ISP_CVE_IP_ERROR, 0, 0);
		break;
	case 41:
		mdr_system_error(MODID_ISP_CVE_DATA_DMA_READ_ERROR, 0, 0);
		break;
	case 42:
		mdr_system_error(MODID_ISP_CVE_DATA_DMA_WRITE_ERROR, 0, 0);
		break;
	case 43:
		mdr_system_error(MODID_ISP_CVE_ALGIN_MFNR_ERROR, 0, 0);
		break;
	case 44:
		mdr_system_error(MODID_ISP_CVE_ALGIN1_CMD_DMA_ERROR, 0, 0);
		break;
	case 45:
		mdr_system_error(MODID_ISP_CVE_MFNR_CMD_DMA_ERROR, 0, 0);
		break;
	case 46:
		mdr_system_error(MODID_ISP_PE_DATA_DMA_READ_ERROR, 0, 0);
		break;
	case 47:
		mdr_system_error(MODID_ISP_PE_DATA_DMA_WRITE_ERROR, 0, 0);
		break;
	case 48:
		mdr_system_error(MODID_ISP_PE_CMD_DMA_ERROR, 0, 0);
		break;
	case 49:
		mdr_system_error(MODID_ISP_PE_WARPEIS_SYS_ERROR, 0, 0);
		break;
	case 50:
		mdr_system_error(MODID_ISP_PE_WAPRTNR_SYS_ERROR, 0, 0);
		break;
	case 51:
		mdr_system_error(MODID_ISP_I2C_ERROR, 0, 0);
		break;
	case 52:
		mdr_system_error(MODID_ISP_I3C_ERROR, 0, 0);
		break;
	case 53:
		mdr_system_error(MODID_ISP_OIS_CAM_PROBE_ERROR, 0, 0);
		break;
	case 54:
		mdr_system_error(MODID_ISP_OIS_CAM_INIT_ERROR, 0, 0);
		break;
	case 55:
		mdr_system_error(MODID_ISP_CIS_CAM_PROBE_ERROR, 0, 0);
		break;
	case 56:
		mdr_system_error(MODID_ISP_EEPROM_CAM_PROBE_ERROR, 0, 0);
		break;
	default:
		break;
	}
	return count;
}

static const struct proc_ops xrisp_mdr_proc_ops = {
	.proc_flags	= PROC_ENTRY_PERMANENT,
	.proc_open	= proc_open,
	.proc_read	= mdr_test_proc_read,
	.proc_write	= mdr_test_proc_write,
	.proc_release	= single_release,
	.proc_lseek	= seq_lseek,
};

int __init xring_mdr_init(void)
{
	proc_create(PROC_NAME, 0644, NULL, &xrisp_mdr_proc_ops);
	return 0;
}

void __exit xring_mdr_exit(void)
{
	remove_proc_entry(PROC_NAME, NULL);
}

MODULE_AUTHOR("fang jun<fangjun3@xiaomi.com>");
MODULE_DESCRIPTION("x-ring edr mdr");
MODULE_LICENSE("GPL v2");

