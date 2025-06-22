// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * xr coul driver for Xring coulomb meter
 *
 * Copyright (c) 2024, XRing Technologies Co., Ltd.
 */

#include <linux/spmi.h>
#include <linux/of_platform.h>
#include <linux/spinlock.h>

#include "xr_coul_xp2210.h"
#include "xr_coul_nv.h"

#define COUL_CIC_DEC_512        512
#define COUL_CIC_DEC_1024       1024
#define COUL_FIFO_LOCK          1
#define COUL_FIFO_UNLOCK        0
#define COUL_CAPTURE_LP_EN      1
#define COUL_CAPTURE_LP_DIS     0
#define T_V_ARRAY_LENGTH        38

#define ADC_REFER_VOLT          1800 /* 1800 mv */
#define ADC_MEASURE_NUM         4096 /* 12 bit ADC */

#define UV_TO_REG_MULTI         524288
#define UV_TO_REG_DIV           759375
#define REG_TO_UAH_MULTI        293
#define REG_TO_UAH_DIV          67500000
#define UAH_TO_REG_MULTI        10737418240
#define UAH_TO_REG_DIV          2637
#define MVA_TO_UVA              1000
#define UA_TO_REG_MULTI         (2000 * 4194304)
#define UA_TO_REG_DIV           243

#define OTP_REG_LOTID1_2_REG    0x865

static struct coul_ic_device *ic_dev;
static u64 last_lp_q_in;
static u64 last_lp_q_out;
static int cali_flag = COUL_CALI_DONE;
int coul_curr_mohm = COUL_C_R_MOHM;
static struct coul_ic_ops *coul_core_ops;

/* NTC Table */
struct temp_volt_table {
	int temp;
	int volt;
};

static struct temp_volt_table batt_temp_table[T_V_ARRAY_LENGTH] = {
	{ -2730, 1800 }, { -400, 1751 }, { -360, 1734 }, { -320, 1712 },
	{ -280, 1684 }, { -240, 1650 }, { -200, 1608 }, { -160, 1559 },
	{ -120, 1501 }, { -80, 1434 }, { -40, 1360 }, { 0, 1278 },
	{ 40, 1192 }, { 80, 1101 }, { 120, 1009 }, { 160, 916 },
	{ 200, 826 }, { 240, 740 }, { 280, 659 }, { 320, 584 },
	{ 360, 515 }, { 400, 453 }, { 440, 398 }, { 480, 348 },
	{ 520, 305 }, { 560, 267 }, { 600, 233 }, { 640, 204 },
	{ 680, 179 }, { 720, 157 }, { 760, 138 }, { 800, 121 },
	{ 840, 107 }, { 880, 94 }, { 920, 83 }, { 960, 74 },
	{ 1000, 66 }, { 1200, 37 },
};

static irqreturn_t coul_lp_out_thread_fn(int irq, void *data)
{
	coul_ic_err("coul-lp-out irq\n");
	return IRQ_HANDLED;
}

static irqreturn_t cou_low_vol_thread_fn(int irq, void *data)
{
	coul_ic_err("coul-low-vol irq\n");
	return IRQ_HANDLED;
}

static irqreturn_t coul_cc_in_thread_fn(int irq, void *data)
{
	coul_ic_err("coul-cc-in irq\n");
	return IRQ_HANDLED;
}

static irqreturn_t coul_cc_out_thread_fn(int irq, void *data)
{
	coul_ic_err("coul-cc-out irq\n");
	return IRQ_HANDLED;
}

static irqreturn_t coul_in_ocp_thread_fn(int irq, void *data)
{
	coul_ic_err("coul-in-ocp irq\n");
	return IRQ_HANDLED;
}

static irqreturn_t coul_out_ocp_thread_fn(int irq, void *data)
{
	coul_ic_err("coul-out-ocp irq\n");
	return IRQ_HANDLED;
}

static irqreturn_t coul_cali_done_thread_fn(int irq, void *data)
{
	unsigned long flags;
	struct coul_ic_device *chip = data;

	coul_ic_err("coul-cali-done irq\n");

	spin_lock_irqsave(&chip->cali_status_lock, flags);
	cali_flag = COUL_CALI_DONE;
	spin_unlock_irqrestore(&chip->cali_status_lock, flags);

	return IRQ_HANDLED;
}

static struct coul_irq_info coul_irqs[] = {
	[COUL_LP_OUT_IRQ] = {
		.name = "coul-lp-out",
		.thread_fn = coul_lp_out_thread_fn,
	},
	[COUL_LOW_VOL_IRQ] = {
		.name = "coul-low-vol",
		.thread_fn = cou_low_vol_thread_fn,
	},
	[COUL_CC_IN_IRQ] = {
		.name = "coul-cc-in",
		.thread_fn = coul_cc_in_thread_fn,
	},
	[COUL_CC_OUT_IRQ] = {
		.name = "coul-cc-out",
		.thread_fn = coul_cc_out_thread_fn,
	},
	[COUL_IN_OCP_IRQ] = {
		.name = "coul-in-ocp",
		.thread_fn = coul_in_ocp_thread_fn,
	},
	[COUL_OUT_OCP_IRQ] = {
		.name = "coul-out-ocp",
		.thread_fn = coul_out_ocp_thread_fn,
	},
	[COUL_CALI_DONE_IRQ] = {
		.name = "coul-cali-done",
		.thread_fn = coul_cali_done_thread_fn,
	},
};

static int coul_get_irq_index_byname(const char *irq_name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(coul_irqs); i++) {
		if (strcmp(coul_irqs[i].name, irq_name) == 0)
			return i;
	}

	return -ENOENT;
}

static int coul_request_interrupt(struct coul_ic_device *chip,
		struct device_node *node, const char *irq_name)
{
	int rc, irq, irq_index;

	irq = of_irq_get_byname(node, irq_name);
	if (irq < 0) {
		coul_ic_err("Failed to get irq %s byname\n", irq_name);
		return irq;
	}

	irq_index = coul_get_irq_index_byname(irq_name);
	if (irq_index < 0) {
		coul_ic_err("%s is not a defined irq\n", irq_name);
		return irq_index;
	}

	if (!coul_irqs[irq_index].thread_fn)
		return 0;

	rc = devm_request_threaded_irq(chip->dev, irq, NULL,
				coul_irqs[irq_index].thread_fn,
				IRQF_ONESHOT, irq_name, chip);
	if (rc < 0) {
		coul_ic_err("Failed to request irq %d\n", irq);
		return rc;
	}

	coul_irqs[irq_index].irq = irq;

	coul_ic_debug("IRQ %s registered\n", coul_irqs[irq_index].name);

	return 0;
}

static int coul_request_irqs(struct coul_ic_device *chip)
{
	struct device_node *node = chip->dev->of_node;
	struct device_node *child;
	const char *name;
	struct property *prop;
	int rc = 0;

	for_each_available_child_of_node(node, child) {
		of_property_for_each_string(child, "interrupt-names",
					    prop, name) {
			rc = coul_request_interrupt(chip, child, name);
			if (rc < 0)
				return rc;
		}
	}

	return 0;
}

static int coul_request_batt_adc(struct coul_ic_device *chip)
{
	int ret;
	struct device_node *node = chip->dev->of_node;
	const char *chann_name;

	ret = of_property_read_string_index(node, "io-channel-names", 0, &chann_name);
	if (ret < 0) {
		coul_ic_err("error parser io-channel-names: %d\n", ret);
		return ret;
	}

	chip->bat1_iio = devm_fwnode_iio_channel_get_by_name(chip->dev, &node->fwnode, chann_name);
	if (IS_ERR(chip->bat1_iio) || !chip->bat1_iio) {
		coul_ic_err("error getting channel: %s\n", chann_name);
		return -EINVAL;
	}

	coul_ic_debug("getting channel: %s\n", chann_name);
	return ret;
}

int coul_read(struct coul_ic_device *chip, u32 addr, u8 *val, int len)
{
	int rc;

	if (!chip) {
		coul_ic_err("chip is null addr:0x%x, len:%d\n", addr, len);
		return -EINVAL;
	}

	if (!chip->regmap) {
		coul_ic_err("chip->regmap is null addr:0x%x, len:%d\n", addr, len);
		return -EINVAL;
	}

	mutex_lock(&chip->bus_lock);
	rc = regmap_bulk_read(chip->regmap, addr, val, len);
	if (rc < 0) {
		coul_ic_err("Failed regmap_read for address %04x rc=%d\n", addr, rc);
		goto out;
	}

out:
	mutex_unlock(&chip->bus_lock);
	return rc;
}

int coul_write(struct coul_ic_device *chip, u32 addr, u8 *val, int len)
{
	int rc;

	if (!chip) {
		coul_ic_err("chip is null addr:0x%x, len:%d\n", addr, len);
		return -EINVAL;
	}

	if (!chip->regmap) {
		coul_ic_err("chip->regmap is null addr:0x%x, len:%d\n", addr, len);
		return -EINVAL;
	}

	mutex_lock(&chip->bus_lock);

	if (len > 1)
		rc = regmap_bulk_write(chip->regmap, addr, val, len);
	else
		rc = regmap_write(chip->regmap, addr, *val);

	if (rc < 0) {
		coul_ic_err("Failed regmap_write for address %04x rc=%d\n",
				addr, rc);
		goto out;
	}
out:
	mutex_unlock(&chip->bus_lock);
	return rc;
}

static unsigned int coul_convert_ua2regval(int ua)
{
	unsigned int ret;
	u64 curr;
	u32 val;

	if (ua > 0)
		val = ua;
	else
		val = -ua;

	curr = val * coul_curr_mohm;
	curr = curr * REG_TO_UA_DIV;
	curr = div_u64(curr, REG_TO_UA_MULTI);

	if (ua < 0) {
		curr |= COUL_ADC_REG_MASK;
		curr = ((~curr) + 1) | COUL_SING_BIT_MASK;
	}

	ret = (unsigned int)curr;

	coul_ic_debug("ua: %d, regval: 0x%x\n", ua, ret);

	return ret;
}

/*
 * Description:  convert uv value to Bit for register
 * Remark:
 */
static unsigned int coul_uv_convert_regval(int uv_val)
{
	unsigned int ret;
	u64 volt;
	int val;

	if (uv_val > 0)
		val = uv_val;
	else
		val = -uv_val;

	/*
	 * uv_val = code(23bit) * 1.215 * 5 * a * 10^6 / 2^22 + b
	 *     = code * 243 * 3125 * a / 2^16 / 8 + b
	 *     = code * 759375 *a / 524288 + b
	 * code = (uv_val - b) * 167772160 / 243 / a
	 */

	volt = (u64)val * UV_TO_REG_MULTI;
	volt = div_u64(volt, UV_TO_REG_DIV);

	if (uv_val < 0) {
		volt |= COUL_ADC_REG_MASK;
		volt = ((~volt) + 1) | COUL_SING_BIT_MASK;
	}

	ret = (unsigned int)volt;

	coul_ic_debug("uv: %d, regval: 0x%x\n", uv_val, ret);

	return ret;
}

/*
 * Description: convert register value to uah
 * Input: reg_val:cc reg val
 * Return: uah value of reg
 */
static int coul_regval_convert_uah(u64 reg_val)
{
	int ret;
	s64 curr;
	u8 coul_clk_mode;
	struct coul_cali_params cali_params = {
		0, 0,
		DEFAULT_C_OFF_A, DEFAULT_C_OFFSET_B
	};

	/*
	 *  Current = code(23bit) * 1.215 * 10^9 / 2^22 / 10 / Rsense
	 *                 = code * 3796875 * a/ 131072 / Rsense + b
	 *  derT = (1000/(38.4/4/293*100)*clk)/1000 ms
	 *       = (1000/(38.4/4/293*100)*clk)/1000/3600000 h
	 *       = 293/67500000 h
	 *  uah = Current * dertT
	 */
	get_coul_cali_params(&cali_params);

	if (!cali_params.c_offset_a ||
		(cali_params.c_offset_a == DEFAULT_C_OFF_A && cali_params.c_offset_b == 0)) {
		curr = (s64)reg_val * REG_TO_UA_MULTI;
		curr = div_s64(curr, REG_TO_UA_DIV);
		if (coul_curr_mohm)
			curr = div_s64(curr, coul_curr_mohm);
	} else {
		curr = (s64)reg_val * cali_params.c_offset_a;
		curr += cali_params.c_offset_b;
		curr = curr / AB_DIV_OFFSET;
	}

	curr *= REG_TO_UAH_MULTI;
	curr = div_s64(curr, REG_TO_UAH_DIV);

	ret = coul_read(ic_dev, TOP_REG_SW_COUL1_CIC_DEC,
				&coul_clk_mode, 1);
	if (ret)
		goto err;
	if (coul_clk_mode & TOP_REG_SW_COUL1_CIC_DEC_SW_COUL1_CIC_DEC_MASK)
		curr = curr * COUL_CIC_DEC_1024 / COUL_CIC_DEC_512;

	ret = curr;

	coul_ic_debug("reg_val: 0x%llx, uah: %d, c_offset_a:%d, c_offset_b:%d\n",
		reg_val, ret, cali_params.c_offset_a, cali_params.c_offset_b);

err:
	return ret;
}

/* Description: Get the temp of battery by soh */
static int coul_get_tbatt_adc_code(void)
{
	int tbat_code = 0;
	int ret;

	if (IS_ERR(ic_dev->bat1_iio) || !ic_dev->bat1_iio) {
		coul_ic_err("bat1_iio is err\n");
		return -EINVAL;
	}

	ret = iio_read_channel_raw(ic_dev->bat1_iio, &tbat_code);
	if (ret < 0) {
		coul_ic_err("read bat1_iio err:%d, tbat_code:%d\n", ret, tbat_code);
		return ret;
	}

	coul_ic_debug("tbat_code: %d\n", tbat_code);
	return tbat_code;
}

static int coul_get_adc_tbatt(void)
{
	int i;
	int temprature = 0;
	int adc_volt;

	adc_volt = coul_get_tbatt_adc_code() * ADC_REFER_VOLT / ADC_MEASURE_NUM;
	if (adc_volt < 0)
		return ERR_TEMP;

	coul_ic_debug("adc_volt: %d\n", adc_volt);

	if (adc_volt >= batt_temp_table[0].volt)
		return batt_temp_table[0].temp;
	if (adc_volt <= batt_temp_table[T_V_ARRAY_LENGTH - 1].volt)
		return batt_temp_table[T_V_ARRAY_LENGTH - 1].temp;
	for (i = 1; i < T_V_ARRAY_LENGTH; i++) {
		if (adc_volt == batt_temp_table[i].volt)
			return batt_temp_table[i].temp;
		if (adc_volt > batt_temp_table[i].volt)
			break;
	}

	if (i == T_V_ARRAY_LENGTH)
		i = T_V_ARRAY_LENGTH - 1;

	if ((batt_temp_table[i].volt - batt_temp_table[i - 1].volt) != 0)
		temprature = batt_temp_table[i - 1].temp +
			(long)(adc_volt - batt_temp_table[i - 1].volt) *
			(batt_temp_table[i].temp - batt_temp_table[i - 1].temp) /
			(batt_temp_table[i].volt - batt_temp_table[i - 1].volt);

	coul_ic_debug("temprature: %d\n", temprature);

	return temprature;
}

/*
 * Description: set i in gate
 * Input: ma, should < 0
 */
static void coul_set_curr_in_alert_thr(int ma)
{
	unsigned int reg_val;

	if (ma > 0)
		ma = -ma;
	reg_val = coul_convert_ua2regval(ma * PERMILLAGE);

	coul_write(ic_dev, TOP_REG_SW_COUL1_I_IN_OCD_TH_0, (u8 *)(&reg_val), REG_NUM);
}

/*
 * Description: set i out gate
 * Input: ma, should > 0
 */
static void coul_set_curr_out_alert_thr(int ma)
{
	unsigned int reg_val;

	if (ma < 0)
		ma = -ma;
	reg_val = coul_convert_ua2regval(ma * PERMILLAGE);

	coul_write(ic_dev, TOP_REG_SW_COUL1_I_OUT_OCD_TH_0, (u8 *)(&reg_val), REG_NUM);
}

static void coul_capture_lp_en(u8 en)
{
	coul_write(ic_dev, TOP_REG_SW_COUL1_CAPTURE_LP_EN, &en, 1);
}

static void coul_vc_fifo_update_en(u8 mode)
{
	coul_write(ic_dev, TOP_REG_SW_COUL1_VI_RO_FIFO_EN, &mode, 1);
}

static void coul_en_cali_vi(void)
{
	static int reg = 3;

	coul_write(ic_dev, TOP_REG_SW_COUL1_CAL_VLD_EN, (u8 *)(&reg), 1);
}

/* Description: config init */
static void coul_chip_init(void)
{
	coul_set_curr_in_alert_thr(DEFAULT_I_GATE_VALUE);
	coul_set_curr_out_alert_thr(DEFAULT_I_GATE_VALUE);

	coul_capture_lp_en(COUL_CAPTURE_LP_EN);
	coul_vc_fifo_update_en((u8)VC_FIFO_UPDATE_MODE_ALL);
	coul_en_cali_vi();
}

/*
 * Description: set low int vol val
 * Input: vol_value:low int vol val(mV)
 */
static void coul_set_low_vol_alert_thr(int vol_mv)
{
	unsigned int regval;

	regval = coul_uv_convert_regval(vol_mv * PERMILLAGE);
	coul_ic_debug("mv %d, reg 0x%x\n", vol_mv, regval);
	coul_write(ic_dev, TOP_REG_SW_COUL1_UV_ALARM_TH_0, (u8 *)(&regval), REG_NUM);
}

/*
 * Description:  value of out_uah - in_uah recorded by  coulomb
 * Return: value of uah through coulomb
 * Remark: adjusted by offset integrated on time
 */
static int get_cc_uah(void)
{
	u64 q_in_reg = 0;
	u64 q_out_reg = 0;
	int q_in_uah;
	int q_out_uah;
	int q_uah;
	int ret;

	ret = coul_read(ic_dev, TOP_REG_COUL1_CAL_Q_IN_0,
				(u8 *)(&q_in_reg), CC_REG_NUM);
	if (ret)
		goto err;

	ret = coul_read(ic_dev, TOP_REG_COUL1_CAL_Q_OUT_0,
				(u8 *)(&q_out_reg), CC_REG_NUM);
	if (ret)
		goto err;

	q_out_uah = coul_regval_convert_uah(q_out_reg);
	q_in_uah = coul_regval_convert_uah(q_in_reg);

	q_uah = q_out_uah - q_in_uah;
	coul_ic_debug("q_out_reg=0x%llx,q_in_reg=0x%llx,q_uah=%duah\n",
			q_out_reg, q_in_reg, q_uah);
	return q_uah;
err:
	return ret;
}

/* Description: convert uah value to reg */
static u64 coul_convert_uah2regval(unsigned int uah)
{
	int ret;
	u64 regval;
	u64 cap;
	u8 coul_clk_mode;
	int clk_rate;

	struct coul_cali_params cali_params = {
		0, 0,
		DEFAULT_C_OFF_A, DEFAULT_C_OFFSET_B
	};

	/*
	 *  Current = code(23bit) * 1.215 * 10^9 / 2^22 / 10 / Rsense
	 *                 = code * 3796875 * a/ 131072 / Rsense + b
	 *  derT = (1000/(38.4/4/293*100)*clk)/1000 ms
	 *       = (1000/(38.4/4/293*100)*clk)/1000/3600000 h
	 *  uah = Current * dertT * a + b
	 *      = 2637 * code * clk / 512 * a / 20971520 / Rsense + b
	 *  code = (uah - b) * 512 * 20971520 * Rsense / 2637 * clk * a
	 *       = (uah - b) * Rsense * 10737418240 / 2637 * clk * a
	 */

	get_coul_cali_params(&cali_params);

	ret = coul_read(ic_dev, TOP_REG_SW_COUL1_CIC_DEC,
				&coul_clk_mode, 1);
	if (ret)
		goto err;

	if (coul_clk_mode & TOP_REG_SW_COUL1_CIC_DEC_SW_COUL1_CIC_DEC_MASK)
		clk_rate = COUL_CIC_DEC_1024 / COUL_CIC_DEC_512;
	else
		clk_rate = COUL_CIC_DEC_512 / COUL_CIC_DEC_512;

	cap = uah / clk_rate;
	cap *= REG_TO_UAH_DIV;
	cap /= REG_TO_UAH_MULTI;

	if (!cali_params.c_offset_a ||
		(cali_params.c_offset_a == DEFAULT_C_OFF_A && cali_params.c_offset_b == 0)) {
		cap *= coul_curr_mohm;
		cap *= REG_TO_UA_DIV;
		cap /= REG_TO_UA_MULTI;
	} else {
		cap *= AB_DIV_OFFSET;
		cap -= cali_params.c_offset_b;
		cap /= cali_params.c_offset_a;
	}

	regval = cap;

	coul_ic_debug("uah = %u, regval = 0x%llx\n", uah, regval);

	return regval;

err:
	return (u64)ret;
}

static void set_cc_uah(int cc_uah)
{
	u64 reg;

	coul_ic_debug("cc_uah=%duah\n", cc_uah);
	if (cc_uah > 0) {
		reg = coul_convert_uah2regval(cc_uah);
		coul_write(ic_dev, TOP_REG_SW_COUL1_Q_OUT_CFG_0, (u8 *)(&reg), CC_REG_NUM);
		reg = 0;
		coul_write(ic_dev, TOP_REG_SW_COUL1_Q_IN_CFG_0, (u8 *)(&reg), CC_REG_NUM);
	} else {
		reg = coul_convert_uah2regval(-cc_uah);
		coul_write(ic_dev, TOP_REG_SW_COUL1_Q_IN_CFG_0, (u8 *)(&reg), CC_REG_NUM);
		reg = 0;
		coul_write(ic_dev, TOP_REG_SW_COUL1_Q_OUT_CFG_0, (u8 *)(&reg), CC_REG_NUM);
	}

	reg |= EN_SW_Q_IN_OUT_CFG;
	coul_write(ic_dev, TOP_REG_SW_COUL1_DEBUG_EN, (u8 *)(&reg), 1);
}

/*
 * Description:  convert register value to uv
 * Input: reg_val:voltage reg value
 * Return: value of register in uV
 */
static int coul_regval_convert_uv(unsigned int reg_val)
{
	s64 volt;
	int val;
	int ret;
	struct coul_cali_params cali_params = {
		DEFAULT_V_OFF_A, DEFAULT_V_OFF_B,
		0, 0
	};

	get_coul_cali_params(&cali_params);
	/*
	 * uv_val = code(23bit) * 1.215 * 5 * a * 10^6 / 2^22 + b
	 *     = code * 243 * 3125 * a / 2^16 / 8 + b
	 *     = code * 759375 *a / 524288 + b
	 */

	if (reg_val & COUL_SING_BIT_MASK) {
		reg_val |= COUL_ADC_REG_MASK;
		val = ((~reg_val) + 1) & (~COUL_SING_BIT_MASK);
	} else {
		val = (int)reg_val;
	}

	if (!cali_params.v_offset_a ||
		(cali_params.v_offset_a == DEFAULT_V_OFF_A && cali_params.v_offset_b == 0)) {
		volt = (s64)val * REG_TO_UV_MULTI;
		volt = div_s64(volt, REG_TO_UV_DIV);
	} else {
		volt = (s64)val * cali_params.v_offset_a;
		volt += cali_params.v_offset_b;
		volt = volt / AB_DIV_OFFSET;
	}

	ret = (int)volt;

	coul_ic_debug("reg_val: 0x%x, uv: %d, v_offset_a:%d, v_offset_b:%d\n",
		reg_val, ret, cali_params.v_offset_a, cali_params.v_offset_b);

	if (reg_val & COUL_SING_BIT_MASK)
		return -ret;
	return ret;
}

/*
 * Description: convert register value to current(uA)
 * Remark:
 *  if Rsense in mohm, Current in mA
 *  if Rsense in uohm, Current in uA
 *  high bit = 1 is in, 0 is out
 */
static int coul_regval_convert_ua(unsigned int reg_val)
{
	int ret;
	s64 curr;
	int val;
	struct coul_cali_params cali_params = {
		0, 0,
		DEFAULT_C_OFF_A, DEFAULT_C_OFFSET_B
	};
	/*
	 *  Current = code(23bit) * 1.215 * 10^9 a / 2^22 / 10 / Rsense + b
	 *                 = code * 3796875 * a/ 131072 / Rsense + b
	 */
	get_coul_cali_params(&cali_params);
	if (reg_val & COUL_SING_BIT_MASK) {
		reg_val |= COUL_ADC_REG_MASK;
		val = ((~reg_val) + 1) & (~COUL_SING_BIT_MASK);
	} else {
		val = (int)reg_val;
	}

	if (!cali_params.c_offset_a ||
		(cali_params.c_offset_a == DEFAULT_C_OFF_A && cali_params.c_offset_b == 0)) {
		curr = (s64)val * REG_TO_UA_MULTI;
		curr = div_s64(curr, REG_TO_UA_DIV);
		if (coul_curr_mohm)
			curr = div_s64(curr, coul_curr_mohm);
	} else {
		curr = (s64)val * cali_params.c_offset_a;
		curr += cali_params.c_offset_b;
		curr = curr / AB_DIV_OFFSET;
	}

	ret = curr;

	coul_ic_debug("reg_val: 0x%x, ua: %d, c_offset_a:%d, c_offset_b:%d\n",
		reg_val, ret, cali_params.c_offset_a, cali_params.c_offset_b);

	if (reg_val & COUL_SING_BIT_MASK)
		return -ret;

	return ret;
}

static int coul_get_fifo_depth(void)
{
	return FIFO_DEPTH;
}

/*
 * Description: get coul nv read success flag
 * Return:  success:1 fail:0
 * Remark:  the flag is written by fastboot
 */
int coul_get_nv_read_flag(void)
{
	unsigned char val;
	int ret;

	ret = coul_read(ic_dev, COUL_NV_READ_RESULT,
				&val, 1);
	if (ret)
		goto err;

	if (val & NV_READ_RST_MASK)
		return NV_READ_SUCCESS;
	return NV_READ_FAIL;
err:
	return ret;
}

/*
 * set coul nv save success flag
 * Input: nv_flag: success 1, fail 0
 * Remark: the flag is read by fastboot
 */
void coul_set_nv_save_flag(int nv_flag)
{
	unsigned char val;
	int ret;

	ret = coul_read(ic_dev, COUL_NV_SAVE_RESULT,
				&val, 1);
	if (ret)
		return;

	if (nv_flag == NV_SAVE_SUCCESS) {
		val |= NV_SAVE_BITMASK;
		ret = coul_write(ic_dev, COUL_NV_SAVE_RESULT, &val, 1);
		if (ret)
			return;
	} else {
		val &= (~NV_SAVE_BITMASK);
		coul_write(ic_dev, COUL_NV_SAVE_RESULT, &val, 1);
	}
}

/*
 * Return: 1:saved ocv, 0:not saved ocv
 * Remark:  the flag is written by fastboot
 */
int coul_get_use_saved_ocv_flag(void)
{
	unsigned char val;
	int ret;

	ret = coul_read(ic_dev, COUL_SEL_OCV_SOURCE,
				&val, 1);
	if (ret)
		return ret;

	if (val & USE_STORE_OCV_FLAG)
		return 1;
	return 0;
}

/*
 * Return: 1:can update fcc, 0:can not update fcc
 * Remark:  the flag is written by fastboot
 */
int coul_get_ocv_update_fcc_flag(void)
{
	unsigned char val;
	int ret;

	ret = coul_read(ic_dev, COUL_OCV_CAN_UPDATE_OCV,
				&val, 1);
	if (ret)
		return 0;

	coul_ic_debug("ocv update fcc flag :%lu\n", (val & OCV_CAN_UPDATE_OCV));

	if (val & OCV_CAN_UPDATE_OCV)
		return 0;

	return 1;
}

/*
 * Return: 1:update fcc flag okay, 0:update fcc flag fail
 */
int coul_set_ocv_update_fcc_flag(int val)
{
	unsigned char reg_val;
	int ret;

	ret = coul_read(ic_dev, COUL_OCV_CAN_UPDATE_OCV,
				&reg_val, 1);
	if (ret)
		return ret;

	if (val)
		reg_val |= OCV_CAN_UPDATE_OCV;
	else
		reg_val &= (~OCV_CAN_UPDATE_OCV);

	coul_write(ic_dev, COUL_OCV_CAN_UPDATE_OCV,
			(u8 *)(&reg_val), 1);

	return 0;
}

/*
 * Description: get saved ocv
 * Return: OCV(reg value)
 */
int coul_get_ocv(void)
{
	unsigned int ocvreg = 0;
	int ret;
	int ocv;

	ret = coul_read(ic_dev, COUL_STORE_OCV_ADDR,
				(u8 *)&ocvreg, REG_NUM);
	if (ret)
		return ret;

	ocv = coul_regval_convert_uv(ocvreg);

	coul_ic_debug("regval: 0x%x, ocv: %d\n", ocvreg, ocv);

	return ocv;
}

/*
 * Description: coul save ocv
 * Input: ocv:ocv vol uv
 */
void coul_set_ocv(int ocv)
{
	unsigned int ocvreg = coul_uv_convert_regval(ocv);

	coul_write(ic_dev, COUL_STORE_OCV_ADDR,
			(u8 *)(&ocvreg), REG_NUM);

	coul_ic_info("save ocv, ocv=%d,reg=0x%x", ocv, ocvreg);
}

void coul_clear_ocv(void)
{
	unsigned short ocvreg = 0;

	coul_write(ic_dev, COUL_STORE_OCV_ADDR,
			(u8 *)(&ocvreg), 2); /* 2: regs_num */
}

/*
 * Description: coul save ocv temp
 * Input: ocv_temp: temp*10
 */
void coul_set_ocv_temp(int ocv_temp)
{
	coul_write(ic_dev, COUL_OCV_TEMP_ADDR,
			(u8 *)(&ocv_temp), 2); /* 2: regs_num */
	coul_ic_debug("ocv_temp=%d\n", ocv_temp);
}

/*
 * Description: get saved ocv temp
 * Return: OCV temp(temp*10)
 */
int coul_get_ocv_temp(void)
{
	int ret;
	int ocv_temp = 0;

	ret = coul_read(ic_dev, COUL_OCV_TEMP_ADDR,
			(u8 *)(&ocv_temp), 2); /* 2: regs_num */
	if (ret)
		return ret;

	coul_ic_debug("ocv_temp=%d\n", ocv_temp);
	return ocv_temp;
}

/*
 * Description: get coulomb total(in and out) time
 * Return:  sum of total time
 */
static unsigned int coul_read_coul_time(void)
{
	unsigned int cl_in_time = 0;
	unsigned int cl_out_time = 0;
	int ret;

	ret = coul_read(ic_dev, TOP_REG_SW_COUL1_CHARGE_TIME_0,
			(u8 *)(&cl_in_time), 4); /* 4: regs_num */
	if (ret)
		goto err;

	ret = coul_read(ic_dev, TOP_REG_SW_COUL1_DISCHARGE_TIME_0,
			(u8 *)(&cl_out_time), 4); /* 4: regs_num */
	if (ret)
		goto err;

	return (cl_in_time + cl_out_time);
err:
	return ret;
}

/* Description: clear coulomb total(in and out) time */
static void coul_rst_coul_time(void)
{
	unsigned int cl_time = 0;
	int ret;

	ret = coul_write(ic_dev, TOP_REG_SW_COUL1_CHARGE_TIME_0,
			(u8 *)(&cl_time), 4); /* 4: regs_num */
	if (ret)
		return;

	coul_write(ic_dev, TOP_REG_SW_COUL1_DISCHARGE_TIME_0,
			(u8 *)(&cl_time), 4); /* 4: regs_num */
}

static void coul_rst_cc_register(void)
{
	u64 reg = 0;

	coul_write(ic_dev, TOP_REG_SW_COUL1_Q_OUT_CFG_0, (u8 *)(&reg), CC_REG_NUM);
	coul_write(ic_dev, TOP_REG_SW_COUL1_Q_IN_CFG_0, (u8 *)(&reg), CC_REG_NUM);

	reg |= EN_SW_Q_IN_OUT_CFG;
	coul_write(ic_dev, TOP_REG_SW_COUL1_DEBUG_EN, (u8 *)(&reg), 1);
}

/* Description: coul calibration */
static void coul_cali_adc(void)
{
	unsigned char reg_val = 1;
	unsigned long flags;

	spin_lock_irqsave(&ic_dev->cali_status_lock, flags);
	if (cali_flag == COUL_CALI_ING) {
		coul_ic_info("cali ing, don't do it again!\n");
		spin_unlock_irqrestore(&ic_dev->cali_status_lock, flags);
		return;
	}

	cali_flag = COUL_CALI_ING;
	spin_unlock_irqrestore(&ic_dev->cali_status_lock, flags);

	coul_write(ic_dev, TOP_REG_SW_COUL1_CAL_EN,
			&reg_val, 1);
}

int coul_get_batt_vol_regval(unsigned int *regval)
{
	int ret;

	if (regval == NULL) {
		coul_ic_err("regval is NULL\n");
		ret = -EINVAL;
		return ret;
	}

	ret = coul_read(ic_dev, TOP_REG_COUL1_V_RO_0, (u8 *)(regval), REG_NUM);
	if (ret)
		return ret;

	return 0;
}

int coul_get_batt_cur_regval(unsigned int *regval)
{
	int ret;

	if (regval == NULL) {
		coul_ic_err("regval is NULL\n");
		ret = -EINVAL;
		return ret;
	}

	ret = coul_read(ic_dev, TOP_REG_COUL1_I_RO_0, (u8 *)(regval), REG_NUM);
	if (ret)
		return ret;

	return 0;
}

int coul_get_chipid(u64 *chipid)
{
	int ret;

	if (chipid == NULL) {
		coul_ic_err("chipid is NULL\n");
		ret = -EINVAL;
		return ret;
	}

	ret = coul_read(ic_dev, OTP_REG_LOTID1_2_REG, (u8 *)(chipid), sizeof(*chipid));
	if (ret)
		return ret;

	return 0;
}

int coul_get_batt_vol(void)
{
	int ret;
	int uv;
	unsigned int regval = 0;

	ret = coul_get_batt_vol_regval(&regval);
	if (ret)
		return ret;

	uv = coul_regval_convert_uv(regval);

	coul_ic_debug("regval: 0x%x, uv: %d\n", regval, uv);
	return uv;
}

int coul_get_batt_cur(void)
{
	int ret;
	int ua;
	unsigned int regval = 0;

	ret = coul_get_batt_cur_regval(&regval);
	if (ret)
		return ret;

	ua = coul_regval_convert_ua(regval);
	coul_ic_err("regval: 0x%x, ua: %d\n", regval, ua);

	return ua;
}

static void coul_fifo_data_lock(void)
{
	u8 val = COUL_FIFO_LOCK;

	coul_write(ic_dev, TOP_REG_SW_COUL_DATA_LOCK, &val, 1);
}

static void coul_fifo_data_unlock(void)
{
	u8 val = COUL_FIFO_UNLOCK;

	coul_write(ic_dev, TOP_REG_SW_COUL_DATA_LOCK, &val, 1);
}
/*
 * Description:   get battery vol in uv from fifo
 * Intput: *data: data from fifo
 */
void coul_get_batt_vol_fifo(int *data)
{
	int ret;
	int fifo_order;
	unsigned int regval = 0;
	int vol_fifo[FIFO_DEPTH] = {0};

	coul_fifo_data_lock();

	for (fifo_order = 0; fifo_order < FIFO_DEPTH; fifo_order++) {
		ret = coul_read(ic_dev, TOP_REG_COUL1_V_RO_0_0 + fifo_order * REG_NUM,
				(u8 *)(&regval), REG_NUM);
		if (ret)
			continue;
		else {
			vol_fifo[fifo_order] = coul_regval_convert_uv(regval);
			coul_ic_debug("regval:0x%x, vol_fifo[%d]:%d\n", regval,
				fifo_order, vol_fifo[fifo_order]);
		}
	}

	coul_fifo_data_unlock();

	memcpy(data, vol_fifo, FIFO_DEPTH * sizeof(int));
}

/*
 * Description: get battery cur in ua from fifo
 * Intput: *data: data from fifo
 */

void coul_get_batt_cur_fifo(int *data)
{
	int ret;
	int fifo_order;
	unsigned int regval = 0;
	static int cur_fifo[FIFO_DEPTH];

	coul_fifo_data_lock();

	for (fifo_order = 0; fifo_order < FIFO_DEPTH; fifo_order++) {
		ret = coul_read(ic_dev, TOP_REG_COUL1_I_RO_0_0 + fifo_order * REG_NUM,
				(u8 *)(&regval), REG_NUM);
		if (ret) {
			continue;
		} else {
			cur_fifo[fifo_order] = coul_regval_convert_ua(regval);
			coul_ic_debug("regval:0x%x, cur_fifo[%d]:%d\n", regval,
				fifo_order, cur_fifo[fifo_order]);
		}
	}

	coul_fifo_data_unlock();

	memcpy(data, cur_fifo, FIFO_DEPTH * sizeof(int));
}

/*
 * Description: get battery cur in ua from fifo
 * Intput: fifo_order:fifo serial number 0-9
 */
int coul_get_batt_cur_from_fifo(short fifo_order)
{
	int ret;
	int ua;
	unsigned int regval = 0;

	if (fifo_order > FIFO_DEPTH)
		fifo_order = 0;

	coul_fifo_data_lock();

	ret = coul_read(ic_dev, TOP_REG_COUL1_I_RO_0_0 + REG_NUM * fifo_order,
				(u8 *)(&regval), REG_NUM);
	if (ret)
		return ret;

	ua = coul_regval_convert_ua(regval);

	coul_ic_debug("0x%x, ua %d\n", regval, ua);

	coul_fifo_data_unlock();

	return ua;
}

/* Description: show key register info for bug */
void coul_dump_reg(void)
{
	int ret;
	unsigned int reg1 = 0;
	unsigned int reg2 = 0;
	unsigned int reg3 = 0;
	unsigned int reg4 = 0;
	unsigned int reg5 = 0;

	ret = coul_read(ic_dev, TOP_REG_COUL1_V_RO_0_0,
				(u8 *)(&reg1), REG_NUM);
	if (ret)
		return;

	ret = coul_read(ic_dev, TOP_REG_COUL1_I_RO_0_0,
				(u8 *)(&reg2), REG_NUM);
	if (ret)
		return;

	ret = coul_read(ic_dev, TOP_REG_COUL1_CAL_I_0,
				(u8 *)(&reg3), REG_NUM);
	if (ret)
		return;

	ret = coul_read(ic_dev, TOP_REG_COUL1_V_RO_0,
				(u8 *)(&reg4), REG_NUM);
	if (ret)
		return;

	ret = coul_read(ic_dev, TOP_REG_COUL1_I_RO_0,
				(u8 *)(&reg5), REG_NUM);
	if (ret)
		return;

	coul_ic_debug("\n0x001d~0x001f(vol fifo) = 0x%x,", reg1);
	coul_ic_debug("0x0035~0x0037(cur fifo) = 0x%x\n", reg2);
	coul_ic_debug("0x0009-0x000b(capcity offset) = 0x%x,\n", reg3);
	coul_ic_debug("0x0012~0x0014(vol)= 0x%x,", reg4);
	coul_ic_debug("0x0015~0x0017(cur)= 0x%x,\n", reg5);
}

/* Remark: coul eco follow pmu eco */
static void coul_lp_enable(void)
{
	int ret;
	u64 eco_in_reg = 0;
	u64 eco_out_reg = 0;

	ret = coul_read(ic_dev, TOP_REG_COUL1_CAL_Q_IN_0,
				(u8 *)(&eco_in_reg), CC_REG_NUM);
	if (ret)
		return;

	ret = coul_read(ic_dev, TOP_REG_COUL1_CAL_Q_OUT_0,
				(u8 *)(&eco_out_reg), CC_REG_NUM);
	if (ret)
		return;

	last_lp_q_in = eco_in_reg;
	last_lp_q_out = eco_out_reg;
}
/*
 * Description: calculate capacity leak from existing ECO MODE to calc soc first time
 * Remark: ECO uah register keep the same value after exist from ECO
 */
static int coul_get_lp_compensate_cc(void)
{
	int ret;
	int rst_uah;
	int eco_uah;
	int cur_uah;
	int eco_in_uah;
	int eco_out_uah;
	int present_in_uah;
	int present_out_uah;
	u64 in_val = 0;
	u64 out_val = 0;

	ret = coul_read(ic_dev, TOP_REG_COUL1_Q_IN_LP_0,
				(u8 *)(&in_val), CC_REG_NUM);
	if (ret)
		goto err;

	ret = coul_read(ic_dev, TOP_REG_COUL1_Q_OUT_LP_0,
				(u8 *)(&out_val), CC_REG_NUM);
	if (ret)
		goto err;

	/* if: first time to calc soc after exiting from ECO Mode */
	if ((last_lp_q_in != in_val) || (last_lp_q_out != out_val)) {
		eco_out_uah = coul_regval_convert_uah(out_val);
		eco_in_uah = coul_regval_convert_uah(in_val);
		eco_uah = eco_out_uah - eco_in_uah;
		/* current cc */

		ret = coul_read(ic_dev, TOP_REG_COUL1_CAL_Q_IN_0,
			(u8 *)(&in_val), CC_REG_NUM);
		if (ret)
			goto err;

		ret = coul_read(ic_dev, TOP_REG_COUL1_CAL_Q_OUT_0,
					(u8 *)(&out_val), CC_REG_NUM);
		if (ret)
			goto err;

		present_in_uah = coul_regval_convert_uah(in_val);
		present_out_uah = coul_regval_convert_uah(out_val);
		cur_uah = present_out_uah - present_in_uah;
		/* leak cc from exisingt eco mode to first calc soc */
		rst_uah = cur_uah - eco_uah;

		coul_ic_err(
			"eco_in=%d,eco_out=%d,cc_in=%d,cc_out=%d,leakcc=%d\n",
			eco_in_uah, eco_out_uah, present_in_uah,
			present_out_uah, rst_uah);
	} else {
		rst_uah = 0;
		coul_ic_info(
			"Not the FIRST calc soc out eco, leak cc=0\n");
	}
	return rst_uah;
err:
	return ret;
}

static void coul_set_last_soc(short soc)
{
	u8 val;

	if (soc < 0)
		soc = 0;
	val = (u8)soc;
	val &= SAVE_LAST_SOC_MASK;
	val |= SAVE_LAST_SOC_FLAG;

	coul_ic_info("save_soc:%d, reg_val:0x%x\n", soc, val);
	coul_write(ic_dev, COUL_LAST_SOC_ADDR,
			&val, 1);
}

static void coul_read_last_soc(short *soc)
{
	int ret;
	u8 val;

	ret = coul_read(ic_dev, COUL_LAST_SOC_ADDR,
			&val, 1);
	if (ret)
		return;

	coul_ic_debug("last_soc:%lu, reg_val:0x%x\n", (val & SAVE_LAST_SOC_MASK), val);
	*soc = (short)(val & SAVE_LAST_SOC_MASK);
}

static void coul_rst_last_soc_flag(void)
{
	u8 val = 0;
	int ret;

	ret = coul_read(ic_dev, COUL_LAST_SOC_ADDR,
		&val, 1);
	if (ret)
		return;

	val &= (~SAVE_LAST_SOC_FLAG);

	coul_write(ic_dev, COUL_LAST_SOC_ADDR,
			&val, 1);
	coul_ic_debug("clear last soc flag !\n");
}

static void coul_read_last_soc_flag(bool *valid)
{
	int ret;
	u8 val;

	ret = coul_read(ic_dev, COUL_LAST_SOC_ADDR,
		&val, 1);
	if (ret)
		return;

	coul_ic_debug("reg_val:0x%x\n", val);
	*valid = (val &= SAVE_LAST_SOC_FLAG) ? true : false;
}

static void coul_set_ocv_level(int level)
{
	int ret;
	u8 val;

	ret = coul_read(ic_dev, COUL_OCV_LEVEL_ADDR,
			&val, 1);
	if (ret)
		return;

	val &= ~SET_OCV_LEVEL_MASK;
	val |= (level & SET_OCV_LEVEL_MASK);

	coul_ic_debug("level:%d, regval:0x%x\n", level, val);
	coul_write(ic_dev, COUL_OCV_LEVEL_ADDR,
			&val, 1);
}

static int coul_read_ocv_level(void)
{
	int val;
	int ret;

	ret = coul_read(ic_dev, COUL_OCV_LEVEL_ADDR,
			(u8 *)(&val), 1);
	if (ret)
		return ret;

	val &= SET_OCV_LEVEL_MASK;

	coul_ic_debug("regval:0x%x\n", val);
	return val;
}

static int coul_is_new_battery(int *new_batt)
{
	int val;
	int ret;

	if (new_batt == NULL) {
		coul_ic_err("new_batt is NULL\n");
		ret = -EINVAL;
		return ret;
	}

	ret = coul_read(ic_dev, COUL_NEW_BATT,
			(u8 *)(&val), 1);
	if (ret)
		return ret;

	if (val & COUL_NEW_BATT_FLAG)
		*new_batt = 1;
	else
		*new_batt = 0;

	return ret;
}

struct coul_ic_ops coul_hw_ops = {
	.get_cc = get_cc_uah,
	.set_cc = set_cc_uah,
	.get_fifo_depth = coul_get_fifo_depth,
	.get_nv_read_flag = coul_get_nv_read_flag,
	.set_nv_save_flag = coul_set_nv_save_flag,
	.get_use_saved_ocv_flag = coul_get_use_saved_ocv_flag,
	.get_ocv_update_fcc_flag = coul_get_ocv_update_fcc_flag,
	.set_ocv_update_fcc_flag = coul_set_ocv_update_fcc_flag,
	.set_ocv = coul_set_ocv,
	.get_ocv = coul_get_ocv,
	.clear_ocv = coul_clear_ocv,
	.set_ocv_temp = coul_set_ocv_temp,
	.get_ocv_temp = coul_get_ocv_temp,
	.set_low_vol_alert_thr = coul_set_low_vol_alert_thr,
	.read_coul_time = coul_read_coul_time,
	.rst_coul_time = coul_rst_coul_time,
	.rst_cc = coul_rst_cc_register,
	.cali_adc = coul_cali_adc,
	.get_batt_vol = coul_get_batt_vol,
	.get_batt_cur = coul_get_batt_cur,
	.get_batt_vol_fifo = coul_get_batt_vol_fifo,
	.get_batt_cur_fifo = coul_get_batt_cur_fifo,
	.get_batt_cur_from_fifo = coul_get_batt_cur_from_fifo,
	.dump_reg = coul_dump_reg,
	.lp_enable = coul_lp_enable,
	.get_lp_compensate_cc = coul_get_lp_compensate_cc,
	.set_last_soc = coul_set_last_soc,
	.read_last_soc = coul_read_last_soc,
	.rst_last_soc_flag = coul_rst_last_soc_flag,
	.read_last_soc_flag = coul_read_last_soc_flag,
	.set_ocv_level = coul_set_ocv_level,
	.read_ocv_level = coul_read_ocv_level,
	.set_curr_in_alert_thr = coul_set_curr_in_alert_thr,
	.set_curr_out_alert_thr = coul_set_curr_out_alert_thr,
	.get_adc_tbatt = coul_get_adc_tbatt,
	.regval_convert_uv = coul_regval_convert_uv,
	.is_new_battery = coul_is_new_battery,
};

int coul_devops_register(struct coul_core_device *chip)
{
	if (!chip)
		return -EINVAL;

	if (!coul_core_ops) {
		coul_err("coul_core_devops is NULL\n");
		return -EINVAL;
	}

	chip->core_ic_ops = coul_core_ops;

	return 0;
}

static int coul_ic_probe(struct platform_device *pdev)
{
	struct coul_ic_device *chip = NULL;
	struct device_node *np = NULL;
	int retval;

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->dev = &pdev->dev;

	np = chip->dev->of_node;
	if (!np) {
		coul_ic_err("np is null!\n");
		return -EINVAL;
	}

	chip->regmap = dev_get_regmap(chip->dev->parent, NULL);
	if (!chip->regmap) {
		coul_ic_err("Parent regmap is unavailable\n");
		return -ENXIO;
	}

	platform_set_drvdata(pdev, chip);

	ic_dev = chip;

	spin_lock_init(&chip->cali_status_lock);

	retval = coul_request_batt_adc(chip);
	if (retval) {
		coul_ic_err("Failed to request batt adc iio, rc=%d\n", retval);
		goto failed_1;
	}

	retval = coul_request_irqs(chip);
	if (retval) {
		coul_ic_err("Failed to register QG interrupts, rc=%d\n", retval);
		goto failed_1;
	}

	mutex_init(&chip->bus_lock);

	/* config coul ctrl and irq */
	coul_chip_init();

	/* set shutdown vol level */
	coul_set_low_vol_alert_thr(DEFAULT_BATTERY_VOL_0_PERCENT);

	coul_core_ops = &coul_hw_ops;

	retval = of_property_read_u32(
		of_find_compatible_node(NULL, NULL, "xring,coul_core"),
		"coul_curr_mohm", &coul_curr_mohm);
	if (retval) {
		coul_curr_mohm = COUL_C_R_MOHM;
		coul_ic_err(
			"get coul_curr_mohm fail, use default value 10 mohm!\n");
	}

	coul_debug("coul ic probe success!!!\n");
	return 0;

failed_1:
	platform_set_drvdata(pdev, NULL);
	coul_ic_err("xr coul hardware probe failed!\n");
	return retval;
}

static int coul_ic_remove(struct platform_device *pdev)
{
	struct coul_ic_device *chip = platform_get_drvdata(pdev);

	if (!chip) {
		coul_ic_err("chip is null\n");
		return -EINVAL;
	}

	mutex_destroy(&chip->bus_lock);

	devm_kfree(&pdev->dev, chip);

	return 0;
}

static const struct of_device_id coul_match_table[] = {
	{
		.compatible = "xring,coul_xp2210",
	},
	{
	},
};

static struct platform_driver coul_driver = {
	.probe = coul_ic_probe,
	.remove = coul_ic_remove,
	.driver = {
		.name = "xr_cul_xp2210",
		.owner = THIS_MODULE,
		.of_match_table = coul_match_table,
	},
};

int coul_ic_init(void)
{
	return platform_driver_register(&coul_driver);
}

void coul_ic_exit(void)
{
	platform_driver_unregister(&coul_driver);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("XRing Technologies Co., Ltd");
MODULE_DESCRIPTION("xr coul xp2210 driver");
