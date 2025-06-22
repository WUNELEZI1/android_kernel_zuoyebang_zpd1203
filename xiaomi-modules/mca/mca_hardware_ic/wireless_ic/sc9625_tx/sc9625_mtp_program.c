// SPDX-License-Identifier: GPL-2.0
/*
 * sc9625_mtp_program.c
 *
 * wireless TX ic driver
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

#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
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
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>
#include <linux/sched/clock.h>
#include <uapi/linux/sched/types.h>

#include "inc/sc9625_reg.h"
#include "inc/sc9625_mtp_program.h"
#include <mca/common/mca_log.h>

#ifndef MCA_LOG_TAG
#define MCA_LOG_TAG "sc9625_mtp_program"
#endif

#define MTP_SIZE                32 * 1024
#define MTP_SECTOR              256
//------------------- i2c api -------------------
static int __sc9625_read_block(struct sc9625 *sc, uint16_t reg, uint8_t length, uint8_t *data)
{
    int ret;

    ret = regmap_raw_read(sc->regmap, reg, data, length);
    if (ret) {
        mca_log_err("i2c read fail: can't read from reg 0x%04X\n", reg);
        return -EIO;
    }

    return 0;
}

static int __sc9625_write_block(struct sc9625 *sc, uint16_t reg, uint8_t length, uint8_t *data)
{
    int ret;

    ret = regmap_raw_write(sc->regmap, reg, data, length);
    if (ret) {
        mca_log_err("i2c write fail: can't write 0x%04X: %d\n", reg, ret);
        return -EIO;
    }

    return 0;
}

static int mtp_read_block(struct sc9625 *sc, uint16_t reg, uint8_t *data, uint8_t len)
{
    int ret;

    mutex_lock(&sc->i2c_rw_lock);
    ret = __sc9625_read_block(sc, reg, len, data);
    mutex_unlock(&sc->i2c_rw_lock);

    return ret;
}

static int mtp_write_block(struct sc9625 *sc, uint16_t reg, uint8_t *data, uint8_t len)
{
    int ret;

    mutex_lock(&sc->i2c_rw_lock);
    ret = __sc9625_write_block(sc, reg, len, data);
    mutex_unlock(&sc->i2c_rw_lock);

    return ret;
}

static int mtp_read_byte(struct sc9625 *sc, uint16_t reg, uint8_t *data)
{
    int ret;

    mutex_lock(&sc->i2c_rw_lock);
    ret = __sc9625_read_block(sc, reg, 1, data);
    mutex_unlock(&sc->i2c_rw_lock);

    return ret;
}

static int mtp_write_byte(struct sc9625 *sc, uint16_t reg, uint8_t data)
{
    int ret;

    mutex_lock(&sc->i2c_rw_lock);
    ret = __sc9625_write_block(sc, reg, 1, &data);
    mutex_unlock(&sc->i2c_rw_lock);

    return ret;
}

static uint32_t endian_conversion(uint32_t value)
{
    return (uint32_t)(((value & 0xFF) << 24) | ((value & 0xFF00) << 8) | ((value & 0xFF0000) >> 8) | ((value & 0xFF000000) >> 24));
}

static uint8_t sc9625_func_crc8(uint8_t data, uint8_t crc_init)
{
    uint32_t polynomial = 0x39;
    uint32_t crc_temp = 0;
    uint8_t i = 0;

    crc_temp = data ^ crc_init;
    for (i = 0; i < 8; i++) {
        crc_temp = (crc_temp & 0x80) ? 
            ((crc_temp << 1) ^ polynomial) : (crc_temp << 1);
    }

    return (uint8_t)crc_temp;
}

static uint32_t sc9625_func_crc32(uint32_t data, uint32_t crc_init)
{
    uint32_t crc_poly = 0x04c11db7;
    uint8_t i = 0;


    for(i = 0; i < 32; i++) {
        crc_init = (crc_init << 1) ^ ((((crc_init >> 31) & 0x01) ^ ((data >> i) & 0x01))
                        == 0x01 ? 0xFFFFFFFF & crc_poly : 0x00000000 & crc_poly);
    }

    return (uint32_t)crc_init;
}

static uint8_t sc9625_get_crc8(uint8_t *data)
{
    uint8_t crc_data = 0xFF;
    uint8_t i;

    for (i = 0; i < 16; i++) {
        crc_data = sc9625_func_crc8(data[i], crc_data);
    }

    return crc_data;
}


static bool dig_tm_status(struct sc9625 *sc)
{
    int ret;
    SC9625_tm_st_e tm_st;

    ret = mtp_read_byte(sc, SC9625_TM_ST, &tm_st.value);
    if (ret < 0) {
        mca_log_err("read 0xFF7F fail\n");
        return false;
    }

    return tm_st.dig_tm ? true : false;
}

static int dig_tm_entry(struct sc9625 *sc, bool enable)
{
    int ret;
    int f = 0;

    if (enable) {
        if (dig_tm_status(sc)) {
            return 0;
        }

        ret = mtp_write_byte(sc, SC9625_PASSWD, SC9625_PASSWD1);
        ret |= mtp_write_byte(sc, SC9625_PASSWD, SC9625_PASSWD2);
        ret |= mtp_write_byte(sc, SC9625_PASSWD, SC9625_PASSWD3);
        ret |= mtp_write_byte(sc, SC9625_PASSWD, SC9625_PASSWD4);
        if (ret < 0) {
            mca_log_err("enter dig tm fail\n");
            return ret;
        }
        for (f = 0; f < 100; f++) {
            if (dig_tm_status(sc)) {
                mca_log_info("enter dig tm success\n");
                return 0;
            }
            msleep(10);
        }
        ret = -EIO;
    } else {
        ret = mtp_write_byte(sc, SC9625_PASSWD, SC9625_PASSWD0);
        mca_log_info("exit dig tm success\n");
    }

    return ret;
}

static int read_chip_id(struct sc9625 *sc, uint8_t *chipid)
{
    int ret = mtp_read_block(sc, SC9625_PID0, chipid, 6);
    if (ret < 0) {
        mca_log_err("read chip failed, ret = %d\n", ret);
    } else {
        mca_log_info("chip id=%02X %02X %02X %02X %02X %02X\n", *(chipid), *(chipid + 1), *(chipid + 2), *(chipid + 3), *(chipid + 4), *(chipid + 5));
    }
    return ret;
}

static int wait_warmup_done(struct sc9625 *sc)
{
    int i;
    int ret = 0;
    SC9625_st_e st;

    for (i = 0; i < 10; i++) {
        ret = mtp_read_byte(sc, SC9625_ST, &st.value);
        if (ret < 0) {
            mca_log_err("read 0xFFFF fail\n");
            return ret;
        }
        if (st.warmup_done) {
            return 0;
        }
        msleep(10);
    }
    return ret;
}

static int ate_mode_ctrl(struct sc9625 *sc, bool enable)
{
    int ret;
    SC9625_ate_en_e ate_en;

    ret = mtp_read_byte(sc, SC9625_ATE_EN, &ate_en.value);
    if (ret < 0) {
        mca_log_err("read 0xFFB0 fail\n");
        return ret;
    }

    if (enable) {
        ate_en.en_ate = 1;
    }
    else {
        ate_en.en_ate = 0;
    }

    ret = mtp_write_byte(sc, SC9625_ATE_EN, ate_en.value);
    if (ret < 0) {
        mca_log_err("ate mode ctrl write failed,en=%d\n", enable);
    } else {
        mca_log_info("ato mode ctrl successfully,en=%d\n", enable);
    }
    return ret;
}

static int iic_send_por(struct sc9625 *sc)
{
    return mtp_write_byte(sc, SC9625_ATE_ST, 0x69);
}

static int mcu_ctrl(struct sc9625 *sc, bool enable)
{
    int ret;
    SC9625_st_e st;

    ret = mtp_read_byte(sc, SC9625_ST, &st.value);
    if (ret < 0) {
        mca_log_err("read 0xFFFF fail\n");
        return ret;
    }

    if (enable) {
        if (!st.mcu_en) {
            ret = mtp_write_byte(sc, SC9625_SRAM_BIST_CTRL0, 0x02);
        }
    }
    else {
        if (st.mcu_en) {
            ret = mtp_write_byte(sc, SC9625_SRAM_BIST_CTRL0, 0x02);
        }
    }

    if (ret < 0) {
        mca_log_err("mcu ctrl write failed,val = %d\n", enable);
    } else {
        mca_log_err("mcu ctrl write successfully,val = %d\n", enable);
    }

    return ret;
}

static int hirc_ctrl(struct sc9625 *sc, bool enable)
{
    int ret;
    SC9625_st_e st;

    ret = mtp_read_byte(sc, SC9625_ST, &st.value);
    if (ret < 0) {
        mca_log_err("read 0xFFFF fail\n");
        return ret;
    }
    
    if (enable) {
        if (!st.ate_hirc_en) {
            ret = mtp_write_byte(sc, SC9625_SRAM_BIST_CTRL0, 0x04);
        }
    }
    else {
        if (st.ate_hirc_en) {
            ret = mtp_write_byte(sc, SC9625_SRAM_BIST_CTRL0, 0x04);
        }
    }

    if (ret < 0) {
        mca_log_err("hirc ctrl write failed,val = %d\n", enable);
    } else {
        mca_log_err("hirc ctrl write successfully,val = %d\n", enable);
    }

    return ret;
}

static int write_amba(struct sc9625 *sc, uint32_t reg, uint32_t data)
{
    int ret;
    uint16_t reg_addr = (reg >> 16) & 0xFFFF;

    //write high addr
    reg_addr = __builtin_bswap16(reg_addr);
    ret = mtp_write_block(sc, SC9625_HADDR_MSB, (uint8_t *)&reg_addr, 2);
    if (ret < 0) {
        return ret;
    }

    ret = mtp_write_block(sc, (uint16_t)(reg & 0xFFFF), (uint8_t *)&data, 4);
    if (ret < 0) {
        return ret;
    }

    reg_addr = 0x2000;
    reg_addr = __builtin_bswap16(reg_addr);
    return mtp_write_block(sc, SC9625_HADDR_MSB, (uint8_t *)&reg_addr, 2);
}

static int iic_mtp_ctrl(struct sc9625 *sc, bool enable)
{
    int ret;
    SC9625_ate_en_e ate_en;

    ret = mtp_read_byte(sc, SC9625_ATE_EN, &ate_en.value);
    if (ret < 0) {
        mca_log_err("read 0xFFB0 fail\n");
        return ret;
    }

    if (enable) {
        ate_en.iic_mtp_en = 1;
    }
    else {
        ate_en.iic_mtp_en = 0;
    }
    return mtp_write_byte(sc, SC9625_ATE_EN, ate_en.value);
}

static int mtp_power_ctrl(struct sc9625 *sc, bool enable)
{
    int ret;
    SC9625_ctrl_e ctrl;

    ret = mtp_read_byte(sc, SC9625_CTRL, &ctrl.value);
    if (ret < 0) {
        mca_log_err("read 0xFFA9 fail\n");
        return ret;
    }

    if (enable) {
        ctrl.pdn = 1;
    }
    else {
        ctrl.pdn = 0;
    }
    return mtp_write_byte(sc, SC9625_CTRL, ctrl.value);
}

static int mtp_iic_cfg(struct sc9625 *sc)
{
    int ret;
    SC9625_ctrl_e ctrl;

    ret = mtp_read_byte(sc, SC9625_CTRL, &ctrl.value);
    if (ret < 0) {
        mca_log_err("read 0xFFA9 fail\n");
        return ret;
    }

    ctrl.iic_cfg = 1;
    return mtp_write_byte(sc, SC9625_CTRL, ctrl.value);
}

static int mtp_cp_vol_set(struct sc9625 *sc, uint8_t vol)
{
    int ret;
    SC9625_ctrl_e ctrl;

    ret = mtp_read_byte(sc, SC9625_CTRL, &ctrl.value);
    if (ret < 0) {
        mca_log_err("read 0xFFA9 fail\n");
        return ret;
    }

    switch(vol){
    case CP_VOL_97:
        ctrl.cp_vol = 3;
        break;
    case CP_VOL_100:
        ctrl.cp_vol = 2;
        break;
    case CP_VOL_103:
        ctrl.cp_vol = 0;
        break;
    case CP_VOL_106:
        ctrl.cp_vol = 1;
        break;
    default:
        ctrl.cp_vol = 2;
        break;
    }
    return mtp_write_byte(sc, SC9625_CTRL, ctrl.value);
}

static int mtp_write_unlock(struct sc9625 *sc, bool enable)
{
    int ret;
    SC9625_ctrl_e ctrl;

    ret = mtp_read_byte(sc, SC9625_CTRL, &ctrl.value);
    if (ret < 0) {
        mca_log_err("read 0xFFA9 fail\n");
        return ret;
    }

    if (enable) {
        ctrl.lckn = 1;
    }
    else {
        ctrl.lckn = 0;
    }

    return mtp_write_byte(sc, SC9625_CTRL, ctrl.value);
}

static int mtp_bist_ctrl(struct sc9625 *sc, bool enable)
{
    int ret;
    SC9625_ctrl_e ctrl;

    ret = mtp_read_byte(sc, SC9625_CTRL, &ctrl.value);
    if (ret < 0) {
        mca_log_err("read 0xFFA9 fail\n");
        return ret;
    }

    if (enable) {
        ctrl.mtp_en = 1;
    }
    else {
        ctrl.mtp_en = 0;
    }

    return mtp_write_byte(sc, SC9625_CTRL, ctrl.value);
}

static int mtp_op_time_set(struct sc9625 *sc, int time)
{
    return mtp_write_byte(sc, SC9625_TPGHF, time & 0x1F);
}

static int mtp_mode_set(struct sc9625 *sc, uint8_t mode)
{
    return mtp_write_byte(sc, SC9625_MTP_OP_SEL, (uint8_t)mode);
}

static int mtp_addr_set(struct sc9625 *sc, uint16_t bks, uint16_t addr)
{
    uint16_t val = (bks << 13) | addr;

    val = __builtin_bswap16(val);
    return mtp_write_block(sc, SC9625_AD_MSB, (uint8_t *)&val, 2);
}

static int mtp_opnum_set(struct sc9625 *sc, uint16_t number)
{
    uint16_t val = number & 0x7FFF;

    val = __builtin_bswap16(val);
    return mtp_write_block(sc, SC9625_OP_NUM_MSB, (uint8_t *)&val, 2);
}

static int bist_start_ctrl(struct sc9625 *sc, bool enable)
{
    if (enable) {
        return mtp_write_byte(sc, SC9625_REG_RSV, 0x01);
    } else {
        return mtp_write_byte(sc, SC9625_REG_RSV, 0x00);
    }
}

static bool mtp_set_margin(struct sc9625 *sc, uint8_t margin)
{
    int ret;
    SC9625_cp_e cp;

    ret = mtp_read_byte(sc, SC9625_CP, &cp.value);
    if (ret < 0) {
        mca_log_err("read 0xFFAC fail\n");
        return false;
    }

    cp.marrd = margin;

    return mtp_write_byte(sc, SC9625_CP, cp.value);
}

static bool mtp_cp_ctrl(struct sc9625 *sc, bool enable)
{
        int ret;
    SC9625_cp_e cp;

    ret = mtp_read_byte(sc, SC9625_CP, &cp.value);
    if (ret < 0) {
        mca_log_err("read 0xFFAC fail\n");
        return false;
    }

    if (enable) {
        cp.cp_en = 1;
    }
    else {
        cp.cp_en = 0;
    }

    return mtp_write_byte(sc, SC9625_CP, cp.value);
}

static bool read_sector_crc(struct sc9625 *sc, uint32_t *r_crc)
{
    int ret;
    uint32_t data;

    ret = mtp_read_block(sc, SC9625_DOUT_CRC_B3, (uint8_t *)&data, 4);
    if (ret < 0) {
        return false;
    }

    *r_crc = endian_conversion(data);

    return true;
}

static int write_pdin(struct sc9625 *sc, uint8_t *data_in, uint8_t len)
{
    int ret;
    int i;
    uint8_t w_data[17] = {0};
    SC9625_mtp_st_e mtp_st;

    if (len != 16) {
        mca_log_err("Length is not 16-byte aligned\n");
        return -EINVAL;
    }

    for (i = 0; i < len; i += 4) {
        *(uint32_t *)(w_data + i) = endian_conversion(*(uint32_t *)(data_in + i));
    }

    w_data[16] = sc9625_get_crc8(w_data);
    ret = mtp_write_block(sc, SC9625_DIN0_B3, w_data, 17);
    if (ret < 0) {
        mca_log_err("write data fail\n");
        return ret;
    }

    //check crc
    ret = mtp_read_byte(sc, SC9625_MTP_ST, &mtp_st.value);
    if (ret < 0) {
        mca_log_err("read 0xFFAD fail\n");
        return ret;
    }

    if (mtp_st.din_crc_fail) {
        mca_log_err("check crc fail\n");
        return ret;
    }

    return 0;
}

static bool mtp_erase_chip(struct sc9625 *sc)
{
    int ret;
    int i;
    SC9625_mtp_st_e mtp_st;

    mca_log_info("start erase chip\n");
    //chip erase
    ret = mtp_power_ctrl(sc, true);
    ret |= mtp_iic_cfg(sc);
    ret |= mtp_cp_vol_set(sc, CP_VOL_106);
    ret |= mtp_op_time_set(sc, ERASE_OP_TIME);
    ret |= mtp_mode_set(sc, MTP_CERS_MODE);
    ret |= mtp_addr_set(sc, MAIN_BKS, MTP_START_ADDR);
    ret |= mtp_write_unlock(sc, true);
    ret |= mtp_bist_ctrl(sc, true);
    ret |= bist_start_ctrl(sc, true);
    if (ret < 0) {
        mca_log_err("erase init fail\n");
        goto erase_fail;
    }

    for (i = 0; i < 100; i++){
        ret = mtp_read_byte(sc, SC9625_MTP_ST, &mtp_st.value);
        if (ret < 0) {
            mca_log_err("read 0xFFAD fail\n");
            goto erase_fail;
        }

        if (!mtp_st.mtp_busy) {
            break;
        }

        if (i > 90) {
            mca_log_err("erase timeout\n");
            goto erase_fail;
        }
        msleep(10);
    }

    ret = mtp_mode_set(sc, MTP_CLR_MODE);
    ret |= mtp_write_unlock(sc, false);
    ret |= mtp_power_ctrl(sc, false);
    ret |= mtp_bist_ctrl(sc, false);

    if (ret < 0) {
        mca_log_err("erase deinit fail\n");
        goto erase_fail;
    }

    mca_log_info("erase successful\n");
    return true;

erase_fail:
    mca_log_err("erase fail\n");
    return false;
}

static bool mtp_erase_check(struct sc9625 *sc)
{
    int ret;
    int i;
    SC9625_mtp_st_e mtp_st;
    uint8_t w_data[16] = {0};

    mca_log_info("start erase check\n");
    ret = mtp_power_ctrl(sc, true);
    ret |= mtp_iic_cfg(sc);
    ret |= mtp_set_margin(sc, MARGIN3);
    ret |= mtp_addr_set(sc, MAIN_BKS, MTP_START_ADDR);
    ret |= mtp_mode_set(sc, MTP_RV_MODE);
    ret |= mtp_opnum_set(sc, (MTP_SIZE / MTP_SECTOR) * 16 - 1);
    ret |= write_pdin(sc, w_data, 16);
    ret |= mtp_bist_ctrl(sc, true);
    ret |= bist_start_ctrl(sc, true);
    if (ret < 0) {
        mca_log_err("erase check init fail\n");
        goto erase_check_fail;
    }

    for (i = 0; i< 100; i++) {
        ret = mtp_read_byte(sc, SC9625_MTP_ST, &mtp_st.value);
        if (ret < 0) {
            mca_log_err("read 0xFFAD fail\n");
            goto erase_check_fail;
        }

        if (!mtp_st.mtp_busy) {
            break;
        }

        if (i > 90) {
            mca_log_err("erase check timeout\n");
            goto erase_check_fail;
        }
        msleep(10);
    }

    ret = mtp_mode_set(sc, MTP_CLR_MODE);
    ret |= mtp_power_ctrl(sc, false);
    ret |= mtp_bist_ctrl(sc, false);

    ret = mtp_read_byte(sc, SC9625_MTP_ST, &mtp_st.value);
    if (ret < 0) {
        mca_log_err("read 0xFFAD fail\n");
        goto erase_check_fail;
    }

    if (mtp_st.mtp_fail) {
        goto erase_check_fail;
    }

    mca_log_info("erase check successful\n");
    return true;

erase_check_fail:
    mca_log_err("erase check fail\n");
    return false;
}

static bool mtp_write(struct sc9625 *sc, uint8_t *buf, uint32_t buf_len)
{
    int ret;
    int i, timeout;
    SC9625_mtp_st_e mtp_st;

    mca_log_info("mtp write start\n");

    if (buf == NULL || (buf_len % MTP_SECTOR) != 0) {
        mca_log_err("buf is NULL or Length is not sector aligned\n");
        goto mtp_write_fail;
    }

    ret = mtp_power_ctrl(sc, true);
    ret |= mtp_iic_cfg(sc);
    ret |= mtp_cp_vol_set(sc, CP_VOL_106);
    ret |= mtp_op_time_set(sc, WRITE_OP_TIME);
    ret |= mtp_addr_set(sc, MAIN_BKS, MTP_START_ADDR);
    ret |= mtp_mode_set(sc, MTP_WV_MODE);
    ret |= mtp_write_unlock(sc, true);
    ret |= mtp_opnum_set(sc, (buf_len >> 4) - 1);
    ret |= mtp_bist_ctrl(sc, true);
    ret |= bist_start_ctrl(sc, true);
    if (ret < 0) {
        mca_log_err("mtp write init fail\n");
        goto mtp_write_fail;
    }

    for (i = 0; i < buf_len; i += 16) {
        //wait wr_avb
        for (timeout = 0; timeout < 100; timeout++) {
            ret = mtp_read_byte(sc, SC9625_MTP_ST, &mtp_st.value);
            if (ret < 0) {
                mca_log_err("read 0xFFAD fail\n");
                goto mtp_write_fail;
            }

            if (mtp_st.wr_avb) {
                break;
            }

            if (timeout > 90) {
                mca_log_err("mtp write wait wr avb timeout\n");
                goto mtp_write_fail;
            }

            msleep(10);
        }

        if (write_pdin(sc, buf + i, 16) < 0) {
            mca_log_err("mtp write pdin fail\n");
            goto mtp_write_fail;
        }
    }

    for (timeout = 0; timeout < 10; timeout++) {
        ret = mtp_read_byte(sc, SC9625_MTP_ST, &mtp_st.value);
        if (ret < 0) {
            mca_log_err("read 0xFFAD fail\n");
            goto mtp_write_fail;
        }

        if (!mtp_st.mtp_busy) {
            break;
        }

        if(mtp_st.mtp_fail){
            mca_log_err("mtp_fail\n");
            goto mtp_write_fail;
        }

        if (timeout > 8) {
            mca_log_err("mtp write timeout\n");
            goto mtp_write_fail;
        }
        msleep(10);
    }

    ret = mtp_write_unlock(sc, false);
    ret |= mtp_cp_ctrl(sc, false);
    ret |= mtp_mode_set(sc, MTP_CLR_MODE);
    ret |= mtp_power_ctrl(sc, false);
    ret |= bist_start_ctrl(sc, false);
    ret |= mtp_bist_ctrl(sc, false);
    if (ret < 0) {
        mca_log_err("mtp write deinit fail\n");
        goto mtp_write_fail;
    }

    mca_log_info("mtp write successful\n");
    return true;

mtp_write_fail:
    mca_log_err("mtp write fail\n");
    return false;
}

bool mtp_chip_crc_check(struct sc9625 *sc, uint32_t mtp_len)
{
    int ret = 0;
    uint32_t              i;
    uint32_t              timeout;
    uint32_t              r_crc = 0;
    SC9625_mtp_st_e     mtp_st;
    uint32_t              crc_127 = 0xFFFFFFFF;
    uint32_t              temp;

    mca_log_info("MTP chip CRC check start\n");

    ret |= dig_tm_entry(sc, true);
    ret |= wait_warmup_done(sc);
    if (ret < 0) {
        mca_log_err("wait_warmup_done error %d\n", ret);
        goto mtp_crc_check_end;
    }
    ret = ate_mode_ctrl(sc, true);
    ret |= mcu_ctrl(sc, false);
    ret |= hirc_ctrl(sc, true);
    temp = 0x7FFFFFFF;
    //need check
    ret |= write_amba(sc, 0x4000D008, temp);
    temp = 0x1F08;
    ret |= write_amba(sc, 0x4000A000, temp);
    ret |= iic_mtp_ctrl(sc, true);
    if (ret < 0) {
        mca_log_err("init fail\n");
        goto mtp_crc_check_end;
    }

    ret = mtp_power_ctrl(sc, true);
    ret |= mtp_set_margin(sc, MARGIN1);
    ret |= mtp_iic_cfg(sc);
    ret |= mtp_addr_set(sc, MAIN_BKS, MTP_START_ADDR);
    ret |= mtp_mode_set(sc, MTP_CRC_MODE);
    ret |= mtp_opnum_set(sc, (mtp_len >> 4) - 1);
    ret |= mtp_bist_ctrl(sc, true);
    ret |= bist_start_ctrl(sc, true);
    if (ret < 0) {
        mca_log_err("mtp crc check init fail\n");
        goto mtp_crc_check_end;
    }
    for (i = 0; i < (mtp_len / MTP_SECTOR); i++) {
        for (timeout = 0;; timeout++) {
            ret = mtp_read_byte(sc, SC9625_MTP_ST, &mtp_st.value);
            if (ret < 0) {
                mca_log_err("read 0xFFAD fail\n");
                goto mtp_crc_check_end;
            }

            if (mtp_st.rd_avb) {
                break;
            }

            if (timeout > 1000) {
                ret = -EIO;
                mca_log_err("mtp crc check timeout\n");
                goto mtp_crc_check_end;
            }
            mdelay(1);
        }

        if (!read_sector_crc(sc, &r_crc)) {
            ret = -EIO;
            mca_log_err("mtp crc check read crc fail\n");
            goto mtp_crc_check_end;
        }

        if (i < mtp_len / MTP_SECTOR - 1) {
            crc_127 = sc9625_func_crc32(r_crc, crc_127);
        } else {
            crc_127 = sc9625_func_crc32(crc_127, 0);
            if (r_crc != crc_127) {
                ret = -EIO;
                goto mtp_crc_check_end;
            }
        }
    }

mtp_crc_check_end:
    mca_log_info("crc read_value: 0x%08x\n", r_crc);
    mca_log_info("crc cal_val: 0x%08x\n", crc_127);

    ret |= mtp_mode_set(sc, MTP_CLR_MODE);
    ret |= mtp_power_ctrl(sc, false);
    ret |= bist_start_ctrl(sc, false);
    ret |= mtp_bist_ctrl(sc, false);

    ret |= iic_mtp_ctrl(sc, false);
    ret |= ate_mode_ctrl(sc, false);
    ret |= iic_send_por(sc);
    ret |= dig_tm_entry(sc, false);

    if (ret < 0) {
        mca_log_err("MTP CRC chip check failed\n");
        return false;
    }else{
        mca_log_info("MTP CRC chip check successfully\n");
        return true;
    }
}

static bool mtp_crc_check(struct sc9625 *sc, uint8_t margin, uint32_t crc_start, uint32_t *crc_stop, uint8_t *buf, uint32_t buf_len)
{
    int ret;
    int i, j;
    int timeout;
    uint32_t data32 = crc_start;
    uint32_t r_crc ;
    SC9625_mtp_st_e mtp_st;

    mca_log_info("mtp crc check margin : %d start\n", margin);
    ret = mtp_power_ctrl(sc, true);
    ret |= mtp_set_margin(sc, margin);
    ret |= mtp_iic_cfg(sc);
    ret |= mtp_addr_set(sc, MAIN_BKS, MTP_START_ADDR);
    ret |= mtp_mode_set(sc, MTP_CRC_MODE);
    ret |= mtp_opnum_set(sc, (buf_len >> 4) - 1);
    ret |= mtp_bist_ctrl(sc, true);
    ret |= bist_start_ctrl(sc, true);
    if (ret < 0) {
        mca_log_err("mtp crc check init fail\n");
        goto mtp_crc_check_fail;
    }

    for (i = 0; i < (buf_len / MTP_SECTOR); i++) {
        //buf crc
        for (j = 0; j < (MTP_SECTOR >> 2); j++) {
            data32 = sc9625_func_crc32(*(uint32_t *)(buf + i * MTP_SECTOR + j * 4), data32);
        }

        for (timeout = 0; timeout < 10; timeout++) {
            ret = mtp_read_byte(sc, SC9625_MTP_ST, &mtp_st.value);
            if (ret < 0) {
                mca_log_err("read 0xFFAD fail\n");
                goto mtp_crc_check_fail;
            }

            if (mtp_st.rd_avb) {
                break;
            }

            if (timeout > 8) {
                mca_log_err("mtp crc check timeout\n");
                goto mtp_crc_check_fail;
            }
            msleep(10);
        }

        if (!read_sector_crc(sc, &r_crc)) {
            mca_log_err("mtp crc check read crc fail\n");
            goto mtp_crc_check_fail;
        }
        *crc_stop = r_crc;
        if (data32 != r_crc) {
            mca_log_err("sector %d check crc fail, w : 0x%08x  r: 0x%08x\n",
                i, data32, r_crc);
            goto mtp_crc_check_fail;
        }

        data32 = 0xFFFFFFFF;
    }

    if(margin == MARGIN1){
        mca_log_info(" crc  margin1 : %08x\n", r_crc);
    }else{
        mca_log_info(" crc  margin3 : %08x\n", r_crc);
    }

    ret = mtp_mode_set(sc, MTP_CLR_MODE);
    ret |= mtp_power_ctrl(sc, false);
    ret |= bist_start_ctrl(sc, false);
    ret |= mtp_bist_ctrl(sc, false);
    if (ret < 0) {
        mca_log_err("mtp crc check deinit fail\n");
        goto mtp_crc_check_fail;
    }

    mca_log_info("mtp crc check margin : %d successful\n", margin);
    return true;

mtp_crc_check_fail:
    ret = mtp_mode_set(sc, MTP_CLR_MODE);
    ret |= mtp_power_ctrl(sc, false);
    ret |= bist_start_ctrl(sc, false);
    ret |= mtp_bist_ctrl(sc, false);
    mca_log_err("mtp crc check margin : %d fail\n", margin);
    return false;
}

static int mtp_trim_read(struct sc9625 *sc, uint32_t *trim0, uint32_t *trim1, uint32_t *trim_data) {
    int ret = 0;
    trim_info_space_t   space_list[] = {{TRIM0_WORD_ADDR, TRIM_WORD_LEN}, {INFO0_WORD_ADDR, INFO_WORD_LEN},
                                        {TRIM1_WORD_ADDR, TRIM_WORD_LEN}, {INFO1_WORD_ADDR, INFO_WORD_LEN}};
    uint32_t              trim[INFOR_READ_LEN * 2];
    uint32_t              i, j, timeout;
    uint32_t              read_count = 0;
    SC9625_mtp_st_e     mtp_st;

    ret = mtp_power_ctrl(sc, true);
    ret |= mtp_iic_cfg(sc);
    ret |= mtp_cp_vol_set(sc, CP_VOL_106);
    ret |= mtp_addr_set(sc, INFO_BKS, space_list[0].word_addr);
    ret |= mtp_mode_set(sc, MTP_RD_MODE);
    ret |= mtp_set_margin(sc, MARGIN1);
    ret |= mtp_opnum_set(sc, ((space_list[0].word_len*4) >> 4) - 1);
    ret |= mtp_bist_ctrl(sc, true);
    ret |= bist_start_ctrl(sc, true);
    if (ret < 0) {
        mca_log_err("trim read init fail\n");
        goto trim_read_end;
    }

    read_count = 0;
    for (j = 0; j < 4; j++) {
        ret |= mtp_addr_set(sc, INFO_BKS, space_list[j].word_addr);
        ret |= mtp_opnum_set(sc, ((space_list[j].word_len*4) >> 4) - 1);
        ret |= mtp_bist_ctrl(sc, true);
        ret |= bist_start_ctrl(sc, true);
        if (ret < 0) {
            mca_log_err("trim read set addr fail\n");
            goto trim_read_end;
        }
        for (i = 0; i < space_list[j].word_len; i += 4) {
            //wait write avalible
            for (timeout = 0; timeout < 20; timeout++) {
                ret = mtp_read_byte(sc, SC9625_MTP_ST, &mtp_st.value);
                if (ret < 0) {
                    mca_log_err("read status fail\n");
                    goto trim_read_end;
                }
                if (mtp_st.rd_avb) {
                    break;
                }
                if (timeout > 18) {
                    mca_log_err("read status timeout\n");
                    ret = -EIO;
                    goto trim_read_end;
                }
                mdelay(1);
            }
            ret = mtp_read_block(sc, SC9625_DOUT0_B3, (uint8_t*)&trim[read_count] , 16);
            if (ret < 0) {
                mca_log_err("trim read fail\n");
                goto trim_read_end;
            }
            trim[read_count+0] = SWAP_UINT32(trim[read_count+0]);
            trim[read_count+1] = SWAP_UINT32(trim[read_count+1]);
            trim[read_count+2] = SWAP_UINT32(trim[read_count+2]);
            trim[read_count+3] = SWAP_UINT32(trim[read_count+3]);
            read_count += 4;
        }
    }

    if (trim_data) {
        for (i = 0; i < INFOR_READ_LEN; i++) {
            trim_data[i] = trim[i] | trim[INFOR_READ_LEN + i];
        }
    }
    if (trim0) {
        memcpy(trim0, &trim[0], INFOR_READ_LEN * 4);
    }
    if (trim1) {
        memcpy(trim1, &trim[INFOR_READ_LEN], INFOR_READ_LEN * 4);
    }

trim_read_end:
    ret |= mtp_mode_set(sc, MTP_CLR_MODE);
    ret |= mtp_power_ctrl(sc, false);
    ret |= bist_start_ctrl(sc, false);
    ret |= mtp_bist_ctrl(sc, false);
    if (ret < 0){
        mca_log_err("trim read fail\n");
    } else {
        mca_log_info("trim read successful\n");
    }
    return ret;
}

static bool mtp_trim_check(struct sc9625 *sc, uint32_t *trim_data) {
    int ret = 0;
    uint32_t              trim[2][INFOR_READ_LEN];
    uint32_t              i;

    ret = mtp_trim_read(sc, trim[0], trim[1], NULL);
    if (ret < 0) {
        mca_log_err("mtp trim read fail\n");
        return false;
    }

    for (i = 0; i < INFOR_COMPARE_WORD_SIZE; i++) {
        if (trim_data[i] != trim[0][i] || trim_data[i] != trim[1][i]) {
            mca_log_err("mtp trim check fail\n");
            return false;
        }
    }
    mca_log_info("trim check successful\n");
    return true;
}

int sc9625_mtp_program(struct sc9625 *sc, uint8_t *image, uint32_t len)
{
    int ret;
    uint32_t crc_start = 0xFFFFFFFF;
    uint32_t crc_stop = crc_start;
    uint32_t trim_data[INFOR_READ_LEN];
    uint8_t chip_id[6];
    uint8_t write_times = 0;
    uint32_t r_crc;
    bool crc_check = true;

    mca_log_info("program start\n");
    sc->fw_program = true;

    mca_log_err("firmware len-1 ---> %d\n", len);

    //op init
    do {
        ret = dig_tm_entry(sc, true);
        ret |= read_chip_id(sc, &chip_id[0]);
        ret |= wait_warmup_done(sc);
        if (ret < 0) {
            mca_log_err("wait_warmup_done error %d\n", ret);
            break;
        }

        ret = ate_mode_ctrl(sc, true);
        ret |= mcu_ctrl(sc, false);
        ret |= hirc_ctrl(sc, true);
        ret |= write_amba(sc, 0x4000D008,0x7FFFFFFF);//open clk
        ret |= write_amba(sc, 0x4000A000, 0x1F08);//open clk
        ret |= iic_mtp_ctrl(sc, true);
        if (ret < 0) {
            mca_log_err("op init fail\n");
            break;
        }

        mca_log_info("read trim\n");
        ret |= mtp_trim_read(sc, NULL, NULL, trim_data);

        mca_log_info("erase mtp\n");
        if (!mtp_erase_chip(sc))
        {
            ret = -EIO;
            goto program_fail;
        }
        mca_log_info("check trim\n");
        if (!mtp_trim_check(sc, trim_data)) {
            mca_log_err("check trim fail\n");
        }

        if (!mtp_erase_check(sc)) {
            ret = -EIO;
            goto program_fail;
        }
        while (true) {
            if (write_times >= 2) {
                ret = -EIO;
                goto program_fail;
            }
            write_times++;
            if (!mtp_write(sc, image, len)) {
                continue;
            }
            if(write_times == 2 && !crc_check){
                if (!read_sector_crc(sc, &r_crc)) {
                    mca_log_err("read crc fail\n");
                    goto program_fail;
                }
                crc_start = r_crc;
            }
            if (!mtp_crc_check(sc, MARGIN1, crc_start, &crc_stop, image, len)) {
                crc_check = false;
                continue;
            }
            if (!mtp_crc_check(sc, MARGIN3, crc_stop, &crc_start, image, len)) {
                crc_check = false;
                continue;
            }
            break;
        }

    } while (0);

    ret = iic_mtp_ctrl(sc, false);
    ret |= ate_mode_ctrl(sc, false);
    ret |= iic_send_por(sc);
    ret |= dig_tm_entry(sc, false);
    if (ret < 0) {
        mca_log_err("op deinit fail\n");
        goto program_fail;
    }

    mca_log_info("program successful\n");

    sc->fw_program = false;
    return 0;

program_fail:
    mca_log_err("program fail\n");

    sc->fw_program = false;
    return ret;
}