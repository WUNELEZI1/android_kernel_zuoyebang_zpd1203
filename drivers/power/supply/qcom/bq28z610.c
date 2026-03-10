/*
 * bq28z610 fuel gauge driver
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/param.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <linux/regmap.h>
#include <linux/random.h>
#include <linux/ktime.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include "bq28z610.h"
#include <linux/pmic-voter.h>
#include "charger_partition.h"
#include "lc_notify.h"

enum product_name {
	XAGA_NO,
	XAGA,
	XAGAPRO,
	DAUMIER,
};

static int vbus_ctrl_gpio = -EINVAL;
static int log_level = 1;
static int product_name = XAGA_NO;
static ktime_t time_init = -1;

#define pr_fg(fmt) "%s: " fmt, __func__

#define fg_err(fmt, ...)                                        \
	do {                                                    \
		if (log_level >= 0)                             \
			printk(KERN_ERR pr_fg(fmt), ##__VA_ARGS__); \
	} while (0)

#define fg_info(fmt, ...)                                       \
	do {                                                    \
		if (log_level >= 1)                             \
			printk(KERN_ERR pr_fg(fmt), ##__VA_ARGS__); \
	} while (0)

#define fg_dbg(fmt, ...)                                        \
	do {                                                    \
		if (log_level >= 2)                             \
			printk(KERN_ERR pr_fg(fmt), ##__VA_ARGS__); \
	} while (0)

static struct regmap_config fg_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = 0xFF,
};

static int calculate_delta_time_for_i2c(ktime_t time_last, int *delta_time)
{
	ktime_t time_now;

	time_now = ktime_get();

	*delta_time = ktime_ms_delta(time_now, time_last);
	if (*delta_time < 0)
		*delta_time = 0;

	fg_dbg("%s: now:%ld, last:%ld, delta:%d\n", __func__, time_now, time_last,
	       *delta_time);

	return 0;
}

static int __fg_read_byte(struct i2c_client *client, u8 reg, u8 *val)
{
	int ret = 0;

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		fg_info("i2c read byte failed: can't read from reg 0x%02X faild\n",
			reg);
		return ret;
	}

	*val = (u8)ret;

	return 0;
}

static int fg_read_byte(struct bq_fg_chip *bq, u8 reg, u8 *val)
{
	int ret;
	mutex_lock(&bq->i2c_rw_lock);
	ret = __fg_read_byte(bq->client, reg, val);
	mutex_unlock(&bq->i2c_rw_lock);

	return ret;
}

static int fg_read_word(struct bq_fg_chip *bq, u8 reg, u16 *val)
{
	//u8 data[2] = { 0, 0 };
	int ret = 0;

	if (atomic_read(&bq->fg_in_sleep)) {
		fg_err("%s in sleep\n", __func__);
		return -EINVAL;
	}

	//ret = regmap_raw_read(bq->regmap, reg, data, 2);
	mutex_lock(&bq->i2c_rw_lock);
	ret = i2c_smbus_read_word_data(bq->client, reg);
	mutex_unlock(&bq->i2c_rw_lock);
	if (ret < 0) {
		fg_info("%s I2C failed to read 0x%02x, ret:%d \n", bq->log_tag,
			reg, ret);
		return ret;
	} else
		*val = (u16)ret;
	return 0;
}

static int fg_read_block(struct bq_fg_chip *bq, u8 reg, u8 *buf, u8 len)
{
	int ret = 0, i = 0;
	unsigned int data = 0;

	if (atomic_read(&bq->fg_in_sleep)) {
		fg_err("%s in sleep\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < len; i++) {
		ret = regmap_read(bq->regmap, reg + i, &data);
		if (ret) {
			fg_info("%s I2C failed to read 0x%02x\n", bq->log_tag,
				reg + i);
			return ret;
		}
		buf[i] = data;
	}

	return ret;
}

static int fg_write_block(struct bq_fg_chip *bq, u8 reg, u8 *data, u8 len)
{
	int ret = 0, i = 0;

	if (atomic_read(&bq->fg_in_sleep)) {
		fg_err("%s in sleep\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < len; i++) {
		ret = regmap_write(bq->regmap, reg + i, (unsigned int)data[i]);
		if (ret) {
			fg_err("%s I2C failed to write 0x%02x\n", bq->log_tag,
			       reg + i);
			return ret;
		}
	}

	return ret;
}

static u8 fg_checksum(u8 *data, u8 len)
{
	u8 i;
	u16 sum = 0;

	for (i = 0; i < len; i++) {
		sum += data[i];
	}

	sum &= 0xFF;

	return 0xFF - sum;
}

static int fg_mac_read_block(struct bq_fg_chip *bq, u16 cmd, u8 *buf, u8 len)
{
	int ret;
	u8 cksum_calc, cksum;
	u8 t_buf[40];
	u8 t_len;
	int i;

	mutex_lock(&bq->i2c_rw_lock);
	t_buf[0] = (u8)cmd;
	t_buf[1] = (u8)(cmd >> 8);
	ret = fg_write_block(bq, bq->regs[BQ_FG_REG_ALT_MAC], t_buf, 2);
	if (ret < 0) {
		mutex_unlock(&bq->i2c_rw_lock);
		return ret;
	}

	msleep(4);

	ret = fg_read_block(bq, bq->regs[BQ_FG_REG_ALT_MAC], t_buf, 36);
	if (ret < 0) {
		mutex_unlock(&bq->i2c_rw_lock);
		return ret;
	}

	cksum = t_buf[34];
	t_len = t_buf[35];

	if(t_len <= 3){
		fg_err("%s len is less invaild, force vaild\n", bq->log_tag);
		t_len = 3;
	}

	if(t_len >= 42){
		fg_err("%s len is over invaild, force vaild\n", bq->log_tag);
		t_len = 42;
	}

	cksum_calc = fg_checksum(t_buf, t_len - 2);
	if (cksum_calc != cksum) {
		fg_err("%s failed to checksum\n", bq->log_tag);
		mutex_unlock(&bq->i2c_rw_lock);
		return 1;
	}

	for (i = 0; i < len; i++)
		buf[i] = t_buf[i + 2];

	mutex_unlock(&bq->i2c_rw_lock);
	return 0;
}

static int fg_mac_write_block(struct bq_fg_chip *bq, u16 cmd, u8 *data, u8 len)
{
	int ret;
	u8 cksum;
	u8 t_buf[40];
	int i;

	mutex_lock(&bq->i2c_rw_lock);
	if (len > 32)
		return -1;

	t_buf[0] = (u8)cmd;
	t_buf[1] = (u8)(cmd >> 8);
	for (i = 0; i < len; i++)
		t_buf[i + 2] = data[i];

	/*write command/addr, data*/
	ret = fg_write_block(bq, bq->regs[BQ_FG_REG_ALT_MAC], t_buf, len + 2);
	if (ret < 0) {
		fg_err("%s failed to write block\n", bq->log_tag);
		mutex_unlock(&bq->i2c_rw_lock);
		return ret;
	}

	cksum = fg_checksum(t_buf, len + 2);
	t_buf[0] = cksum;
	t_buf[1] = len + 4; /*buf length, cmd, CRC and len byte itself*/
	/*write checksum and length*/
	ret = fg_write_block(bq, bq->regs[BQ_FG_REG_MAC_CHKSUM], t_buf, 2);

	mutex_unlock(&bq->i2c_rw_lock);
	return ret;
}

static int fg_sha256_auth(struct bq_fg_chip *bq, u8 *challenge, int length)
{
	int ret = 0;
	u8 cksum_calc = 0, data[2] = { 0 };

	/*
	1. The host writes 0x00 to 0x3E.
	2. The host writes 0x00 to 0x3F
	*/
	mutex_lock(&bq->i2c_rw_lock);
	data[0] = 0x00;
	data[1] = 0x00;
	ret = fg_write_block(bq, bq->regs[BQ_FG_REG_ALT_MAC], data, 2);
	if (ret < 0) {
		mutex_unlock(&bq->i2c_rw_lock);
		return ret;
	}
	/*
	3. Write the random challenge should be written in a 32-byte block to address 0x40-0x5F
	*/
	msleep(2);

	ret = fg_write_block(bq, bq->regs[BQ_FG_REG_MAC_DATA], challenge,
			     length);
	if (ret < 0) {
		mutex_unlock(&bq->i2c_rw_lock);
		return ret;
	}

	/*4. Write the checksum (2’s complement sum of (1), (2), and (3)) to address 0x60.*/
	cksum_calc = fg_checksum(challenge, length);
	ret = regmap_write(bq->regmap, bq->regs[BQ_FG_REG_MAC_CHKSUM],
			   cksum_calc);
	if (ret < 0) {
		mutex_unlock(&bq->i2c_rw_lock);
		return ret;
	}

	/*5. Write the length to address 0x61.*/
	ret = regmap_write(bq->regmap, bq->regs[BQ_FG_REG_MAC_DATA_LEN],
			   length + 4);
	if (ret < 0) {
		mutex_unlock(&bq->i2c_rw_lock);
		return ret;
	}

	msleep(300);

	ret = fg_read_block(bq, bq->regs[BQ_FG_REG_MAC_DATA], bq->digest,
			    length);
	if (ret < 0) {
		mutex_unlock(&bq->i2c_rw_lock);
		return ret;
	}

	mutex_unlock(&bq->i2c_rw_lock);
	return 0;
}

static int fg_read_status(struct bq_fg_chip *bq)
{
	u16 flags = 0;
	int ret = 0;

	ret = fg_read_word(bq, bq->regs[BQ_FG_REG_BATT_STATUS], &flags);
	if (ret < 0) {
		pr_err("%s read battery status fail, recover_batt_fc: %d\n", __func__, bq->recover.batt_fc);
		bq->batt_fc = bq->recover.batt_fc;
		return ret;
	} else {
		bq->batt_fc = !!(flags & BIT(5));
		bq->recover.batt_fc = bq->batt_fc;
	}
	return 0;
}

#define RSOC_READ_INTERVAL_MIN_MS 100
static int fg_read_rsoc(struct bq_fg_chip *bq)
{
	u16 soc = 0;
	int ret = 0;
	int change_delta = 0;
	static ktime_t last_change_time;

	if (bq->i2c_err_flag) {
		return 15;
	}

	last_change_time = bq->last_rt.rsoc_read_time;
	calculate_delta_time_for_i2c(last_change_time, &change_delta);
	if(change_delta <= RSOC_READ_INTERVAL_MIN_MS){
		fg_dbg("%s read rsoc time interval is less than %d ms, use last rsoc: %d\n", bq->log_tag, RSOC_READ_INTERVAL_MIN_MS, bq->recover.rsoc);
		soc = bq->recover.rsoc;
		goto out;
	}

	ret = fg_read_word(bq, bq->regs[BQ_FG_REG_SOC], &soc);
	if (ret < 0) {
		fg_err("%s failed to read RSOC, recover_rsoc: %d\n", bq->log_tag, bq->recover.rsoc);
		soc = bq->recover.rsoc;
		goto out;
	}
	bq->last_rt.rsoc_read_time = ktime_get();
	bq->recover.rsoc = soc;
out:
	return soc;
}

#define TEMP_READ_INTERVAL_MIN_MS 500
static int fg_read_temperature(struct bq_fg_chip *bq)
{
	u16 tbat = 0;
	int ret = 0;
	int temp = 0;
	int change_delta = 0;
	static ktime_t last_change_time;

	last_change_time = bq->last_rt.temp_read_time;
	calculate_delta_time_for_i2c(last_change_time, &change_delta);
	if(change_delta <= TEMP_READ_INTERVAL_MIN_MS){
		fg_dbg("%s read temp time interval is less than %d ms, use last temp: %d\n", bq->log_tag, TEMP_READ_INTERVAL_MIN_MS, bq->recover.temp);
		temp = bq->recover.temp;
		goto out;
	}

	if (bq->fake_tbat != -999){
		temp = bq->fake_tbat;
		goto out;
	}

	ret = fg_read_word(bq, bq->regs[BQ_FG_REG_TEMP], &tbat);
	if (ret < 0) {
		fg_err("%s failed to read TBAT, recover_temp: %d\n", bq->log_tag, bq->recover.temp);
		temp = bq->recover.temp;
		goto out;
	} else {
		temp = tbat - 2730;
		if (temp >= 1000) {
			pr_err("abnormal temp = %d,return recover_temp = %d\n",
				temp, bq->recover.temp);
			temp = bq->recover.temp;
		} else {
			bq->recover.temp = temp;
                }
	}
	bq->last_rt.temp_read_time = ktime_get();
	fg_dbg("%s read FG TBAT = %d\n", bq->log_tag, temp);
out:
	return temp;
}

static int fg_read_fg1_df_check(struct bq_fg_chip *bq)
{
	u8 data[32] = {0};
	int ret = 0;

	ret = fg_mac_read_block(bq, FG_MAC_CMD_FG1_DF_CHECK, data, 32);
	if (ret < 0) {
		fg_err("%s failed to read fg1_df_check\n", bq->log_tag);
		return -EINVAL;
	}

	fg_info("%s read fg1_df_check = %x\n", bq->log_tag, data[0]);
	return data[0];
}

static int fg_read_fg1_chemid(struct bq_fg_chip *bq)
{
	u8 data[32] = {0};
	int ret = 0;

	ret = fg_mac_read_block(bq, FG_MAC_CMD_FG1_CHEMID, data, 32);
	if (ret < 0) {
		fg_err("%s failed to read fg1_chemid\n", bq->log_tag);
		return -EINVAL;
	}

	fg_info("%s read fg1_chemid = %x\n", bq->log_tag, data[0]);
	return data[0];
}

static int fg_read_pack_vendor(struct bq_fg_chip *bq)
{
	u8 data[32] = {0};
	int ret = 0, pack_vendor = 0;

	ret = fg_mac_read_block(bq, FG_MAC_CMD_DEVICE_CHEM, data, 32);
	if (ret < 0) {
		fg_err("%s failed to read pack_vendor\n", bq->log_tag);
		return -EINVAL;
	}

	fg_info("%s read pack_vendor = %x\n", bq->log_tag, data);
	for(int i = 0; i <= 3; i++)
		pack_vendor |= data[i] << i*8;
	return pack_vendor;
}

static void fg_read_cell_voltage(struct bq_fg_chip *bq)
{
	u8 data[64] = { 0 };
	int ret = 0;

	ret = fg_mac_read_block(bq, FG_MAC_CMD_DASTATUS1, data, 32);
	if (ret) {
		fg_err("%s failed to read cell voltage, recover_cell_voltage: %d\n", bq->log_tag, bq->recover.cell_voltage);
		bq->cell_voltage[0] = bq->recover.cell_voltage;
		bq->cell_voltage[1] = bq->recover.cell_voltage;
		bq->cell_voltage[2] = bq->recover.cell_voltage;
		return;
	} else {
		bq->cell_voltage[0] = (data[1] << 8) | data[0];
		bq->cell_voltage[1] = (data[3] << 8) | data[2];
	}

	bq->cell_voltage[2] = 2 * max(bq->cell_voltage[0], bq->cell_voltage[1]);
	bq->recover.cell_voltage = bq->cell_voltage[2];
}

#define VOLT_READ_INTERVAL_MIN_MS 150
static int fg_read_volt(struct bq_fg_chip *bq)
{
	u16 vbat = 0;
	int ret = 0;
	int change_delta = 0;
	static ktime_t last_change_time;

	last_change_time = bq->last_rt.vbat_read_time;
	calculate_delta_time_for_i2c(last_change_time, &change_delta);
	if(change_delta <= VOLT_READ_INTERVAL_MIN_MS){
		fg_dbg("%s read volt interval is less than %d ms, use last vbat: %d\n", bq->log_tag, VOLT_READ_INTERVAL_MIN_MS, bq->recover.vbat);
		bq->vbat = bq->recover.vbat;
		goto out;
	}


	if (bq->i2c_err_flag) {
		bq->vbat = bq->recover.vbat;
		pr_err("%s i2c err, vbat: %d\n", bq->log_tag, bq->vbat);
		goto out;
	}

	ret = fg_read_word(bq, bq->regs[BQ_FG_REG_VOLT], &vbat);
	if (ret < 0) {
		fg_err("%s failed to read VBAT, recover_vbat: %d\n", bq->log_tag, bq->recover.vbat);
		bq->vbat = bq->recover.vbat;
		goto out;
	}
	bq->last_rt.vbat_read_time = ktime_get();
	bq->vbat = (int)vbat;
	bq->recover.vbat = bq->vbat;

	if (bq->device_name == BQ_FG_BQ28Z610)
		fg_read_cell_voltage(bq);
	else
		bq->cell_voltage[0] = bq->cell_voltage[1] =
			bq->cell_voltage[2] = bq->vbat;

out:
	return ret;
}
#if 0
#if IS_ENABLED(CONFIG_XM_FG_I2C_ERR)
static int fuelguage_check_i2c_function(struct fuel_gauge_dev *fuel_gauge)
{
	struct bq_fg_chip *bq = dev_get_drvdata(fuel_gauge);
	int ret = 0;

	ret = fg_read_volt(bq);
	return ret;
}
#endif
#endif
static int fg_read_avg_current(struct bq_fg_chip *bq)
{
	s16 avg_ibat = 0;
	int ret = 0;

	ret = fg_read_word(bq, bq->regs[BQ_FG_REG_AI], (u16 *)&avg_ibat);
	if (ret < 0) {
		fg_err("%s failed to read AVG_IBAT, recover_avg_curr: %d\n", bq->log_tag, bq->recover.avg_curr);
		avg_ibat = bq->recover.avg_curr;
		goto out;
	}

	avg_ibat = -1 * avg_ibat;
	bq->recover.avg_curr = avg_ibat;

out:
	return avg_ibat;
}

#define CURR_READ_INTERVAL_MIN_MS 60
static int curr_read_intval_ms = 60;
module_param_named(curr_read_intval_ms, curr_read_intval_ms, int, 0600);

static int fg_read_current(struct bq_fg_chip *bq)
{
	s16 ibat = 0;
	int ret = 0;
	int change_delta = 0;
	static ktime_t last_change_time;

	last_change_time = bq->last_rt.ibat_read_time;
	calculate_delta_time_for_i2c(last_change_time, &change_delta);
	if(change_delta <= curr_read_intval_ms){
		fg_dbg("%s read ibat interval is less than %d ms, use last ibat: %d\n", bq->log_tag, curr_read_intval_ms, bq->recover.curr);
		ibat = bq->recover.curr;
		goto out;
	}

	ret = fg_read_word(bq, bq->regs[BQ_FG_REG_CN], (u16 *)&ibat);
	if (ret < 0) {
		fg_err("%s failed to read IBAT, recover_avg_curr: %d\n", bq->log_tag, bq->recover.curr);
		ibat = bq->recover.curr;
		goto out;
	}

	fg_dbg("%s successed to read IBAT:%d\n", bq->log_tag, ibat);

	bq->last_rt.ibat_read_time = ktime_get();
	bq->recover.curr = ibat;

out:
	return ibat;
}

static int fg_read_fcc(struct bq_fg_chip *bq)
{
	u16 fcc = 0;
	int ret = 0;

	ret = fg_read_word(bq, bq->regs[BQ_FG_REG_FCC], &fcc);
	if (ret < 0) {
#ifdef CONFIG_TARGET_PRODUCT_PEARL
		fcc = 5160;
#else
		fcc = bq->recover.fcc;
#endif
		bq->fcc = fcc;
		fg_err("%s failed to read FCC,  recover_fcc: %d\n", bq->log_tag, bq->fcc);
		return fcc;
	}

	bq->fcc = fcc;
	bq->recover.fcc = bq->fcc;
	return fcc;
}

static int fg_read_rm(struct bq_fg_chip *bq)
{
	u16 rm = 0;
	int ret = 0;

	ret = fg_read_word(bq, bq->regs[BQ_FG_REG_RM], &rm);
	if (ret < 0) {
#ifdef CONFIG_TARGET_PRODUCT_PEARL
		rm = 718;
#else
		rm = bq->recover.rm;
#endif
		bq->rm = rm;
		fg_err("%s failed to read RM, recover_rm: %d\n", bq->log_tag, bq->rm);
		return rm;
	}
	bq->rm = rm;
	bq->recover.rm = bq->rm;
	return rm;
}

static int fg_read_dc(struct bq_fg_chip *bq)
{
	u16 dc = 0;
	int ret = 0;

	ret = fg_read_word(bq, bq->regs[BQ_FG_REG_DC], &dc);
	if (ret < 0) {
		dc = bq->recover.dc;
		fg_err("%s failed to read DC, recover_dc: %d\n", bq->log_tag, dc);
		return dc;
	}

	bq->recover.dc = dc;
	return dc;
}

static int fg_read_soh(struct bq_fg_chip *bq)
{
	u16 soh = 0;
	int ret = 0;

	ret = fg_read_word(bq, bq->regs[BQ_FG_REG_SOH], &soh);
	if (ret < 0) {
		soh = bq->recover.soh;
		fg_err("%s failed to read SOH, recover_soh: %d\n", bq->log_tag, soh);
		return soh;
	}

	bq->recover.soh = soh;
	return soh;
}

#if 0
static int fg_read_cv(struct bq_fg_chip *bq)
{
	u16 cv = 0;
	bool retry = false;
	int ret = 0;

retry:
	ret = fg_read_word(bq, bq->regs[BQ_FG_REG_CHG_VOL], &cv);
	if (ret < 0) {
		if (!retry) {
			retry = true;
			msleep(10);
			goto retry;
		} else {
			fg_err("%s failed to read CV\n", bq->log_tag);
			cv = 4480;
		}
	}

	return cv;
}

static int fg_read_cc(struct bq_fg_chip *bq)
{
	u16 cc = 0;
	bool retry = false;
	int ret = 0;

retry:
	ret = fg_read_word(bq, bq->regs[BQ_FG_REG_CHG_CUR], &cc);
	if (ret < 0) {
		if (!retry) {
			retry = true;
			msleep(10);
			goto retry;
		} else {
			fg_err("%s failed to read CC\n", bq->log_tag);
			cc = 11000;
		}
	}

	return cc;
}
#endif

static int fg_read_cyclecount(struct bq_fg_chip *bq)
{
	u16 cc = 0;
	int ret = 0;

	if (bq->fake_cycle_count >= 0) {
		fg_info("%s fake_cycle_count: %d\n", bq->log_tag, bq->fake_cycle_count);
		return bq->fake_cycle_count;
	}

	ret = fg_read_word(bq, bq->regs[BQ_FG_REG_CC], &cc);
	if (ret < 0) {
		cc = bq->recover.cyclecount;
		fg_err("%s failed to read CC, recover_cyclecount: %d\n", bq->log_tag, cc);
		return cc;
	}

	bq->recover.cyclecount = cc;
	return cc;
}

static int fg_get_raw_soc(struct bq_fg_chip *bq)
{
	int raw_soc = 0;

	bq->rm = fg_read_rm(bq);
	bq->fcc = fg_read_fcc(bq);

	raw_soc = bq->rm * 10000 / bq->fcc;

	return raw_soc;
}

static int fg_get_soc_decimal_rate(struct bq_fg_chip *bq)
{
	int soc, i;
	if (bq->dec_rate_len <= 0)
		return 0;

	if (bq->i2c_err_flag)
		return 0;

	soc = fg_read_rsoc(bq);
	for (i = 0; i < bq->dec_rate_len; i += 2) {
		if (soc < bq->dec_rate_seq[i]) {
			return bq->dec_rate_seq[i - 1];
		}
	}
	return bq->dec_rate_seq[bq->dec_rate_len - 1];
}

static int fg_get_soc_decimal(struct bq_fg_chip *bq)
{
	int rsoc, raw_soc;
	if (!bq)
		return 0;

	if (bq->i2c_err_flag)
		return 0;

	rsoc = fg_read_rsoc(bq);
	raw_soc = fg_get_raw_soc(bq);
	if (bq->ui_soc > rsoc)
		return 0;
	return raw_soc % 100;
}

static void fg_read_qmax(struct bq_fg_chip *bq)
{
	u8 data[64] = { 0 };
	int ret = 0;

	if (bq->device_name == BQ_FG_BQ27Z561 ||
	    bq->device_name == BQ_FG_NFG1000A ||
	    bq->device_name == BQ_FG_NFG1000B) {
		ret = fg_mac_read_block(bq, FG_MAC_CMD_QMAX, data, 14);
		if (ret < 0)
			fg_err("%s failed to read MAC\n", bq->log_tag);
	} else if (bq->device_name == BQ_FG_BQ28Z610) {
		ret = fg_mac_read_block(bq, FG_MAC_CMD_QMAX, data, 20);
		if (ret < 0)
			fg_err("%s failed to read MAC\n", bq->log_tag);
	} else {
		fg_err("%s not support device name\n", bq->log_tag);
	}

	bq->qmax[0] = (data[1] << 8) | data[0];
	bq->qmax[1] = (data[3] << 8) | data[2];
}

static int fg_set_fastcharge_mode(struct bq_fg_chip *bq, bool enable)
{
	u8 data[5] = { 0 };
	int ret = 0;

	if (bq->fast_chg == enable)
		return ret;
	else
		data[0] = bq->fast_chg = enable;

	if (bq->device_name == BQ_FG_BQ28Z610)
		return ret;

	if (enable) {
		ret = fg_mac_write_block(bq, FG_MAC_CMD_FASTCHARGE_EN, data, 2);
		fg_info("%s write 3e fastcharge = %d success\n", bq->log_tag,
			ret);
		if (ret) {
			fg_err("%s failed to write fastcharge = %d\n",
			       bq->log_tag, ret);
			return ret;
		}
	} else {
		ret = fg_mac_write_block(bq, FG_MAC_CMD_FASTCHARGE_DIS, data,
					 2);
		fg_info("%s write 3f fastcharge = %d success\n", bq->log_tag,
			ret);
		if (ret) {
			fg_err("%s failed to write fastcharge = %d\n",
			       bq->log_tag, ret);
			return ret;
		}
	}

	return ret;
}

static int fg_set_charger_to_full(struct bq_fg_chip *bq)
{
	u8 data[5] = { 1 };
	int ret = 0;

	if (bq->device_name == BQ_FG_BQ28Z610)
		return ret;

	bq->rsoc_smooth = bq->raw_soc;

	ret = fg_mac_write_block(bq, FG_MAC_CMD_TO_FULL, data, 2);
	if (ret) {
		fg_err("%s failed to write 31 charger done to full = %d\n", bq->log_tag, ret);
		return ret;
	}
	fg_info("%s write 31 charger done to full = %d success\n", bq->log_tag, ret);

	bq->raw_soc = 10000;//R_SOC set 100%

	return ret;
}

#if 0
static int fuelguage_set_fastcharge_mode(struct fuel_gauge_dev *fuel_gauge, bool en)
{
	struct bq_fg_chip *bq = dev_get_drvdata(fuel_gauge);
	int ret = 0;

	ret = fg_set_fastcharge_mode(bq, en);
	return ret;
}

static int fuelguage_get_fastcharge_mode(struct fuel_gauge_dev *fuel_gauge)
{
	struct bq_fg_chip *bq = dev_get_drvdata(fuel_gauge);

	return bq->fast_chg;
}
#endif
static int calc_delta_time(ktime_t time_last, int *delta_time)
{
	ktime_t time_now;

	time_now = ktime_get();

	*delta_time = ktime_ms_delta(time_now, time_last);
	if (*delta_time < 0)
		*delta_time = 0;

	fg_dbg("%s: now:%ld, last:%ld, delta:%d\n", __func__, time_now, time_last,
	       *delta_time);

	return 0;
}

struct ffc_smooth {
	int curr_lim;
	int time;
};

struct ffc_smooth ffc_dischg_smooth[FFC_SMOOTH_LEN] = {
	{ 0, 50000 },
	{ 300, 20000 },
	{ 600, 15000 },
	{ 1000, 10000 },
};

static int bq_battery_soc_smooth_tracking_sencond(struct bq_fg_chip *bq,
						  int raw_soc, int batt_soc,
						  int soc)
{
	static ktime_t changed_time = -1;
	int unit_time = 0, delta_time = 0;
	int change_delta = 0;
	int soc_changed = 0;

	if (bq->tbat < 150) {
		bq->monitor_delay = FG_MONITOR_DELAY_5S;
	}
	if (raw_soc > bq->report_full_rsoc) {
		if (raw_soc == 10000 && bq->last_soc < 99) {
			unit_time = 20000;
			calc_delta_time(changed_time, &change_delta);
			if (delta_time < 0) {
				changed_time = ktime_get();
				delta_time = 0;
			}
			delta_time = change_delta / unit_time;
			soc_changed = min(1, delta_time);
			if (soc_changed) {
				soc = bq->last_soc + soc_changed;
				fg_info("%s soc increase changed = %d\n",
					bq->log_tag, soc_changed);
			} else {
				soc = bq->last_soc;
			}
		} else {
			soc = 100;
		}
	} else if (raw_soc > 990) {
		soc += bq->soc_gap;
		if (soc > 99)
			soc = 99;
	} else {
		if (raw_soc == 0 && bq->last_soc > 1) {
			bq->ffc_smooth = false;
			unit_time = 5000;
			calc_delta_time(changed_time, &change_delta);
			delta_time = change_delta / unit_time;
			if (delta_time < 0) {
				changed_time = ktime_get();
				delta_time = 0;
			}
			soc_changed = min(1, delta_time);
			if (soc_changed) {
				fg_info("%s soc reduce changed = %d\n",
					bq->log_tag, soc_changed);
				soc = bq->last_soc - soc_changed;
			} else
				soc = bq->last_soc;
		} else {
			soc = (raw_soc + 89) / 90;
		}
	}

	if (soc >= 100)
		soc = 100;
	if (soc < 0)
		soc = batt_soc;

	if (bq->last_soc <= 0)
		bq->last_soc = soc;
	if (bq->last_soc != soc) {
		if (abs(soc - bq->last_soc) > 1) {
			calc_delta_time(changed_time, &change_delta);
			delta_time = change_delta / LOW_TEMP_CHARGING_DELTA;
			if (delta_time < 0) {
				changed_time = ktime_get();
				delta_time = 0;
			}
			soc_changed = min(1, delta_time);
			if (soc_changed) {
				changed_time = ktime_get();
			}

			fg_info("avoid jump soc = %d last = %d soc_change = %d state = %d ,delta_time = %d\n",
				soc, bq->last_soc, soc_changed, bq->param.batt_status,
				change_delta);

			if (bq->param.batt_status == POWER_SUPPLY_STATUS_CHARGING) {
				if (soc > bq->last_soc) {
					soc = bq->last_soc + soc_changed;
					bq->last_soc = soc;
				} else {
					fg_info("Do not smooth waiting real soc increase here\n");
					soc = bq->last_soc;
				}
			} else if (bq->param.batt_status != POWER_SUPPLY_STATUS_FULL) {
				if (soc < bq->last_soc) {
					soc = bq->last_soc - soc_changed;
					bq->last_soc = soc;
				} else {
					fg_info("Do not smooth waiting real soc decrease here\n");
					soc = bq->last_soc;
				}
			}
		} else {
			changed_time = ktime_get();
			bq->last_soc = soc;
		}
	}
	return soc;
}

static int bq_battery_soc_smooth_tracking(struct bq_fg_chip *bq, int raw_soc,
					  int batt_soc, int batt_temp,
					  int batt_ma)
{
	static int last_batt_soc = -1, system_soc, cold_smooth;
	static int last_status;
	int change_delta = 0;
	int optimiz_delta = 0;
	static ktime_t last_change_time;
	static ktime_t last_optimiz_time;
	int unit_time = 0;
	int soc_changed = 0, delta_time = 0;
	static int optimiz_soc, last_raw_soc;
	int batt_ma_avg, i;

	if (bq->optimiz_soc > 0) {
		bq->ffc_smooth = true;
		last_batt_soc = bq->optimiz_soc;
		system_soc = bq->optimiz_soc;
		last_change_time = ktime_get();
		bq->optimiz_soc = 0;
	}

	if (last_batt_soc < 0)
		last_batt_soc = batt_soc;

	if (raw_soc == FG_RAW_SOC_FULL)
		bq->ffc_smooth = false;

	if (bq->ffc_smooth) {
		if (batt_soc == system_soc) {
			bq->ffc_smooth = false;
			return batt_soc;
		}
		if (bq->param.batt_status != last_status) {
			if (last_status == POWER_SUPPLY_STATUS_CHARGING &&
			    bq->param.batt_status == POWER_SUPPLY_STATUS_DISCHARGING)
				last_change_time = ktime_get();
			last_status = bq->param.batt_status;
		}
	}

	if (bq->fast_chg && raw_soc >= bq->report_full_rsoc &&
	    raw_soc != FG_RAW_SOC_FULL) {
		if (last_optimiz_time == 0)
			last_optimiz_time = ktime_get();
		calc_delta_time(last_optimiz_time, &optimiz_delta);
		delta_time = optimiz_delta / FG_OPTIMIZ_FULL_TIME;
		soc_changed = min(1, delta_time);
		if (raw_soc > last_raw_soc && soc_changed) {
			last_raw_soc = raw_soc;
			optimiz_soc += soc_changed;
			last_optimiz_time = ktime_get();
			fg_info("optimiz_soc:%d, last_optimiz_time%ld\n",
				optimiz_soc, last_optimiz_time);
			if (optimiz_soc > 100)
				optimiz_soc = 100;
			bq->ffc_smooth = true;
		}
		if (batt_soc > optimiz_soc) {
			optimiz_soc = batt_soc;
			last_optimiz_time = ktime_get();
		}
		if (bq->ffc_smooth)
			batt_soc = optimiz_soc;
		last_change_time = ktime_get();
	} else {
		optimiz_soc = batt_soc + 1;
		last_raw_soc = raw_soc;
		last_optimiz_time = ktime_get();
	}

	calc_delta_time(last_change_time, &change_delta);
	batt_ma_avg = fg_read_avg_current(bq);
	if (batt_temp > 150 /* BATT_COOL_THRESHOLD */ && !cold_smooth &&
	    batt_soc != 0) {
		if (bq->ffc_smooth &&
		    (bq->param.batt_status == POWER_SUPPLY_STATUS_DISCHARGING ||
		     bq->param.batt_status == POWER_SUPPLY_STATUS_NOT_CHARGING ||
		     batt_ma_avg > 50)) {
			for (i = 1; i < FFC_SMOOTH_LEN; i++) {
				if (batt_ma_avg <
				    ffc_dischg_smooth[i].curr_lim) {
					unit_time =
						ffc_dischg_smooth[i - 1].time;
					break;
				}
			}
			if (i == FFC_SMOOTH_LEN) {
				unit_time =
					ffc_dischg_smooth[FFC_SMOOTH_LEN - 1]
						.time;
			}
		}
	} else {
		/* Calculated average current > 1000mA */
		if (batt_ma_avg > BATT_HIGH_AVG_CURRENT)
			/* Heavy loading current, ignore battery soc limit*/
			unit_time = LOW_TEMP_CHARGING_DELTA;
		else
			unit_time = LOW_TEMP_DISCHARGING_DELTA;
		if (batt_soc != last_batt_soc)
			cold_smooth = true;
		else
			cold_smooth = false;
	}
	if (unit_time > 0) {
		delta_time = change_delta / unit_time;
		soc_changed = min(1, delta_time);
	} else {
		if (!bq->ffc_smooth)
			bq->update_now = true;
	}

	fg_info("batt_ma_avg:%d, batt_ma:%d, cold_smooth:%d, optimiz_soc:%d",
		batt_ma_avg, batt_ma, cold_smooth, optimiz_soc);
	fg_info("delta_time:%d, change_delta:%d, unit_time:%d"
		" soc_changed:%d, bq->update_now:%d, bq->ffc_smooth:%d,bq->fast_chg:%d",
		delta_time, change_delta, unit_time, soc_changed,
		bq->update_now, bq->ffc_smooth, bq->fast_chg);

	if (last_batt_soc < batt_soc && batt_ma < 0)
		/* Battery in charging status
		 * update the soc when resuming device
		 */
		last_batt_soc = bq->update_now ? batt_soc :
						 last_batt_soc + soc_changed;
	else if (last_batt_soc > batt_soc && batt_ma > 0) {
		/* Battery in discharging status
		 * update the soc when resuming device
		 */
		last_batt_soc = bq->update_now ? batt_soc :
						 last_batt_soc - soc_changed;
	}
	bq->update_now = false;

	if (system_soc != last_batt_soc) {
		system_soc = last_batt_soc;
		last_change_time = ktime_get();
	}

	fg_info("raw_soc:%d batt_soc:%d,last_batt_soc:%d,system_soc:%d",
		raw_soc, batt_soc, last_batt_soc, system_soc);

	return system_soc;
}

static int bq_battery_soc_smooth_tracking_new(struct bq_fg_chip *bq,
					      int raw_soc, int batt_soc,
					      int batt_ma)
{
	static int system_soc, last_system_soc;
	int soc_changed = 0, unit_time = 10000, delta_time = 0, soc_delta = 0;
	static ktime_t last_change_time = -1;
	int change_delta = 0;
	int i = 0, batt_ma_avg = 0;
	static int ibat_pos_count = 0;
	struct timespec64 time;
	ktime_t tmp_time = 0;
	tmp_time = ktime_get_boottime();
	time = ktime_to_timespec64(tmp_time);

	if ((batt_ma > 0) && (ibat_pos_count < 10))
		ibat_pos_count++;
	else if (batt_ma <= 0)
		ibat_pos_count = 0;

	bq->bq_charging_status = bq->param.batt_status;
	if (bq->tbat < 150) {
		bq->monitor_delay = FG_MONITOR_DELAY_3S;
	}
	if (!raw_soc) {
		bq->monitor_delay = FG_MONITOR_DELAY_10S;
	}

	batt_ma_avg = fg_read_avg_current(bq);
	if (batt_ma_avg <= -10 || bq->bq_charging_status == POWER_SUPPLY_STATUS_DISCHARGING)
		bq->rsoc_smooth = 0;
	if (bq->rsoc_smooth <= 9800 && bq->rsoc_smooth) {
		raw_soc = bq->rsoc_smooth;
		bq->rsoc_smooth = bq->rsoc_smooth + 100;
	} else {
		bq->rsoc_smooth = 0;
	}

	fg_dbg("raw_soc:%d, smooth_soc:%d \n", raw_soc, bq->rsoc_smooth);

	/*Map system_soc value according to raw_soc */
	if (raw_soc >= bq->report_full_rsoc)
		system_soc = 100;
	else if (bq->max_chg_power_120w || product_name == XAGAPRO) {
		system_soc = ((raw_soc + 94) / 95);
		if (system_soc > 99)
			system_soc = 99;
	} else {
		system_soc = ((raw_soc + 96) / 97);
		if (system_soc > 99)
			system_soc = 100;
	}
	fg_dbg("%s smooth_new: fisrt step, system_soc = %d\n", __func__,
		system_soc);
	/*Get the initial value for the first time */
	if (last_change_time == -1) {
		last_change_time = ktime_get();
		if (system_soc != 0){
			last_system_soc = system_soc;
		}else
			last_system_soc = batt_soc;
		fg_dbg("eu_mode:%d, last_system_soc:%d, system_soc:%d \n",
                       bq->is_eu_mode, last_system_soc, system_soc);
	}
	if ((bq->param.batt_status == POWER_SUPPLY_STATUS_DISCHARGING ||
	     bq->param.batt_status == POWER_SUPPLY_STATUS_NOT_CHARGING) &&
	    !bq->rm && bq->tbat < 150 && last_system_soc >= 1) {
		batt_ma_avg = fg_read_avg_current(bq);
		for (i = FFC_SMOOTH_LEN - 1; i >= 0; i--) {
			if (batt_ma_avg > ffc_dischg_smooth[i].curr_lim) {
				unit_time = ffc_dischg_smooth[i].time;
				break;
			}
		}
		fg_info("enter low temperature smooth unit_time=%d batt_ma_avg=%d\n",
			unit_time, batt_ma_avg);
	}

	/*If the soc jump, will smooth one cap every 10S */
	soc_delta = abs(system_soc - last_system_soc);
#ifndef CONFIG_TARGET_PRODUCT_XAGA
	if (soc_delta > 1 || (bq->vbat < 3300 && system_soc > 0) ||
	    (unit_time != 10000 && soc_delta == 1)) {
#else
	if (soc_delta > 1 || (bq->vbat < 3400 && system_soc > 0) ||
	    (unit_time != 10000 && soc_delta == 1)) {
#endif
		//unit_time != 10000 && soc_delta == 1 fix low temperature 2% jump to 0%
		calc_delta_time(last_change_time, &change_delta);
		delta_time = change_delta / unit_time;
		if (delta_time < 0) {
			last_change_time = ktime_get();
			delta_time = 0;
		}
		soc_changed = min(1, delta_time);
		if (soc_changed) {
			if (bq->param.batt_status == POWER_SUPPLY_STATUS_CHARGING) {
				if(system_soc > last_system_soc) {
					system_soc = last_system_soc + soc_changed;
				} else if (system_soc < last_system_soc) {
					system_soc = last_system_soc - soc_changed;
				}
			} else if (bq->param.batt_status == POWER_SUPPLY_STATUS_DISCHARGING && system_soc < last_system_soc) {
				system_soc = last_system_soc - soc_changed;
			}
		} else {
			system_soc = last_system_soc;
		}
		fg_dbg("fg jump smooth soc_changed=%d, system_soc = %d, last_system_soc = %d\n", soc_changed, system_soc, last_system_soc);
	}
	if (system_soc < last_system_soc)
		system_soc = last_system_soc - 1;
	/*Avoid mismatches between charging status and soc changes  */
	if ((bq->param.batt_status == POWER_SUPPLY_STATUS_DISCHARGING) && (system_soc > last_system_soc)) {
		system_soc = last_system_soc;
	}
	fg_info("smooth_new:sys_soc:%d last_sys_soc:%d soc_delta:%d charging_status:%d unit_time:%d batt_ma_avg=%d\n",
		system_soc, last_system_soc, soc_delta, bq->param.batt_status,
		unit_time, batt_ma_avg);

	if (system_soc != last_system_soc) {
		last_change_time = ktime_get();
		last_system_soc = system_soc;
	}
	if (system_soc > 100)
		system_soc = 100;
	if (system_soc < 0)
		system_soc = 0;

	if ((system_soc == 0) &&
	    ((bq->vbat >= 3400) || ((time.tv_sec <= 20)))) {
		system_soc = 1;
		fg_info("uisoc::hold 1 when volt > 3400mv. \n");
	}

	if (bq->last_soc != system_soc) {
		bq->last_soc = system_soc;
	}

	return system_soc;
}

static void calculate_average_current(struct bq_fg_chip *bq)
{
	int i;
	int iavg_ma = bq->param.batt_ma;
	/* only continue if ibat has changed */
	if (bq->param.batt_ma == bq->param.batt_ma_prev)
		goto unchanged;
	else
		bq->param.batt_ma_prev = bq->param.batt_ma;
	bq->param.batt_ma_avg_samples[bq->param.samples_index] = iavg_ma;
	bq->param.samples_index = (bq->param.samples_index + 1) % BATT_MA_AVG_SAMPLES;
	bq->param.samples_num++;
	if (bq->param.samples_num >= BATT_MA_AVG_SAMPLES)
		bq->param.samples_num = BATT_MA_AVG_SAMPLES;
	if (bq->param.samples_num) {
		iavg_ma = 0;
		/* maintain a AVG_SAMPLES sample average of ibat */
		for (i = 0; i < bq->param.samples_num; i++) {
			pr_debug("iavg_samples_ma[%d] = %d\n", i, bq->param.batt_ma_avg_samples[i]);
			iavg_ma += bq->param.batt_ma_avg_samples[i];
		}
		bq->param.batt_ma_avg = DIV_ROUND_CLOSEST(iavg_ma, bq->param.samples_num);
	}
unchanged:
	pr_info("current_now_ma = %d, averaged_iavg_ma = %d\n",
			bq->param.batt_ma, bq->param.batt_ma_avg);
}

static int calculate_delta_time(ktime_t *time_stamp, int *delta_time_s)
{
	ktime_t now_time;
	/* default to delta time = 0 if anything fails */
	*delta_time_s = 0;
	//now_time = ktime_get_seconds();
	now_time = ktime_get_boottime();
	*delta_time_s = ktime_ms_delta(now_time, *time_stamp) / 1000;
	if(*delta_time_s < 0)
		*delta_time_s = 0;
	/* remember this time */
	*time_stamp = now_time;
	return 0;
}

#define LOW_TBAT_THRESHOLD		150
#define CHANGE_SOC_TIME_LIMIT_20S	20
#define CHANGE_SOC_TIME_LIMIT_60S	60
#define HEAVY_DISCHARGE_CURRENT		1000
#define FORCE_TO_FULL_SOC		95
#define MIN_DISCHARGE_CURRENT		(-25)
#define MIN_CHARGING_CURRENT		25
#define FULL_SOC			100
#define FORCE_TO_FULL_RAW_SOC_EEA 	99
static int bq_battery_soc_smooth_tracking_eea(struct bq_fg_chip *bq)
{
	static ktime_t last_change_time;
	int delta_time = 0;
	int soc_changed;
	int last_batt_soc = bq->param.batt_soc;
	int time_since_last_change_sec = 0;
	static int raw_soc = 0;
	struct power_supply *bat_psy = NULL;

	bat_psy = power_supply_get_by_name("battery");
	if (IS_ERR_OR_NULL(bat_psy)) {
		pr_err("%s, get battery psy fail\n", __func__);
		bat_psy = power_supply_get_by_name("battery");
	}

	raw_soc = bq->param.batt_raw_soc * 100;
	last_change_time = bq->param.last_soc_change_time;
	calculate_delta_time(&last_change_time, &time_since_last_change_sec);

	if (bq->param.batt_temp > LOW_TBAT_THRESHOLD) {
		/* Battery in normal temperture */
		if (bq->param.batt_ma < 0 || abs(bq->param.batt_raw_soc - bq->param.batt_soc) > 2)
			delta_time = time_since_last_change_sec / CHANGE_SOC_TIME_LIMIT_20S;
		else
			delta_time = time_since_last_change_sec / CHANGE_SOC_TIME_LIMIT_60S;
	} else {
		/* Battery in low temperture */
		calculate_average_current(bq);
		/* Calculated average current > 1000mA */
		if (bq->param.batt_ma_avg > HEAVY_DISCHARGE_CURRENT || abs(bq->param.batt_raw_soc - bq->param.batt_soc > 2))
			/* Heavy loading current, ignore battery soc limit*/
			delta_time = time_since_last_change_sec / CHANGE_SOC_TIME_LIMIT_20S;
		else
			delta_time = time_since_last_change_sec / CHANGE_SOC_TIME_LIMIT_20S;
	}

	if (delta_time < 0)
		delta_time = 0;
	soc_changed = min(1, delta_time);
	if(raw_soc <= 10000)
		last_batt_soc = (raw_soc + 100) / 101;

	pr_info("smooth_new:soc:%d, last_soc:%d, raw_soc:%d, soc_changed:%d, update_now:%d, charge_status:%d, batt_ma:%d\n",
			bq->param.batt_soc, last_batt_soc, raw_soc, soc_changed, bq->param.update_now,
			bq->param.batt_status, bq->param.batt_ma);

	if (last_batt_soc >= 0){
		if(bq->param.batt_status == POWER_SUPPLY_STATUS_FULL){
			if(last_batt_soc != FULL_SOC && bq->param.batt_raw_soc >= FORCE_TO_FULL_RAW_SOC_EEA){
				/* Unlikely status */
				last_batt_soc = bq->param.update_now ? FULL_SOC : last_batt_soc + soc_changed;
				pr_debug("charge full, smooth soc to 100 \n");
			}

			if(last_batt_soc == FULL_SOC && bq->param.batt_raw_soc >= 100){
				/* Unlikely status */
				last_batt_soc = bq->param.update_now ? FULL_SOC : last_batt_soc;
				pr_debug("charge full, keep soc to 100 \n");
			}

			if(last_batt_soc > bq->param.batt_raw_soc){
				last_batt_soc = bq->param.update_now ? bq->param.batt_raw_soc : last_batt_soc - soc_changed;
				pr_debug("soc reduce with raw_soc before recharge\n");
			}

		}else if (bq->param.batt_status == POWER_SUPPLY_STATUS_CHARGING){
			if(last_batt_soc < bq->param.batt_raw_soc){
				/* Battery in charging status
				* update the soc when resuming device
				*/
				last_batt_soc = bq->param.update_now ? bq->param.batt_raw_soc : last_batt_soc + soc_changed;
			}else if(bq->param.batt_soc == 99 && bq->param.batt_raw_soc == 100){
				/* Battery in charging status
				* but cannot update soc to 100
				* keep soc in 99 untill charge termination
				*/
				last_batt_soc = bq->param.batt_soc;
			}
		}else{
			if(last_batt_soc > bq->param.batt_raw_soc){
				/* Battery in discharging status
				* update the soc when resuming device
				*/
				last_batt_soc = bq->param.update_now ? bq->param.batt_raw_soc : last_batt_soc - soc_changed;
			}
		}
		bq->param.update_now = false;
	}else{
		last_batt_soc = bq->param.batt_raw_soc;
	}

	if (last_batt_soc > FULL_SOC)
		last_batt_soc = FULL_SOC;
	else if (last_batt_soc < 0)
		last_batt_soc = 0;

	if (bq->first_flag == true && bq->param.batt_soc == 0 && last_batt_soc > 0) {
		bq->first_flag = false;
		bq->param.batt_soc = last_batt_soc;
		bq->param.last_soc_change_time = last_change_time;
		if (bat_psy)
			power_supply_changed(bat_psy);
	} else if (bq->param.batt_soc > last_batt_soc) {
		bq->param.batt_soc = bq->param.batt_soc - 1;
		bq->param.last_soc_change_time = last_change_time;
		pr_info("%s update down soc %d %d\n", __func__, bq->param.batt_soc, last_batt_soc);
		if (bat_psy)
			power_supply_changed(bat_psy);
	} else if (bq->param.batt_soc < last_batt_soc  &&
					(bq->param.batt_status == POWER_SUPPLY_STATUS_FULL ||
					bq->param.batt_status == POWER_SUPPLY_STATUS_CHARGING)) {
		pr_info("%s update up soc %d %d\n", __func__, bq->param.batt_soc, last_batt_soc);
		bq->param.batt_soc = bq->param.batt_soc + 1;
		bq->param.last_soc_change_time = last_change_time;
		if (bat_psy)
			power_supply_changed(bat_psy);
	}

	return 0;

}

static int fg_set_shutdown_mode(struct bq_fg_chip *bq)
{
	int ret = 0;
	u8 data[5] = { 0 };

	fg_info("%s fg_set_shutdown_mode\n", bq->log_tag);
	bq->shutdown_mode = true;

	data[0] = 1;

	ret = fg_mac_write_block(bq, FG_MAC_CMD_SHUTDOWN, data, 2);
	if (ret)
		fg_err("%s failed to send shutdown cmd 0\n", bq->log_tag);

	ret = fg_mac_write_block(bq, FG_MAC_CMD_SHUTDOWN, data, 2);
	if (ret)
		fg_err("%s failed to send shutdown cmd 1\n", bq->log_tag);

	return ret;
}

static bool battery_get_psy(struct bq_fg_chip *bq)
{
	bq->batt_psy = power_supply_get_by_name("battery");
	if (!bq->batt_psy) {
		fg_err("%s failed to get batt_psy", bq->log_tag);
		return false;
	}
	return true;
}

static void fg_update_status(struct bq_fg_chip *bq)
{
	int temp_soc = 0, delta_temp = 0;
	static int last_soc = 0, last_temp = 0;
	ktime_t time_now = -1;
	union power_supply_propval pval = {0,};
	int rc;

	if (bq->i2c_err_flag) {
		bq->cycle_count = 0;
		bq->rsoc = 15;
		bq->ui_soc = 15;
		bq->soh = 100;
		bq->ibat = 500;
		bq->tbat = 250;
		return;
	}

	mutex_lock(&bq->data_lock);
	bq->cycle_count = fg_read_cyclecount(bq);
	bq->rsoc = fg_read_rsoc(bq);
	bq->soh = fg_read_soh(bq);
	bq->raw_soc = fg_get_raw_soc(bq);
	bq->ibat = fg_read_current(bq);
	bq->tbat = fg_read_temperature(bq);
	fg_read_volt(bq);
	fg_read_status(bq);
	mutex_unlock(&bq->data_lock);
	fg_info("%s fg_update rsoc=%d, raw_soc=%d, vbat=%d, cycle_count=%d\n",
	       __func__, bq->rsoc, bq->raw_soc, bq->vbat, bq->cycle_count);

	bq->usb_psy = power_supply_get_by_name("usb");
	if (IS_ERR_OR_NULL(bq->usb_psy)) {
		fg_err("usb psy not found\n");
	} else {
		rc = power_supply_get_property(
			bq->usb_psy, POWER_SUPPLY_PROP_ONLINE, &pval);
		bq->usb_online = pval.intval;
	}

	time_now = ktime_get();
	if (time_init != -1 && (time_now - time_init < 10000)) {
		bq->ui_soc = bq->rsoc;
		goto out;
	}

	if (battery_get_psy(bq)) {
		rc = power_supply_get_property(
			bq->batt_psy, POWER_SUPPLY_PROP_STATUS, &pval);
		bq->param.batt_status = pval.intval;
	} else {
		fg_err("%s fg_update failed to get battery psy\n", bq->log_tag);
		bq->param.batt_status = POWER_SUPPLY_STATUS_DISCHARGING;
	}

	if(bq->is_eu_mode == true){
		bq->param.batt_ma = bq->ibat;
		bq->param.batt_temp = bq->tbat;
		bq->param.batt_raw_soc = bq->rsoc;
		bq_battery_soc_smooth_tracking_eea(bq);
		bq->ui_soc = bq->param.batt_soc;
	}else{
		bq->ui_soc = bq_battery_soc_smooth_tracking_new(
			bq, bq->raw_soc, bq->rsoc, bq->ibat);
	}

	goto out;
	temp_soc = bq_battery_soc_smooth_tracking(
		bq, bq->raw_soc, bq->rsoc, bq->tbat, bq->ibat);
	bq->ui_soc = bq_battery_soc_smooth_tracking_sencond(
		bq, bq->raw_soc, bq->rsoc, temp_soc);

out:
	fg_info("%s [FG_STATUS] [UISOC RSOC RAWSOC TEMP_SOC SOH] = [%d %d %d %d %d], [VBAT CELL0 CELL1 IBAT TBAT FC FAST_MODE] = [%d %d %d %d %d %d %d]\n",
		bq->log_tag, bq->ui_soc, bq->rsoc, bq->raw_soc,
		temp_soc, bq->soh, bq->vbat, bq->cell_voltage[0],
		bq->cell_voltage[1], bq->ibat, bq->tbat, bq->batt_fc,
		bq->fast_chg);

	delta_temp = abs(last_temp - bq->tbat);
	if (bq->batt_psy && (last_soc != bq->ui_soc || delta_temp > 5 ||
			     bq->ui_soc == 0 || bq->rsoc == 0)) {
		fg_info("%s last_soc = %d, last_temp = %d, delta_temp = %d\n",
		       __func__, last_soc, last_temp, delta_temp);
		power_supply_changed(bq->batt_psy);
	}

	last_soc = bq->ui_soc;
	if (delta_temp > 5)
		last_temp = bq->tbat;
}

static int get_count(struct bq_fg_chip *bq, int *count_1000)
{
	u8 buf[32] = {0};
	int ret = 0;
	int count_1, count_2, count_3;
	if(bq->dod_count>0){
		*count_1000 = bq->dod_count*1000;
		return ret;
	}
	// TODO: read count_1 count_2 count_3 from guage
	ret = fg_mac_read_block(bq, 0x2001, buf, 32);
	if(ret){
		msleep(150);
		ret = fg_mac_read_block(bq, 0x2001, buf, 32);
	}
	if(ret){
		return ret;
	}
	// get count1/2/3 from buf
	count_1 = buf[9]<<8 | buf[8];
	count_2 = buf[11]<<8 | buf[10];
	count_3 = buf[13]<<8 | buf[12];
	*count_1000 = (2000*count_1 + 1500*count_2 + 1300*count_3); //(2*count1+1.5*count2+1.3*count3)
	pr_debug("%s count_1=%d, count_2=%d, count_3=%d, count_1000=%d\n", __func__, count_1, count_2, count_3, *count_1000);

	return ret;
}

static int set_termv(struct bq_fg_chip *bq, int termv)
{
	int ret = -EINVAL;
	u8 data[5] = {0};
  	u8 read_back[2] = {0};
	u16 ter_mv = termv;
  	u16 read_mv = 0;

	if (bq->i2c_err_flag)
		return 0;

	data[0] = (u8)ter_mv;
	data[1] = (u8)(ter_mv>>8);
	// TODO: set termv to guage
	ret = fg_mac_write_block(bq, 0x0050, data, 2);
	if(ret){
		msleep(150);
		ret = fg_mac_write_block(bq, 0x0050, data, 2);
	}
	if(!ret){
          	msleep(150);
          	ret = fg_mac_read_block(bq, 0x0050, read_back, 2);
          	if(!ret){
                	read_mv = (read_back[1] << 8) | read_back[0];
					pr_info("%s set_termv=%d, read_mv=%d", __func__, ter_mv, read_mv);
                  	if(ter_mv != read_mv){
                        	pr_err("%s Termv written failed!", __func__);
                        } else{
                          	bq->termv = termv;
                        }
                } else{
                	pr_err("%s Failed to read back termv value!", __func__);
                }
	}
	return ret;
}

static void fg_select_poweroff_voltage_config(struct bq_fg_chip *bq)
{
	int ret;
	int count_x1000;
	int poweroff_vol, shutdown_delay_vol, termv;
  	static int report_count = 0;
	struct poweroff_voltage_config conf_count = {0};
	struct poweroff_voltage_config conf_cyclecount = {0};
	ret = get_count(bq, &count_x1000);
	if(!ret){
		select_poweroff_by_count(bq->tbat, count_x1000, &conf_count);
	}
	select_poweroff_by_cyclecount(bq->tbat, bq->cycle_count, &conf_cyclecount);
	// compare the voltage form count and cyclecount
	if (conf_count.shutdown_delay_voltage >= conf_cyclecount.shutdown_delay_voltage) {
		poweroff_vol = conf_count.poweroff_voltage;
		shutdown_delay_vol = conf_count.shutdown_delay_voltage;
		termv = conf_count.termv;
	} else {
		poweroff_vol = conf_cyclecount.poweroff_voltage;
		shutdown_delay_vol = conf_cyclecount.shutdown_delay_voltage;
		termv = conf_cyclecount.termv;
	}
	// set termv
	if(termv!=0 && termv!=bq->termv){
		ret = set_termv(bq, termv);
		if(ret){
			pr_err("%s set_termv fail, ret=%d!\n", __func__, termv);
		}
	}
	if(poweroff_vol!=0 && shutdown_delay_vol!=0) {
		if(bq->poweroff_conf.poweroff_voltage != poweroff_vol \
					|| bq->poweroff_conf.shutdown_delay_voltage != shutdown_delay_vol){
			if( (poweroff_vol>2500&&poweroff_vol<3500) \
						&& (shutdown_delay_vol>2900 && shutdown_delay_vol<3600) ){
				bq->poweroff_conf.poweroff_voltage = poweroff_vol;
				bq->poweroff_conf.shutdown_delay_voltage = shutdown_delay_vol;
				lc_charger_notifier_call_chain(CHARGER_EVENT_SHUTDOWN_VOLTAGE_CHANGED, (void *)&bq->poweroff_conf);
                          	report_count = 0;
				pr_info("notify poweroff config[%d,%d] \n", poweroff_vol, shutdown_delay_vol);
			}
		}
          	if(bq->poweroff_conf.poweroff_voltage == poweroff_vol \
					&& bq->poweroff_conf.shutdown_delay_voltage == shutdown_delay_vol){
                  	report_count++;
                  	if(report_count < 10){
                          	lc_charger_notifier_call_chain(CHARGER_EVENT_SHUTDOWN_VOLTAGE_CHANGED, (void *)&bq->poweroff_conf);
				pr_info("notify poweroff config[%d,%d] \n", poweroff_vol, shutdown_delay_vol);
                        }
                }
	}
}

static int dod_count_get(struct bq_fg_chip *gm,
	struct mtk_bms_sysfs_field_info *attr,
	int *val)
{
	if(gm && gm->dod_count > 0)
	{
		*val = gm->dod_count;
		return 0;
	}
	if (gm)
		*val = gm->dod_count;
	else
		*val = 0;
	pr_debug("%s %d\n", __func__, *val);
	return 0;
}

static int dod_count_set(struct bq_fg_chip *gm,
	struct mtk_bms_sysfs_field_info *attr,
	int val)
{
	if (gm)
		gm->dod_count = val;
	pr_debug("%s %d\n", __func__, val);
	return 0;
}

static int cis_alert_get(struct bq_fg_chip *gm,
	struct mtk_bms_sysfs_field_info *attr,
	int *val)
{
	if(gm){
		if(gm->fake_cis_alert > 0){
			*val = gm->fake_cis_alert;
			pr_info("%s use fake cis alert %d\n", __func__, *val);
		}else{
			*val = gm->cis_alert;
			pr_debug("%s %d\n", __func__, *val);
		}
	} else {
		*val = 0;
	}

	return 0;
}

static int cis_alert_set(struct bq_fg_chip *gm,
	struct mtk_bms_sysfs_field_info *attr,
	int val)
{
	if (gm)
		gm->fake_cis_alert = val;
	pr_debug("%s %d\n", __func__, val);
	return 0;
}

static void low_vbat_power_off(struct bq_fg_chip *bq)
{
	static int count = 0;
	int force_shutdown_volt = SW_LOW_BAT_UVLO_CONF_MV;

	if (!bq->batt_psy) {
		if (!battery_get_psy(bq)) {
			pr_err("%s %s failed to get battery psy\n", __func__, bq->log_tag);
			return;
		}
	}
	if (bq->usb_online)
		force_shutdown_volt = USB_ONLINE_LOW_BAT_CONF_MV;
	// check vbat<=2.9V 4 times
	do {
		if (bq->vbat <= force_shutdown_volt) {
			count++;
			msleep(1000);
			fg_read_volt(bq); // update vbat
			continue;
		} else {
			count = 0;
			break;
		}
	} while (count < 5);
	if (count > 4) {
		bq->batt_cap_level_critical = 1;
		power_supply_changed(bq->batt_psy);
		return;
	}
}

static void fg_monitor_workfunc(struct work_struct *work)
{
	struct bq_fg_chip *bq =
		container_of(work, struct bq_fg_chip, monitor_work.work);

	bq->is_eu_mode = get_eu_mode();
	pr_debug("%s: is_eu_mode:%d \n", __func__, bq->is_eu_mode);

	fg_update_status(bq);
	if (!bq->i2c_err_flag) {
		fg_select_poweroff_voltage_config(bq);
		low_vbat_power_off(bq);
	}

	schedule_delayed_work(&bq->monitor_work,
			      msecs_to_jiffies(bq->monitor_delay));
	if (bq->bms_wakelock->active)
		__pm_relax(bq->bms_wakelock);
}

static int fastcharge_mode_get(struct bq_fg_chip *gm,
			       struct mtk_bms_sysfs_field_info *attr, int *val)
{
	if (gm)
		*val = gm->fast_chg;
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int fastcharge_mode_set(struct bq_fg_chip *gm,
			       struct mtk_bms_sysfs_field_info *attr, int val)
{
	if (gm)
		fg_set_fastcharge_mode(gm, !!val);
	fg_dbg("%s %d\n", __func__, val);
	return 0;
}

static int monitor_delay_get(struct bq_fg_chip *gm,
			     struct mtk_bms_sysfs_field_info *attr, int *val)
{
	if (gm)
		*val = gm->monitor_delay;
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int monitor_delay_set(struct bq_fg_chip *gm,
			     struct mtk_bms_sysfs_field_info *attr, int val)
{
	if (gm)
		gm->monitor_delay = val;
	fg_dbg("%s %d\n", __func__, val);
	return 0;
}

static int fcc_get(struct bq_fg_chip *gm, struct mtk_bms_sysfs_field_info *attr,
		   int *val)
{
	if (gm)
		*val = gm->fcc;
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int rm_get(struct bq_fg_chip *gm, struct mtk_bms_sysfs_field_info *attr,
		  int *val)
{
	if (gm)
		*val = gm->rm;
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int rsoc_get(struct bq_fg_chip *gm,
		    struct mtk_bms_sysfs_field_info *attr, int *val)
{
	if (gm)
		*val = gm->rsoc;
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int shutdown_delay_get(struct bq_fg_chip *gm,
			      struct mtk_bms_sysfs_field_info *attr, int *val)
{
	if (gm)
		*val = gm->shutdown_delay;
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int shutdown_delay_set(struct bq_fg_chip *gm,
			      struct mtk_bms_sysfs_field_info *attr, int val)
{
	if (gm)
		gm->fake_shutdown_delay_enable = val;
	fg_dbg("%s %d\n", __func__, val);
	return 0;
}

static int capacity_raw_get(struct bq_fg_chip *gm,
			    struct mtk_bms_sysfs_field_info *attr, int *val)
{
	if (gm)
		*val = gm->raw_soc;
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int soc_decimal_get(struct bq_fg_chip *gm,
			   struct mtk_bms_sysfs_field_info *attr, int *val)
{
	if (gm)
		*val = fg_get_soc_decimal(gm);
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int av_current_get(struct bq_fg_chip *gm,
			  struct mtk_bms_sysfs_field_info *attr, int *val)
{
	if (gm)
		*val = fg_read_avg_current(gm);
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int soc_decimal_rate_get(struct bq_fg_chip *gm,
				struct mtk_bms_sysfs_field_info *attr, int *val)
{
	if (gm)
		*val = fg_get_soc_decimal_rate(gm);
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int resistance_id_get(struct bq_fg_chip *gm,
			     struct mtk_bms_sysfs_field_info *attr, int *val)
{
	if (gm)
		*val = 100000;
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int resistance_get(struct bq_fg_chip *gm,
			  struct mtk_bms_sysfs_field_info *attr, int *val)
{
	if (gm)
		*val = 100000;
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int authentic_get(struct bq_fg_chip *gm,
			 struct mtk_bms_sysfs_field_info *attr, int *val)
{
	if (gm)
		*val = gm->authenticate;
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int authentic_set(struct bq_fg_chip *gm,
			 struct mtk_bms_sysfs_field_info *attr, int val)
{
	if (gm)
		gm->authenticate = !!val;
	fg_dbg("%s %d\n", __func__, val);
	return 0;
}

static int shutdown_mode_get(struct bq_fg_chip *gm,
			     struct mtk_bms_sysfs_field_info *attr, int *val)
{
	if (gm)
		*val = gm->authenticate;
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int shutdown_mode_set(struct bq_fg_chip *gm,
			     struct mtk_bms_sysfs_field_info *attr, int val)
{
	if (gm)
		fg_set_shutdown_mode(gm);
	fg_dbg("%s %d\n", __func__, val);
	return 0;
}

static int chip_ok_get(struct bq_fg_chip *gm,
		       struct mtk_bms_sysfs_field_info *attr, int *val)
{
	if (gm)
		*val = gm->chip_ok;
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int charge_done_get(struct bq_fg_chip *gm,
			   struct mtk_bms_sysfs_field_info *attr, int *val)
{
	if (gm)
		*val = gm->batt_fc;
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int soh_get(struct bq_fg_chip *gm, struct mtk_bms_sysfs_field_info *attr,
		   int *val)
{
	if (gm)
		*val = gm->soh;
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int bms_slave_connect_error_get(struct bq_fg_chip *gm,
				       struct mtk_bms_sysfs_field_info *attr,
				       int *val)
{
	*val = gpio_get_value(gm->slave_connect_gpio);
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int cell_supplier_get(struct bq_fg_chip *gm,
			     struct mtk_bms_sysfs_field_info *attr, int *val)
{
	if (gm)
		*val = gm->cell_supplier;
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int i2c_error_count_get(struct bq_fg_chip *gm,
			       struct mtk_bms_sysfs_field_info *attr, int *val)
{
	if (gm && gm->fake_i2c_error_count > 0) {
		*val = gm->fake_i2c_error_count;
		return 0;
	}
	if (gm)
		*val = gm->i2c_error_count;
	else
		*val = 0;
	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int i2c_error_count_set(struct bq_fg_chip *gm,
			       struct mtk_bms_sysfs_field_info *attr, int val)
{
	if (gm)
		gm->fake_i2c_error_count = val;
	fg_dbg("%s %d\n", __func__, val);
	return 0;
}

static ssize_t bms_sysfs_store(struct device *dev,
			       struct device_attribute *attr, const char *buf,
			       size_t count)
{
	struct power_supply *psy;
	struct bq_fg_chip *gm;
	struct mtk_bms_sysfs_field_info *usb_attr;
	int val;
	ssize_t ret;

	ret = kstrtos32(buf, 0, &val);
	if (ret < 0)
		return ret;

	psy = dev_get_drvdata(dev);
	gm = (struct bq_fg_chip *)power_supply_get_drvdata(psy);

	usb_attr = container_of(attr, struct mtk_bms_sysfs_field_info, attr);
	if (usb_attr->set != NULL)
		usb_attr->set(gm, usb_attr, val);

	return count;
}

static ssize_t bms_sysfs_show(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	struct power_supply *psy;
	struct bq_fg_chip *gm;
	struct mtk_bms_sysfs_field_info *usb_attr;
	int val = 0;
	ssize_t count;

	psy = dev_get_drvdata(dev);
	gm = (struct bq_fg_chip *)power_supply_get_drvdata(psy);

	usb_attr = container_of(attr, struct mtk_bms_sysfs_field_info, attr);
	if (usb_attr->get != NULL)
		usb_attr->get(gm, usb_attr, &val);

	count = scnprintf(buf, PAGE_SIZE, "%d\n", val);
	return count;
}

static int temp_max_get(struct bq_fg_chip *gm,
			struct mtk_bms_sysfs_field_info *attr, int *val)
{
	char data_limetime1[32];
	int ret = 0;

	memset(data_limetime1, 0, sizeof(data_limetime1));

	ret = fg_mac_read_block(gm, FG_MAC_CMD_LIFETIME1, data_limetime1,
				sizeof(data_limetime1));
	if (ret)
		fg_err("failed to get FG_MAC_CMD_LIFETIME1\n");
	*val = data_limetime1[6];

	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

static int time_ot_get(struct bq_fg_chip *gm,
		       struct mtk_bms_sysfs_field_info *attr, int *val)
{
	char data_limetime3[32];
	char data[32];
	int ret = 0;

	memset(data_limetime3, 0, sizeof(data_limetime3));
	memset(data, 0, sizeof(data));

	ret = fg_mac_read_block(gm, FG_MAC_CMD_LIFETIME3, data_limetime3,
				sizeof(data_limetime3));
	if (ret)
		fg_err("failed to get FG_MAC_CMD_LIFETIME3\n");

	ret = fg_mac_read_block(gm, FG_MAC_CMD_MANU_NAME, data, sizeof(data));
	if (ret)
		fg_err("failed to get FG_MAC_CMD_MANU_NAME\n");

	if (data[2] == 'C') //TI
	{
		ret = fg_mac_read_block(gm, FG_MAC_CMD_FW_VER, data,
					sizeof(data));
		if (ret)
			fg_err("failed to get FG_MAC_CMD_FW_VER\n");

		if ((data[3] == 0x0) && (data[4] == 0x1)) //R0 FW
			*val = ((data_limetime3[15] << 8) |
				(data_limetime3[14] << 0))
			       << 2;
		else if ((data[3] == 0x1) && (data[4] == 0x2)) //R1 FW
			*val = ((data_limetime3[9] << 8) |
				(data_limetime3[8] << 0))
			       << 2;
	} else if (data[2] == '4') //NVT
		*val = (data_limetime3[15] << 8) | (data_limetime3[14] << 0);

	fg_dbg("%s %d\n", __func__, *val);
	return 0;
}

int cis_alert_level_get(struct bq_fg_chip *bq)
{
	int ret = 0;
	u8 isc_alert_level = 0;

	if (bq->device_name != BQ_FG_NFG1000A &&
	    bq->device_name != BQ_FG_NFG1000B) {
		fg_err("%s: this Bq_Fg is not support this function.\n",
		       __func__);
		return -1;
	}

	if(bq->fake_cis_alert != 0){
		fg_err("%s fake_cis_alert:%d \n", bq->log_tag, bq->fake_cis_alert);
		return bq->fake_cis_alert;
	}

	ret = fg_read_byte(bq, bq->regs[NVT_FG_REG_ISC], &isc_alert_level);
	if (ret < 0) {
		fg_err("%s: read isc_alert_level occur error.\n", __func__);
		isc_alert_level = 0;
	}else{
		fg_err("%s: cis_alert_level:%d.\n", __func__, isc_alert_level);
	}

	return isc_alert_level;
}

int soa_alert_level_get(struct bq_fg_chip *gm,
			struct mtk_bms_sysfs_field_info *attr, int *val)
{
	int ret = 0;
	u8 soa_alert_level = 0;

	if (gm->device_name != BQ_FG_NFG1000A &&
	    gm->device_name != BQ_FG_NFG1000B) {
		fg_err("%s: this Bq_Fg is not support this function.\n",
		       __func__);
		return -1;
	}

	ret = fg_read_byte(gm, gm->regs[NVT_FG_REG_SOA_L], &soa_alert_level);

	if (ret < 0) {
		fg_err("%s: read soa_alert_level occur error.\n", __func__);
		return ret;
	}
	*val = soa_alert_level;
	fg_dbg("%s:now soa:%d.\n", __func__, *val);
	return ret;
}

int charging_state_get(struct bq_fg_chip *gm,
		       struct mtk_bms_sysfs_field_info *attr, int *val)
{
	int ret = 0;
	u16 charging_state = 0;

	ret = fg_read_word(gm, FG_MAC_CMD_CHARGING_STATUS, &charging_state);
	if (ret < 0)
		fg_err("%s failed to read MAC\n", __func__);

	*val = !!(charging_state & BIT(3));
	fg_dbg("%s:charging_state:%d.\n", __func__, *val);
	return ret;
}

/* Must be in the same order as BMS_PROP_* */
static struct mtk_bms_sysfs_field_info bms_sysfs_field_tbl[] = {
	BMS_SYSFS_FIELD_RW(fastcharge_mode, BMS_PROP_FASTCHARGE_MODE),
	BMS_SYSFS_FIELD_RW(monitor_delay, BMS_PROP_MONITOR_DELAY),
	BMS_SYSFS_FIELD_RO(fcc, BMS_PROP_FCC),
	BMS_SYSFS_FIELD_RO(rm, BMS_PROP_RM),
	BMS_SYSFS_FIELD_RO(rsoc, BMS_PROP_RSOC),
	BMS_SYSFS_FIELD_RW(shutdown_delay, BMS_PROP_SHUTDOWN_DELAY),
	BMS_SYSFS_FIELD_RO(capacity_raw, BMS_PROP_CAPACITY_RAW),
	BMS_SYSFS_FIELD_RO(soc_decimal, BMS_PROP_SOC_DECIMAL),
	BMS_SYSFS_FIELD_RO(soc_decimal_rate, BMS_PROP_SOC_DECIMAL_RATE),
	BMS_SYSFS_FIELD_RO(resistance_id, BMS_PROP_RESISTANCE_ID),
	BMS_SYSFS_FIELD_RW(authentic, BMS_PROP_AUTHENTIC),
	BMS_SYSFS_FIELD_RW(shutdown_mode, BMS_PROP_SHUTDOWN_MODE),
	BMS_SYSFS_FIELD_RO(chip_ok, BMS_PROP_CHIP_OK),
	BMS_SYSFS_FIELD_RO(charge_done, BMS_PROP_CHARGE_DONE),
	BMS_SYSFS_FIELD_RO(soh, BMS_PROP_SOH),
	BMS_SYSFS_FIELD_RO(resistance, BMS_PROP_RESISTANCE),
	BMS_SYSFS_FIELD_RW(i2c_error_count, BMS_PROP_I2C_ERROR_COUNT),
	BMS_SYSFS_FIELD_RO(av_current, BMS_PROP_AV_CURRENT),
	BMS_SYSFS_FIELD_RO(temp_max, BMS_PROP_TEMP_MAX),
	BMS_SYSFS_FIELD_RO(time_ot, BMS_PROP_TIME_OT),
	BMS_SYSFS_FIELD_RO(bms_slave_connect_error,
			   BMS_PROP_BMS_SLAVE_CONNECT_ERROR),
	BMS_SYSFS_FIELD_RO(cell_supplier, BMS_PROP_CELL_SUPPLIER),
	BMS_SYSFS_FIELD_RO(soa_alert_level, BMS_PROP_SOA_ALERT_LEVEL),
	BMS_SYSFS_FIELD_RO(charging_state, BMS_PROP_CHARGING_STATE),
	BMS_SYSFS_FIELD_RW(dod_count, BMS_PROP_DOD_COUNT),
	BMS_SYSFS_FIELD_RW(cis_alert, BMS_PROP_CIS_ALERT),
};

int bms_get_property(enum bms_property bp, int *val)
{
	struct bq_fg_chip *gm;
	struct power_supply *psy;

	psy = power_supply_get_by_name("bms");
	if (psy == NULL)
		return -ENODEV;

	gm = (struct bq_fg_chip *)power_supply_get_drvdata(psy);
	if (bms_sysfs_field_tbl[bp].prop == bp)
		bms_sysfs_field_tbl[bp].get(gm, &bms_sysfs_field_tbl[bp], val);
	else {
		fg_err("%s usb bp:%d idx error\n", __func__, bp);
		return -ENOTSUPP;
	}

	return 0;
}
EXPORT_SYMBOL(bms_get_property);

int bms_set_property(enum bms_property bp, int val)
{
	struct bq_fg_chip *gm;
	struct power_supply *psy;

	psy = power_supply_get_by_name("bms");
	if (psy == NULL)
		return -ENODEV;

	gm = (struct bq_fg_chip *)power_supply_get_drvdata(psy);

	if (bms_sysfs_field_tbl[bp].prop == bp)
		bms_sysfs_field_tbl[bp].set(gm, &bms_sysfs_field_tbl[bp], val);
	else {
		fg_err("%s usb bp:%d idx error\n", __func__, bp);
		return -ENOTSUPP;
	}
	return 0;
}
EXPORT_SYMBOL(bms_set_property);

static struct attribute *bms_sysfs_attrs[ARRAY_SIZE(bms_sysfs_field_tbl) + 1];

static const struct attribute_group bms_sysfs_attr_group = {
	.attrs = bms_sysfs_attrs,
};

static void bms_sysfs_init_attrs(void)
{
	int i, limit = ARRAY_SIZE(bms_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		bms_sysfs_attrs[i] = &bms_sysfs_field_tbl[i].attr.attr;

	bms_sysfs_attrs[limit] = NULL; /* Has additional entry for this */
}

static int bms_sysfs_create_group(struct power_supply *psy)
{
	bms_sysfs_init_attrs();

	return sysfs_create_group(&psy->dev.kobj, &bms_sysfs_attr_group);
}

static enum power_supply_property fg_props[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CAPACITY_ALERT_MAX,
	POWER_SUPPLY_PROP_CHARGE_EMPTY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_MODEL_NAME,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_CYCLE_COUNT,
	POWER_SUPPLY_PROP_CHARGE_CONTROL_END_THRESHOLD,
	POWER_SUPPLY_PROP_CALIBRATE,
	POWER_SUPPLY_PROP_CAPACITY_LEVEL,
	POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_VOLTAGE_BOOT,
	POWER_SUPPLY_PROP_SCOPE,
};

#define SHUTDOWN_DELAY_VOL 3300
#define SHUTDOWN_VOL 3400
static int fg_get_property(struct power_supply *psy,
			   enum power_supply_property psp,
			   union power_supply_propval *val)
{
	struct bq_fg_chip *bq = power_supply_get_drvdata(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = bq->model_name;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		mutex_lock(&bq->data_lock);
		fg_read_volt(bq);
		val->intval = bq->cell_voltage[2] * 1000;
		mutex_unlock(&bq->data_lock);
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		mutex_lock(&bq->data_lock);
		if (bq->i2c_err_flag) {
			val->intval = 500;
		} else {
			bq->ibat = fg_read_current(bq);
			val->intval = bq->ibat * 1000;
		}
		mutex_unlock(&bq->data_lock);
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		if (bq->fake_soc) {
			val->intval = bq->fake_soc;
			break;
		}

		if (bq->i2c_err_flag) {
			val->intval = 15;
		}else if (bq->ui_soc <= 1)
			val->intval = 1;
		else
			val->intval = bq->ui_soc;
		break;
    case POWER_SUPPLY_PROP_CAPACITY_ALERT_MAX:
        val->intval = bq->rsoc;
        break;
    case POWER_SUPPLY_PROP_CAPACITY_ALERT_MIN:
        val->intval = bq->raw_soc;
        break;
    case POWER_SUPPLY_PROP_CHARGE_EMPTY:
        val->intval = bq->soh;
        break;
	case POWER_SUPPLY_PROP_TEMP:
		if (bq->fake_tbat != -999) {
			val->intval = bq->fake_tbat;
			break;
		}

		if (bq->i2c_err_flag) {
			val->intval = 250;
		} else {
			val->intval = bq->tbat;
		}

		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL:
		if (bq->device_name == BQ_FG_BQ27Z561 ||
		    bq->device_name == BQ_FG_NFG1000A ||
		    bq->device_name == BQ_FG_NFG1000B)
			val->intval = bq->fcc;
		else if (bq->device_name == BQ_FG_BQ28Z610)
			val->intval = bq->fcc * 2;
		else
			val->intval = 4500;
		if(val->intval > 7000)
			val->intval = 7000;
		val->intval *= 1000;
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		val->intval = 7000 * 1000;
		break;
	case POWER_SUPPLY_PROP_CHARGE_COUNTER:
		val->intval = bq->rm * 1000;
		break;
	case POWER_SUPPLY_PROP_CYCLE_COUNT:
		val->intval = bq->cycle_count;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LIPO;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		if (bq->i2c_err_flag) {
			val->intval = 1;
		}else{
			val->intval = bq->chip_ok;
		}
		break;
	case POWER_SUPPLY_PROP_TIME_TO_FULL_NOW:
		val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_CURRENT_BOOT:
		fg_dbg("get fast_charge_mode \n");
		val->intval = bq->fast_chg;
		break;
	case POWER_SUPPLY_PROP_AUTHENTIC:
		fg_dbg("get secret_result \n");
		break;
	case POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN:
		fg_dbg("get soc_decimal_rate \n");
		val->intval = fg_get_soc_decimal_rate(bq);
		break;
	case POWER_SUPPLY_PROP_ENERGY_FULL:
		fg_dbg("get soc_decimal \n");
		val->intval = fg_get_soc_decimal(bq);
		break;
	case POWER_SUPPLY_PROP_CALIBRATE:
		val->intval = bq->cell_supplier;
		fg_dbg("batt id: %d\n", bq->cell_supplier);
		break;
	case POWER_SUPPLY_PROP_ENERGY_AVG:
		//pr_err("get bq28z610 fg1_df_check \n");
		val->intval = fg_read_fg1_df_check(bq);
		break;
	case POWER_SUPPLY_PROP_ENERGY_NOW:
		fg_dbg("get fg1_chemid \n");
		val->intval = fg_read_fg1_chemid(bq);
		break;
	case POWER_SUPPLY_PROP_ENERGY_EMPTY:
		fg_dbg("get pack_vendor \n");
		val->intval = fg_read_pack_vendor(bq);
		break;
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		if (bq->i2c_err_flag) {
			val->intval = POWER_SUPPLY_CAPACITY_LEVEL_NORMAL;
			break;
		}
		if (bq->batt_cap_level_critical){
			val->intval = POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL;
			pr_debug("%s fg critical level true\n", __func__);
			break;
		}
		break;
	case POWER_SUPPLY_PROP_TEMP_AMBIENT_ALERT_MIN:
		val->intval = cis_alert_level_get(bq);
		bq->cis_alert = val->intval;
		break;
	case POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT:
		val->intval = bq->iterm; //uA
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		val->intval = bq->cv;  //uV
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int fg_set_property(struct power_supply *psy,
			   enum power_supply_property prop,
			   const union power_supply_propval *val)
{
	struct bq_fg_chip *bq = power_supply_get_drvdata(psy);
	int ret = 0;

	switch (prop) {
	case POWER_SUPPLY_PROP_TEMP:
		bq->fake_tbat = val->intval;
		power_supply_changed(bq->batt_psy);
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		bq->fake_soc = val->intval;
		power_supply_changed(bq->fg_psy);
		break;
	case POWER_SUPPLY_PROP_CYCLE_COUNT:
		pr_debug("%s: set cycle_count:%d \n", __func__, val->intval);
		bq->fake_cycle_count = val->intval;
		break;
	case POWER_SUPPLY_PROP_CURRENT_BOOT:
		fg_dbg("set fast_charge_mode \n");
		ret = fg_set_fastcharge_mode(bq, val->intval);
		break;
	case POWER_SUPPLY_PROP_CHARGE_CONTROL_END_THRESHOLD:
		ret = fg_set_charger_to_full(bq);
		break;
	case POWER_SUPPLY_PROP_TEMP_AMBIENT_ALERT_MIN:
		bq->fake_cis_alert = val->intval;
		break;
	case POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT:
		bq->iterm = val->intval; //uA
		break;
	/*set vbus_ctrl gpio5*/
	case POWER_SUPPLY_PROP_VOLTAGE_BOOT:
		pr_debug("%s: set vbus_ctrl:%d \n", __func__, val->intval);
		gpio_direction_output(vbus_ctrl_gpio, val->intval);
		break;
	/*vbus force to fix 5V by connector protect*/
	case POWER_SUPPLY_PROP_SCOPE:
		bq->connector_protect = val->intval;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		bq->cv = val->intval;  //uV
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static char *mtk_bms_supplied_to[] = {
	"battery",
	"usb",
};

static int fg_prop_is_writeable(struct power_supply *psy,
				enum power_supply_property prop)
{
	int ret;

	switch (prop) {
	case POWER_SUPPLY_PROP_TEMP:
	case POWER_SUPPLY_PROP_CAPACITY:
	case POWER_SUPPLY_PROP_CYCLE_COUNT:
	case POWER_SUPPLY_PROP_CHARGE_CONTROL_END_THRESHOLD:
	case POWER_SUPPLY_PROP_TEMP_AMBIENT_ALERT_MIN:
	case POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT:
	case POWER_SUPPLY_PROP_VOLTAGE_BOOT:
	case POWER_SUPPLY_PROP_SCOPE:
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		ret = 1;
		break;
	default:
		ret = 0;
		break;
	}
	return ret;
}

static int fg_init_psy(struct bq_fg_chip *bq)
{
	struct power_supply_config fg_psy_cfg = {};

	bq->fg_psy_d.name = "bq28z610";
	bq->fg_psy_d.type = POWER_SUPPLY_TYPE_UNKNOWN;
	bq->fg_psy_d.properties = fg_props;
	bq->fg_psy_d.num_properties = ARRAY_SIZE(fg_props);
	bq->fg_psy_d.get_property = fg_get_property;
	bq->fg_psy_d.set_property = fg_set_property;
	bq->fg_psy_d.property_is_writeable = fg_prop_is_writeable;
	fg_psy_cfg.supplied_to = mtk_bms_supplied_to;
	fg_psy_cfg.num_supplicants = ARRAY_SIZE(mtk_bms_supplied_to);
	fg_psy_cfg.drv_data = bq;

	bq->fg_psy =
		devm_power_supply_register(bq->dev, &bq->fg_psy_d, &fg_psy_cfg);
	if (IS_ERR(bq->fg_psy)) {
		fg_err("%s failed to register fg_psy", bq->log_tag);
		return PTR_ERR(bq->fg_psy);
	} else
		bms_sysfs_create_group(bq->fg_psy);

	return 0;
}

static ssize_t fg_show_qmax0(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bq_fg_chip *bq = i2c_get_clientdata(client);
	int ret = 0;

	mutex_lock(&bq->data_lock);
	fg_read_qmax(bq);
	mutex_unlock(&bq->data_lock);
	ret = snprintf(buf, PAGE_SIZE, "%d\n", bq->qmax[0]);

	return ret;
}

static ssize_t fg_show_qmax1(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bq_fg_chip *bq = i2c_get_clientdata(client);
	int ret = 0;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", bq->qmax[1]);

	return ret;
}

static ssize_t fg_show_cell0_voltage(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bq_fg_chip *bq = i2c_get_clientdata(client);
	int ret = 0;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", bq->cell_voltage[0]);

	return ret;
}

static ssize_t fg_show_cell1_voltage(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bq_fg_chip *bq = i2c_get_clientdata(client);
	int ret = 0;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", bq->cell_voltage[1]);

	return ret;
}

static ssize_t fg_show_rsoc(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bq_fg_chip *bq = i2c_get_clientdata(client);
	int rsoc = 0, ret = 0;

	mutex_lock(&bq->data_lock);
	rsoc = fg_read_rsoc(bq);
	mutex_unlock(&bq->data_lock);
	ret = snprintf(buf, PAGE_SIZE, "%d\n", rsoc);

	return ret;
}

static ssize_t fg_show_fcc(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bq_fg_chip *bq = i2c_get_clientdata(client);
	int fcc = 0, ret = 0;

	mutex_lock(&bq->data_lock);
	fcc = fg_read_fcc(bq);
	mutex_unlock(&bq->data_lock);
	ret = snprintf(buf, PAGE_SIZE, "%d\n", fcc);

	return ret;
}

static ssize_t fg_show_rm(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bq_fg_chip *bq = i2c_get_clientdata(client);
	int rm = 0, ret = 0;

	mutex_lock(&bq->data_lock);
	rm = fg_read_rm(bq);
	mutex_unlock(&bq->data_lock);
	ret = snprintf(buf, PAGE_SIZE, "%d\n", rm);

	return ret;
}

int fg_stringtohex(char *str, unsigned char *out, unsigned int *outlen)
{
	char *p = str;
	char high = 0, low = 0;
	int tmplen = strlen(p), cnt = 0;
	tmplen = strlen(p);
	while (cnt < (tmplen / 2)) {
		high = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ?
			       *p - 48 - 7 :
			       *p - 48;
		low = (*(++p) > '9' && ((*p <= 'F') || (*p <= 'f'))) ?
			      *(p)-48 - 7 :
			      *(p)-48;
		out[cnt] = ((high & 0x0f) << 4 | (low & 0x0f));
		p++;
		cnt++;
	}
	if (tmplen % 2 != 0)
		out[cnt] = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ?
				   *p - 48 - 7 :
				   *p - 48;

	if (outlen != NULL)
		*outlen = tmplen / 2 + tmplen % 2;

	return tmplen / 2 + tmplen % 2;
}

static ssize_t fg_verify_digest_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bq_fg_chip *bq = i2c_get_clientdata(client);
	u8 digest_buf[4] = { 0 };
	int len = 0, i = 0;

	if (bq->device_name == BQ_FG_BQ27Z561 ||
	    bq->device_name == BQ_FG_NFG1000A ||
	    bq->device_name == BQ_FG_NFG1000B) {
		for (i = 0; i < RANDOM_CHALLENGE_LEN_BQ27Z561; i++) {
			memset(digest_buf, 0, sizeof(digest_buf));
			snprintf(digest_buf, sizeof(digest_buf) - 1, "%02x",
				 bq->digest[i]);
			strlcat(buf, digest_buf,
				RANDOM_CHALLENGE_LEN_BQ27Z561 * 2 + 1);
		}
	} else if (bq->device_name == BQ_FG_BQ28Z610) {
		for (i = 0; i < RANDOM_CHALLENGE_LEN_BQ28Z610; i++) {
			memset(digest_buf, 0, sizeof(digest_buf));
			snprintf(digest_buf, sizeof(digest_buf) - 1, "%02x",
				 bq->digest[i]);
			strlcat(buf, digest_buf,
				RANDOM_CHALLENGE_LEN_BQ28Z610 * 2 + 1);
		}
	} else {
		fg_err("%s not support device name\n", bq->log_tag);
	}

	len = strlen(buf);
	buf[len] = '\0';

	return strlen(buf) + 1;
}

ssize_t fg_verify_digest_store(struct device *dev,
			       struct device_attribute *attr, const char *buf,
			       size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bq_fg_chip *bq = i2c_get_clientdata(client);
	int i = 0;
	u8 random[RANDOM_CHALLENGE_LEN_MAX] = { 0 };
	char kbuf[70] = { 0 };

	memset(kbuf, 0, sizeof(kbuf));
	strncpy(kbuf, buf, count - 1);
	fg_stringtohex(kbuf, random, &i);
	if (bq->device_name == BQ_FG_BQ27Z561 ||
	    bq->device_name == BQ_FG_NFG1000A ||
	    bq->device_name == BQ_FG_NFG1000B)
		fg_sha256_auth(bq, random, RANDOM_CHALLENGE_LEN_BQ27Z561);
	else if (bq->device_name == BQ_FG_BQ28Z610)
		fg_sha256_auth(bq, random, RANDOM_CHALLENGE_LEN_BQ28Z610);
	schedule_delayed_work(&bq->auth_work, 10 * HZ);

	return count;
}

static ssize_t fg_show_log_level(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bq_fg_chip *bq = i2c_get_clientdata(client);
	int ret = 0;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", log_level);
	fg_info("%s show log_level = %d\n", bq->log_tag, log_level);

	return ret;
}

static ssize_t fg_store_log_level(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bq_fg_chip *bq = i2c_get_clientdata(client);
	int ret = 0;

	ret = sscanf(buf, "%d", &log_level);
	fg_info("%s store log_level = %d\n", bq->log_tag, log_level);

	return count;
}

static ssize_t ntc_temp_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bq_fg_chip *bq = i2c_get_clientdata(client);
	u8 manuInfoC_data[32];
	unsigned int ntc_temp;
	int ret;

	ret = fg_mac_read_block(bq, FG_MAC_CMD_NTC_TEMP, manuInfoC_data, 6);
	if (ret) {
		fg_err("get FG_MAC_CMD_NTC_TEMP failed:%d\n", ret);
		return -EPERM;
	}
	ntc_temp = (manuInfoC_data[1] << 8) | manuInfoC_data[0];
	ntc_temp = ntc_temp - 2730;
	fg_info("ntc_temp:%u\n", ntc_temp);

	return snprintf(buf, PAGE_SIZE, "%u\n", ntc_temp);
}

static DEVICE_ATTR(fcc, S_IRUGO, fg_show_fcc, NULL);
static DEVICE_ATTR(rm, S_IRUGO, fg_show_rm, NULL);
static DEVICE_ATTR(rsoc, S_IRUGO, fg_show_rsoc, NULL);
static DEVICE_ATTR(cell0_voltage, S_IRUGO, fg_show_cell0_voltage, NULL);
static DEVICE_ATTR(cell1_voltage, S_IRUGO, fg_show_cell1_voltage, NULL);
static DEVICE_ATTR(qmax0, S_IRUGO, fg_show_qmax0, NULL);
static DEVICE_ATTR(qmax1, S_IRUGO, fg_show_qmax1, NULL);
static DEVICE_ATTR(verify_digest, S_IRUGO | S_IWUSR, fg_verify_digest_show,
		   fg_verify_digest_store);
static DEVICE_ATTR(log_level, S_IRUGO | S_IWUSR, fg_show_log_level,
		   fg_store_log_level);
static DEVICE_ATTR(ntc_temp, S_IRUGO, ntc_temp_show, NULL);

static struct attribute *fg_attributes[] = {
	&dev_attr_rm.attr,
	&dev_attr_fcc.attr,
	&dev_attr_rsoc.attr,
	&dev_attr_cell0_voltage.attr,
	&dev_attr_cell1_voltage.attr,
	&dev_attr_qmax0.attr,
	&dev_attr_qmax1.attr,
	&dev_attr_verify_digest.attr,
	&dev_attr_log_level.attr,
	&dev_attr_ntc_temp.attr,
	NULL,
};

static const struct attribute_group fg_attr_group = {
	.attrs = fg_attributes,
};

static int fg_parse_dt(struct bq_fg_chip *bq)
{
	struct device_node *node = bq->dev->of_node;
	int ret = 0, size = 0;

	bq->max_chg_power_120w =
		of_property_read_bool(node, "max_chg_power_120w");
	bq->enable_shutdown_delay =
		of_property_read_bool(node, "enable_shutdown_delay");

#if defined(CONFIG_TARGET_PRODUCT_YUECHU)
	bq->slave_connect_gpio =
		of_get_named_gpio(node, "slave_connect_gpio", 0);
	fg_dbg("%s slave_connect_gpio = %d \n", bq->log_tag,
	       bq->slave_connect_gpio);
	if (!gpio_is_valid(bq->slave_connect_gpio)) {
		fg_info("failed to parse slave_connect_gpio\n");
		return -1;
	}
#endif

	ret = of_property_read_u32(node, "normal_shutdown_vbat_1s",
				   &bq->normal_shutdown_vbat);
	if (ret)
		fg_err("%s failed to parse normal_shutdown_vbat_1s\n",
		       bq->log_tag);

	ret = of_property_read_u32(node, "critical_shutdown_vbat_1s",
				   &bq->critical_shutdown_vbat);
	if (ret)
		fg_err("%s failed to parse critical_shutdown_vbat_1s\n",
		       bq->log_tag);

	ret = of_property_read_u32(node, "cool_critical_shutdown_vbat_1s",
				   &bq->cool_critical_shutdown_vbat);
	if (ret)
		fg_err("%s failed to parse cool_critical_shutdown_vbat_1s\n",
		       bq->log_tag);

	ret = of_property_read_u32(node, "old_critical_shutdown_vbat_1s",
				   &bq->old_critical_shutdown_vbat);
	if (ret)
		fg_err("%s failed to parse old_critical_shutdown_vbat_1s\n",
		       bq->log_tag);

#ifndef CONFIG_TARGET_PRODUCT_XAGA
	ret = of_property_read_u32(node, "report_full_rsoc_1s",
				   &bq->report_full_rsoc);
	if (ret)
		fg_err("%s failed to parse report_full_rsoc_1s\n", bq->log_tag);
#else
	if (product_name == XAGA)
		bq->report_full_rsoc = 9700;
	else if (product_name == XAGAPRO)
		bq->report_full_rsoc = 9500;
#endif

	ret = of_property_read_u32(node, "soc_gap_1s", &bq->soc_gap);
	if (ret)
		fg_err("%s failed to parse soc_gap_1s\n", bq->log_tag);

	of_get_property(node, "soc_decimal_rate", &size);
	if (size) {
		bq->dec_rate_seq = devm_kzalloc(bq->dev, size, GFP_KERNEL);
		if (bq->dec_rate_seq) {
			bq->dec_rate_len = (size / sizeof(*bq->dec_rate_seq));
			if (bq->dec_rate_len % 2) {
				fg_err("%s invalid soc decimal rate seq\n",
				       bq->log_tag);
				return -EINVAL;
			}
			of_property_read_u32_array(node, "soc_decimal_rate",
						   bq->dec_rate_seq,
						   bq->dec_rate_len);
		} else {
			fg_err("%s error allocating memory for dec_rate_seq\n",
			       bq->log_tag);
		}
	}

	return ret;
}

static void fg_auth_workfunc(struct work_struct *work)
{
	struct power_supply *psy_usb = NULL;
	struct bq_fg_chip *bq = container_of(work, struct bq_fg_chip, auth_work.work);

	if (!bq->powerok && bq->authenticate) {
		bq->irregular_batt = IRREGULATOR_BATT1;
		fg_info("irregular battery1 found\n");
	}

	if (!bq->i2c_err_flag && !bq->authenticate) {
		bq->irregular_batt = IRREGULATOR_BATT2;
		fg_info("irregular battery2 found\n");
	} else if (bq->powerok && bq->authenticate && !bq->i2c_err_flag) {
		bq->irregular_batt = 0;
		fg_info("regular battery found\n");
	}

	psy_usb = power_supply_get_by_name("usb");
	if (IS_ERR_OR_NULL(psy_usb)) {
		fg_err("usb psy not found\n");
	} else {
		power_supply_changed(psy_usb);
	}
}

static int fg_check_device(struct bq_fg_chip *bq)
{
	u8 data[32];
	int ret = 0, i = 0;
	int retry = 0;

	while (retry++ < 5) {
		ret = fg_mac_read_block(bq, FG_MAC_CMD_DEVICE_NAME, data, 32);
		if (ret) {
			bq->i2c_err_flag = true;
			bq->irregular_batt = IRREGULATOR_BATT3;
			fg_info("irregular battery3 found\n");
			fg_err("failed to get FG_MAC_CMD_DEVICE_NAME ret =%d\n", ret);
		} else {
			fg_dbg("data : %s\n", data);
			bq->i2c_err_flag = false;
			bq->irregular_batt = 0;
			break;
		}
		msleep(5);
	}

	for (i = 0; i < 8; i++) {
		if (data[i] >= 'A' && data[i] <= 'Z')
			data[i] += 32;
	}

	if (!strncmp(data, "sn27z565R1", 8)) {
		bq->device_name = BQ_FG_BQ27Z561;
		strcpy(bq->model_name, "sn27z565R1");
		strcpy(bq->log_tag, "[FG_SN27Z565R1]");
	} else if (!strncmp(data, "bq27z561", 8)) {
		bq->device_name = BQ_FG_BQ27Z561;
		strcpy(bq->model_name, "bq27z561");
		strcpy(bq->log_tag, "[FG_BQ27Z561]");
	} else if (!strncmp(data, "bq28z610", 8)) {
		bq->device_name = BQ_FG_BQ28Z610;
		strcpy(bq->model_name, "bq28z610");
		strcpy(bq->log_tag, "[FG_BQ28Z610]");
	} else if (!strncmp(data, "nfg1000a", 8)) {
		bq->device_name = BQ_FG_NFG1000A;
		strcpy(bq->model_name, "nfg1000a");
		strcpy(bq->log_tag, "[FG_NFG1000A]");
	} else if (!strncmp(data, "nfg1000b", 8)) {
		bq->device_name = BQ_FG_NFG1000B;
		strcpy(bq->model_name, "nfg1000b");
		strcpy(bq->log_tag, "[FG_NFG1000B]");
	} else if (!strncmp(data, "m9@bp4p#", 8)) {
		bq->device_name = BQ_FG_NFG1000A;
		strcpy(bq->model_name, "nfg1000a");
		strcpy(bq->log_tag, "[FG_NFG1000A]");
	} else if (!strncmp(data, "m12a@bm5t#", 8)) {
		bq->device_name = BQ_FG_NFG1000A;
		strcpy(bq->model_name, "nfg1000a");
		strcpy(bq->log_tag, "[FG_NFG1000A]");
	} else if (!strncmp(data, "m11r@bm5f#", 8)) {
		bq->device_name = BQ_FG_NFG1000B;
		strcpy(bq->model_name, "nfg1000b");
		strcpy(bq->log_tag, "[FG_NFG1000B]");
	} else if (!strncmp(data, "n6@bm5t#", 8)) {
		ret = fg_mac_read_block(bq, FG_MAC_CMD_MANU_NAME, data, 32);
		fg_dbg("FG_MAC_CMD_MANU_NAME:data : %s\n", data);
		if (!strncmp(&data[2], "C", 1)) {
			bq->device_name = BQ_FG_BQ27Z561;
			strcpy(bq->model_name, "bq27z561");
			strcpy(bq->log_tag, "[FG_BQ27Z561]");
		} else if (!strncmp(&data[2], "5", 1)) {
			bq->device_name = BQ_FG_NFG1000A;
			strcpy(bq->model_name, "nfg1000a");
			strcpy(bq->log_tag, "[FG_NFG1000A]");
		}
	} else if (!strncmp(data, "xm32@bm69#", 10)) {
		bq->device_name = BQ_FG_NFG1000B;
		strcpy(bq->model_name, "nfg1000b");
		strcpy(bq->log_tag, "[FG_NFG1000]");
	} else if (!strncmp(data, "xm81@bn70#", 10)) {
		bq->device_name = BQ_FG_NFG1000B;
		strcpy(bq->model_name, "nfg1000b");
		strcpy(bq->log_tag, "[FG_NFG1000]");
	} else if (!strncmp(data, "xm81@bm71#", 10)) {
		bq->device_name = BQ_FG_NFG1000B;
		strcpy(bq->model_name, "nfg1000b");
		strcpy(bq->log_tag, "[FG_NFG1000]");
	} else {
		bq->device_name = BQ_FG_UNKNOWN;
		strcpy(bq->model_name, "UNKNOWN");
		strcpy(bq->log_tag, "[FG_UNKNOWN]");
	}

	ret = fg_mac_read_block(bq, FG_MAC_CMD_MANU_NAME, data, 32);
	if (ret)
		fg_info("failed to get FG_MAC_CMD_MANU_NAME\n");

	fg_info("battert power:%02x/%c\n", data[4], data[4]);
	bq->powerok = (data[4] == '4') ? 1 : 0;

	if (!strncmp(data, "MI", 2) && bq->device_name != BQ_FG_UNKNOWN)
		bq->chip_ok = true;
	else
		bq->chip_ok = false;

	ret = fg_mac_read_block(bq, FG_MAC_CMD_DEVICE_CHEM, data, 32);
	if (ret)
		fg_info("failed to get FG_MAC_CMD_DEVICE_CHEM\n");

	if (!strncmp(&data[2], "S", 1) && bq->device_name != BQ_FG_UNKNOWN) {
		bq->cell_supplier = BMS_CELL_LWN;
		strcpy(bq->device_chem, "LWN");
	} else if (!strncmp(&data[2], "N", 1) &&
		   bq->device_name != BQ_FG_UNKNOWN) {
		bq->cell_supplier = BMS_CELL_ATL;
		strcpy(bq->device_chem, "ATL");
	} else if (!strncmp(&data[2], "C", 1) &&
		   bq->device_name != BQ_FG_UNKNOWN) {
		bq->cell_supplier = BMS_CELL_COS;
		strcpy(bq->device_chem, "COS");
	} else {
		bq->cell_supplier = BMS_CELL_UNKNOWN;
		strcpy(bq->device_chem, "UNKNOWN");
	}

	return ret;
}
#if 0
static struct fuel_gauge_ops fuel_gauge_ops = {
	.get_soc_decimal = fuel_guage_get_soc_decimal,
	.get_soc_decimal_rate = fuel_guage_get_soc_decimal_rate,
#if IS_ENABLED(CONFIG_XM_FG_I2C_ERR)
	.check_i2c_function = fuelguage_check_i2c_function,
#endif
	.set_fastcharge_mode = fuelguage_set_fastcharge_mode,
	.get_fastcharge_mode = fuelguage_get_fastcharge_mode,
};
#endif
static int defer_count;
static int fg_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct bq_fg_chip *bq;
	int ret = 0;
	char *name = NULL;
	u8 data[5] = { 0 };

#if defined(CONFIG_TARGET_PRODUCT_XAGA)
	const char *sku = get_hw_sku();
	if (!strncmp(sku, "xagapro", strlen("xagapro")))
		product_name = XAGAPRO;
	else if (!strncmp(sku, "xaga", strlen("xaga")))
		product_name = XAGA;
#endif

#if defined(CONFIG_TARGET_PRODUCT_DAUMIER)
	product_name = DAUMIER;
#endif

	fg_info("FG probe enter\n");
	bq = devm_kzalloc(&client->dev, sizeof(*bq), GFP_DMA);
	if (!bq)
		return -ENOMEM;

	bq->dev = &client->dev;
	bq->client = client;
	bq->monitor_delay = FG_MONITOR_DELAY_10S;

	memcpy(bq->regs, bq_fg_regs, NUM_REGS);

	i2c_set_clientdata(client, bq);
	name = devm_kasprintf(bq->dev, GFP_KERNEL, "%s",
			      "bms suspend wakelock");
	bq->bms_wakelock = wakeup_source_register(NULL, name);
	bq->shutdown_mode = false;
	bq->shutdown_flag = false;
	bq->fake_cycle_count = -999;
	bq->fake_cis_alert = 0;
	bq->fake_tbat = -999;
	bq->raw_soc = -ENODATA;
	bq->last_soc = -EINVAL;
	bq->i2c_error_count = 0;
	bq->first_flag = true;
	bq->iterm = 480000; //uA
	bq->cv = 4560000;   //uV
	bq->connector_protect = 0;
	bq->rsoc_smooth = 0;

	bq->recover.rsoc = 15;
	bq->recover.temp = 250;
	bq->recover.batt_fc = 0;
	bq->recover.vbat = 4000;
	bq->recover.cell_voltage = 4000;
	bq->recover.avg_curr = -500;
	bq->recover.curr = -500;
	bq->recover.fcc = 5160;
	bq->recover.rm = 2580;
	bq->recover.dc = 5160;
	bq->recover.soh = 50;
	bq->recover.cyclecount = 0;

	bq->last_rt.rsoc_read_time = -EINVAL;
	bq->last_rt.temp_read_time = -EINVAL;
	bq->last_rt.vbat_read_time = -EINVAL;
	bq->last_rt.ibat_read_time = -EINVAL;

	mutex_init(&bq->i2c_rw_lock);
	mutex_init(&bq->data_lock);
	atomic_set(&bq->fg_in_sleep, 0);

	bq->regmap = devm_regmap_init_i2c(client, &fg_regmap_config);
	if (IS_ERR(bq->regmap)) {
		fg_err("failed to allocate regmap\n");
		return PTR_ERR(bq->regmap);
	}

	fg_check_device(bq);

	ret = fg_parse_dt(bq);
	if (ret) {
		fg_err("%s failed to parse DTS\n", bq->log_tag);
		return ret;
	}

	time_init = ktime_get();
	bq->is_eu_mode = get_eu_mode();
	if(bq->is_eu_mode < 0 && defer_count < 10){
		pr_err("%s: get eu_modefailed: %d \n", __func__, bq->is_eu_mode);
		defer_count ++;
		return -EPROBE_DEFER;
	}else{
		pr_err("%s: is_eu_mode:%d \n", __func__, bq->is_eu_mode);
	}
	fg_update_status(bq);
	fg_select_poweroff_voltage_config(bq);
	ret = fg_init_psy(bq);
	if (ret) {
		fg_err("%s failed to init psy\n", bq->log_tag);
		return ret;
	}

	ret = sysfs_create_group(&bq->dev->kobj, &fg_attr_group);
	if (ret) {
		fg_err("%s failed to register sysfs\n", bq->log_tag);
		return ret;
	}

	bq->update_now = true;
	INIT_DELAYED_WORK(&bq->monitor_work, fg_monitor_workfunc);
	schedule_delayed_work(&bq->monitor_work, msecs_to_jiffies(5000));

	bq->dc = fg_read_dc(bq);

	/* init fast charge mode */
	data[0] = 0;
	fg_dbg("-fastcharge init-\n");
	ret = fg_mac_write_block(bq, FG_MAC_CMD_FASTCHARGE_DIS, data, 2);
	if (ret) {
		fg_err("%s failed to write fastcharge = %d\n", bq->log_tag,
		       ret);
	}
#if 0
	bq->fuel_gauge = fuel_gauge_register("fuel_gauge",
						bq->dev, &fuel_gauge_ops, bq);
	if (!bq->fuel_gauge) {
		ret = PTR_ERR(bq->fuel_gauge);
		fg_err("%s failed to register fuel_gauge\n", bq->log_tag);
		return ret;
	}

	bq->charger = charger_find_dev_by_name("primary_chg");
	if (!bq->charger)
		pr_err("failed to master_charge device\n");
#endif
	bq->pfg_read_soh = fg_read_soh;
	bq->pfg_read_cyclecount = fg_read_cyclecount;
	bq->pfg_mac_read_block = fg_mac_read_block;
	bq->pfg_mac_write_block = fg_mac_write_block;
	bq->pfg_write_block = fg_write_block;

	INIT_DELAYED_WORK(&bq->auth_work, fg_auth_workfunc);

	vbus_ctrl_gpio = of_get_named_gpio(client->dev.of_node, "vbus_ctrl", 0);
	if (gpio_is_valid(vbus_ctrl_gpio)) {
		if (!gpio_request(vbus_ctrl_gpio, "vbus_ctrl")) {
			gpio_direction_output(vbus_ctrl_gpio, 0);
			pr_err("vbus ctrl gpio set low\n");
		} else
			pr_err("vbus ctrl gpio request failed\n");
	} else
		pr_err("vbus ctrl gpio invalid\n");

	fg_info("%s FG probe success\n", bq->log_tag);

	return 0;
}

static int fg_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bq_fg_chip *bq = i2c_get_clientdata(client);
	atomic_set(&bq->fg_in_sleep, 1);
	fg_info("%s in sleep\n", __func__);

	cancel_delayed_work_sync(&bq->monitor_work);

	return 0;
}

static int fg_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bq_fg_chip *bq = i2c_get_clientdata(client);
	atomic_set(&bq->fg_in_sleep, 0);
	fg_info("%s resume in sleep\n", __func__);
	if (!bq->bms_wakelock->active)
		__pm_stay_awake(bq->bms_wakelock);
	schedule_delayed_work(&bq->monitor_work, 0);
	bq->param.update_now = true;

	return 0;
}

static void fg_remove(struct i2c_client *client)
{
	struct bq_fg_chip *bq = i2c_get_clientdata(client);

	power_supply_unregister(bq->fg_psy);
	mutex_destroy(&bq->data_lock);
	mutex_destroy(&bq->i2c_rw_lock);
	sysfs_remove_group(&bq->dev->kobj, &fg_attr_group);
}

static void fg_shutdown(struct i2c_client *client)
{
	struct bq_fg_chip *bq = i2c_get_clientdata(client);

	fg_info("%s bq fuel gauge driver shutdown!\n", bq->log_tag);
}

static struct of_device_id fg_match_table[] = {
	{
		.compatible = "bq28z610",
	},
	{},
};
MODULE_DEVICE_TABLE(of, fg_match_table);

static const struct i2c_device_id fg_id[] = {
	{ "bq28z610", 0 },
	{},
};
MODULE_DEVICE_TABLE(i2c, fg_id);

static const struct dev_pm_ops fg_pm_ops = {
	.resume = fg_resume,
	.suspend = fg_suspend,
};

static struct i2c_driver fg_driver = {
	.driver	= {
		.name   = "bq28z610",
		.owner  = THIS_MODULE,
		.of_match_table = fg_match_table,
		.pm     = &fg_pm_ops,
	},
	.id_table   = fg_id,

	.probe      = fg_probe,
	.remove		= fg_remove,
	.shutdown	= fg_shutdown,
};

module_i2c_driver(fg_driver);

MODULE_DESCRIPTION("TI GAUGE Driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Texas Instruments");
