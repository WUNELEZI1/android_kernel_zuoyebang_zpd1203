// SPDX-License-Identifier: GPL-2.0
/*****************************************************************************
 *
 * Filename:
 * ---------
 *	 sc1320_mipi_raw.c
 *
 * Project:
 * --------
 *	 ALPS
 *
 * Description:
 * ------------
 *	 Source code of Sensor driver
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/types.h>
#include "sc1320cs_truly_main_i_mipi_raw.h"

#define PFX "sc1320cs_truly_main_i_mipi_raw"
#define LOG_INFO(format, args...) \
	pr_debug(PFX "[%s] " format, __func__, ##args)
#define LOG_ERR(format, args...) \
	pr_err(PFX "[%s] " format, __func__, ##args)

#define sc1320cs_SENSOR_GAIN_MAX_VALID_INDEX  5
#define sc1320cs_SENSOR_GAIN_MAP_SIZE	 5

#define sc1320cs_SENSOR_BASE_GAIN		 0x400
#define sc1320cs_SENSOR_MAX_GAIN		 (16 * sc1320cs_SENSOR_BASE_GAIN)
#define MODE_NUM				 5

#if 1
extern unsigned char fusion_id_main[96];
extern unsigned char sn_main[96];
#endif

static DEFINE_SPINLOCK(imgsensor_drv_lock);

static struct imgsensor_info_struct imgsensor_info = {
	.sensor_id = SC1320CS_TRULY_MAIN_I_SENSOR_ID,	
	.checksum_value = 0xf7375923,			/* checksum value for Camera Auto Test */

	.pre = {
		.pclk = 120000000,
		.linelength  = 1250,
		.framelength = 3200,
		.startx = 0,
		.starty = 0,
		.grabwindow_width  = 4208,
		.grabwindow_height = 3120,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
		.mipi_pixel_rate = 435200000,
	},
	.cap = {
		.pclk = 120000000,
		.linelength  = 1250,
		.framelength = 3200,
		.startx = 0,
		.starty = 0,
		.grabwindow_width  = 4208,
		.grabwindow_height = 3120,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
		.mipi_pixel_rate = 435200000,
	},
	.cap1 = {
		.pclk = 120000000,
		.linelength = 1250,
		.framelength = 3200,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 4208,
		.grabwindow_height = 3120,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
		.mipi_pixel_rate = 480000000,
	},
	.normal_video = {
		.pclk = 120000000,
		.linelength = 1250,
		.framelength = 3200,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 4208,
		.grabwindow_height = 2368,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
		.mipi_pixel_rate = 480000000,
	},
	.hs_video = {
		.pclk = 120000000,
		.linelength = 1250,
		.framelength = 800,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1280,
		.grabwindow_height = 720,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 1200,
		.mipi_pixel_rate = 480000000,
	},
	.slim_video = {
		.pclk = 120000000,
		.linelength = 1250,
		.framelength = 3200,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1920,
		.grabwindow_height = 1080,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
		.mipi_pixel_rate = 480000000,
	},

	.margin = 5,
	.min_shutter = 3,
	.min_gain = 64,
	.max_gain = 1024,
	.min_gain_iso = 50,
	.gain_step = 1,
	.gain_type = 3,
	.max_frame_length = 0xFFFF4,
	.ae_shut_delay_frame = 0,
	.ae_sensor_gain_delay_frame = 0,
	.ae_ispGain_delay_frame = 2,
	.ihdr_support = 0,
	.ihdr_le_firstline = 0,
	.sensor_mode_num = MODE_NUM,			/* support sensor mode num */

	.pre_delay_frame = 2,				/* enter preview delay frame num */
	.cap_delay_frame = 3,				/* enter capture delay frame num */
	.video_delay_frame = 2,				/* enter normal_video delay frame num */
	.hs_video_delay_frame = 2,			/* enter high_speed_video delay frame num */
	.slim_video_delay_frame = 2,			/* enter slim_video delay frame num */
	.frame_time_delay_frame = 2,

	.isp_driving_current = ISP_DRIVING_6MA,
	.sensor_interface_type = SENSOR_INTERFACE_TYPE_MIPI,
	.mipi_sensor_type = MIPI_OPHY_NCSI2,		/* 0,MIPI_OPHY_NCSI2;  1,MIPI_OPHY_CSI2 */
	.mipi_settle_delay_mode = 0,
	.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_B,
	.mclk = 24,
	.mipi_lane_num = SENSOR_MIPI_4_LANE,
	.i2c_addr_table = {0x20, 0xff},
	.i2c_speed = 400,
};

static struct imgsensor_struct imgsensor = {
	.mirror = IMAGE_HV_MIRROR,				/* mirrorflip information */
	.sensor_mode = IMGSENSOR_MODE_INIT,
	.shutter = 0x4de,
	.gain = 0x40,
	.dummy_pixel = 0,
	.dummy_line = 0,
	.current_fps = 300,
	.autoflicker_en = KAL_FALSE,
	.test_pattern = KAL_FALSE,
	.current_scenario_id = MSDK_SCENARIO_ID_CAMERA_PREVIEW,
	.i2c_write_id = 0x20,
};

/* Sensor output window information */
static struct SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[MODE_NUM] = {
	{4208, 3120, 0, 0, 4208, 3120, 4208, 3120, 0,   0, 4208, 3120, 0, 0, 4208, 3120}, /* Preview */
	{4208, 3120, 0, 0, 4208, 3120, 4208, 3120, 0,   0, 4208, 3120, 0, 0, 4208, 3120}, /* capture */
	{4208, 3120, 0, 0, 4208, 3120, 4208, 3120, 0, 376, 4208, 2368, 0, 0, 4208, 2368}, /* video */
	{4208, 3120, 0, 0, 4208, 3120, 2104, 1560, 412,   420, 1280, 720, 0, 0, 1280, 720}, /* hs video */
	{4208, 3120, 0, 0, 4208, 3120, 2104, 1560,  92,   240, 1920, 1080, 0, 0, 1920, 1080}, /* slim */
};

static struct SET_PD_BLOCK_INFO_T imgsensor_pd_info = {
	.i4OffsetX = 59,
	.i4OffsetY = 55,
	.i4PitchX = 32,
	.i4PitchY = 32,
	.i4PairNum = 8,
	.i4SubBlkW = 16,
	.i4SubBlkH = 8,
	.i4PosL = {
		{60, 56},{76, 56},{68, 68},{84, 68},{60, 72},{76, 72},{68, 84},{84, 84}},
	.i4PosR = {
		{60, 60},{76, 60},{68, 64},{84, 64},{60, 76},{76, 76},{68, 80},{84, 80}},
	//.iMirrorFlip = IMAGE_HV_MIRROR,
	.iMirrorFlip = 0,
	.i4BlockNumX = 128,
	.i4BlockNumY = 94,
	.i4Crop = { {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
};

static struct SET_PD_BLOCK_INFO_T imgsensor_pd_info_16_9 = {
	.i4OffsetX = 59,
	.i4OffsetY = 1,
	.i4PitchX = 32,
	.i4PitchY = 32,
	.i4PairNum = 8,
	.i4SubBlkW = 16,
	.i4SubBlkH = 8,
	.i4PosL = {
		{60, 2},{76, 2},{68, 14},{84, 14},{60, 18},{76, 18},{68, 30},{84, 30}},
	.i4PosR = {
		{60, 6},{76, 6},{68, 10},{84, 10},{60, 22},{76, 22},{68, 26},{84, 26}},
	//.iMirrorFlip = IMAGE_HV_MIRROR,
	.iMirrorFlip = 0,
	.i4BlockNumX = 128,
	.i4BlockNumY = 73,
	.i4Crop = { {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
};

static kal_uint16 read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte = 0;
	char pu_send_cmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF) };

	iReadRegI2C(pu_send_cmd, 2, (u8 *)&get_byte, 1, imgsensor.i2c_write_id);

	return get_byte;
}

static void write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
	char pu_send_cmd[3] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para & 0xFF)};

	iWriteRegI2C(pu_send_cmd, 3, imgsensor.i2c_write_id);
}

static void set_dummy(void)
{
	LOG_INFO("frame length = %d", imgsensor.frame_length);
	write_cmos_sensor(0x326d, (imgsensor.frame_length >> 16) & 0x7f);
	write_cmos_sensor(0x320e, (imgsensor.frame_length >> 8) & 0xff);
	write_cmos_sensor(0x320f, imgsensor.frame_length & 0xff);
}

static kal_uint32 return_sensor_id(void)
{
	return ((read_cmos_sensor(0x3107) << 8) | read_cmos_sensor(0x3108));
}

static void set_max_framerate(UINT16 framerate, kal_bool min_framelength_en)
{
	kal_uint32 frame_length = imgsensor.frame_length;

	LOG_INFO("framerate = %d, min framelength enable: %d", framerate, min_framelength_en);
	frame_length = imgsensor.pclk / framerate * 10 / imgsensor.line_length;
	spin_lock(&imgsensor_drv_lock);
	imgsensor.frame_length = (frame_length > imgsensor.min_frame_length) ?
		frame_length : imgsensor.min_frame_length;
	imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;

	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;
	if (min_framelength_en)
		imgsensor.min_frame_length = imgsensor.frame_length;
	spin_unlock(&imgsensor_drv_lock);
	set_dummy();
}

/*************************************************************************
 * FUNCTION
 *	set_shutter
 *
 * DESCRIPTION
 *	This function set e-shutter of sensor to change exposure time.
 *
 * PARAMETERS
 *	iShutter : exposured lines
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static void set_shutter(kal_uint16 shutter)
{
	kal_uint16 realtime_fps = 0;

	spin_lock(&imgsensor_drv_lock);
	if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)
		imgsensor.frame_length = shutter + imgsensor_info.margin;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);

	shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
	shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin)) ?
		(imgsensor_info.max_frame_length - imgsensor_info.margin) : shutter;

	if (imgsensor.autoflicker_en) {
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
		if (realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296, 0);
		else if (realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146, 0);
		else {
			write_cmos_sensor(0x326d, (imgsensor.frame_length >> 16) & 0x7f);
			write_cmos_sensor(0x320e, (imgsensor.frame_length >> 8) & 0xff);
			write_cmos_sensor(0x320f, imgsensor.frame_length & 0xFF);
		}
	} else {
		/* Extend frame length */
		write_cmos_sensor(0x326d, (imgsensor.frame_length >> 16) & 0x7f);
		write_cmos_sensor(0x320e, (imgsensor.frame_length >> 8) & 0xff);
		write_cmos_sensor(0x320f, imgsensor.frame_length & 0xFF);
	}
	/* Update Shutter */
	shutter = shutter * 2;
	write_cmos_sensor(0x3e20, (shutter >> 20) & 0x0F);
	write_cmos_sensor(0x3e00, (shutter >> 12) & 0xFF);
	write_cmos_sensor(0x3e01, (shutter >>  4) & 0xFF);
	write_cmos_sensor(0x3e02, (shutter <<  4) & 0xF0);
	LOG_INFO("shutter = %d, framelength = %d", shutter, imgsensor.frame_length);
}

static kal_uint16 gain2reg(const kal_uint16 gain)
{
	kal_uint16 reg_gain = gain << 4;

	if (reg_gain < sc1320cs_SENSOR_BASE_GAIN)
		reg_gain = sc1320cs_SENSOR_BASE_GAIN;
	else if (reg_gain > sc1320cs_SENSOR_MAX_GAIN)
		reg_gain = sc1320cs_SENSOR_MAX_GAIN;

	return (kal_uint16)reg_gain;
}

/*
 * FUNCTION
 *	set_gain
 *
 * DESCRIPTION
 *	This function is to set global gain to sensor.
 *
 * PARAMETERS
 *	iGain : sensor global gain(base: 0x40)
 *
 * RETURNS
 *	the actually gain set to sensor.
 *
 * GLOBALS AFFECTED
 *
 */
static kal_uint16 set_gain(kal_uint16 gain)
{
	kal_uint16 reg_gain;
	kal_uint32 temp_gain;
	kal_int16 gain_index;
	kal_uint16 sc1320cs_LY_AGC_Param[sc1320cs_SENSOR_GAIN_MAP_SIZE][2] = {
		{  1024,  0x00 },
		{  2048,  0x08 },
		{  4096,  0x09 },
		{  8192,  0x0B },
		{ 16384,  0x0f },
	};

	reg_gain = gain2reg(gain);

	for (gain_index = sc1320cs_SENSOR_GAIN_MAX_VALID_INDEX - 1; gain_index >= 0; gain_index--)
		if (reg_gain >= sc1320cs_LY_AGC_Param[gain_index][0])
			break;

	if (gain_index < 0) {
		gain_index = 0;
		LOG_ERR("there is a error change gain_index = 0");
	}

	write_cmos_sensor(0x3e09, sc1320cs_LY_AGC_Param[gain_index][1]);
	temp_gain = reg_gain * sc1320cs_SENSOR_BASE_GAIN / sc1320cs_LY_AGC_Param[gain_index][0];
	write_cmos_sensor(0x3e07, (temp_gain >> 3) & 0xff);
	LOG_INFO("LY_AGC_Param[gain_index][1] = 0x%x, temp_gain = 0x%x, reg_gain = %d, gain = %d",
		sc1320cs_LY_AGC_Param[gain_index][1], temp_gain, reg_gain, gain);

	return reg_gain;
}

static void ihdr_write_shutter_gain(kal_uint16 le, kal_uint16 se, kal_uint16 gain)
{
	LOG_INFO("le: 0x%x, se: 0x%x, gain: 0x%x", le, se, gain);
}

/*
 * FUNCTION
 *	night_mode
 *
 * DESCRIPTION
 *	This function night mode of sensor.
 *
 * PARAMETERS
 *	bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 */
static void night_mode(kal_bool enable)
{
	/* No Need to implement this function */
}

static void sensor_init(void)
{
	LOG_INFO("%s()", __func__);
	write_cmos_sensor(0x0103,0x01);
	write_cmos_sensor(0x0100,0x00);
	write_cmos_sensor(0x36e9,0x80);
	write_cmos_sensor(0x37f9,0x80);
	write_cmos_sensor(0x36ea,0x11);
	write_cmos_sensor(0x36eb,0x0c);
	write_cmos_sensor(0x36ec,0x4b);
	write_cmos_sensor(0x36ed,0x18);
	write_cmos_sensor(0x37fa,0x0f);
	write_cmos_sensor(0x37fb,0xb2);
	write_cmos_sensor(0x37fc,0x10);
	write_cmos_sensor(0x37fd,0x5a);
	write_cmos_sensor(0x301f,0x49);
	write_cmos_sensor(0x320c,0x04);
	write_cmos_sensor(0x320d,0xe2);
	write_cmos_sensor(0x320e,0x0c);
	write_cmos_sensor(0x320f,0x80);
	write_cmos_sensor(0x3279,0x10);
	write_cmos_sensor(0x3301,0x07);
	write_cmos_sensor(0x3302,0x18);
	write_cmos_sensor(0x3304,0x38);
	write_cmos_sensor(0x3306,0x58);
	write_cmos_sensor(0x3308,0x0c);
	write_cmos_sensor(0x3309,0x80);
	write_cmos_sensor(0x330b,0xb8);
	write_cmos_sensor(0x330d,0x10);
	write_cmos_sensor(0x330e,0x2b);
	write_cmos_sensor(0x3314,0x15);
	write_cmos_sensor(0x331c,0x04);
	write_cmos_sensor(0x331e,0x29);
	write_cmos_sensor(0x331f,0x71);
	write_cmos_sensor(0x3320,0x08);
	write_cmos_sensor(0x3323,0x04);
	write_cmos_sensor(0x3333,0x10);
	write_cmos_sensor(0x3334,0x40);
	write_cmos_sensor(0x3351,0x06);
	write_cmos_sensor(0x335d,0x60);
	write_cmos_sensor(0x3364,0x56);
	write_cmos_sensor(0x337f,0x13);
	write_cmos_sensor(0x3390,0x09);
	write_cmos_sensor(0x3391,0x0f);
	write_cmos_sensor(0x3392,0x1f);
	write_cmos_sensor(0x3393,0x10);
	write_cmos_sensor(0x3394,0x1c);
	write_cmos_sensor(0x3395,0x20);
	write_cmos_sensor(0x33ac,0x0a);
	write_cmos_sensor(0x33ad,0x14);
	write_cmos_sensor(0x33ae,0x14);
	write_cmos_sensor(0x33af,0x60);
	write_cmos_sensor(0x33b0,0x0f);
	write_cmos_sensor(0x33b2,0x50);
	write_cmos_sensor(0x33b3,0x28);
	write_cmos_sensor(0x349f,0x1e);
	write_cmos_sensor(0x34a6,0x08);
	write_cmos_sensor(0x34a7,0x09);
	write_cmos_sensor(0x34a8,0x18);
	write_cmos_sensor(0x34a9,0x18);
	write_cmos_sensor(0x34f8,0x0f);
	write_cmos_sensor(0x34f9,0x18);
	write_cmos_sensor(0x3630,0xc6);
	write_cmos_sensor(0x3637,0x47);
	write_cmos_sensor(0x363c,0x8d);
	write_cmos_sensor(0x3670,0x0c);
	write_cmos_sensor(0x367b,0x56);
	write_cmos_sensor(0x367c,0x67);
	write_cmos_sensor(0x367d,0x67);
	write_cmos_sensor(0x367e,0x08);
	write_cmos_sensor(0x367f,0x0f);
	write_cmos_sensor(0x3690,0x84);
	write_cmos_sensor(0x3691,0x87);
	write_cmos_sensor(0x3692,0x8e);
	write_cmos_sensor(0x3693,0xb4);
	write_cmos_sensor(0x3694,0x09);
	write_cmos_sensor(0x3695,0x0b);
	write_cmos_sensor(0x3696,0x0f);
	write_cmos_sensor(0x3697,0x8b);
	write_cmos_sensor(0x3698,0x8d);
	write_cmos_sensor(0x3699,0x8d);
	write_cmos_sensor(0x369a,0x08);
	write_cmos_sensor(0x369b,0x0f);
	write_cmos_sensor(0x370f,0x01);
	write_cmos_sensor(0x3724,0x41);
	write_cmos_sensor(0x3771,0x03);
	write_cmos_sensor(0x3772,0x03);
	write_cmos_sensor(0x3773,0x63);
	write_cmos_sensor(0x377a,0x08);
	write_cmos_sensor(0x377b,0x0f);
	write_cmos_sensor(0x3900,0x0d);
	write_cmos_sensor(0x3903,0x20);
	write_cmos_sensor(0x3905,0x0c);
	write_cmos_sensor(0x3908,0x40);
	write_cmos_sensor(0x391a,0x30);
	write_cmos_sensor(0x391b,0x21);
	write_cmos_sensor(0x391c,0x13);
	write_cmos_sensor(0x391d,0x00);
	write_cmos_sensor(0x391f,0x41);
	write_cmos_sensor(0x3926,0xe0);
	write_cmos_sensor(0x393f,0x80);
	write_cmos_sensor(0x3940,0x00);
	write_cmos_sensor(0x3941,0x00);
	write_cmos_sensor(0x3942,0x00);
	write_cmos_sensor(0x3943,0x7c);
	write_cmos_sensor(0x3944,0x79);
	write_cmos_sensor(0x3945,0x7d);
	write_cmos_sensor(0x3946,0x7d);
	write_cmos_sensor(0x39dd,0x00);
	write_cmos_sensor(0x39de,0x04);
	write_cmos_sensor(0x39e7,0x04);
	write_cmos_sensor(0x39e8,0x03);
	write_cmos_sensor(0x39e9,0x80);
	write_cmos_sensor(0x3e00,0x01);
	write_cmos_sensor(0x3e01,0x8f);
	write_cmos_sensor(0x3e02,0x90);
	write_cmos_sensor(0x3f08,0x0a);
	write_cmos_sensor(0x4401,0x13);
	write_cmos_sensor(0x4402,0x03);
	write_cmos_sensor(0x4403,0x0c);
	write_cmos_sensor(0x4404,0x24);
	write_cmos_sensor(0x4405,0x2f);
	write_cmos_sensor(0x440c,0x3c);
	write_cmos_sensor(0x440d,0x3c);
	write_cmos_sensor(0x440e,0x2d);
	write_cmos_sensor(0x440f,0x4b);
	write_cmos_sensor(0x4413,0x01);
	write_cmos_sensor(0x441b,0x18);
	write_cmos_sensor(0x4509,0x18);
	write_cmos_sensor(0x450d,0x13);
	write_cmos_sensor(0x4800,0x24);
	write_cmos_sensor(0x4837,0x1d);
	write_cmos_sensor(0x5000,0x1e);
	write_cmos_sensor(0x5002,0x00);
	write_cmos_sensor(0x5015,0x41);
	write_cmos_sensor(0x550e,0x00);
	write_cmos_sensor(0x550f,0x7a);
	write_cmos_sensor(0x5780,0x76);
	write_cmos_sensor(0x5784,0x08);
	write_cmos_sensor(0x5787,0x0a);
	write_cmos_sensor(0x5788,0x0a);
	write_cmos_sensor(0x5789,0x0a);
	write_cmos_sensor(0x578a,0x0a);
	write_cmos_sensor(0x578b,0x0a);
	write_cmos_sensor(0x578c,0x0a);
	write_cmos_sensor(0x578d,0x40);
	write_cmos_sensor(0x5790,0x08);
	write_cmos_sensor(0x5791,0x04);
	write_cmos_sensor(0x5792,0x04);
	write_cmos_sensor(0x5793,0x08);
	write_cmos_sensor(0x5794,0x04);
	write_cmos_sensor(0x5795,0x04);
	write_cmos_sensor(0x5799,0x44);
	write_cmos_sensor(0x57a6,0x1c);
	write_cmos_sensor(0x57aa,0xeb);
	write_cmos_sensor(0x57ab,0x7f);
	write_cmos_sensor(0x57ac,0x00);
 	write_cmos_sensor(0x57ad,0x00);
	write_cmos_sensor(0x58e0,0xb4);
	write_cmos_sensor(0x36e9,0x14);
	write_cmos_sensor(0x37f9,0x24);

	write_cmos_sensor(0x3652,0x13);
	write_cmos_sensor(0x3653,0x31);
	write_cmos_sensor(0x3654,0x32);
 	write_cmos_sensor(0x3221,0x66);
}

static void preview_setting(void)
{
	write_cmos_sensor(0x0103,0x01);
	write_cmos_sensor(0x0100,0x00);
	write_cmos_sensor(0x36e9,0x80);
	write_cmos_sensor(0x37f9,0x80);
	write_cmos_sensor(0x36ea,0x11);
	write_cmos_sensor(0x36eb,0x0c);
	write_cmos_sensor(0x36ec,0x4b);
	write_cmos_sensor(0x36ed,0x18);
	write_cmos_sensor(0x37fa,0x0f);
	write_cmos_sensor(0x37fb,0xb2);
	write_cmos_sensor(0x37fc,0x10);
	write_cmos_sensor(0x37fd,0x5a);
	write_cmos_sensor(0x301f,0x49);
	write_cmos_sensor(0x320c,0x04);
	write_cmos_sensor(0x320d,0xe2);
	write_cmos_sensor(0x320e,0x0c);
	write_cmos_sensor(0x320f,0x80);
	write_cmos_sensor(0x3279,0x10);
	write_cmos_sensor(0x3301,0x07);
	write_cmos_sensor(0x3302,0x18);
	write_cmos_sensor(0x3304,0x38);
	write_cmos_sensor(0x3306,0x58);
	write_cmos_sensor(0x3308,0x0c);
	write_cmos_sensor(0x3309,0x80);
	write_cmos_sensor(0x330b,0xb8);
	write_cmos_sensor(0x330d,0x10);
	write_cmos_sensor(0x330e,0x2b);
	write_cmos_sensor(0x3314,0x15);
	write_cmos_sensor(0x331c,0x04);
	write_cmos_sensor(0x331e,0x29);
	write_cmos_sensor(0x331f,0x71);
	write_cmos_sensor(0x3320,0x08);
	write_cmos_sensor(0x3323,0x04);
	write_cmos_sensor(0x3333,0x10);
	write_cmos_sensor(0x3334,0x40);
	write_cmos_sensor(0x3351,0x06);
	write_cmos_sensor(0x335d,0x60);
	write_cmos_sensor(0x3364,0x56);
	write_cmos_sensor(0x337f,0x13);
	write_cmos_sensor(0x3390,0x09);
	write_cmos_sensor(0x3391,0x0f);
	write_cmos_sensor(0x3392,0x1f);
	write_cmos_sensor(0x3393,0x10);
	write_cmos_sensor(0x3394,0x1c);
	write_cmos_sensor(0x3395,0x20);
	write_cmos_sensor(0x33ac,0x0a);
	write_cmos_sensor(0x33ad,0x14);
	write_cmos_sensor(0x33ae,0x14);
	write_cmos_sensor(0x33af,0x60);
	write_cmos_sensor(0x33b0,0x0f);
	write_cmos_sensor(0x33b2,0x50);
	write_cmos_sensor(0x33b3,0x28);
	write_cmos_sensor(0x349f,0x1e);
	write_cmos_sensor(0x34a6,0x08);
	write_cmos_sensor(0x34a7,0x09);
	write_cmos_sensor(0x34a8,0x18);
	write_cmos_sensor(0x34a9,0x18);
	write_cmos_sensor(0x34f8,0x0f);
	write_cmos_sensor(0x34f9,0x18);
	write_cmos_sensor(0x3630,0xc6);
	write_cmos_sensor(0x3637,0x47);
	write_cmos_sensor(0x363c,0x8d);
	write_cmos_sensor(0x3670,0x0c);
	write_cmos_sensor(0x367b,0x56);
	write_cmos_sensor(0x367c,0x67);
	write_cmos_sensor(0x367d,0x67);
	write_cmos_sensor(0x367e,0x08);
	write_cmos_sensor(0x367f,0x0f);
	write_cmos_sensor(0x3690,0x84);
	write_cmos_sensor(0x3691,0x87);
	write_cmos_sensor(0x3692,0x8e);
	write_cmos_sensor(0x3693,0xb4);
	write_cmos_sensor(0x3694,0x09);
	write_cmos_sensor(0x3695,0x0b);
	write_cmos_sensor(0x3696,0x0f);
	write_cmos_sensor(0x3697,0x8b);
	write_cmos_sensor(0x3698,0x8d);
	write_cmos_sensor(0x3699,0x8d);
	write_cmos_sensor(0x369a,0x08);
	write_cmos_sensor(0x369b,0x0f);
	write_cmos_sensor(0x370f,0x01);
	write_cmos_sensor(0x3724,0x41);
	write_cmos_sensor(0x3771,0x03);
	write_cmos_sensor(0x3772,0x03);
	write_cmos_sensor(0x3773,0x63);
	write_cmos_sensor(0x377a,0x08);
	write_cmos_sensor(0x377b,0x0f);
	write_cmos_sensor(0x3900,0x0d);
	write_cmos_sensor(0x3903,0x20);
	write_cmos_sensor(0x3905,0x0c);
	write_cmos_sensor(0x3908,0x40);
	write_cmos_sensor(0x391a,0x30);
	write_cmos_sensor(0x391b,0x21);
	write_cmos_sensor(0x391c,0x13);
	write_cmos_sensor(0x391d,0x00);
	write_cmos_sensor(0x391f,0x41);
	write_cmos_sensor(0x3926,0xe0);
	write_cmos_sensor(0x393f,0x80);
	write_cmos_sensor(0x3940,0x00);
	write_cmos_sensor(0x3941,0x00);
	write_cmos_sensor(0x3942,0x00);
	write_cmos_sensor(0x3943,0x7c);
	write_cmos_sensor(0x3944,0x79);
	write_cmos_sensor(0x3945,0x7d);
	write_cmos_sensor(0x3946,0x7d);
	write_cmos_sensor(0x39dd,0x00);
	write_cmos_sensor(0x39de,0x04);
	write_cmos_sensor(0x39e7,0x04);
	write_cmos_sensor(0x39e8,0x03);
	write_cmos_sensor(0x39e9,0x80);
	write_cmos_sensor(0x3e00,0x01);
	write_cmos_sensor(0x3e01,0x8f);
	write_cmos_sensor(0x3e02,0x90);
	write_cmos_sensor(0x3f08,0x0a);
	write_cmos_sensor(0x4401,0x13);
	write_cmos_sensor(0x4402,0x03);
	write_cmos_sensor(0x4403,0x0c);
	write_cmos_sensor(0x4404,0x24);
	write_cmos_sensor(0x4405,0x2f);
	write_cmos_sensor(0x440c,0x3c);
	write_cmos_sensor(0x440d,0x3c);
	write_cmos_sensor(0x440e,0x2d);
	write_cmos_sensor(0x440f,0x4b);
	write_cmos_sensor(0x4413,0x01);
	write_cmos_sensor(0x441b,0x18);
	write_cmos_sensor(0x4509,0x18);
	write_cmos_sensor(0x450d,0x13);
	write_cmos_sensor(0x4800,0x24);
	write_cmos_sensor(0x4837,0x1d);
	write_cmos_sensor(0x5000,0x1e);
	write_cmos_sensor(0x5002,0x00);
	write_cmos_sensor(0x5015,0x41);
	write_cmos_sensor(0x550e,0x00);
	write_cmos_sensor(0x550f,0x7a);
	write_cmos_sensor(0x5780,0x76);
	write_cmos_sensor(0x5784,0x08);
	write_cmos_sensor(0x5787,0x0a);
	write_cmos_sensor(0x5788,0x0a);
	write_cmos_sensor(0x5789,0x0a);
	write_cmos_sensor(0x578a,0x0a);
	write_cmos_sensor(0x578b,0x0a);
	write_cmos_sensor(0x578c,0x0a);
	write_cmos_sensor(0x578d,0x40);
	write_cmos_sensor(0x5790,0x08);
	write_cmos_sensor(0x5791,0x04);
	write_cmos_sensor(0x5792,0x04);
	write_cmos_sensor(0x5793,0x08);
	write_cmos_sensor(0x5794,0x04);
	write_cmos_sensor(0x5795,0x04);
	write_cmos_sensor(0x5799,0x44);
	write_cmos_sensor(0x57a6,0x1c);
	write_cmos_sensor(0x57aa,0xeb);
	write_cmos_sensor(0x57ab,0x7f);
	write_cmos_sensor(0x57ac,0x00);
 	write_cmos_sensor(0x57ad,0x00);
	write_cmos_sensor(0x58e0,0xb4);
	write_cmos_sensor(0x36e9,0x14);
	write_cmos_sensor(0x37f9,0x24);

	write_cmos_sensor(0x3652,0x13);
	write_cmos_sensor(0x3653,0x31);
	write_cmos_sensor(0x3654,0x32);
 	write_cmos_sensor(0x3221,0x66);
}

static void capture_setting(void)
{
	write_cmos_sensor(0x0103,0x01);
	write_cmos_sensor(0x0100,0x00);
	write_cmos_sensor(0x36e9,0x80);
	write_cmos_sensor(0x37f9,0x80);
	write_cmos_sensor(0x36ea,0x11);
	write_cmos_sensor(0x36eb,0x0c);
	write_cmos_sensor(0x36ec,0x4b);
	write_cmos_sensor(0x36ed,0x18);
	write_cmos_sensor(0x37fa,0x0f);
	write_cmos_sensor(0x37fb,0xb2);
	write_cmos_sensor(0x37fc,0x10);
	write_cmos_sensor(0x37fd,0x5a);
	write_cmos_sensor(0x301f,0x49);
	write_cmos_sensor(0x320c,0x04);
	write_cmos_sensor(0x320d,0xe2);
	write_cmos_sensor(0x320e,0x0c);
	write_cmos_sensor(0x320f,0x80);
	write_cmos_sensor(0x3279,0x10);
	write_cmos_sensor(0x3301,0x07);
	write_cmos_sensor(0x3302,0x18);
	write_cmos_sensor(0x3304,0x38);
	write_cmos_sensor(0x3306,0x58);
	write_cmos_sensor(0x3308,0x0c);
	write_cmos_sensor(0x3309,0x80);
	write_cmos_sensor(0x330b,0xb8);
	write_cmos_sensor(0x330d,0x10);
	write_cmos_sensor(0x330e,0x2b);
	write_cmos_sensor(0x3314,0x15);
	write_cmos_sensor(0x331c,0x04);
	write_cmos_sensor(0x331e,0x29);
	write_cmos_sensor(0x331f,0x71);
	write_cmos_sensor(0x3320,0x08);
	write_cmos_sensor(0x3323,0x04);
	write_cmos_sensor(0x3333,0x10);
	write_cmos_sensor(0x3334,0x40);
	write_cmos_sensor(0x3351,0x06);
	write_cmos_sensor(0x335d,0x60);
	write_cmos_sensor(0x3364,0x56);
	write_cmos_sensor(0x337f,0x13);
	write_cmos_sensor(0x3390,0x09);
	write_cmos_sensor(0x3391,0x0f);
	write_cmos_sensor(0x3392,0x1f);
	write_cmos_sensor(0x3393,0x10);
	write_cmos_sensor(0x3394,0x1c);
	write_cmos_sensor(0x3395,0x20);
	write_cmos_sensor(0x33ac,0x0a);
	write_cmos_sensor(0x33ad,0x14);
	write_cmos_sensor(0x33ae,0x14);
	write_cmos_sensor(0x33af,0x60);
	write_cmos_sensor(0x33b0,0x0f);
	write_cmos_sensor(0x33b2,0x50);
	write_cmos_sensor(0x33b3,0x28);
	write_cmos_sensor(0x349f,0x1e);
	write_cmos_sensor(0x34a6,0x08);
	write_cmos_sensor(0x34a7,0x09);
	write_cmos_sensor(0x34a8,0x18);
	write_cmos_sensor(0x34a9,0x18);
	write_cmos_sensor(0x34f8,0x0f);
	write_cmos_sensor(0x34f9,0x18);
	write_cmos_sensor(0x3630,0xc6);
	write_cmos_sensor(0x3637,0x47);
	write_cmos_sensor(0x363c,0x8d);
	write_cmos_sensor(0x3670,0x0c);
	write_cmos_sensor(0x367b,0x56);
	write_cmos_sensor(0x367c,0x67);
	write_cmos_sensor(0x367d,0x67);
	write_cmos_sensor(0x367e,0x08);
	write_cmos_sensor(0x367f,0x0f);
	write_cmos_sensor(0x3690,0x84);
	write_cmos_sensor(0x3691,0x87);
	write_cmos_sensor(0x3692,0x8e);
	write_cmos_sensor(0x3693,0xb4);
	write_cmos_sensor(0x3694,0x09);
	write_cmos_sensor(0x3695,0x0b);
	write_cmos_sensor(0x3696,0x0f);
	write_cmos_sensor(0x3697,0x8b);
	write_cmos_sensor(0x3698,0x8d);
	write_cmos_sensor(0x3699,0x8d);
	write_cmos_sensor(0x369a,0x08);
	write_cmos_sensor(0x369b,0x0f);
	write_cmos_sensor(0x370f,0x01);
	write_cmos_sensor(0x3724,0x41);
	write_cmos_sensor(0x3771,0x03);
	write_cmos_sensor(0x3772,0x03);
	write_cmos_sensor(0x3773,0x63);
	write_cmos_sensor(0x377a,0x08);
	write_cmos_sensor(0x377b,0x0f);
	write_cmos_sensor(0x3900,0x0d);
	write_cmos_sensor(0x3903,0x20);
	write_cmos_sensor(0x3905,0x0c);
	write_cmos_sensor(0x3908,0x40);
	write_cmos_sensor(0x391a,0x30);
	write_cmos_sensor(0x391b,0x21);
	write_cmos_sensor(0x391c,0x13);
	write_cmos_sensor(0x391d,0x00);
	write_cmos_sensor(0x391f,0x41);
	write_cmos_sensor(0x3926,0xe0);
	write_cmos_sensor(0x393f,0x80);
	write_cmos_sensor(0x3940,0x00);
	write_cmos_sensor(0x3941,0x00);
	write_cmos_sensor(0x3942,0x00);
	write_cmos_sensor(0x3943,0x7c);
	write_cmos_sensor(0x3944,0x79);
	write_cmos_sensor(0x3945,0x7d);
	write_cmos_sensor(0x3946,0x7d);
	write_cmos_sensor(0x39dd,0x00);
	write_cmos_sensor(0x39de,0x04);
	write_cmos_sensor(0x39e7,0x04);
	write_cmos_sensor(0x39e8,0x03);
	write_cmos_sensor(0x39e9,0x80);
	write_cmos_sensor(0x3e00,0x01);
	write_cmos_sensor(0x3e01,0x8f);
	write_cmos_sensor(0x3e02,0x90);
	write_cmos_sensor(0x3f08,0x0a);
	write_cmos_sensor(0x4401,0x13);
	write_cmos_sensor(0x4402,0x03);
	write_cmos_sensor(0x4403,0x0c);
	write_cmos_sensor(0x4404,0x24);
	write_cmos_sensor(0x4405,0x2f);
	write_cmos_sensor(0x440c,0x3c);
	write_cmos_sensor(0x440d,0x3c);
	write_cmos_sensor(0x440e,0x2d);
	write_cmos_sensor(0x440f,0x4b);
	write_cmos_sensor(0x4413,0x01);
	write_cmos_sensor(0x441b,0x18);
	write_cmos_sensor(0x4509,0x18);
	write_cmos_sensor(0x450d,0x13);
	write_cmos_sensor(0x4800,0x24);
	write_cmos_sensor(0x4837,0x1d);
	write_cmos_sensor(0x5000,0x1e);
	write_cmos_sensor(0x5002,0x00);
	write_cmos_sensor(0x5015,0x41);
	write_cmos_sensor(0x550e,0x00);
	write_cmos_sensor(0x550f,0x7a);
	write_cmos_sensor(0x5780,0x76);
	write_cmos_sensor(0x5784,0x08);
	write_cmos_sensor(0x5787,0x0a);
	write_cmos_sensor(0x5788,0x0a);
	write_cmos_sensor(0x5789,0x0a);
	write_cmos_sensor(0x578a,0x0a);
	write_cmos_sensor(0x578b,0x0a);
	write_cmos_sensor(0x578c,0x0a);
	write_cmos_sensor(0x578d,0x40);
	write_cmos_sensor(0x5790,0x08);
	write_cmos_sensor(0x5791,0x04);
	write_cmos_sensor(0x5792,0x04);
	write_cmos_sensor(0x5793,0x08);
	write_cmos_sensor(0x5794,0x04);
	write_cmos_sensor(0x5795,0x04);
	write_cmos_sensor(0x5799,0x44);
	write_cmos_sensor(0x57a6,0x1c);
	write_cmos_sensor(0x57aa,0xeb);
	write_cmos_sensor(0x57ab,0x7f);
	write_cmos_sensor(0x57ac,0x00);
	write_cmos_sensor(0x57ad,0x00);
	write_cmos_sensor(0x58e0,0xb4);
	write_cmos_sensor(0x36e9,0x14);
	write_cmos_sensor(0x37f9,0x24);

	write_cmos_sensor(0x3652,0x13);
	write_cmos_sensor(0x3653,0x31);
	write_cmos_sensor(0x3654,0x32);
 	write_cmos_sensor(0x3221,0x66);
}

static void normal_video_setting(void)
{
    write_cmos_sensor(0x0103,0x01);
	write_cmos_sensor(0x0100,0x00);
	write_cmos_sensor(0x36e9,0x80);
	write_cmos_sensor(0x37f9,0x80);
	write_cmos_sensor(0x36e9,0x24);
	write_cmos_sensor(0x37f9,0x24);
	write_cmos_sensor(0x301f,0x1b);
	write_cmos_sensor(0x3200,0x00);
	write_cmos_sensor(0x3201,0x00);
	write_cmos_sensor(0x3202,0x01);
	write_cmos_sensor(0x3203,0x7a);
	write_cmos_sensor(0x3204,0x10);
	write_cmos_sensor(0x3205,0x7f);
	write_cmos_sensor(0x3206,0x0a);
	write_cmos_sensor(0x3207,0xc9);
	write_cmos_sensor(0x3208,0x10);
	write_cmos_sensor(0x3209,0x70);
	write_cmos_sensor(0x320a,0x09);
	write_cmos_sensor(0x320b,0x40);
	write_cmos_sensor(0x3210,0x00);
	write_cmos_sensor(0x3211,0x08);
	write_cmos_sensor(0x3212,0x00);
	write_cmos_sensor(0x3213,0x08);
	write_cmos_sensor(0x320c,0x04);
	write_cmos_sensor(0x320d,0xe2);
	write_cmos_sensor(0x320e,0x0c);
	write_cmos_sensor(0x320f,0x80);
	write_cmos_sensor(0x3279,0x10);
	write_cmos_sensor(0x3301,0x07);
	write_cmos_sensor(0x3302,0x18);
	write_cmos_sensor(0x3304,0x38);
	write_cmos_sensor(0x3306,0x58);
	write_cmos_sensor(0x3308,0x0c);
	write_cmos_sensor(0x3309,0x80);
	write_cmos_sensor(0x330b,0xb8);
	write_cmos_sensor(0x330d,0x10);
	write_cmos_sensor(0x330e,0x2b);
	write_cmos_sensor(0x3314,0x15);
	write_cmos_sensor(0x331c,0x04);
	write_cmos_sensor(0x331e,0x29);
	write_cmos_sensor(0x331f,0x71);
	write_cmos_sensor(0x3320,0x08);
	write_cmos_sensor(0x3323,0x04);
	write_cmos_sensor(0x3333,0x10);
	write_cmos_sensor(0x3334,0x40);
	write_cmos_sensor(0x3351,0x06);
	write_cmos_sensor(0x335d,0x60);
	write_cmos_sensor(0x3364,0x56);
	write_cmos_sensor(0x337f,0x13);
	write_cmos_sensor(0x3390,0x09);
	write_cmos_sensor(0x3391,0x0f);
	write_cmos_sensor(0x3392,0x1f);
	write_cmos_sensor(0x3393,0x10);
	write_cmos_sensor(0x3394,0x1c);
	write_cmos_sensor(0x3395,0x20);
	write_cmos_sensor(0x33ac,0x0a);
	write_cmos_sensor(0x33ad,0x14);
	write_cmos_sensor(0x33ae,0x14);
	write_cmos_sensor(0x33af,0x60);
	write_cmos_sensor(0x33b0,0x0f);
	write_cmos_sensor(0x33b2,0x50);
	write_cmos_sensor(0x33b3,0x28);
	write_cmos_sensor(0x349f,0x1e);
	write_cmos_sensor(0x34a6,0x08);
	write_cmos_sensor(0x34a7,0x09);
	write_cmos_sensor(0x34a8,0x18);
	write_cmos_sensor(0x34a9,0x18);
	write_cmos_sensor(0x34f8,0x0f);
	write_cmos_sensor(0x34f9,0x18);
	write_cmos_sensor(0x3630,0xc6);
	write_cmos_sensor(0x3637,0x47);
	write_cmos_sensor(0x363c,0x8d);
	write_cmos_sensor(0x3670,0x0c);
	write_cmos_sensor(0x367b,0x56);
	write_cmos_sensor(0x367c,0x67);
	write_cmos_sensor(0x367d,0x67);
	write_cmos_sensor(0x367e,0x08);
	write_cmos_sensor(0x367f,0x0f);
	write_cmos_sensor(0x3690,0x84);
	write_cmos_sensor(0x3691,0x87);
	write_cmos_sensor(0x3692,0x8e);
	write_cmos_sensor(0x3693,0xb4);
	write_cmos_sensor(0x3694,0x09);
	write_cmos_sensor(0x3695,0x0b);
	write_cmos_sensor(0x3696,0x0f);
	write_cmos_sensor(0x3697,0x8b);
	write_cmos_sensor(0x3698,0x8d);
	write_cmos_sensor(0x3699,0x8d);
	write_cmos_sensor(0x369a,0x08);
	write_cmos_sensor(0x369b,0x0f);
	write_cmos_sensor(0x370f,0x01);
	write_cmos_sensor(0x3724,0x41);
	write_cmos_sensor(0x3771,0x03);
	write_cmos_sensor(0x3772,0x03);
	write_cmos_sensor(0x3773,0x63);
	write_cmos_sensor(0x377a,0x08);
	write_cmos_sensor(0x377b,0x0f);
	write_cmos_sensor(0x3900,0x0d);
	write_cmos_sensor(0x3903,0x20);
	write_cmos_sensor(0x3905,0x0c);
	write_cmos_sensor(0x3908,0x40);
	write_cmos_sensor(0x391a,0x30);
	write_cmos_sensor(0x391b,0x21);
	write_cmos_sensor(0x391c,0x13);
	write_cmos_sensor(0x391d,0x00);
	write_cmos_sensor(0x391f,0x41);
	write_cmos_sensor(0x3926,0xe0);
	write_cmos_sensor(0x393f,0x80);
	write_cmos_sensor(0x3940,0x00);
	write_cmos_sensor(0x3941,0x00);
	write_cmos_sensor(0x3942,0x00);
	write_cmos_sensor(0x3943,0x7c);
	write_cmos_sensor(0x3944,0x79);
	write_cmos_sensor(0x3945,0x7d);
	write_cmos_sensor(0x3946,0x7d);
	write_cmos_sensor(0x39dd,0x00);
	write_cmos_sensor(0x39de,0x04);
	write_cmos_sensor(0x39e7,0x04);
	write_cmos_sensor(0x39e8,0x03);
	write_cmos_sensor(0x39e9,0x80);
	write_cmos_sensor(0x3e00,0x01);
	write_cmos_sensor(0x3e01,0x8f);
	write_cmos_sensor(0x3e02,0x90);
	write_cmos_sensor(0x3f08,0x0a);
	write_cmos_sensor(0x4401,0x13);
	write_cmos_sensor(0x4402,0x03);
	write_cmos_sensor(0x4403,0x0c);
	write_cmos_sensor(0x4404,0x24);
	write_cmos_sensor(0x4405,0x2f);
	write_cmos_sensor(0x440c,0x3c);
	write_cmos_sensor(0x440d,0x3c);
	write_cmos_sensor(0x440e,0x2d);
	write_cmos_sensor(0x440f,0x4b);
	write_cmos_sensor(0x4413,0x01);
	write_cmos_sensor(0x441b,0x18);
	write_cmos_sensor(0x4509,0x18);
	write_cmos_sensor(0x450d,0x13);
	write_cmos_sensor(0x4800,0x24);
	write_cmos_sensor(0x5000,0x1e);
	write_cmos_sensor(0x5002,0x00);
	write_cmos_sensor(0x5015,0x41);
	write_cmos_sensor(0x550e,0x00);
	write_cmos_sensor(0x550f,0x7a);
	write_cmos_sensor(0x5780,0x76);
	write_cmos_sensor(0x5784,0x08);
	write_cmos_sensor(0x5787,0x0a);
	write_cmos_sensor(0x5788,0x0a);
	write_cmos_sensor(0x5789,0x0a);
	write_cmos_sensor(0x578a,0x0a);
	write_cmos_sensor(0x578b,0x0a);
	write_cmos_sensor(0x578c,0x0a);
	write_cmos_sensor(0x578d,0x40);
	write_cmos_sensor(0x5790,0x08);
	write_cmos_sensor(0x5791,0x04);
	write_cmos_sensor(0x5792,0x04);
	write_cmos_sensor(0x5793,0x08);
	write_cmos_sensor(0x5794,0x04);
	write_cmos_sensor(0x5795,0x04);
	write_cmos_sensor(0x5799,0x44);
	write_cmos_sensor(0x57a6,0x1c);
	write_cmos_sensor(0x57aa,0xeb);
	write_cmos_sensor(0x57ab,0x7f);
	write_cmos_sensor(0x57ac,0x00);
	write_cmos_sensor(0x57ad,0x00);
	write_cmos_sensor(0x58e0,0xb4);
 	write_cmos_sensor(0x3221,0x66);
}

static void hs_video_setting(void)
{
    write_cmos_sensor(0x0103,0x01);
	write_cmos_sensor(0x0100,0x00);
	write_cmos_sensor(0x36e9,0x80);
	write_cmos_sensor(0x37f9,0x80);
	write_cmos_sensor(0x36e9,0x24);
	write_cmos_sensor(0x37f9,0x24);
	write_cmos_sensor(0x301f,0x29);
	write_cmos_sensor(0x3200,0x00);
	write_cmos_sensor(0x3201,0x00);
	write_cmos_sensor(0x3202,0x03);
	write_cmos_sensor(0x3203,0x48);
	write_cmos_sensor(0x3204,0x10);
	write_cmos_sensor(0x3205,0x7f);
	write_cmos_sensor(0x3206,0x08);
	write_cmos_sensor(0x3207,0xf7);
	write_cmos_sensor(0x3208,0x05);
	write_cmos_sensor(0x3209,0x00);
	write_cmos_sensor(0x320a,0x02);
	write_cmos_sensor(0x320b,0xd0);
	write_cmos_sensor(0x320c,0x04);
	write_cmos_sensor(0x320d,0xe2);
	write_cmos_sensor(0x320e,0x03);
	write_cmos_sensor(0x320f,0x20);
	write_cmos_sensor(0x3210,0x01);
	write_cmos_sensor(0x3211,0xa0);
	write_cmos_sensor(0x3212,0x00);
	write_cmos_sensor(0x3213,0x04);
	write_cmos_sensor(0x3215,0x31);
	write_cmos_sensor(0x3220,0x01);
	write_cmos_sensor(0x3279,0x10);
	write_cmos_sensor(0x3301,0x07);
	write_cmos_sensor(0x3302,0x18);
	write_cmos_sensor(0x3304,0x38);
	write_cmos_sensor(0x3306,0x58);
	write_cmos_sensor(0x3308,0x0c);
	write_cmos_sensor(0x3309,0x80);
	write_cmos_sensor(0x330b,0xb8);
	write_cmos_sensor(0x330d,0x10);
	write_cmos_sensor(0x330e,0x2b);
	write_cmos_sensor(0x3314,0x15);
	write_cmos_sensor(0x331c,0x04);
	write_cmos_sensor(0x331e,0x29);
	write_cmos_sensor(0x331f,0x71);
	write_cmos_sensor(0x3320,0x08);
	write_cmos_sensor(0x3323,0x04);
	write_cmos_sensor(0x3333,0x10);
	write_cmos_sensor(0x3334,0x40);
	write_cmos_sensor(0x3351,0x06);
	write_cmos_sensor(0x335d,0x60);
	write_cmos_sensor(0x3364,0x56);
	write_cmos_sensor(0x337f,0x13);
	write_cmos_sensor(0x3390,0x09);
	write_cmos_sensor(0x3391,0x0f);
	write_cmos_sensor(0x3392,0x1f);
	write_cmos_sensor(0x3393,0x10);
	write_cmos_sensor(0x3394,0x1c);
	write_cmos_sensor(0x3395,0x20);
	write_cmos_sensor(0x33ac,0x0a);
	write_cmos_sensor(0x33ad,0x14);
	write_cmos_sensor(0x33ae,0x14);
	write_cmos_sensor(0x33af,0x60);
	write_cmos_sensor(0x33b0,0x0f);
	write_cmos_sensor(0x33b2,0x50);
	write_cmos_sensor(0x33b3,0x28);
	write_cmos_sensor(0x349f,0x1e);
	write_cmos_sensor(0x34a6,0x08);
	write_cmos_sensor(0x34a7,0x09);
	write_cmos_sensor(0x34a8,0x18);
	write_cmos_sensor(0x34a9,0x18);
	write_cmos_sensor(0x34f8,0x0f);
	write_cmos_sensor(0x34f9,0x18);
	write_cmos_sensor(0x3630,0xc6);
	write_cmos_sensor(0x3637,0x47);
	write_cmos_sensor(0x363c,0x8d);
	write_cmos_sensor(0x3670,0x0c);
	write_cmos_sensor(0x367b,0x56);
	write_cmos_sensor(0x367c,0x67);
	write_cmos_sensor(0x367d,0x67);
	write_cmos_sensor(0x367e,0x08);
	write_cmos_sensor(0x367f,0x0f);
	write_cmos_sensor(0x3690,0x84);
	write_cmos_sensor(0x3691,0x87);
	write_cmos_sensor(0x3692,0x8e);
	write_cmos_sensor(0x3693,0xb4);
	write_cmos_sensor(0x3694,0x09);
	write_cmos_sensor(0x3695,0x0b);
	write_cmos_sensor(0x3696,0x0f);
	write_cmos_sensor(0x3697,0x8b);
	write_cmos_sensor(0x3698,0x8d);
	write_cmos_sensor(0x3699,0x8d);
	write_cmos_sensor(0x369a,0x08);
	write_cmos_sensor(0x369b,0x0f);
	write_cmos_sensor(0x370f,0x01);
	write_cmos_sensor(0x3724,0x41);
	write_cmos_sensor(0x3771,0x03);
	write_cmos_sensor(0x3772,0x03);
	write_cmos_sensor(0x3773,0x63);
	write_cmos_sensor(0x377a,0x08);
	write_cmos_sensor(0x377b,0x0f);
	write_cmos_sensor(0x3900,0x0d);
	write_cmos_sensor(0x3903,0x20);
	write_cmos_sensor(0x3905,0x0c);
	write_cmos_sensor(0x3908,0x40);
	write_cmos_sensor(0x391a,0x30);
	write_cmos_sensor(0x391b,0x21);
	write_cmos_sensor(0x391c,0x13);
	write_cmos_sensor(0x391d,0x00);
	write_cmos_sensor(0x391f,0x41);
	write_cmos_sensor(0x3926,0xe0);
	write_cmos_sensor(0x393f,0x80);
	write_cmos_sensor(0x3940,0x00);
	write_cmos_sensor(0x3941,0x00);
	write_cmos_sensor(0x3942,0x00);
	write_cmos_sensor(0x3943,0x7c);
	write_cmos_sensor(0x3944,0x79);
	write_cmos_sensor(0x3945,0x7d);
	write_cmos_sensor(0x3946,0x7d);
	write_cmos_sensor(0x39dd,0x00);
	write_cmos_sensor(0x39de,0x04);
	write_cmos_sensor(0x39e7,0x04);
	write_cmos_sensor(0x39e8,0x03);
	write_cmos_sensor(0x39e9,0x80);
	write_cmos_sensor(0x3e00,0x00);
	write_cmos_sensor(0x3e01,0x63);
	write_cmos_sensor(0x3e02,0x90);
	write_cmos_sensor(0x3f08,0x0a);
	write_cmos_sensor(0x4401,0x13);
	write_cmos_sensor(0x4402,0x03);
	write_cmos_sensor(0x4403,0x0c);
	write_cmos_sensor(0x4404,0x24);
	write_cmos_sensor(0x4405,0x2f);
	write_cmos_sensor(0x440c,0x3c);
	write_cmos_sensor(0x440d,0x3c);
	write_cmos_sensor(0x440e,0x2d);
	write_cmos_sensor(0x440f,0x4b);
	write_cmos_sensor(0x4413,0x01);
	write_cmos_sensor(0x441b,0x18);
	write_cmos_sensor(0x4509,0x18);
	write_cmos_sensor(0x450d,0x13);
	write_cmos_sensor(0x4800,0x24);
	write_cmos_sensor(0x5000,0x5e);
    write_cmos_sensor(0x5002,0x00);
	write_cmos_sensor(0x5015,0x41);
	write_cmos_sensor(0x550e,0x00);
	write_cmos_sensor(0x550f,0x7a);
	write_cmos_sensor(0x5780,0x76);
	write_cmos_sensor(0x5784,0x08);
	write_cmos_sensor(0x5787,0x0a);
	write_cmos_sensor(0x5788,0x0a);
	write_cmos_sensor(0x5789,0x0a);
	write_cmos_sensor(0x578a,0x0a);
	write_cmos_sensor(0x578b,0x0a);
	write_cmos_sensor(0x578c,0x0a);
	write_cmos_sensor(0x578d,0x40);
	write_cmos_sensor(0x5790,0x08);
	write_cmos_sensor(0x5791,0x04);
	write_cmos_sensor(0x5792,0x04);
	write_cmos_sensor(0x5793,0x08);
	write_cmos_sensor(0x5794,0x04);
	write_cmos_sensor(0x5795,0x04);
	write_cmos_sensor(0x5799,0x44);
	write_cmos_sensor(0x57a6,0x1c);
	write_cmos_sensor(0x57aa,0xeb);
	write_cmos_sensor(0x57ab,0x7f);
	write_cmos_sensor(0x57ac,0x00);
	write_cmos_sensor(0x57ad,0x00);
	write_cmos_sensor(0x58e0,0xb4);
	write_cmos_sensor(0x5900,0x01);
	write_cmos_sensor(0x5901,0x04);
 	write_cmos_sensor(0x3221,0x66);
}

static void slim_video_setting(void)
{
    write_cmos_sensor(0x0103,0x01);
	write_cmos_sensor(0x0100,0x00);
	write_cmos_sensor(0x36e9,0x80);
	write_cmos_sensor(0x37f9,0x80);
	write_cmos_sensor(0x36e9,0x24);
	write_cmos_sensor(0x37f9,0x24);
	write_cmos_sensor(0x301f,0x28);
	write_cmos_sensor(0x3200,0x00);
	write_cmos_sensor(0x3201,0x00);
	write_cmos_sensor(0x3202,0x01);
	write_cmos_sensor(0x3203,0xe0);
	write_cmos_sensor(0x3204,0x10);
	write_cmos_sensor(0x3205,0x7f);
	write_cmos_sensor(0x3206,0x0a);
	write_cmos_sensor(0x3207,0x5f);
	write_cmos_sensor(0x3208,0x07);
	write_cmos_sensor(0x3209,0x80);
	write_cmos_sensor(0x320a,0x04);
	write_cmos_sensor(0x320b,0x38);
	write_cmos_sensor(0x320c,0x04);
	write_cmos_sensor(0x320d,0xe2);
	write_cmos_sensor(0x320e,0x0c);
	write_cmos_sensor(0x320f,0x80);
	write_cmos_sensor(0x3210,0x00);
	write_cmos_sensor(0x3211,0x60);
	write_cmos_sensor(0x3212,0x00);
	write_cmos_sensor(0x3213,0x04);
	write_cmos_sensor(0x3215,0x31);
	write_cmos_sensor(0x3220,0x01);
	write_cmos_sensor(0x3279,0x10);
	write_cmos_sensor(0x3301,0x07);
	write_cmos_sensor(0x3302,0x18);
	write_cmos_sensor(0x3304,0x38);
	write_cmos_sensor(0x3306,0x58);
	write_cmos_sensor(0x3308,0x0c);
	write_cmos_sensor(0x3309,0x80);
	write_cmos_sensor(0x330b,0xb8);
	write_cmos_sensor(0x330d,0x10);
	write_cmos_sensor(0x330e,0x2b);
	write_cmos_sensor(0x3314,0x15);
	write_cmos_sensor(0x331c,0x04);
	write_cmos_sensor(0x331e,0x29);
	write_cmos_sensor(0x331f,0x71);
	write_cmos_sensor(0x3320,0x08);
	write_cmos_sensor(0x3323,0x04);
	write_cmos_sensor(0x3333,0x10);
	write_cmos_sensor(0x3334,0x40);
	write_cmos_sensor(0x3351,0x06);
	write_cmos_sensor(0x335d,0x60);
	write_cmos_sensor(0x3364,0x56);
	write_cmos_sensor(0x337f,0x13);
	write_cmos_sensor(0x3390,0x09);
	write_cmos_sensor(0x3391,0x0f);
	write_cmos_sensor(0x3392,0x1f);
	write_cmos_sensor(0x3393,0x10);
	write_cmos_sensor(0x3394,0x1c);
	write_cmos_sensor(0x3395,0x20);
	write_cmos_sensor(0x33ac,0x0a);
	write_cmos_sensor(0x33ad,0x14);
	write_cmos_sensor(0x33ae,0x14);
	write_cmos_sensor(0x33af,0x60);
	write_cmos_sensor(0x33b0,0x0f);
	write_cmos_sensor(0x33b2,0x50);
	write_cmos_sensor(0x33b3,0x28);
	write_cmos_sensor(0x349f,0x1e);
	write_cmos_sensor(0x34a6,0x08);
	write_cmos_sensor(0x34a7,0x09);
	write_cmos_sensor(0x34a8,0x18);
	write_cmos_sensor(0x34a9,0x18);
	write_cmos_sensor(0x34f8,0x0f);
	write_cmos_sensor(0x34f9,0x18);
	write_cmos_sensor(0x3630,0xc6);
	write_cmos_sensor(0x3637,0x47);
	write_cmos_sensor(0x363c,0x8d);
	write_cmos_sensor(0x3670,0x0c);
	write_cmos_sensor(0x367b,0x56);
	write_cmos_sensor(0x367c,0x67);
	write_cmos_sensor(0x367d,0x67);
	write_cmos_sensor(0x367e,0x08);
	write_cmos_sensor(0x367f,0x0f);
	write_cmos_sensor(0x3690,0x84);
	write_cmos_sensor(0x3691,0x87);
	write_cmos_sensor(0x3692,0x8e);
	write_cmos_sensor(0x3693,0xb4);
	write_cmos_sensor(0x3694,0x09);
	write_cmos_sensor(0x3695,0x0b);
	write_cmos_sensor(0x3696,0x0f);
	write_cmos_sensor(0x3697,0x8b);
	write_cmos_sensor(0x3698,0x8d);
	write_cmos_sensor(0x3699,0x8d);
	write_cmos_sensor(0x369a,0x08);
	write_cmos_sensor(0x369b,0x0f);
	write_cmos_sensor(0x370f,0x01);
	write_cmos_sensor(0x3724,0x41);
	write_cmos_sensor(0x3771,0x03);
	write_cmos_sensor(0x3772,0x03);
	write_cmos_sensor(0x3773,0x63);
	write_cmos_sensor(0x377a,0x08);
	write_cmos_sensor(0x377b,0x0f);
	write_cmos_sensor(0x3900,0x0d);
	write_cmos_sensor(0x3903,0x20);
	write_cmos_sensor(0x3905,0x0c);
	write_cmos_sensor(0x3908,0x40);
	write_cmos_sensor(0x391a,0x30);
	write_cmos_sensor(0x391b,0x21);
	write_cmos_sensor(0x391c,0x13);
	write_cmos_sensor(0x391d,0x00);
	write_cmos_sensor(0x391f,0x41);
	write_cmos_sensor(0x3926,0xe0);
	write_cmos_sensor(0x393f,0x80);
	write_cmos_sensor(0x3940,0x00);
	write_cmos_sensor(0x3941,0x00);
	write_cmos_sensor(0x3942,0x00);
	write_cmos_sensor(0x3943,0x7c);
	write_cmos_sensor(0x3944,0x79);
	write_cmos_sensor(0x3945,0x7d);
	write_cmos_sensor(0x3946,0x7d);
	write_cmos_sensor(0x39dd,0x00);
	write_cmos_sensor(0x39de,0x04);
	write_cmos_sensor(0x39e7,0x04);
	write_cmos_sensor(0x39e8,0x03);
	write_cmos_sensor(0x39e9,0x80);
	write_cmos_sensor(0x3e00,0x01);
	write_cmos_sensor(0x3e01,0x8f);
	write_cmos_sensor(0x3e02,0x90);
	write_cmos_sensor(0x3f08,0x0a);
	write_cmos_sensor(0x4401,0x13);
	write_cmos_sensor(0x4402,0x03);
	write_cmos_sensor(0x4403,0x0c);
	write_cmos_sensor(0x4404,0x24);
	write_cmos_sensor(0x4405,0x2f);
	write_cmos_sensor(0x440c,0x3c);
	write_cmos_sensor(0x440d,0x3c);
	write_cmos_sensor(0x440e,0x2d);
	write_cmos_sensor(0x440f,0x4b);
	write_cmos_sensor(0x4413,0x01);
	write_cmos_sensor(0x441b,0x18);
	write_cmos_sensor(0x4509,0x18);
	write_cmos_sensor(0x450d,0x13);
	write_cmos_sensor(0x4800,0x24);
	write_cmos_sensor(0x5000,0x5e);
    write_cmos_sensor(0x5002,0x00);
	write_cmos_sensor(0x5015,0x41);
	write_cmos_sensor(0x550e,0x00);
	write_cmos_sensor(0x550f,0x7a);
	write_cmos_sensor(0x5780,0x76);
	write_cmos_sensor(0x5784,0x08);
	write_cmos_sensor(0x5787,0x0a);
	write_cmos_sensor(0x5788,0x0a);
	write_cmos_sensor(0x5789,0x0a);
	write_cmos_sensor(0x578a,0x0a);
	write_cmos_sensor(0x578b,0x0a);
	write_cmos_sensor(0x578c,0x0a);
	write_cmos_sensor(0x578d,0x40);
	write_cmos_sensor(0x5790,0x08);
	write_cmos_sensor(0x5791,0x04);
	write_cmos_sensor(0x5792,0x04);
	write_cmos_sensor(0x5793,0x08);
	write_cmos_sensor(0x5794,0x04);
	write_cmos_sensor(0x5795,0x04);
	write_cmos_sensor(0x5799,0x44);
	write_cmos_sensor(0x57a6,0x1c);
	write_cmos_sensor(0x57aa,0xeb);
	write_cmos_sensor(0x57ab,0x7f);
	write_cmos_sensor(0x57ac,0x00);
	write_cmos_sensor(0x57ad,0x00);
	write_cmos_sensor(0x58e0,0xb4);
	write_cmos_sensor(0x5900,0x01);
	write_cmos_sensor(0x5901,0x04);
 	write_cmos_sensor(0x3221,0x66);
}

static kal_uint32 set_test_pattern_mode(kal_bool enable)
{
	kal_uint16 sc1320_vaule;
	sc1320_vaule=read_cmos_sensor(0x5000);
	LOG_INFO("set_test_pattern_mode enable: %d", enable);

	if (enable) {
	    write_cmos_sensor(0x0100,0x00);
	    write_cmos_sensor(0x3337,0x30);
	    write_cmos_sensor(0x5000, sc1320_vaule&0xf9);
	    write_cmos_sensor(0x391f,0x40);
	    write_cmos_sensor(0x3e06,0x00);
	    write_cmos_sensor(0x3e07,0x80);
	    write_cmos_sensor(0x3e09,0x00);
	    write_cmos_sensor(0x0100,0x01); }
	else {
	    write_cmos_sensor(0x0100,0x00);
	    write_cmos_sensor(0x3337,0x00);
	    write_cmos_sensor(0x5000, sc1320_vaule|0x06);
	    write_cmos_sensor(0x391f,0x41);
	    write_cmos_sensor(0x0100,0x01); }

	LOG_INFO("sc1320 sensor 0x5000 reg id1: 0x%x,id2:0x%x,id3:0x%x,id4:0x%x\n",(read_cmos_sensor(0x5000)),sc1320_vaule, (sc1320_vaule&0xf9),(sc1320_vaule|0x06));
	spin_lock(&imgsensor_drv_lock);
	imgsensor.test_pattern = enable;
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
}

#if 1
static kal_uint16 read_cmos_sensor_sc1320cs_truly_otp(kal_uint32 addr)
{
	kal_uint16 get_byte = 0;
	char pu_send_cmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF) };
	iReadRegI2C(pu_send_cmd, 2, (u8 *)&get_byte, 1, 0xB0);
	return get_byte;
}

static void sc1320cs_truly_fusion_id_read(void)
{
	int i;
	for (i=0; i<6; i++) {
		fusion_id_main[i] = read_cmos_sensor_sc1320cs_truly_otp(0x10+i);
	LOG_INFO("%s %d zengx fusion_id_main[%d]=0x%2x\n",__func__, __LINE__, i, fusion_id_main[i]);
	}
}
static void sc1320cs_truly_sn_id_read(void)
{
	int i;
	for (i=0; i<25; i++) {
	        sn_main[i] = read_cmos_sensor_sc1320cs_truly_otp(0x3FD8+i);
		LOG_INFO("%s %d zengx sn_main[%d]=0x%2x\n",__func__, __LINE__, i, sn_main[i]);
	}
}
#endif

static kal_uint32 get_imgsensor_id(UINT32 *sensor_id)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;

	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			*sensor_id = return_sensor_id();
			if (*sensor_id == imgsensor_info.sensor_id) {
				LOG_INFO("i2c write id  : 0x%x, sensor id: 0x%x",
					 imgsensor.i2c_write_id, *sensor_id);
#if 1
					sc1320cs_truly_fusion_id_read();
					sc1320cs_truly_sn_id_read();
#endif
				return ERROR_NONE;
			}
			LOG_ERR("Read sensor id fail, write id: 0x%x,sensor id: 0x%x\n",
				 imgsensor.i2c_write_id, *sensor_id);
			retry--;
		} while (retry > 0);
		i++;
		retry = 2;
	}
	if (*sensor_id != imgsensor_info.sensor_id) {
		*sensor_id = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}

	return ERROR_NONE;
}

/*
 * FUNCTION
 *	open
 *
 * DESCRIPTION
 *	This function initialize the registers of CMOS sensor
 *
 * PARAMETERS
 *	None
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 */
static kal_uint32 open(void)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	kal_uint32 sensor_id = 0;

	//LOG_INFO("preview 1632*1224@30fps,360Mbps/lane; capture 3264*2448@30fps,880Mbps/lane");
	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			sensor_id = return_sensor_id();
			if (sensor_id == imgsensor_info.sensor_id) {
				LOG_INFO("i2c write id: 0x%x, sensor id: 0x%x",
					 imgsensor.i2c_write_id, sensor_id);
				break;
			}
			LOG_ERR("open:Read sensor id fail open i2c write id: 0x%x, id: 0x%x",
				 imgsensor.i2c_write_id, sensor_id);
			retry--;
		} while (retry > 0);
		i++;
		if (sensor_id == imgsensor_info.sensor_id)
			break;
		retry = 2;
	}
	if (imgsensor_info.sensor_id != sensor_id)
		return ERROR_SENSOR_CONNECT_FAIL;
	/* initail sequence write in  */
	sensor_init();

	spin_lock(&imgsensor_drv_lock);
	imgsensor.autoflicker_en = KAL_FALSE;
	imgsensor.sensor_mode = IMGSENSOR_MODE_INIT;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.dummy_pixel = 0;
	imgsensor.dummy_line = 0;
	imgsensor.ihdr_en = 0;
	imgsensor.test_pattern = KAL_FALSE;
	imgsensor.current_fps = imgsensor_info.pre.max_framerate;
	imgsensor.pdaf_mode = 0;
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
}

static kal_uint32 close(void)
{
	LOG_INFO("close E");

	return ERROR_NONE;
}

/*
 * FUNCTION
 *	preview
 *
 * DESCRIPTION
 *	This function start the sensor preview.
 *
 * PARAMETERS
 *	*image_window : address pointer of pixel numbers in one period of HSYNC
 *	*sensor_config_data : address pointer of line numbers in one period of VSYNC
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 */
static kal_uint32 preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INFO("Enter %s", __func__);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_PREVIEW;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.current_fps = imgsensor_info.pre.max_framerate;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	preview_setting();

	return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*	capture
*
* DESCRIPTION
*	This function setup the CMOS sensor in capture MY_OUTPUT mode
*
* PARAMETERS
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INFO("Enter %s", __func__);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;
	if (imgsensor.current_fps == imgsensor_info.cap1.max_framerate) {
		imgsensor.pclk = imgsensor_info.cap1.pclk;
		imgsensor.line_length = imgsensor_info.cap1.linelength;
		imgsensor.frame_length = imgsensor_info.cap1.framelength;
		imgsensor.min_frame_length = imgsensor_info.cap1.framelength;
		imgsensor.autoflicker_en = KAL_FALSE;
	} else {
		if (imgsensor.current_fps != imgsensor_info.cap.max_framerate)
			LOG_ERR("current_fps %d fps is not support, so use cap's setting: %d fps",
				 imgsensor.current_fps, imgsensor_info.cap.max_framerate / 10);
		imgsensor.pclk = imgsensor_info.cap.pclk;
		imgsensor.line_length = imgsensor_info.cap.linelength;
		imgsensor.frame_length = imgsensor_info.cap.framelength;
		imgsensor.min_frame_length = imgsensor_info.cap.framelength;
		imgsensor.autoflicker_en = KAL_FALSE;
	}
	spin_unlock(&imgsensor_drv_lock);
	capture_setting();

	return ERROR_NONE;
}

static kal_uint32 normal_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
				MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INFO("Enter %s", __func__);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_VIDEO;
	imgsensor.pclk = imgsensor_info.normal_video.pclk;
	imgsensor.line_length = imgsensor_info.normal_video.linelength;
	imgsensor.frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.current_fps = 300;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	normal_video_setting();
	return ERROR_NONE;
}

static kal_uint32 hs_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
				MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INFO("Enter %s", __func__);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_HIGH_SPEED_VIDEO;
	imgsensor.pclk = imgsensor_info.hs_video.pclk;
	imgsensor.line_length = imgsensor_info.hs_video.linelength;
	imgsensor.frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	hs_video_setting();

	return ERROR_NONE;
}

static kal_uint32 slim_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INFO("Enter %s", __func__);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_SLIM_VIDEO;
	imgsensor.pclk = imgsensor_info.slim_video.pclk;
	imgsensor.line_length = imgsensor_info.slim_video.linelength;
	imgsensor.frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	slim_video_setting();

	return ERROR_NONE;
}

static kal_uint32 get_resolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *sensor_resolution)
{
	LOG_INFO("Enter %s", __func__);
	sensor_resolution->SensorFullWidth = imgsensor_info.cap.grabwindow_width;
	sensor_resolution->SensorFullHeight = imgsensor_info.cap.grabwindow_height;
	sensor_resolution->SensorPreviewWidth = imgsensor_info.pre.grabwindow_width;
	sensor_resolution->SensorPreviewHeight = imgsensor_info.pre.grabwindow_height;
	sensor_resolution->SensorVideoWidth = imgsensor_info.normal_video.grabwindow_width;
	sensor_resolution->SensorVideoHeight = imgsensor_info.normal_video.grabwindow_height;
	sensor_resolution->SensorHighSpeedVideoWidth = imgsensor_info.hs_video.grabwindow_width;
	sensor_resolution->SensorHighSpeedVideoHeight = imgsensor_info.hs_video.grabwindow_height;
	sensor_resolution->SensorSlimVideoWidth = imgsensor_info.slim_video.grabwindow_width;
	sensor_resolution->SensorSlimVideoHeight = imgsensor_info.slim_video.grabwindow_height;

	return ERROR_NONE;
}

static kal_uint32 get_info(enum MSDK_SCENARIO_ID_ENUM scenario_id,
			  MSDK_SENSOR_INFO_STRUCT *sensor_info,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INFO("scenario_id = %d", scenario_id);
	sensor_info->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorInterruptDelayLines = 4;
	sensor_info->SensorResetActiveHigh = FALSE;
	sensor_info->SensorResetDelayCount = 5;
	sensor_info->SensroInterfaceType = imgsensor_info.sensor_interface_type;
	sensor_info->MIPIsensorType = imgsensor_info.mipi_sensor_type;
	sensor_info->SettleDelayMode = imgsensor_info.mipi_settle_delay_mode;
	sensor_info->SensorOutputDataFormat = imgsensor_info.sensor_output_dataformat;
	sensor_info->CaptureDelayFrame = imgsensor_info.cap_delay_frame;
	sensor_info->PreviewDelayFrame = imgsensor_info.pre_delay_frame;
	sensor_info->VideoDelayFrame = imgsensor_info.video_delay_frame;
	sensor_info->HighSpeedVideoDelayFrame = imgsensor_info.hs_video_delay_frame;
	sensor_info->SlimVideoDelayFrame = imgsensor_info.slim_video_delay_frame;
	sensor_info->SensorMasterClockSwitch = 0;
	sensor_info->SensorDrivingCurrent = imgsensor_info.isp_driving_current;
	sensor_info->AEShutDelayFrame = imgsensor_info.ae_shut_delay_frame;
	sensor_info->AESensorGainDelayFrame = imgsensor_info.ae_sensor_gain_delay_frame;
	sensor_info->AEISPGainDelayFrame = imgsensor_info.ae_ispGain_delay_frame;
	sensor_info->IHDR_Support = imgsensor_info.ihdr_support;
	sensor_info->IHDR_LE_FirstLine = imgsensor_info.ihdr_le_firstline;
	sensor_info->SensorModeNum = imgsensor_info.sensor_mode_num;
	sensor_info->SensorMIPILaneNumber = imgsensor_info.mipi_lane_num;
	sensor_info->SensorClockFreq = imgsensor_info.mclk;
	sensor_info->SensorClockDividCount = 3;
	sensor_info->SensorClockRisingCount = 0;
	sensor_info->SensorClockFallingCount = 2;
	sensor_info->SensorPixelClockCount = 3;
	sensor_info->SensorDataLatchCount = 2;
	sensor_info->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->SensorWidthSampling = 0;
	sensor_info->SensorHightSampling = 0;
	sensor_info->SensorPacketECCOrder = 1;
	sensor_info->PDAF_Support = 1;

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		sensor_info->SensorGrabStartX = imgsensor_info.cap.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.cap.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.cap.mipi_data_lp2hs_settle_dc;
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		sensor_info->SensorGrabStartX = imgsensor_info.normal_video.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.normal_video.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.normal_video.mipi_data_lp2hs_settle_dc;
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		sensor_info->SensorGrabStartX = imgsensor_info.hs_video.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.hs_video.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.hs_video.mipi_data_lp2hs_settle_dc;
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		sensor_info->SensorGrabStartX = imgsensor_info.slim_video.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.slim_video.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.slim_video.mipi_data_lp2hs_settle_dc;
		break;
	default:
		sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
		break;
	}

	return ERROR_NONE;
}


static kal_uint32 control(enum MSDK_SCENARIO_ID_ENUM scenario_id,
			  MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INFO("scenario_id = %d", scenario_id);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.current_scenario_id = scenario_id;
	spin_unlock(&imgsensor_drv_lock);
	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		preview(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		capture(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		normal_video(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		hs_video(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		slim_video(image_window, sensor_config_data);
		break;
	default:
		LOG_ERR("Error ScenarioId setting");
		preview(image_window, sensor_config_data);
		return ERROR_INVALID_SCENARIO_ID;
	}

	return ERROR_NONE;
}

static kal_uint32 set_video_mode(UINT16 framerate)
{
	LOG_INFO("framerate = %d ", framerate);
	if (framerate == 0)
		return ERROR_NONE;
	spin_lock(&imgsensor_drv_lock);
	if ((framerate == 30) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 296;
	else if ((framerate == 15) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 146;
	else
		imgsensor.current_fps = 10 * framerate;
	spin_unlock(&imgsensor_drv_lock);
	set_max_framerate(imgsensor.current_fps, 1);
	set_dummy();

	return ERROR_NONE;
}

static kal_uint32 set_auto_flicker_mode(kal_bool enable, UINT16 framerate)
{
	LOG_INFO("enable = %d, framerate = %d ", enable, framerate);
	spin_lock(&imgsensor_drv_lock);
	if (enable)
		imgsensor.autoflicker_en = KAL_FALSE;
	else
		imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
}

static kal_uint32 set_max_framerate_by_scenario(enum MSDK_SCENARIO_ID_ENUM scenario_id,
						MUINT32 framerate)
{
	kal_uint32 frame_length;

	LOG_INFO("scenario_id = %d, framerate = %d", scenario_id, framerate);
	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		frame_length = imgsensor_info.pre.pclk / framerate * 10 /
			imgsensor_info.pre.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line = (frame_length > imgsensor_info.pre.framelength) ?
			(frame_length - imgsensor_info.pre.framelength) : 0;
		imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		if (framerate == 0)
			return ERROR_NONE;
		frame_length = imgsensor_info.normal_video.pclk / framerate * 10 /
			imgsensor_info.normal_video.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line = (frame_length > imgsensor_info.normal_video.framelength) ?
			(frame_length - imgsensor_info.normal_video.framelength) : 0;
		imgsensor.frame_length = imgsensor_info.normal_video.framelength +
			imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		if (imgsensor.current_fps == imgsensor_info.cap1.max_framerate) {
			frame_length = imgsensor_info.cap1.pclk / framerate * 10 /
					imgsensor_info.cap1.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.cap1.framelength) ?
				(frame_length - imgsensor_info.cap1.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.cap1.framelength +
				imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
		} else {
			if (imgsensor.current_fps != imgsensor_info.cap.max_framerate)
				LOG_ERR("current_fps %d fps not support, use cap's: %d fps",
					 framerate, imgsensor_info.cap.max_framerate/10);
			frame_length = imgsensor_info.cap.pclk / framerate * 10 /
				imgsensor_info.cap.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.cap.framelength) ?
				(frame_length - imgsensor_info.cap.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.cap.framelength +
				imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
		}
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		frame_length = imgsensor_info.hs_video.pclk / framerate * 10 /
			imgsensor_info.hs_video.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line = (frame_length > imgsensor_info.hs_video.framelength) ?
			(frame_length - imgsensor_info.hs_video.framelength) : 0;
		imgsensor.frame_length = imgsensor_info.hs_video.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		frame_length = imgsensor_info.slim_video.pclk / framerate * 10 /
			imgsensor_info.slim_video.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line = (frame_length > imgsensor_info.slim_video.framelength) ?
			(frame_length - imgsensor_info.slim_video.framelength) : 0;
		imgsensor.frame_length = imgsensor_info.slim_video.framelength +
			imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	default:
		frame_length = imgsensor_info.pre.pclk / framerate * 10 /
			imgsensor_info.pre.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line = (frame_length > imgsensor_info.pre.framelength) ?
			(frame_length - imgsensor_info.pre.framelength) : 0;
		imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		LOG_ERR("error scenario_id = %d, we use preview scenario", scenario_id);
		break;
	}

	return ERROR_NONE;
}

static kal_uint32 get_default_framerate_by_scenario(enum MSDK_SCENARIO_ID_ENUM scenario_id,
						    MUINT32 *framerate)
{
	LOG_INFO("scenario_id = %d", scenario_id);
	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		*framerate = imgsensor_info.pre.max_framerate;
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		*framerate = imgsensor_info.normal_video.max_framerate;
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		*framerate = imgsensor_info.cap.max_framerate;
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		*framerate = imgsensor_info.hs_video.max_framerate;
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		*framerate = imgsensor_info.slim_video.max_framerate;
		break;
	default:
		break;
	}

	return ERROR_NONE;
}

static kal_uint32 streaming_control(kal_bool enable)
{
	LOG_INFO("streaming %s", enable ? "enabled" : "disabled");
	if (enable) {
		write_cmos_sensor(0x0100, 0x01);
		mdelay(10);
		//write_cmos_sensor(0x302d, 0x00);
	} else {
		write_cmos_sensor(0x0100, 0x00);
	}
	mdelay(10);

	return ERROR_NONE;
}

static kal_uint32 feature_control(MSDK_SENSOR_FEATURE_ENUM feature_id,
				 UINT8 *feature_para, UINT32 *feature_para_len)
{
	kal_uint32 rate;
	UINT16 *feature_return_para_16 = (UINT16 *) feature_para;
	UINT16 *feature_data_16 = (UINT16 *) feature_para;
	UINT32 *feature_return_para_32 = (UINT32 *) feature_para;
	UINT32 *feature_data_32 = (UINT32 *) feature_para;
	unsigned long long *feature_data = (unsigned long long *) feature_para;
	struct SET_PD_BLOCK_INFO_T *PDAFinfo;
	struct SENSOR_WINSIZE_INFO_STRUCT *wininfo;
	MSDK_SENSOR_REG_INFO_STRUCT *sensor_reg_data = (MSDK_SENSOR_REG_INFO_STRUCT *) feature_para;

	LOG_INFO("feature_id = %d", feature_id);
	switch (feature_id) {
	case SENSOR_FEATURE_GET_GAIN_RANGE_BY_SCENARIO:
		*(feature_data + 1) = imgsensor_info.min_gain;
		*(feature_data + 2) = imgsensor_info.max_gain;
		break;
	case SENSOR_FEATURE_GET_BASE_GAIN_ISO_AND_STEP:
		*(feature_data + 0) = imgsensor_info.min_gain_iso;
		*(feature_data + 1) = imgsensor_info.gain_step;
		*(feature_data + 2) = imgsensor_info.gain_type;
		break;
	case SENSOR_FEATURE_GET_MIN_SHUTTER_BY_SCENARIO:
		*(feature_data + 1) = imgsensor_info.min_shutter;
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		case MSDK_SCENARIO_ID_CUSTOM1:
			*(feature_data + 2) = 2;
			break;
		default:
			*(feature_data + 2) = 1;
			break;
		}
		break;
	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ_BY_SCENARIO:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.cap.pclk;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.normal_video.pclk;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.hs_video.pclk;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.slim_video.pclk;
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.custom1.pclk;
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.pre.pclk;
			break;
		}
		break;
	case SENSOR_FEATURE_GET_PERIOD_BY_SCENARIO:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.cap.framelength << 16)
				+ imgsensor_info.cap.linelength;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.normal_video.framelength << 16)
				+ imgsensor_info.normal_video.linelength;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.hs_video.framelength << 16)
				+ imgsensor_info.hs_video.linelength;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.slim_video.framelength << 16)
				+ imgsensor_info.slim_video.linelength;
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.pre.framelength << 16)
				+ imgsensor_info.pre.linelength;
			break;
		}
		break;
	case SENSOR_FEATURE_GET_PDAF_DATA:
	        break;
	case SENSOR_FEATURE_GET_PDAF_INFO:
		pr_debug("SENSOR_FEATURE_GET_PDAF_INFO scenarioId:%d\n",
				 (UINT16) *feature_data);
		PDAFinfo =
			(struct SET_PD_BLOCK_INFO_T
			 *)(uintptr_t) (*(feature_data + 1));
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			memcpy((void *)PDAFinfo, (void *)&imgsensor_pd_info,
				 sizeof(struct SET_PD_BLOCK_INFO_T));
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			memcpy((void *)PDAFinfo, (void *)&imgsensor_pd_info_16_9,
				 sizeof(struct SET_PD_BLOCK_INFO_T));
			break;

		case MSDK_SCENARIO_ID_SLIM_VIDEO:
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		default:
			break;
		}
		break;
	case SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 1;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 1;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 0;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 0;
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 1;
			break;
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 0;
			break;
		}
		break;
	case SENSOR_FEATURE_SET_PDAF:
		break;
	case SENSOR_FEATURE_GET_PERIOD:
		*feature_return_para_16++ = imgsensor.line_length;
		*feature_return_para_16 = imgsensor.frame_length;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
		*feature_return_para_32 = imgsensor.pclk;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_GET_MIPI_PIXEL_RATE:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			rate = imgsensor_info.cap.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			rate = imgsensor_info.normal_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			rate = imgsensor_info.hs_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			rate = imgsensor_info.slim_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			rate = imgsensor_info.pre.mipi_pixel_rate;
			break;
		}
		*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = rate;
		break;
	case SENSOR_FEATURE_SET_ESHUTTER:
		set_shutter((kal_uint16)(*feature_data));
		break;
	case SENSOR_FEATURE_SET_NIGHTMODE:
		night_mode((BOOL)(*feature_data));
		break;
	case SENSOR_FEATURE_SET_GAIN:
		set_gain((UINT16)(*feature_data));
		break;
	case SENSOR_FEATURE_SET_FLASHLIGHT:
		break;
	case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
		break;
	case SENSOR_FEATURE_SET_REGISTER:
		write_cmos_sensor(sensor_reg_data->RegAddr, sensor_reg_data->RegData);
		break;
	case SENSOR_FEATURE_GET_REGISTER:
		sensor_reg_data->RegData = read_cmos_sensor(sensor_reg_data->RegAddr);
		LOG_INFO("adb_i2c_read 0x%x = 0x%x", sensor_reg_data->RegAddr,
			sensor_reg_data->RegData);
		break;
	case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
		*feature_return_para_32 = LENS_DRIVER_ID_DO_NOT_CARE;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_VIDEO_MODE:
		set_video_mode((UINT16)(*feature_data));
		break;
	case SENSOR_FEATURE_CHECK_SENSOR_ID:
		get_imgsensor_id(feature_return_para_32);
		break;
	case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
		set_auto_flicker_mode((BOOL)(*feature_data_16), (UINT16)(*(feature_data_16 + 1)));
		break;
	case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
		set_max_framerate_by_scenario((enum MSDK_SCENARIO_ID_ENUM)(*feature_data),
					      (MUINT32)(*(feature_data + 1)));
		break;
	case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
		get_default_framerate_by_scenario((enum MSDK_SCENARIO_ID_ENUM)(*feature_data),
						  (MUINT32 *)(uintptr_t)(*(feature_data + 1)));
		break;
	case SENSOR_FEATURE_SET_TEST_PATTERN:
		set_test_pattern_mode((BOOL)(*feature_data));
		break;
	case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
		*feature_return_para_32 = imgsensor_info.checksum_value;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_FRAMERATE:
		spin_lock(&imgsensor_drv_lock);
		imgsensor.current_fps = (UINT16)(*feature_data_32);
		spin_unlock(&imgsensor_drv_lock);
		break;
	case SENSOR_FEATURE_SET_HDR:
		LOG_INFO("ihdr enable: %d", (UINT16)(*feature_data_32));
		spin_lock(&imgsensor_drv_lock);
		imgsensor.ihdr_en = (UINT8)(*feature_data_32);
		spin_unlock(&imgsensor_drv_lock);
		break;
	case SENSOR_FEATURE_GET_CROP_INFO:
		LOG_INFO("SENSOR_FEATURE_GET_CROP_INFO scenarioId: %d", (UINT32)(*feature_data));
		wininfo = (struct SENSOR_WINSIZE_INFO_STRUCT *)(uintptr_t)(*(feature_data + 1));
		switch (*feature_data_32) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[1],
				sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[2],
				sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[3],
				sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[4],
				sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[0],
				sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		}
		break;
	case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
		LOG_INFO("SENSOR_SET_SENSOR_IHDR LE = %d, SE = %d, Gain = %d",
			(UINT16)(*feature_data), (UINT16)(*(feature_data + 1)),
			(UINT16)(*(feature_data + 2)));
		ihdr_write_shutter_gain((UINT16)*feature_data,
			(UINT16)(*(feature_data + 1)), (UINT16)(*(feature_data + 2)));
		break;
	case SENSOR_FEATURE_SET_STREAMING_SUSPEND:
		LOG_INFO("SENSOR_FEATURE_SET_STREAMING_SUSPEND");
		streaming_control(KAL_FALSE);
		break;
	case SENSOR_FEATURE_SET_STREAMING_RESUME:
		LOG_INFO("SENSOR_FEATURE_SET_STREAMING_RESUME, shutter:%hu",
			 (kal_uint16)(*feature_data));
		if ((*feature_data) != 0)
			set_shutter((kal_uint16)(*feature_data));
		streaming_control(KAL_TRUE);
		break;
	default:
		break;
	}

	return ERROR_NONE;
}

static struct SENSOR_FUNCTION_STRUCT sensor_func = {
	open,
	get_info,
	get_resolution,
	feature_control,
	control,
	close
};

UINT32 SC1320CS_TRULY_MAIN_I_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc)
{
	if (pfFunc != NULL)
		*pfFunc = &sensor_func;

	return ERROR_NONE;
}

