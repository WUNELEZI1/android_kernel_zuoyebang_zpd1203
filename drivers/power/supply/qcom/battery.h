/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017, 2019-2020 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
 */
#ifndef __BATTERY_H
#define __BATTERY_H

struct charger_param {
	u32 fcc_step_delay_ms;
	u32 fcc_step_size_ua;
	u32 fcc_step_start_ua;
	u32 smb_version;
	u32 hvdcp2_max_icl_ua;
	u32 hvdcp3_max_icl_ua;
	u32 qc4_max_icl_ua;
	u32 forced_main_fcc;
	int (*iio_read)(struct device *dev, int iio_chan, int *val);
	int (*iio_write)(struct device *dev, int iio_chan, int val);
};

static const char * const power_supply_usb_type_text[] = {
	"Unknown", "Battery", "UPS", "Mains", "USB",
	"USB_DCP", "USB_CDP", "USB_ACA", "USB_C",
	"USB_PD", "USB_PD_DRP", "BrickID", "Wireless",
	"USB_HVDCP", "USB_HVDCP_3", "USB_HVDCP_3P5", "USB_FLOAT",
	"USB_PPS",
};

static const char * const country_name_text[] = {
	"Unknown", "CN", "IN", "EEA", "GLOBAL",
};

int qcom_batt_init(struct device *dev, struct charger_param *param);
void qcom_batt_deinit(void);
#endif /* __BATTERY_H */
