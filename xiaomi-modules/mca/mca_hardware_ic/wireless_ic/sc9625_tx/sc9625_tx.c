// SPDX-License-Identifier: GPL-2.0
/*
 * sc9625_tx.c
 *
 * wireless TRX ic driver
 *
 * Copyright (c) 2023-2023 Xiaomi Technologies Co., Ltd.
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

#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/irqreturn.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/regmap.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/module.h>
#include <linux/pinctrl/consumer.h>
#include <linux/version.h>

#include "inc/sc9625_reg.h"
#include "inc/sc9625_mtp_program.h"
#include "inc/sc9625_fw.h"
#include <mca/common/mca_log.h>
#include <mca/common/mca_event.h>
#include <mca/common/mca_parse_dts.h>
#include <mca/common/mca_adsp_glink.h>
#include <mca/platform/platform_wireless_class.h>
#include <mca/platform/platform_cp_class.h>
#include <mca/common/mca_sysfs.h>
#include <mca/strategy/strategy_class.h>

#ifndef MCA_LOG_TAG
#define MCA_LOG_TAG "sc9625_tx"
#endif

#define SC9625_DRV_VERSION     "1.0.0_G"

//--------------------------IIC API-----------------------------
static const struct regmap_config sc9625_regmap_config = {
    .reg_bits = 16,
    .val_bits = 8,
    .max_register = 0xFFFF,
};

static int __sc9625_read_block(struct sc9625 *sc, uint16_t reg, 
        uint8_t *data, uint8_t length)
{
    int ret;

    ret = regmap_raw_read(sc->regmap, reg, data, length);
    if (ret) {
        mca_log_err("i2c read fail: can't read from reg 0x%04X\n", reg);
        return -EIO;
    }

    return 0;
}

static int __sc9625_write_block(struct sc9625 *sc, uint16_t reg, 
        uint8_t *data, uint8_t length)
{
    int ret;

    ret = regmap_raw_write(sc->regmap, reg, data, length);
    if (ret) {
        mca_log_err("i2c write fail: can't write 0x%04X: %x\n", reg, ret);
        return -EIO;
    }

    return 0;
}

static int sc9625_read_block(struct sc9625 *sc, uint32_t reg, 
        uint8_t *data, uint8_t len)
{
    int ret;
    uint8_t *reg_data;
    uint8_t reg_len;
    uint16_t reg_start;

    if (sc->fw_program) {
        mca_log_err("firmware programming\n");
        return -EBUSY;
    }

    reg_start = (reg - (reg % 4));
    reg_len = (reg + len - reg_start) % 4 == 0 ? 
        (reg + len - reg_start) : ((((reg + len - reg_start) / 4) + 1) * 4);
    
    reg_data = (uint8_t*)kmalloc(reg_len, GFP_KERNEL);
    if (!reg_data) {
        mca_log_err("The Pointer is NULL\n");
        return -ENOMEM;
    }

    //mca_log_err("Enter %s, Reg: %04x, len:%x, reg_start:%04x, reg_len:%x.\n",
    //     __func__, reg, len, reg_start, reg_len);

    mutex_lock(&sc->i2c_rw_lock);
    ret = __sc9625_read_block(sc, reg_start, reg_data, reg_len);
    mutex_unlock(&sc->i2c_rw_lock);

    if (ret >= 0) {
        memcpy(data, reg_data + (reg % 4), len);
    }

    kfree(reg_data);
    return ret;
}

static int sc9625_write_block(struct sc9625 *sc, uint32_t reg,
        uint8_t *data, uint8_t len)
{
    int ret = 0;
    uint8_t *reg_data;
    uint8_t reg_len;
    int reg_start;

    if (sc->fw_program) {
        mca_log_err("firmware programming\n");
        return -EBUSY;
    }

    reg_start = (reg - (reg % 4));
    reg_len = (reg + len - reg_start) % 4 == 0 ? 
        (reg + len - reg_start) : ((((reg + len - reg_start) / 4) + 1) * 4);

    reg_data = (uint8_t*)kmalloc(reg_len, GFP_KERNEL);
    if (!reg_data) {
        mca_log_err("The Pointer is NULL\n");
        return -ENOMEM;
    }

    mutex_lock(&sc->i2c_rw_lock);
    if (reg_start != reg || reg_len != len) {
        ret = __sc9625_read_block(sc, reg_start, reg_data, reg_len);
    }
    memcpy(reg_data + (reg % 4), data, len);
    ret |= __sc9625_write_block(sc, reg_start, reg_data, reg_len);
    mutex_unlock(&sc->i2c_rw_lock);

    kfree(reg_data);
    return ret;
}
//-------------------------------------------------------------

//-------------------sc9625 system interface-------------------
int sc9625_tx_get_int(struct sc9625 *sc, uint32_t *txint)
{
	int ret = 0;
	ret = readCust2(sc, cust_tx.IntFlag, txint);
	if (ret < 0)
	{
		mca_log_err("sc962x get Tx interrupt fail\n");
	} else {
		mca_log_info("sc962x Tx interrupt=0x%X\n", *txint);
	}
	return ret;
}

static int sc9625_tx_set_cmd(struct sc9625 *sc, TX_CMD cmd)
{
    int ret = 0;
    ret = writeCust(sc, cust_tx.Cmd, &cmd);
    if (ret < 0) {
        mca_log_err("tx set cmd=0x%X failed\n", cmd);
        return ret;
    }
    return ret;
}

int sc9625_tx_clr_int(struct sc9625 *sc, uint32_t txint)
{
	int ret = 0;
	ret = writeCust(sc, cust_tx.IntClr, &txint);
	if (ret < 0)
	{
		mca_log_err("sc962x clear tx interrupt 0x%X fail\n", txint);
		return ret;
	}
	ret = sc9625_tx_set_cmd(sc, TX_CLEAR_INT);
	if (ret < 0)
	{
		mca_log_err("sc962x set cmd fail\n");
	} else {
		mca_log_info("sc962x clear Tx interrupt=0x%X\n", txint);
	}
	return ret;
}

static int sc9625_check_i2c(struct sc9625 *sc)
{
    int ret = 0;
    uint8_t tmp = 0x55;

    ret = writeCust(sc, cust_rx.iic_check, &tmp);
    if (ret < 0) {
        mca_log_err("sc9625 write iic_check fail\n");
        return ret;
    }

    mdelay(20);

    ret = readCust(sc, cust_rx.iic_check, &tmp);
    if (ret < 0) {
        mca_log_err("sc9625 read iic_check fail\n");
        return ret;
    }

    if (tmp == 0x55) {
        mca_log_info("i2c check ok!\n");
    } else {
        mca_log_err("i2c check failed!\n");
        return -1;
    }

    return ret;
}

static int sc9625_tx_enable(struct sc9625 *sc, bool enable)
{
	int ret = 0;

    if (enable) {
        ret = sc9625_tx_set_cmd(sc, TX_ENABLE);
        if (ret) {
            mca_log_err("enter TX mode fail\n");
        } else {
            mca_log_info("enter TX mode success\n");
        }
    } else {
        ret = sc9625_tx_set_cmd(sc, TX_DISABLE);
        if (ret) {
            mca_log_err("exit TX mode fail\n");
        } else {
            mca_log_info("exit TX mode success\n");
        }
    }

	return ret;
}

static int sc9625_get_ss_reg_value(struct sc9625 *sc, int *ss_value)
{
	int ret = 0;
    uint8_t ss = 0;

	ret = readCust2(sc, cust_tx.ssVal, &ss);
    if (ret < 0) {
        mca_log_err("tx get ss failed\n");
        *ss_value = 0;
        return ret;
    }

    *ss_value = ss;
    sc->reverse_ss = *ss_value;
    mca_log_info("tx get ss :%d\n", sc->reverse_ss);
	return ret;
}

static int sc9625_tx_get_voltage(struct sc9625 *sc, uint16_t *volt)
{
	int ret = 0;

	ret = sc9625_tx_set_cmd(sc, TX_REFRESH);
	ret = readCust2(sc, cust_tx.Voltage, volt);
    if (ret < 0) {
        mca_log_err("tx get voltage fail\n");
    } else {
        mca_log_info("tx get voltage :%d\n", *volt);
    }

	return ret;
}

static int sc9625_tx_get_current(struct sc9625 *sc, uint16_t *curr)
{
	int ret = 0;

	ret = sc9625_tx_set_cmd(sc, TX_REFRESH);
	ret = readCust2(sc, cust_tx.Current, curr);
    if (ret < 0) {
        mca_log_err("tx get current fail\n");
    } else {
        mca_log_info("tx get current :%d\n", *curr);
    }

	return ret;
}

static int sc9625_tx_get_tdie(struct sc9625 *sc, uint16_t *tdie)
{
	int ret = 0;

	ret = sc9625_tx_set_cmd(sc, TX_REFRESH);
	ret = readCust2(sc, cust_tx.Tdie, tdie);
    if (ret < 0) {
        mca_log_err("tx get tdie fail\n");
    } else {
        mca_log_info("tx get tdie :%d\n", *tdie);
    }

	return ret;
}

static int sc9625_tx_get_pen_soc(struct sc9625 *sc)
{
    int ret = 0;
    uint8_t battery_level = 0;

    ret = readCust2(sc, cust_tx.BATTERY_LEVEL, &battery_level);
    if (ret < 0) {
        mca_log_err("sc9625 get data of pen battery level fail\n");
        sc->reverse_pen_soc = 255;
        sc->reverse_pen_full = 0;
        return ret;
    } else if ((battery_level == 0) && (!sc->pen_soc_update)) {
        mca_log_err("battery level is 0 when soc not update\n");
        sc->reverse_pen_soc = 255;
        sc->reverse_pen_full = 0;
        return ret;
    } else {
        mca_log_info("sc9625 get data of pen battery level: 0x%02x\n", battery_level);
    }

    sc->reverse_pen_soc = battery_level & 0x7F;
    sc->reverse_pen_full = (battery_level & 0x80) >> 7;
    mca_log_info("sc9625 get pen soc :%d, pen full: %d\n", sc->reverse_pen_soc, sc->reverse_pen_full);

    if ((sc->reverse_pen_soc > 100) || (sc->reverse_pen_soc < 0))
        sc->reverse_pen_soc = 255;

    return ret;
}

static int sc9625_tx_get_pen_ble_mac(struct sc9625 *sc)
{
    int ret = 0;
    uint8_t mac_buf1[2] = { 0 };
    uint8_t mac_buf2[4] = { 0 };

    ret = readCust2(sc, cust_tx.BLE_MAC1, mac_buf1);
    if (ret < 0) {
        mca_log_err("sc9625 get data of pen mac1 fail\n");
        return ret;
    }

    ret = readCust2(sc, cust_tx.BLE_MAC2, mac_buf2);
    if (ret < 0) {
        mca_log_err("sc9625 get data of pen mac2 fail\n");
        return ret;
    }

    mca_log_info("sc9625 get raw data of pen mac: 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\n",
        mac_buf1[0], mac_buf1[1], mac_buf2[0], mac_buf2[1], mac_buf2[2], mac_buf2[3]);

    if (mac_buf2[0] == 0 && mac_buf2[1] == 0 && mac_buf2[2] == 0 && mac_buf2[3] == 0) {
        mca_log_err("sc9625 get ble mac fail\n");
        return -1;
    }

    //get pen mac
    sc->pen_mac_data[0] = mac_buf1[0];
    sc->pen_mac_data[1] = mac_buf1[1];
    sc->pen_mac_data[2] = mac_buf2[0];
    sc->pen_mac_data[3] = mac_buf2[1];
    sc->pen_mac_data[4] = mac_buf2[2];
    sc->pen_mac_data[5] = mac_buf2[3];
    mca_log_info("sc9625 get pen mac addr: 0x%02x:0x%02x:0x%02x 0x%02x:0x%02x:0x%02x\n",
        sc->pen_mac_data[0], sc->pen_mac_data[1], sc->pen_mac_data[2],
        sc->pen_mac_data[3], sc->pen_mac_data[4], sc->pen_mac_data[5]);

    return ret;
}

static int sc9625_get_fwver(struct sc9625 *sc, uint32_t *fw_ver)
{
    int ret;

    ret = readCust2(sc, cust_rx.FwVer, fw_ver);
    if (ret) {
        mca_log_err("sc9625 get fw ver fail\n");
    }

    return ret;
}

static int sc9625_get_image_fwver(const unsigned char *firmware, const uint32_t len, uint32_t *image_ver)
{
    if (len < 0x200) {
        mca_log_err("Firmware image length is too short\n");
        return -1;
    }

    *image_ver = (uint32_t)firmware[0X100 + 4] & 0x00FF;
    *image_ver |= ((uint32_t)firmware[0X100 + 5] & 0x00FF) << 8;
    *image_ver |= ((uint32_t)firmware[0X100 + 6] & 0x00FF) << 16;
    *image_ver |= ((uint32_t)firmware[0X100 + 7] & 0x00FF) << 24;

    return 0;
}

static int sc9625_mtp_chip_crc_check(struct sc9625 *sc)
{
	int ret = 0;
	bool crc_check = false;

	crc_check = mtp_chip_crc_check(sc, 32 * 1024);
	if (!crc_check) {
		sc->wls_fw_data->fw_boot_id = 0xfe;
		sc->wls_fw_data->fw_rx_id = 0xfe;
		sc->wls_fw_data->fw_tx_id = 0xfe;
		mca_log_info("mtp_chip_crc_check:fail, update fw version: 0xfe.\n");
	} else {
        msleep(100);
		mca_log_info("mtp_chip_crc_check:success.\n");
	}
	return ret;
}
//--------------------------------------------------------------

//--------------------------ops functions------------------------
static int sc9625_download_fw_from_bin(void *data)
{
    int ret = 0;
    struct sc9625 *sc = (struct sc9625 *)data;
    uint32_t cur_ver = 0x00;
    uint32_t bin_image_ver = 0;

    ret = sc9625_get_fwver(sc, &cur_ver);
    if (ret < 0) {
        mca_log_err("sc962x get firmware version before mtp download from bin failed\n");
        return ret;
    }
    ret = sc9625_get_image_fwver(sc->fw_bin, sc->fw_bin_length, &bin_image_ver);
    mca_log_info("sc9625 current firmware version=%x, new bin firmware version=%x\n", cur_ver,
        bin_image_ver);

    mca_log_info("sc9625 start download mtp from bin\n");
    //need check
    ret = sc9625_mtp_program(sc, sc->fw_bin, sc->fw_bin_length);
    if (ret < 0) {
        sc->wls_fw_data->fw_boot_id = 0xfe;
        sc->wls_fw_data->fw_rx_id = 0xfe;
        sc->wls_fw_data->fw_tx_id = 0xfe;
        mca_log_err("sc9625_mtp_program: from bin failed, update fw version: 0xfe.\n");
    } else {
        sc9625_mtp_chip_crc_check(sc);
        mca_log_info("sc9625 download mtp from bin successfully\n");
    }

    return ret;
}

static int sc9625_set_fw_bin(const char *buf, int count, void *data)
{
    struct sc9625 *chip = (struct sc9625 *)data;
    static uint16_t total_length;
    static uint8_t serial_number;
    static uint8_t fw_area;

    mca_log_info("buf:%s, count:%d\n", buf, count);
    if (strncmp("length:", buf, 7) == 0) {
        if (kstrtou16(buf+7, 10, &total_length))
            return -EINVAL;
        chip->fw_bin_length = total_length;
        serial_number = 0;
        mca_log_info("total_length:%d, serial_number:%d\n", total_length, serial_number);
    } else if (strncmp("area:", buf, 5) == 0) {
        if (kstrtou8(buf+5, 10, &fw_area))
            return -EINVAL;
        mca_log_info("area:%d\n", fw_area);
    } else {
        memcpy((chip->fw_bin + serial_number * count), buf, count);
        serial_number++;
        mca_log_info("serial_number:%d, count:%d\n", serial_number, count);
    }
    return count;
}

static int sc9625_check_firmware_state(bool *update, void *data)
{
    int ret = 0;
    struct sc9625 *chip = (struct sc9625 *)data;
    uint32_t fw_ver = 0, image_ver = 0;

    if (sc9625_check_i2c(chip)) {
        ret = -1;
        goto err;
    }

    ret = sc9625_get_fwver(chip, &fw_ver);
    if (ret)
        goto err;

    ret = sc9625_get_image_fwver(chip->fw_data_ptr, chip->fw_data_size, &image_ver);
    if (ret < 0)
        goto err;

    chip->wls_fw_data->fw_boot_id = fw_ver >> 16;
    chip->wls_fw_data->fw_rx_id = fw_ver >> 8;
    chip->wls_fw_data->fw_tx_id = fw_ver;

    mca_log_info("ic fw version: %02x.%02x.%02x, img version: 0x%x\n",
        chip->wls_fw_data->fw_boot_id, chip->wls_fw_data->fw_rx_id, chip->wls_fw_data->fw_tx_id, image_ver);

    if (fw_ver == 0 || fw_ver < image_ver) {
        mca_log_info("need update\n");
        *update = true;
    } else {
        mca_log_info("no need update\n");
        *update = false;
    }

    return ret;

err:
    mca_log_err("i2c error!\n");
    *update = false;
    return ret;
}

static int sc9625_get_fw_version_check(uint8_t *check_result, void *data)
{
    int ret = 0;
    struct sc9625 *sc = (struct sc9625 *)data;
    uint32_t fw_check = 0, fw_ver = 0, image_ver = 0;

    ret = readCust2(sc, cust_rx.firmware_check, &fw_check);
    ret |= sc9625_get_fwver(sc, &fw_ver);
    if (ret) {
        mca_log_err("sc9625 get fw check failed\n");
        return ret;
    }

    sc->wls_fw_data->fw_boot_id = fw_ver >> 16;
    sc->wls_fw_data->fw_rx_id = fw_ver >> 8;
    sc->wls_fw_data->fw_tx_id = fw_ver;

    ret = sc9625_get_image_fwver(sc->fw_data_ptr, sc->fw_data_size, &image_ver);

    if (fw_check == fw_ver && fw_ver >= image_ver)
        *check_result = BOOT_CHECK_SUCCESS | RX_CHECK_SUCCESS | TX_CHECK_SUCCESS;

    mca_log_info("sc9625 get fw check success, ver:0x%x, check:0x%x, res:0x%x\n",
        fw_ver, fw_check, *check_result);

    return ret;
}

static int sc9625_download_fw(void *data)
{
	int ret = 0;
    struct sc9625 *sc = (struct sc9625 *)data;
	uint32_t image_ver = 0;
	uint32_t cur_ver = 0x00;

    ret = sc9625_get_fwver(sc, &cur_ver);
	if (ret < 0) {
		mca_log_err("sc962x get firmware version before mtp download failed\n");
		return ret;
	}

	ret = sc9625_get_image_fwver(sc->fw_data_ptr, sc->fw_data_size, &image_ver);
	mca_log_info("sc9625 current firmware version=%x, new firmware version=%x\n", cur_ver, image_ver);

    mca_log_info("sc9625 start download mtp\n");
    //need check
    ret = sc9625_mtp_program(sc, sc->fw_data_ptr, sc->fw_data_size);
    if (ret < 0) {
        sc->wls_fw_data->fw_boot_id = 0xfe;
        sc->wls_fw_data->fw_rx_id = 0xfe;
        sc->wls_fw_data->fw_tx_id = 0xfe;
        mca_log_err("sc9625_mtp_program:fail, update fw version: 0xfe.\n");
    } else {
        sc9625_mtp_chip_crc_check(sc);
        mca_log_info("sc9625 download mtp successfully\n");
    }

	return ret;
}

static int sc9625_get_firmware_version(char *buf, void *data)
{
    struct sc9625 *chip = (struct sc9625 *)data;

    scnprintf(buf, 10, "%02x.%02x.%02x",
                chip->wls_fw_data->fw_boot_id, chip->wls_fw_data->fw_rx_id, chip->wls_fw_data->fw_tx_id);

    mca_log_info("rx firmware version: boot = 0x%x, rx = 0x%x, tx = 0x%x\n",
        chip->wls_fw_data->fw_boot_id, chip->wls_fw_data->fw_rx_id, chip->wls_fw_data->fw_tx_id);

    return 0;
}

static int sc9625_enable_reverse_chg(bool enable, void *data)
{
    struct sc9625 *chip = (struct sc9625 *)data;

    return sc9625_tx_enable(chip, enable);
}

static int sc9625_ops_check_i2c(void *data)
{
    int ret = 0;
    struct sc9625 *chip = (struct sc9625 *)data;

    ret = sc9625_check_i2c(chip);

    return ret;
}

static int sc9625_set_external_boost_enable(bool enable, void *data)
{
	int ret = 0;
	int en = !!enable;
	int gpio_enable_val = 0;
    struct sc9625 *chip = (struct sc9625 *)data;

	mca_log_info("set external boost enable:%d\n", enable);
	chip->reverse_boost_en = enable;

	if (enable) {
		if (gpio_is_valid(chip->reverse_txon_gpio)) {
			ret = gpio_request(chip->reverse_txon_gpio, "reverse-txon-gpio");
			if (ret)
				mca_log_err("request txon gpio [%d] failed\n", chip->reverse_txon_gpio);
			ret = gpio_direction_output(chip->reverse_txon_gpio, en);
			if (ret)
				mca_log_err("set txon gpio [%d] output failed\n", chip->reverse_txon_gpio);
			gpio_enable_val = gpio_get_value(chip->reverse_txon_gpio);
			mca_log_err("reverse txon gpio is %d\n", gpio_enable_val);
			gpio_free(chip->reverse_txon_gpio);
		}

		if (gpio_is_valid(chip->reverse_boost_gpio)) {
			ret = gpio_request(chip->reverse_boost_gpio, "reverse-boost-gpio");
			if (ret)
				mca_log_err("request boost gpio [%d] failed\n", chip->reverse_boost_gpio);
			ret = gpio_direction_output(chip->reverse_boost_gpio, en);
			if (ret)
				mca_log_err("set boost gpio [%d] output failed\n", chip->reverse_boost_gpio);
			gpio_free(chip->reverse_boost_gpio);
		}
	} else {
		if (gpio_is_valid(chip->reverse_boost_gpio)) {
			ret = gpio_request(chip->reverse_boost_gpio, "reverse-boost-gpio");
			if (ret)
				mca_log_err("request boost gpio [%d] failed\n", chip->reverse_boost_gpio);
			ret = gpio_direction_output(chip->reverse_boost_gpio, en);
			if (ret)
				mca_log_err("set boost gpio [%d] output failed\n", chip->reverse_boost_gpio);
			gpio_free(chip->reverse_boost_gpio);
		}

		if (gpio_is_valid(chip->reverse_txon_gpio)) {
			ret = gpio_request(chip->reverse_txon_gpio, "reverse-txon-gpio");
			if (ret)
				mca_log_err("request txon gpio [%d] failed\n", chip->reverse_txon_gpio);
			ret = gpio_direction_output(chip->reverse_txon_gpio, en);
			if (ret)
				mca_log_err("set txon gpio [%d] output failed\n", chip->reverse_txon_gpio);
			gpio_enable_val = gpio_get_value(chip->reverse_txon_gpio);
			mca_log_err("reverse txon gpio is %d\n", gpio_enable_val);
			gpio_free(chip->reverse_txon_gpio);
		}
	}
	return ret;
}

static int sc9625_get_tx_ss(int *ss, void *data)
{
    struct sc9625 *chip = (struct sc9625 *)data;
    int ret = 0;

    if ((!chip->hall3_online && !chip->hall4_online) || (!chip->reverse_boost_en))
        *ss = 0;
    else {
        ret = sc9625_get_ss_reg_value(chip, ss);
        if (ret < 0) {
            mca_log_err("get ss reg value failed\n");
            *ss = 0;
        }
    }

    return 0;
}

static int sc9625_get_pen_hall3(int *value, void *data)
{
    struct sc9625 *chip = (struct sc9625 *)data;

    if (chip->hall3_online)
        *value = 0;
    else
        *value = 1;

    return 0;
}

static int sc9625_get_pen_hall4(int *value, void *data)
{
    struct sc9625 *chip = (struct sc9625 *)data;

    if (chip->hall4_online)
        *value = 0;
    else
        *value = 1;

    return 0;
}

static int sc9625_get_pen_hall3_s(int *value, void *data)
{
    struct sc9625 *chip = (struct sc9625 *)data;

    *value = chip->hall3_s_val;

    return 0;
}

static int sc9625_get_pen_hall4_s(int *value, void *data)
{
    struct sc9625 *chip = (struct sc9625 *)data;

    *value = chip->hall4_s_val;

    return 0;
}

static int sc9625_get_pen_hall_ppe_n(int *value, void *data)
{
    struct sc9625 *chip = (struct sc9625 *)data;

    *value = chip->hall_ppe_n_val;

    return 0;
}

static int sc9625_get_pen_hall_ppe_s(int *value, void *data)
{
    struct sc9625 *chip = (struct sc9625 *)data;

    *value = chip->hall_ppe_s_val;

    return 0;
}

static int sc9625_get_pen_place_err(int *value, void *data)
{
    struct sc9625 *chip = (struct sc9625 *)data;

    *value = chip->pen_place_err;

    return 0;
}

static int sc9625_set_pen_place_err(int err, void *data)
{
    struct sc9625 *chip = (struct sc9625 *)data;

    chip->pen_place_err = err;

    return 0;
}

static int sc9625_get_tx_vout(int *value, void *data)
{
    struct sc9625 *chip = (struct sc9625 *)data;
    int ret = 0;
    u16 tx_vout = 0;

    if ((!chip->hall3_online && !chip->hall4_online) || (!chip->reverse_boost_en))
        *value = 0;
    else {
        ret = sc9625_tx_get_voltage(chip, &tx_vout);
        if (ret < 0) {
            mca_log_err("get tx vout failed\n");
            *value  = 0;
        } else {
            *value = tx_vout;
        }
    }
    return 0;
}

static int sc9625_get_tx_iout(int *value, void *data)
{
    struct sc9625 *chip = (struct sc9625 *)data;
    int ret = 0;
    u16 tx_iout = 0;


    if ((!chip->hall3_online && !chip->hall4_online) || (!chip->reverse_boost_en))
        *value = 0;
    else {
        ret = sc9625_tx_get_current(chip, &tx_iout);
        if (ret < 0) {
            mca_log_err("get tx iout failed\n");
            *value  = 0;
        } else {
            *value = tx_iout;
        }
    }
    return 0;
}

static int sc9625_get_tx_tdie(int *temp, void *data)
{
    struct sc9625 *chip = (struct sc9625 *)data;
    int ret = 0;
    u16 tx_tdie = 0;


    if ((!chip->hall3_online && !chip->hall4_online) || (!chip->reverse_boost_en))
        *temp = 0;
    else {
        ret = sc9625_tx_get_tdie(chip, &tx_tdie);
        if (ret < 0) {
            mca_log_err("get tx tdie failed\n");
            *temp  = 0;
        } else {
            *temp = tx_tdie;
        }
    }
    return 0;
}

static int sc9625_get_pen_soc(int *soc, void *data)
{
    struct sc9625 *chip = (struct sc9625 *)data;
    int ret = 0;

    if ((!chip->hall3_online && !chip->hall4_online) || (!chip->reverse_boost_en))
        *soc = 255;
    else {
        ret = sc9625_tx_get_pen_soc(chip);
        if (ret < 0) {
            mca_log_err("get pen soc failed\n");
            *soc  = 255;
        } else {
            *soc = chip->reverse_pen_soc;
        }
    }
    return 0;
}

static int sc9625_get_pen_mac(u64 *mac, void *data)
{
    struct sc9625 *sc = (struct sc9625 *)data;
    int ret = 0;
    u64 mac_l = 0;
    u64 mac_h = 0;

    if ((!sc->hall3_online && !sc->hall4_online) || (!sc->reverse_boost_en))
        *mac = 0;
    else {
        ret = sc9625_tx_get_pen_ble_mac(sc);
        if (ret < 0) {
            mca_log_err("get pen mac failed\n");
            *mac  = 0;
        } else {
            mac_l = (u64)(sc->pen_mac_data[0] | sc->pen_mac_data[1] << 8 | sc->pen_mac_data[2] << 16 |
            sc->pen_mac_data[3] << 24);
            mac_h = (u64)(sc->pen_mac_data[4] | sc->pen_mac_data[5] << 8);
            // set high 32bit to zero
            mac_l <<= 32;
            mac_l >>= 32;
            *mac = (u64)(mac_h << 32 | mac_l);
        }
    }
    return 0;
}

static int sc9625_get_pen_full_flag(int *pen_full, void *data)
{
    struct sc9625 *chip = (struct sc9625 *)data;
    int ret = 0;

    if ((!chip->hall3_online && !chip->hall4_online) || (!chip->reverse_boost_en))
        *pen_full = 0;
    else {
        ret = sc9625_tx_get_pen_soc(chip);
        if (ret < 0) {
            mca_log_err("get pen full flag failed\n");
            *pen_full  = 0;
        } else {
            *pen_full = chip->reverse_pen_full;
        }
    }
    return 0;
}

static struct platform_class_wireless_ops sc9625_wls_ops = {
    .wls_enable_reverse_chg = sc9625_enable_reverse_chg,
    .wls_check_i2c_is_ok = sc9625_ops_check_i2c,
    .wls_enable_rev_fod = NULL,
    .wls_get_fw_version = sc9625_get_firmware_version,
    .wls_download_fw = sc9625_download_fw,
    .wls_get_fw_version_check = sc9625_get_fw_version_check,
    .wls_check_firmware_state = sc9625_check_firmware_state,
    .wls_set_fw_bin = sc9625_set_fw_bin,
    .wls_download_fw_from_bin = sc9625_download_fw_from_bin,
    .wls_set_external_boost_enable = sc9625_set_external_boost_enable,
    .wls_get_poweroff_err_code = NULL,
    .wls_get_tx_vout = sc9625_get_tx_vout,
    .wls_get_tx_iout = sc9625_get_tx_iout,
    .wls_get_tx_tdie = sc9625_get_tx_tdie,
    .wls_get_tx_ss = sc9625_get_tx_ss,
    .wls_get_pen_mac = sc9625_get_pen_mac,
    .wls_get_pen_soc = sc9625_get_pen_soc,
    .wls_get_pen_full_flag = sc9625_get_pen_full_flag,
    .wls_get_pen_hall3 = sc9625_get_pen_hall3,
    .wls_get_pen_hall4 = sc9625_get_pen_hall4,
    .wls_get_pen_hall3_s = sc9625_get_pen_hall3_s,
    .wls_get_pen_hall4_s = sc9625_get_pen_hall4_s,
    .wls_get_pen_hall_ppe_n = sc9625_get_pen_hall_ppe_n,
    .wls_get_pen_hall_ppe_s = sc9625_get_pen_hall_ppe_s,
    .wls_get_pen_place_err = sc9625_get_pen_place_err,
    .wls_set_pen_place_err = sc9625_set_pen_place_err,
};

static int sc9625_parse_fw_data(struct sc9625 *chip)
{
    int ret = 0;

    switch (chip->project_vendor) {
    case WLS_CHIP_VENDOR_SC9625:
        chip->fw_data_ptr = SC9625_FIRMWARE[chip->fw_version_index];
        chip->fw_data_size = sizeof(SC9625_FIRMWARE[chip->fw_version_index]);
        break;
    default:
        chip->fw_data_ptr = SC9625_FIRMWARE[chip->fw_version_index];
        chip->fw_data_size = sizeof(SC9625_FIRMWARE[chip->fw_version_index]);
        break;
    }

    mca_log_info("cur fw_data is:[0]:%02x; [1]:%02x; [2]:%02x\n",
                chip->fw_data_ptr[0], chip->fw_data_ptr[1], chip->fw_data_ptr[2]);
    return ret;
}

static int sc9625_parse_dt(struct sc9625 *chip)
{
    struct device_node *node = chip->dev->of_node;

    if (!node) {
        mca_log_err("No DT data Failing Probe\n");
        return -EINVAL;
    }

    (void)mca_parse_dts_u32(node, "project_vendor", &chip->project_vendor, 0);
    (void)mca_parse_dts_u32(node, "rx_role", &chip->rx_role, 0);
    (void)mca_parse_dts_u32(node, "fw_version_index", &chip->fw_version_index, 0);

    mca_log_err("project_vendor=%d, rx_role=%d, fw_version_index=%d", 
        chip->project_vendor, chip->rx_role, chip->fw_version_index);

    sc9625_parse_fw_data(chip);

    chip->irq_gpio = of_get_named_gpio(node, "rx-int", 0);
    if (!gpio_is_valid(chip->irq_gpio)) {
        mca_log_err("fail_irq_gpio %d\n", chip->irq_gpio);
        return -EINVAL;
    }

    chip->hall3_gpio = of_get_named_gpio(node, "hall-int3", 0);
    if (!gpio_is_valid(chip->hall3_gpio)) {
        mca_log_err("fail_hall3_gpio %d\n", chip->hall3_gpio);
        return -EINVAL;
    }

    chip->hall4_gpio = of_get_named_gpio(node, "hall-int4", 0);
    if (!gpio_is_valid(chip->hall4_gpio)) {
        mca_log_err("fail_hall4_gpio %d\n", chip->hall4_gpio);
        return -EINVAL;
    }

    chip->hall3_s_gpio = of_get_named_gpio(node, "hall-int3-s", 0);
    if (!gpio_is_valid(chip->hall3_s_gpio)) {
        mca_log_err("fail_hall3_s_gpio %d\n", chip->hall3_s_gpio);
        return -EINVAL;
    }

    chip->hall4_s_gpio = of_get_named_gpio(node, "hall-int4-s", 0);
    if (!gpio_is_valid(chip->hall4_s_gpio)) {
        mca_log_err("fail_hall4_s_gpio %d\n", chip->hall4_s_gpio);
        return -EINVAL;
    }

    chip->hall_ppe_n_gpio = of_get_named_gpio(node, "hall-ppe-n", 0);
    if (!gpio_is_valid(chip->hall_ppe_n_gpio)) {
        mca_log_err("fail_hall_ppe_n_gpio %d\n", chip->hall_ppe_n_gpio);
        return -EINVAL;
    }

    chip->hall_ppe_s_gpio = of_get_named_gpio(node, "hall-ppe-s", 0);
    if (!gpio_is_valid(chip->hall_ppe_s_gpio)) {
        mca_log_err("fail_hall_ppe_s_gpio %d\n", chip->hall_ppe_s_gpio);
        return -EINVAL;
    }

    chip->reverse_txon_gpio = of_get_named_gpio(node, "reverse-txon-gpio", 0);
    if ((!gpio_is_valid(chip->reverse_txon_gpio))) {
        mca_log_err("fail_get_reverse_txon_gpio %d\n", chip->reverse_txon_gpio);
        return -EINVAL;
    }

    chip->reverse_boost_gpio = of_get_named_gpio(node, "reverse-boost-gpio", 0);
    if ((!gpio_is_valid(chip->reverse_boost_gpio))) {
        mca_log_err("fail_get_reverse_boost_gpio %d\n", chip->reverse_boost_gpio);
        return -EINVAL;
    }

    return 0;
}

static int_map_t tx_irq_map[] = {
    DECL_INTERRUPT_MAP(INT_PING, RTX_INT_PING),
    DECL_INTERRUPT_MAP(INT_POWER_TRANS, RTX_INT_GET_RX),
    DECL_INTERRUPT_MAP(INT_CEP_TIMEOUT, RTX_INT_CEP_TIMEOUT),
    DECL_INTERRUPT_MAP(INT_REMOVE_POEWR, RTX_INT_EPT),
    DECL_INTERRUPT_MAP(INT_DET_AC, RTX_INT_GET_TX),
    DECL_INTERRUPT_MAP(INT_FOD, RTX_INT_FOD),
    DECL_INTERRUPT_MAP(INT_DET_RX, RTX_INT_TX_DET_RX),
    DECL_INTERRUPT_MAP(INT_TX_CONFIG_PKT, RTX_INT_TX_CONFIG),
    DECL_INTERRUPT_MAP(INT_TX_CHS_UPDATE, RTX_INT_TX_CHS_UPDATE),
    DECL_INTERRUPT_MAP(INT_TX_BLE_CONNECT, RTX_INT_TX_BLE_CONNECT),
};

static void sc9625_ppe_hall_interrupt_work(struct work_struct *work)
{
    struct sc9625 *chip = container_of(work, struct sc9625, ppe_hall_irq_work.work);
    int hall3_s_gpio_val = 1;
    int hall4_s_gpio_val = 1;
    int hall_ppe_n_gpio_val = 1;
    int hall_ppe_s_gpio_val = 1;

    if (gpio_is_valid(chip->hall3_s_gpio)) {
        hall3_s_gpio_val = gpio_get_value(chip->hall3_s_gpio);
        chip->hall3_s_val = hall3_s_gpio_val;
        mca_log_info("hall3_s gpio is %d \n", hall3_s_gpio_val);
    } else {
        mca_log_err("hall3_s irq gpio not provided\n");
        return;
    }

    if (gpio_is_valid(chip->hall4_s_gpio)) {
        hall4_s_gpio_val = gpio_get_value(chip->hall4_s_gpio);
        chip->hall4_s_val = hall4_s_gpio_val;
        mca_log_info("hall4_s gpio is %d \n", hall4_s_gpio_val);
    } else {
        mca_log_err("hall4_s irq gpio not provided\n");
        return;
    }

    if (gpio_is_valid(chip->hall_ppe_n_gpio)) {
        hall_ppe_n_gpio_val = gpio_get_value(chip->hall_ppe_n_gpio);
        chip->hall_ppe_n_val = hall_ppe_n_gpio_val;
        mca_log_info("hall_ppe_n gpio is %d \n", hall_ppe_n_gpio_val);
    } else {
        mca_log_err("hall_ppe_n irq gpio not provided\n");
        return;
    }

    if (gpio_is_valid(chip->hall_ppe_s_gpio)) {
        hall_ppe_s_gpio_val = gpio_get_value(chip->hall_ppe_s_gpio);
        chip->hall_ppe_s_val = hall_ppe_s_gpio_val;
        mca_log_info("hall_ppe_s gpio is %d \n", hall_ppe_s_gpio_val);
    } else {
        mca_log_err("hall_ppe_s irq gpio not provided\n");
        return;
    }

    if (!hall3_s_gpio_val || !hall4_s_gpio_val || !hall_ppe_n_gpio_val || !hall_ppe_s_gpio_val) {
        mca_log_info("ppe hall triger, pen place error\n");
        chip->pen_place_err = PPE_HALL;
    } else {
        chip->pen_place_err = PPE_NONE;
    }

    mca_strategy_func_process(STRATEGY_FUNC_TYPE_REV_WIRELESS,
        MCA_EVENT_WIRELESS_PEN_PPE_HALL_CHANGE, chip->pen_place_err);

    return;
}

static void sc9625_hall_interrupt_work(struct work_struct *work)
{
    struct sc9625 *chip = container_of(work, struct sc9625, hall_irq_work.work);
    int hall3_gpio_val = 1;
    int hall4_gpio_val = 1;

    if (gpio_is_valid(chip->hall3_gpio)) {
        hall3_gpio_val = gpio_get_value(chip->hall3_gpio);
        mca_log_info("hall3 gpio is %d \n", hall3_gpio_val);
    } else {
        mca_log_err("hall3 irq gpio not provided\n");
        return;
    }

    if (gpio_is_valid(chip->hall4_gpio)) {
        hall4_gpio_val = gpio_get_value(chip->hall4_gpio);
        mca_log_info("hall4 gpio is %d \n", hall4_gpio_val);
    } else {
        mca_log_err("hall4 irq gpio not provided\n");
        return;
    }

    if (!hall3_gpio_val || !hall4_gpio_val) {
        if (chip->hall3_online == 1 || chip->hall4_online == 1) {
            mca_log_info("hall3 or hall4 is already online, return\n");
            return;
        }
        mca_log_info("pen attached!\n");

        if (!hall3_gpio_val)
            chip->hall3_online = 1;
        if (!hall4_gpio_val)
            chip->hall4_online = 1;

        mca_log_info("hall3 online: %d, hall4 online: %d\n", chip->hall3_online, chip->hall4_online);

        mca_strategy_func_process(STRATEGY_FUNC_TYPE_REV_WIRELESS,
            MCA_EVENT_WIRELESS_PEN_HALL_CHANGE, chip->hall3_online << 3 | chip->hall4_online << 4);
    } else {
        if (chip->hall3_online == 0 && chip->hall4_online == 0) {
            mca_log_info("hall3 and hall4 are already offline, return \n");
            return;
        }
        chip->hall3_online = 0;
        chip->hall4_online = 0;
        chip->pen_soc_update = false;
        mca_log_info("pen dettached!\n");
        mca_strategy_func_process(STRATEGY_FUNC_TYPE_REV_WIRELESS,
            MCA_EVENT_WIRELESS_PEN_HALL_CHANGE, 0);
    }

    return;
}

static void sc9625_interrupt_work(struct work_struct *work)
{
    struct sc9625 *chip = container_of(work, struct sc9625, interrupt_work.work);
    int ret = 0;
    uint32_t regval;
    int i = 0;

    mutex_lock(&chip->wireless_chg_int_lock);

    ret = sc9625_tx_get_int(chip, &regval);
    if (ret < 0) {
        mca_log_err("get tx int flag fail\n");
        goto exit;
    }

    // clear intflag first
    sc9625_tx_clr_int(chip, regval);

    for (i = 0; i < ARRAY_SIZE(tx_irq_map); i++) {
        if (tx_irq_map[i].irq_regval & regval) {
            chip->proc_data.int_flag = tx_irq_map[i].irq_flag;
            mca_log_info("process irq flag: %d\n", chip->proc_data.int_flag);
            if (chip->proc_data.int_flag == RTX_INT_TX_CHS_UPDATE)
                chip->pen_soc_update = true;
            mca_strategy_func_process(STRATEGY_FUNC_TYPE_REV_WIRELESS,
                MCA_EVENT_WIRELESS_INT_CHANGE, chip->proc_data.int_flag);
        }
    }

exit:
    chip->proc_data.int_flag = 0;
    mutex_unlock(&chip->wireless_chg_int_lock);
    mca_log_info("already clear int and unlock mutex\n");
}

static irqreturn_t sc9625_interrupt_handler(int irq, void *dev_id)
{
    struct sc9625 *chip = dev_id;
    schedule_delayed_work(&chip->interrupt_work, msecs_to_jiffies(0));
    return IRQ_HANDLED;
}

static irqreturn_t sc9625_hall3_interrupt_handler(int irq, void *dev_id)
{
    struct sc9625 *chip = dev_id;
    schedule_delayed_work(&chip->hall_irq_work, msecs_to_jiffies(0));
    return IRQ_HANDLED;
}

static irqreturn_t sc9625_hall4_interrupt_handler(int irq, void *dev_id)
{
    struct sc9625 *chip = dev_id;
    schedule_delayed_work(&chip->hall_irq_work, msecs_to_jiffies(0));
    return IRQ_HANDLED;
}

static irqreturn_t sc9625_hall3_s_interrupt_handler(int irq, void *dev_id)
{
    struct sc9625 *chip = dev_id;
    schedule_delayed_work(&chip->ppe_hall_irq_work, msecs_to_jiffies(0));
    return IRQ_HANDLED;
}

static irqreturn_t sc9625_hall4_s_interrupt_handler(int irq, void *dev_id)
{
    struct sc9625 *chip = dev_id;
    schedule_delayed_work(&chip->ppe_hall_irq_work, msecs_to_jiffies(0));
    return IRQ_HANDLED;
}

static irqreturn_t sc9625_hall_ppe_n_interrupt_handler(int irq, void *dev_id)
{
    struct sc9625 *chip = dev_id;
    schedule_delayed_work(&chip->ppe_hall_irq_work, msecs_to_jiffies(0));
    return IRQ_HANDLED;
}

static irqreturn_t sc9625_hall_ppe_s_interrupt_handler(int irq, void *dev_id)
{
    struct sc9625 *chip = dev_id;
    schedule_delayed_work(&chip->ppe_hall_irq_work, msecs_to_jiffies(0));
    return IRQ_HANDLED;
}

static int sc9625_irq_init(struct sc9625 *chip)
{
    int ret = 0;

    if (gpio_is_valid(chip->irq_gpio)) {
        chip->irq = gpio_to_irq(chip->irq_gpio);
        if (chip->irq < 0) {
            mca_log_err("gpio_to_irq Fail!\n");
            goto fail_irq_gpio;
        }
    } else {
        mca_log_err("irq gpio not provided\n");
        goto fail_irq_gpio;
    }
    if (chip->irq) {
        ret = devm_request_threaded_irq(&chip->client->dev, chip->irq, NULL,
                sc9625_interrupt_handler,
                (IRQF_TRIGGER_FALLING | IRQF_ONESHOT),
                "sc9625_chg_stat_irq", chip);
        if (ret)
            mca_log_err("Failed irq = %d ret = %d\n", chip->irq, ret);
    }
    enable_irq_wake(chip->irq);

    if (gpio_is_valid(chip->hall3_gpio)) {
        chip->hall3_irq = gpio_to_irq(chip->hall3_gpio);
        if (chip->hall3_irq < 0) {
            mca_log_err("hall3 gpio_to_irq Fail!\n");
            goto fail_hall3_irq_gpio;
        }
    } else {
        mca_log_err("hall3 irq gpio not provided\n");
        goto fail_hall3_irq_gpio;
    }
    if (chip->hall3_irq) {
        ret = devm_request_threaded_irq(&chip->client->dev, chip->hall3_irq, NULL,
                sc9625_hall3_interrupt_handler,
                (IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING | IRQF_ONESHOT),
                "sc9625_hall3_irq", chip);
        if (ret)
            mca_log_err("Failed hall3 irq = %d ret = %d\n", chip->hall3_irq, ret);
    }
    enable_irq_wake(chip->hall3_irq);

    if (gpio_is_valid(chip->hall4_gpio)) {
        chip->hall4_irq = gpio_to_irq(chip->hall4_gpio);
        if (chip->hall4_irq < 0) {
            mca_log_err("hall4 gpio_to_irq Fail!\n");
            goto fail_hall4_irq_gpio;
        }
    } else {
        mca_log_err("hall4 irq gpio not provided\n");
        goto fail_hall4_irq_gpio;
    }
    if (chip->hall4_irq) {
        ret = devm_request_threaded_irq(&chip->client->dev, chip->hall4_irq, NULL,
                sc9625_hall4_interrupt_handler,
                (IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING | IRQF_ONESHOT),
                "sc9625_hall4_irq", chip);
        if (ret)
            mca_log_err("Failed hall4 irq = %d ret = %d\n", chip->hall4_irq, ret);
    }
    enable_irq_wake(chip->hall4_irq);
 
    if (gpio_is_valid(chip->hall3_s_gpio)) {
        chip->hall3_s_irq = gpio_to_irq(chip->hall3_s_gpio);
        if (chip->hall3_s_irq < 0) {
            mca_log_err("hall3_s gpio_to_irq Fail!\n");
            goto fail_hall3_s_irq_gpio;
        }
    } else {
        mca_log_err("hall3_s irq gpio not provided\n");
        goto fail_hall3_s_irq_gpio;
    }
    if (chip->hall3_s_irq) {
        ret = devm_request_threaded_irq(&chip->client->dev, chip->hall3_s_irq, NULL,
                sc9625_hall3_s_interrupt_handler,
                (IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING | IRQF_ONESHOT),
                "sc9625_hall3_s_irq", chip);
        if (ret)
            mca_log_err("Failed hall3_s irq = %d ret = %d\n", chip->hall3_s_irq, ret);
    }
    enable_irq_wake(chip->hall3_s_irq);

    if (gpio_is_valid(chip->hall4_s_gpio)) {
        chip->hall4_s_irq = gpio_to_irq(chip->hall4_s_gpio);
        if (chip->hall4_s_irq < 0) {
            mca_log_err("hall4_s gpio_to_irq Fail!\n");
            goto fail_hall4_s_irq_gpio;
        }
    } else {
        mca_log_err("hall4_s irq gpio not provided\n");
        goto fail_hall4_s_irq_gpio;
    }
    if (chip->hall4_s_irq) {
        ret = devm_request_threaded_irq(&chip->client->dev, chip->hall4_s_irq, NULL,
                sc9625_hall4_s_interrupt_handler,
                (IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING | IRQF_ONESHOT),
                "sc9625_hall4_s_irq", chip);
        if (ret)
            mca_log_err("Failed hall4_s irq = %d ret = %d\n", chip->hall4_s_irq, ret);
    }
    enable_irq_wake(chip->hall4_s_irq);

    if (gpio_is_valid(chip->hall_ppe_n_gpio)) {
        chip->hall_ppe_n_irq = gpio_to_irq(chip->hall_ppe_n_gpio);
        if (chip->hall_ppe_n_irq < 0) {
            mca_log_err("hall_ppe_n gpio_to_irq Fail!\n");
            goto fail_hall_ppe_n_irq_gpio;
        }
    } else {
        mca_log_err("hall_ppe_n irq gpio not provided\n");
        goto fail_hall_ppe_n_irq_gpio;
    }
    if (chip->hall_ppe_n_irq) {
        ret = devm_request_threaded_irq(&chip->client->dev, chip->hall_ppe_n_irq, NULL,
                sc9625_hall_ppe_n_interrupt_handler,
                (IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING | IRQF_ONESHOT),
                "sc9625_hall_ppe_n_irq", chip);
        if (ret)
            mca_log_err("Failed hall_ppe_n irq = %d ret = %d\n", chip->hall_ppe_n_irq, ret);
    }
    enable_irq_wake(chip->hall_ppe_n_irq);

    if (gpio_is_valid(chip->hall_ppe_s_gpio)) {
        chip->hall_ppe_s_irq = gpio_to_irq(chip->hall_ppe_s_gpio);
        if (chip->hall_ppe_s_irq < 0) {
            mca_log_err("hall_ppe_s gpio_to_irq Fail!\n");
            goto fail_hall_ppe_s_irq_gpio;
        }
    } else {
        mca_log_err("hall_ppe_s irq gpio not provided\n");
        goto fail_hall_ppe_s_irq_gpio;
    }
    if (chip->hall_ppe_s_irq) {
        ret = devm_request_threaded_irq(&chip->client->dev, chip->hall_ppe_s_irq, NULL,
                sc9625_hall_ppe_s_interrupt_handler,
                (IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING | IRQF_ONESHOT),
                "sc9625_hall_ppe_s_irq", chip);
        if (ret)
            mca_log_err("Failed hall_ppe_s irq = %d ret = %d\n", chip->hall_ppe_s_irq, ret);
    }
    enable_irq_wake(chip->hall_ppe_s_irq);

    return ret;

fail_hall_ppe_s_irq_gpio:
    gpio_free(chip->hall_ppe_s_gpio);
fail_hall_ppe_n_irq_gpio:
    gpio_free(chip->hall_ppe_n_gpio);
fail_hall4_s_irq_gpio:
    gpio_free(chip->hall4_s_gpio);
fail_hall3_s_irq_gpio:
    gpio_free(chip->hall3_s_gpio);
fail_hall4_irq_gpio:
    gpio_free(chip->hall4_gpio);
fail_hall3_irq_gpio:
    gpio_free(chip->hall3_gpio);
fail_irq_gpio:
    gpio_free(chip->irq_gpio);

    return ret;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 9))
static int sc9625_charger_probe(struct i2c_client *client)
#else
static int sc9625_charger_probe(struct i2c_client *client, const struct i2c_device_id *id)
#endif
{
    int ret = 0;
    struct sc9625 *chip;

    mca_log_info("sc9625 %s probe start!\n", SC9625_DRV_VERSION);

    chip = devm_kzalloc(&client->dev, sizeof(struct sc9625), GFP_KERNEL);
    if (!chip)
        return -ENOMEM;

    chip->wls_fw_data = devm_kzalloc(&client->dev, sizeof(struct wls_fw_parameters), GFP_KERNEL);
    if (!chip->wls_fw_data)
        return -ENOMEM;

    chip->regmap = devm_regmap_init_i2c(client, &sc9625_regmap_config);
    if (IS_ERR(chip->regmap)) {
        mca_log_err("failed to allocate register map\n");
        return PTR_ERR(chip->regmap);
    }

    chip->client = client;
    chip->dev = &client->dev;
    device_init_wakeup(&client->dev, TRUE);
    i2c_set_clientdata(client, chip);

    mutex_init(&chip->i2c_rw_lock);
    mutex_init(&chip->wireless_chg_int_lock);

    sc9625_parse_dt(chip);
    sc9625_irq_init(chip);

    INIT_DELAYED_WORK(&chip->interrupt_work, sc9625_interrupt_work);
    INIT_DELAYED_WORK(&chip->hall_irq_work, sc9625_hall_interrupt_work);
    INIT_DELAYED_WORK(&chip->ppe_hall_irq_work, sc9625_ppe_hall_interrupt_work);

    ret = platform_class_wireless_register_ops(chip->rx_role, chip, &sc9625_wls_ops);
    if (ret) {
        mca_log_err("register ops fail\n");
        goto err_dev;
    }

    // notify pen presence after device is powered on
    mca_log_err("check pen attachment after 14s\n");
    schedule_delayed_work(&chip->hall_irq_work, msecs_to_jiffies(14000));

    // update chip->hall3_s_val, chip->hall4_s_val to correspond to gpio after powering on
    schedule_delayed_work(&chip->ppe_hall_irq_work, msecs_to_jiffies(0));

    mca_log_err("sc9625 probe success!\n");
    return ret;

err_dev:
    mca_log_err("sc9625 probe failed!\n");
    return 0;
}

static int sc9625_suspend(struct device *dev)
{
    struct sc9625 *chip = dev_get_drvdata(dev);

    mca_log_info("Suspend successfully!");
    if (device_may_wakeup(dev))
        enable_irq_wake(chip->irq);

    return 0;
}

static int sc9625_resume(struct device *dev)
{
    struct sc9625 *chip = dev_get_drvdata(dev);

    mca_log_info("Resume successfully!");
    if (device_may_wakeup(dev))
        disable_irq_wake(chip->irq);

    return 0;
}

static const struct dev_pm_ops sc9625_pm_ops = {
    .resume     = sc9625_resume,
    .suspend    = sc9625_suspend,
};

static void sc9625_charger_remove(struct i2c_client *client)
{
    struct sc9625 *chip = i2c_get_clientdata(client);

    mutex_destroy(&chip->i2c_rw_lock);
    mutex_destroy(&chip->wireless_chg_int_lock);

    if (chip->irq_gpio > 0)
        gpio_free(chip->irq_gpio);
}

static void sc9625_charger_shutdown(struct i2c_client *client)
{
    return;
}

static struct of_device_id sc9625_charger_match_table[] = {
    {
        .compatible = "sc,sc9625-wireless-charger",
        .data = NULL,
    },
    {},
};
MODULE_DEVICE_TABLE(of, sc9625_charger_match_table);

static const struct i2c_device_id sc9625_id[] = {
    {"sc9625", 0},
    {},
};

static struct i2c_driver sc9625_wireless_charger_driver = {
    .driver     = {
        .name   = "sc-wireless-charger",
        .owner  = THIS_MODULE,
        .of_match_table = of_match_ptr(sc9625_charger_match_table),
        .pm     = &sc9625_pm_ops,
    },
    .probe      = sc9625_charger_probe,
    .remove     = sc9625_charger_remove,
    .shutdown   = sc9625_charger_shutdown,
    .id_table	= sc9625_id,
};

module_i2c_driver(sc9625_wireless_charger_driver);

MODULE_DESCRIPTION("SC SC9625 Wireless Charge Driver");
MODULE_AUTHOR("tianye9@xiaomi.com");
MODULE_LICENSE("GPL v2");