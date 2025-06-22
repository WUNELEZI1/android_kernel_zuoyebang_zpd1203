/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * xr coul driver for Xring coulomb meter
 *
 * Copyright (c) 2024, XRing Technologies Co., Ltd.
 */

#ifndef _XR_COUL_XP2210_H_
#define _XR_COUL_XP2210_H_

#include <asm/irq.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/iio/iio.h>
#include <linux/iio/consumer.h>
#include <dt-bindings/xring/platform-specific/pmic/top_reg.h>

#include "xr_coul_core.h"

#define coul_ic_info(fmt, args...) \
	pr_info("[coul_ic] [%s:%d]" fmt, __func__, __LINE__, ## args)
#define coul_ic_warn(fmt, args...) \
	pr_warn("[coul_ic] [%s:%d]" fmt, __func__, __LINE__, ## args)
#define coul_ic_err(fmt, args...) \
	pr_err("[coul_ic] [%s:%d]" fmt, __func__, __LINE__, ## args)

// #define DEBGU_XP2210
#ifdef DEBGU_XP2210
#define coul_ic_debug(fmt, args...) \
	pr_err("[coul_ic] [%s:%d]" fmt, __func__, __LINE__, ## args)
#else
#define coul_ic_debug(fmt, args...) do {} while (0)
#endif

#define PERMILLAGE                      1000
#define REG_NUM                         3
#define CC_REG_NUM                      5
#define FIFO_DEPTH                      8
#define DEFAULT_I_GATE_VALUE            5000 /* 5000 mA */
#define DEFAULT_BATTERY_VOL_0_PERCENT   3150
#define DEFAULT_C_OFFSET_B              0
#define SECONDS_PER_HOUR                3600
#define COUL_FIFO_VOL_DEFAULT           0xffffff
#define COUL_CALI_ING                   0x1
#define COUL_CALI_DONE                  0
#define COUL_SING_BIT_MASK              0x800000
#define COUL_ADC_REG_MASK                 0xFF000000
#define COUL_C_R_MOHM                   1 /* resisitance mohm */
#define AB_DIV_OFFSET                   1000

#define REG_TO_UV_MULTI         759375
#define REG_TO_UV_DIV           524288
#define REG_TO_UA_MULTI         3796875
#define REG_TO_UA_DIV           131072

extern int coul_curr_mohm;

struct coul_ic_device {
	struct device *dev;
	struct regmap *regmap;
	struct mutex bus_lock;
	struct delayed_work irq_work;
	int irq;
	struct iio_channel *bat1_iio;
	spinlock_t cali_status_lock;
};

struct coul_irq_info {
	const char *name;
	const irq_handler_t	thread_fn;
	int irq;
};

enum coul_irq {
	COUL_LP_OUT_IRQ,
	COUL_LOW_VOL_IRQ,
	COUL_CC_IN_IRQ,
	COUL_CC_OUT_IRQ,
	COUL_IN_OCP_IRQ,
	COUL_OUT_OCP_IRQ,
	COUL_CALI_DONE_IRQ,
	COUL_MAX_IRQ,
};

#define COUL_NV_SAVE_RESULT        TOP_REG_RESERVE_POR_R5 /* use bit 0 */
#define COUL_NV_READ_RESULT        TOP_REG_RESERVE_POR_R5 /* use bit 1 */
#define COUL_SEL_OCV_SOURCE        TOP_REG_RESERVE_POR_R5 /* use bit 2 */
#define COUL_OCV_CAN_UPDATE_OCV    TOP_REG_RESERVE_POR_R5 /* use bit 3 */
#define COUL_NEW_BATT              TOP_REG_RESERVE_POR_R5 /* use bit 6 */
#define NV_SAVE_BITMASK            BIT(0)
#define NV_READ_RST_MASK           BIT(1)
#define USE_STORE_OCV_FLAG         BIT(2)
#define OCV_CAN_UPDATE_OCV         BIT(3)
#define COUL_NEW_BATT_FLAG         BIT(6)

/* record last soc */
/* last soc use 0-6bit  last soc flag use bit7*/
#define COUL_LAST_SOC_ADDR         TOP_REG_RESERVE_POR_R6
#define SAVE_LAST_SOC_MASK (BIT(6) | BIT(5) | BIT(4) | BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define SAVE_LAST_SOC_FLAG         BIT(7)

#define COUL_OCV_LEVEL_ADDR        TOP_REG_RESERVE_POR_R7 /* ocv level 0-3bit */
#define SET_OCV_LEVEL_MASK         (BIT(3) | BIT(2) | BIT(1) | BIT(0))

#define COUL_STORE_OCV_ADDR        TOP_REG_RESERVE_POR_R8 /* use 3bytes reserve8, reserve9 and reserve10  */

#define COUL_OCV_TEMP_ADDR         TOP_REG_RESERVE_POR_R14 /* use 2bytes reserve14 and reserve15 */

#define EN_SW_Q_IN_OUT_CFG         (BIT(1) | BIT(0)) /*BIT1:Q_IN BIT0:Q_OUT*/
#define VC_FIFO_UPDATE_MODE_ALL    (BIT(1) | BIT(0)) /*BIT1:LP_MODE BIT0:NORMAL_MODE*/

int coul_devops_register(struct coul_core_device *chip);

int coul_ic_init(void);
void coul_ic_exit(void);

int coul_get_batt_vol_regval(unsigned int *regval);
int coul_get_batt_cur_regval(unsigned int *regval);
int coul_get_chipid(u64 *chipid);

#endif
