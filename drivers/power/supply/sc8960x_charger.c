
// SPDX-License-Identifier: GPL-2.0
/*
* Copyright (c) 2022 Southchip Semiconductor Technology(Shanghai) Co., Ltd.
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
#include <linux/extcon-provider.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/machine.h>
#include <linux/debugfs.h>
#include <linux/bitops.h>
#include <linux/math64.h>
#include <linux/regmap.h>
#include <linux/iio/consumer.h>
#include <linux/reboot.h>
#include <linux/reboot-mode.h>

#if IS_ENABLED(CONFIG_MTK_CHARGER)
#include "charger_class.h"
#include "mtk_charger.h"
#endif /*CONFIG_MTK_CLASS*/

//extern void Charger_Detect_Init(void);
//extern void Charger_Detect_Release(void);

#define SC8960X_DRV_VERSION         "1.0.0_G"

#define SC8960X_REG_7F              0x7F
#define REG7F_KEY1                  0x5A
#define REG7F_KEY2                  0x68
#define REG7F_KEY3                  0x65
#define REG7F_KEY4                  0x6E
#define REG7F_KEY5                  0x67
#define REG7F_KEY6                  0x4C
#define REG7F_KEY7                  0x69
#define REG7F_KEY8                  0x6E

#define SC8960X_REG_80              0x80
#define SC8960X_REG_81              0x81
#define SC8960X_REG_90              0x90
#define SC8960X_REG_93              0x93
#define REG93_VAL                   0x2A
#define REG81_HVDCP_EN_MASK         0x02
#define REG81_HVDCP_EN_SHIFT        1
#define REG81_HVDCP_ENABLE          1
#define REG81_HVDCP_DISABLE         0

#define SC8960X_REG_08              0x08
#define SC8960X_REG_0A              0x0A
#define SC8960X_REG_0E              0x0E
#define SC8960X_REG_92              0x92
#define REG92_PFM_VAL               0x71

#define SC8960X_REGMAX              0x0E

#define DEFAULT_BC12_TIME_EXPIRE    900
#define DEFAULT_HIZ_CUT_TIME_EXPRIE 300UL
#define VINDPM_BEFORCE_BC12         0
#define VIMDPM_DEFAULT_IN_BC12      5000

enum sc8960x_part_no {
    SC89619D_ID = 0x02,
    SC89601D_ID = 0x03,
};

enum sc8960x_vbus_stat {
    VBUS_STAT_NO_INPUT = 0,
    VBUS_STAT_SDP,
    VBUS_STAT_CDP,
    VBUS_STAT_DCP,
    VBUS_STAT_HVDCP,
    VBUS_STAT_UNKOWN,
    VBUS_STAT_NONSTAND,
    VBUS_STAT_OTG,
};

enum sc8960x_chg_stat {
    CHG_STAT_NOT_CHARGE = 0,
    CHG_STAT_PRE_CHARGE,
    CHG_STAT_FAST_CHARGE,
    CHG_STAT_CHARGE_DONE,
};

enum vindpm_track {
    SC8960X_TRACK_DIS,
    SC8960X_TRACK_200,
    SC8960X_TRACK_250,
    SC8960X_TRACK_300,
};

enum sc8960x_dpdm_stat {
    DPDM_HIZ,
    DMDM_PULL_DOWN,
    DPDM_0p6,
    DPDM_1p2,
    DPDM_2p0,
    DPDM_2p7,
    DPDM_3p3,
};

enum sc8960x_fields {
    EN_HIZ, EN_STAT_PIN, IINDPM, /*reg00*/
    PFM_DIS, WD_RST, OTG_CFG, CHG_CFG, VSYS_MIN, VBATLOW_OTG, /*reg01*/
    BOOST_LIM, ICC, /*reg02*/
    ITC, ITERM, /*reg03*/
    VBAT_REG, TOP_OFF_TIMER, VRECHG, /*reg04*/
    EN_TERM, TWD, EN_TIMER, CHG_TIMER, TREG, JEITA_COOL_ISET, /*reg05*/
    VAC_OVP, BOOSTV0, VINDPM, /*reg06*/ 
    FORCE_DPDM, TMR2X_EN, BATFET_DIS, JEITA_WARM_VSET1, BATFET_DLY, 
    BATFET_RST_EN, VINDPM_TRACK, /*reg07*/
    VBUS_STAT, CHG_STAT, PG_STAT, THERM_STAT, VSYS_STAT, /*reg08*/
    WD_FAULT, BOOST_FAULT, CHRG_FAULT, BAT_FAULT, NTC_FAULT, /*reg09*/
    VBUS_GD, VINDPM_STAT, IINDPM_STAT, CV_STAT, TOP_OFF_ACTIVE, 
    ACOV_STAT, VINDPM_INT_MASK, IINDPM_INT_MASK, /*reg0A*/
    REG_RST, PN, DEV_VERSION, /*reg0B*/
    JEITA_COOL_ISET2, JEITA_WARM_VSET2, JEITA_WARM_ISET, JEITA_COOL_TEMP,
    JEITA_WARM_TEMP, /*reg0C*/
    VBAT_REG_FT, BOOST_NTC_HOT_TEMP, BOOST_NTC_COLD_TEMP, BOOSTV1, ISHORT, /*reg0D*/
    VTC, INPUT_DET_DONE, AUTO_DPDM_EN, BUCK_FREQ, BOOST_FREQ, VSYSOVP, 
    NTC_DIS, /*reg0E*/
    THERMAL_LOOP_DIS,/*reg80*/
    EN_HVDCP, DM_DRIVE, DP_DRIVE,/*reg81*/
    IBUS_LOOP_DIS,/*reg90*/
    VALLEY_CUR_LIMIT,/*reg93*/

    F_MAX_FIELDS,
};

enum sc8960_reg_range {
    SC8960X_IINDPM,
    SC8960X_ICHG,
    SC8960X_IBOOST,
    SC8960X_VBAT_REG,
    SC8960X_VINDPM,
    SC8960X_ITERM,
};

struct reg_range {
    u32 min;
    u32 max;
    u32 step;
    u32 offset;
    const u32 *table;
    u16 num_table;
    bool round_up;
};

struct sc8960x_param_e {
    int pfm_dis;
    int vsys_min;
    int vbat_low_otg;
    int pre_chg_curr;
    int term_chg_curr;
    int vbat_cv;
    int top_off_timer;
    int vrechg_volt;
    int term_en;
    int wdt_timer;
    int en_chg_saft_timer;
    int charge_timer;
    int vac_ovp;
    int iboost;
    int vboost;
    int vindpm_track;
    int vindpm_int_mask;
    int iindpm_int_mask;
    int vpre_to_cc;
    int auto_dpdm_en;
    int ntc_pin_dis;
    int support_otgcc_2A;
};

struct sc8960x_chip {
    struct device *dev;
    struct i2c_client *client;
    struct regmap *regmap;
    struct regmap_field *rmap_fields[F_MAX_FIELDS];

    struct sc8960x_param_e sc8960x_param;

    int irq_gpio;
    int irq;

    int part_no;
    int dev_version;

    int psy_usb_type;
    int chg_type;
    int vbus_type;

    bool vbus_good;
    int typec_attach;

    struct delayed_work force_detect_dwork;
    int force_detect_count;
    bool pr_swap;

    struct delayed_work chg_psy_dwork;

    struct power_supply_desc psy_desc;
    struct power_supply_config psy_cfg;
    struct power_supply *psy;

#if IS_ENABLED(CONFIG_MTK_CHARGER)
    struct charger_device *chg_dev;
#endif /*CONFIG_MTK_CLASS*/

    const char *chg_dev_name;

    bool true_online;

    int power_good;
    atomic_t qc3p0_done;
    int qc_vbus;

    bool bc12_detect;
    struct mutex bc_detect_lock;
    struct delayed_work bc12_timeout_dwork;
    bool bc12_recovery;

    struct delayed_work hiz_cut_dwork;
    bool hiz_cut_flag;

    int vindpm_value;
    struct delayed_work otg_retry_work;
    bool except_otg_flag;
    int ship_mode;
    int erp_force_hiz;
    struct iio_channel *chan_vbus;
    struct notifier_block reboot_notifier;
    bool charge_full;
    bool en;
    struct delayed_work td_dwork;    
};

static const u32 sc8960x_iboost[] = {
    500, 1200,
};

static const u32 sc8960x_vindpm[] = {
    3900, 4000, 4100, 4200, 4300, 4400, 4500, 4600,
    4700, 4800, 4900, 5000, 5100, 8000, 8200, 8400,
};

#define SC8960x_CHG_RANGE(_min, _max, _step, _offset, _ru) \
{ \
    .min = _min, \
    .max = _max, \
    .step = _step, \
    .offset = _offset, \
    .round_up = _ru, \
}

#define SC8960x_CHG_RANGE_T(_table, _ru) \
    { .table = _table, .num_table = ARRAY_SIZE(_table), .round_up = _ru, }


static struct reg_range sc8960x_reg_range[] = {
    [SC8960X_IINDPM]    = SC8960x_CHG_RANGE(100, 3200, 100, 100, false),
    [SC8960X_ICHG]      = SC8960x_CHG_RANGE(0, 3000, 60, 0, false),
    [SC8960X_IBOOST]    = SC8960x_CHG_RANGE_T(sc8960x_iboost, false),
    [SC8960X_VBAT_REG]  = SC8960x_CHG_RANGE(3848, 4864, 8, 3848, false),
    [SC8960X_VINDPM]    = SC8960x_CHG_RANGE_T(sc8960x_vindpm, false),
    [SC8960X_ITERM]     = SC8960x_CHG_RANGE(60, 960, 60, 60, false),
};

__maybe_unused static struct reg_range sc89619x_reg_range[] = {
    [SC8960X_ICHG]      = SC8960x_CHG_RANGE(0, 1280, 20, 0, false),
    [SC8960X_ITERM]     = SC8960x_CHG_RANGE(20, 320, 20, 20, false),
};

//REGISTER
static const struct reg_field sc8960x_reg_fields[] = {
    /*reg00*/
    [EN_HIZ] = REG_FIELD(0x00, 7, 7),
    [EN_STAT_PIN] = REG_FIELD(0x00, 5, 6),
    [IINDPM] = REG_FIELD(0x00, 0, 4),
    /*reg01*/
    [PFM_DIS] = REG_FIELD(0x01, 7, 7),
    [WD_RST] = REG_FIELD(0x01, 6, 6),
    [OTG_CFG] = REG_FIELD(0x01, 5, 5),
    [CHG_CFG] = REG_FIELD(0x01, 4, 4),
    [VSYS_MIN] = REG_FIELD(0x01, 1, 3),
    [VBATLOW_OTG] = REG_FIELD(0x01, 0, 0),
    /*reg02*/
    [BOOST_LIM] = REG_FIELD(0x02, 7, 7),
    [ICC] = REG_FIELD(0x02, 0, 5),
    /*reg03*/
    [ITC] = REG_FIELD(0x03, 4, 7),
    [ITERM] = REG_FIELD(0x03, 0, 3),
    /*reg04*/
    [VBAT_REG] = REG_FIELD(0x04, 3, 7),
    [TOP_OFF_TIMER] = REG_FIELD(0x04, 1, 2),
    [VRECHG] = REG_FIELD(0x04, 0, 0),
    /*reg05*/
    [EN_TERM] = REG_FIELD(0x05, 7, 7),
    [TWD] = REG_FIELD(0x05, 4, 5),
    [EN_TIMER] = REG_FIELD(0x05, 3, 3),
    [CHG_TIMER] = REG_FIELD(0x05, 2, 2),
    [TREG] = REG_FIELD(0x05, 1, 1),
    [JEITA_COOL_ISET] = REG_FIELD(0x05, 0, 0),
    /*reg06*/
    [VAC_OVP] = REG_FIELD(0x06, 6, 7),
    [BOOSTV0] = REG_FIELD(0x06, 4, 5),
    [VINDPM] = REG_FIELD(0x06, 0, 3),
    /*reg07*/
    [FORCE_DPDM] = REG_FIELD(0x07, 7, 7),
    [TMR2X_EN] = REG_FIELD(0x07, 6, 6),
    [BATFET_DIS] = REG_FIELD(0x07, 5, 5),
    [JEITA_WARM_VSET1] = REG_FIELD(0x07, 4, 4),
    [BATFET_DLY] = REG_FIELD(0x07, 3, 3),
    [BATFET_RST_EN] = REG_FIELD(0x07, 2, 2),
    [VINDPM_TRACK] = REG_FIELD(0x07, 0, 1),
    /*reg08*/
    [VBUS_STAT] = REG_FIELD(0x08, 5, 7),
    [CHG_STAT] = REG_FIELD(0x08, 3, 4),
    [PG_STAT] = REG_FIELD(0x08, 2, 2),
    [THERM_STAT] = REG_FIELD(0x08, 1, 1),
    [VSYS_STAT] = REG_FIELD(0x08, 0, 0),
    /*reg09*/
    [WD_FAULT] = REG_FIELD(0x09, 7, 7),
    [BOOST_FAULT] = REG_FIELD(0x09, 6, 6),
    [CHRG_FAULT] = REG_FIELD(0x09, 4, 5),
    [BAT_FAULT] = REG_FIELD(0x09, 3, 3),
    [NTC_FAULT] = REG_FIELD(0x09, 0, 2),
    /*reg0A*/
    [VBUS_GD] = REG_FIELD(0x0A, 7, 7),
    [VINDPM_STAT] = REG_FIELD(0x0A, 6, 6),
    [IINDPM_STAT] = REG_FIELD(0x0A, 5, 5),
    [CV_STAT] = REG_FIELD(0x0A, 4, 4),
    [TOP_OFF_ACTIVE] = REG_FIELD(0x0A, 3, 3),
    [ACOV_STAT] = REG_FIELD(0x0A, 2, 2),
    [VINDPM_INT_MASK] = REG_FIELD(0x0A, 1, 1),
    [IINDPM_INT_MASK] = REG_FIELD(0x0A, 0, 0),
    /*reg0B*/
    [REG_RST] = REG_FIELD(0x0B, 7, 7),
    [PN] = REG_FIELD(0x0B, 3, 6),
    [DEV_VERSION] = REG_FIELD(0x0B, 0, 2),
    /*reg0C*/
    [JEITA_COOL_ISET2] = REG_FIELD(0x0C, 7, 7),
    [JEITA_WARM_VSET2] = REG_FIELD(0x0C, 6, 6),
    [JEITA_WARM_ISET] = REG_FIELD(0x0C, 4, 5),
    [JEITA_COOL_TEMP] = REG_FIELD(0x0C, 2, 3),
    [JEITA_WARM_TEMP] = REG_FIELD(0x0C, 0, 1),
    /*reg0D*/
    [VBAT_REG_FT] = REG_FIELD(0x0D, 6, 7),
    [BOOST_NTC_HOT_TEMP] = REG_FIELD(0x0D, 4, 5),
    [BOOST_NTC_COLD_TEMP] = REG_FIELD(0x0D, 3, 3),
    [BOOSTV1] = REG_FIELD(0x0D, 1, 2),
    [ISHORT] = REG_FIELD(0x0D, 0, 0),
    /*reg0E*/
    [VTC] = REG_FIELD(0x0E, 7, 7),
    [INPUT_DET_DONE] = REG_FIELD(0x0E, 6, 6),
    [AUTO_DPDM_EN] = REG_FIELD(0x0E, 5, 5),
    [BUCK_FREQ] = REG_FIELD(0x0E, 4, 4),
    [BOOST_FREQ] = REG_FIELD(0x0E, 3, 3),
    [VSYSOVP] = REG_FIELD(0x0E, 1, 2),
    [NTC_DIS] = REG_FIELD(0x0E, 0, 0),
    /*0x80*/
    [THERMAL_LOOP_DIS] = REG_FIELD(0x80, 6, 6),
    /*reg81*/
    [DP_DRIVE] = REG_FIELD(0x81, 5, 7),
    [DM_DRIVE] = REG_FIELD(0x81, 2, 4),
    [EN_HVDCP] = REG_FIELD(0x81, 1, 1),
    /*0x90*/
    [IBUS_LOOP_DIS] = REG_FIELD(0x90, 6, 6),
    /*0x93*/
    [VALLEY_CUR_LIMIT] = REG_FIELD(0x93, 6, 6),
};

static int sc8960x_regmap_write(void *context, unsigned int reg,
				       unsigned int val)
{
    struct sc8960x_chip *sc = context;
    struct i2c_client *i2c = to_i2c_client(sc->dev);
    int ret = 0;

    if (val > 0xff || reg > 0xff)
        return -EINVAL;

    if (reg == 0x7) {
        ret = i2c_smbus_read_byte_data(i2c, reg);
        ret &= 0x80;
        if (ret) {
            val &= 0x7F;
            dev_err(sc->dev, "bc12 do not finish, clear force dpdm bit, new val = 0x%x\n", val);
        }
    }

    return i2c_smbus_write_byte_data(i2c, reg, val);
}

static int sc8960x_regmap_read(void *context, unsigned int reg,
				      unsigned int *val)
{
	struct sc8960x_chip *sc = context;
	struct i2c_client *i2c = to_i2c_client(sc->dev);
	int ret;

	if (reg > 0xff)
		return -EINVAL;

	ret = i2c_smbus_read_byte_data(i2c, reg);
	if (ret < 0)
		return ret;

	*val = ret;

	return 0;
}

static const struct regmap_bus sc8960x_regmap_bus = {
	.reg_write = sc8960x_regmap_write,
	.reg_read = sc8960x_regmap_read,
};

static const struct regmap_config sc8960x_regmap_config = {
    .reg_bits = 8,
    .val_bits = 8,
    
    // .max_register = SC8960X_REGMAX,
};

/********************COMMON API***********************/
static u8 val2reg(enum sc8960_reg_range id, u32 val)
{
    int i;
    u8 reg;
    const struct reg_range *range = &sc8960x_reg_range[id];

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

static u32 reg2val(enum sc8960_reg_range id, u8 reg)
{
    const struct reg_range *range = &sc8960x_reg_range[id];
    if (!range)
        return reg;
    return range->table ? range->table[reg] :
                  range->offset + range->step * reg;
}

/*********************I2C API*********************/
static int sc8960x_field_read(struct sc8960x_chip *sc,
                enum sc8960x_fields field_id, int *val)
{
    int ret;

    ret = regmap_field_read(sc->rmap_fields[field_id], val);
    if (ret < 0) {
        dev_err(sc->dev, "sc8960x read field %d fail: %d\n", field_id, ret);
    }
    
    return ret;
}

static int sc8960x_field_write(struct sc8960x_chip *sc,
                enum sc8960x_fields field_id, int val)
{
    int ret;
    
    ret = regmap_field_write(sc->rmap_fields[field_id], val);
    if (ret < 0) {
        dev_err(sc->dev, "sc8960x read field %d fail: %d\n", field_id, ret);
    }
    
    return ret;
}


/*********************CHIP API*********************/
static int sc8960x_set_key(struct sc8960x_chip *sc)
{
    regmap_write(sc->regmap, SC8960X_REG_7F, REG7F_KEY1);
    regmap_write(sc->regmap, SC8960X_REG_7F, REG7F_KEY2);
    regmap_write(sc->regmap, SC8960X_REG_7F, REG7F_KEY3);
    regmap_write(sc->regmap, SC8960X_REG_7F, REG7F_KEY4);
    regmap_write(sc->regmap, SC8960X_REG_7F, REG7F_KEY5);
    regmap_write(sc->regmap, SC8960X_REG_7F, REG7F_KEY6);
    regmap_write(sc->regmap, SC8960X_REG_7F, REG7F_KEY7);
    return regmap_write(sc->regmap, SC8960X_REG_7F, REG7F_KEY8);
}

static int sc8960x_get_key_status(struct sc8960x_chip *sc) {
    int reg_val;
    int ret;

    ret = regmap_read(sc->regmap, SC8960X_REG_7F, &reg_val);
    if (ret < 0) {
        dev_err(sc->dev, "%s: sc8960x read reg 0x7f fail: %d\n", __func__, ret);
        return ret;
    }
    return reg_val;
}

__attribute__((unused)) 
static int sc8960x_soft_enable_hvdcp(struct sc8960x_chip *sc)
{
    int ret;
    int val = 0;
    ret = regmap_read(sc->regmap, SC8960X_REG_81, &val);
    if (ret) {
        sc8960x_set_key(sc);
    }

    /*dp and dm connected,dp 0.6V dm Hiz*/
    sc8960x_field_write(sc, DP_DRIVE, 0x02);
    sc8960x_field_write(sc, DM_DRIVE, 0x00);
    regmap_read(sc->regmap, SC8960X_REG_81, &val);
    pr_err("%s ----> reg 81 = 0x%02x\n", __func__, val);

    return sc8960x_set_key(sc);
}

__attribute__((unused)) 
static int sc8960x_enable_hvdcp(struct sc8960x_chip *sc, bool enable)
{
    int ret;
	int val = (enable == true) ? 
		REG81_HVDCP_ENABLE : REG81_HVDCP_DISABLE;

    ret = regmap_read(sc->regmap, SC8960X_REG_81, &val);
    if (ret) {
        sc8960x_set_key(sc);
    }
	sc8960x_field_write(sc, EN_HVDCP, val);
	regmap_read(sc->regmap, SC8960X_REG_81, &val);
	dev_err(sc->dev, "----> reg 81 = 0x%02x\n", val);

	return sc8960x_set_key(sc);
}

__maybe_unused
static int sc8960x_auto_bc12_en(struct sc8960x_chip *sc, bool enable)
{
    int ret;
    int val = (enable == true) ? 1 : 0;
    dev_err(sc->dev, "%s\n", __func__);
    ret = regmap_read(sc->regmap, SC8960X_REG_0E, &val);
    if (ret) {
        sc8960x_set_key(sc);
    }
        sc8960x_field_write(sc, AUTO_DPDM_EN, val);
        regmap_read(sc->regmap, SC8960X_REG_0E, &val);
        dev_err(sc->dev, "----> reg 0x0E = 0x%02x\n", val);

        return sc8960x_set_key(sc);
}

__attribute__((unused)) 
static int sc8960x_set_qc3p0_dpdm_volt_init(struct sc8960x_chip *sc)
{
    int ret;
    int val = 0;
    ret = regmap_read(sc->regmap, SC8960X_REG_81, &val);
    if (ret) {
        sc8960x_set_key(sc);
    }

    /*dp and dm connected,dp 0.6V dm 3.3V*/
    sc8960x_field_write(sc, DP_DRIVE, 0x02);
    sc8960x_field_write(sc, DM_DRIVE, 0x06);
    regmap_read(sc->regmap, SC8960X_REG_81, &val);
    pr_err("%s ----> reg 81 = 0x%02x\n", __func__, val);

    return sc8960x_set_key(sc);
}

#define QC_3P0_VOLT_STEP 200
__attribute__((unused)) 
static int sc8960x_set_hvdcp_3p0_increase_volt(struct sc8960x_chip *sc, int vbus_target_mv, int vbus_now_mv)
{
    int ret = 0;
    int up_step = 0;
    int i = 0;
    int val = 0;

    up_step = (vbus_target_mv - vbus_now_mv) / QC_3P0_VOLT_STEP;

    ret = regmap_read(sc->regmap, SC8960X_REG_81, &val);
    if (ret) {
        sc8960x_set_key(sc);
    }

    for (i = 0; i < up_step; i++) {
        if (sc->vbus_good == 0) {
            dev_err(sc->dev, "vbus gd become 0, exit process\n");
            goto vbus_absent;
        }
        sc8960x_field_write(sc, DP_DRIVE, 0x06);
        mdelay(8);
        sc8960x_field_write(sc, DP_DRIVE, 0x02);
    }

	return sc8960x_set_key(sc);

vbus_absent:
    sc8960x_field_write(sc, DP_DRIVE, 0x00);
    sc8960x_field_write(sc, DM_DRIVE, 0x00);
    return sc8960x_set_key(sc);
}

__attribute__((unused)) 
static int sc8960x_set_hvdcp_3p0_decrease_volt(struct sc8960x_chip *sc, int vbus_target_mv, int vbus_now_mv)
{
    int ret = 0;
    int down_step = 0;
    int i = 0;
    int val = 0;

    down_step = (vbus_now_mv - vbus_target_mv) / QC_3P0_VOLT_STEP;

    ret = regmap_read(sc->regmap, SC8960X_REG_81, &val);
    if (ret) {
        sc8960x_set_key(sc);
    }

    for (i = 0; i < down_step; i++) {
        if (sc->vbus_good == 0) {
            dev_err(sc->dev, "vbus gd become 0, exit process\n");
            goto vbus_absent;
        }
        sc8960x_field_write(sc, DM_DRIVE, 0x02);
        mdelay(8);
        sc8960x_field_write(sc, DM_DRIVE, 0x06);
    }

	return sc8960x_set_key(sc);

vbus_absent:
    sc8960x_field_write(sc, DP_DRIVE, 0x00);
    sc8960x_field_write(sc, DM_DRIVE, 0x00);
    return sc8960x_set_key(sc);
}

__attribute__((unused)) 
static int sc8960x_dis_thermal(struct sc8960x_chip *sc)
{
    int ret;
    int val = 0;
    ret = regmap_read(sc->regmap, SC8960X_REG_80, &val);
    if (ret) {
        sc8960x_set_key(sc);
    }
	sc8960x_field_write(sc, THERMAL_LOOP_DIS, 1);
	regmap_read(sc->regmap, SC8960X_REG_80, &val);
	dev_err(sc->dev, "----> reg 80 = 0x%02x\n", val);

	return sc8960x_set_key(sc);
}

__attribute__((unused)) 
static int sc8960x_set_nonstandard_detect(struct sc8960x_chip *sc)
{
    int ret;
    int reg_val;
    
    ret = regmap_read(sc->regmap, SC8960X_REG_92, &reg_val);
    if (ret) {
        sc8960x_set_key(sc);
    }
    regmap_write(sc->regmap, SC8960X_REG_92, REG92_PFM_VAL);
    
    return sc8960x_set_key(sc);
}

__attribute__((unused)) 
static int sc8960x_set_dp(struct sc8960x_chip *sc, enum sc8960x_dpdm_stat stat)
{
    int ret;
    int reg_val;
    
    ret = regmap_read(sc->regmap, SC8960X_REG_81, &reg_val);
    if (ret) {
        sc8960x_set_key(sc);
    }
    sc8960x_field_write(sc, DP_DRIVE, stat);
    
    return sc8960x_set_key(sc);
}

__attribute__((unused)) 
static int sc8960x_set_dm(struct sc8960x_chip *sc, enum sc8960x_dpdm_stat stat)
{
    int ret;
    int reg_val;
    
    ret = regmap_read(sc->regmap, SC8960X_REG_81, &reg_val);
    if (ret) {
        sc8960x_set_key(sc);
    }
    sc8960x_field_write(sc, DM_DRIVE, stat);
    
    return sc8960x_set_key(sc);
}

__attribute__((unused)) 
static int sc8960x_set_dpdm_hiz(struct sc8960x_chip *sc)
{
    int ret;
    ret = sc8960x_set_dm(sc, DPDM_HIZ);
    ret |= sc8960x_set_dp(sc, DPDM_HIZ);
    return ret;
}

__attribute__((unused)) 
static int sc8960x_set_hiz(struct sc8960x_chip *sc, bool enable)
{
    int reg_val = enable ? 1 : 0;
    int read_reg_val = 0;
    int ret = 0;

    ret = sc8960x_field_read(sc, EN_HIZ, &read_reg_val);
    if(ret < 0) {
        dev_err(sc->dev, "%s: sc8960x read hiz fail: %d\n", __func__, ret);
        return ret;
    }
    if (read_reg_val == reg_val) {
        dev_info(sc->dev, "%s: hiz already %d\n", __func__, reg_val);
        return 0;
    }
    dev_info(sc->dev, "%s hiz\n", enable ? "enable" : "disable");
    if (enable == false) {
        schedule_delayed_work(&sc->hiz_cut_dwork,msecs_to_jiffies(DEFAULT_HIZ_CUT_TIME_EXPRIE));
        sc->hiz_cut_flag = true;
    } else {
        if (sc->hiz_cut_flag) {
            dev_info(sc->dev, "disable hiz not end, after 300ms retry\n");
            //return -EBUSY;
        }
    }
    return sc8960x_field_write(sc, EN_HIZ, reg_val);
}

static int sc8960x_set_vindpm_track(struct sc8960x_chip *sc, enum vindpm_track track)
{
    return sc8960x_field_write(sc, VINDPM_TRACK, track);
}

static int sc8960x_set_iindpm(struct sc8960x_chip *sc, int curr_ma)
{
    int reg_val = val2reg(SC8960X_IINDPM, curr_ma);

    return sc8960x_field_write(sc, IINDPM, reg_val);
}

static int sc8960x_get_iindpm(struct sc8960x_chip *sc, int *curr_ma)
{
    int ret, reg_val;

    ret = sc8960x_field_read(sc, IINDPM, &reg_val);
    if (ret) {
        dev_err(sc->dev, "read iindpm failed(%d)\n", ret);
        return ret;
    }

    *curr_ma = reg2val(SC8960X_IINDPM, reg_val);

    return ret;
}

__attribute__((unused)) static int sc8960x_reset_wdt(struct sc8960x_chip *sc)
{
    return sc8960x_field_write(sc, WD_RST, 1);
}

static int sc8960x_set_chg_enable(struct sc8960x_chip *sc, bool enable)
{
    int reg_val = enable ? 1 : 0;

    return sc8960x_field_write(sc, CHG_CFG, reg_val);
}

__attribute__((unused)) static int sc8960x_check_chg_enabled(struct sc8960x_chip *sc, bool *enable)
{
    int ret, reg_val;

    ret = sc8960x_field_read(sc, CHG_CFG, &reg_val);
    if (ret) {
        dev_err(sc->dev, "read charge enable failed(%d)\n", ret);
        return ret;
    }
    *enable = !!reg_val;

    return ret;
}

__attribute__((unused)) static int sc8960x_set_otg_enable(struct sc8960x_chip *sc, bool enable)
{
    int ret;
    int reg_val = enable ? 1 : 0;
    int read_reg_val = 0;
    
    sc8960x_field_write(sc, BOOST_FREQ, 1);
    if (sc->sc8960x_param.support_otgcc_2A == 0) {
        ret = sc8960x_field_write(sc, OTG_CFG, reg_val);
    } else {
        if (enable) {
            if (sc8960x_get_key_status(sc) == 0) {
                sc8960x_set_key(sc);
            }
            if (sc8960x_field_write(sc, THERMAL_LOOP_DIS, 1) != 0) {
                dev_info(sc->dev, "THERMAL_LOOP_DIS set 1 failed\n");
            }
            if (sc8960x_field_write(sc, IBUS_LOOP_DIS, 1) != 0) {
                dev_info(sc->dev, "close ibus loop failed\n");
            }
            regmap_write(sc->regmap, SC8960X_REG_93, REG93_VAL);
            ret = regmap_read(sc->regmap, SC8960X_REG_80, &read_reg_val);
            dev_err(sc->dev, "%s: sc8960x read reg 0x80 ret:%d val:0x%02x\n", __func__, ret, read_reg_val);
            ret = regmap_read(sc->regmap, SC8960X_REG_90, &read_reg_val);
            dev_err(sc->dev, "%s: sc8960x read reg 0x90 ret:%d val:0x%02x\n", __func__, ret, read_reg_val);
            ret = regmap_read(sc->regmap, SC8960X_REG_93, &read_reg_val);
            dev_err(sc->dev, "%s: sc8960x read reg 0x93 ret:%d val:0x%02x\n", __func__, ret, read_reg_val);
            ret = sc8960x_set_key(sc);
            mdelay(10);
            ret |= sc8960x_field_write(sc, OTG_CFG, reg_val);
        } else {
            ret = sc8960x_field_write(sc, OTG_CFG, reg_val);
            if (sc8960x_get_key_status(sc) == 0) {
                sc8960x_set_key(sc);
            }
            if (sc8960x_field_write(sc, THERMAL_LOOP_DIS, 0) != 0) {
                dev_info(sc->dev, "THERMAL_LOOP_DIS set 0 failed\n");
            }
            if (sc8960x_field_write(sc, IBUS_LOOP_DIS, 0) != 0) {
                dev_info(sc->dev, "open ibus loop failed\n");
            }
            regmap_write(sc->regmap, SC8960X_REG_93, REG93_VAL);
            ret = regmap_read(sc->regmap, SC8960X_REG_80, &read_reg_val);
            dev_err(sc->dev, "%s: sc8960x read reg 0x80 ret:%d val:0x%02x\n", __func__, ret, read_reg_val);
            ret = regmap_read(sc->regmap, SC8960X_REG_90, &read_reg_val);
            dev_err(sc->dev, "%s: sc8960x read reg 0x90 ret:%d val:0x%02x\n", __func__, ret, read_reg_val);
            ret = regmap_read(sc->regmap, SC8960X_REG_93, &read_reg_val);
            dev_err(sc->dev, "%s: sc8960x read reg 0x93 ret:%d val:0x%02x\n", __func__, ret, read_reg_val);
            ret |= sc8960x_set_key(sc);
        }
        dev_info(sc->dev, "%s OTG iboost 2A  %s\n", enable ? "enable" : "disable",!ret ? "successfully" : "failed");
    }
    sc->except_otg_flag = enable;
    return ret;
}

__attribute__((unused)) static int sc8960x_set_iboost(struct sc8960x_chip *sc, int curr_ma)
{
    int reg_val = val2reg(SC8960X_IBOOST, curr_ma);

    return sc8960x_field_write(sc, BOOST_LIM, reg_val);
}

static int sc8960x_set_ichg(struct sc8960x_chip *sc, int curr_ma)
{
    int reg_val = val2reg(SC8960X_ICHG, curr_ma);

    return sc8960x_field_write(sc, ICC, reg_val);
}

static int sc8960x_get_ichg(struct sc8960x_chip *sc, int *curr_ma)
{
    int ret, reg_val;

    ret = sc8960x_field_read(sc, ICC, &reg_val);
    if (ret) {
        dev_err(sc->dev, "read ICC failed(%d)\n", ret);
        return ret;
    }

    *curr_ma = reg2val(SC8960X_ICHG, reg_val);

    return ret;
}

__maybe_unused static int sc8960x_set_term_curr(struct sc8960x_chip *sc, u32 curr_ma)
{
    int reg_val = val2reg(SC8960X_ITERM, curr_ma);

    return sc8960x_field_write(sc, ITERM, reg_val);
}

__maybe_unused static int sc8960x_get_term_curr(struct sc8960x_chip *sc, int *curr_ma)
{
    int ret, reg_val;

    ret = sc8960x_field_read(sc, ITERM, &reg_val);
    if (ret)
        return ret;
    
    *curr_ma = reg2val(SC8960X_ITERM, reg_val);

    return ret;
}

__attribute__((unused)) static int sc8960x_set_safet_timer(struct sc8960x_chip *sc, bool enable)
{
    int reg_val = enable ? 1 : 0;

    return sc8960x_field_write(sc, EN_TIMER, reg_val);
}

__attribute__((unused)) static int sc8960x_check_safet_timer(struct sc8960x_chip *sc, bool *enabled)
{
    int ret, reg_val;

    ret = sc8960x_field_read(sc, EN_TIMER, &reg_val);
    if (ret) {
        dev_err(sc->dev, "read ICEN_TIMERC failed(%d)\n", ret);
        return ret;
    }

    *enabled = reg_val ? true : false;

    return ret;
}

static int sc8960x_set_vbat(struct sc8960x_chip *sc, int volt_mv)
{
    int reg_val = val2reg(SC8960X_VBAT_REG, volt_mv);

    sc8960x_field_write(sc, VBAT_REG_FT, reg_val & 0x03);
    return sc8960x_field_write(sc, VBAT_REG, reg_val >> 2);
}

static int sc8960x_get_vbat(struct sc8960x_chip *sc, int *volt_mv)
{
    int ret, reg_val_l, reg_val_h;

    ret = sc8960x_field_read(sc, VBAT_REG_FT, &reg_val_l);
    ret |= sc8960x_field_read(sc, VBAT_REG, &reg_val_h);
    if (ret) {
        dev_err(sc->dev, "read vbat reg failed(%d)\n", ret);
        return ret;
    }

    *volt_mv = reg2val(SC8960X_VBAT_REG, (reg_val_h << 2) | reg_val_l);

    return ret;
}

bool work_flag = 0;
static int sc8960x_set_vindpm(struct sc8960x_chip *sc, int volt_mv)
{
    int reg_val = 0;
    int ret = 0;

    if(work_flag == 1) {
	dev_info(sc->dev, "td_work run!!!\n");
	return ret;
    }

    if (volt_mv == VINDPM_BEFORCE_BC12 && sc->bc12_detect) {
        volt_mv = VIMDPM_DEFAULT_IN_BC12;
        dev_info(sc->dev, "bc12 running, set vindpm to 0, avoid adapter fall\n");
    } else {
        sc->vindpm_value = volt_mv;
        dev_info(sc->dev, "update vindpm value = %d\n", sc->vindpm_value);
    }

    reg_val = val2reg(SC8960X_VINDPM, volt_mv);
    
    return sc8960x_field_write(sc, VINDPM, reg_val);
}

__maybe_unused static int sc8960x_get_vindpm(struct sc8960x_chip *sc, int *volt_mv)
{
    int ret, reg_val;

    ret = sc8960x_field_read(sc, VINDPM, &reg_val);
    if (ret)
        return ret;

    *volt_mv = reg2val(SC8960X_VINDPM, reg_val);

    return ret;
}

__maybe_unused 
static int sc8960x_qc3p0_minus(struct sc8960x_chip *sc)
{
    int ret;

    ret = sc8960x_set_dm(sc, DPDM_0p6);
    usleep_range(1000, 5000);
    ret |= sc8960x_set_dm(sc, DPDM_3p3);
    usleep_range(1000, 2000);

    return ret;
}

__maybe_unused 
static int sc8960x_qc3p0_plus(struct sc8960x_chip *sc)
{
    int ret;

    ret = sc8960x_set_dp(sc, DPDM_3p3);
    usleep_range(1000, 5000);
    ret |= sc8960x_set_dp(sc, DPDM_0p6);
    usleep_range(1000, 2000);

    return ret;
}

__maybe_unused
static int sc8960x_qc3p0_request_vbus(struct sc8960x_chip *sc, int mv)
{
    int count = 0, i;
    int ret = 0;

    ret = atomic_read(&sc->qc3p0_done);
    if(!ret){
        dev_err(sc->dev, "qc3p0 not done\n");
        ret = -EPERM;
        goto out;
    }
    atomic_set(&sc->qc3p0_done, 0);
    count  = (mv - sc->qc_vbus) / 200;
    for (i = 0; i < abs(count); i++){
        if (count > 0)
            ret |= sc8960x_qc3p0_plus(sc);
        else
            ret |= sc8960x_qc3p0_minus(sc);        
    }
    if (ret >= 0){
        sc->qc_vbus = mv;
        atomic_set(&sc->qc3p0_done, 1);
    } else {
        dev_err(sc->dev, "qc3p0 set fail\n");
        sc->qc_vbus = 5000;
        atomic_set(&sc->qc3p0_done, 1);        
    }
        
out:
    return ret;
}

static int sc8960x_force_dpdm(struct sc8960x_chip *sc);
static void sc8960x_bc12_timeout_dwork_handler(struct work_struct *work) {
    int ret;
    int vbus_stat = 0;
    int force_dpdm_stat = 0;
    int pg_state = 0;
    struct sc8960x_chip *sc = container_of(work,
                                        struct sc8960x_chip,
                                        bc12_timeout_dwork.work);


    ret = sc8960x_field_read(sc, VBUS_STAT, &vbus_stat);
    dev_info(sc->dev, "vbus stat = %d\n", vbus_stat);
    ret = sc8960x_field_read(sc, PG_STAT, &pg_state);
    dev_info(sc->dev, "pg stat = %d\n", pg_state);
    ret = regmap_read(sc->regmap, 0x07, &force_dpdm_stat);
    force_dpdm_stat &= 0x80;
    dev_info(sc->dev, "force_dpdm = %d\n", force_dpdm_stat);

    mutex_lock(&sc->bc_detect_lock);
    sc->bc12_detect = false;
    mutex_unlock(&sc->bc_detect_lock);
    if (force_dpdm_stat && !vbus_stat && !pg_state && !sc->bc12_recovery) {
	dev_info(sc->dev, "BC1.2 timeout\n");
	sc8960x_force_dpdm(sc);
        sc->bc12_recovery = true;
    }
}

static irqreturn_t sc8960x_irq_handler(int irq, void *data);
static void sc8960x_hiz_cut_dwork_handler(struct work_struct *work) {
    struct sc8960x_chip *sc = container_of(work,
                                    struct sc8960x_chip,
                                    hiz_cut_dwork.work);
    dev_info(sc->dev, "disable hiz cut end\n");
    sc->hiz_cut_flag = false;
    sc8960x_irq_handler(sc->irq, (void *)sc);
}

static int sc8960x_force_dpdm(struct sc8960x_chip *sc)
{
    int reg_val;
    struct i2c_client *i2c = to_i2c_client(sc->dev);

    dev_info(sc->dev, "%s\n", __func__);
    sc8960x_set_vindpm(sc, 4800);
    mutex_lock(&sc->bc_detect_lock);
    if (sc->bc12_detect) {
        dev_err(sc->dev, "bc12_detect is true, return!\n");
        mutex_unlock(&sc->bc_detect_lock);
        return -EBUSY;
    }
    sc->bc12_detect = true;
    mutex_unlock(&sc->bc_detect_lock);
    sc8960x_set_vindpm(sc, 0);
    schedule_delayed_work(&sc->bc12_timeout_dwork,
                            msecs_to_jiffies(DEFAULT_BC12_TIME_EXPIRE));
    reg_val = i2c_smbus_read_byte_data(i2c, 0x7);
    sc->power_good = 0;

    return i2c_smbus_write_byte_data(i2c, 0x07, reg_val | 0x80);   
}

static int sc8960x_force_dpdm_reg(struct sc8960x_chip *sc)
{
    int reg_val;
    struct i2c_client *i2c = to_i2c_client(sc->dev);

    dev_info(sc->dev, "%s\n", __func__);

    reg_val = i2c_smbus_read_byte_data(i2c, 0x7);

    return i2c_smbus_write_byte_data(i2c, 0x07, reg_val | 0x80);
}

__maybe_unused static int sc8960x_get_charge_stat(struct sc8960x_chip *sc)
{
    int ret;
    int chg_stat, vbus_stat;

    ret = sc8960x_field_read(sc, CHG_STAT, &chg_stat);
    if (ret)
        return ret;
    
    ret = sc8960x_field_read(sc, VBUS_STAT, &vbus_stat);
    if (ret)
        return ret;
    
    if (vbus_stat == VBUS_STAT_OTG) {
        return POWER_SUPPLY_STATUS_DISCHARGING;
    } else {
        if (sc->charge_full && !sc->en) {
            dev_info(sc->dev, "%s soft charge full\n", __func__);
            return POWER_SUPPLY_STATUS_FULL;
        }
        switch (chg_stat) {
        case CHG_STAT_NOT_CHARGE:
            return POWER_SUPPLY_STATUS_NOT_CHARGING;
        case CHG_STAT_PRE_CHARGE:
        case CHG_STAT_FAST_CHARGE:
            return POWER_SUPPLY_STATUS_CHARGING;
        case CHG_STAT_CHARGE_DONE:
            return POWER_SUPPLY_STATUS_FULL;
        }
    }

    return POWER_SUPPLY_STATUS_UNKNOWN;
}

__attribute__((unused)) static int sc8960x_check_charge_done(struct sc8960x_chip *sc, bool *chg_done)
{
    int ret, reg_val;

    ret = sc8960x_field_read(sc, CHG_STAT, &reg_val);
    if (ret) {
        dev_err(sc->dev, "read charge stat failed(%d)\n", ret);
        return ret;
    }

    *chg_done = (reg_val == CHG_STAT_CHARGE_DONE) ? true : false;
    return ret;
}

static int sc8960x_detect_device(struct sc8960x_chip *sc)
{
    int ret;

    ret = sc8960x_field_read(sc, DEV_VERSION, &sc->dev_version);
    if (ret) {
        dev_err(sc->dev, "read device version failed(%d)\n", ret);
        return ret;
    }

    return sc8960x_field_read(sc, PN, &sc->part_no);
}

static int sc8960x_reg_reset(struct sc8960x_chip *sc)
{
    return sc8960x_field_write(sc, REG_RST, 1);  
}

static int sc8960x_dump_register(struct sc8960x_chip *sc)
{
    int addr;
    int val;
    int ret;

    for (addr = 0x0; addr <= SC8960X_REGMAX; addr++) {
        ret = regmap_read(sc->regmap, addr, &val);
        if (ret) {
            dev_err(sc->dev, "read reg 0x%02x failed(%d)\n", addr, ret);
            return ret;
        }
        dev_info(sc->dev, "Reg[0x%02x] = 0x%02x\n", addr, val);
    }
    return ret;
}


#if IS_ENABLED(CONFIG_MTK_CHARGER)
/********************MTK OPS***********************/
static int sc8960x_plug_in(struct charger_device *chg_dev)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);
    int ret = 0;

    dev_info(sc->dev, "%s\n", __func__);

    // ret = sc8960x_set_chg_enable(sc, true);
    // if (ret) {
    //     dev_err(sc->dev, "Failed to enable charging:%d\n", ret);
    // }

    return ret;
}

static int sc8960x_plug_out(struct charger_device *chg_dev)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);
    int ret = 0;

    dev_info(sc->dev, "%s\n", __func__);

    // ret = sc8960x_set_chg_enable(sc, false);
    // if (ret) {
    //     dev_err(sc->dev, "Failed to disable charging:%d\n", ret);
    // }

    return ret;
}

static int sc8960x_enable(struct charger_device *chg_dev, bool enable)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);
    int ret = 0;
    sc->en = enable;
    if(enable)
        sc8960x_set_hiz(sc, false);

    ret = sc8960x_set_chg_enable(sc, enable);

    dev_info(sc->dev, "%s charger %s\n", enable ? "enable" : "disable",
        !ret ? "successfully" : "failed");

    return ret;
}

static int sc8960x_is_enabled(struct charger_device *chg_dev, bool *enabled)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);
    int ret;
    
    ret = sc8960x_check_chg_enabled(sc, enabled);
    dev_info(sc->dev, "charger is %s\n", *enabled ? "charging" : "not charging");

    return ret;
}

static int sc8960x_get_charging_current(struct charger_device *chg_dev, u32 *curr)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);
    int curr_ma;
    int ret;

    dev_info(sc->dev, "%s\n", __func__);

    ret = sc8960x_get_ichg(sc, &curr_ma);
    if (!ret) {
        *curr = curr_ma * 1000;
    }

    return ret;
}

static int sc8960x_set_charging_current(struct charger_device *chg_dev, u32 curr)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);

    dev_info(sc->dev, "%s: charge curr = %duA\n", __func__, curr);

    return sc8960x_set_ichg(sc, curr / 1000);
}

static int sc8960x_get_input_current(struct charger_device *chg_dev, u32 *curr)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);
    int curr_ma;
    int ret;

    dev_info(sc->dev, "%s\n", __func__);

    ret = sc8960x_get_iindpm(sc, &curr_ma);
    if (!ret) {
        *curr = curr_ma * 1000;
    }

    return ret;
}

static int sc8960x_set_input_current(struct charger_device *chg_dev, u32 curr)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);

    dev_info(sc->dev, "%s: iindpm curr = %duA\n", __func__, curr);

    return sc8960x_set_iindpm(sc, curr / 1000);
}

static int sc8960x_get_constant_voltage(struct charger_device *chg_dev, u32 *volt)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);
    int volt_mv;
    int ret;

    dev_info(sc->dev, "%s\n", __func__);

    ret = sc8960x_get_vbat(sc, &volt_mv);
    if (!ret) {
        *volt = volt_mv * 1000;
    }

    return ret;
}

static int sc8960x_set_constant_voltage(struct charger_device *chg_dev, u32 volt)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);

    dev_info(sc->dev, "%s: charge volt = %dmV\n", __func__, volt);

    return sc8960x_set_vbat(sc, volt / 1000);
}

static int sc8960x_kick_wdt(struct charger_device *chg_dev)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);

    dev_info(sc->dev, "%s\n", __func__);
    return sc8960x_reset_wdt(sc);
}

static int sc8960x_set_ivl(struct charger_device *chg_dev, u32 volt)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);

    dev_info(sc->dev, "%s: vindpm volt = %d\n", __func__, volt);

    return sc8960x_set_vindpm(sc, volt / 1000);
}

static int sc8960x_is_charging_done(struct charger_device *chg_dev, bool *done)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);
    int ret;

    ret = sc8960x_check_charge_done(sc, done);

    if (sc->charge_full) {
        *done = sc->charge_full;
    }

    dev_info(sc->dev, "%s: charge %s done\n", __func__, *done ? "is" : "not");
    return ret;
}

static int sc8960x_get_min_ichg(struct charger_device *chg_dev, u32 *curr)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);

    *curr = 60 * 1000;
    dev_err(sc->dev, "%s\n", __func__);
    return 0;
}

static int sc8960x_dump_registers(struct charger_device *chg_dev)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);

    dev_info(sc->dev, "%s\n", __func__);

    return sc8960x_dump_register(sc);
}

static int sc8960x_set_eoc_current(struct charger_device *chg_dev, u32 ieoc)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);

    dev_info(sc->dev, "%s  ieoc =%d\n", __func__, ieoc);

    return sc8960x_set_term_curr(sc, ieoc /1000);
}

static int sc8960x_send_ta_current_pattern(struct charger_device *chg_dev, 
        bool is_increase)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);
    int ret;
    int i;

    dev_info(sc->dev, "%s: %s\n", __func__, is_increase ? 
            "pumpx up" : "pumpx dn");
    //pumpx start
    ret = sc8960x_set_iindpm(sc, 100);
    if (ret)
        return ret;
    msleep(10);
        
    for (i = 0; i < 6; i++) {
        if (i < 3) {
            sc8960x_set_iindpm(sc, 800);
            is_increase ? mdelay(100) : mdelay(300);
            if (!(sc->vbus_good))
                goto vbus_absent;
        } else {
            sc8960x_set_iindpm(sc, 800);
            is_increase ? mdelay(300) : mdelay(100);
            if (!(sc->vbus_good))
                goto vbus_absent;
        }
        sc8960x_set_iindpm(sc, 100);
        mdelay(100);
        if (!(sc->vbus_good))
            goto vbus_absent;
    }

    //pumpx stop
    sc8960x_set_iindpm(sc, 800);
    mdelay(500);
    if (!(sc->vbus_good))
        goto vbus_absent;
    //pumpx wdt, max 240ms
    sc8960x_set_iindpm(sc, 100);
    mdelay(100);
    if (!(sc->vbus_good))
        goto vbus_absent;

    return sc8960x_set_iindpm(sc, 1500);

vbus_absent:
    return 0;
}

static int sc8960x_send_ta20_current_pattern(struct charger_device *chg_dev, 
        u32 uV)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);
    u8 val = 0;
    int i;

    if (uV < 5500000) {
        uV = 5500000;
    } else if (uV > 15000000) {
        uV = 15000000;
    }

    val = (uV - 5500000) / 500000;

    dev_info(sc->dev, "%s ta20 vol=%duV, val=%d\n", __func__, uV, val);
    
    sc8960x_set_iindpm(sc, 100);
    mdelay(150);
    if (!(sc->vbus_good))
        goto vbus_absent;
    for (i = 4; i >= 0; i--) {
        sc8960x_set_iindpm(sc, 800);
        (val & (1 << i)) ? mdelay(100) : mdelay(50);
        if (!(sc->vbus_good))
            goto vbus_absent;
        sc8960x_set_iindpm(sc, 100);
        (val & (1 << i)) ? mdelay(50) : mdelay(100);
        if (!(sc->vbus_good))
            goto vbus_absent;
    }
    sc8960x_set_iindpm(sc, 800);
    mdelay(150);
    if (!(sc->vbus_good))
        goto vbus_absent;
    sc8960x_set_iindpm(sc, 100);
    mdelay(150);
    if (!(sc->vbus_good))
        goto vbus_absent;

    return sc8960x_set_iindpm(sc, 800);

vbus_absent:
    return 0;
}

static int sc8960x_set_ta20_reset(struct charger_device *chg_dev)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);
    int curr;
    int ret;
    
    ret = sc8960x_get_iindpm(sc, &curr);

    ret = sc8960x_set_iindpm(sc, 100);
    msleep(300);
    return sc8960x_set_iindpm(sc, curr);
}

static int sc8960x_get_adc(struct charger_device *chg_dev, enum adc_channel chan,
            int *min, int *max)
{
    switch (chan) {
    case ADC_CHANNEL_VSYS:
        *min = 4500 * 1000;
        *max = 4500 * 1000;
        return 0;
        break;
    
    default:
        break;
    }

    return -95;
}

static int sc8960x_set_otg(struct charger_device *chg_dev, bool enable)
{
    int ret;
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);

    if(enable){
        sc8960x_set_hiz(sc,false);
    }
    ret = sc8960x_set_otg_enable(sc, enable);
    ret |= sc8960x_set_chg_enable(sc, !enable);
 

    dev_info(sc->dev, "%s OTG %s\n", enable ? "enable" : "disable",
        !ret ? "successfully" : "failed");

    return ret;
}

static int sc8960x_set_safety_timer(struct charger_device *chg_dev, bool enable)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);

    dev_info(sc->dev, "%s  %s\n", __func__, enable ? "enable" : "disable");

    return sc8960x_set_safet_timer(sc, enable);
}

static int sc8960x_is_safety_timer_enabled(struct charger_device *chg_dev,
                    bool *enabled)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);
    
    return sc8960x_check_safet_timer(sc, enabled);
}

static int sc8960x_set_boost_ilmt(struct charger_device *chg_dev, u32 curr)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);

    dev_info(sc->dev, "%s otg curr = %d\n", __func__, curr);
    return sc8960x_set_iboost(sc, curr / 1000);
}

static int sc8960x_ops_set_hvdcp_5V(struct sc8960x_chip *sc);
static int sc8960x_do_event(struct charger_device *chg_dev, u32 event, u32 args)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);
    dev_info(sc->dev, "%s\n", __func__);
    switch (event) {
    case EVENT_FULL:
    if (sc->chg_type == POWER_SUPPLY_TYPE_USB_ACA) {
                sc8960x_ops_set_hvdcp_5V(sc);
                pr_info("%s reset hvdcp\n", __func__);
        }
        fallthrough;
    case EVENT_RECHARGE:
    case EVENT_DISCHARGE:
        power_supply_changed(sc->psy);
        break;
    default:
        break;
    }

    return 0;
}

static int sc8960x_enable_hz(struct charger_device *chg_dev, bool enable)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);

    dev_info(sc->dev, "%s %s\n", __func__, enable ? "enable" : "disable");

    if(sc->erp_force_hiz && !enable) {
        dev_info(sc->dev, "hold_hiz for erp test");
        return 0;
    }

    return sc8960x_set_hiz(sc, enable);
}

static int sc8960x_force_bc12(struct charger_device *chg_dev)
{
	struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);
	int ret;

	pr_err("%s\n", __func__);
	sc8960x_auto_bc12_en(sc, true);
	ret = sc8960x_force_dpdm_reg(sc);
	if (ret) {
		pr_err("Failed to enable charging:%d\n", ret);
	}

        sc->vbus_type = 0;

	return ret;
}


static int sc8960x_ops_set_property(struct charger_device *chg_dev,
		enum charger_property prop, union charger_propval *val)
{
	struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);
        int ret = 0;

	switch (prop)
	{
	case CHARGER_PROP_SHIP_MODE:
		sc->ship_mode = val->intval;
                pr_info("%s prop:%d, val:%d\n", __func__, prop, val->intval);
		break;
        case CHARGER_PROP_ERP_FORCE_HIZ:
                sc->erp_force_hiz = val->intval;
		break;
	default:
		ret = -EINVAL;
		break;
	}
        pr_info("%s prop:%d, val:%d\n", __func__, prop, val->intval);
	return ret;
}

#define R_CHARGER_1     330
#define R_CHARGER_2     39
static int get_vbus_mv(struct sc8960x_chip *sc, u32 *vbus)
{
	int ret;

	if (IS_ERR_OR_NULL(sc->chan_vbus)) {
                sc->chan_vbus = devm_iio_channel_get(sc->dev, "pmic_vbus");
	} 
        
        if (!IS_ERR_OR_NULL(sc->chan_vbus)) {
		ret = iio_read_channel_processed(sc->chan_vbus, vbus);
		if (ret < 0) {
			pr_err("[%s](%d) read fail,ret = %d\n", __func__,__LINE__ ,ret);
			return ret;
		}
	} else {
		pr_err("[%s](%d) chan error\n", __func__, __LINE__);
                return -EOPNOTSUPP;
        }

	*vbus = (((R_CHARGER_1 + R_CHARGER_2) * 100 * (*vbus)) / R_CHARGER_2) / 100;

	return ret;
}

static int sc8960x_ops_get_vbus(struct charger_device *chg_dev, u32 *uv)
{
	struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);
	int mv = 0;
	int ret = 0;

	ret = get_vbus_mv(sc, &mv);
	if (ret < 0) {
		pr_err("get vbus fail, ret:%d\n", ret);
		return ret;
	}

	*uv = mv * 1000;

	return ret;
}

static int sc8960x_ops_get_charger_type(struct charger_device *chg_dev, enum lc_chg_type *type)
{
    int ret;
    int reg_val = 0;
    u32 vbus = 0;
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);

    ret = sc8960x_field_read(sc, VBUS_STAT, &reg_val);
    if (ret) {
        pr_err("Failed to get VBUS_STAT:%d\n", ret);
        return ret;
    }

    switch (reg_val) {
    case VBUS_STAT_NO_INPUT:
        if (sc->vbus_good) {
                sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_DCP;
                sc->chg_type = POWER_SUPPLY_TYPE_USB;
        } else {
                sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_UNKNOWN;
                sc->chg_type = POWER_SUPPLY_TYPE_UNKNOWN;
        }
        *type = LC_CHG_TYPE_UNKNOWN;
        break;
    case VBUS_STAT_SDP:
        sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_SDP;
        sc->chg_type = POWER_SUPPLY_TYPE_USB;
        *type = LC_CHG_TYPE_SDP;
        break;
    case VBUS_STAT_CDP:
        sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_CDP;
        sc->chg_type = POWER_SUPPLY_TYPE_USB_CDP;
        *type = LC_CHG_TYPE_CDP;
        break;
    case VBUS_STAT_DCP:
    case VBUS_STAT_HVDCP:
        sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_DCP;
        sc->chg_type = POWER_SUPPLY_TYPE_USB_DCP;
        *type = LC_CHG_TYPE_DCP;
        ret = get_vbus_mv(sc, &vbus);
        pr_info("%s vbus:%d, ret:%d\n", __func__, vbus, ret);
        if (ret >= 0 && vbus > 7000) {
                sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_ACA;
                sc->chg_type = POWER_SUPPLY_TYPE_USB_ACA;
                *type = LC_CHG_TYPE_HVDCP;
        }
        break;
    case VBUS_STAT_UNKOWN:
        sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_DCP;
        sc->chg_type = POWER_SUPPLY_TYPE_USB;
        *type = LC_CHG_TYPE_FLOAT;
        break;
    case VBUS_STAT_NONSTAND:
        sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_DCP;
        sc->chg_type = POWER_SUPPLY_TYPE_USB;
        *type = LC_CHG_TYPE_FLOAT;
        break;
    default:
        sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_DCP;
        sc->chg_type = POWER_SUPPLY_TYPE_USB;
        *type = LC_CHG_TYPE_UNKNOWN;
        break;
    }

    //sc->psy_desc.type = sc->chg_type;

    power_supply_changed(sc->psy);

    dev_info(sc->dev, "%s vbus_stat:0x%02x, type=%d\n", __func__, reg_val, *type);

    return ret;
}

static int sc8960x_ops_set_hvdcp_9V(struct charger_device *chg_dev)
{
	struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);
	int ret;

	pr_err("%s\n", __func__);

        /*dp0.6 DM hiz*/
        ret = sc8960x_set_dp(sc, DPDM_0p6);
        ret |= sc8960x_set_dm(sc, DPDM_HIZ);
        if (ret) {
		pr_err("Failed to set DP-0.6 DM-hiz:%d\n", ret);
                return ret;
	}

	msleep(1500); // may short

	/* set DP 3.3V DM 0.6V*/
	ret = sc8960x_set_dp(sc, DPDM_3p3);
        ret |= sc8960x_set_dm(sc, DPDM_0p6);
        if (ret) {
		pr_err("Failed to set DP-3.3V DM-0.6V:%d\n", ret);
                return ret;
	}

	return ret;
}

static int sc8960x_ops_set_hvdcp_5V(struct sc8960x_chip *sc)
{
    int ret;

    pr_err("%s\n", __func__);

    /*dp0.6 DM hiz*/
    ret = sc8960x_set_dp(sc, DPDM_0p6);
    ret |= sc8960x_set_dm(sc, DPDM_HIZ);
    if (ret) {
        pr_err("Failed to set DP-0.6 DM-hiz:%d\n", ret);
        return ret;
    }

    return ret;
}

static int sc8960x_ops_set_charge_full(struct charger_device *chg_dev, bool full)
{
    struct sc8960x_chip *sc = dev_get_drvdata(&chg_dev->dev);

    pr_info("%s set charge full(%d)\n", __func__, full);
    sc->charge_full = full;

    return 0;
}

static struct charger_ops sc8960x_chg_ops = {
    /* Normal charging */
    .plug_in = sc8960x_plug_in,
    .plug_out = sc8960x_plug_out,
    .enable = sc8960x_enable,
    .is_enabled = sc8960x_is_enabled,
    .get_charging_current = sc8960x_get_charging_current,
    .set_charging_current = sc8960x_set_charging_current,
    .get_input_current = sc8960x_get_input_current,
    .set_input_current = sc8960x_set_input_current,
    .get_constant_voltage = sc8960x_get_constant_voltage,
    .set_constant_voltage = sc8960x_set_constant_voltage,
    .kick_wdt = sc8960x_kick_wdt,
    .set_mivr = sc8960x_set_ivl,
    .is_charging_done = sc8960x_is_charging_done,
    .get_min_charging_current = sc8960x_get_min_ichg,
    .dump_registers = sc8960x_dump_registers,
    .set_eoc_current = sc8960x_set_eoc_current,

    /*Sw charge full*/
    .set_charging_full = sc8960x_ops_set_charge_full,

    /* Safety timer */
    .enable_safety_timer = sc8960x_set_safety_timer,
    .is_safety_timer_enabled = sc8960x_is_safety_timer_enabled,

    /* Power path */
    .enable_powerpath = NULL,
    .is_powerpath_enabled = NULL,

    /* OTG */
    .enable_otg = sc8960x_set_otg,
    .set_boost_current_limit = sc8960x_set_boost_ilmt,
    .enable_discharge = NULL,
 
    /* PE+/PE+20 */
    .send_ta_current_pattern = sc8960x_send_ta_current_pattern,
    .set_pe20_efficiency_table = NULL,
    .send_ta20_current_pattern = sc8960x_send_ta20_current_pattern,
    .reset_ta = sc8960x_set_ta20_reset,
    .enable_cable_drop_comp = NULL,

    /* ADC */
    .get_adc = sc8960x_get_adc,

    .event = sc8960x_do_event,
    .enable_hz = sc8960x_enable_hz,

    /* BC12 */
    .force_bc12 = sc8960x_force_bc12,
    .set_hvdcp_9V = sc8960x_ops_set_hvdcp_9V,
    .get_chg_type = sc8960x_ops_get_charger_type,
    .get_vbus_adc = sc8960x_ops_get_vbus,

    .set_property = sc8960x_ops_set_property,
};

static const struct charger_properties sc8960x_chg_props = {
    .alias_name = "sc89601x_chg",
};

static void sc8960x_set_chg_type(struct sc8960x_chip *sc, int type)
{
}

#endif /*CONFIG_MTK_CLASS */

/**********************interrupt*********************/
static void sc8960x_force_detection_dwork_handler(struct work_struct *work)
{
    int ret;
    struct sc8960x_chip *sc = container_of(work, struct sc8960x_chip, force_detect_dwork.work);

    //Charger_Detect_Init();

    ret = sc8960x_force_dpdm(sc);    
    if (ret) {
        dev_err(sc->dev, "%s: force dpdm failed(%d)\n", __func__, ret);
        return;
    }

    sc->force_detect_count++;
}

static int __maybe_unused sc8960x_get_charger_type(struct sc8960x_chip *sc)
{
    int ret;
    int reg_val = 0;

    if (sc->pr_swap) {
        sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_SDP;
        sc->chg_type = POWER_SUPPLY_TYPE_USB;
        return 0;
    }

    ret = sc8960x_field_read(sc, VBUS_STAT, &reg_val);
    if (ret) {
        return ret;
    }

    switch (reg_val) {
    case VBUS_STAT_NO_INPUT:
        sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_UNKNOWN;
        sc->chg_type = POWER_SUPPLY_TYPE_UNKNOWN;
        break;
    case VBUS_STAT_SDP:
        sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_SDP;
        sc->chg_type = POWER_SUPPLY_TYPE_USB;
        //Charger_Detect_Release();
        break;
    case VBUS_STAT_CDP:
        sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_CDP;
        sc->chg_type = POWER_SUPPLY_TYPE_USB_CDP;
        //Charger_Detect_Release();
        break;
    case VBUS_STAT_DCP:
    case VBUS_STAT_HVDCP:
        sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_DCP;
        sc->chg_type = POWER_SUPPLY_TYPE_USB_DCP;
        //qc3.0 demo 1.enable hvdcp 2. set dm to 3.3V 3. request vbus
        // sc8960x_set_dm(sc, DPDM_3p3);
        // mdelay(1000);
        // sc8960x_qc3p0_request_vbus(sc, 8000);
        // mdelay(1000);
        // sc8960x_qc3p0_request_vbus(sc, 7000);
        break;
    case VBUS_STAT_UNKOWN:
        sc->psy_usb_type =  POWER_SUPPLY_USB_TYPE_UNKNOWN;
        sc->chg_type = POWER_SUPPLY_TYPE_USB;
        //if (sc->force_detect_count < 10) {
        //    schedule_delayed_work(&sc->force_detect_dwork, msecs_to_jiffies(100));
        //}
        break;
    case VBUS_STAT_NONSTAND:
        sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_DCP;
        sc->chg_type = POWER_SUPPLY_TYPE_USB_DCP;
        break;
    default:
        sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_UNKNOWN;
        sc->chg_type = POWER_SUPPLY_TYPE_UNKNOWN;
        break;
    }

#if IS_ENABLED(CONFIG_MTK_CHARGER)
    sc8960x_set_chg_type(sc, reg_val);
#endif /*CONFIG_MTK_CLASS */

    if (reg_val == VBUS_STAT_SDP || reg_val == VBUS_STAT_UNKOWN) {
        sc8960x_set_iindpm(sc, 500);
        sc8960x_set_ichg(sc, 500);
    } else if (reg_val == VBUS_STAT_CDP) {
        sc8960x_set_iindpm(sc, 1500);
        sc8960x_set_ichg(sc, 1500);
    } else if (reg_val == VBUS_STAT_DCP || reg_val == VBUS_STAT_HVDCP) {
        sc8960x_set_iindpm(sc, 2000);
        sc8960x_set_ichg(sc, 2000);
    }

    dev_info(sc->dev, "%s vbus stat: 0x%02x\n", __func__, reg_val);

    return ret;
}

static void sc8960x_inserted_irq(struct sc8960x_chip *sc)
{
    dev_info(sc->dev, "%s: adapter/usb inserted\n", __func__);
    if (sc->pr_swap)
        return;

    sc8960x_set_vindpm_track(sc, SC8960X_TRACK_300);
    sc->force_detect_count = 0;
    sc->qc_vbus = 5000;
    atomic_set(&sc->qc3p0_done, 1);
    //sc8960x_request_dpdm(sc, true);
    //schedule_delayed_work(&sc->force_detect_dwork, msecs_to_jiffies(80));
}

static void sc8960x_removed_irq(struct sc8960x_chip *sc)
{
    dev_info(sc->dev, "%s: adapter/usb removed\n", __func__);
    if (sc->pr_swap)
        sc->pr_swap = false;
    //sc8960x_request_dpdm(sc, false);
    cancel_delayed_work_sync(&sc->force_detect_dwork);
    cancel_delayed_work_sync(&sc->bc12_timeout_dwork);
    mutex_lock(&sc->bc_detect_lock);
    sc->bc12_detect = false;
    sc->bc12_recovery = false;
    mutex_unlock(&sc->bc_detect_lock);
    sc8960x_set_dpdm_hiz(sc);
    sc8960x_set_iindpm(sc, 500);
    sc8960x_set_ichg(sc, 500);
}

void sc8960x_td_test_work_func(struct work_struct *work)
{
    struct delayed_work *td_dwork = NULL;
    struct sc8960x_chip *sc = NULL;
    int bc12_flag, attch_flag, ret;

    pr_notice("%s start\n", __func__);

    td_dwork = container_of(work, struct delayed_work, work);
    if (td_dwork == NULL) {
        pr_err("Cann't get td_dwork\n");
        return;
    }

    sc = container_of(td_dwork, struct sc8960x_chip, td_dwork);
    if (sc == NULL) {
        pr_err("Cann't get sc8960x_device\n");
        return;
    }
    ret = sc8960x_field_read(sc, VBUS_GD, &attch_flag);
    ret |= sc8960x_field_read(sc, PG_STAT, &bc12_flag);
    if (ret) {
        dev_err(sc->dev, "read vbat reg failed(%d)\n", ret);
        return;
    }


    if((bc12_flag == 0)&&(attch_flag == 1))
    {
        pr_notice("%s adapter/usb insert first\n", __func__);
        sc8960x_field_write(sc, VINDPM, val2reg(SC8960X_VINDPM, 12000));
        work_flag = 1;
        pr_notice("%s msleep 1500ms\n", __func__);
        msleep(1500);
        work_flag = 0;
	pr_notice("%s vindpm_value = %d\n", __func__, sc->vindpm_value);
        sc8960x_field_write(sc, VINDPM, val2reg(SC8960X_VINDPM, sc->vindpm_value));
    }

    work_flag = 0;
}

static irqreturn_t sc8960x_irq_handler(int irq, void *data)
{
    int ret;
    int vbus_type = 0;
    int reg_val;
    bool prev_vbus_gd;
    int true_power_good = 0;
    struct sc8960x_chip *sc = (struct sc8960x_chip *)data;

    dev_info(sc->dev, "%s: sc8960x_irq_handler\n", __func__);
    if (sc->hiz_cut_flag) {
        dev_info(sc->dev, "%s: sc8960x in the process of disable hiz\n", __func__);
        return IRQ_HANDLED;
    }
    /* delay400ms 过滤适配器vbus跳变电压的情况 */
    // mdelay(400); 
    // schedule_delayed_work(&sc->td_dwork, msecs_to_jiffies(0));
    vbus_type = sc->vbus_type;
    ret = sc8960x_field_read(sc, VBUS_GD, &reg_val);
    ret |= sc8960x_field_read(sc, PG_STAT, &true_power_good);
    ret |= sc8960x_field_read(sc, VBUS_STAT, &sc->vbus_type);
    if (ret) {
        return IRQ_HANDLED;
    }
    dev_info(sc->dev, "VBUS_GD:%d PG_STAT:%d VBUS_STATE:%d\n",
                reg_val, true_power_good, sc->vbus_type);
    prev_vbus_gd = sc->vbus_good;
    sc->vbus_good = !!reg_val;
    
    if (!prev_vbus_gd && sc->vbus_good) {
        dev_info(sc->dev, "%s: adapter/usb inserted\n", __func__);
	schedule_delayed_work(&sc->td_dwork, msecs_to_jiffies(0));
        sc8960x_inserted_irq(sc);
        sc->true_online = sc->vbus_good;
        sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_DCP;
        sc->chg_type = POWER_SUPPLY_TYPE_USB;
    } else if (prev_vbus_gd && !sc->vbus_good) {
        dev_info(sc->dev, "%s: adapter/usb removed\n", __func__);
        sc8960x_removed_irq(sc);
        //sc8960x_get_charger_type(sc);
        sc->true_online = sc->vbus_good;
        sc->vbus_type = 0;
        sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_UNKNOWN;
        sc->chg_type = POWER_SUPPLY_TYPE_UNKNOWN;
	cancel_delayed_work_sync(&sc->td_dwork);
	sc8960x_auto_bc12_en(sc, false);
	work_flag = 0;
    }

    if (sc->vbus_good) {
        if (sc->chg_type == POWER_SUPPLY_TYPE_UNKNOWN) {//wakeup charge_thread
                sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_DCP;
                sc->chg_type = POWER_SUPPLY_TYPE_USB;
        }
        if (vbus_type != sc->vbus_type) {//notify to mtk ctd
                charger_dev_notify(sc->chg_dev, CHARGER_DEV_NOTIFY_FORCE_BC12_DONE);
		sc8960x_auto_bc12_en(sc, false);
        }
    }

    if (!sc->power_good && true_power_good) {
        cancel_delayed_work_sync(&sc->bc12_timeout_dwork);
        sc8960x_set_iindpm(sc, 500);
        mutex_lock(&sc->bc_detect_lock);
        sc->bc12_detect = false;
        sc->bc12_recovery = false;
        mutex_unlock(&sc->bc_detect_lock);
        sc8960x_set_vindpm(sc, sc->vindpm_value);
        //sc8960x_get_charger_type(sc);
	sc8960x_auto_bc12_en(sc, false);
    }
    sc->power_good = true_power_good;
    power_supply_changed(sc->psy);

    ret = sc8960x_field_read(sc, BOOST_FAULT, &reg_val);
    if(reg_val) {
        dev_err(sc->dev, "Buck otg FAULT\n");
        schedule_delayed_work(&sc->otg_retry_work, msecs_to_jiffies(2000));
    }
    sc8960x_dump_register(sc);
    return IRQ_HANDLED;
}


/**********************system*********************/
static int sc8960x_parse_dt(struct sc8960x_chip *sc)
{
    struct device_node *np = sc->client->dev.of_node;

    int ret, i;
    const struct {
        const char *name;
        u32 *val;
    } param_data[] = {
        {"sc8960x,pfm-dis",           &(sc->sc8960x_param.pfm_dis)},
        {"sc8960x,vsys-min",          &(sc->sc8960x_param.vsys_min)},
        {"sc8960x,vbat-low-otg",      &(sc->sc8960x_param.vbat_low_otg)},
        {"sc8960x,ipre-chg",          &(sc->sc8960x_param.pre_chg_curr)},
        {"sc8960x,itermination",      &(sc->sc8960x_param.term_chg_curr)},
        {"sc8960x,vbat-volt",         &(sc->sc8960x_param.vbat_cv)},
        {"sc8960x,top-off-timer",     &(sc->sc8960x_param.top_off_timer)},
        {"sc8960x,vrechg-volt",       &(sc->sc8960x_param.vrechg_volt)},
        {"sc8960x,en-termination",    &(sc->sc8960x_param.term_en)},
        {"sc8960x,wdt-timer",         &(sc->sc8960x_param.wdt_timer)},
        {"sc8960x,en-chg-saft-timer", &(sc->sc8960x_param.en_chg_saft_timer)},
        {"sc8960x,charge-timer",      &(sc->sc8960x_param.charge_timer)},
        {"sc8960x,vac-ovp",           &(sc->sc8960x_param.vac_ovp)},
        {"sc8960x,iboost",            &(sc->sc8960x_param.iboost)},
        {"sc8960x,vboost",            &(sc->sc8960x_param.vboost)},
        {"sc8960x,vindpm-track",      &(sc->sc8960x_param.vindpm_track)},
        {"sc8960x,vindpm-int-mask",   &(sc->sc8960x_param.vindpm_int_mask)},
        {"sc8960x,iindpm-int-mask",   &(sc->sc8960x_param.iindpm_int_mask)},
        {"sc8960x,vpre-to-cc",        &(sc->sc8960x_param.vpre_to_cc)},
        {"sc8960x,auto-dpdm-en",      &(sc->sc8960x_param.auto_dpdm_en)},
        {"sc8960x,ntc-pin-dis",       &(sc->sc8960x_param.ntc_pin_dis)},
	{"sc8960x,support-otgcc-2A",  &(sc->sc8960x_param.support_otgcc_2A)},
        
    };
    if(of_property_read_string(np, "sc8960x,charger_name", &sc->chg_dev_name)) {
        sc->chg_dev_name = "primary_chg";
        dev_err(sc->dev, "no charger name\n");
    }

    for (i = 0;i < ARRAY_SIZE(param_data);i++) {
        ret = of_property_read_u32(np, param_data[i].name, param_data[i].val);
        if (ret < 0) {
            dev_err(sc->dev, "not find property %s\n", param_data[i].name);
            return ret;
        } else {
            dev_info(sc->dev,"%s: %d\n", param_data[i].name,
                        (int)*param_data[i].val);
        }
    }

    sc->irq_gpio = of_get_named_gpio(np, "sc8960x,intr-gpio", 0);
    if (!gpio_is_valid(sc->irq_gpio)) {
        dev_err(sc->dev, "fail to valid gpio : %d\n", sc->irq_gpio);
        return -EINVAL;
    }

    return ret;
}

static int sc8960x_hw_init(struct sc8960x_chip *sc)
{
    int ret, i = 0;
    const struct {
        enum sc8960x_fields field;
        int val;
    } param_init[] = {
        {PFM_DIS,         sc->sc8960x_param.pfm_dis},
        {VSYS_MIN,        sc->sc8960x_param.vsys_min},
        {VBATLOW_OTG,     sc->sc8960x_param.vbat_low_otg},
        {ITC,             sc->sc8960x_param.pre_chg_curr},
        {ITERM,           sc->sc8960x_param.term_chg_curr},
        {TOP_OFF_TIMER,   sc->sc8960x_param.top_off_timer},
        {VRECHG,          sc->sc8960x_param.vrechg_volt},
        {EN_TERM,         sc->sc8960x_param.term_en},
        {TWD,             sc->sc8960x_param.wdt_timer},
        {EN_TIMER,        sc->sc8960x_param.en_chg_saft_timer},
        {CHG_TIMER,       sc->sc8960x_param.charge_timer},
        {VAC_OVP,         sc->sc8960x_param.vac_ovp},
        {BOOST_LIM,       sc->sc8960x_param.iboost},
        {VINDPM_TRACK,    sc->sc8960x_param.vindpm_track},
        {VINDPM_INT_MASK, sc->sc8960x_param.vindpm_int_mask},
        {IINDPM_INT_MASK, sc->sc8960x_param.iindpm_int_mask},
        {VTC,             sc->sc8960x_param.vpre_to_cc},
        {AUTO_DPDM_EN,    sc->sc8960x_param.auto_dpdm_en},
        {NTC_DIS,         sc->sc8960x_param.ntc_pin_dis},
    };

    for (i = 0;i < ARRAY_SIZE(param_init);i++) {
        ret = sc8960x_field_write(sc, param_init[i].field, param_init[i].val);
        if (ret) {
            dev_err(sc->dev, "write field %d failed\n", param_init[i].field);
            return ret;
        }
    }

    //set vbat cv
    ret = sc8960x_field_write(sc, VBAT_REG, sc->sc8960x_param.vbat_cv >> 2);
    ret |= sc8960x_field_write(sc, VBAT_REG_FT, sc->sc8960x_param.vbat_cv & 0x03);
    if (ret) {
        dev_err(sc->dev, "set vbat cv failed\n");
        return ret;
    }

    //set vboost
    ret = sc8960x_field_write(sc, BOOSTV0, (sc->sc8960x_param.vboost & 0x06) >> 1);
    ret |= sc8960x_field_write(sc, BOOSTV1,
        ((sc->sc8960x_param.vboost & 0x08) >> 2) | (sc->sc8960x_param.vboost & 0x01));
    if (ret) {
        dev_err(sc->dev, "set vboost failed\n");
        return ret;
    }

#ifdef CONFIG_NONSTANDARD_DETECT
    ret = sc8960x_set_nonstandard_detect(sc);
    if (ret) {
       dev_err(sc->dev, "set private failed\n");
    }
#endif
    sc8960x_enable_hvdcp(sc, true);
    ret = sc8960x_auto_bc12_en(sc, false);
    if (ret) {
	pr_err("disale auto bc12 fail, ret = %d\n", ret);
    }

    return ret;
}

static int sc8960x_irq_register(struct sc8960x_chip *sc)
{
    int ret;

    ret = gpio_request_one(sc->irq_gpio, GPIOF_DIR_IN, "sc8960x_irq");
    if (ret) {
        dev_err(sc->dev, "failed to request %d\n", sc->irq_gpio);
        return -EINVAL;
    }

    sc->irq = gpio_to_irq(sc->irq_gpio);
    if (sc->irq < 0) {
        dev_err(sc->dev, "failed to gpio_to_irq\n");
        return -EINVAL;
    }

    ret = devm_request_threaded_irq(sc->dev, sc->irq, NULL,
                    sc8960x_irq_handler,
                    IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
                    "sc_irq", sc);
    if (ret < 0) {
        dev_err(sc->dev, "request thread irq failed:%d\n", ret);
        return ret;
    }

    enable_irq_wake(sc->irq);
    device_init_wakeup(sc->dev, true);

    return ret;
}


static void determine_initial_status(struct sc8960x_chip *sc)
{
    sc8960x_irq_handler(sc->irq, (void *)sc);
}

static ssize_t sc8960x_show_registers(struct device *dev, 
            struct device_attribute *attr,
            char *buf)
{
    struct sc8960x_chip *sc = dev_get_drvdata(dev);
    u8 tmpbuf[256];
    int addr, val, len, ret, idx = 0;

    idx = snprintf(buf, PAGE_SIZE, "%s:\n", "sc8960x Reg");
    for (addr = 0x0; addr <= SC8960X_REGMAX; addr++) {
        ret = regmap_read(sc->regmap, addr, &val);
        if (ret == 0) {
            len = snprintf(tmpbuf, PAGE_SIZE - idx,
                    "Reg[%.2x] = 0x%.2x\n", addr, val);
            memcpy(&buf[idx], tmpbuf, len);
            idx += len;
        }
    }

    return idx;
}

static ssize_t sc8960x_store_registers(struct device *dev,
            struct device_attribute *attr, 
            const char *buf,
            size_t count)
{
    struct sc8960x_chip *sc = dev_get_drvdata(dev);
    int ret;
    int reg, val;

    ret = sscanf(buf, "%x %x", &reg, &val);
    if (ret == 2 && reg < SC8960X_REGMAX) {
        regmap_write(sc->regmap, reg, val);
    }

    return count;
}

static DEVICE_ATTR(registers, 0660, sc8960x_show_registers, sc8960x_store_registers);

static void sc8960x_create_device_node(struct device *dev)
{
    device_create_file(dev, &dev_attr_registers);
}


static enum power_supply_usb_type sc8960x_chg_psy_usb_types[] = {
    POWER_SUPPLY_USB_TYPE_UNKNOWN,
    POWER_SUPPLY_USB_TYPE_SDP,
    POWER_SUPPLY_USB_TYPE_CDP,
    POWER_SUPPLY_USB_TYPE_DCP,
    POWER_SUPPLY_USB_TYPE_ACA,
};

static enum power_supply_property sc8960x_chg_psy_properties[] = {
    POWER_SUPPLY_PROP_MANUFACTURER,
    POWER_SUPPLY_PROP_ONLINE,
    POWER_SUPPLY_PROP_STATUS,
    POWER_SUPPLY_PROP_TYPE,
    POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT,
    POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE,
    POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT,
    POWER_SUPPLY_PROP_INPUT_VOLTAGE_LIMIT,
    POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT,
    POWER_SUPPLY_PROP_USB_TYPE,
    POWER_SUPPLY_PROP_CURRENT_MAX,
    POWER_SUPPLY_PROP_VOLTAGE_MAX,
};

static int sc8960x_chg_property_is_writeable(struct power_supply *psy,
                        enum power_supply_property psp)
{
    switch (psp) {
    case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT:
    case POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE:
    case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT:
    case POWER_SUPPLY_PROP_INPUT_VOLTAGE_LIMIT:
    case POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT:
    case POWER_SUPPLY_PROP_STATUS:
    case POWER_SUPPLY_PROP_ONLINE:
    case POWER_SUPPLY_PROP_ENERGY_EMPTY:
        return 1;
    default:
        return 0;
    }
    return 0;
}

static int sc8960x_chg_get_property(struct power_supply *psy,
                enum power_supply_property psp,
                union power_supply_propval *val)
{
    int ret = 0;
    int data = 0;
    struct sc8960x_chip *sc = power_supply_get_drvdata(psy);

    switch (psp) {
    case POWER_SUPPLY_PROP_MANUFACTURER:
        val->strval = "SouthChip";
        break;
    case POWER_SUPPLY_PROP_ONLINE:
        val->intval = sc->true_online;
        if (sc->typec_attach) {
            val->intval = 1;
        }
        break;
    case POWER_SUPPLY_PROP_STATUS:
        ret = sc8960x_get_charge_stat(sc);
        if (ret < 0)
            break;
        val->intval = ret;
        break;
    case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT:
        ret = sc8960x_get_ichg(sc, &data);
        if (ret)
            break;
        val->intval = data * 1000;
        break;
    case POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE:
        ret = sc8960x_get_vbat(sc, &data);
        if (ret < 0)
            break;
        val->intval = data * 1000;
        break;
    case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT:
        ret = sc8960x_get_iindpm(sc, &data);
        if (ret < 0)
            break;
        val->intval = data * 1000;
        break;
    case POWER_SUPPLY_PROP_INPUT_VOLTAGE_LIMIT:
        ret = sc8960x_get_vindpm(sc, &data);
        if (ret < 0)
            break;
        val->intval = data * 1000;
        break;
    case POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT:
        ret = sc8960x_get_term_curr(sc, &data);
        if (ret < 0)
            break;
        val->intval = data * 1000;
        break;
    case POWER_SUPPLY_PROP_USB_TYPE:
        val->intval = sc->psy_usb_type;
        break;
    case POWER_SUPPLY_PROP_CURRENT_MAX:
        if (sc->chg_type == POWER_SUPPLY_TYPE_USB)
            val->intval = 500000;
        else
            val->intval = 1500000;
        break;
    case POWER_SUPPLY_PROP_VOLTAGE_MAX:
        if (sc->chg_type == POWER_SUPPLY_TYPE_USB)
            val->intval = 5000000;
        else
            val->intval = 12000000;
        break;
    case POWER_SUPPLY_PROP_TYPE:
        val->intval = sc->chg_type;
        break;
    default:
        ret = -EINVAL;
        break;
    }
    return ret;
}

static int sc8960x_chg_set_property(struct power_supply *psy,
                enum power_supply_property psp,
                const union power_supply_propval *val)
{
    int ret = 0;
    struct sc8960x_chip *sc = power_supply_get_drvdata(psy);

    switch (psp) {
    case POWER_SUPPLY_PROP_ONLINE:
        // dev_info(sc->dev, "%s  %d\n", __func__, val->intval);
        // if (val->intval == 2) {
        //     schedule_delayed_work(&sc->force_detect_dwork, msecs_to_jiffies(300));
        // } else if (val->intval == 4) {
        //     sc->psy_desc.type = POWER_SUPPLY_TYPE_USB;
        //     sc->psy_usb_type = POWER_SUPPLY_USB_TYPE_SDP;
        // }
        //需要修改pd 通知出pr swap
        if (val->intval == 7) {
            sc->pr_swap = true;
        } else {
            sc->pr_swap = false;
        }
        sc->typec_attach = val->intval;
        break;
    case POWER_SUPPLY_PROP_STATUS:
        ret = sc8960x_set_chg_enable(sc, !!val->intval);
        break;
    case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT:
        ret = sc8960x_set_ichg(sc, val->intval / 1000);
        break;
    case POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE:
        ret = sc8960x_set_vbat(sc, val->intval / 1000);
        break;
    case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT:
        ret = sc8960x_set_iindpm(sc, val->intval / 1000);
        break;
    case POWER_SUPPLY_PROP_INPUT_VOLTAGE_LIMIT:
        ret = sc8960x_set_vindpm(sc, val->intval / 1000);
        break;
    case POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT:
        ret = sc8960x_set_term_curr(sc, val->intval / 1000);
        break;
    default:
        ret = -EINVAL;
        break;
    }
    return ret;
}

static char *sc8960x_psy_supplied_to[] = {
    "battery",
    "mtk-master-charger",
};

static const struct power_supply_desc sc8960x_psy_desc = {
    .name = "primary_chg",
    .type = POWER_SUPPLY_TYPE_UNKNOWN,
    .usb_types = sc8960x_chg_psy_usb_types,
    .num_usb_types = ARRAY_SIZE(sc8960x_chg_psy_usb_types),
    .properties = sc8960x_chg_psy_properties,
    .num_properties = ARRAY_SIZE(sc8960x_chg_psy_properties),
    .property_is_writeable = sc8960x_chg_property_is_writeable,
    .get_property = sc8960x_chg_get_property,
    .set_property = sc8960x_chg_set_property,
};

static int sc8960x_psy_register(struct sc8960x_chip *sc)
{
    struct power_supply_config cfg = {
        .drv_data = sc,
        .of_node = sc->dev->of_node,
        .supplied_to = sc8960x_psy_supplied_to,
        .num_supplicants = ARRAY_SIZE(sc8960x_psy_supplied_to),
    };

    memcpy(&sc->psy_desc, &sc8960x_psy_desc, sizeof(sc->psy_desc));
    sc->psy = devm_power_supply_register(sc->dev, &sc->psy_desc,
                        &cfg);
    return IS_ERR(sc->psy) ? PTR_ERR(sc->psy) : 0;
}

static struct of_device_id sc8960x_charger_match_table[] = {
    { .compatible = "southchip,sc89601d", },
    { .compatible = "southchip,sc89601s", },
    { .compatible = "southchip,sc89619d", },
    { },
};
MODULE_DEVICE_TABLE(of, sc8960x_charger_match_table);

static int sc8960x_set_shipmode_delay(struct sc8960x_chip *sc, int use_delay)
{
	int ret = 0;

	if (use_delay) {
		ret = sc8960x_field_write(sc, BATFET_DLY, 1);
		if (ret) {
			pr_err("%s failed to set delay\n", __func__);
			return ret;
		}
	}

	ret = sc8960x_field_write(sc, BATFET_DIS, 1);
	if (ret) {
		pr_err("%s failed to set delay\n", __func__);
	}

	return ret;
}

static int sc_set_shipmode(struct notifier_block *reboot_notifier,
			   unsigned long mode, void *nouse)
{
	struct sc8960x_chip *sc = container_of(
		reboot_notifier, struct sc8960x_chip, reboot_notifier);

	if (sc->ship_mode) {
		dev_notice(sc->dev, "successfully set shipmode\n");
		sc8960x_set_shipmode_delay(sc, 0);
	}

	return 0;
}

static void otg_retry_workfunc(struct work_struct *work)
{
    struct sc8960x_chip *sc = container_of(work,
                    struct sc8960x_chip, otg_retry_work.work);
    int boost_reg, ret;
    ret = sc8960x_field_read(sc, OTG_CFG, &boost_reg);
    if (boost_reg != sc->except_otg_flag)
    {
        dev_info(sc->dev, "reset otg %d\n", sc->except_otg_flag);
        sc8960x_set_otg_enable(sc, sc->except_otg_flag);
    }
}

static int sc8960x_charger_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct sc8960x_chip *sc;
    int i, ret = 0;

    dev_info(&client->dev, "%s (%s)\n", __func__, SC8960X_DRV_VERSION);

    sc = devm_kzalloc(&client->dev, sizeof(struct sc8960x_chip), GFP_KERNEL);
    if (!sc)
        return -ENOMEM;

    sc->dev = &client->dev;
    sc->client = client;
    sc->regmap = devm_regmap_init(sc->dev, &sc8960x_regmap_bus, sc,
                                    &sc8960x_regmap_config);
    if (IS_ERR(sc->regmap)) {
        dev_err(sc->dev, "Failed to initialize regmap\n");
        return -EINVAL;
    }

    for (i = 0; i < ARRAY_SIZE(sc8960x_reg_fields); i++) {
        const struct reg_field *reg_fields = sc8960x_reg_fields;
        sc->rmap_fields[i] = devm_regmap_field_alloc(sc->dev,
                                                sc->regmap,
                                                reg_fields[i]);
        if (IS_ERR(sc->rmap_fields[i])) {
            dev_err(sc->dev, "cannot allocate regmap field\n");
            return PTR_ERR(sc->rmap_fields[i]);
        }
    }

    ret = sc8960x_detect_device(sc);
    if (ret) {
        dev_err(sc->dev, "No sc8960x device found!\n");
        goto err_detect_dev;
    }

    if (sc->part_no != SC89601D_ID) {
        dev_err(sc->dev,"not find device, part_no = 0x%02x\n",sc->part_no);
        goto err_detect_dev;
    }

    sc->chan_vbus = devm_iio_channel_get(sc->dev, "pmic_vbus");
    if (IS_ERR_OR_NULL(sc->chan_vbus)) {
        dev_err(sc->dev, "sc8960x Failed get pmic_vbus!\n");
    }

    i2c_set_clientdata(client, sc);
    sc8960x_create_device_node(&(client->dev));

    INIT_DELAYED_WORK(&sc->otg_retry_work, otg_retry_workfunc);
    INIT_DELAYED_WORK(&sc->force_detect_dwork, sc8960x_force_detection_dwork_handler);
    INIT_DELAYED_WORK(&sc->bc12_timeout_dwork,
                        sc8960x_bc12_timeout_dwork_handler);
    INIT_DELAYED_WORK(&sc->hiz_cut_dwork,
                        sc8960x_hiz_cut_dwork_handler);
    INIT_DELAYED_WORK(&sc->td_dwork, sc8960x_td_test_work_func);
    mutex_init(&sc->bc_detect_lock);
    sc->bc12_detect = false;
    sc->bc12_recovery = false;
    sc->hiz_cut_flag = false;
 
    ret = sc8960x_parse_dt(sc);
    if (ret) {
        dev_err(sc->dev, "%s parse dt failed(%d)\n", __func__, ret);
        goto err_parse_dt;
    }

    ret = sc8960x_hw_init(sc);
    if (ret) {
        dev_err(sc->dev, "%s failed to init device(%d)\n", __func__, ret);
        goto err_init_device;
    }

    ret = sc8960x_psy_register(sc);
    if (ret) {
        dev_err(sc->dev, "%s psy register failed(%d)\n", __func__, ret);
        goto err_psy_register;
    }

    ret = sc8960x_irq_register(sc);
    if (ret) {
        dev_err(sc->dev, "%s irq register failed(%d)\n", __func__, ret);
        goto err_irq_register;
    }

#if IS_ENABLED(CONFIG_MTK_CHARGER)
    sc->chg_dev = charger_device_register(sc->chg_dev_name,
                        &client->dev, sc,
                        &sc8960x_chg_ops,
                        &sc8960x_chg_props);
    if (IS_ERR_OR_NULL(sc->chg_dev)) {
        ret = PTR_ERR(sc->chg_dev);
        devm_kfree(&client->dev, sc);
        return ret;
    }
#endif /*CONFIG_MTK_CLASS*/

    determine_initial_status(sc);

    sc->reboot_notifier.notifier_call = sc_set_shipmode;
    sc->reboot_notifier.priority = 255;
    register_restart_handler(&sc->reboot_notifier);

    dev_info(sc->dev, "sc8960x probe successfully, Part Num:%d, Revision:%d\n!",
        sc->part_no, sc->dev_version);

    return ret;

err_irq_register:
err_psy_register:
err_parse_dt:
err_init_device:
err_detect_dev:
    dev_info(sc->dev, "sc8960x prob failed\n");
    devm_kfree(&client->dev, sc);
    return -ENODEV;
}

static int sc8960x_charger_remove(struct i2c_client *client)
{
    struct sc8960x_chip *sc = i2c_get_clientdata(client);

    sc8960x_reg_reset(sc);
    disable_irq(sc->irq);
  
    return 0;
}

#ifdef CONFIG_PM_SLEEP
static int sc8960x_suspend(struct device *dev)
{
    struct sc8960x_chip *sc = dev_get_drvdata(dev);

    dev_info(dev, "%s\n", __func__);

    if (device_may_wakeup(dev))
        enable_irq_wake(sc->client->irq);

    disable_irq(sc->client->irq);

    return 0;
}

static int sc8960x_resume(struct device *dev)
{
    struct sc8960x_chip *sc = dev_get_drvdata(dev);

    dev_info(dev, "%s\n", __func__);

    enable_irq(sc->client->irq);
    if (device_may_wakeup(dev))
        disable_irq_wake(sc->client->irq);

    return 0;
}

static SIMPLE_DEV_PM_OPS(sc8960x_pm_ops, sc8960x_suspend, sc8960x_resume);
#endif /*CONFIG_PM_SLEEP*/

static void sc8960x_charger_shutdown(struct i2c_client *client)
{
	struct sc8960x_chip *sc = i2c_get_clientdata(client);
	int ret;

	if (IS_ERR_OR_NULL(sc)) {
		pr_err("get sc8960x_chip faild\n");
		return;
	}

	ret = sc8960x_set_otg_enable(sc, false);
	pr_err("%s: dis_otg: %d\n", __func__, ret);

	return;
}

static struct i2c_driver sc8960x_charger_driver = {
    .driver = {
        .name = "sc8960x-charger",
        .owner = THIS_MODULE,
        .of_match_table = sc8960x_charger_match_table,
#ifdef CONFIG_PM_SLEEP
        .pm = &sc8960x_pm_ops,
#endif /*CONFIG_PM_SLEEP*/
    },

    .probe = sc8960x_charger_probe,
    .remove = sc8960x_charger_remove,
    .shutdown = sc8960x_charger_shutdown,
};

module_i2c_driver(sc8960x_charger_driver);

MODULE_AUTHOR("SouthChip<Aiden-yu@southchip.com>");
MODULE_DESCRIPTION("SC8960x Charger Driver");
MODULE_LICENSE("GPL v2");

