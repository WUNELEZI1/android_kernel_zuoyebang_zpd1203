// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include "kd_imgsensor.h"


#include "imgsensor_hw.h"
#include "imgsensor_cfg_table.h"

/* Legacy design */
struct IMGSENSOR_HW_POWER_SEQ sensor_power_sequence[] = {

/*add for p15*/
#if defined(S5KJNS_TRULY_MAIN_I_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5KJNS_TRULY_MAIN_I_MIPI_RAW,
		{
			{PDN, Vol_Low, 1},
			{RST, Vol_Low, 1},
			{DOVDD, Vol_1800, 1},
			{DVDD, Vol_1050, 1},
			{AVDD, Vol_2800, 1},
			{AFVDD, Vol_2800, 0},
			{RST, Vol_High, 1},
			{SensorMCLK, Vol_High, 14}
		},
	},
#endif
#if defined(IMX852_OFILM_MAIN_II_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX852_OFILM_MAIN_II_MIPI_RAW,
		{
			{PDN, Vol_Low, 1},
			{SensorMCLK, Vol_High, 5},
			{RST, Vol_Low, 1},
			{AVDD, Vol_2900, 1},
			{DOVDD, Vol_1800, 1},
			{DVDD, Vol_1100, 1},
			{AFVDD, Vol_2800, 0},
			{RST, Vol_High, 5}
		},
	},
#endif
#if defined(GC50F6_TXD_MAIN_III_MIPI_RAW)
	{
		SENSOR_DRVNAME_GC50F6_TXD_MAIN_III_MIPI_RAW,
		{
			{PDN, Vol_Low, 1},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 1},
			{DVDD, Vol_1100, 1},
			{AVDD, Vol_2800, 1},
			{AFVDD, Vol_2800, 1},
			{RST, Vol_High, 4},
			{SensorMCLK, Vol_High, 4}
		},
	},
#endif
#if defined(SC1320CS_TRULY_MAIN_I_MIPI_RAW)
	{
		SENSOR_DRVNAME_SC1320CS_TRULY_MAIN_I_MIPI_RAW,
		{
			{PDN, Vol_Low, 1},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 1},
			{DVDD, Vol_1200, 1},
			{AVDD, Vol_2800, 1},
			{AFVDD, Vol_2800, 1},
			{RST, Vol_High, 6},
			{SensorMCLK, Vol_High, 4}
		},
	},
#endif
#if defined(GC13A0_OFILM_MAIN_II_MIPI_RAW )
	{
		SENSOR_DRVNAME_GC13A0_OFILM_MAIN_II_MIPI_RAW,
		{
			{PDN, Vol_Low, 1},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 1},
			{DVDD, Vol_1200, 1},
			{AVDD, Vol_2800, 1},
			{AFVDD, Vol_2800, 1},
			{RST, Vol_High, 4},
			{SensorMCLK, Vol_High, 4}
		},
	},
#endif
#if defined(SC520_TRULY_FRONT_I_MIPI_RAW)
	{
		SENSOR_DRVNAME_SC520_TRULY_FRONT_I_MIPI_RAW,
		{
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 1},
			{DVDD, Vol_1200, 1},
			{AVDD, Vol_2800, 1},
			{RST, Vol_High, 5},
			{SensorMCLK, Vol_High, 5}
		},
	},
#endif
#if defined(GC05A2_CXT_FRONT_II_MIPI_RAW)
	{
		SENSOR_DRVNAME_GC05A2_CXT_FRONT_II_MIPI_RAW,
		{
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 1},
			{AVDD, Vol_2800, 1},
			{DVDD, Vol_1200, 1},
			{SensorMCLK, Vol_High, 5},
			{RST, Vol_High, 5}
		},
	},
#endif
#if defined(SC820CS_TRULY_FRONT_I_MIPI_RAW)
	{
		SENSOR_DRVNAME_SC820CS_TRULY_FRONT_I_MIPI_RAW,
		{
			{RST, Vol_Low, 1},
			{DOVDD, Vol_1800, 1},
			{DVDD, Vol_1200, 1},
			{AVDD, Vol_2800, 1},
			{SensorMCLK, Vol_High, 1},
			{RST, Vol_High, 5}
		},
	},
#endif
#if defined(OV08F_OFILM_FRONT_II_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV08F_OFILM_FRONT_II_MIPI_RAW,
		{
			{RST, Vol_Low, 1},
			{DOVDD, Vol_1800, 1},
			{AVDD, Vol_2800, 1},
			{DVDD, Vol_1200, 5},
			{RST, Vol_High, 1},
			{SensorMCLK, Vol_High, 8}
		},
	},
#endif
#if defined(GC08A8_CXT_FRONT_III_MIPI_RAW)
	{
		SENSOR_DRVNAME_GC08A8_CXT_FRONT_III_MIPI_RAW,
		{
			{RST, Vol_Low, 1},
			{DOVDD, Vol_1800, 1},
			{DVDD, Vol_1200, 1},
			{AVDD, Vol_2800, 3},
			{SensorMCLK, Vol_High, 3},
			{RST, Vol_High, 3}
		},
	},
#endif
	/* add new sensor before this line */
	{NULL,},
};

