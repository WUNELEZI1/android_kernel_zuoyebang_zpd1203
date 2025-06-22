// SPDX-License-Identifier: GPL-2.0-only
/*
 * extcon-madera.c - Extcon driver for Cirrus Logic Madera codecs
 *
 * Copyright 2015-2017 Cirrus Logic
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/extcon-provider.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/math64.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/property.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/regulator/consumer.h>
#include <linux/of.h>
#include <linux/version.h>
#include <linux/ktime.h>


#include <sound/soc.h>

/* kernel5.15 Header file definition*/
#include "../include/extcon/extcon-madera.h"
#include "../include/extcon/extcon-madera-pdata.h"
#include "../include/dt-bindings/extcon/extcon-madera.h"
#include "../include/mfd/madera/core.h"
#include "../include/mfd/madera/pdata.h"
#include "../include/mfd/madera/registers.h"

#include <linux/irqchip/irq-madera.h>
#include <linux/debugfs.h>

#include "../mfd/madera.h"
#include <soc/xring/fsa4480-i2c.h>
#define DEBUG

#define MIN_BUTTON_DETECT_DELAY_US 160000

#define MADERA_MAX_MICD_RANGE		8

#define MADERA_MICD_CLAMP_MODE_JD1L	0x4
#define MADERA_MICD_CLAMP_MODE_JD1H	0x5
#define MADERA_MICD_CLAMP_MODE_JD1L_JD2L 0x8
#define MADERA_MICD_CLAMP_MODE_JD1L_JD2H 0x9
#define MADERA_MICD_CLAMP_MODE_JD1H_JD2H 0xb

#define MADERA_HPDET_MAX_OHM		10000
#define MADERA_HPDET_MAX_HOHM		(MADERA_HPDET_MAX_OHM * 100)
#define MADERA_HP_SHORT_IMPEDANCE_MIN	4

#define MADERA_HPDET_DEBOUNCE_MS	500
#define MADERA_DEFAULT_MICD_TIMEOUT_MS	2000

#define MADERA_HPDONE_PROBE_INTERVAL_MS	20
#define MADERA_HPDONE_PROBE_COUNT	15

#define MADERA_MICROPHONE_MIN_OHM	1258
#define MADERA_MICROPHONE_MAX_OHM	30000

#define MADERA_HP_TUNING_INVALID	-1

#define MEDIA_PREVIOUS_SCAN_CODE 257
#define MEDIA_NEXT_SCAN_CODE 258
/* Audio End */
/* add headset_status */
#define HEADSET_STATUS_RECORD
#ifdef HEADSET_STATUS_RECORD
#define HEADSET_STATUS_RECORD_INDEX_PLUGIN (0)
#define HEADSET_STATUS_RECORD_INDEX_KEY_PREVIOUS (1)
#define HEADSET_STATUS_RECORD_INDEX_KEY_NEXT (2)
#define HEADSET_STATUS_RECORD_INDEX_KEY_MEDIA (3)
#define HEADSET_STATUS_RECORD_INDEX_PLUGOUT (4)
#define HEADSET_EVENT_PLUGIN_HEADPHONE (0)
#define HEADSET_EVENT_PLUGIN_MICROPHONE (4)
#define HEADSET_EVENT_PLUGIN_JACK (8)
#define HEADSET_EVENT_KEY_PREVIOUS_DOWN (0)
#define HEADSET_EVENT_KEY_PREVIOUS_UP (4)
#define HEADSET_EVENT_KEY_NEXT_DOWN (0)
#define HEADSET_EVENT_KEY_NEXT_UP (4)
#define HEADSET_EVENT_KEY_MEDIA_DOWN (0)
#define HEADSET_EVENT_KEY_MEDIA_UP (4)
#define HEADSET_EVENT_PLUGOUT_HEADPHONE (0)
#define HEADSET_EVENT_PLUGOUT_MICROPHONE (4)
#define HEADSET_EVENT_PLUGOUT_JACK (8)
#define DEBUGFS_DIR_NAME "accdet"
#define DEBUGFS_HEADSET_STATUS_FILE_NAME "headset_status"
#define HEADSET_EVENT_MAX (5)
#define HEADSET_EVENT_OFFSET_MAX (12)
static u16 headset_status[HEADSET_EVENT_MAX] = {0,0,0,0,0};
// static u32 headphone_status = 0;
// static u32 microphone_status = 0;
static struct dentry* accdet_debugfs_dir;
#endif

#ifdef HEADSET_STATUS_RECORD
static ssize_t headset_status_read(struct file *filp, char __user *buffer,
	size_t count, loff_t *ppos);
static ssize_t headset_status_write(struct file *filp, const char __user *buffer,
	size_t count, loff_t *ppos);
static void add_headset_event(u32 event_index, u32 event_offset);
#endif

#ifdef HEADSET_STATUS_RECORD
static void add_headset_event(u32 event_index, u32 event_offset) {
	u16 status;
	if (event_index >= HEADSET_EVENT_MAX) {
		return;
	}
	status = (headset_status[event_index] & (0xF << event_offset));
	status += (0x1 << event_offset);
	if (status > (0xF << event_offset)) {
		status = (0xF << event_offset);
	}
	headset_status[event_index] = (headset_status[event_index] & (~(0xF << event_offset)))
								  + status;
	pr_info("%s:HEADSET_STATUS_RECORD 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x",__func__,
		headset_status[0], headset_status[1],
		headset_status[2], headset_status[3],
		headset_status[4]);
	return;
}
static ssize_t headset_status_read(struct file *filp, char __user *buffer,
	size_t count, loff_t *ppos) {
	char buf[64];
	sprintf(buf, "0x%04x 0x%04x 0x%04x 0x%04x 0x%04x\n",
		headset_status[0], headset_status[1],
		headset_status[2], headset_status[3],
		headset_status[4]);
	pr_info("%s() headset_status_debug 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x\n", __func__,
				 headset_status[0], headset_status[1],
				 headset_status[2], headset_status[3],
				 headset_status[4]);
	return simple_read_from_buffer(buffer, count, ppos, buf, strlen(buf));
}
static ssize_t headset_status_write(struct file *filp, const char __user *buffer,
	size_t count, loff_t *ppos) {
	char buf[4];
	size_t buf_size = min(count, sizeof(buf) - 1);
	if (copy_from_user(buf, buffer, buf_size))
		return -EFAULT;
	if (strncmp(buf, "0", 1) == 0) {
		memset(headset_status, 0, sizeof(headset_status));
	}
	pr_info("%s() headset_status_debug 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x\n", __func__,
				 headset_status[0], headset_status[1],
				 headset_status[2], headset_status[3],
				 headset_status[4]);
	return count;
}
static const struct file_operations accdet_headset_status_fops = {
	.owner = THIS_MODULE,
	.read = headset_status_read,
	.write = headset_status_write,
};
#endif
#ifdef HEADSET_STATUS_RECORD
enum madera_accdet_test {
	MADERA_ACCDET_TEST_PLUGIN,
	MADERA_ACCDET_TEST_PLUGOUT,
	MADERA_ACCDET_TEST_KEY_PREVIOUS_DOWN,
	MADERA_ACCDET_TEST_KEY_NEXT_DOWN,
	MADERA_ACCDET_TEST_KEY_MEDIA_DOWN,
	MADERA_ACCDET_TEST_KEY_PREVIOUS_UP,
	MADERA_ACCDET_TEST_KEY_NEXT_UP,
	MADERA_ACCDET_TEST_KEY_MEDIA_UP,
};

static ssize_t headset_key_test_write(struct file *filp, const char __user *buffer,
	size_t count, loff_t *ppos) {
	char buf[4];
	unsigned long val,ret;
	size_t buf_size = min(count, sizeof(buf) - 1);
	if (copy_from_user(buf, buffer, buf_size))
		return -EFAULT;
	ret = kstrtoul(buf, 16, &val);
	if (ret)
		return ret;
	switch (val)
	{
		case MADERA_ACCDET_TEST_PLUGIN:
			pr_info("%s() MADERA_ACCDET_TEST_PLUGIN\n", __func__);
			add_headset_event(HEADSET_STATUS_RECORD_INDEX_PLUGIN,HEADSET_EVENT_PLUGIN_HEADPHONE);
			break;
		case MADERA_ACCDET_TEST_PLUGOUT:
			pr_info("%s() MADERA_ACCDET_TEST_PLUGOUT\n", __func__);
			add_headset_event(HEADSET_STATUS_RECORD_INDEX_PLUGOUT,HEADSET_STATUS_RECORD_INDEX_PLUGOUT);
			break;
		case MADERA_ACCDET_TEST_KEY_PREVIOUS_DOWN:
			pr_info("%s() MADERA_ACCDET_TEST_KEY_PREVIOUS_DOWN KEY_VOLUMEUP\n", __func__);
			add_headset_event(HEADSET_STATUS_RECORD_INDEX_KEY_PREVIOUS,HEADSET_EVENT_KEY_PREVIOUS_DOWN);
			break;
		case MADERA_ACCDET_TEST_KEY_NEXT_DOWN:
			pr_info("%s() MADERA_ACCDET_TEST_KEY_NEXT_DOWN KEY_VOLUMEDOWN\n", __func__);
			add_headset_event(HEADSET_STATUS_RECORD_INDEX_KEY_NEXT,HEADSET_EVENT_KEY_NEXT_DOWN);
			break;
		case MADERA_ACCDET_TEST_KEY_MEDIA_DOWN:
			pr_info("%s() MADERA_ACCDET_TEST_KEY_MEDIA_DOWN\n", __func__);
			add_headset_event(HEADSET_STATUS_RECORD_INDEX_KEY_MEDIA,HEADSET_EVENT_KEY_MEDIA_DOWN);
			break;

		case MADERA_ACCDET_TEST_KEY_PREVIOUS_UP:
			pr_info("%s() MADERA_ACCDET_TEST_KEY_PREVIOUS_UP KEY_VOLUMEUP\n", __func__);
			add_headset_event(HEADSET_STATUS_RECORD_INDEX_KEY_PREVIOUS,HEADSET_EVENT_KEY_PREVIOUS_UP);
			break;
		case MADERA_ACCDET_TEST_KEY_NEXT_UP:
			pr_info("%s() MADERA_ACCDET_TEST_KEY_NEXT_UP KEY_VOLUMEDOWN\n", __func__);
			add_headset_event(HEADSET_STATUS_RECORD_INDEX_KEY_NEXT,HEADSET_EVENT_KEY_NEXT_UP);
			break;
		case MADERA_ACCDET_TEST_KEY_MEDIA_UP:
			pr_info("%s() MADERA_ACCDET_TEST_KEY_MEDIA_UP\n", __func__);
			add_headset_event(HEADSET_STATUS_RECORD_INDEX_KEY_MEDIA,HEADSET_EVENT_KEY_MEDIA_UP);
			break;

		default:
			break;
	}
	return count;
}
static const struct file_operations headset_test_fops = {
	.owner = THIS_MODULE,
	.read = headset_status_read,
	.write = headset_key_test_write,
};
#endif

static const unsigned int madera_cable[] = {
//	EXTCON_MECHANICAL,//no need jackdet
	EXTCON_JACK_MICROPHONE,
	EXTCON_JACK_HEADPHONE,
	EXTCON_NONE,
};

static const struct madera_micd_config cs47l85_micd_default_modes[] = {
	{ MADERA_ACCD_SENSE_MICDET2, 0, MADERA_ACCD_BIAS_SRC_MICBIAS1, 0, 0 },
	{ MADERA_ACCD_SENSE_MICDET1, 0, MADERA_ACCD_BIAS_SRC_MICBIAS2, 1, 0 },
};

static const struct madera_micd_config madera_micd_default_modes[] = {
	{ MADERA_MICD1_SENSE_MICDET1, MADERA_MICD1_GND_MICDET2,
	  MADERA_MICD_BIAS_SRC_MICBIAS1A, 0, MADERA_HPD_GND_HPOUTFB2 },
	{ MADERA_MICD1_SENSE_MICDET2, MADERA_MICD1_GND_MICDET1,
	  MADERA_MICD_BIAS_SRC_MICBIAS1B, 1, MADERA_HPD_GND_HPOUTFB1 },
};

static const unsigned int madera_default_hpd_pins[4] = {
	[0] = MADERA_HPD_OUT_OUT1L,
	[1] = MADERA_HPD_SENSE_HPDET1,
	[2] = MADERA_HPD_OUT_OUT1R,
	[3] = MADERA_HPD_SENSE_HPDET1,
};

static struct madera_micd_range madera_micd_default_ranges[] = {
	{ .max = 70,   .key = KEY_MEDIA },//KEY_PLAYPAUSE
	{ .max = 186,  .key = KEY_VOICECOMMAND },
	{ .max = 350,  .key = MEDIA_PREVIOUS_SCAN_CODE },//KEY_VOLUMEUP
	{ .max = 800,  .key = MEDIA_NEXT_SCAN_CODE },//KEY_VOLUMEDOWN
};

/* The number of levels in madera_micd_levels valid for button thresholds */
#define MADERA_NUM_MICD_BUTTON_LEVELS 64

/* ohms for each micd level */
static const int madera_micd_levels[] = {
	3, 6, 8, 11, 13, 16, 18, 21, 23, 26, 28, 31, 34, 36, 39, 41, 44, 46,
	49, 52, 54, 57, 60, 62, 65, 67, 70, 73, 75, 78, 81, 83, 89, 94, 100,
	105, 111, 116, 122, 127, 139, 150, 161, 173, 186, 196, 209, 220, 245,
	270, 295, 321, 348, 375, 402, 430, 489, 550, 614, 681, 752, 903, 1071,
	1257, 30000,
};
struct madera_extcon *g_madera_extcon_info = NULL;

/*
 * HP calibration data
 * See the datasheet for the meanings of the constants and their values
 */
struct madera_hpdet_calibration_data {
	int	min;		/* ohms */
	int	max;		/* ohms */
	s64	C0;		/* value * 1000000 */
	s64	C1;		/* value * 10000 */
	s64	C2;		/* not multiplied */
	s64	C3;		/* value * 1000000 */
	s64	C4_x_C3;	/* value * 1000000 */
	s64	C5;		/* value * 1000000 */
	s64	dacval_adjust;
};

static const struct madera_hpdet_calibration_data cs47l15_hpdet_ranges[] = {
	{},
	{ 33,   123,   1000000,   -4300,   7975, 69600000, 382800, 33350000,
	  500000},
	{ 123,  1033,  9633000,   -79500,  7300, 62900000, 283050, 33350000,
	  500000},
	{ 1033, 10033, 100684000, -949400, 7300, 63200000, 284400, 33350000,
	  500000},
};


static const struct madera_hpdet_calibration_data cs47l85_hpdet_ranges[] = {
	{ 4,    30,    1007000,   -7200,   4003, 69300000, 381150, 250000, 500000},
	{ 8,    100,   1007000,   -7200,   7975, 69600000, 382800, 250000, 500000},
	{ 100,  1000,  9696000,   -79500,  7300, 62900000, 345950, 250000, 500000},
	{ 1000, 10000, 100684000, -949400, 7300, 63200000, 347600, 250000, 500000},
};

static const struct madera_hpdet_calibration_data madera_hpdet_ranges[] = {
	{ 4,    30,    1014000,   -4300,   3950, 69300000, 381150, 700000, 500000},
	{ 8,    100,   1014000,   -8600,   7975, 69600000, 382800, 700000, 500000},
	{ 100,  1000,  9744000,   -79500,  7300, 62900000, 345950, 700000, 500000},
	{ 1000, 10000, 101158000, -949400, 7300, 63200000, 347600, 700000, 500000},
};

static const struct madera_hpdet_calibration_data cs47l92_hpdet_ranges[] = {
	{ 4,    30,    1007000,   -7200,   4005, 69300000, 381150, 600000, 500000},
	{ 8,    100,   1007000,   -7200,   7975, 69600000, 382800, 600000, 500000},
	{ 100,  1000,  9744000,   -79500,  7300, 62900000, 345950, 600000, 500000},
	{ 1000, 10000, 100684000, -949400, 7300, 63200000, 347600, 600000, 500000},
};

struct madera_hp_tuning {
	int max_hohm;
	const struct reg_sequence *patch;
	int patch_len;
};

static const struct reg_sequence cs47l15_low_impedance_patch[] = {
	{ 0x460, 0x0C00 },
	{ 0x461, 0xCB59 },
	{ 0x462, 0x0C00 },
	{ 0x463, 0x6037 },
	{ 0x464, 0x0C01 },
	{ 0x465, 0x2D86 },
	{ 0x466, 0x0801 },
	{ 0x467, 0x264E },
	{ 0x468, 0x0801 },
	{ 0x469, 0x1E6D },
	{ 0x46A, 0x0802 },
	{ 0x46B, 0x199A },
	{ 0x46C, 0x0802 },
	{ 0x46D, 0x1220 },
	{ 0x46E, 0x0802 },
	{ 0x46F, 0x0E65 },
	{ 0x470, 0x0806 },
	{ 0x471, 0x0A31 },
	{ 0x472, 0x080E },
	{ 0x473, 0x040F },
	{ 0x474, 0x080E },
	{ 0x475, 0x0339 },
	{ 0x476, 0x080E },
	{ 0x477, 0x028F },
	{ 0x478, 0x080E },
	{ 0x479, 0x0209 },
	{ 0x47A, 0x080E },
	{ 0x47B, 0x00CF },
	{ 0x47C, 0x080E },
	{ 0x47D, 0x0001 },
	{ 0x47E, 0x081F },
};

static const struct reg_sequence cs47l15_normal_impedance_patch[] = {
	{ 0x460, 0x0C00 },
	{ 0x461, 0xCB59 },
	{ 0x462, 0x0C00 },
	{ 0x463, 0xB53C },
	{ 0x464, 0x0C01 },
	{ 0x465, 0x4827 },
	{ 0x466, 0x0801 },
	{ 0x467, 0x3950 },
	{ 0x468, 0x0801 },
	{ 0x469, 0x264E },
	{ 0x46A, 0x0802 },
	{ 0x46B, 0x1E6D },
	{ 0x46C, 0x0802 },
	{ 0x46D, 0x199A },
	{ 0x46E, 0x0802 },
	{ 0x46F, 0x1456 },
	{ 0x470, 0x0806 },
	{ 0x471, 0x1220 },
	{ 0x472, 0x080E },
	{ 0x473, 0x040F },
	{ 0x474, 0x080E },
	{ 0x475, 0x0339 },
	{ 0x476, 0x080E },
	{ 0x477, 0x028F },
	{ 0x478, 0x080E },
	{ 0x479, 0x0209 },
	{ 0x47A, 0x080E },
	{ 0x47B, 0x00CF },
	{ 0x47C, 0x080E },
	{ 0x47D, 0x0001 },
	{ 0x47E, 0x081F },
};

static const struct reg_sequence cs47l15_high_impedance_patch[] = {
	{ 0x460, 0x0C00 },
	{ 0x461, 0xCB59 },
	{ 0x462, 0x0C00 },
	{ 0x463, 0xB53C },
	{ 0x464, 0x0C01 },
	{ 0x465, 0x6037 },
	{ 0x466, 0x0801 },
	{ 0x467, 0x4827 },
	{ 0x468, 0x0801 },
	{ 0x469, 0x3950 },
	{ 0x46A, 0x0802 },
	{ 0x46B, 0x264E },
	{ 0x46C, 0x0802 },
	{ 0x46D, 0x1E6D },
	{ 0x46E, 0x0802 },
	{ 0x46F, 0x199A },
	{ 0x470, 0x0806 },
	{ 0x471, 0x1220 },
	{ 0x472, 0x080E },
	{ 0x473, 0x040F },
	{ 0x474, 0x080E },
	{ 0x475, 0x0339 },
	{ 0x476, 0x080E },
	{ 0x477, 0x028F },
	{ 0x478, 0x080E },
	{ 0x479, 0x0209 },
	{ 0x47A, 0x080E },
	{ 0x47B, 0x00CF },
	{ 0x47C, 0x080E },
	{ 0x47D, 0x0001 },
	{ 0x47E, 0x081F },
};

static const struct madera_hp_tuning cs47l15_hp_tuning[] = {
	{
		1600,
		cs47l15_low_impedance_patch,
		ARRAY_SIZE(cs47l15_low_impedance_patch),
	},
	{
		3200,
		cs47l15_normal_impedance_patch,
		ARRAY_SIZE(cs47l15_normal_impedance_patch),
	},
	{
		MADERA_HPDET_MAX_HOHM,
		cs47l15_high_impedance_patch,
		ARRAY_SIZE(cs47l15_high_impedance_patch),
	},
};

static const struct reg_sequence cs47l35_low_impedance_patch[] = {
	{ 0x460, 0x0C40 },
	{ 0x461, 0xCD1A },
	{ 0x462, 0x0C40 },
	{ 0x463, 0xB53B },
	{ 0x464, 0x0C41 },
	{ 0x465, 0x4826 },
	{ 0x466, 0x0C41 },
	{ 0x467, 0x2EDA },
	{ 0x468, 0x0C41 },
	{ 0x469, 0x203A },
	{ 0x46A, 0x0841 },
	{ 0x46B, 0x121F },
	{ 0x46C, 0x0446 },
	{ 0x46D, 0x0B6F },
	{ 0x46E, 0x0446 },
	{ 0x46F, 0x0818 },
	{ 0x470, 0x04C6 },
	{ 0x471, 0x05BB },
	{ 0x472, 0x04C6 },
	{ 0x473, 0x040F },
	{ 0x474, 0x04CE },
	{ 0x475, 0x0339 },
	{ 0x476, 0x05DF },
	{ 0x477, 0x028F },
	{ 0x478, 0x05DF },
	{ 0x479, 0x0209 },
	{ 0x47A, 0x05DF },
	{ 0x47B, 0x00CF },
	{ 0x47C, 0x05DF },
	{ 0x47D, 0x0001 },
	{ 0x47E, 0x07FF },
};

static const struct reg_sequence cs47l35_normal_impedance_patch[] = {
	{ 0x460, 0x0C40 },
	{ 0x461, 0xCD1A },
	{ 0x462, 0x0C40 },
	{ 0x463, 0xB53B },
	{ 0x464, 0x0C40 },
	{ 0x465, 0x7503 },
	{ 0x466, 0x0C40 },
	{ 0x467, 0x4A41 },
	{ 0x468, 0x0041 },
	{ 0x469, 0x3491 },
	{ 0x46A, 0x0841 },
	{ 0x46B, 0x1F50 },
	{ 0x46C, 0x0446 },
	{ 0x46D, 0x14ED },
	{ 0x46E, 0x0446 },
	{ 0x46F, 0x1455 },
	{ 0x470, 0x04C6 },
	{ 0x471, 0x1220 },
	{ 0x472, 0x04C6 },
	{ 0x473, 0x040F },
	{ 0x474, 0x04CE },
	{ 0x475, 0x0339 },
	{ 0x476, 0x05DF },
	{ 0x477, 0x028F },
	{ 0x478, 0x05DF },
	{ 0x479, 0x0209 },
	{ 0x47A, 0x05DF },
	{ 0x47B, 0x00CF },
	{ 0x47C, 0x05DF },
	{ 0x47D, 0x0001 },
	{ 0x47E, 0x07FF },
};

static const struct madera_hp_tuning cs47l35_hp_tuning[] = {
	{
		1300,
		cs47l35_low_impedance_patch,
		ARRAY_SIZE(cs47l35_low_impedance_patch),
	},
	{
		MADERA_HPDET_MAX_HOHM,
		cs47l35_normal_impedance_patch,
		ARRAY_SIZE(cs47l35_normal_impedance_patch),
	},
};

static const struct reg_sequence cs47l85_low_impedance_patch[] = {
	{ 0x465, 0x4C6D },
	{ 0x467, 0x3950 },
	{ 0x469, 0x2D86 },
	{ 0x46B, 0x1E6D },
	{ 0x46D, 0x199A },
	{ 0x46F, 0x1456 },
	{ 0x483, 0x0826 },
};

static const struct reg_sequence cs47l85_normal_impedance_patch[] = {
	{ 0x465, 0x8A43 },
	{ 0x467, 0x7259 },
	{ 0x469, 0x65EA },
	{ 0x46B, 0x50F4 },
	{ 0x46D, 0x41CD },
	{ 0x46F, 0x199A },
	{ 0x483, 0x0023 },
};

static const struct madera_hp_tuning cs47l85_hp_tuning[] = {
	{
		1300,
		cs47l85_low_impedance_patch,
		ARRAY_SIZE(cs47l85_low_impedance_patch),
	},
	{
		MADERA_HPDET_MAX_HOHM,
		cs47l85_normal_impedance_patch,
		ARRAY_SIZE(cs47l85_normal_impedance_patch),
	},
};

static const struct reg_sequence cs47l90_low_impedance_patch[] = {
	{ 0x460, 0x0C21 },
	{ 0x461, 0xB53C },
	{ 0x462, 0x0C21 },
	{ 0x463, 0xA186 },
	{ 0x464, 0x0C21 },
	{ 0x465, 0x8FF6 },
	{ 0x466, 0x0C24 },
	{ 0x467, 0x804E },
	{ 0x468, 0x0C24 },
	{ 0x469, 0x725A },
	{ 0x46A, 0x0C24 },
	{ 0x46B, 0x5AD5 },
	{ 0x46C, 0x0C28 },
	{ 0x46D, 0x50F4 },
	{ 0x46E, 0x0C2C },
	{ 0x46F, 0x4827 },
	{ 0x470, 0x0C31 },
	{ 0x471, 0x404E },
	{ 0x472, 0x0020 },
	{ 0x473, 0x3950 },
	{ 0x474, 0x0028 },
	{ 0x475, 0x3314 },
	{ 0x476, 0x0030 },
	{ 0x477, 0x2893 },
	{ 0x478, 0x003F },
	{ 0x479, 0x2429 },
	{ 0x47A, 0x0830 },
	{ 0x47B, 0x203A },
	{ 0x47C, 0x0420 },
	{ 0x47D, 0x1027 },
	{ 0x47E, 0x0430 },
};

static const struct reg_sequence cs47l90_normal_impedance_patch[] = {
	{ 0x460, 0x0C21 },
	{ 0x461, 0xB53C },
	{ 0x462, 0x0C25 },
	{ 0x463, 0xA186 },
	{ 0x464, 0x0C26 },
	{ 0x465, 0x8FF6 },
	{ 0x466, 0x0C28 },
	{ 0x467, 0x804E },
	{ 0x468, 0x0C30 },
	{ 0x469, 0x725A },
	{ 0x46A, 0x0C30 },
	{ 0x46B, 0x65EA },
	{ 0x46C, 0x0028 },
	{ 0x46D, 0x5AD5 },
	{ 0x46E, 0x0028 },
	{ 0x46F, 0x50F4 },
	{ 0x470, 0x0030 },
	{ 0x471, 0x4827 },
	{ 0x472, 0x0030 },
	{ 0x473, 0x404E },
	{ 0x474, 0x003F },
	{ 0x475, 0x3950 },
	{ 0x476, 0x0830 },
	{ 0x477, 0x3314 },
	{ 0x478, 0x0420 },
	{ 0x479, 0x2D86 },
	{ 0x47A, 0x0428 },
	{ 0x47B, 0x2893 },
	{ 0x47C, 0x0428 },
	{ 0x47D, 0x203A },
	{ 0x47E, 0x0428 },
};

static const struct reg_sequence cs47l90_high_impedance_patch[] = {
	{ 0x460, 0x0C21 },
	{ 0x461, 0xB53C },
	{ 0x462, 0x0C26 },
	{ 0x463, 0xA186 },
	{ 0x464, 0x0C28 },
	{ 0x465, 0x8FF6 },
	{ 0x466, 0x0C2A },
	{ 0x467, 0x804E },
	{ 0x468, 0x0025 },
	{ 0x469, 0x725A },
	{ 0x46A, 0x0030 },
	{ 0x46B, 0x65EA },
	{ 0x46C, 0x0030 },
	{ 0x46D, 0x5AD5 },
	{ 0x46E, 0x003F },
	{ 0x46F, 0x50F4 },
	{ 0x470, 0x003F },
	{ 0x471, 0x4827 },
	{ 0x472, 0x0830 },
	{ 0x473, 0x404E },
	{ 0x474, 0x083F },
	{ 0x475, 0x3950 },
	{ 0x476, 0x0420 },
	{ 0x477, 0x3314 },
	{ 0x478, 0x0430 },
	{ 0x479, 0x2D86 },
	{ 0x47A, 0x0430 },
	{ 0x47B, 0x2893 },
	{ 0x47C, 0x0430 },
	{ 0x47D, 0x203A },
	{ 0x47E, 0x0430 },
};

static const struct madera_hp_tuning cs47l90_hp_tuning[] = {
	{
		1400,
		cs47l90_low_impedance_patch,
		ARRAY_SIZE(cs47l90_low_impedance_patch),
	},
	{	2400,
		cs47l90_normal_impedance_patch,
		ARRAY_SIZE(cs47l90_normal_impedance_patch),
	},
	{	MADERA_HPDET_MAX_HOHM,
		cs47l90_high_impedance_patch,
		ARRAY_SIZE(cs47l90_high_impedance_patch),
	},
};

static const struct reg_sequence cs47l92_low_impedance_patch[] = {
	{ 0x460, 0x0C21 },
	{ 0x461, 0xB53C },
	{ 0x462, 0x0C21 },
	{ 0x463, 0xA186 },
	{ 0x464, 0x0C21 },
	{ 0x465, 0x8FF6 },
	{ 0x466, 0x0C21 },
	{ 0x467, 0x804E },
	{ 0x468, 0x0C21 },
	{ 0x469, 0x725A },
	{ 0x46A, 0x0C21 },
	{ 0x46B, 0x5AD5 },
	{ 0x46C, 0x0C21 },
	{ 0x46D, 0x50F4 },
	{ 0x46E, 0x0C21 },
	{ 0x46F, 0x4827 },
	{ 0x470, 0x0C21 },
	{ 0x471, 0x404E },
	{ 0x472, 0x0020 },
	{ 0x473, 0x3950 },
	{ 0x474, 0x0021 },
	{ 0x475, 0x3314 },
	{ 0x476, 0x0021 },
	{ 0x477, 0x2893 },
	{ 0x478, 0x0021 },
	{ 0x479, 0x2429 },
	{ 0x47A, 0x0821 },
	{ 0x47B, 0x203A },
	{ 0x47C, 0x0420 },
	{ 0x47D, 0x1027 },
	{ 0x47E, 0x0421 },
};

static const struct reg_sequence cs47l92_normal_impedance_patch[] = {
	{ 0x460, 0x0C21 },
	{ 0x461, 0xB53C },
	{ 0x462, 0x0C21 },
	{ 0x463, 0xA186 },
	{ 0x464, 0x0C21 },
	{ 0x465, 0x8FF6 },
	{ 0x466, 0x0C21 },
	{ 0x467, 0x804E },
	{ 0x468, 0x0C21 },
	{ 0x469, 0x725A },
	{ 0x46A, 0x0C21 },
	{ 0x46B, 0x65EA },
	{ 0x46C, 0x0021 },
	{ 0x46D, 0x5AD5 },
	{ 0x46E, 0x0021 },
	{ 0x46F, 0x50F4 },
	{ 0x470, 0x0021 },
	{ 0x471, 0x4827 },
	{ 0x472, 0x0021 },
	{ 0x473, 0x404E },
	{ 0x474, 0x0021 },
	{ 0x475, 0x3950 },
	{ 0x476, 0x0821 },
	{ 0x477, 0x3314 },
	{ 0x478, 0x0420 },
	{ 0x479, 0x2D86 },
	{ 0x47A, 0x0421 },
	{ 0x47B, 0x2893 },
	{ 0x47C, 0x0421 },
	{ 0x47D, 0x203A },
	{ 0x47E, 0x0421 },
};

static const struct reg_sequence cs47l92_high_impedance_patch[] = {
	{ 0x460, 0x0C21 },
	{ 0x461, 0xB53C },
	{ 0x462, 0x0C21 },
	{ 0x463, 0xA186 },
	{ 0x464, 0x0C21 },
	{ 0x465, 0x8FF6 },
	{ 0x466, 0x0021 },
	{ 0x467, 0x804E },
	{ 0x468, 0x0021 },
	{ 0x469, 0x725A },
	{ 0x46A, 0x0021 },
	{ 0x46B, 0x65EA },
	{ 0x46C, 0x0021 },
	{ 0x46D, 0x5AD5 },
	{ 0x46E, 0x0021 },
	{ 0x46F, 0x50F4 },
	{ 0x470, 0x0021 },
	{ 0x471, 0x4827 },
	{ 0x472, 0x0821 },
	{ 0x473, 0x404E },
	{ 0x474, 0x0821 },
	{ 0x475, 0x3950 },
	{ 0x476, 0x0420 },
	{ 0x477, 0x3314 },
	{ 0x478, 0x0421 },
	{ 0x479, 0x2D86 },
	{ 0x47A, 0x0421 },
	{ 0x47B, 0x2893 },
	{ 0x47C, 0x0421 },
	{ 0x47D, 0x203A },
	{ 0x47E, 0x0421 },
};

static const struct madera_hp_tuning cs47l92_hp_tuning[] = {
	{
		1400,
		cs47l92_low_impedance_patch,
		ARRAY_SIZE(cs47l92_low_impedance_patch),
	},
	{	2400,
		cs47l92_normal_impedance_patch,
		ARRAY_SIZE(cs47l92_normal_impedance_patch),
	},
	{	MADERA_HPDET_MAX_HOHM,
		cs47l92_high_impedance_patch,
		ARRAY_SIZE(cs47l92_high_impedance_patch),
	},
};

static ssize_t madera_extcon_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct madera_extcon *info = platform_get_drvdata(pdev);

	return scnprintf(buf, PAGE_SIZE, "%d\n",
			 info->madera->hp_impedance_x100[0]);
}

static DEVICE_ATTR(hp1_impedance, 0444, madera_extcon_show, NULL);

void madera_extcon_report(struct madera_extcon *info, int which, bool attached)
{
	int ret;
	dev_info(info->dev, "%s:enter",__func__);
	dev_info(info->dev, "Extcon report: %d is %s\n",
		which, attached ? "attached" : "removed");

	ret = extcon_set_state_sync(info->edev, which, attached);
	if (ret != 0)
		dev_info(info->dev, "Failed to report cable state: %d\n", ret);
	info->event_flag = which;
	if (IS_ENABLED(CONFIG_XRING_CODEC_MFD_CS47L92)) {
		switch (which) {
		case EXTCON_MECHANICAL:
			dev_info(info->dev, "EXTCON_MECHANICAL report\n");
			input_report_switch(info->input,
						SW_JACK_PHYSICAL_INSERT,
						attached);
			break;
		case EXTCON_JACK_HEADPHONE:
			dev_info(info->dev, "EXTCON_JACK_HEADPHONE report\n");
			input_report_switch(info->input,
						SW_HEADPHONE_INSERT,
						attached);
			break;
		case EXTCON_JACK_MICROPHONE:
			dev_info(info->dev, "EXTCON_JACK_MICROPHONE report\n");
			input_report_switch(info->input,
						SW_MICROPHONE_INSERT,
						attached);
			break;
		default:
			return;
		}

		input_sync(info->input);
	}
}
EXPORT_SYMBOL_GPL(madera_extcon_report);



static enum madera_accdet_mode madera_jds_get_mode(struct madera_extcon *info)
{
	if (info->state)
		return info->state->mode;
	else
		return MADERA_ACCDET_MODE_INVALID;
}

int madera_jds_set_state(struct madera_extcon *info,
			 const struct madera_jd_state *new_state)
{
	int ret = 0;
	dev_info(info->dev, "%s:enter==================",__func__);
	if (new_state != info->state) {
		if (info->state)
			info->state->stop(info);

		info->state = new_state;

		if (info->state) {
			ret = info->state->start(info);
			if (ret < 0)
				info->state = NULL;
		}
	}
	dev_info(info->dev, "%s:end==================",__func__);
	return ret;
}
EXPORT_SYMBOL_GPL(madera_jds_set_state);

static void madera_jds_reading(struct madera_extcon *info, int val)
{
	int ret;

	ret = info->state->reading(info, val);
	dev_info(info->dev, "madera_jds_reading finished, state.mode=%d, val = %d\n",
		 info->state->mode, val);

	if (ret == -EAGAIN && info->state->restart)
		info->state->restart(info);
}

static inline bool madera_jds_cancel_timeout(struct madera_extcon *info)
{
	return cancel_delayed_work_sync(&info->state_timeout_work);
}

static void madera_jds_start_timeout(struct madera_extcon *info)
{
	const struct madera_jd_state *state = info->state;

	if (!state)
		return;

	if (state->timeout_ms && state->timeout) {
		int ms = state->timeout_ms(info);

		schedule_delayed_work(&info->state_timeout_work,
					  msecs_to_jiffies(ms));
	}
}

static void madera_jds_timeout_work(struct work_struct *work)
{
	struct madera_extcon *info =
		container_of(work, struct madera_extcon,
				 state_timeout_work.work);

	mutex_lock(&info->lock);

	if (!info->state) {
		dev_warn(info->dev, "Spurious timeout in idle state\n");
	} else if (!info->state->timeout) {
		dev_warn(info->dev, "Spurious timeout state.mode=%d\n",
			 info->state->mode);
	} else {
		info->state->timeout(info);
		madera_jds_start_timeout(info);
	}

	mutex_unlock(&info->lock);
}

static void madera_extcon_hp_clamp(struct madera_extcon *info, bool clamp)
{
	struct madera *madera = info->madera;
	unsigned int mask = 0, ep_mask = 0, val = 0;
	int ret;

	snd_soc_dapm_mutex_lock(madera->dapm);

	switch (madera->type) {
	case CS47L85:
	case WM1840:
		break;
	case CS47L35:
		ep_mask = MADERA_EP_SEL_MASK;
		break;
	case CS47L15:
		ep_mask = MADERA_EP_SEL_MASK;
		/* Intentional enter */
		__attribute__((__fallthrough__));
	default:
		mask = MADERA_HPD_OVD_ENA_SEL_MASK;
		if (!clamp)
			val = MADERA_HPD_OVD_ENA_SEL_MASK;
		else
			val = 0;
		break;
	}

	madera->out_clamp[0] = clamp;

	/* Keep the HP output stages disabled while disabling the clamp */
	if (!clamp) {
		ret = regmap_update_bits(madera->regmap,
					 MADERA_OUTPUT_ENABLES_1,
					 ep_mask |
					 ((MADERA_OUT1L_ENA |
					  MADERA_OUT1R_ENA) <<
					 (2 * (info->pdata->output - 1))),
					 0);
		if (ret)
			dev_warn(info->dev,
				 "Failed to disable headphone outputs: %d\n",
				 ret);
	}

	dev_dbg(info->dev, "%s clamp mask=0x%x val=0x%x\n",
		clamp ? "Setting" : "Clearing", mask, val);

	switch (madera->type) {
	case CS47L35:
	case CS47L85:
	case WM1840:
		break;
	default:
		ret = regmap_update_bits(madera->regmap,
					 MADERA_HEADPHONE_DETECT_0,
					 MADERA_HPD_OVD_ENA_SEL_MASK,
					 val);
		if (ret)
			dev_warn(info->dev, "Failed to do clamp: %d\n", ret);
		break;
	}

	/* Restore the desired state when restoring the clamp */
	if (clamp) {
		if (ep_mask) {
			ret = regmap_update_bits(madera->regmap,
						 MADERA_OUTPUT_ENABLES_1,
						 ep_mask, madera->ep_sel);
			if (ret)
				dev_warn(info->dev,
					 "Failed to restore output demux: %d\n",
					 ret);
		}

		madera->out_shorted[0] = (madera->hp_impedance_x100[0] <=
					  info->hpdet_short_x100);

		if (!madera->out_shorted[0]) {
			ret = regmap_update_bits(madera->regmap,
						 MADERA_OUTPUT_ENABLES_1,
						 (MADERA_OUT1L_ENA |
						  MADERA_OUT1R_ENA) <<
						 (2 * (info->pdata->output - 1)),
						 madera->hp_ena);
			if (ret)
				dev_warn(info->dev,
					 "Failed to restore headphone outputs: %d\n",
					 ret);
		}
	}

	snd_soc_dapm_mutex_unlock(madera->dapm);
}

static const char *madera_extcon_get_micbias_src(struct madera_extcon *info)
{
	struct madera *madera = info->madera;
	unsigned int bias = info->micd_modes[info->micd_mode].bias;

	switch (madera->type) {
	case CS47L15:
		switch (bias) {
		case 0:
		case 1:
		case 2:
			return "MICBIAS1";
		default:
			return "MICVDD";
		}
		break;
	case CS47L35:
	case CS47L85:
	case WM1840:
		return NULL;
	case CS47L90:
	case CS47L91:
		switch (bias) {
		case 0:
		case 1:
		case 2:
		case 3:
			return "MICBIAS1";
		case 4:
		case 5:
		case 6:
		case 7:
			return "MICBIAS2";
		default:
			return "MICVDD";
		}
		break;
	case CS42L92:
	case CS47L92:
	case CS47L93:
		switch (bias) {
		case 0:
		case 1:
		case 2:
		case 3:
			return "MICBIAS1";
		case 4:
		case 5:
			return "MICBIAS2";
		default:
			return "MICVDD";
		}
		break;
	default:
		return NULL;
	}
}

static const char *madera_extcon_get_micbias(struct madera_extcon *info)
{
	struct madera *madera = info->madera;
	unsigned int bias = info->micd_modes[info->micd_mode].bias;

	switch (madera->type) {
	case CS47L15:
		switch (bias) {
		case 0:
			return "MICBIAS1A";
		case 1:
			return "MICBIAS1B";
		case 2:
			return "MICBIAS1C";
		default:
			return "MICVDD";
		}
	case CS47L35:
		switch (bias) {
		case 1:
			return "MICBIAS1A";
		case 2:
			return "MICBIAS1B";
		case 3:
			return "MICBIAS2A";
		default:
			return "MICVDD";
		}
	case CS47L85:
	case WM1840:
		switch (bias) {
		case 1:
			return "MICBIAS1";
		case 2:
			return "MICBIAS2";
		case 3:
			return "MICBIAS3";
		case 4:
			return "MICBIAS4";
		default:
			return "MICVDD";
		}
	case CS47L90:
	case CS47L91:
		switch (bias) {
		case 0:
			return "MICBIAS1A";
		case 1:
			return "MICBIAS1B";
		case 2:
			return "MICBIAS1C";
		case 3:
			return "MICBIAS1D";
		case 4:
			return "MICBIAS2A";
		case 5:
			return "MICBIAS2B";
		case 6:
			return "MICBIAS2C";
		case 7:
			return "MICBIAS2D";
		default:
			return "MICVDD";
		}
	case CS42L92:
	case CS47L92:
	case CS47L93:
		switch (bias) {
		case 0:
			return "MICBIAS1A";
		case 1:
			return "MICBIAS1B";
		case 2:
			return "MICBIAS1C";
		case 3:
			return "MICBIAS1D";
		case 4:
			return "MICBIAS2A";
		case 5:
			return "MICBIAS2B";
		default:
			return "MICVDD";
		}
	default:
		return NULL;
	}
}

static void madera_extcon_enable_micbias_pin(struct madera_extcon *info,
						 const char *widget)
{
	struct madera *madera = info->madera;
	struct snd_soc_dapm_context *dapm = madera->dapm;
	struct snd_soc_component *component = snd_soc_dapm_to_component(dapm);
	int ret;

	ret = snd_soc_component_force_enable_pin(component, widget);
	if (ret)
		dev_warn(info->dev, "Failed to enable %s: %d\n", widget, ret);

	snd_soc_dapm_sync(dapm);

	dev_dbg(info->dev, "Enabled %s\n", widget);
}

static void madera_extcon_disable_micbias_pin(struct madera_extcon *info,
						  const char *widget)
{
	struct madera *madera = info->madera;
	struct snd_soc_dapm_context *dapm = madera->dapm;
	struct snd_soc_component *component = snd_soc_dapm_to_component(dapm);
	int ret;

	ret = snd_soc_component_disable_pin(component, widget);
	if (ret)
		dev_warn(info->dev, "Failed to enable %s: %d\n", widget, ret);

	snd_soc_dapm_sync(dapm);

	dev_dbg(info->dev, "Disabled %s\n", widget);
}

static void madera_extcon_set_micd_bias(struct madera_extcon *info, bool enable)
{
	int old_bias = info->micd_bias.bias;
	int new_bias = info->micd_modes[info->micd_mode].bias;
	const char *widget;

	info->micd_bias.bias = new_bias;

	if ((new_bias == old_bias) && (info->micd_bias.enabled == enable))
		return;

	widget = madera_extcon_get_micbias_src(info);
	WARN_ON(!widget);

	if (info->micd_bias.enabled)
		madera_extcon_disable_micbias_pin(info, widget);

	if (enable)
		madera_extcon_enable_micbias_pin(info, widget);

	info->micd_bias.enabled = enable;
}

static void madera_extcon_enable_micbias(struct madera_extcon *info)
{
	struct madera *madera = info->madera;
	const char *widget = madera_extcon_get_micbias(info);

	switch (madera->type) {
	case CS47L35:
	case CS47L85:
	case WM1840:
		break;
	default:
		madera_extcon_set_micd_bias(info, true);
		break;
	}

	/*
	 * If forced we must manually control the pin state, otherwise
	 * the codec will manage this automatically
	 */
	if (info->pdata->micd_force_micbias)
		madera_extcon_enable_micbias_pin(info, widget);
}

static void madera_extcon_disable_micbias(struct madera_extcon *info)
{
	struct madera *madera = info->madera;
	const char *widget = madera_extcon_get_micbias(info);

	switch (madera->type) {
	case CS47L35:
	case CS47L85:
	case WM1840:
		break;
	default:
		madera_extcon_set_micd_bias(info, false);
		break;
	}

	/*
	 * If forced we must manually control the pin state, otherwise
	 * the codec will manage this automatically
	 */
	if (info->pdata->micd_force_micbias)
		madera_extcon_disable_micbias_pin(info, widget);
}

static void madera_extcon_set_mode(struct madera_extcon *info, int mode)
{
	struct madera *madera = info->madera;

	dev_dbg(info->dev,
		"set mic_mode[%d] src=0x%x gnd=0x%x bias=0x%x gpio=%d hp_gnd=%d\n",
		mode, info->micd_modes[mode].src, info->micd_modes[mode].gnd,
		info->micd_modes[mode].bias, info->micd_modes[mode].gpio,
		info->micd_modes[mode].hp_gnd);

	if (info->micd_pol_gpio)
		gpiod_set_value_cansleep(info->micd_pol_gpio,
					 info->micd_modes[mode].gpio);

	switch (madera->type) {
	case CS47L35:
	case CS47L85:
	case WM1840:
		regmap_update_bits(madera->regmap,
				   MADERA_MIC_DETECT_1_CONTROL_1,
				   MADERA_MICD_BIAS_SRC_MASK,
				   info->micd_modes[mode].bias <<
				   MADERA_MICD_BIAS_SRC_SHIFT);
		regmap_update_bits(madera->regmap,
				   MADERA_ACCESSORY_DETECT_MODE_1,
				   MADERA_ACCDET_SRC,
				   info->micd_modes[mode].src <<
				   MADERA_ACCDET_SRC_SHIFT);
		break;
	default:
		regmap_update_bits(madera->regmap,
				   MADERA_MIC_DETECT_1_CONTROL_1,
				   MADERA_MICD_BIAS_SRC_MASK,
				   info->micd_modes[mode].bias <<
				   MADERA_MICD_BIAS_SRC_SHIFT);
		regmap_update_bits(madera->regmap,
				   MADERA_MIC_DETECT_1_CONTROL_0,
				   MADERA_MICD1_SENSE_MASK,
				   info->micd_modes[mode].src <<
				   MADERA_MICD1_SENSE_SHIFT);
		regmap_update_bits(madera->regmap,
				   MADERA_MIC_DETECT_1_CONTROL_0,
				   MADERA_MICD1_GND_MASK,
				   info->micd_modes[mode].gnd <<
				   MADERA_MICD1_GND_SHIFT);
		regmap_update_bits(madera->regmap,
				   MADERA_HEADPHONE_DETECT_0,
				   MADERA_HPD_GND_SEL_MASK,
				   info->micd_modes[mode].gnd <<
				   MADERA_HPD_GND_SEL_SHIFT);
		regmap_update_bits(madera->regmap,
				   MADERA_OUTPUT_PATH_CONFIG_1 +
				   (8 * (info->pdata->output - 1)),
				   MADERA_HP1_GND_SEL_MASK,
				   info->micd_modes[mode].hp_gnd <<
				   MADERA_HP1_GND_SEL_SHIFT);
		break;
	}

	info->micd_mode = mode;
}

// static void madera_extcon_next_mode(struct madera_extcon *info)
// {
// 	int old_mode = info->micd_mode;
// 	int new_mode;
// 	bool change_bias = false;

// 	new_mode = (old_mode + 1) % info->num_micd_modes;

// 	dev_dbg(info->dev, "change micd mode %d->%d (bias %d->%d)\n",
// 		old_mode, new_mode,
// 		info->micd_modes[old_mode].bias,
// 		info->micd_modes[new_mode].bias);

// 	if (info->micd_modes[old_mode].bias !=
// 	    info->micd_modes[new_mode].bias) {
// 		change_bias = true;

// 		madera_extcon_disable_micbias(info);
// 	}

// 	madera_extcon_set_mode(info, new_mode);

// 	if (change_bias)
// 		madera_extcon_enable_micbias(info);
// }

static int madera_micd_adc_read(struct madera_extcon *info)
{
	struct madera *madera = info->madera;
	unsigned int val = 0;
	int ret;

	/* Must disable MICD before we read the ADCVAL */
	ret = regmap_update_bits(madera->regmap, MADERA_MIC_DETECT_1_CONTROL_1,
				 MADERA_MICD_ENA, 0);
	if (ret) {
		dev_err(info->dev, "Failed to disable MICD: %d\n", ret);
		return ret;
	}

	ret = regmap_read(madera->regmap, MADERA_MIC_DETECT_1_CONTROL_4, &val);
	if (ret) {
		dev_err(info->dev, "Failed to read MICDET_ADCVAL: %d\n", ret);
		return ret;
	}

	dev_dbg(info->dev, "MICDET_ADCVAL: 0x%x\n", val);

	val &= MADERA_MICDET_ADCVAL_MASK;
	if (val < ARRAY_SIZE(madera_micd_levels))
		val = madera_micd_levels[val];
	else
		val = INT_MAX;

	return val;
}

static int madera_micd_read(struct madera_extcon *info)
{
	struct madera *madera = info->madera;
	unsigned int val = 0;
	int ret, i;

	for (i = 0; i < 10 && !(val & MADERA_MICD_LVL_0_TO_8); i++) {
		ret = regmap_read(madera->regmap,
				  MADERA_MIC_DETECT_1_CONTROL_3, &val);
		if (ret) {
			dev_err(info->dev,
				"Failed to read MICDET: %d\n", ret);
			return ret;
		}

		dev_dbg(info->dev, "MICDET: 0x%x\n", val);

		if (!(val & MADERA_MICD_VALID)) {
			dev_warn(info->dev,
				 "Microphone detection state invalid\n");
			return -EINVAL;
		}
	}

	if (i == 10 && !(val & MADERA_MICD_LVL_0_TO_8)) {
		dev_warn(info->dev, "Failed to get valid MICDET value\n");
		return -EINVAL;
	}

	if (!(val & MADERA_MICD_STS)) {
		val = INT_MAX;
	} else if (!(val & MADERA_MICD_LVL_0_TO_7)) {
		val = madera_micd_levels[ARRAY_SIZE(madera_micd_levels) - 1];
	} else {
		int lvl;

		lvl = (val & MADERA_MICD_LVL_MASK) >> MADERA_MICD_LVL_SHIFT;
		lvl = ffs(lvl) - 1;

		if (lvl < info->num_micd_ranges) {
			val = info->micd_ranges[lvl].max;
		} else {
			i = ARRAY_SIZE(madera_micd_levels) - 2;
			val = madera_micd_levels[i];
		}
	}

	return val;
}

static void madera_extcon_notify_micd(const struct madera_extcon *info,
					  bool present,
					  unsigned int impedance)
{
	struct madera_micdet_notify_data data;

	data.present = present;
	data.impedance_x100 = madera_ohm_to_hohm(impedance);
	data.out_num = info->pdata->output;

	blocking_notifier_call_chain(&info->madera->notifier,
					 MADERA_NOTIFY_MICDET, &data);
}

static int madera_hpdet_calc_calibration(const struct madera_extcon *info,
			int dacval,
			const struct madera_hpdet_trims *trims,
			const struct madera_hpdet_calibration_data *calib)
{
	int grad_x4 = trims->grad_x4;
	int off_x4 = trims->off_x4;
	s64 val = dacval;
	s64 n;

	val = (val * 1000000) + calib->dacval_adjust;
	val = div64_s64(val, calib->C2);

	n = div_s64(1000000000000LL, calib->C3 +
			((calib->C4_x_C3 * grad_x4) / 4));
	n = val - n;
	if (n <= 0)
		return MADERA_HPDET_MAX_HOHM;

	val = calib->C0 + ((calib->C1 * off_x4) / 4);
	val *= 1000000;

	val = div_s64(val, n);
	val -= calib->C5;

	/* Round up and divide to get hundredths of an ohm */
	val += 500000;
	val = div_s64(val, 10000);

	if (val < 0)
		return 0;
	else if (val > MADERA_HPDET_MAX_HOHM)
		return MADERA_HPDET_MAX_HOHM;

	return (int)val;
}

static int madera_hpdet_calibrate(struct madera_extcon *info,
				  unsigned int range, unsigned int *ohms_x100)
{
	struct madera *madera = info->madera;
	unsigned int dacval, dacval_down;
	int ret;

	ret = regmap_read(madera->regmap, MADERA_HEADPHONE_DETECT_3, &dacval);
	if (ret) {
		dev_err(info->dev, "Failed to read HP DACVAL: %d\n", ret);
		return -EAGAIN;
	}

	dacval = (dacval >> MADERA_HP_DACVAL_SHIFT) & MADERA_HP_DACVAL_MASK;

	switch (madera->type) {
	case CS47L35:
	case CS47L85:
	case WM1840:
		ret = regmap_read(madera->regmap, MADERA_HEADPHONE_DETECT_5,
				  &dacval_down);
		if (ret) {
			dev_err(info->dev,
				"Failed to read HP DACVAL_down: %d\n", ret);
			return -EAGAIN;
		}

		dacval_down = (dacval_down >> MADERA_HP_DACVAL_DOWN_SHIFT) &
				MADERA_HP_DACVAL_DOWN_MASK;

		dacval = (dacval + dacval_down) / 2;
		break;
	default:
		break;
	}

	dev_dbg(info->dev,
		"hpdet_d calib range %d dac %d\n", range, dacval);

	*ohms_x100 = madera_hpdet_calc_calibration(info, dacval,
						   &info->hpdet_trims[range],
						   &info->hpdet_ranges[range]);
	return 0;
}

static int madera_hpdet_read(struct madera_extcon *info)
{
	struct madera *madera = info->madera;
	unsigned int val, range, sense_pin, ohms_x100;
	int ret;
	bool is_jdx_micdetx_pin = false;
	int hpdet_ext_res_x100;

	dev_dbg(info->dev, "HPDET read\n");

	ret = regmap_read(madera->regmap, MADERA_HEADPHONE_DETECT_2, &val);
	if (ret) {
		dev_err(info->dev, "Failed to read HPDET status: %d\n", ret);
		return ret;
	}

	if (!(val & MADERA_HP_DONE_MASK)) {
		dev_warn(info->dev, "HPDET did not complete: %x\n", val);
		return -EAGAIN;
	}

	switch (madera->type) {
	case CS47L35:
	case CS47L85:
	case WM1840:
		break;
	default:
		regmap_read(madera->regmap, MADERA_HEADPHONE_DETECT_0,
				&sense_pin);
		sense_pin = (sense_pin & MADERA_HPD_SENSE_SEL_MASK) >>
				MADERA_HPD_SENSE_SEL_SHIFT;

		switch (sense_pin) {
		case MADERA_HPD_SENSE_HPDET1:
		case MADERA_HPD_SENSE_HPDET2:
			is_jdx_micdetx_pin = false;
			break;
		default:
			dev_dbg(info->dev, "is_jdx_micdetx_pin\n");
			is_jdx_micdetx_pin = true;
		}
		break;
	}

	val &= MADERA_HP_LVL_MASK;
	/* The value is in 0.5 ohm increments, get it in hundredths */
	ohms_x100 = val * 50;

	if (is_jdx_micdetx_pin)
		goto done;

	regmap_read(madera->regmap, MADERA_HEADPHONE_DETECT_1, &range);
	range = (range & MADERA_HP_IMPEDANCE_RANGE_MASK) >>
		MADERA_HP_IMPEDANCE_RANGE_SHIFT;

	/* Skip up a range, or report? */
	if (range < info->num_hpdet_ranges - 1 &&
		((val / 2) >= info->hpdet_ranges[range].max)) {
		range++;
		dev_dbg(info->dev, "Moving to HPDET range %d-%d\n",
			info->hpdet_ranges[range].min,
			info->hpdet_ranges[range].max);

		regmap_update_bits(madera->regmap,
				   MADERA_HEADPHONE_DETECT_1,
				   MADERA_HP_IMPEDANCE_RANGE_MASK,
				   range <<
				   MADERA_HP_IMPEDANCE_RANGE_SHIFT);
		return -EAGAIN;
	}

	if (info->hpdet_trims) {
		/* Perform calibration */
		ret = madera_hpdet_calibrate(info, range, &ohms_x100);
		if (ret)
			return ret;
	} else {
		/* Use uncalibrated reading */
		if (range && ((val / 2) < info->hpdet_ranges[range].min)) {
			dev_dbg(info->dev,
				"Reporting range boundary %d\n",
				info->hpdet_ranges[range].min);
			ohms_x100 =
				  madera_ohm_to_hohm(info->hpdet_ranges[range].min);
		}
	}

	hpdet_ext_res_x100 = info->pdata->hpdet_ext_res_x100;
	if (hpdet_ext_res_x100) {
		if (hpdet_ext_res_x100 >= ohms_x100) {
			dev_dbg(info->dev,
				"External resistor (%d.%02d) >= measurement (%d.00)\n",
				hpdet_ext_res_x100 / 100,
				hpdet_ext_res_x100 % 100,
				val);
			ohms_x100 = 0;	/* treat as a short */
		} else {
			dev_dbg(info->dev,
				"Compensating for external %d.%02d ohm resistor\n",
				hpdet_ext_res_x100 / 100,
				hpdet_ext_res_x100 % 100);

			ohms_x100 -= hpdet_ext_res_x100;
		}
	}

done:
	dev_dbg(info->dev, "HP impedance %d.%02d ohms\n",
		ohms_x100 / 100, ohms_x100 % 100);

	return (int)ohms_x100;
}

static int madera_tune_headphone(struct madera_extcon *info, int reading)
{
	struct madera *madera = info->madera;
	const struct madera_hp_tuning *tuning;
	int n_tunings;
	int i, ret;

	switch (madera->type) {
	case CS47L15:
		tuning = cs47l15_hp_tuning;
		n_tunings = ARRAY_SIZE(cs47l15_hp_tuning);
		break;
	case CS47L35:
		tuning = cs47l35_hp_tuning;
		n_tunings = ARRAY_SIZE(cs47l35_hp_tuning);
		break;
	case CS47L85:
	case WM1840:
		tuning = cs47l85_hp_tuning;
		n_tunings = ARRAY_SIZE(cs47l85_hp_tuning);
		break;
	case CS47L90:
	case CS47L91:
		tuning = cs47l90_hp_tuning;
		n_tunings = ARRAY_SIZE(cs47l90_hp_tuning);
		break;
	case CS42L92:
	case CS47L92:
	case CS47L93:
		tuning = cs47l92_hp_tuning;
		n_tunings = ARRAY_SIZE(cs47l92_hp_tuning);
		break;
	default:
		return 0;
	}

	if (reading <= info->hpdet_short_x100) {
		/* Headphones are always off here so just mark them */
		dev_warn(info->dev, "Possible HP short, disabling\n");
		return 0;
	}

	if (reading == MADERA_HP_Z_OPEN) {
		if (info->hp_tuning_level == 1)
			return 0;

		dev_dbg(info->dev, "No jack: Setting tuning level 1\n");

		info->hp_tuning_level = 1;

		ret = regmap_multi_reg_write(madera->regmap,
						 tuning[1].patch,
						 tuning[1].patch_len);
		return ret;
	}

	/*
	 * Check for tuning, we don't need to compare against the last
	 * tuning entry because we always select that if reading is not
	 * in range of the lower tunings
	 */
	for (i = 0; i < n_tunings - 1; ++i) {
		if (reading <= tuning[i].max_hohm)
			break;
	}

	if (info->hp_tuning_level != i) {
		dev_dbg(info->dev, "New tuning level %d\n", i);

		info->hp_tuning_level = i;

		ret = regmap_multi_reg_write(madera->regmap,
						 tuning[i].patch,
						 tuning[i].patch_len);
		if (ret) {
			dev_err(info->dev,
				"Failed to apply HP tuning %d\n", ret);
			return ret;
		}
	}

	return 0;
}

void madera_set_headphone_imp(struct madera_extcon *info, int ohms_x100)
{
	struct madera *madera = info->madera;
	struct madera_hpdet_notify_data data;

	madera->hp_impedance_x100[0] = ohms_x100;

	data.impedance_x100 = ohms_x100;
	blocking_notifier_call_chain(&info->madera->notifier,
					 MADERA_NOTIFY_HPDET, &data);

	madera_tune_headphone(info, ohms_x100);
}
EXPORT_SYMBOL_GPL(madera_set_headphone_imp);

static void madera_hpdet_start_micd(struct madera_extcon *info)
{
	struct madera *madera = info->madera;

	regmap_update_bits(madera->regmap, MADERA_IRQ1_MASK_6,
			   MADERA_IM_MICDET1_EINT1_MASK,
			   MADERA_IM_MICDET1_EINT1);
	regmap_update_bits(madera->regmap, MADERA_MIC_DETECT_1_CONTROL_0,
			   MADERA_MICD1_ADC_MODE_MASK,
			   MADERA_MICD1_ADC_MODE_MASK);
	regmap_update_bits(madera->regmap, MADERA_MIC_DETECT_1_CONTROL_1,
			   MADERA_MICD_BIAS_STARTTIME_MASK |
			   MADERA_MICD_RATE_MASK |
			   MADERA_MICD_DBTIME_MASK |
			   MADERA_MICD_ENA, MADERA_MICD_ENA);
}

static void madera_hpdet_stop_micd(struct madera_extcon *info)
{
	struct madera *madera = info->madera;
	unsigned int start_time = 1, dbtime = 1, rate = 1;

	if (info->pdata->micd_bias_start_time)
		start_time = info->pdata->micd_bias_start_time;

	if (info->pdata->micd_rate)
		rate = info->pdata->micd_rate;

	if (info->pdata->micd_dbtime)
		dbtime = info->pdata->micd_dbtime;

	regmap_update_bits(madera->regmap, MADERA_MIC_DETECT_1_CONTROL_1,
			   MADERA_MICD_BIAS_STARTTIME_MASK |
			   MADERA_MICD_RATE_MASK |
			   MADERA_MICD_DBTIME_MASK |
			   MADERA_MICD_ENA,
			   start_time << MADERA_MICD_BIAS_STARTTIME_SHIFT |
			   rate << MADERA_MICD_RATE_SHIFT |
			   dbtime << MADERA_MICD_DBTIME_SHIFT);

	usleep_range(100, 400);

	/* Clear any spurious IRQs that have happened */
	regmap_write(madera->regmap, MADERA_IRQ1_STATUS_6,
			 MADERA_MICDET1_EINT1);
	regmap_update_bits(madera->regmap, MADERA_IRQ1_MASK_6,
			   MADERA_IM_MICDET1_EINT1_MASK, 0);
}

int madera_hpdet_start(struct madera_extcon *info)
{
	struct madera *madera = info->madera;
	int ret;
	unsigned int hpd_sense, hpd_clamp, val, hpd_gnd;
	pr_info("%s:enter",__func__);
	dev_dbg(info->dev, "Starting HPDET\n");

	/* If we specified to assume a fixed impedance skip HPDET */
	if (info->pdata->fixed_hpdet_imp_x100) {
		madera_set_headphone_imp(info,
					 info->pdata->fixed_hpdet_imp_x100);
		ret = -EEXIST;
		goto skip;
	}

	/* Make sure we keep the device enabled during the measurement */
	pm_runtime_get_sync(info->dev);

	switch (madera->type) {
	case CS47L35:
	case CS47L85:
	case WM1840:
		madera_extcon_hp_clamp(info, false);
		ret = regmap_update_bits(madera->regmap,
					 MADERA_ACCESSORY_DETECT_MODE_1,
					 MADERA_ACCDET_MODE_MASK,
					 info->state->mode);
		if (ret) {
			dev_err(info->dev,
				"Failed to set HPDET mode (%d): %d\n",
				info->state->mode,
				ret);
			goto err;
		}
		break;
	default:
		if (info->state->mode == MADERA_ACCDET_MODE_HPL) {
			hpd_clamp = info->pdata->hpd_pins[0];
			hpd_sense = info->pdata->hpd_pins[1];
		} else {
			hpd_clamp = info->pdata->hpd_pins[2];
			hpd_sense = info->pdata->hpd_pins[3];
		}

		hpd_gnd = info->micd_modes[info->micd_mode].gnd;

		val = (hpd_sense << MADERA_HPD_SENSE_SEL_SHIFT) |
				(hpd_clamp << MADERA_HPD_OUT_SEL_SHIFT) |
				(hpd_sense << MADERA_HPD_FRC_SEL_SHIFT) |
				(hpd_gnd << MADERA_HPD_GND_SEL_SHIFT);

		ret = regmap_update_bits(madera->regmap,
					 MADERA_MIC_DETECT_1_CONTROL_0,
					 MADERA_MICD1_GND_MASK,
					 hpd_gnd << MADERA_MICD1_GND_SHIFT);
		if (ret) {
			dev_err(madera->dev, "Failed to set MICD_GND: %d\n",
				ret);
			goto err;
		}

		ret = regmap_update_bits(madera->regmap,
					 MADERA_HEADPHONE_DETECT_0,
					 MADERA_HPD_GND_SEL_MASK |
					 MADERA_HPD_SENSE_SEL_MASK |
					 MADERA_HPD_FRC_SEL_MASK |
					 MADERA_HPD_OUT_SEL_MASK,
					 val);
		if (ret) {
			dev_err(info->dev,
				"Failed to set HPDET sense: %d\n", ret);
			goto err;
		}
		madera_extcon_hp_clamp(info, false);
		madera_hpdet_start_micd(info);
		break;
	}

	ret = regmap_update_bits(madera->regmap, MADERA_HEADPHONE_DETECT_1,
				 MADERA_HP_POLL, MADERA_HP_POLL);
	if (ret) {
		dev_err(info->dev, "Can't start HPDET measurement: %d\n", ret);
		goto err;
	}

	return 0;

err:
	madera_extcon_hp_clamp(info, true);

	pm_runtime_put_autosuspend(info->dev);

skip:
	return ret;
}
EXPORT_SYMBOL_GPL(madera_hpdet_start);

void madera_hpdet_restart(struct madera_extcon *info)
{
	struct madera *madera = info->madera;

	/* Reset back to starting range */
	regmap_update_bits(madera->regmap, MADERA_MIC_DETECT_1_CONTROL_1,
			   MADERA_MICD_ENA_MASK, 0);

	regmap_update_bits(madera->regmap, MADERA_HEADPHONE_DETECT_1,
			   MADERA_HP_IMPEDANCE_RANGE_MASK | MADERA_HP_POLL,
			   info->hpdet_init_range <<
			   MADERA_HP_IMPEDANCE_RANGE_SHIFT);

	switch (madera->type) {
	case CS47L35:
	case CS47L85:
	case WM1840:
		break;
	default:
		regmap_update_bits(madera->regmap,
				   MADERA_MIC_DETECT_1_CONTROL_1,
				   MADERA_MICD_ENA_MASK, MADERA_MICD_ENA);
		break;
	}

	regmap_update_bits(madera->regmap, MADERA_HEADPHONE_DETECT_1,
			   MADERA_HP_POLL, MADERA_HP_POLL);
}
EXPORT_SYMBOL_GPL(madera_hpdet_restart);

static int madera_hpdet_wait(struct madera_extcon *info)
{
	struct madera *madera = info->madera;
	unsigned int val;
	int i, ret;

	for (i = 0; i < MADERA_HPDONE_PROBE_COUNT; i++) {
		ret = regmap_read(madera->regmap, MADERA_HEADPHONE_DETECT_2,
				  &val);
		if (ret) {
			dev_err(madera->dev, "Failed to read HPDET state: %d\n",
				ret);
			return ret;
		}

		if (val & MADERA_HP_DONE_MASK)
			return 0;

		msleep(MADERA_HPDONE_PROBE_INTERVAL_MS);
	}

	dev_err(madera->dev, "HPDET did not appear to complete\n");

	return -ETIMEDOUT;
}

void madera_hpdet_stop(struct madera_extcon *info)
{
	struct madera *madera = info->madera;

	dev_dbg(info->dev, "Stopping HPDET\n");

	/*
	 * If the jack was removed we abort this state.
	 * Ensure that the detect hardware has returned to idle
	 */
	madera_hpdet_wait(info);

	/* Reset back to starting range */
	madera_hpdet_stop_micd(info);

	regmap_update_bits(madera->regmap, MADERA_HEADPHONE_DETECT_1,
			   MADERA_HP_IMPEDANCE_RANGE_MASK | MADERA_HP_POLL,
			   info->hpdet_init_range <<
			   MADERA_HP_IMPEDANCE_RANGE_SHIFT);

	switch (madera->type) {
	case CS47L35:
	case CS47L85:
	case WM1840:
		/* Reset to default mode */
		regmap_update_bits(madera->regmap,
				   MADERA_ACCESSORY_DETECT_MODE_1,
				   MADERA_ACCDET_MODE_MASK, 0);
		break;
	default:
		break;
	}

	madera_extcon_hp_clamp(info, true);

	pm_runtime_mark_last_busy(info->dev);
	pm_runtime_put_autosuspend(info->dev);
}
EXPORT_SYMBOL_GPL(madera_hpdet_stop);

int madera_hpdet_reading(struct madera_extcon *info, int val)
{
	pr_info("%s:enter",__func__);
	dev_info(info->dev, "Reading HPDET %d\n", val);

	if (val < 0)
		return val;

	madera_set_headphone_imp(info, val);

	// madera_extcon_report(info, EXTCON_JACK_HEADPHONE, true);

	if (info->have_mic)
		madera_jds_set_state(info, &madera_micd_button);
	else
		madera_jds_set_state(info, &madera_micd_button);

	dev_info(info->dev, "Reading HPDET end %d\n", val);
	info->jd_start_time = ktime_get();
	return 0;
}
EXPORT_SYMBOL_GPL(madera_hpdet_reading);

int madera_micd_start(struct madera_extcon *info)
{
	struct madera *madera = info->madera;
	int ret;
	unsigned int micd_mode;
	pr_info("%s:enter",__func__);
	/* Microphone detection can't use idle mode */
	pm_runtime_get_sync(info->dev);

	dev_dbg(info->dev, "Disabling MICD_OVD\n");
	regmap_update_bits(madera->regmap,
			   MADERA_MICD_CLAMP_CONTROL,
			   MADERA_MICD_CLAMP_OVD_MASK, 0);

	ret = regulator_enable(info->micvdd);
	if (ret)
		dev_err(info->dev, "Failed to enable MICVDD: %d\n", ret);

#ifdef MADERA_FPGA_DEBUG
	ret = regmap_write(madera->regmap, MADERA_MICD_CLAMP_CONTROL, 0x0);
	madera_configure_power_fpga(madera);
#endif

	switch (madera->type) {
	case CS47L35:
	case CS47L85:
	case WM1840:
		regmap_update_bits(madera->regmap,
				   MADERA_ACCESSORY_DETECT_MODE_1,
				   MADERA_ACCDET_MODE_MASK, info->state->mode);
		break;
	default:
		if (info->state->mode == MADERA_ACCDET_MODE_ADC)
			micd_mode = MADERA_MICD1_ADC_MODE_MASK;
		else
			micd_mode = 0;

		regmap_update_bits(madera->regmap,
				   MADERA_MIC_DETECT_1_CONTROL_0,
				   MADERA_MICD1_ADC_MODE_MASK, micd_mode);
		break;
	}

	madera_extcon_enable_micbias(info);

	regmap_update_bits(madera->regmap, MADERA_MIC_DETECT_1_CONTROL_1,
			   MADERA_MICD_ENA, MADERA_MICD_ENA);

	return 0;
}
EXPORT_SYMBOL_GPL(madera_micd_start);

void madera_micd_stop(struct madera_extcon *info)
{
	struct madera *madera = info->madera;
	pr_info("%s:enter",__func__);
	regmap_update_bits(madera->regmap, MADERA_MIC_DETECT_1_CONTROL_1,
			   MADERA_MICD_ENA, 0);

	madera_extcon_disable_micbias(info);

	switch (madera->type) {
	case CS47L35:
	case CS47L85:
	case WM1840:
		/* Reset to default mode */
		regmap_update_bits(madera->regmap,
				   MADERA_ACCESSORY_DETECT_MODE_1,
				   MADERA_ACCDET_MODE_MASK, 0);
		break;
	default:
		break;
	}

	regulator_disable(info->micvdd);

	dev_dbg(info->dev, "Enabling MICD_OVD\n");
	regmap_update_bits(madera->regmap, MADERA_MICD_CLAMP_CONTROL,
			   MADERA_MICD_CLAMP_OVD_MASK, MADERA_MICD_CLAMP_OVD);

	pm_runtime_mark_last_busy(info->dev);
	pm_runtime_put_autosuspend(info->dev);
}
EXPORT_SYMBOL_GPL(madera_micd_stop);

static void madera_micd_restart(struct madera_extcon *info)
{
	struct madera *madera = info->madera;
	pr_info("%s:enter",__func__);
	regmap_update_bits(madera->regmap, MADERA_MIC_DETECT_1_CONTROL_1,
			   MADERA_MICD_ENA, 0);
	regmap_update_bits(madera->regmap, MADERA_MIC_DETECT_1_CONTROL_1,
			   MADERA_MICD_ENA, MADERA_MICD_ENA);
}

static int madera_micd_button_debounce(struct madera_extcon *info, int val)
{
	int debounce_lim = info->pdata->micd_manual_debounce;

	if (debounce_lim) {
		if (info->micd_debounce != val)
			info->micd_count = 0;

		info->micd_debounce = val;
		info->micd_count++;

		if (info->micd_count == debounce_lim) {
			info->micd_count = 0;
			if (val == info->micd_res_old)
				return 0;

			info->micd_res_old = val;
		} else {
			dev_dbg(info->dev, "Software debounce: %d,%x\n",
				info->micd_count, val);
			madera_micd_restart(info);
			return -EAGAIN;
		}
	}

	return 0;
}

static int madera_micd_button_process(struct madera_extcon *info, int val)
{
	int i, key=0;
	dev_info(info->dev, "%s:enter",__func__);
	if (val < MADERA_MICROPHONE_MIN_OHM) {
		dev_info(info->dev, "Mic button detected\n");

		for (i = 0; i < info->num_micd_ranges; i++)
			input_report_key(info->input,
					 info->micd_ranges[i].key, 0);

		for (i = 0; i < info->num_micd_ranges; i++) {
			if (val <= info->micd_ranges[i].max) {
				key = info->micd_ranges[i].key;
				dev_info(info->dev, "Key %d down\n", key);
				input_report_key(info->input, key, 1);
#ifdef HEADSET_STATUS_RECORD
				switch (key)
				{
				case KEY_MEDIA://KEY_PLAYPAUSE
					dev_info(info->dev, "Key down KEY_MEDIA\n");
					add_headset_event(HEADSET_STATUS_RECORD_INDEX_KEY_MEDIA,HEADSET_EVENT_KEY_MEDIA_DOWN);
					break;
				case MEDIA_NEXT_SCAN_CODE://KEY_VOLUMEDOWN
					dev_info(info->dev, "Key down KEY_VOLUMEDOWN\n");
					add_headset_event(HEADSET_STATUS_RECORD_INDEX_KEY_NEXT,HEADSET_EVENT_KEY_NEXT_DOWN);
					break;
				case MEDIA_PREVIOUS_SCAN_CODE://KEY_VOLUMEUP
					dev_info(info->dev, "Key down KEY_VOLUMEUP\n");
					add_headset_event(HEADSET_STATUS_RECORD_INDEX_KEY_PREVIOUS,HEADSET_EVENT_KEY_PREVIOUS_DOWN);
					break;
				default:
					dev_info(info->dev, "Key down err!!!!!!\n");
					break;
				}
#endif
				input_sync(info->input);
				break;
			}
		}

		if (i == info->num_micd_ranges)
			dev_info(info->dev,
				 "Button level %u out of range\n", val);
	} else {
		dev_info(info->dev, "Mic button released\n");

		for (i = 0; i < info->num_micd_ranges; i++){
			input_report_key(info->input,
					 info->micd_ranges[i].key, 0);
#ifdef HEADSET_STATUS_RECORD
			key = info->micd_ranges[i].key;
			switch (key)
			{
			case KEY_MEDIA://KEY_PLAYPAUSE
				dev_info(info->dev, "Key up KEY_MEDIA\n");
				add_headset_event(HEADSET_STATUS_RECORD_INDEX_KEY_MEDIA,HEADSET_EVENT_KEY_MEDIA_UP);
				break;
			case MEDIA_NEXT_SCAN_CODE://KEY_VOLUMEDOWN
				dev_info(info->dev, "Key up KEY_VOLUMEDOWN\n");
				add_headset_event(HEADSET_STATUS_RECORD_INDEX_KEY_NEXT,HEADSET_EVENT_KEY_NEXT_UP);
				break;
			case MEDIA_PREVIOUS_SCAN_CODE://KEY_VOLUMEUP
				dev_info(info->dev, "Key up KEY_VOLUMEUP\n");
				add_headset_event(HEADSET_STATUS_RECORD_INDEX_KEY_PREVIOUS,HEADSET_EVENT_KEY_PREVIOUS_UP);
				break;
			default:
				dev_info(info->dev, "Key up err!!!!!!\n");
				break;
			}
#endif
		}
		input_sync(info->input);
	}

	return 0;
}

int madera_micd_button_reading(struct madera_extcon *info, int val)
{
	int ret;
	unsigned int ohms;
        struct madera *madera = info->madera;
	pr_info("%s:enter",__func__);
	if (val < 0)
		return val;

	ohms = madera_hohm_to_ohm((unsigned int)val);

	//detect mic open circuit, mute headset mic due to pop suppress
	if((info->have_mic) && (ohms == INT_MAX)) {
		pr_info("%s:when headset mic is detect open circuit, mute it",__func__);
		regmap_update_bits(madera->regmap, MADERA_ADC_DIGITAL_VOLUME_1L,
					  MADERA_IN1L_MUTE | MADERA_IN_VU,
					  MADERA_IN1L_MUTE | MADERA_IN_VU);
	}

	dev_info(info->dev, "ohms: %d\n", ohms);

	ret = madera_micd_button_debounce(info, ohms);
	if (ret < 0)
		return ret;
	info->btd_start_time = ktime_get();
	//if insert a jack meanwhile button is pushed, ignore this button
	if (ktime_us_delta(info->btd_start_time, info->jd_start_time) < MIN_BUTTON_DETECT_DELAY_US)
		return 0;
	return madera_micd_button_process(info, ohms);
}
EXPORT_SYMBOL_GPL(madera_micd_button_reading);

int madera_micd_mic_start(struct madera_extcon *info)
{
	int ret;
	pr_info("%s:enter",__func__);
	info->detecting = true;

	ret = regulator_allow_bypass(info->micvdd, false);
	if (ret)
		dev_err(info->dev, "Failed to regulate MICVDD: %d\n", ret);

	return madera_micd_start(info);
}
EXPORT_SYMBOL_GPL(madera_micd_mic_start);

void madera_micd_mic_stop(struct madera_extcon *info)
{
	int ret;
	pr_info("%s:enter",__func__);

	madera_micd_stop(info);

	ret = regulator_allow_bypass(info->micvdd, true);
	if (ret)
		dev_err(info->dev, "Failed to bypass MICVDD: %d\n", ret);

	info->detecting = false;
}
EXPORT_SYMBOL_GPL(madera_micd_mic_stop);

int madera_micd_mic_reading(struct madera_extcon *info, int val)
{
	unsigned int ohms;
	pr_info("%s:enter",__func__);
	if (val < 0)
		return val;

	ohms = madera_hohm_to_ohm((unsigned int)val);

	dev_info(info->dev, "ohms: %d\n", ohms);

	/* Due to jack detect this should never happen */
	if (ohms > MADERA_MICROPHONE_MAX_OHM) {
		dev_warn(info->dev, "Detected open circuit\n");
		//info->have_mic = info->pdata->micd_open_circuit_declare;
		goto done;
	}

	/* If we got a high impedence we should have a headset, report it.
	if (ohms >= MADERA_MICROPHONE_MIN_OHM) {
		dev_dbg(info->dev, "Detected headset\n");
		info->have_mic = true;
		goto done;
	}
	 */
	/*
	 * If we detected a lower impedence during initial startup
	 * then we probably have the wrong polarity, flip it.  Don't
	 * do this for the lowest impedences to speed up detection of
	 * plain headphones.  If both polarities report a low
	 * impedence then give up and report headphones.

	if (ohms > info->micd_ranges[0].max &&
		info->num_micd_modes > 1) {
		if (info->jack_flips >= info->num_micd_modes * 10) {
			dev_dbg(info->dev, "Detected HP/line\n");
			goto done;
		} else {
			madera_extcon_next_mode(info);

			info->jack_flips++;

			return -EAGAIN;
		}
	}
	*/
	/*
	 * If we're still detecting and we detect a short then we've
	 * got a headphone.
	 */

//	dev_dbg(info->dev, "Headphone detected\n");

done:
	pm_runtime_mark_last_busy(info->dev);

	if (info->pdata->hpdet_channel)
		madera_jds_set_state(info, &madera_hpdet_right);
	else
		madera_jds_set_state(info, &madera_hpdet_left);
	
	madera_extcon_report(info, EXTCON_JACK_MICROPHONE, info->have_mic);

	madera_extcon_notify_micd(info, info->have_mic, ohms);

	return 0;
}
EXPORT_SYMBOL_GPL(madera_micd_mic_reading);

int madera_micd_mic_timeout_ms(struct madera_extcon *info)
{
	if (info->pdata->micd_timeout_ms)
		return info->pdata->micd_timeout_ms;
	else
		return MADERA_DEFAULT_MICD_TIMEOUT_MS;
}
EXPORT_SYMBOL_GPL(madera_micd_mic_timeout_ms);

void madera_micd_mic_timeout(struct madera_extcon *info)
{
	int ret;

	dev_dbg(info->dev, "MICD timed out, reporting HP\n");

	if (info->pdata->hpdet_channel) {
		dev_dbg(info->dev, "madera_hpdet_right start\n");
		ret = madera_jds_set_state(info, &madera_hpdet_right);
	} else {
		dev_dbg(info->dev, "madera_hpdet_left start\n");
		ret = madera_jds_set_state(info, &madera_hpdet_left);
	}

	if (ret < 0)
		madera_extcon_report(info, EXTCON_JACK_MICROPHONE, false);
}
EXPORT_SYMBOL_GPL(madera_micd_mic_timeout);
/*
static int madera_jack_present(struct madera_extcon *info,
				   unsigned int *jack_val)
{
	struct madera *madera = info->madera;
	unsigned int present, val;
	int ret;
	pr_info("%s:enter",__func__);

	ret = regmap_read(madera->regmap, MADERA_IRQ1_RAW_STATUS_7, &val);
	if (ret) {
		dev_err(info->dev, "Failed to read jackdet status: %d\n", ret);
		return ret;
	}

	dev_info(info->dev, "IRQ1_RAW_STATUS_7=0x%x\n", val);

	if (info->pdata->jd_use_jd2) {
		val &= MADERA_MICD_CLAMP_RISE_STS1;
		present = 0;
	} else if (info->pdata->jd_invert) {
		val &= MADERA_JD1_FALL_STS1_MASK;
		present = MADERA_JD1_FALL_STS1;
	} else {
		val &= MADERA_JD1_RISE_STS1_MASK;
		present = MADERA_JD1_RISE_STS1;
	}

	dev_info(info->dev, "jackdet val=0x%x present=0x%x\n", val, present);

	if (jack_val)
		*jack_val = val;

	if (val == present)
		return 1;
	else
		return 0;
}
*/
static irqreturn_t madera_hpdet_handler(int irq, void *data)
{
	pr_info("%s:enter",__func__);
	struct madera_extcon *info = data;
	int ret;

	dev_dbg(info->dev, "HPDET handler\n");

	madera_jds_cancel_timeout(info);

	mutex_lock(&info->lock);

	switch (madera_jds_get_mode(info)) {
	case MADERA_ACCDET_MODE_HPL:
	case MADERA_ACCDET_MODE_HPR:
	case MADERA_ACCDET_MODE_HPM:
		/* Fall through to spurious if no jack present */
		//if (madera_jack_present(info, NULL) > 0)
		if(info->last_jackdet)
			break;
		__attribute__((__fallthrough__));
	default:
		dev_warn(info->dev, "Spurious HPDET IRQ\n");
		madera_jds_start_timeout(info);
		mutex_unlock(&info->lock);
		return IRQ_NONE;
	}

	ret = madera_hpdet_read(info);
	if (ret == -EAGAIN)
		goto out;

	madera_jds_reading(info, ret);

out:
	madera_jds_start_timeout(info);

	pm_runtime_mark_last_busy(info->dev);

	mutex_unlock(&info->lock);

	return IRQ_HANDLED;
}

static void madera_micd_handler(struct work_struct *work)
{
	struct madera_extcon *info = container_of(work,
						  struct madera_extcon,
						  micd_detect_work.work);
	enum madera_accdet_mode mode;
	int ret;
	pr_info("%s:enter",__func__);
	madera_jds_cancel_timeout(info);

	mutex_lock(&info->lock);

	/*
	 * Must check that we are in a micd state before accessing
	 * any codec registers
	 */
	mode = madera_jds_get_mode(info);
	switch (mode) {
	case MADERA_ACCDET_MODE_MIC:
	case MADERA_ACCDET_MODE_ADC:
		break;
	default:
		goto spurious;
	}

//	if (madera_jack_present(info, NULL) <= 0)
	if(info->last_jackdet <= 0)
		goto spurious;

	switch (mode) {
	case MADERA_ACCDET_MODE_MIC:
		ret = madera_micd_read(info);
		break;
	case MADERA_ACCDET_MODE_ADC:
		ret = madera_micd_adc_read(info);
		break;
	default:	/* we can't get here but compiler still warns */
		ret = 0;
		break;
	}

	if (ret == -EAGAIN)
		goto out;

	if (ret >= 0) {
		dev_dbg(info->dev, "Mic impedance %d ohms\n", ret);
		ret = madera_ohm_to_hohm((unsigned int)ret);
	}

	madera_jds_reading(info, ret);

out:
	madera_jds_start_timeout(info);

	pm_runtime_mark_last_busy(info->dev);

	mutex_unlock(&info->lock);

	return;

spurious:
	dev_warn(info->dev, "Spurious MICDET IRQ\n");
	madera_jds_start_timeout(info);
	mutex_unlock(&info->lock);
}

static irqreturn_t madera_micdet(int irq, void *data)
{
	pr_info("%s:enter",__func__);
	struct madera_extcon *info = data;
	int debounce = info->pdata->micd_detect_debounce_ms;

	dev_dbg(info->dev, "micdet IRQ");

	cancel_delayed_work_sync(&info->micd_detect_work);

	mutex_lock(&info->lock);

	if (!info->detecting)
		debounce = 0;

	mutex_unlock(&info->lock);

	/*
	 * Defer to the workqueue to ensure serialization
	 * and prevent race conditions if an IRQ occurs while
	 * running the delayed work
	 */
	schedule_delayed_work(&info->micd_detect_work,
				  msecs_to_jiffies(debounce));

	return IRQ_HANDLED;
}

const struct madera_jd_state madera_hpdet_left = {
	.mode = MADERA_ACCDET_MODE_HPL,
	.start = madera_hpdet_start,
	.reading = madera_hpdet_reading,
	.stop = madera_hpdet_stop,
};
EXPORT_SYMBOL_GPL(madera_hpdet_left);

const struct madera_jd_state madera_hpdet_right = {
	.mode = MADERA_ACCDET_MODE_HPR,
	.start = madera_hpdet_start,
	.reading = madera_hpdet_reading,
	.stop = madera_hpdet_stop,
};
EXPORT_SYMBOL_GPL(madera_hpdet_right);

const struct madera_jd_state madera_micd_button = {
	.mode = MADERA_ACCDET_MODE_MIC,
	.start = madera_micd_start,
	.reading = madera_micd_button_reading,
	.stop = madera_micd_stop,
};
EXPORT_SYMBOL_GPL(madera_micd_button);

const struct madera_jd_state madera_micd_adc_mic = {
	.mode = MADERA_ACCDET_MODE_ADC,
	.start = madera_micd_mic_start,
	.restart = madera_micd_restart,
	.reading = madera_micd_mic_reading,
	.stop = madera_micd_mic_stop,

	.timeout_ms = madera_micd_mic_timeout_ms,
	.timeout = madera_micd_mic_timeout,
};
EXPORT_SYMBOL_GPL(madera_micd_adc_mic);

const struct madera_jd_state madera_micd_microphone = {
	.mode = MADERA_ACCDET_MODE_MIC,
	.start = madera_micd_mic_start,
	.reading = madera_micd_mic_reading,
	.stop = madera_micd_mic_stop,

	.timeout_ms = madera_micd_mic_timeout_ms,
	.timeout = madera_micd_mic_timeout,
};
EXPORT_SYMBOL_GPL(madera_micd_microphone);

// static irqreturn_t madera_jackdet(int irq, void *data)
// {
// 	struct madera_extcon *info = data;
// 	struct madera *madera = info->madera;
// 	unsigned int val, mask;
// 	bool cancelled_state;
// 	int i, present;

// 	dev_dbg(info->dev, "jackdet IRQ");

// 	cancelled_state = madera_jds_cancel_timeout(info);

// 	pm_runtime_get_sync(info->dev);

// 	mutex_lock(&info->lock);

// 	val = 0;
// 	present = madera_jack_present(info, &val);
// 	if (present < 0) {
// 		mutex_unlock(&info->lock);
// 		pm_runtime_put_autosuspend(info->dev);
// 		return IRQ_NONE;
// 	}

// 	if (val == info->last_jackdet) {
// 		dev_dbg(info->dev, "Suppressing duplicate JACKDET\n");
// 		if (cancelled_state)
// 			madera_jds_start_timeout(info);

// 		goto out;
// 	}
// 	info->last_jackdet = val;

// 	mask = MADERA_MICD_CLAMP_DB | MADERA_JD1_DB;

// 	if (info->pdata->jd_use_jd2)
// 		mask |= MADERA_JD2_DB;

// 	if (present) {
// 		dev_dbg(info->dev, "Detected jack\n");

// 		madera_extcon_report(info, EXTCON_MECHANICAL, true);

// 		info->have_mic = false;
// 		info->jack_flips = 0;

// 		if (info->pdata->custom_jd)
// 			madera_jds_set_state(info, info->pdata->custom_jd);
// 		else if (info->pdata->micd_software_compare)
// 			madera_jds_set_state(info, &madera_micd_adc_mic);
// 		else
// 			madera_jds_set_state(info, &madera_micd_microphone);

// 		madera_jds_start_timeout(info);

// 		regmap_update_bits(madera->regmap, MADERA_INTERRUPT_DEBOUNCE_7,
// 				   mask, 0);
// 	} else {
// 		dev_dbg(info->dev, "Detected jack removal\n");

// 		info->have_mic = false;
// 		info->micd_res_old = 0;
// 		info->micd_debounce = 0;
// 		info->micd_count = 0;
// 		madera_jds_set_state(info, NULL);

// 		for (i = 0; i < info->num_micd_ranges; i++)
// 			input_report_key(info->input,
// 					 info->micd_ranges[i].key, 0);
// 		input_sync(info->input);

// 		for (i = 0; i < ARRAY_SIZE(madera_cable) - 1; i++)
// 			madera_extcon_report(info, madera_cable[i], false);

// 		regmap_update_bits(madera->regmap, MADERA_INTERRUPT_DEBOUNCE_7,
// 				   mask, mask);

// 		madera_set_headphone_imp(info, MADERA_HP_Z_OPEN);

// 		madera_extcon_notify_micd(info, false, 0);
// 	}

// out:
// 	mutex_unlock(&info->lock);

// 	pm_runtime_mark_last_busy(info->dev);
// 	pm_runtime_put_autosuspend(info->dev);

// 	return IRQ_HANDLED;
// }

int madera_extcon_detect(int present, bool have_mic, struct madera_extcon *madera_extcon_info)
{
	struct madera_extcon *info = madera_extcon_info;
	struct madera *madera = info->madera;
	unsigned int val, mask;
	bool cancelled_state;
	int i;
	if (present) {
		if(have_mic)  {
			dev_info(info->dev, "madera_extcon_detect begin headset detected\n");
		} else {
			dev_info(info->dev, "madera_extcon_detect begin headphone detected\n");
		}
	}
	info->have_mic = have_mic;

	cancelled_state = madera_jds_cancel_timeout(info);

	pm_runtime_get_sync(info->dev);

	mutex_lock(&info->lock);

	val = 0;
//	present = madera_jack_present(info, &val);
	if (present < 0) {
		mutex_unlock(&info->lock);
		pm_runtime_put_autosuspend(info->dev);
		return IRQ_NONE;
	}

	if (present == info->last_jackdet) {
		dev_dbg(info->dev, "Suppressing duplicate JACKDET\n");
		if (cancelled_state)
			madera_jds_start_timeout(info);

		goto out;
	}
	info->last_jackdet = present;


	mask = MADERA_MICD_CLAMP_DB;

	if (info->pdata->jd_use_jd2)
		mask |= MADERA_JD2_DB;

	if (present) {
		dev_info(info->dev, "Detected jack\n");

		//madera_extcon_report(info, EXTCON_MECHANICAL, true);

		//info->have_mic = false;
		// if (info->have_mic)
		// 	madera_extcon_report(info, EXTCON_JACK_MICROPHONE, info->have_mic);
		if(have_mic)  {
			dev_info(info->dev, "madera_extcon_report EXTCON_JACK_MICROPHONE detected\n");
			madera_extcon_report(info, EXTCON_JACK_MICROPHONE, true);
		} else {
			dev_info(info->dev, "madera_extcon_report EXTCON_JACK_HEADPHONE detected\n");
			madera_extcon_report(info, EXTCON_JACK_HEADPHONE, true);
		}
		info->jack_flips = 0;


		/* ignore mic reading process as micphone doesn't need detect
		if (info->pdata->custom_jd)
			madera_jds_set_state(info, info->pdata->custom_jd);
		else if (info->pdata->micd_software_compare)
			madera_jds_set_state(info, &madera_micd_adc_mic);
		else
			madera_jds_set_state(info, &madera_micd_microphone);
		*/
		if (info->pdata->hpdet_channel)
			 madera_jds_set_state(info, &madera_hpdet_right);
		else
			madera_jds_set_state(info, &madera_hpdet_left);

		madera_jds_start_timeout(info);

		regmap_update_bits(madera->regmap, MADERA_INTERRUPT_DEBOUNCE_7,
				   mask, 0);
	} else {
		dev_dbg(info->dev, "Detected jack removal\n");

		info->have_mic = false;
		info->micd_res_old = 0;
		info->micd_debounce = 0;
		info->micd_count = 0;
		madera_jds_set_state(info, NULL);

		for (i = 0; i < info->num_micd_ranges; i++)
			input_report_key(info->input,
					 info->micd_ranges[i].key, 0);
		input_sync(info->input);


		// for (i = 0; i < ARRAY_SIZE(madera_cable) - 1; i++)
		// 	madera_extcon_report(info, madera_cable[i], false);
		if(info->event_flag)  {
			dev_info(info->dev, "madera_extcon_report EXTCON_JACK_MICROPHONE remove\n");
			madera_extcon_report(info, info->event_flag, false);
		} else {
			dev_err(info->dev, "madera_extcon_report read last madera_extcon_report err\n");
		}
		info->event_flag = 0;
		regmap_update_bits(madera->regmap, MADERA_INTERRUPT_DEBOUNCE_7,
				   mask, mask);

		madera_set_headphone_imp(info, MADERA_HP_Z_OPEN);

		madera_extcon_notify_micd(info, false, 0);
	}

out:
	mutex_unlock(&info->lock);

	pm_runtime_mark_last_busy(info->dev);
	pm_runtime_put_autosuspend(info->dev);

	return IRQ_HANDLED;
}
EXPORT_SYMBOL_GPL(madera_extcon_detect);


/* Map a level onto a slot in the register bank */
static void madera_micd_set_level(struct madera *madera, int index,
				  unsigned int level)
{
	int reg;
	unsigned int mask;

	reg = MADERA_MIC_DETECT_1_LEVEL_4 - (index / 2);

	if (!(index % 2)) {
		mask = 0x3f00;
		level <<= 8;
	} else {
		mask = 0x3f;
	}

	/* Program the level itself */
	regmap_update_bits(madera->regmap, reg, mask, level);
}

static void madera_extcon_of_get_micd_ranges(struct madera_extcon *info,
						struct fwnode_handle *node,
						struct madera_accdet_pdata *pdata)
{
	struct madera_micd_range *micd_ranges;
	u32 *values;
	int nvalues, nranges, i, j;
	int ret;

	nvalues = fwnode_property_read_u32_array(node, "cirrus,micd-ranges",
						 NULL, 0);
	if (nvalues < 0)
		return;

	values = kmalloc_array(nvalues, sizeof(u32), GFP_KERNEL);
	if (!values)
		return;

	ret = fwnode_property_read_u32_array(node, "cirrus,micd-ranges",
						 values, nvalues);
	if (ret < 0)
		goto err;

	nranges = nvalues / 2;
	micd_ranges = devm_kcalloc(info->dev,
				   nranges,
				   sizeof(struct madera_micd_range),
				   GFP_KERNEL);

	for (i = 0, j = 0; i < nranges; ++i) {
		micd_ranges[i].max = values[j++];
		micd_ranges[i].key = values[j++];
	}

	pdata->micd_ranges = micd_ranges;
	pdata->num_micd_ranges = nranges;

err:
	kfree(values);
}

static void madera_extcon_get_micd_configs(struct madera_extcon *info,
					   struct fwnode_handle *node)
{
	struct madera_micd_config *micd_configs;
	u32 *values;
	int nvalues, nconfigs, i, j;
	int ret;

	nvalues = fwnode_property_read_u32_array(node,
						 "cirrus,micd-configs",
						 NULL, 0);
	if (nvalues == -EINVAL) {
		return;	/* not found */
	} else if ((nvalues < 0) || (nvalues % 5)) {
		dev_warn(info->dev, "cirrus,micd-configs is malformed\n");
		return;
	}

	values = kmalloc_array(nvalues, sizeof(u32), GFP_KERNEL);
	if (!values)
		return;

	ret = fwnode_property_read_u32_array(node,
						 "cirrus,micd-configs",
						 values, nvalues);
	if (ret < 0)
		goto err;

	nconfigs = nvalues / 5;
	micd_configs = devm_kcalloc(info->dev,
					nconfigs,
					sizeof(struct madera_micd_config),
					GFP_KERNEL);
	if (!micd_configs)
		goto err;

	for (i = 0, j = 0; i < nconfigs; ++i) {
		micd_configs[i].src = values[j++];
		micd_configs[i].gnd = values[j++];
		micd_configs[i].bias = values[j++];
		micd_configs[i].gpio = values[j++];
		micd_configs[i].hp_gnd = values[j++];
	}

	info->micd_modes = micd_configs;
	info->num_micd_modes = nconfigs;

err:
	kfree(values);
}

static void madera_extcon_get_hpd_pins(struct madera_extcon *info,
					   struct fwnode_handle *node,
					   struct madera_accdet_pdata *pdata)
{
	int i, ret;

	BUILD_BUG_ON(ARRAY_SIZE(pdata->hpd_pins) !=
			 ARRAY_SIZE(madera_default_hpd_pins));

	memcpy(pdata->hpd_pins, madera_default_hpd_pins,
		   sizeof(pdata->hpd_pins));

	ret = fwnode_property_read_u32_array(node,
						 "cirrus,hpd-pins",
						 pdata->hpd_pins,
						 ARRAY_SIZE(pdata->hpd_pins));
	if (ret) {
		if (ret != -EINVAL)
			dev_warn(info->dev,
				 "Malformed cirrus,hpd-pins: %d\n", ret);
		return;
	}

	/* supply defaults where requested */
	for (i = 0; i < ARRAY_SIZE(pdata->hpd_pins); ++i)
		if (pdata->hpd_pins[i] > 0xFFFF)
			pdata->hpd_pins[i] = madera_default_hpd_pins[i];
}

static void madera_extcon_process_accdet_node(struct madera_extcon *info,
						  struct fwnode_handle *node)
{
	struct madera *madera = info->madera;
	struct madera_accdet_pdata *pdata;
	u32 out_num;
	int i, ret;
	enum gpiod_flags gpio_status;

	ret = fwnode_property_read_u32(node, "reg", &out_num);
	if (ret < 0) {
		dev_warn(info->dev,
			 "failed to read reg property (%d)\n",
			 ret);
		return;
	}

	if (out_num == 0) {
		dev_warn(info->dev, "accdet node illegal reg %u\n", out_num);
		return;
	}

	dev_dbg(info->dev, "processing accdet reg=%u\n", out_num);

	for (i = 0; i < ARRAY_SIZE(madera->pdata.accdet); i++)
		if (!madera->pdata.accdet[i].enabled)
			break;

	if (i == ARRAY_SIZE(madera->pdata.accdet)) {
		dev_warn(madera->dev, "Too many accdet nodes: %d\n", i + 1);
		return;
	}

	pdata = &madera->pdata.accdet[i];
	pdata->enabled = true;	/* implied by presence of properties node */
	pdata->output = out_num;

	fwnode_property_read_u32(node, "cirrus,micd-detect-debounce-ms",
				 &pdata->micd_detect_debounce_ms);

	fwnode_property_read_u32(node, "cirrus,micd-manual-debounce",
				 &pdata->micd_manual_debounce);

	fwnode_property_read_u32(node, "cirrus,micd-bias-start-time",
				 &pdata->micd_bias_start_time);

	fwnode_property_read_u32(node, "cirrus,micd-rate",
				 &pdata->micd_rate);

	fwnode_property_read_u32(node, "cirrus,micd-dbtime",
				 &pdata->micd_dbtime);

	fwnode_property_read_u32(node, "cirrus,micd-timeout-ms",
				 &pdata->micd_timeout_ms);

	/* don't override any preset force_micbias enable */
	if (fwnode_property_present(node, "cirrus,micd-force-micbias"))
		pdata->micd_force_micbias = true;

	pdata->micd_software_compare =
		fwnode_property_present(node,
					"cirrus,micd-software-compare");

	pdata->micd_open_circuit_declare =
		fwnode_property_present(node,
					"cirrus,micd-open-circuit-declare");

	pdata->jd_use_jd2 = fwnode_property_present(node,
							"cirrus,jd-use-jd2");

	dev_dbg(info->dev, "jd_use_jd2:%d\n", pdata->jd_use_jd2);

	pdata->jd_invert = fwnode_property_present(node,
						   "cirrus,jd-invert");

	fwnode_property_read_u32(node, "cirrus,fixed-hpdet-imp",
				 &pdata->fixed_hpdet_imp_x100);

	fwnode_property_read_u32(node, "cirrus,hpdet-short-circuit-imp",
				 &pdata->hpdet_short_circuit_imp);

	fwnode_property_read_u32(node, "cirrus,hpdet-channel",
				 &pdata->hpdet_channel);

	fwnode_property_read_u32(node, "cirrus,micd-clamp-mode",
				 &pdata->micd_clamp_mode);

	fwnode_property_read_u32(node, "cirrus,hpdet-ext-res",
				 &pdata->hpdet_ext_res_x100);

	madera_extcon_get_hpd_pins(info, node, pdata);
	madera_extcon_get_micd_configs(info, node);
	madera_extcon_of_get_micd_ranges(info, node, pdata);

	if (info->micd_modes[0].gpio)
		gpio_status = GPIOD_OUT_HIGH;
	else
		gpio_status = GPIOD_OUT_LOW;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,6,0)
	info->micd_pol_gpio = devm_fwnode_gpiod_get(info->dev,
					node,
					"cirrus,micd-pol",
					gpio_status,
					"cirrus,micd-pol");
#else
	info->micd_pol_gpio = devm_fwnode_get_gpiod_from_child(info->dev,
					"cirrus,micd-pol",
					node,
					gpio_status,
					"cirrus,micd-pol");
#endif

	if (IS_ERR(info->micd_pol_gpio)) {
		if (PTR_ERR(info->micd_pol_gpio) != -ENOENT)
			dev_warn(info->dev,
				 "Malformed cirrus,micd-pol-gpios ignored: %ld\n",
				 PTR_ERR(info->micd_pol_gpio));
		info->micd_pol_gpio = NULL;
	}
}

static int madera_extcon_get_device_pdata(struct madera_extcon *info)
{
	struct device_node *parent, *child;
	struct madera *madera = info->madera;

	/*
	 * a GPSW is not necessarily exclusive to a single accessory detect
	 * channel so is not in the subnodes
	 */
	device_property_read_u32_array(info->madera->dev, "cirrus,gpsw",
					   info->madera->pdata.gpsw,
					   ARRAY_SIZE(info->madera->pdata.gpsw));

	parent = of_get_child_by_name(madera->dev->of_node, "cirrus,accdet");
	if (!parent) {
		dev_dbg(madera->dev, "No DT nodes\n");
		return 0;
	}

	for_each_child_of_node(parent, child)
		madera_extcon_process_accdet_node(info, &child->fwnode);

	of_node_put(parent);

	return 0;
}
int madera_jds_get_extcon_data(struct madera_extcon *madera_extcon_info)
{

	madera_extcon_info = g_madera_extcon_info;
	return 0;

}
EXPORT_SYMBOL_GPL(madera_jds_get_extcon_data);

int madera_jds_set_extcon_data(struct madera_extcon *madera_extcon_info)
{

	g_madera_extcon_info = madera_extcon_info;
	return 0;

}
EXPORT_SYMBOL_GPL(madera_jds_set_extcon_data);


#ifdef DEBUG
#define MADERA_EXTCON_PDATA_DUMP(x, f) \
	dev_dbg(info->dev, "\t" #x ": " f "\n", pdata->x)

static void madera_extcon_dump_config(struct madera_extcon *info)
{
	const struct madera_accdet_pdata *pdata;
	int i, j;

	dev_dbg(info->dev, "extcon pdata gpsw=[0x%x 0x%x]\n",
		info->madera->pdata.gpsw[0], info->madera->pdata.gpsw[1]);

	for (i = 0; i < ARRAY_SIZE(info->madera->pdata.accdet); ++i) {
		pdata = &info->madera->pdata.accdet[i];

		dev_dbg(info->dev, "extcon pdata OUT%u\n", pdata->output);
		MADERA_EXTCON_PDATA_DUMP(enabled, "%u");
		MADERA_EXTCON_PDATA_DUMP(jd_use_jd2, "%u");
		MADERA_EXTCON_PDATA_DUMP(jd_invert, "%u");
		MADERA_EXTCON_PDATA_DUMP(fixed_hpdet_imp_x100, "%d");
		MADERA_EXTCON_PDATA_DUMP(hpdet_ext_res_x100, "%d");
		MADERA_EXTCON_PDATA_DUMP(hpdet_short_circuit_imp, "%d");
		MADERA_EXTCON_PDATA_DUMP(hpdet_channel, "%d");
		MADERA_EXTCON_PDATA_DUMP(micd_detect_debounce_ms, "%d");
		MADERA_EXTCON_PDATA_DUMP(hpdet_short_circuit_imp, "%d");
		MADERA_EXTCON_PDATA_DUMP(hpdet_channel, "%d");
		MADERA_EXTCON_PDATA_DUMP(micd_detect_debounce_ms, "%d");
		MADERA_EXTCON_PDATA_DUMP(micd_manual_debounce, "%d");
		MADERA_EXTCON_PDATA_DUMP(micd_bias_start_time, "%d");
		MADERA_EXTCON_PDATA_DUMP(micd_rate, "%d");
		MADERA_EXTCON_PDATA_DUMP(micd_dbtime, "%d");
		MADERA_EXTCON_PDATA_DUMP(micd_timeout_ms, "%d");
		MADERA_EXTCON_PDATA_DUMP(micd_clamp_mode, "%u");
		MADERA_EXTCON_PDATA_DUMP(micd_force_micbias, "%u");
		MADERA_EXTCON_PDATA_DUMP(micd_open_circuit_declare, "%u");
		MADERA_EXTCON_PDATA_DUMP(micd_software_compare, "%u");

		if (info->micd_pol_gpio)
			dev_dbg(info->dev, "micd_pol_gpio: %d\n",
				desc_to_gpio(info->micd_pol_gpio));
		else
			dev_dbg(info->dev, "micd_pol_gpio: unused\n");

		dev_dbg(info->dev, "\tmicd_ranges {\n");
		for (j = 0; j < info->num_micd_ranges; ++j)
			dev_dbg(info->dev, "\t\tmax: %d key: %d\n",
				info->micd_ranges[j].max,
				info->micd_ranges[j].key);
		dev_dbg(info->dev, "\t}\n");

		dev_dbg(info->dev, "\tmicd_configs {\n");
		for (j = 0; j < info->num_micd_modes; ++j)
			dev_dbg(info->dev,
				"\t\tsrc: 0x%x gnd: 0x%x bias: %u gpio: %u hp_gnd: %d\n",
				info->micd_modes[j].src,
				info->micd_modes[j].gnd,
				info->micd_modes[j].bias,
				info->micd_modes[j].gpio,
				info->micd_modes[j].hp_gnd);
		dev_dbg(info->dev, "\t}\n");

		dev_dbg(info->dev, "\thpd_pins: %u %u %u %u\n",
			pdata->hpd_pins[0], pdata->hpd_pins[1],
			pdata->hpd_pins[2], pdata->hpd_pins[3]);
	}
}
#else
static inline void madera_extcon_dump_config(struct madera_extcon *info)
{
}
#endif

/* See datasheet for a description of this calibration data */
static int madera_extcon_read_calibration(struct madera_extcon *info)
{
	struct madera *madera = info->madera;
	struct madera_hpdet_trims *trims;
	int ret = -EIO;
	unsigned int offset, gradient, interim_val;
	unsigned int otp_hpdet_calib_1, otp_hpdet_calib_2;

	switch (madera->type) {
	case CS47L35:
		otp_hpdet_calib_1 = CS47L35_OTP_HPDET_CAL_1;
		otp_hpdet_calib_2 = CS47L35_OTP_HPDET_CAL_2;
		break;
	case CS47L85:
	case WM1840:
		otp_hpdet_calib_1 = CS47L85_OTP_HPDET_CAL_1;
		otp_hpdet_calib_2 = CS47L85_OTP_HPDET_CAL_2;
		break;
	default:
		otp_hpdet_calib_1 = MADERA_OTP_HPDET_CAL_1;
		otp_hpdet_calib_2 = MADERA_OTP_HPDET_CAL_2;
		break;
	}

	ret = regmap_read(madera->regmap_32bit, otp_hpdet_calib_1, &offset);
	if (ret) {
		dev_err(info->dev,
			"Failed to read HP CALIB OFFSET value: %d\n", ret);
		return ret;
	}

	ret = regmap_read(madera->regmap_32bit, otp_hpdet_calib_2, &gradient);
	if (ret) {
		dev_err(info->dev,
			"Failed to read HP CALIB OFFSET value: %d\n", ret);
		return ret;
	}

	if (((offset == 0) && (gradient == 0)) ||
		((offset == 0xFFFFFFFF) && (gradient == 0xFFFFFFFF))) {
		dev_warn(info->dev, "No HP trims\n");
		return 0;
	}

	trims = devm_kcalloc(info->dev, 4,
				 sizeof(struct madera_hpdet_trims),
				 GFP_KERNEL);
	if (!trims) {
		dev_err(info->dev, "Failed to alloc hpdet trims\n");
		return -ENOMEM;
	}

	interim_val = (offset & MADERA_OTP_HPDET_CALIB_OFFSET_00_MASK) >>
			   MADERA_OTP_HPDET_CALIB_OFFSET_00_SHIFT;
	trims[0].off_x4 = 128 - interim_val;

	interim_val = (gradient & MADERA_OTP_HPDET_GRADIENT_0X_MASK) >>
			   MADERA_OTP_HPDET_GRADIENT_0X_SHIFT;
	trims[0].grad_x4 = 128 - interim_val;

	interim_val = (offset & MADERA_OTP_HPDET_CALIB_OFFSET_01_MASK) >>
			MADERA_OTP_HPDET_CALIB_OFFSET_01_SHIFT;
	trims[1].off_x4 = 128 - interim_val;

	trims[1].grad_x4 = trims[0].grad_x4;

	interim_val = (offset & MADERA_OTP_HPDET_CALIB_OFFSET_10_MASK) >>
			   MADERA_OTP_HPDET_CALIB_OFFSET_10_SHIFT;
	trims[2].off_x4 = 128 - interim_val;

	interim_val = (gradient & MADERA_OTP_HPDET_GRADIENT_1X_MASK) >>
			   MADERA_OTP_HPDET_GRADIENT_1X_SHIFT;
	trims[2].grad_x4 = 128 - interim_val;

	interim_val = (offset & MADERA_OTP_HPDET_CALIB_OFFSET_11_MASK) >>
			   MADERA_OTP_HPDET_CALIB_OFFSET_11_SHIFT;
	trims[3].off_x4 = 128 - interim_val;

	trims[3].grad_x4 = trims[2].grad_x4;

	info->hpdet_trims = trims;

	dev_dbg(info->dev,
		"trims_x_4: %d,%d %d,%d %d,%d %d,%d\n",
		trims[0].off_x4, trims[0].grad_x4,
		trims[1].off_x4, trims[1].grad_x4,
		trims[2].off_x4, trims[2].grad_x4,
		trims[3].off_x4, trims[3].grad_x4);

	return 0;
}

static void madera_extcon_set_micd_clamp_mode(struct madera_extcon *info)
{
	struct madera *madera = info->madera;
	unsigned int clamp_ctrl_val;

	/*
	 * If the user has supplied a micd_clamp_mode, assume they know
	 * what they are doing and just write it out
	 */
	if (info->pdata->micd_clamp_mode) {
		clamp_ctrl_val = info->pdata->micd_clamp_mode;
	} else if (info->pdata->jd_use_jd2) {
		if (info->pdata->jd_invert)
			clamp_ctrl_val = MADERA_MICD_CLAMP_MODE_JD1H_JD2H;
		else
			clamp_ctrl_val = MADERA_MICD_CLAMP_MODE_JD1L_JD2L;
	} else {
		if (info->pdata->jd_invert)
			clamp_ctrl_val = MADERA_MICD_CLAMP_MODE_JD1H;
		else
			clamp_ctrl_val = info->pdata->micd_clamp_mode;
	}

	/* xr unused jd1 & jd2 */
	if (!madera->bind_product)
		regmap_write(info->madera->regmap, MADERA_MICD_CLAMP_CONTROL, 0x0);

	regmap_update_bits(info->madera->regmap,
			   MADERA_MICD_CLAMP_CONTROL,
			   MADERA_MICD_CLAMP_MODE_MASK,
			   clamp_ctrl_val);

	regmap_update_bits(info->madera->regmap,
			   MADERA_INTERRUPT_DEBOUNCE_7,
			   MADERA_MICD_CLAMP_DB,
			   MADERA_MICD_CLAMP_DB);
}

static int madera_extcon_add_micd_levels(struct madera_extcon *info)
{
	struct madera *madera = info->madera;
	int i, j;
	int ret = 0;

	BUILD_BUG_ON(ARRAY_SIZE(madera_micd_levels) <
			 MADERA_NUM_MICD_BUTTON_LEVELS);

	/* Disable all buttons by default */
	regmap_update_bits(madera->regmap, MADERA_MIC_DETECT_1_CONTROL_2,
			   MADERA_MICD_LVL_SEL_MASK, 0x81);

	/* Set up all the buttons the user specified */
	for (i = 0; i < info->num_micd_ranges; i++) {
		for (j = 0; j < MADERA_NUM_MICD_BUTTON_LEVELS; j++)
			if (madera_micd_levels[j] >= info->micd_ranges[i].max)
				break;

		if (j == MADERA_NUM_MICD_BUTTON_LEVELS) {
			dev_err(info->dev, "Unsupported MICD level %d\n",
				info->micd_ranges[i].max);
			ret = -EINVAL;
			goto err_input;
		}

		dev_dbg(info->dev, "%d ohms for MICD threshold %d\n",
			madera_micd_levels[j], i);

		madera_micd_set_level(madera, i, j);
		if (info->micd_ranges[i].key > 0)
			input_set_capability(info->input, EV_KEY,
						 info->micd_ranges[i].key);

		/* Enable reporting of that range */
		regmap_update_bits(madera->regmap,
				   MADERA_MIC_DETECT_1_CONTROL_2,
				   1 << i, 1 << i);
	}

	/* Set all the remaining keys to a maximum */
	for (; i < MADERA_MAX_MICD_RANGE; i++)
		madera_micd_set_level(madera, i, 0x3f);

err_input:
	return ret;
}

static int madera_extcon_init_micd_ranges(struct madera_extcon *info)
{
	const struct madera_accdet_pdata *pdata = info->pdata;
	struct madera_micd_range *ranges;
	int i;

	if (pdata->num_micd_ranges == 0) {
		info->micd_ranges = madera_micd_default_ranges;
		info->num_micd_ranges =
			ARRAY_SIZE(madera_micd_default_ranges);
		return 0;
	}

	if (pdata->num_micd_ranges > MADERA_MAX_MICD_RANGE) {
		dev_err(info->dev, "Too many MICD ranges: %d\n",
			pdata->num_micd_ranges);
		return -EINVAL;
	}

	ranges = devm_kmalloc_array(info->dev,
					pdata->num_micd_ranges,
					sizeof(struct madera_micd_range),
					GFP_KERNEL);
	if (!ranges) {
		dev_err(info->dev, "Failed to kalloc micd ranges\n");
		return -ENOMEM;
	}

	memcpy(ranges, pdata->micd_ranges,
		   sizeof(struct madera_micd_range) * pdata->num_micd_ranges);
	info->micd_ranges = ranges;
	info->num_micd_ranges = pdata->num_micd_ranges;

	for (i = 0; i < info->num_micd_ranges - 1; i++) {
		if (info->micd_ranges[i].max > info->micd_ranges[i + 1].max) {
			dev_err(info->dev, "MICD ranges must be sorted\n");
			goto err_free;
		}
	}

	return 0;

err_free:
	devm_kfree(info->dev, ranges);

	return -EINVAL;
}

static void madera_extcon_xlate_pdata(struct madera_accdet_pdata *pdata)
{
	int i;

	BUILD_BUG_ON(ARRAY_SIZE(pdata->hpd_pins) !=
			 ARRAY_SIZE(madera_default_hpd_pins));

	/* translate from pdata format where 0=default and >0xFFFF means 0 */
	for (i = 0; i < ARRAY_SIZE(pdata->hpd_pins); ++i) {
		if (pdata->hpd_pins[i] == 0)
			pdata->hpd_pins[i] = madera_default_hpd_pins[i];
		else if (pdata->hpd_pins[i] > 0xFFFF)
			pdata->hpd_pins[i] = 0;
	}
}

static struct madera_extcon *madera_info;
void madera_usb_port_set(int value);
void accdet_madera_report(int which, bool attached)
{
	int device_type = 0;
	bool have_mic = false;
	switch (which) {
		case 0x8://4POLE_0
				madera_usb_port_set(0);
				device_type = EXTCON_JACK_MICROPHONE;
				have_mic = true;
				break;
		case 0x4://4POLE_1
				madera_usb_port_set(1);
				device_type = EXTCON_JACK_MICROPHONE;
				have_mic = true;
				break;
		case 0x2://3POLE
				device_type = EXTCON_JACK_HEADPHONE;
				have_mic = false;
				break;
		}
	// pr_info("%s:enter MADERA_MICD_CLAMP_CONTROL 0x0",__func__);
	// regmap_write(madera_info->madera->regmap, MADERA_MICD_CLAMP_CONTROL, 0x0);

	#ifdef HEADSET_STATUS_RECORD
		pr_info("%s:HEADSET_STATUS_RECORD plugin event",__func__);
		add_headset_event(
				attached ? HEADSET_STATUS_RECORD_INDEX_PLUGIN : HEADSET_STATUS_RECORD_INDEX_PLUGOUT,
				attached ? HEADSET_EVENT_PLUGIN_HEADPHONE : HEADSET_EVENT_PLUGOUT_HEADPHONE);
	#endif
	// madera_extcon_report(madera_info, device_type, attached);
	madera_extcon_detect(attached, have_mic, madera_info);
}
EXPORT_SYMBOL(accdet_madera_report);

static int madera_extcon_event_changed(struct notifier_block *nb_ptr,
				      unsigned long evt, void *ptr)
{
	int which = 0;
	bool attached;
	struct madera_extcon *madera_extcon_info =
			container_of(nb_ptr, struct madera_extcon, nb);
	struct device *dev;
	int ret;

	pr_info("%s:enter", __func__);
	dev = madera_extcon_info->dev;
	if (!dev)
		return -EINVAL;
	which = ((struct madera_extcon_info *)ptr)->which;
	attached = ((struct madera_extcon_info *)ptr)->attached;
	pr_info("%s:which:%d,attached:%d", __func__, which, attached);

	if (attached) {
		dev_err(dev, "enable 32k clock\n");
		ret = clk_prepare_enable(madera_extcon_info->madera->mclk[MADERA_MCLK2].clk);
		if (ret) {
			dev_err(dev, "Failed to enable 32k clock: %d\n", ret);
			return -1;
		}
	}

	accdet_madera_report(which,attached);

	if (!attached) {
		dev_err(dev, "disable 32k clock\n");
		clk_disable_unprepare(madera_extcon_info->madera->mclk[MADERA_MCLK2].clk);
	}

	pr_info("%s:exit", __func__);
	return 0;
}

static int madera_extcon_probe(struct platform_device *pdev)
{
	struct madera *madera = dev_get_drvdata(pdev->dev.parent);
	struct madera_accdet_pdata *pdata = &madera->pdata.accdet[0];
	struct madera_extcon *info;
	// unsigned int debounce_val, analog_val;
	// int jack_irq_fall, jack_irq_rise;
	int ret, mode, i, hpdet_short_measured;

	/* quick exit if Madera irqchip driver hasn't completed probe */
	if (!madera->irq_dev) {
		dev_info(&pdev->dev, "irqchip driver not ready\n");
		return -EPROBE_DEFER;
	}

	if (!madera->dapm || !madera->dapm->card) {
		dev_info(&pdev->dev, "madera dapm prepare\n");
		return -EPROBE_DEFER;
	}

	info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;
	madera_info = info;
	info->pdata = pdata;
	info->madera = madera;
	info->dev = &pdev->dev;
	mutex_init(&info->lock);
	init_completion(&info->manual_mic_completion);
	INIT_DELAYED_WORK(&info->micd_detect_work, madera_micd_handler);
	INIT_DELAYED_WORK(&info->state_timeout_work, madera_jds_timeout_work);
	platform_set_drvdata(pdev, info);

	switch (madera->type) {
	case CS47L15:
		info->hpdet_init_range = 1; /* range 0 not used on CS47L15 */
		info->hpdet_ranges = cs47l15_hpdet_ranges;
		info->num_hpdet_ranges = ARRAY_SIZE(cs47l15_hpdet_ranges);
		info->micd_modes = madera_micd_default_modes;
		info->num_micd_modes = ARRAY_SIZE(madera_micd_default_modes);
		break;
	case CS47L35:
		pdata->micd_force_micbias = true;
		info->hpdet_ranges = cs47l85_hpdet_ranges;
		info->num_hpdet_ranges = ARRAY_SIZE(cs47l85_hpdet_ranges);
		info->micd_modes = cs47l85_micd_default_modes;
		info->num_micd_modes = ARRAY_SIZE(cs47l85_micd_default_modes);
		break;
	case CS47L85:
	case WM1840:
		info->hpdet_ranges = cs47l85_hpdet_ranges;
		info->num_hpdet_ranges = ARRAY_SIZE(cs47l85_hpdet_ranges);
		info->micd_modes = cs47l85_micd_default_modes;
		info->num_micd_modes = ARRAY_SIZE(cs47l85_micd_default_modes);
		break;
	case CS42L92:
	case CS47L92:
	case CS47L93:
		info->hpdet_ranges = cs47l92_hpdet_ranges;
		info->num_hpdet_ranges = ARRAY_SIZE(cs47l92_hpdet_ranges);
		info->micd_modes = madera_micd_default_modes;
		info->num_micd_modes = ARRAY_SIZE(madera_micd_default_modes);
		break;
	default:
		info->hpdet_ranges = madera_hpdet_ranges;
		info->num_hpdet_ranges = ARRAY_SIZE(madera_hpdet_ranges);
		info->micd_modes = madera_micd_default_modes;
		info->num_micd_modes = ARRAY_SIZE(madera_micd_default_modes);
		break;
	}

	if (dev_get_platdata(madera->dev)) {
		madera_extcon_xlate_pdata(pdata);

		if (pdata->num_micd_configs) {
			info->micd_modes = pdata->micd_configs;
			info->num_micd_modes = pdata->num_micd_configs;
		}

		if (info->micd_modes[0].gpio)
			mode = GPIOF_OUT_INIT_HIGH;
		else
			mode = GPIOF_OUT_INIT_LOW;

		ret = devm_gpio_request_one(&pdev->dev,
						pdata->micd_pol_gpio,
						mode,
						"MICD polarity");
		if (ret) {
			dev_err(info->dev, "Failed to request GPIO%d: %d\n",
				pdata->micd_pol_gpio, ret);
			return ret;
		}

		info->micd_pol_gpio = gpio_to_desc(pdata->micd_pol_gpio);
	} else {
		ret = madera_extcon_get_device_pdata(info);
		if (ret < 0)
			return ret;
	}

	if (!pdata->enabled || pdata->output == 0)
		return -ENODEV; /* no accdet output configured */

	info->hpdet_short_x100 =
		madera_ohm_to_hohm(pdata->hpdet_short_circuit_imp);

	/* Actual measured short is increased by external resistance */
	hpdet_short_measured = pdata->hpdet_short_circuit_imp +
				   madera_hohm_to_ohm(pdata->hpdet_ext_res_x100);

	if (hpdet_short_measured < MADERA_HP_SHORT_IMPEDANCE_MIN) {
		/*
		 * increase comparison threshold to minimum we can measure
		 * taking into account that threshold does not include external
		 * resistance
		 */
		info->hpdet_short_x100 =
			madera_ohm_to_hohm(MADERA_HP_SHORT_IMPEDANCE_MIN) -
			pdata->hpdet_ext_res_x100;
		dev_warn(info->dev,
			 "Increasing HP short circuit impedance from %d to %d\n",
			 pdata->hpdet_short_circuit_imp,
			 madera_hohm_to_ohm(info->hpdet_short_x100));
	}

	info->micvdd = devm_regulator_get(&pdev->dev, "MICVDD");
	if (IS_ERR(info->micvdd)) {
		ret = PTR_ERR(info->micvdd);
		dev_err(info->dev, "Failed to get MICVDD: %d\n", ret);
		return ret;
	}

	if (pdata->jd_invert)
		info->last_jackdet =
			~(MADERA_MICD_CLAMP_RISE_STS1 | MADERA_JD1_FALL_STS1);
	else
		info->last_jackdet =
			~(MADERA_MICD_CLAMP_RISE_STS1 | MADERA_JD1_RISE_STS1);

	info->edev = devm_extcon_dev_allocate(&pdev->dev, madera_cable);
	if (IS_ERR(info->edev)) {
		dev_err(&pdev->dev, "failed to allocate extcon device\n");
		return -ENOMEM;
	}

	ret = devm_extcon_dev_register(&pdev->dev, info->edev);
	if (ret < 0) {
		dev_err(info->dev, "extcon_dev_register() failed: %d\n", ret);
		return ret;
	}

	info->input = devm_input_allocate_device(&pdev->dev);
	if (!info->input) {
		dev_err(info->dev, "Can't allocate input dev\n");
		ret = -ENOMEM;
		goto err_register;
	}

	info->input->name = "Headset";
	info->input->phys = "madera/extcon";
	info->input->dev.parent = &pdev->dev;

	if (madera->pdata.gpsw[0] > 0)
		regmap_update_bits(madera->regmap,
				   MADERA_GP_SWITCH_1,
				   MADERA_SW1_MODE_MASK,
				   madera->pdata.gpsw[0] <<
				   MADERA_SW1_MODE_SHIFT);
	switch (madera->type) {
	case CS47L90:
	case CS47L91:
	case CS42L92:
	case CS47L92:
	case CS47L93:
		if (madera->pdata.gpsw[1] > 0)
			regmap_update_bits(madera->regmap,
					   MADERA_GP_SWITCH_1,
					   MADERA_SW2_MODE_MASK,
					   madera->pdata.gpsw[1] <<
					   MADERA_SW2_MODE_SHIFT);
		break;
	default:
		break;
	}

	if (info->pdata->micd_bias_start_time)
		regmap_update_bits(madera->regmap,
				   MADERA_MIC_DETECT_1_CONTROL_1,
				   MADERA_MICD_BIAS_STARTTIME_MASK,
				   info->pdata->micd_bias_start_time
				   << MADERA_MICD_BIAS_STARTTIME_SHIFT);

	if (info->pdata->micd_rate)
		regmap_update_bits(madera->regmap,
				   MADERA_MIC_DETECT_1_CONTROL_1,
				   MADERA_MICD_RATE_MASK,
				   info->pdata->micd_rate
				   << MADERA_MICD_RATE_SHIFT);

	if (info->pdata->micd_dbtime)
		regmap_update_bits(madera->regmap,
				   MADERA_MIC_DETECT_1_CONTROL_1,
				   MADERA_MICD_DBTIME_MASK,
				   info->pdata->micd_dbtime
				   << MADERA_MICD_DBTIME_SHIFT);

	ret = madera_extcon_init_micd_ranges(info);
	if (ret)
		goto err_input;

	ret = madera_extcon_add_micd_levels(info);
	if (ret)
		goto err_input;

	madera_extcon_set_micd_clamp_mode(info);

	if ((info->num_micd_modes > 2) && !info->micd_pol_gpio)
		dev_warn(info->dev, "Have >1 mic_configs but no pol_gpio\n");

	madera_extcon_set_mode(info, 0);

	/*
	 * Invalidate the tuning level so that the first detection
	 * will always apply a tuning
	 */
	info->hp_tuning_level = MADERA_HP_TUNING_INVALID;

	pm_runtime_enable(&pdev->dev);
	pm_runtime_idle(&pdev->dev);

	pm_runtime_get_sync(&pdev->dev);

	madera_extcon_read_calibration(info);
	if (info->hpdet_trims) {
		switch (madera->type) {
		case CS47L35:
		case CS47L85:
		case WM1840:
			/* set for accurate HP impedance detection */
			regmap_update_bits(madera->regmap,
				MADERA_ACCESSORY_DETECT_MODE_1,
				MADERA_ACCDET_POLARITY_INV_ENA_MASK,
				1 << MADERA_ACCDET_POLARITY_INV_ENA_SHIFT);
			break;
		default:
			break;
		}
	} else {
		switch (madera->type) {
		case CS47L15:
			pdata->hpdet_ext_res_x100 += 3300;
			break;
		default:
			break;
		}
	}

	/* Skip any HPDET ranges less than the external resistance */
	for (i = info->hpdet_init_range; i < info->num_hpdet_ranges; ++i) {
		if (madera_ohm_to_hohm(info->hpdet_ranges[i].max) >=
			pdata->hpdet_ext_res_x100) {
			info->hpdet_init_range = i;
			break;
		}
	}
	if (i == info->num_hpdet_ranges) {
		dev_err(&pdev->dev,
			"No possible range for external resistance %u.%02u\n",
			pdata->hpdet_ext_res_x100 / 100,
			pdata->hpdet_ext_res_x100 % 100);
		goto err_input;
	}

	regmap_update_bits(madera->regmap, MADERA_HEADPHONE_DETECT_1,
			   MADERA_HP_IMPEDANCE_RANGE_MASK,
			   info->hpdet_init_range <<
			   MADERA_HP_IMPEDANCE_RANGE_SHIFT);

	ret = madera_request_irq(madera, MADERA_IRQ_MICDET1,
				 "MICDET", madera_micdet, info);
	if (ret) {
		dev_err(&pdev->dev, "Failed to get MICDET IRQ: %d\n", ret);
		goto err_micdet;
	}

	ret = madera_request_irq(madera, MADERA_IRQ_HPDET,
				 "HPDET", madera_hpdet_handler, info);
	if (ret) {
		dev_err(&pdev->dev, "Failed to get HPDET IRQ: %d\n", ret);
		goto err_hpdet;
	}
	/*
	if (info->pdata->jd_use_jd2) {
		debounce_val = MADERA_JD1_DB | MADERA_JD2_DB;
		analog_val = MADERA_JD1_ENA | MADERA_JD2_ENA;
		jack_irq_rise = MADERA_IRQ_MICD_CLAMP_RISE;
		jack_irq_fall = MADERA_IRQ_MICD_CLAMP_FALL;
	} else {
		debounce_val = MADERA_JD1_DB;
		analog_val = MADERA_JD1_ENA;
		jack_irq_rise = MADERA_IRQ_JD1_RISE;
		jack_irq_fall = MADERA_IRQ_JD1_FALL;
	}


	regmap_update_bits(madera->regmap, MADERA_INTERRUPT_DEBOUNCE_7,
			   debounce_val, debounce_val);
	regmap_update_bits(madera->regmap, MADERA_JACK_DETECT_ANALOGUE,
			   analog_val, analog_val);

	ret = madera_request_irq(madera, jack_irq_rise,
				 "JACKDET rise", madera_jackdet, info);
	if (ret) {
		dev_err(&pdev->dev,
			"Failed to get JACKDET rise IRQ: %d\n", ret);
		goto err_hpdet;
	}

	ret = madera_set_irq_wake(madera, jack_irq_rise, 1);
	if (ret) {
		dev_err(&pdev->dev,
			"Failed to set JD rise IRQ wake: %d\n",	ret);
		goto err_rise;
	}

	ret = madera_request_irq(madera, jack_irq_fall,
				 "JACKDET fall", madera_jackdet, info);
	if (ret) {
		dev_err(&pdev->dev, "Failed to get JD fall IRQ: %d\n", ret);
		goto err_rise_wake;
	}

	ret = madera_set_irq_wake(madera, jack_irq_fall, 1);
	if (ret) {
		dev_err(&pdev->dev,
			"Failed to set JD fall IRQ wake: %d\n", ret);
		goto err_fall;
	}
*/
	ret = regulator_allow_bypass(info->micvdd, true);
	if (ret)
		dev_warn(info->dev,
			 "Failed to set MICVDD to bypass: %d\n", ret);

	pm_runtime_put(&pdev->dev);

	if (IS_ENABLED(CONFIG_XRING_CODEC_MFD_CS47L92)) {
		input_set_capability(info->input,
					 EV_SW,
					 SW_MICROPHONE_INSERT);
		input_set_capability(info->input,
					 EV_SW,
					 SW_HEADPHONE_INSERT);
		input_set_capability(info->input,
					 EV_SW,
					 SW_JACK_PHYSICAL_INSERT);
	}

	ret = input_register_device(info->input);
	if (ret) {
		dev_err(&pdev->dev, "Can't register input device: %d\n", ret);
		goto err_register;
	}

	ret = device_create_file(&pdev->dev, &dev_attr_hp1_impedance);
	if (ret)
		dev_warn(&pdev->dev,
			 "Failed to create sysfs node for hp_impedance %d\n",
			 ret);
	madera_jds_set_extcon_data(info);

	madera_extcon_dump_config(info);

#ifdef HEADSET_STATUS_RECORD
	// new headset status record
	accdet_debugfs_dir = debugfs_create_dir(DEBUGFS_DIR_NAME, NULL);
	if (!IS_ERR(accdet_debugfs_dir)) {
		debugfs_create_file(DEBUGFS_HEADSET_STATUS_FILE_NAME, 0666,
			accdet_debugfs_dir, NULL, &accdet_headset_status_fops);
		debugfs_create_file("headset_test", 0666,
			accdet_debugfs_dir, NULL, &headset_test_fops);
	}
#endif

	dev_info(&pdev->dev, "madera_extcon_probe acore_reg_notifier.\n");
	info->nb.notifier_call = madera_extcon_event_changed;
	info->nb.priority = 0;
	ret = acore_reg_notifier(&info->nb);

	if (ret) {
		dev_err(&pdev->dev,
			"%s: audio switch notifier acore_reg_notifier failed: %d\n",
			__func__, ret);
		// goto err_data;
	}

	dev_info(&pdev->dev, "madera_extcon_probe init end.\n");

	return 0;
/*
err_fall_wake:
	madera_set_irq_wake(madera, jack_irq_fall, 0);
err_fall:
	madera_free_irq(madera, jack_irq_fall, info);
err_rise_wake:
	madera_set_irq_wake(madera, jack_irq_rise, 0);
err_rise:
	madera_free_irq(madera, jack_irq_rise, info);
*/

err_hpdet:
	madera_free_irq(madera, MADERA_IRQ_HPDET, info);
err_micdet:
	madera_free_irq(madera, MADERA_IRQ_MICDET1, info);
err_input:
err_register:
	pm_runtime_disable(&pdev->dev);

	return ret;
}

static int madera_extcon_remove(struct platform_device *pdev)
{
	struct madera_extcon *info = platform_get_drvdata(pdev);
	struct madera *madera = info->madera;
	// int jack_irq_rise, jack_irq_fall;
	int ret;
	ret = acore_unreg_notifier(&info->nb);

	if (ret) {
		dev_err(&pdev->dev,
			"%s: audio switch notifier acore_unreg_notifier failed: %d\n",
			__func__, ret);
		// goto err_data;
	}

	pm_runtime_disable(&pdev->dev);

	regmap_update_bits(madera->regmap, MADERA_MICD_CLAMP_CONTROL,
			   MADERA_MICD_CLAMP_MODE_MASK, 0);
/*
	if (info->pdata->jd_use_jd2) {
		jack_irq_rise = MADERA_IRQ_MICD_CLAMP_RISE;
		jack_irq_fall = MADERA_IRQ_MICD_CLAMP_FALL;
	} else {
		jack_irq_rise = MADERA_IRQ_JD1_RISE;
		jack_irq_fall = MADERA_IRQ_JD1_FALL;
	}

	madera_set_irq_wake(madera, jack_irq_rise, 0);
	madera_set_irq_wake(madera, jack_irq_fall, 0);

	madera_free_irq(madera, jack_irq_rise, info);
	madera_free_irq(madera, jack_irq_fall, info);
	regmap_update_bits(madera->regmap, MADERA_JACK_DETECT_ANALOGUE,
			   MADERA_JD1_ENA | MADERA_JD2_ENA, 0);
	*/
	madera_free_irq(madera, MADERA_IRQ_HPDET, info);
	madera_free_irq(madera, MADERA_IRQ_MICDET1, info);

	device_remove_file(&pdev->dev, &dev_attr_hp1_impedance);

	return 0;
}

static struct platform_driver madera_extcon_driver = {
	.driver		= {
		.name	= "madera-extcon",
	},
	.probe		= madera_extcon_probe,
	.remove		= madera_extcon_remove,
};

int __init madera_extcon_driver_init(void)
{
	return platform_driver_register(&madera_extcon_driver);
}

void madera_extcon_driver_exit(void)
{
	platform_driver_unregister(&madera_extcon_driver);
}

MODULE_DESCRIPTION("Madera extcon driver");
MODULE_AUTHOR("Charles Keepax <ckeepax@opensource.wolfsonmicro.com>");
MODULE_AUTHOR("Richard Fitzgerald <rf@opensource.wolfsonmicro.com>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:extcon-madera");
