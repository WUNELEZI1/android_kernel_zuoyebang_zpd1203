/** ***************************************************************************
 * @file nano_firmware.c
 *
 * @brief implent nanosic firmware load interface
 *
 * <em>Copyright (C) 2010, Nanosic, Inc.  All rights reserved.</em>
 * Author : Bin.yuan bin.yuan@nanosic.com
 * */

/*
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "nano_macro.h"
#include <linux/firmware.h>
#include <linux/wait.h>

#define DELAY_I2C_COMMU_MS (3)
#define MAX_I2C_TRANS_LENGTH (512)
#define IIC_REG_ADDR (0x5c)

/** **************************************************************************
 * @brief cal check sum and return it
 *
 ** */
int Nanosic_firmware_check_sum(int *data, int datalen)
{

    int sum = 0, i = 0;

    datalen = datalen >> 2;

    for (; i < datalen; i++, data++)
        sum += *data;

    return sum;
}

/** **************************************************************************
 * @brief read iic salve device reg value
 *
 ** */
int Nanosic_firmware_read(struct nano_i2c_client *i2c_client, char reg_addr, void *buf, size_t len)
{
    struct i2c_adapter *adap;
    unsigned char addr[4];
    struct i2c_msg msg[2];
    int ret;

    if (IS_ERR_OR_NULL(i2c_client))
    {
        dbgprint(ERROR_LEVEL, "i2c_info is NULL\n");
        return -1;
    }

    addr[0] = reg_addr;

    adap = i2c_get_adapter(i2c_client->i2c_bus_id);
    /*
     * Send out the register address...
     */
    msg[0].len = 1;
    msg[0].addr = i2c_client->i2c_slave_addr;
    msg[0].flags = 0;
    msg[0].buf = addr;
    /*
     * ...then read back the result.
     */
    msg[1].addr = i2c_client->i2c_slave_addr;
    msg[1].flags = I2C_M_RD;
    msg[1].len = len;
    msg[1].buf = buf;

    mutex_lock(&i2c_client->read_mutex);

    ret = i2c_transfer(adap, msg, 2);
    if (ret < 0)
    {
        dbgprint(ERROR_LEVEL, "i2c_transfer read error\n");
        len = ret;
    }

    mutex_unlock(&i2c_client->read_mutex);

    return len;
}

/** **************************************************************************
 * @brief Nanosic_firmware_read_bootID
 *
 ** */
char Nanosic_firmware_read_bootID(struct nano_i2c_client *i2c_client, char reg_addr)
{
    char cmd = 0x4;
    char val = 0x0;

    Nanosic_firmware_write(i2c_client, reg_addr, &cmd, sizeof(cmd));
    Nanosic_firmware_read(i2c_client, reg_addr, &val, sizeof(val));

    dbgprint(INFO_LEVEL, "read bootID %x\n", val);

    return val;
}

/** **************************************************************************
 * @brief Nanosic_firmware_read_status
 *
 ** */
char Nanosic_firmware_read_status(struct nano_i2c_client *i2c_client, char reg_addr)
{
    char cmd = 0x5;
    char val = 0x0;

    /*read status*/
    Nanosic_firmware_write(i2c_client, reg_addr, &cmd, sizeof(cmd));
    Nanosic_firmware_read(i2c_client, reg_addr, &val, sizeof(val));
    dbgprint(ERROR_LEVEL, "read status 0x%x\n", val);

    return val;
}
/** **************************************************************************
 * @brief write iic salve device reg value
 *
 ** */
int Nanosic_firmware_write(struct nano_i2c_client *i2c_client, char reg_addr, void *buf, size_t len)
{
    struct i2c_msg msg;
    struct i2c_adapter *adap;
    unsigned char tmp_buf[MAX_I2C_TRANS_LENGTH + 1] = {0};
    int ret;

    if (IS_ERR_OR_NULL(i2c_client))
    {
        dbgprint(ERROR_LEVEL, "i2c_info is NULL\n");
        return -1;
    }

    adap = i2c_get_adapter(i2c_client->i2c_bus_id);

    tmp_buf[0] = reg_addr;
    memcpy(tmp_buf + 1, buf, len);

    msg.addr = i2c_client->i2c_slave_addr;
    msg.flags = 0;
    msg.len = len + 1;
    msg.buf = tmp_buf;

    mutex_lock(&i2c_client->write_mutex);

    ret = i2c_transfer(adap, &msg, 1);
    if (ret < 0)
    {
        dbgprint(ERROR_LEVEL, "i2c_transfer write error\n");
        len = ret;
    }

    mutex_unlock(&i2c_client->write_mutex);

    msleep(DELAY_I2C_COMMU_MS);

    return len;
}

/** ************************************************************************
 *  @brief reset mcu
 *
 *  fwpath :
 ** */
int Nanosic_firmware_write_start(struct nano_i2c_client *i2c_client)
{
    uint8_t cmd_check = 0x8;
    uint8_t cmd_start = 0x9;

    dbgprint(INFO_LEVEL, "write Start Command\n");

    Nanosic_firmware_write(i2c_client, 0x5c, &cmd_check, sizeof(cmd_check));

    Nanosic_firmware_read_status(i2c_client, IIC_REG_ADDR);

    Nanosic_firmware_write(i2c_client, 0x5c, &cmd_start, sizeof(cmd_start));

    return 0;
}

/** ************************************************************************
 *  @brief
 *
 *  fwpath :
 ** */
int Nanosic_firmware_write_header(struct nano_i2c_client *i2c_client, uint8_t *head_ptr, int32_t head_size)
{
    char load_iic_tmp_buf[65] = {0};
    int i = 0;
    int RamCodeSum = 0, RamCodeSize = 0;

    if (IS_ERR_OR_NULL(head_ptr) || head_size != 0x40)
    {
        dbgprint(ERROR_LEVEL, "Invalid pionter\n");
        return -1;
    }

    dbgprint(INFO_LEVEL, "write Header Command Default Length %d\n", MAX_I2C_TRANS_LENGTH);

    // Command id
    load_iic_tmp_buf[0] = 0x06;

    // CompanyName
    load_iic_tmp_buf[1] = head_ptr[0];
    load_iic_tmp_buf[2] = head_ptr[1];
    load_iic_tmp_buf[3] = head_ptr[2];
    load_iic_tmp_buf[4] = head_ptr[3];
    load_iic_tmp_buf[5] = head_ptr[4];
    load_iic_tmp_buf[6] = head_ptr[5];
    load_iic_tmp_buf[7] = head_ptr[6];

    // Chip ID
    load_iic_tmp_buf[8] = 0x40;
    load_iic_tmp_buf[9] = 0x81;

    dbgprint(INFO_LEVEL, "Chip ID: %X %X\n", load_iic_tmp_buf[8], load_iic_tmp_buf[9]);

    // RAM CODE SOURCE
    load_iic_tmp_buf[10] = 0x45;
    load_iic_tmp_buf[11] = 0x23;
    load_iic_tmp_buf[12] = 0;
    load_iic_tmp_buf[13] = 0;
    dbgprint(INFO_LEVEL, "RAM CODE SOURCE: %X %X %X %X\n", load_iic_tmp_buf[10], load_iic_tmp_buf[11], load_iic_tmp_buf[12], load_iic_tmp_buf[13]);

    // RAM CODE DEST
    load_iic_tmp_buf[14] = 0x40;
    load_iic_tmp_buf[15] = 0;
    load_iic_tmp_buf[16] = 0;
    load_iic_tmp_buf[17] = 0;
    dbgprint(INFO_LEVEL, "RAM CODE DEST: %X %X %X %X\n", load_iic_tmp_buf[14], load_iic_tmp_buf[15], load_iic_tmp_buf[16], load_iic_tmp_buf[17]);

    // RAM CODE SIZE
    load_iic_tmp_buf[18] = head_ptr[48];
    load_iic_tmp_buf[19] = head_ptr[49];
    load_iic_tmp_buf[20] = head_ptr[50];
    load_iic_tmp_buf[21] = head_ptr[51];

    dbgprint(INFO_LEVEL, "RAM CODE SIZE: %X %X %X %X\n", load_iic_tmp_buf[18], load_iic_tmp_buf[19], load_iic_tmp_buf[20], load_iic_tmp_buf[21]);

    // APP ENTRY ADDRESS FIXED 44 00 ff 1f
    load_iic_tmp_buf[22] = head_ptr[21];
    load_iic_tmp_buf[23] = head_ptr[22];
    load_iic_tmp_buf[24] = head_ptr[23];
    load_iic_tmp_buf[25] = head_ptr[24];

    dbgprint(INFO_LEVEL, "APP ENTRY ADDRESS: %X %X %X %X\n", load_iic_tmp_buf[22], load_iic_tmp_buf[23], load_iic_tmp_buf[24], load_iic_tmp_buf[25]);

    // PID
    load_iic_tmp_buf[26] = head_ptr[25];
    load_iic_tmp_buf[27] = head_ptr[26];
    dbgprint(INFO_LEVEL, "PID: %X %X\n", load_iic_tmp_buf[26], load_iic_tmp_buf[27]);

    // VID
    load_iic_tmp_buf[28] = head_ptr[27];
    load_iic_tmp_buf[29] = head_ptr[28];
    dbgprint(INFO_LEVEL, "VID: %X %X\n", load_iic_tmp_buf[28], load_iic_tmp_buf[29]);

    // Firmware Version
    load_iic_tmp_buf[30] = head_ptr[29];
    load_iic_tmp_buf[31] = head_ptr[30];

    dbgprint(INFO_LEVEL, "VERSION: %d.%d\n", load_iic_tmp_buf[30], load_iic_tmp_buf[31]);

    // IMAGE SIZE
    load_iic_tmp_buf[49] = 0;
    load_iic_tmp_buf[50] = 0;
    load_iic_tmp_buf[51] = 0;
    load_iic_tmp_buf[52] = 0;
    dbgprint(INFO_LEVEL, "IMAGE SIZE: %X %X %X %X\n", load_iic_tmp_buf[49], load_iic_tmp_buf[50], load_iic_tmp_buf[51], load_iic_tmp_buf[52]);

    RamCodeSize = load_iic_tmp_buf[18] | load_iic_tmp_buf[19] << 8 | load_iic_tmp_buf[20] << 16 | load_iic_tmp_buf[21] << 24;
    dbgprint(INFO_LEVEL, "RamCodeSize %d\n", RamCodeSize);
    RamCodeSum = Nanosic_firmware_check_sum((int *)(head_ptr + 0x40), RamCodeSize);
    dbgprint(INFO_LEVEL, "RamCodeSum %d\n", RamCodeSum);

    // RAM CODE SUM
    load_iic_tmp_buf[53] = RamCodeSum & 0xFF; // head_ptr[52];
    load_iic_tmp_buf[54] = RamCodeSum >> 8 & 0xFF;
    load_iic_tmp_buf[55] = RamCodeSum >> 16 & 0xFF;
    load_iic_tmp_buf[56] = RamCodeSum >> 24 & 0xFF;
    dbgprint(INFO_LEVEL, "RAM CODE SUM: %X %X %X %X\n", load_iic_tmp_buf[53], load_iic_tmp_buf[54], load_iic_tmp_buf[55], load_iic_tmp_buf[56]);

    // IMAGE CHECK SUM
    load_iic_tmp_buf[57] = 0;
    load_iic_tmp_buf[58] = 0;
    load_iic_tmp_buf[59] = 0;
    load_iic_tmp_buf[60] = 0;
    dbgprint(INFO_LEVEL, "IMAGE SUM: %X %X %X %X\n", load_iic_tmp_buf[57], load_iic_tmp_buf[58], load_iic_tmp_buf[59], load_iic_tmp_buf[60]);

    load_iic_tmp_buf[61] = 0;
    load_iic_tmp_buf[62] = 0;
    load_iic_tmp_buf[63] = 0;

    /*check sum cal*/
    for (i = 1; i < 64; i++)
        load_iic_tmp_buf[64] += load_iic_tmp_buf[i];

    dbgprint(INFO_LEVEL, "HEADER SUM: %X\n", load_iic_tmp_buf[64]);

    rawdata_show("header", load_iic_tmp_buf, sizeof(load_iic_tmp_buf));

    Nanosic_firmware_write(i2c_client, IIC_REG_ADDR, load_iic_tmp_buf, sizeof(load_iic_tmp_buf));

    return 0;
}

/** ************************************************************************
 *  @brief
 *
 *  fwpath :
 ** */
int Nanosic_firmware_write_bin(struct nano_i2c_client *i2c_client, uint8_t *AppImage, int32_t AppImageSize)
{
    uint16_t k;
    uint32_t payload_offset = 0;
    uint32_t payload_len = 0;
    int ret;

    if (IS_ERR_OR_NULL(i2c_client) || !AppImage || AppImageSize < 0)
    {
        dbgprint(ERROR_LEVEL, "Invalid argments\n");
        return -1;
    }

    dbgprint(INFO_LEVEL, "AppImageSize %d , offset %d\n", AppImageSize, payload_offset);

    while (AppImageSize > payload_offset)
    {
        char load_iic_tmp_buf[MAX_I2C_TRANS_LENGTH] = {0};
        // 还有数据未传完
        if (AppImageSize >= (payload_offset + (MAX_I2C_TRANS_LENGTH - 9)))
        {
            payload_len = MAX_I2C_TRANS_LENGTH - 9; // 48byt
        }
        else
        {
            payload_len = AppImageSize - payload_offset;
        }

        load_iic_tmp_buf[0] = 0x07;
        load_iic_tmp_buf[1] = ((payload_offset >> 24) & 0xff);
        load_iic_tmp_buf[2] = ((payload_offset >> 16) & 0xff);
        load_iic_tmp_buf[3] = ((payload_offset >> 8) & 0xff);
        load_iic_tmp_buf[4] = (payload_offset & 0xff);
        load_iic_tmp_buf[5] = ((payload_len >> 24) & 0xff);
        load_iic_tmp_buf[6] = ((payload_len >> 16) & 0xff);
        load_iic_tmp_buf[7] = ((payload_len >> 8) & 0xff);
        load_iic_tmp_buf[8] = (payload_len & 0xff);

        for (k = 0; k < payload_len; k++)
            load_iic_tmp_buf[9 + k] = AppImage[k + payload_offset];

        ret = Nanosic_firmware_write(i2c_client, IIC_REG_ADDR, load_iic_tmp_buf, MAX_I2C_TRANS_LENGTH);

        payload_offset += payload_len;
    }

    return 0;
}

/** ************************************************************************
 *  @brief
 *
 *  fwpath :
 ** */
int Nanosic_firmware_upgrade(struct nano_i2c_client *i2c_client, uint8_t *head_ptr, uint32_t size)
{
    int status = 0;
    int ImageSize = 0;
    int bootID;
    int ret = 0;

    if (IS_ERR_OR_NULL(head_ptr) || IS_ERR_OR_NULL(i2c_client))
    {
        dbgprint(ERROR_LEVEL, "upgrade argments error\n");
        ret = -1;
        goto _out;
    }

    dbgprint(INFO_LEVEL, "upgrade fw start %p -> %d\n", (void *)head_ptr, size);

    bootID = Nanosic_firmware_read_bootID(i2c_client, IIC_REG_ADDR);
    if (bootID != 0xC8)
    {
        dbgprint(ERROR_LEVEL, "bootID %0xx not match\n", bootID);
        ret = -1;
        goto _out;
    }

    /*read status*/
    status = Nanosic_firmware_read_status(i2c_client, IIC_REG_ADDR);

    ret = Nanosic_firmware_write_header(i2c_client, head_ptr, 0x40);
    if (ret)
    {
        dbgprint(ERROR_LEVEL, "write header fail, ret = %d\n", ret);
        goto _out;
    }

    status = Nanosic_firmware_read_status(i2c_client, IIC_REG_ADDR);

    ImageSize = head_ptr[48] | (head_ptr[49] << 8) | (head_ptr[50] << 16) | (head_ptr[51] << 24);
    dbgprint(INFO_LEVEL, "ImageSize %d\n", ImageSize);

    ret = Nanosic_firmware_write_bin(i2c_client, head_ptr + 0x40, ImageSize);
    if (ret)
    {
        dbgprint(ERROR_LEVEL, "write bin fail, ret = %d\n", ret);
        goto _out;
    }

    ret = Nanosic_firmware_write_start(i2c_client);
    if (ret)
    {
        dbgprint(ERROR_LEVEL, "write start fail, ret = %d\n", ret);
        goto _out;
    }

    dbgprint(INFO_LEVEL, "upgrade fw end\n");

_out:

    return ret;
}

/** ************************************************************************
 *  @brief begin to load firmware
 *
 *  fwpath : /system/etc/firmware/nano.bin
 ** */
static int firmware_request_complete(const struct firmware *firmware, void *context)
{
    struct nano_i2c_client *i2c_client = context;
    int ret = 0;

    if (IS_ERR_OR_NULL(firmware))
    {
        dbgprint(ERROR_LEVEL, "Firmware request fail\n");
        ret = -EINVAL;
        goto _out;
    }

    dbgprint(INFO_LEVEL, "Firmware data %p size %zu\n", firmware->data, firmware->size);

    if (firmware->size < 0x8000)
    {
        dbgprint(ERROR_LEVEL, "Firmware length two short\n");
        ret = -EINVAL;
        goto _out;
    }

    rawdata_show("Firmware data head", (char *)(firmware->data + 0x7FC0), 0x40);

    ret = Nanosic_firmware_upgrade(i2c_client, (uint8_t *)(firmware->data + 0x7FC0), firmware->size - 0x7FC0);

_out:

    return ret;
}


/** ************************************************************************
 *  @brief
 *
 *  fwpath : /etc/firmware/nano.bin
 ** */
int Nanosic_firmware_init(struct nano_i2c_client *i2c_client)
{
    int ret = 0;
    int retry = 0;
    const struct firmware *fw_entry = NULL;

    while (1)
    {
        // request firmware
        ret = request_firmware(&fw_entry, "MCU_Upgrade.bin", i2c_client->dev);
        if (ret)
        {
            dbgprint(ERROR_LEVEL, "firmware load failed, ret=%d\n", ret);
            goto _firmware_load_fail;
        }

        if (fw_entry)
        {
            ret = firmware_request_complete(fw_entry, i2c_client);
            if (ret)
                goto _firmware_load_fail;

            break;
        }

_firmware_load_fail:
        retry++;
        if (unlikely(retry > 3))
        {
            dbgprint(ERROR_LEVEL, "error, retry=%d\n", retry);
            break;
        }
    }

    if (fw_entry)
        release_firmware(fw_entry);

#ifdef ENABLE_PMIC_DUMP
    if (retry > 3) // that means retry fail
    {
        dump_regmap(i2c_client, i2c_client->dvdd_addr_base);
        dump_regmap(i2c_client, i2c_client->vdd_addr_base);
    }
#endif // ENABLE_PMIC_DUMP


    return ret;
}

/** ************************************************************************
 *  @brief release firmware
 *
 *  fwpath : /system/etc/firmware/nano.bin
 ** */
int Nanosic_firmware_release(struct nano_i2c_client *i2c_client)
{
    return 0;
}
