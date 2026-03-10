// SPDX-License-Identifier: GPL-2.0
/*
* Copyright (c) 2022 bq Semiconductor Technology(Shanghai) Co., Ltd.
*/
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/err.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/machine.h>
#include <linux/debugfs.h>
#include <linux/bitops.h>
#include <linux/math64.h>
#include <linux/regmap.h>
#include <linux/iio/iio.h>
#include <linux/iio/consumer.h>
#include <dt-bindings/iio/qti_power_supply_iio.h>
#include "bq25960_charger_iio.h"
#include "bq25960_charger.h"
#include "bq25960h_charger.h"

#define BQ25960_DRV_VERSION              "1.0.0_G"
#define BQ25960_CHIP_VENDOR               1

static struct mutex g_i2c_lock;

/********************COMMON API***********************/
__maybe_unused static u8 val2reg(enum bq25960_reg_range id, u32 val)
{
    int i;
    u8 reg;
    const struct reg_range *range= &bq25960_reg_range[id];

    if (!range)
        return val;

    if (range->table) {
        if (val <= range->table[0])
            return 0;
        for (i = 1; i < range->num_table - 1; i++) {
            if (val == range->table[i])
                return i;
            if (val > range->table[i] &&
                val < range->table[i + 1])
                return range->round_up ? i + 1 : i;
        }
        return range->num_table - 1;
    }

    if (val <= range->min)
        reg = 0;
    else if (val >= range->max)
        reg = (range->max - range->offset) / range->step;
    else if (range->round_up)
        reg = (val - range->offset) / range->step + 1;
    else
        reg = (val - range->offset) / range->step;

    return reg;
}

__maybe_unused static u32 reg2val(enum bq25960_reg_range id, u8 reg)
{
    const struct reg_range *range= &bq25960_reg_range[id];

    if (!range)
        return reg;

    return range->table ? range->table[reg] :
                  range->offset + range->step * reg;
}

/*********************************************************/
static int bq25960_field_read(struct bq25960 *bq,
                enum bq25960_fields field_id, int *val)
{
    int ret;

    ret = regmap_field_read(bq->rmap_fields[field_id], val);
    if (ret < 0) {
        dev_err(bq->dev, "bq25960 read field %d fail: %d\n", field_id, ret);
    }

    return ret;
}

static int bq25960_field_write(struct bq25960 *bq,
                enum bq25960_fields field_id, int val)
{
    int ret;

    ret = regmap_field_write(bq->rmap_fields[field_id], val);
    if (ret < 0) {
        dev_err(bq->dev, "bq25960 write field %d fail: %d\n", field_id, ret);
    }

    return ret;
}

static int bq25960_read_block(struct bq25960 *bq,
                int reg, uint8_t *val, int len)
{
    int ret;

    ret = regmap_bulk_read(bq->regmap, reg, val, len);
    if (ret < 0) {
        dev_err(bq->dev, "bq25960 read %02x block failed %d\n", reg, ret);
    }

    return ret;
}

/*******************************************************/
static int bq25960_detect_device(struct bq25960 *bq)
{
    int ret;
    int val;

    ret = bq25960_field_read(bq, DEVICE_ID, &val);
    if (ret < 0) {
        dev_err(bq->dev, "%s fail(%d)\n", __func__, ret);
        return ret;
    }

    if (val != BQ25960_DEVICE_ID) {
        dev_info(bq->dev, "%s not find BQ25960, ID = 0x%02x\n", __func__, ret);

        return -EINVAL;
    }

    return ret;
}

static int bq25960_reg_reset(struct bq25960 *bq)
{
    return bq25960_field_write(bq, REG_RST, 1);
}

static int bq25960_adjust_ovp(struct bq25960 *bq)
{
    int ret;

    //modify the VAC_OVP to 14V
    ret = bq25960_field_write(bq, VAC1_OVP, 3);
    ret = bq25960_field_write(bq, VAC2_OVP, 3);

    return ret;
}

__maybe_unused static int bq25960_dump_reg(struct bq25960 *bq)
{
    int ret;
    int i;
    int val;

    for (i = 0; i <= BQ25960_REGMAX; i++) {
        ret = regmap_read(bq->regmap, i, &val);
        dev_info(bq->dev, "%s reg[0x%02x] = 0x%02x\n",
                __func__, i, val);
    }

    return ret;
}

static int bq25960_enable_charge(struct bq25960 *bq, bool en)
{
    int ret;

    ret = bq25960_field_write(bq, IBUS_UCP, 1);
    ret = bq25960_field_write(bq,CHG_CONFIG_2, 1);
    ret = bq25960_field_write(bq,TSBUS_FLT_DIS, 1);
    ret = bq25960_field_write(bq,TSBAT_FLT_DIS, 1);
    ret = bq25960_field_write(bq,WD_TIMEOUT_DIS, 1);
    ret = bq25960_field_write(bq, CHG_EN, !!en);

    return ret;
}
static int bq25960_check_charge_enabled(struct bq25960 *bq, bool *enabled)
{
    int ret, val;

    ret = bq25960_field_read(bq, CP_SWITCHING_STAT, &val);
    *enabled = (bool)val;

    return ret;
}

__maybe_unused
static int bq25960_set_ovp_gate(struct bq25960 *bq, bool en)
{
    int ret;

    dev_info(bq->dev,"%s:%d",__func__,en);

    if (en) {
        ret = bq25960_field_write(bq, OTG_EN, 1);
        msleep(10);
        ret = bq25960_field_write(bq, ACDRV_BOTH, 0);
        ret = bq25960_field_write(bq, ACDRV1_STAT, 1);
    } else {
        ret = bq25960_field_write(bq, VBUS_PD_EN, 1);
        msleep(10);
        ret = bq25960_field_write(bq, ACDRV1_STAT, 0);
        ret = bq25960_field_write(bq, OTG_EN, 0);
    }

    return ret;
}

static int bq25960_set_otg_preconfigure(struct bq25960 *bq, bool en)
{
    int ret;

    ret = regmap_write(bq->regmap, 0x10, 0x87);
    if (ret < 0) {
        pr_err("regmap_write 0x10 fail ret:%d\n", ret);
    }

    ret = regmap_write(bq->regmap, 0x0E, 0x6C);
    if (ret != 0) {
        dev_err(bq->dev, "%s regmap_write 0x0E fail ret:%d\n", __func__, ret);
    }

    ret = regmap_write(bq->regmap, 0x06, 0x6C);
    if (ret != 0) {
        dev_err(bq->dev, "%s regmap_write 0x06 fail ret:%d \n", __func__, ret);
    }

    ret = regmap_write(bq->regmap, 0x0A, 0x6C);
    if (ret != 0) {
        dev_err(bq->dev, "%s regmap_write 0x0A fail ret:%d\n", __func__, ret);
    }

    ret = regmap_write(bq->regmap, 0x9A, 0x34);
    if (ret != 0) {
        dev_err(bq->dev, "%s regmap_write 0x9A fail ret:%d\n", __func__, ret);
    }

    ret = regmap_write(bq->regmap, 0x9B, 0x5B);
    if (ret != 0) {
        dev_err(bq->dev, "%s regmap_write 0x9B fail ret:%d\n", __func__, ret);
    }

    ret = regmap_write(bq->regmap, 0xA0, 0x80);
    if (ret != 0) {
        dev_err(bq->dev, "%s regmap_write 0xA0 fail ret:%d\n", __func__, ret);
    }
    bq25960h_set_otg_preconfigure(en);

    return ret;
}

static int bq25960_set_otg(struct bq25960 *bq, bool en)
{
    int ret = 0;
    int value = 0;

    dev_err(bq->dev, "%s start \n", __func__);

    ret = bq25960_field_read(bq, ADC_EN, &value);
    if (value)
        ret |= bq25960_field_write(bq, ADC_EN, false);

    if (en) {//open otg 9v
        bq->otg_flag = true;
        mutex_lock(&g_i2c_lock);
        bq25960h_enable_otg(true);
        mutex_unlock(&g_i2c_lock);
    } else {//close otg
        bq->otg_flag = false;
        mutex_lock(&g_i2c_lock);
        bq25960h_enable_otg(false);
        mutex_unlock(&g_i2c_lock);
    }

    dev_err(bq->dev, "%s %d \n", __func__, en);

    return ret;
}

__maybe_unused static int bq25960_get_status(struct bq25960 *bq, uint32_t *status)
{
    int ret, val;

    *status = 0;

    ret = bq25960_field_read(bq, VBUS_ERRORHI_STAT, &val);
    if (ret < 0) {
        dev_err(bq->dev, "%s fail to read VBUS_ERRORHI_STAT(%d)\n", __func__, ret);
        return ret;
    }
    if (val != 0)
        *status |= BIT(ERROR_VBUS_HIGH);

    return ret;
}

static int bq25960_enable_adc(struct bq25960 *bq, bool en)
{
    dev_err(bq->dev,"%s:%d", __func__, en);
    return bq25960_field_write(bq, ADC_EN, !!en);
}
__maybe_unused static int bq25960_set_adc_scanrate(struct bq25960 *bq, bool oneshot)
{
    dev_info(bq->dev,"%s:%d",__func__,oneshot);
    return bq25960_field_write(bq, ADC_RATE, !!oneshot);
}

static int bq25960_get_adc_ibus(struct bq25960 *bq)
{
    int ibus_adc_lsb, ibus_adc_msb;
    u16 ibus_adc;
    int ret;

    ret = regmap_read(bq->regmap, BQ25960_IBUS_ADC_MSB, &ibus_adc_msb);
    if (ret)
        return ret;

    ret = regmap_read(bq->regmap, BQ25960_IBUS_ADC_LSB, &ibus_adc_lsb);
    if (ret)
        return ret;

    ibus_adc = (ibus_adc_msb << 8) | ibus_adc_lsb;

    if (ibus_adc_msb & BQ25960_ADC_POLARITY_BIT)
        return ((ibus_adc ^ 0xffff) + 1);

    return ibus_adc;
}

static int bq25960_get_adc_vbus(struct bq25960 *bq)
{
    int vbus_adc_lsb, vbus_adc_msb;
    u16 vbus_adc;
    int ret;

    ret = regmap_read(bq->regmap, BQ25960_VBUS_ADC_MSB, &vbus_adc_msb);
    if (ret)
        return ret;

    ret = regmap_read(bq->regmap, BQ25960_VBUS_ADC_LSB, &vbus_adc_lsb);
    if (ret)
        return ret;

    vbus_adc = (vbus_adc_msb << 8) | vbus_adc_lsb;

    return vbus_adc;
}

static int bq25960_get_adc_ibat(struct bq25960 *bq)
{
    int ret;
    int ibat_adc_lsb, ibat_adc_msb;
    int ibat_adc;

    ret = regmap_read(bq->regmap, BQ25960_IBAT_ADC_MSB, &ibat_adc_msb);
    if (ret)
        return ret;

    ret = regmap_read(bq->regmap, BQ25960_IBAT_ADC_LSB, &ibat_adc_lsb);
    if (ret)
        return ret;

    ibat_adc = (ibat_adc_msb << 8) | ibat_adc_lsb;

    if (ibat_adc_msb & BQ25960_ADC_POLARITY_BIT)
        return ((ibat_adc ^ 0xffff) + 1);

    return ibat_adc;
}

static int bq25960_get_adc_vbat(struct bq25960 *bq)
{
    int vbat_adc_lsb, vbat_adc_msb;
    u16 vbat_adc;
    int ret;

    ret = regmap_read(bq->regmap, BQ25960_VBAT_ADC_MSB, &vbat_adc_msb);
    if (ret)
        return ret;

    ret = regmap_read(bq->regmap, BQ25960_VBAT_ADC_LSB, &vbat_adc_lsb);
    if (ret)
        return ret;

    vbat_adc = (vbat_adc_msb << 8) | vbat_adc_lsb;

    return vbat_adc;
}

__maybe_unused static int bq25960_get_adc_tbat(struct bq25960 *bq)
{
    int tbat_adc_lsb, tbat_adc_msb;
    u16 tbat_adc;
    int ret;

    ret = regmap_read(bq->regmap, BQ25960_TSBAT_ADC_MSB, &tbat_adc_msb);
    if (ret)
        return ret;

    ret = regmap_read(bq->regmap, BQ25960_TSBAT_ADC_LSB, &tbat_adc_lsb);
    if (ret)
        return ret;

    tbat_adc = (tbat_adc_msb << 8) | tbat_adc_lsb;

    return tbat_adc;
}

__maybe_unused static int bq25960_get_adc_tbus(struct bq25960 *bq)
{
    int tbus_adc_lsb, tbus_adc_msb;
    u16 tbus_adc;
    int ret;

    ret = regmap_read(bq->regmap, BQ25960_TSBUS_ADC_MSB, &tbus_adc_msb);
    if (ret)
        return ret;

    ret = regmap_read(bq->regmap, BQ25960_TSBUS_ADC_LSB, &tbus_adc_lsb);
    if (ret)
        return ret;

    tbus_adc = (tbus_adc_msb << 8) | tbus_adc_lsb;

    return tbus_adc;
}

__maybe_unused static int bq25960_get_adc_tdie(struct bq25960 *bq)
{
    int tdie_adc_lsb, tdie_adc_msb;
    u16 tdie_adc;
    int ret;

    ret = regmap_read(bq->regmap, BQ25960_TSBUS_ADC_MSB, &tdie_adc_msb);
    if (ret)
        return ret;

    ret = regmap_read(bq->regmap, BQ25960_TSBUS_ADC_LSB, &tdie_adc_lsb);
    if (ret)
        return ret;

    tdie_adc = (tdie_adc_msb << 8) | tdie_adc_lsb;

    return tdie_adc;
}

__maybe_unused static int bq25960_set_busovp_th(struct bq25960 *bq, int threshold)
{
    int reg_val = val2reg(BQ25960_VBUS_OVP, threshold);

    dev_info(bq->dev,"%s:%d-%d", __func__, threshold, reg_val);

    return bq25960_field_write(bq, VBUS_OVP, reg_val);
}
__maybe_unused static int bq25960_set_busocp_th(struct bq25960 *bq, int threshold)
{
    int reg_val = val2reg(BQ25960_IBUS_OCP, threshold);

    dev_info(bq->dev,"%s:%d-%d", __func__, threshold, reg_val);

    return bq25960_field_write(bq, IBUS_OCP, reg_val);
}
__maybe_unused static int bq25960_set_batovp_th(struct bq25960 *bq, int threshold)
{
    int reg_val = val2reg(BQ25960_VBAT_OVP, threshold);

    dev_info(bq->dev,"%s:%d-%d", __func__, threshold, reg_val);

    return bq25960_field_write(bq, VBAT_OVP, reg_val);
}
__maybe_unused static int bq25960_set_batocp_th(struct bq25960 *bq, int threshold)
{
    int reg_val = val2reg(BQ25960_IBAT_OCP, threshold);

    dev_info(bq->dev,"%s:%d-%d", __func__, threshold, reg_val);

    return bq25960_field_write(bq, IBAT_OCP, reg_val);
}
__maybe_unused static int bq25960_set_vbusovp_alarm(struct bq25960 *bq, int threshold)
{
    int reg_val = val2reg(BQ25960_VBUS_OVP_ALM, threshold);

    dev_info(bq->dev,"%s:%d-%d", __func__, threshold, reg_val);

    return bq25960_field_write(bq, VBUS_OVP_ALM, reg_val);
}
__maybe_unused static int bq25960_set_vbatovp_alarm(struct bq25960 *bq, int threshold)
{
    int reg_val = val2reg(BQ25960_VBAT_OVP_ALM, threshold);

    dev_info(bq->dev,"%s:%d-%d", __func__, threshold, reg_val);
    return bq25960_field_write(bq, VBAT_OVP_ALM, reg_val);
}
__maybe_unused static int bq25960_disable_vbatovp_alarm(struct bq25960 *bq, bool en)
{
    int ret;

    dev_info(bq->dev,"%s:%d",__func__,en);
    ret = bq25960_field_write(bq, VBAT_OVP_ALM_DIS, !!en);

    return ret;
}
__maybe_unused static int bq25960_disable_vbusovp_alarm(struct bq25960 *bq, bool en)
{
    int ret;

    dev_info(bq->dev,"%s:%d",__func__,en);
    ret = bq25960_field_write(bq, VBUS_OVP_ALM_DIS, !!en);

    return ret;
}

static int bq25960_init_device(struct bq25960 *bq)
{
    int ret = 0;
    int i;

    struct {
        enum bq25960_fields field_id;
        int conv_data;
    } props[] = {
        {VBAT_OVP_DIS, bq->cfg.vbat_ovp_dis},
        {VBAT_OVP, bq->cfg.vbat_ovp},
        {VBAT_OVP_ALM_DIS, bq->cfg.vbat_ovp_alm_dis},
        {VBAT_OVP_ALM, bq->cfg.vbat_ovp_alm},
        {IBAT_OCP_DIS, bq->cfg.ibat_ocp_dis},
        {IBAT_OCP, bq->cfg.ibat_ocp},
        {IBAT_OCP_ALM_DIS, bq->cfg.ibat_ocp_alm_dis},
        {IBAT_OCP_ALM, bq->cfg.ibat_ocp_alm},
        {IBUS_UCP_DIS, bq->cfg.ibus_ucp_dis},
        {IBUS_UCP, bq->cfg.ibus_ucp},
        //{VBUS_IN_RANGE_DIS, bq->cfg.vbus_in_range_dis},
        {CHG_CONFIG_2, bq->cfg.chg_config_2},
        {VBUS_PD_EN, bq->cfg.vbus_pd_en},
        {VBUS_OVP, bq->cfg.vbus_ovp},
        {VBUS_OVP_ALM_DIS, bq->cfg.vbus_ovp_alm_dis},
        {VBUS_OVP_ALM, bq->cfg.vbus_ovp_alm},
        //{IBUS_OCP_DIS, bq->cfg.ibus_ocp_dis},
        {IBUS_OCP, bq->cfg.ibus_ocp},
        //{TSHUT_DIS, bq->cfg.tshut_dis},
        {TSBUS_FLT_DIS, bq->cfg.tsbus_flt_dis},
        {TSBAT_FLT_DIS, bq->cfg.tsbat_flt_dis},
        {TDIE_ALM, bq->cfg.tdie_alm},
        {TSBUS_FLT, bq->cfg.tsbus_flt},
        {TSBAT_FLT, bq->cfg.tsbat_flt},
        {VAC1_OVP, bq->cfg.vac1_ovp},
        {VAC2_OVP, bq->cfg.vac2_ovp},
        {VAC1_PD_EN, bq->cfg.vac1_pd_en},
        {VAC2_PD_EN, bq->cfg.vac2_pd_en},
        {FSW_SET, bq->cfg.fsw_set},
        {WD_TIMEOUT, bq->cfg.wd_timeout},
        {WD_TIMEOUT_DIS, bq->cfg.wd_timeout_dis},
        {SET_IBAT_SNS_RES, bq->cfg.ibat_sns_r},
        {SS_TIMEOUT, bq->cfg.ss_timeout},
        {IBUS_UCP_FALL_DG, bq->cfg.ibus_ucp_fall_dg},
        {VOUT_OVP_DIS, bq->cfg.vout_ovp_dis},
        {VOUT_OVP, bq->cfg.vout_ovp},
        {MS, bq->cfg.ms},
    };

    ret = bq25960_reg_reset(bq);
    if (ret < 0) {
        dev_err(bq->dev, "%s Failed to reset registers(%d)\n", __func__, ret);
    }

    msleep(10);

    for (i = 0; i < ARRAY_SIZE(props); i++) {
        ret = bq25960_field_write(bq, props[i].field_id, props[i].conv_data);
    }

    return ret;
}

/********************creat devices note start*************************************************/
static ssize_t bq25960_show_registers(struct device *dev,
                struct device_attribute *attr, char *buf)
{
    struct bq25960 *bq = dev_get_drvdata(dev);
    u8 addr;
    int val;
    u8 tmpbuf[300];
    int len;
    int idx = 0;
    int ret;

    idx = snprintf(buf, PAGE_SIZE, "%s:\n", "bq25960");
    for (addr = 0x0; addr <= BQ25960_REGMAX; addr++) {
        ret = regmap_read(bq->regmap, addr, &val);
        if (ret == 0) {
            len = snprintf(tmpbuf, PAGE_SIZE - idx,
                    "Reg[%.2X] = 0x%.2x\n", addr, val);
            memcpy(&buf[idx], tmpbuf, len);
            idx += len;
        }
    }

    return idx;
}

static ssize_t bq25960_store_register(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
    struct bq25960 *bq = dev_get_drvdata(dev);
    int ret;
    unsigned int reg;
    unsigned int val;
    ret = sscanf(buf, "%x %x", &reg, &val);
    if (ret == 2 && reg <= BQ25960_REGMAX)
        regmap_write(bq->regmap, (unsigned char)reg, (unsigned char)val);
    return count;
}

static DEVICE_ATTR(registers, 0660, bq25960_show_registers, bq25960_store_register);

static void bq25960_create_device_node(struct device *dev)
{
    device_create_file(dev, &dev_attr_registers);
}
/********************creat devices note end*************************************************/
/*
* interrupt does nothing, just info event chagne, other module could get info
* through power supply interface
*/

static void bq25960_check_fault_status(struct bq25960 *bq)
{
    int ret;
    u8 flag = 0;
    int i,j;

    for (i=0;i < ARRAY_SIZE(cp_intr_flag);i++) {
        ret = bq25960_read_block(bq, cp_intr_flag[i].reg, &flag, 1);
        for (j=0; j <  cp_intr_flag[i].len; j++) {
            if (flag & cp_intr_flag[i].bit[j].mask) {
                dev_info(bq->dev,"trigger :%s\n",cp_intr_flag[i].bit[j].name);
            }
        }
    }
}

static irqreturn_t bq25960_irq_handler(int irq, void *data)
{
    struct bq25960 *bq = data;

    dev_info(bq->dev,"INT OCCURED\n");
    bq25960_check_fault_status(bq);
    power_supply_changed(bq->psy);

    return IRQ_HANDLED;
}

static int bq25960_register_interrupt(struct bq25960 *bq)
{
    int ret;

    if (gpio_is_valid(bq->irq_gpio)) {
        ret = gpio_request_one(bq->irq_gpio, GPIOF_DIR_IN,"bq25960_irq");
        if (ret) {
            dev_err(bq->dev,"failed to request bq25960_irq\n");
            return -EINVAL;
        }
        bq->irq = gpio_to_irq(bq->irq_gpio);
        if (bq->irq < 0) {
            dev_err(bq->dev,"failed to gpio_to_irq\n");
            return -EINVAL;
        }
    } else {
        dev_info(bq->dev,"irq gpio not provided\n");
        return -EINVAL;
    }

    if (bq->irq) {
        ret = devm_request_threaded_irq(&bq->client->dev, bq->irq,
                NULL, bq25960_irq_handler,
                IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
                bq25960_irq_name[bq->mode], bq);
        if (ret < 0) {
            dev_err(bq->dev,"request irq for irq=%d failed, ret =%d\n",
                            bq->irq, ret);
            return ret;
        }
        enable_irq_wake(bq->irq);
    }

    return ret;
}

static enum power_supply_property bq25960_charger_props[] = {
    POWER_SUPPLY_PROP_ONLINE,
    POWER_SUPPLY_PROP_VOLTAGE_NOW,
    POWER_SUPPLY_PROP_CURRENT_NOW,
    POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT,
    POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE,
    POWER_SUPPLY_PROP_TEMP,
};

static int bq25960_charger_get_property(struct power_supply *psy,
                enum power_supply_property psp,
                union power_supply_propval *val)
{
    struct bq25960 *bq = power_supply_get_drvdata(psy);
    int ret;

    switch (psp) {
    case POWER_SUPPLY_PROP_ONLINE:
        bq25960_check_charge_enabled(bq, &bq->charge_enabled);
        val->intval = bq->charge_enabled;
        if(bq->otg_flag)
            val->intval = 0;
        break;
    case POWER_SUPPLY_PROP_VOLTAGE_NOW:
        ret = bq25960_get_adc_vbus(bq);
        if (ret < 0)
            return ret;

        val->intval = ret;
        break;
    case POWER_SUPPLY_PROP_CURRENT_NOW:
        ret = bq25960_get_adc_ibus(bq);
        if (ret < 0)
            return ret;

        val->intval = ret;
        break;
    case POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE:
        ret = bq25960_get_adc_vbat(bq);
        if (ret < 0)
            return ret;

        val->intval = ret;
        break;
    case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT:
        ret = bq25960_get_adc_ibat(bq);
        if (ret < 0)
            return ret;

        val->intval = ret;
        break;
    case POWER_SUPPLY_PROP_TEMP:
        ret = bq25960_get_adc_tbat(bq);
        if (ret < 0)
            return ret;

        val->intval = ret;
        break;
    default:
        return -EINVAL;
    }

    return 0;
}

static int bq25960_charger_set_property(struct power_supply *psy,
                    enum power_supply_property prop,
                    const union power_supply_propval *val)
{
    struct bq25960 *bq = power_supply_get_drvdata(psy);

    switch (prop) {
    case POWER_SUPPLY_PROP_ONLINE:
        bq25960_enable_charge(bq, val->intval);
        pr_info("POWER_SUPPLY_PROP_ONLINE: %s\n",val->intval ? "enable" : "disable");
        break;
    default:
        return -EINVAL;
    }

    return 0;
}

static int bq25960_charger_is_writeable(struct power_supply *psy,
                    enum power_supply_property prop)
{
    return 0;
}

static int bq25960_psy_register(struct bq25960 *bq)
{
    bq->psy_cfg.drv_data = bq;
    bq->psy_cfg.of_node = bq->dev->of_node;
    bq->psy_desc.name = bq25960_psy_name[bq->mode];
    bq->psy_desc.type = POWER_SUPPLY_TYPE_UNKNOWN;
    bq->psy_desc.properties = bq25960_charger_props;
    bq->psy_desc.num_properties = ARRAY_SIZE(bq25960_charger_props);
    bq->psy_desc.get_property = bq25960_charger_get_property;
    bq->psy_desc.set_property = bq25960_charger_set_property;
    bq->psy_desc.property_is_writeable = bq25960_charger_is_writeable;
    bq->psy = devm_power_supply_register(bq->dev, &bq->psy_desc, &bq->psy_cfg);
    if (IS_ERR(bq->psy)) {
        dev_info(bq->dev, "%s failed to register psy\n", __func__);
        return PTR_ERR(bq->psy);
    }
    dev_info(bq->dev, "%s power supply register successfully\n", bq->psy_desc.name);

    return 0;
}

static int bq25960_parse_dt(struct bq25960 *bq, struct device *dev)
{
    struct device_node *np = dev->of_node;
    int i;
    int ret;

    struct {
        char *name;
        int *conv_data;
    } props[] = {
        {"bq,bq25960,vbat-ovp-dis", &(bq->cfg.vbat_ovp_dis)},
        {"bq,bq25960,vbat-ovp", &(bq->cfg.vbat_ovp)},
        {"bq,bq25960,vbat-ovp-alm-dis", &(bq->cfg.vbat_ovp_alm_dis)},
        {"bq,bq25960,vbat-ovp-alm", &(bq->cfg.vbat_ovp_alm)},
        {"bq,bq25960,ibat-ocp-dis", &(bq->cfg.ibat_ocp_dis)},
        {"bq,bq25960,ibat-ocp", &(bq->cfg.ibat_ocp)},
        {"bq,bq25960,ibat-ocp-alm-dis", &(bq->cfg.ibat_ocp_alm_dis)},
        {"bq,bq25960,ibat-ocp-alm", &(bq->cfg.ibat_ocp_alm)},
        {"bq,bq25960,ibus-ucp-dis", &(bq->cfg.ibus_ucp_dis)},
        {"bq,bq25960,ibus-ucp", &(bq->cfg.ibus_ucp)},
        {"bq,bq25960,chg-config-2", &(bq->cfg.chg_config_2)},
        {"bq,bq25960,vbus-pd-en", &(bq->cfg.vbus_pd_en)},
        {"bq,bq25960,vbus-ovp", &(bq->cfg.vbus_ovp)},
        {"bq,bq25960,vbus-ovp-alm-dis", &(bq->cfg.vbus_ovp_alm_dis)},
        {"bq,bq25960,vbus-ovp-alm", &(bq->cfg.vbus_ovp_alm)},
        {"bq,bq25960,ibus-ocp", &(bq->cfg.ibus_ocp)},
        {"bq,bq25960,tsbus-flt-dis", &(bq->cfg.tsbus_flt_dis)},
        {"bq,bq25960,tsbat-flt-dis", &(bq->cfg.tsbat_flt_dis)},
        {"bq,bq25960,tdie-alm", &(bq->cfg.tdie_alm)},
        {"bq,bq25960,tsbus-flt", &(bq->cfg.tsbus_flt)},
        {"bq,bq25960,tsbat-flt", &(bq->cfg.tsbat_flt)},
        {"bq,bq25960,vac1-ovp", &(bq->cfg.vac1_ovp)},
        {"bq,bq25960,vac2-ovp", &(bq->cfg.vac2_ovp)},
        {"bq,bq25960,vac1-pd-en", &(bq->cfg.vac1_pd_en)},
        {"bq,bq25960,vac2-pd-en", &(bq->cfg.vac2_pd_en)},
        {"bq,bq25960,fsw-set", &(bq->cfg.fsw_set)},
        {"bq,bq25960,wd-timeout", &(bq->cfg.wd_timeout)},
        {"bq,bq25960,wd-timeout-dis", &(bq->cfg.wd_timeout_dis)},
        {"bq,bq25960,ibat-sns-r", &(bq->cfg.ibat_sns_r)},
        {"bq,bq25960,ss-timeout", &(bq->cfg.ss_timeout)},
        {"bq,bq25960,ibus-ucp-fall-dg", &(bq->cfg.ibus_ucp_fall_dg)},
        {"bq,bq25960,vout-ovp-dis", &(bq->cfg.vout_ovp_dis)},
        {"bq,bq25960,vout-ovp", &(bq->cfg.vout_ovp)},
        {"bq,bq25960,ms", &(bq->cfg.ms)},
    };

    /* initialize data for optional properties */
    for (i = 0; i < ARRAY_SIZE(props); i++) {
        ret = of_property_read_u32(np, props[i].name, props[i].conv_data);
        if (ret < 0) {
            dev_err(bq->dev, "can not read %s \n", props[i].name);
            return ret;
        }
    }

    bq->irq_gpio = of_get_named_gpio(np, "bq25960,intr_gpio", 0);
    if (!gpio_is_valid(bq->irq_gpio)) {
        dev_info(bq->dev,"fail to valid gpio : %d\n", bq->irq_gpio);
        return -EINVAL;
    }

    if (of_property_read_string(np, "charger_name", &bq->chg_dev_name) < 0) {
        bq->chg_dev_name = "charger";
        dev_info(bq->dev, "no charger name\n");
    }

    return 0;
}

static struct of_device_id bq25960_charger_match_table[] = {
    {   .compatible = "bq,bq25960-standalone",
        .data = &bq25960_mode_data[BQ25960], },
    {},
};

static int bq25960_iio_read_raw(struct iio_dev *indio_dev,
        struct iio_chan_spec const *chan, int *val1,
        int *val2, long mask)
{
    struct bq25960 *bq = iio_priv(indio_dev);
    int ret = 0;
    uint32_t status = 0;

    *val1 = 0;

    switch (chan->channel) {
    case PSY_IIO_SC_PRESENT:
        *val1 = bq->usb_present;
        break;

    case PSY_IIO_SC_CHARGING_ENABLED:
        bq25960_check_charge_enabled(bq, &bq->charge_enabled);
        *val1 = bq->charge_enabled;
        break;

    case PSY_IIO_SC_BUS_VOLTAGE:
        ret = bq25960_get_adc_vbus(bq);
        if (ret < 0)
            return ret;

        *val1 = ret;
        break;

    case PSY_IIO_SC_BUS_CURRENT:
        ret = bq25960_get_adc_ibus(bq);
        if (ret < 0)
            return ret;

        *val1 = ret;
        break;

    case PSY_IIO_SC_STATUS:
        ret = bq25960_get_status(bq, &status);
        if (ret < 0)
            return ret;

        *val1 = status;
        break;

    case PSY_IIO_SC_CHIP_VENDOR:
        *val1 = BQ25960_CHIP_VENDOR;
        break;

    default:
        pr_debug("Unsupported BQ25960 IIO chan %d\n", chan->channel);
        ret = -EINVAL;
        break;
    }

    if (ret < 0) {
        pr_info("Couldn't read IIO channel %d, ret = %d\n",
            chan->channel, ret);
        return ret;
    }

    return IIO_VAL_INT;
}

static int bq25960_iio_write_raw(struct iio_dev *indio_dev,
        struct iio_chan_spec const *chan, int val1,
        int val2, long mask)
{
    struct bq25960 *bq = iio_priv(indio_dev);
    int rc = 0;

    switch (chan->channel) {
    case PSY_IIO_SC_CHARGING_ENABLED:
        bq25960_enable_charge(bq, val1);
        bq25960_check_charge_enabled(bq, &bq->charge_enabled);
        break;

    case PSY_IIO_SC_ADC_CONTROL:
        bq25960_enable_adc(bq, val1);
        break;

    case PSY_IIO_SC_OTG_CONTROL:
        bq25960_set_otg(bq, val1);
        break;

    case PSY_IIO_SC_OVP_GATE_CONTROL:
        if (val1) {
            bq25960_set_otg_preconfigure(bq, true);
        } else {
            bq25960_set_otg(bq, false);
        }
        break;

    default:
        pr_err("Unsupported BQ25960 IIO chan %d\n", chan->channel);
        rc = -EINVAL;
        break;
    }

    if (rc < 0)
        pr_err("Couldn't write IIO channel %d, rc = %d\n",
            chan->channel, rc);

    return rc;
}

static int bq25960_iio_of_xlate(struct iio_dev *indio_dev,
    const struct fwnode_reference_args *iiospec)
{
    struct bq25960 *bq = iio_priv(indio_dev);
    struct iio_chan_spec *iio_chan = bq->iio_chan;
    int i;

    for (i = 0; i < ARRAY_SIZE(bq25960_iio_psy_channels);
                    i++, iio_chan++)
        if (iio_chan->channel == iiospec->args[0])
            return i;

    return -EINVAL;
}

static const struct iio_info bq25960_iio_info = {
    .read_raw = bq25960_iio_read_raw,
    .write_raw = bq25960_iio_write_raw,
    .fwnode_xlate = bq25960_iio_of_xlate,
};

static int bq25960_init_iio_psy(struct bq25960 *bq)
{
    struct iio_dev *indio_dev = bq->indio_dev;
    struct iio_chan_spec *chan;
    int num_iio_channels = ARRAY_SIZE(bq25960_iio_psy_channels);
    int rc, i;

    pr_err("bq25960_init_iio_psy start\n");

    bq->iio_chan = devm_kcalloc(bq->dev, num_iio_channels,
                sizeof(*bq->iio_chan), GFP_KERNEL);

    if (!bq->iio_chan)
        return -ENOMEM;

    bq->int_iio_chans = devm_kcalloc(bq->dev,
                num_iio_channels,
                sizeof(*bq->int_iio_chans),
                GFP_KERNEL);

    if (!bq->int_iio_chans)
        return -ENOMEM;

    indio_dev->info = &bq25960_iio_info;
    indio_dev->dev.parent = bq->dev;
    indio_dev->dev.of_node = bq->dev->of_node;
    indio_dev->modes = INDIO_DIRECT_MODE;
    indio_dev->channels = bq->iio_chan;
    indio_dev->num_channels = num_iio_channels;

    if (bq->mode == BQ25960) {
        indio_dev->name = "bq25960-standalone";
        for (i = 0; i < num_iio_channels; i++) {
            bq->int_iio_chans[i].indio_dev = indio_dev;
            chan = &bq->iio_chan[i];
            bq->int_iio_chans[i].channel = chan;
            chan->address = i;
            chan->channel = bq25960_iio_psy_channels[i].channel_num;
            chan->type = bq25960_iio_psy_channels[i].type;
            chan->datasheet_name = bq25960_iio_psy_channels[i].datasheet_name;
            chan->extend_name = bq25960_iio_psy_channels[i].datasheet_name;
            chan->info_mask_separate = bq25960_iio_psy_channels[i].info_mask;
        }
    }

    rc = devm_iio_device_register(bq->dev, indio_dev);
    if (rc){
        pr_err("Failed to register BQ25960 IIO device, rc=%d\n", rc);
    }
    pr_err("bq25960 iio device init end, rc=%d\n", rc);

    return rc;
}

static int bq25960_charger_probe(struct i2c_client *client,
                    const struct i2c_device_id *id)
{
    struct bq25960 *bq;
    const struct of_device_id *match;
    struct device_node *node = client->dev.of_node;
    int ret = 0;
    int i;
    struct iio_dev *indio_dev;

    dev_err(&client->dev, "%s (%s)\n", __func__, BQ25960_DRV_VERSION);

    indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*bq));

    bq = iio_priv(indio_dev);
    bq->indio_dev = indio_dev;
    bq->dev = &client->dev;
    bq->client = client;

    bq->regmap = devm_regmap_init_i2c(client,
                            &bq25960_regmap_config);
    if (IS_ERR(bq->regmap)) {
        dev_err(bq->dev, "Failed to initialize regmap\n");
        ret = PTR_ERR(bq->regmap);
        goto err_regmap_init;
    }

    for (i = 0; i < ARRAY_SIZE(bq25960_reg_fields); i++) {
        const struct reg_field *reg_fields = bq25960_reg_fields;
        bq->rmap_fields[i] =
            devm_regmap_field_alloc(bq->dev,
                        bq->regmap,
                        reg_fields[i]);
        if (IS_ERR(bq->rmap_fields[i])) {
            dev_err(bq->dev, "cannot allocate regmap field\n");
            ret = PTR_ERR(bq->rmap_fields[i]);
            goto err_regmap_field;
        }
    }

    i2c_set_clientdata(client, bq);

    ret = bq25960_detect_device(bq);
    if (ret < 0) {
        dev_err(bq->dev, "%s detect device fail\n", __func__);
        ret = 0;
        goto err_detect_dev;
    }

    bq25960_create_device_node(&(client->dev));

    match = of_match_node(bq25960_charger_match_table, node);
    if (match == NULL) {
        dev_err(bq->dev, "device tree match not found!\n");
        ret = -ENODEV;
        goto err_match_node;
    }

    bq->mode = *(int *)match->data;

    ret = bq25960_parse_dt(bq, &client->dev);
    if (ret < 0) {
        dev_err(bq->dev, "%s parse dt failed(%d)\n", __func__, ret);
        ret = -EIO;
        goto err_parse_dt;
    }

    ret = bq25960_init_device(bq);
    if (ret < 0) {
        dev_err(bq->dev, "%s init device failed(%d)\n", __func__, ret);
        goto err_init_device;
    }

    ret = bq25960_psy_register(bq);
    if (ret < 0) {
        dev_err(bq->dev, "%s psy register failed(%d)\n", __func__, ret);
        goto err_register_psy;
    }

    ret = bq25960_register_interrupt(bq);
    if (ret < 0) {
        dev_err(bq->dev, "%s register irq fail(%d)\n",
                        __func__, ret);
        goto err_register_irq;
    }

    ret = bq25960_init_iio_psy(bq);
    if (ret)
        goto err_free;

    mutex_init(&g_i2c_lock);

    ret = bq25960_adjust_ovp(bq);
    if (ret < 0) {
        pr_err("error to adjust the ovp \n");
        goto err_adjust_ovp;
    }

    dev_err(&client->dev,"bq25960 probe success\n");

    return 0;

err_adjust_ovp:
err_free:
    mutex_destroy(&g_i2c_lock);
err_register_irq:
err_register_psy:
err_init_device:
    power_supply_unregister(bq->psy);
err_parse_dt:
err_match_node:
err_detect_dev:
err_regmap_field:
err_regmap_init:
    dev_err(&client->dev,"bq25960 probe fail\n");
    return ret;
}

static void bq25960_charger_remove(struct i2c_client *client)
{
    struct bq25960 *bq = i2c_get_clientdata(client);

    power_supply_unregister(bq->psy);
    mutex_destroy(&g_i2c_lock);
    devm_kfree(&client->dev, bq);
}

static void bq25960_charger_shutdown(struct i2c_client *client)
{
    struct bq25960 *bq = i2c_get_clientdata(client);

    bq25960_enable_adc(bq, false);
    mutex_destroy(&g_i2c_lock);
}

#ifdef CONFIG_PM_SLEEP
static int bq25960_suspend(struct device *dev)
{
    struct bq25960 *bq = dev_get_drvdata(dev);

    dev_info(bq->dev, "Suspend successfully!");
    if (device_may_wakeup(dev))
        enable_irq_wake(bq->irq);
    disable_irq(bq->irq);

    return 0;
}

static int bq25960_resume(struct device *dev)
{
    struct bq25960 *bq = dev_get_drvdata(dev);

    dev_info(bq->dev, "Resume successfully!");
    if (device_may_wakeup(dev))
        disable_irq_wake(bq->irq);
    enable_irq(bq->irq);

    return 0;
}

static const struct dev_pm_ops bq25960_pm = {
    SET_SYSTEM_SLEEP_PM_OPS(bq25960_suspend, bq25960_resume)
};
#endif

static struct i2c_driver bq25960_charger_driver = {
    .driver     = {
        .name   = "bq25960",
        .owner  = THIS_MODULE,
        .of_match_table = bq25960_charger_match_table,
#ifdef CONFIG_PM_SLEEP
        .pm = &bq25960_pm,
#endif
    },
    .probe      = bq25960_charger_probe,
    .remove     = bq25960_charger_remove,
    .shutdown   = bq25960_charger_shutdown,
};

module_i2c_driver(bq25960_charger_driver);
MODULE_DESCRIPTION("TI BQ25960 Driver");
MODULE_LICENSE("GPL v2");
