// SPDX-License-Identifier: GPL-2.0
/*
 *sc8510.c
 *
 * sc8510 driver
 *
 * Copyright (c) 2024-2024 Xiaomi Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
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
#include <linux/version.h>
#include <linux/regmap.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <mca/common/mca_log.h>
#include <mca/platform/platform_buckchg_class.h>

#ifndef MCA_LOG_TAG
#define MCA_LOG_TAG "cp_sc8510"
#endif

#define I2C_RETRY_CNT 5

#define SC851X_DRV_VERSION              "1.0.0_G"

#define SC851X_REGMAX                   0x15

enum sc851x_fields {
    V2X_OVP_DIS, V2X_OVP, V2X_UVLO, V2X_S2F, UVLO_DEG,
    V1X_OVP_DIS, V1X_OVP, V1X_SCP_DIS, V1X_SCP_F,
    VAC_OVP_DIS, VAC_OVP, SHIP_VAC, SHIP_VAC_PD,
    RVS_OCP, FWD_OCP,
    WD_TIMEOUT, LNC_SS_TIMEOUT_DIS, LNS_SS_TIMEOUT, SS_TIMEOUT_DIS,
    SHIP_EN, SHIP_RST, T_WAIT,
    ACTDIS_PD_EN, SHIP_WT, HW_RST, VAC_PD_EN, REG_RST, AUDIO_EN, AUDIO_FREQ, AUDIO_INTO_DG,
    FIX_FREQ_EN, FREQ_DITH, FREQ,
    REF_SKIP_R, SKIP_HYST,
    T_VAC_OVP_DG, T_V2X_OVP_DG, T_V1X_OVP_DG, T_FWD_OVP_DG, T_RVS_OCP_DG, T_V2X2VAD_LNC_MAX_DG, T3_SET,
    FWD_OCP_DIS, RVS_OCP_DIS, TSHUT_DIS, TDIE_100_ALM_DIS, TDIE_120_ALM_DIS, V1X_ISS_OPP_DIS,
    DEVICE_ID,
    FAM_EN,
    F_MAX_FIELDS,
};

//REGISTER
static const struct reg_field sc851x_reg_fields[] = {
    /*reg00*/
    [V2X_OVP_DIS] = REG_FIELD(0x00, 7, 7),
    [V2X_OVP] = REG_FIELD(0x00, 5, 6),
    [V2X_UVLO] = REG_FIELD(0x00, 4, 4),
    [V2X_S2F] = REG_FIELD(0x00, 2, 3),
    [UVLO_DEG] = REG_FIELD(0x00, 0, 1),
    /*reg01*/
    [V1X_OVP_DIS] = REG_FIELD(0x01, 7, 7),
    [V1X_OVP] = REG_FIELD(0x01, 2, 6),
    [V1X_SCP_DIS] = REG_FIELD(0x01, 1, 1),
    [V1X_SCP_F] = REG_FIELD(0x01, 0, 0),
    /*reg02*/
    [VAC_OVP_DIS] = REG_FIELD(0x02, 7, 7),
    [VAC_OVP] = REG_FIELD(0x02, 3, 6),
    [SHIP_VAC] = REG_FIELD(0x02, 1, 1),
    [SHIP_VAC_PD] = REG_FIELD(0x02, 0, 0),
    /*reg03*/
    [RVS_OCP] = REG_FIELD(0x03, 4, 7),
    [FWD_OCP] = REG_FIELD(0x03, 0, 3),
    /*reg04*/
    [WD_TIMEOUT] = REG_FIELD(0x04, 4, 7),
    [LNC_SS_TIMEOUT_DIS] = REG_FIELD(0x04, 3, 3),
    [LNS_SS_TIMEOUT] = REG_FIELD(0x04, 1, 2),
    [SS_TIMEOUT_DIS] = REG_FIELD(0x04, 0, 0),
    /*reg05*/
    [SHIP_EN] = REG_FIELD(0x05, 5, 7),
    [SHIP_RST] = REG_FIELD(0x05, 1, 1),
    [T_WAIT] = REG_FIELD(0x05, 0, 0),
    /*reg06*/
    [ACTDIS_PD_EN] = REG_FIELD(0x06, 7, 7),
    [SHIP_WT] = REG_FIELD(0x06, 6, 6),
    [HW_RST] = REG_FIELD(0x06, 5, 5),
    [VAC_PD_EN] = REG_FIELD(0x06, 4, 4),
    [REG_RST] = REG_FIELD(0x06, 3, 3),
    [AUDIO_EN] = REG_FIELD(0x06, 2, 2),
    [AUDIO_FREQ] = REG_FIELD(0x06, 1, 1),
    [AUDIO_INTO_DG] = REG_FIELD(0x06, 0, 0),
    /*reg07*/
    [FIX_FREQ_EN] = REG_FIELD(0x07, 6, 6),
    [FREQ_DITH] = REG_FIELD(0x07, 4, 5),
    [FREQ] = REG_FIELD(0x07, 0, 3),
    /*reg08*/
    [REF_SKIP_R] = REG_FIELD(0x08, 5, 7),
    [SKIP_HYST] = REG_FIELD(0x08, 1, 3),
    /*reg12*/
    [T_VAC_OVP_DG] = REG_FIELD(0x12, 7, 7),
    [T_V2X_OVP_DG] = REG_FIELD(0x12, 6, 6),
    [T_V1X_OVP_DG] = REG_FIELD(0x12, 5, 5),
    [T_FWD_OVP_DG] = REG_FIELD(0x12, 4, 4),
    [T_RVS_OCP_DG] = REG_FIELD(0x12, 3, 3),
    [T_V2X2VAD_LNC_MAX_DG] = REG_FIELD(0x12, 2, 2),
    [T3_SET] = REG_FIELD(0x12, 0, 0),
    /*reg13*/
    [FWD_OCP_DIS] = REG_FIELD(0x13, 7, 7),
    [RVS_OCP_DIS] = REG_FIELD(0x13, 6, 6),
    [TSHUT_DIS] = REG_FIELD(0x13, 5, 5),
    [TDIE_100_ALM_DIS] = REG_FIELD(0x13, 4, 4),
    [TDIE_120_ALM_DIS] = REG_FIELD(0x13, 3, 3),
    [V1X_ISS_OPP_DIS] = REG_FIELD(0x13, 1, 1),
    /*reg14*/
    [DEVICE_ID] = REG_FIELD(0x14, 0, 4),
    /*reg15*/
    [FAM_EN] = REG_FIELD(0x15, 0, 1),
};

static const struct regmap_config sc851x_regmap_config = {
    .reg_bits = 8,
    .val_bits = 8,

    .max_register = SC851X_REGMAX,
};

struct sc851x_cfg_e {
    int v2x_ovp_dis;
    int v2x_ovp;
    int v2x_uvlo;
    int v2x_s2f;
    int uvlo_deg;
    int v1x_ovp_dis;
    int v1x_ovp;
    int v1x_scp_dis;
    int v1x_scp_f;
    int vac_ovp_dis;
    int vac_ovp;
    int rvs_ocp;
    int fwd_ocp;
    int wdt;
    int lnc_ss_timeout_dis;
    int lnc_ss_timeout;
    int ss_timeout_dis;
    int audio_en;
    int audio_freq;
    int audio_into_dg;
    int freq;
    int ref_skip_r;
    int skip_hyst;
    int t_vac_ovp_dg;
    int t_v2x_ovp_dg;
    int t_v1x_ovp_dg;
    int t_fwd_ocp_dg;
    int t_rvs_ocp_dg;
    int t_c2x2vad_lnc_max_dg;
    int t_t3_set;
    int fwd_ocp_dis;
    int rvs_ocp_dis;
    int tshut_dis;
    int v1x_oss_opp_dis;
    int fam_en;
};

struct sc851x_chip {
    struct device *dev;
    struct i2c_client *client;
    struct regmap *regmap;
    struct regmap_field *rmap_fields[F_MAX_FIELDS];
    struct notifier_block reboot_nb;

    struct sc851x_cfg_e cfg;
    int irq_gpio;
    int irq;
    int ship_mode_en;
};

/*********************************************************/
__maybe_unused static int sc851x_field_read(struct sc851x_chip *sc,
                enum sc851x_fields field_id, int *val)
{
    int ret;
    int retry_cnt = 0;

    while(retry_cnt < I2C_RETRY_CNT){
        ret = regmap_field_read(sc->rmap_fields[field_id], val);
        if (ret < 0) {
            mca_log_err("sc851x read field %d fail: %d, cnt:%d\n", field_id, ret, retry_cnt);
            retry_cnt++;
        } else {
            return ret;
        }
    }

    mca_log_err("sc851x read field %d fail: %d, cnt max:%d\n", field_id, ret, I2C_RETRY_CNT);

    return ret;
}

static int sc851x_field_write(struct sc851x_chip *sc,
                enum sc851x_fields field_id, int val)
{
    int ret;

    ret = regmap_field_write(sc->rmap_fields[field_id], val);
    if (ret < 0) {
        mca_log_err("sc851x read field %d fail: %d\n", field_id, ret);
    }

    return ret;
}


static int sc851x_reg_reset(struct sc851x_chip *sc)
{
    return sc851x_field_write(sc, REG_RST, 1);
}

__maybe_unused static int sc851x_dump_reg(struct sc851x_chip *sc)
{
    int ret;
    int i;
    int val;

    for (i = 0; i <= SC851X_REGMAX; i++) {
        ret = regmap_read(sc->regmap, i, &val);
        mca_log_err("reg[0x%02x] = 0x%02x\n",
                i, val);
    }

    return ret;
}

static int sc851x_reboot_callback(struct notifier_block *nb, unsigned long code,
		void *unused)
{
	struct sc851x_chip *sc = container_of(nb, struct sc851x_chip, reboot_nb);
	int ret = 0;

	if (!sc->ship_mode_en)
		return NOTIFY_DONE;

	if (code == SYS_POWER_OFF || code == SYS_RESTART) {
		mca_log_info("set sc8510 shipmode\n");

        /* allow writing to shipping mode register */
        ret |= sc851x_field_write(sc, SHIP_WT, 1);
        /* Ignores VAC signal when entering shipping mode */
        ret |= sc851x_field_write(sc, SHIP_VAC, 1);
        /* Pull down VAC pin in during shipping mode */
        ret |= sc851x_field_write(sc, SHIP_VAC_PD, 1);
        /* set 0b111 to SHIP_EN[3bits] to enable shipping mode */
        ret |= sc851x_field_write(sc, SHIP_EN, 0x7);
		if (ret < 0)
			mca_log_err("Failed to write ship mode: %d\n", ret);
	}

	return NOTIFY_DONE;
}

static int sc8510_set_ship_mode_flag(void *data, bool en)
{
	struct sc851x_chip *sc = (struct sc851x_chip *)data;

	sc->ship_mode_en = en;

	mca_log_info("ship mode flag = %d\n", en);

	return 0;
}

static int sc8510_get_ship_mode_flag(void *data, bool *en)
{
	struct sc851x_chip *sc = (struct sc851x_chip *)data;

	*en = sc->ship_mode_en;

	return 0;
}

static struct platform_class_buckchg_ops g_aux_subpmic_ops = {
    .set_ship_mode = sc8510_set_ship_mode_flag,
    .get_ship_mode = sc8510_get_ship_mode_flag,
};

static int sc851x_init_device(struct sc851x_chip *sc)
{
    int ret = 0;
    int i;
    struct {
        enum sc851x_fields field_id;
        int conv_data;
    } props[] = {
        {V2X_OVP_DIS, sc->cfg.v2x_ovp_dis},
        {V2X_OVP, sc->cfg.v2x_ovp},
        {V2X_UVLO, sc->cfg.v2x_uvlo},
        {V2X_S2F, sc->cfg.v2x_s2f},
        {UVLO_DEG, sc->cfg.uvlo_deg},
        {V1X_OVP_DIS, sc->cfg.v1x_ovp_dis},
        {V1X_OVP, sc->cfg.v1x_ovp},
        {V1X_SCP_DIS, sc->cfg.v1x_scp_dis},
        {V1X_SCP_F, sc->cfg.v1x_scp_f},
        {VAC_OVP_DIS, sc->cfg.vac_ovp_dis},
        {VAC_OVP, sc->cfg.vac_ovp},
        {RVS_OCP, sc->cfg.rvs_ocp},
        {FWD_OCP, sc->cfg.fwd_ocp},
        {WD_TIMEOUT, sc->cfg.wdt},
        {LNC_SS_TIMEOUT_DIS, sc->cfg.lnc_ss_timeout_dis},
        {LNS_SS_TIMEOUT, sc->cfg.lnc_ss_timeout},
        {SS_TIMEOUT_DIS, sc->cfg.ss_timeout_dis},
        {AUDIO_EN, sc->cfg.audio_en},
        {AUDIO_FREQ, sc->cfg.audio_freq},
        {AUDIO_INTO_DG, sc->cfg.audio_into_dg},
        {FREQ, sc->cfg.freq},
        {REF_SKIP_R, sc->cfg.ref_skip_r},
        {SKIP_HYST, sc->cfg.skip_hyst},
        {T_VAC_OVP_DG, sc->cfg.t_vac_ovp_dg},
        {T_V2X_OVP_DG, sc->cfg.t_v2x_ovp_dg},
        {T_V1X_OVP_DG, sc->cfg.t_v1x_ovp_dg},
        {T_FWD_OVP_DG, sc->cfg.t_fwd_ocp_dg},
        {T_RVS_OCP_DG, sc->cfg.t_rvs_ocp_dg},
        {T_V2X2VAD_LNC_MAX_DG, sc->cfg.t_c2x2vad_lnc_max_dg},
        {T3_SET, sc->cfg.t_t3_set},
        {FWD_OCP_DIS, sc->cfg.fwd_ocp_dis},
        {RVS_OCP_DIS, sc->cfg.rvs_ocp_dis},
        {TSHUT_DIS, sc->cfg.tshut_dis},
        {V1X_ISS_OPP_DIS, sc->cfg.v1x_oss_opp_dis},
        {FAM_EN, sc->cfg.fam_en},
    };

    ret = sc851x_reg_reset(sc);
    if (ret < 0) {
        mca_log_err("Failed to reset registers(%d)\n", ret);
    }
    msleep(10);

    for (i = 0; i < ARRAY_SIZE(props); i++) {
        ret = sc851x_field_write(sc, props[i].field_id, props[i].conv_data);
    }

    // return sc851x_dump_reg(sc);

    return ret;
};


static int sc851x_parse_dt(struct sc851x_chip *sc, struct device *dev)
{
    struct device_node *np = dev->of_node;
    int i;
    int ret;
    struct {
        char *name;
        int *conv_data;
    } props[] = {
        {"sc,sc851x,v2x-ovp-dis", &(sc->cfg.v2x_ovp_dis)},
        {"sc,sc851x,v2x-ovp", &(sc->cfg.v2x_ovp)},
        {"sc,sc851x,v2x-uvlo", &(sc->cfg.v2x_uvlo)},
        {"sc,sc851x,v2x-s2f", &(sc->cfg.v2x_s2f)},
        {"sc,sc851x,uvlo-deg", &(sc->cfg.uvlo_deg)},
        {"sc,sc851x,v1x-ovp-dis", &(sc->cfg.v1x_ovp_dis)},
        {"sc,sc851x,v1x-ovp", &(sc->cfg.v1x_ovp)},
        {"sc,sc851x,v1x-scp-dis", &(sc->cfg.v1x_scp_dis)},
        {"sc,sc851x,v1x-scp-f", &(sc->cfg.v1x_scp_f)},
        {"sc,sc851x,vac-ovp-dis", &(sc->cfg.vac_ovp_dis)},
        {"sc,sc851x,vac-ovp", &(sc->cfg.vac_ovp)},
        {"sc,sc851x,rvs-ocp", &(sc->cfg.rvs_ocp)},
        {"sc,sc851x,fwd-ocp", &(sc->cfg.fwd_ocp)},
        {"sc,sc851x,wdt", &(sc->cfg.wdt)},
        {"sc,sc851x,lnc-ss-timeout-dis", &(sc->cfg.lnc_ss_timeout_dis)},
        {"sc,sc851x,lnc-ss-timeout", &(sc->cfg.lnc_ss_timeout)},
        {"sc,sc851x,ss-timeout-dis", &(sc->cfg.ss_timeout_dis)},
        {"sc,sc851x,audio-en", &(sc->cfg.audio_en)},
        {"sc,sc851x,audio-freq", &(sc->cfg.audio_freq)},
        {"sc,sc851x,audio-into-dg", &(sc->cfg.audio_into_dg)},
        {"sc,sc851x,freq", &(sc->cfg.freq)},
        {"sc,sc851x,ref-skip-r", &(sc->cfg.ref_skip_r)},
        {"sc,sc851x,skip-hyst", &(sc->cfg.skip_hyst)},
        {"sc,sc851x,t-vac-ovp-dg", &(sc->cfg.t_vac_ovp_dg)},
        {"sc,sc851x,t-v2x-ovp-dg", &(sc->cfg.t_v2x_ovp_dg)},
        {"sc,sc851x,t-v1x-ovp-dg", &(sc->cfg.t_v1x_ovp_dg)},
        {"sc,sc851x,t-fwd-ocp-dg", &(sc->cfg.t_fwd_ocp_dg)},
        {"sc,sc851x,t-rvs-ocp-dg", &(sc->cfg.t_rvs_ocp_dg)},
        {"sc,sc851x,t-v2x2vad-lnc-max-dg", &(sc->cfg.t_c2x2vad_lnc_max_dg)},
        {"sc,sc851x,t-t3-set", &(sc->cfg.t_t3_set)},
        {"sc,sc851x,fwd-ocp-dis", &(sc->cfg.fwd_ocp_dis)},
        {"sc,sc851x,rvs-ocp-dis", &(sc->cfg.rvs_ocp_dis)},
        {"sc,sc851x,tshut-dis", &(sc->cfg.tshut_dis)},
        {"sc,sc851x,v1x-oss-opp-dis", &(sc->cfg.v1x_oss_opp_dis)},
        {"sc,sc851x,fam-en", &(sc->cfg.fam_en)},
    };

    /* initialize data for optional properties */
    for (i = 0; i < ARRAY_SIZE(props); i++) {
        ret = of_property_read_u32(np, props[i].name,
                        props[i].conv_data);
        if (ret < 0) {
            mca_log_err("can not read %s \n", props[i].name);
            return ret;
        }
    }

    sc->irq_gpio = of_get_named_gpio(np, "sc,sc851x,irq-gpio", 0);
    if (!gpio_is_valid(sc->irq_gpio)) {
        mca_log_err("fail to valid gpio : %d\n", sc->irq_gpio);
        return -EINVAL;
    }

    return 0;
}

static ssize_t sc851x_show_registers(struct device *dev,
                struct device_attribute *attr, char *buf)
{
    struct sc851x_chip *sc = dev_get_drvdata(dev);
    u8 addr;
    int val;
    u8 tmpbuf[300];
    int len;
    int idx = 0;
    int ret;

    idx = snprintf(buf, PAGE_SIZE, "%s:\n", "sc851x");
    for (addr = 0x0; addr <= SC851X_REGMAX; addr++) {
        ret = regmap_read(sc->regmap, addr, &val);
        if (ret == 0) {
            len = snprintf(tmpbuf, PAGE_SIZE - idx,
                    "Reg[%.2X] = 0x%.2x\n", addr, val);
            memcpy(&buf[idx], tmpbuf, len);
            idx += len;
        } else if (ret < 0) {
            mca_log_err("fail to read reg[%.2X]\n", addr);
        }
    }

    return idx;
}

static ssize_t sc851x_store_register(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
    struct sc851x_chip *sc = dev_get_drvdata(dev);
    int ret;
    unsigned int reg;
    unsigned int val;

    ret = sscanf(buf, "%x %x", &reg, &val);
    if (ret == 2 && reg <= SC851X_REGMAX) {
        regmap_write(sc->regmap, reg, val);
    } else if (ret < 0) {
        mca_log_err("fail to write %d to  reg[%.2X]\n", val, reg);
    }


    return count;
}

static DEVICE_ATTR(registers, 0660, sc851x_show_registers, sc851x_store_register);

static int sc851x_create_device_node(struct device *dev)
{
    int ret = 0;
    ret = device_create_file(dev, &dev_attr_registers);
    if (ret < 0) {
        mca_log_err("fail to create device node\n");
        return -1;
    }

    return 0;
}

static void sc851x_check_fault_status(struct sc851x_chip *sc)
{
    int ret;
    int flag = 0;

    ret = regmap_read(sc->regmap, 0x0C, &flag);
    if (!ret)
        mca_log_err("FLAG1 reg[0x0C] = 0x%02X\n", flag);

    ret = regmap_read(sc->regmap, 0x0D, &flag);
    if (!ret)
        mca_log_err("FLAG2 reg[0x0D] = 0x%02X\n", flag);

    ret = regmap_read(sc->regmap, 0x0E, &flag);
    if (!ret)
        mca_log_err("FLAG3 reg[0x0E] = 0x%02X\n", flag);

}


static irqreturn_t sc851x_irq_handler(int irq, void *data)
{
    struct sc851x_chip *sc = data;

    mca_log_err("INT OCCURED\n");

    sc851x_check_fault_status(sc);

    return IRQ_HANDLED;
}

static int sc851x_register_interrupt(struct sc851x_chip *sc)
{
    int ret;

    if (gpio_is_valid(sc->irq_gpio)) {
        ret = gpio_request_one(sc->irq_gpio, GPIOF_DIR_IN,"sc851x_irq");
        if (ret) {
            mca_log_err("failed to request sc851x_irq\n");
            return -EINVAL;
        }
        sc->irq = gpio_to_irq(sc->irq_gpio);
        if (sc->irq < 0) {
            mca_log_err("failed to gpio_to_irq\n");
            return -EINVAL;
        }
    } else {
        mca_log_err("irq gpio not provided\n");
        return -EINVAL;
    }

    if (sc->irq) {
        ret = devm_request_threaded_irq(&sc->client->dev, sc->irq,
                NULL, sc851x_irq_handler,
                IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
                "sc851x-irq", sc);

        if (ret < 0) {
            mca_log_err("request irq for irq=%d failed, ret =%d\n",
                            sc->irq, ret);
            return ret;
        }
        enable_irq_wake(sc->irq);
    }

    return ret;
}

static struct of_device_id sc851x_charger_match_table[] = {
    {   .compatible = "sc,sc851x",},
    {   .compatible = "sc,sc8510",},
    {   .compatible = "sc,sc8517",},
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 9))
static int sc851x_probe(struct i2c_client *client)
#else
static int sc851x_probe(struct i2c_client *client, const struct i2c_device_id *id)
#endif
{
    struct sc851x_chip *sc;
    int ret, i;

    mca_log_err("(%s)\n", SC851X_DRV_VERSION);

    sc = devm_kzalloc(&client->dev, sizeof(struct sc851x_chip), GFP_KERNEL);
    if (!sc) {
        ret = -ENOMEM;
        goto err_kzalloc;
    }

    sc->dev = &client->dev;
    sc->client = client;

    sc->regmap = devm_regmap_init_i2c(client,
                            &sc851x_regmap_config);
    if (IS_ERR(sc->regmap)) {
        mca_log_err("Failed to initialize regmap\n");
        ret = PTR_ERR(sc->regmap);
        goto err_regmap_init;
    }

    for (i = 0; i < ARRAY_SIZE(sc851x_reg_fields); i++) {
        const struct reg_field *reg_fields = sc851x_reg_fields;

        sc->rmap_fields[i] =
            devm_regmap_field_alloc(sc->dev,
                        sc->regmap,
                        reg_fields[i]);
        if (IS_ERR(sc->rmap_fields[i])) {
            mca_log_err("cannot allocate regmap field\n");
            ret = PTR_ERR(sc->rmap_fields[i]);
            goto err_regmap_field;
        }
    }

    i2c_set_clientdata(client, sc);
    ret = sc851x_create_device_node(&(client->dev));
    if(ret < 0) {
        mca_log_err("create device node failed\n");
    }

    ret = sc851x_parse_dt(sc, &client->dev);
    if (ret < 0) {
        mca_log_err("parse dt failed(%d)\n", ret);
        goto err_parse_dt;
    }

    ret = sc851x_init_device(sc);
    if (ret < 0) {
        mca_log_err("init device failed(%d)\n", ret);
        goto err_init_device;
    }

    ret = platform_class_buckchg_ops_register(AUX_BUCK_CHARGER,
        sc, &g_aux_subpmic_ops);
    if (ret) {
        mca_log_err("%s register buckchg ops fail\n", __func__);
        goto err_register_ops;
    }

    sc->reboot_nb.notifier_call = sc851x_reboot_callback;
    sc->reboot_nb.priority = 255;
    ret = register_reboot_notifier(&sc->reboot_nb);
    if(ret) {
        mca_log_err("register reboot notifier fail\n");
        goto err_register_nb;
    }

    ret = sc851x_register_interrupt(sc);
    if (ret < 0) {
        mca_log_err("register irq fail(%d)\n", ret);
        goto err_register_irq;
    }
    mca_log_err("sc851x probe successfully!\n");
    return 0;

err_register_ops:
err_register_nb:
err_register_irq:
err_init_device:
err_parse_dt:
err_regmap_init:
err_regmap_field:
    devm_kfree(&client->dev, sc);
err_kzalloc:
    mca_log_err("sc851x probe fail\n");
    return ret;
}

static void sc851x_remove(struct i2c_client *client)
{
    struct sc851x_chip *sc = i2c_get_clientdata(client);

    devm_kfree(&client->dev, sc);
}

static void sc851x_shutdown(struct i2c_client *client)
{
    struct sc851x_chip *sc = i2c_get_clientdata(client);
    int ret = 0;

    ret |= sc851x_field_write(sc, AUDIO_EN, FALSE);
    if (ret)
        mca_log_err("unable to disable AUDIO_EN\n");

    mca_log_info("sc8510 shutdown!\n");
}

#ifdef CONFIG_PM_SLEEP
static int sc851x_suspend(struct device *dev)
{
    struct sc851x_chip *sc = dev_get_drvdata(dev);

    mca_log_info("Suspend successfully!");
    if (device_may_wakeup(dev))
        enable_irq_wake(sc->irq);
    disable_irq(sc->irq);

    return 0;
}
static int sc851x_resume(struct device *dev)
{
    struct sc851x_chip *sc = dev_get_drvdata(dev);

    mca_log_info("Resume successfully!");
    if (device_may_wakeup(dev))
        disable_irq_wake(sc->irq);
    enable_irq(sc->irq);

    return 0;
}

static const struct dev_pm_ops sc851x_pm = {
    SET_SYSTEM_SLEEP_PM_OPS(sc851x_suspend, sc851x_resume)
};
#endif

static struct i2c_driver sc851x_charger_driver = {
    .driver     = {
        .name   = "sc851x",
        .owner  = THIS_MODULE,
        .of_match_table = sc851x_charger_match_table,
#ifdef CONFIG_PM_SLEEP
        .pm = &sc851x_pm,
#endif
    },
    .probe      = sc851x_probe,
    .remove     = sc851x_remove,
    .shutdown   = sc851x_shutdown,
};

module_i2c_driver(sc851x_charger_driver);

MODULE_DESCRIPTION("SC SC851X Driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("South Chip <tianye9@xiaomi.com>");
