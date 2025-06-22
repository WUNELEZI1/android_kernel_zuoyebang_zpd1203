// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */
#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "tof_stmvl53l8_api", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "tof_stmvl53l8_api", __func__, __LINE__

#include <linux/firmware.h>
#include "xrisp_tof.h"

#define DEVICE_ID_REG_ADDR    0x0000
#define REVISION_ID_REG_ADDR  0x0001
#define MCU_STATUS_0_REG_ADDR 0x0006
#define MCU_STATUS_1_REG_ADDR 0x0007
#define XSHUT_CTRL_REG_ADDR   0x0009
#define PAGE_SELECT_REG_ADDR  0x7FFF

#define DEVICE_ID 0xF0
#define REVISION_ID 0x0C

#define GO2_PAGE 0x00
#define GO1_PAGE 0x01
#define DEFAULT_PAGE 0x02

#define XHUT_GPIO_LOW 0
#define XHUT_GPIO_HIGH 1
#define REGULATOR_DISABLE 0
#define REGULATOR_ENABLE 1

#define BOOT_COMPLETION_TIMEOUT_MS 500
#define COMMS_BUFFER_SIZE_BYTES	5132

enum fw_boot_state {
	FW_BOOT_STATE_LOW = 0,
	FW_BOOT_STATE_HIGH = 1,
	FW_BOOT_STATE_ERROR = 2
};

struct fw_header {
	uint32_t ver_major;
	uint32_t ver_minor;
	uint32_t ver_build;
	uint32_t ver_revision;
	uint32_t fw_offset;
	uint32_t fw_size;
};

static int set_manual_xshut_state(struct xrisp_tof_data *tof_data, uint8_t state)
{
	int ret = 0;
	uint16_t reg_addr = XSHUT_CTRL_REG_ADDR;
	uint8_t reg_val = 0;

	ret = tof_read_byte(tof_data, reg_addr, &reg_val);
	if (ret)
		return ret;

	reg_val &= 0xf8;
	reg_val |= state ? 0x04 : 0x02;
	ret = tof_write_byte(tof_data, reg_addr, reg_val);
	return ret;
}

static int set_regulators(struct xrisp_tof_data *tof_data, uint8_t lp_reg_enable,
		uint8_t hp_reg_enable)
{
	int ret = 0;
	uint16_t reg_addr = 0x000F;
	uint8_t reg_val = 0;
	uint8_t and_mask = 0xFC;
	uint8_t or_mask = 0;

	or_mask |= lp_reg_enable ? 0x00 : 0x02;
	or_mask |= hp_reg_enable ? 0x00 : 0x01;

	ret = tof_read_byte(tof_data, reg_addr, &reg_val);
	if (ret)
		goto exit;
	reg_val &= and_mask;
	reg_val |= or_mask;
	ret = tof_write_byte(tof_data, reg_addr, reg_val);

exit:
	return ret;
}

static int wait_mcu_boot(struct xrisp_tof_data *tof_data, enum fw_boot_state state,
		uint32_t wait_ms, uint32_t timeout_ms)
{
	int ret = 0;
	uint32_t start_ms = 0;
	uint32_t current_ms = 0;

	tof_get_tick_count(&start_ms);
	if (timeout_ms == 0)
		timeout_ms = BOOT_COMPLETION_TIMEOUT_MS;
	if (wait_ms > timeout_ms)
		wait_ms = timeout_ms;
	tof_data->firmware.mcu_reg0.bytes = 0;
	tof_data->firmware.mcu_reg1.bytes = 0;

	do {
		ret = tof_read_byte(tof_data, MCU_STATUS_0_REG_ADDR,
			&tof_data->firmware.mcu_reg0.bytes);
		if (ret)
			goto exit;
		else
			XRISP_PR_INFO("Read reg_addr=0x%x, reg_val=0x%x", MCU_STATUS_0_REG_ADDR,
				tof_data->firmware.mcu_reg0.bytes);

		if (tof_data->firmware.mcu_reg0.mcu_hw_trap_flag) {
			ret = tof_read_byte(tof_data, MCU_STATUS_1_REG_ADDR,
					    &tof_data->firmware.mcu_reg1.bytes);
			if (ret)
				goto exit;
			else
				XRISP_PR_INFO("Read reg_addr=0x%x, reg_val=0x%x",
					      MCU_STATUS_1_REG_ADDR,
					      tof_data->firmware.mcu_reg1.bytes);

			if (tof_data->firmware.mcu_reg1.bytes) {
				XRISP_PR_ERROR("MCU error in hw state.");
				ret = -1;
				goto exit;
			} else {
				XRISP_PR_ERROR("MCU error in power state.");
				ret = -2;
				goto exit;
			}
		}

		if (state == FW_BOOT_STATE_ERROR) {
			if (tof_data->firmware.mcu_reg0.mcu_error_flag) {
				XRISP_PR_ERROR("MCU error in wait state.");
				return -3;
			}
		} else {
			if (((state == FW_BOOT_STATE_HIGH) &&
				tof_data->firmware.mcu_reg0.mcu_boot_complete) ||
				((state == FW_BOOT_STATE_LOW) &&
				(!tof_data->firmware.mcu_reg0.mcu_boot_complete)))
				goto exit_error;
		}

		tof_get_tick_count(&current_ms);
		ret = tof_check_for_timeout(start_ms, current_ms, timeout_ms);
		if (ret) {
			XRISP_PR_ERROR("MCU startup timeout.");
			goto exit;
		}
		mdelay(wait_ms);
	} while (true);

exit_error:
	if (tof_data->firmware.mcu_reg0.mcu_error_flag) {
		(void)tof_read_byte(tof_data, MCU_STATUS_1_REG_ADDR,
			&tof_data->firmware.mcu_reg1.bytes);
		if (tof_data->firmware.mcu_reg1.mcu_spare1) {
			XRISP_PR_ERROR("MCU NVM is not programed.");
			ret = -4;
		} else {
			XRISP_PR_ERROR("MCU error in wait state.");
			ret = -3;
		}
	}
exit:
	return ret;
}

static int set_power_to_hp(struct xrisp_tof_data *tof_data)
{
	int ret = 0;
	enum xrisp_tof_power_mode current_mode = tof_data->power_mode;

	if (current_mode == XRISP_TOF_POWER_MODE_HP) {
		XRISP_PR_INFO("The current power ret is already XRISP_TOF_POWER_MODE_HP");
		return ret;
	}

	ret = tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, GO2_PAGE);
	if (ret)
		goto exit;

	// if (current_mode == XRISP_TOF_POWER_MODE_OFF &&
	//	   tof_data->firmware.device_booted) {
	//     ret = set_manual_xshut_state(tof_data, XHUT_GPIO_HIGH);
	// } else if (current_mode == XRISP_TOF_POWER_MODE_ULP) {
	//     ret = set_manual_xshut_state(tof_data, XHUT_GPIO_HIGH) ||
	//         set_regulators(tof_data, REGULATOR_ENABLE, REGULATOR_ENABLE);
	// } else if (current_mode == XRISP_TOF_POWER_MODE_LP) {
	//     ret = set_manual_xshut_state(tof_data, XHUT_GPIO_HIGH);
	// } else {
	//     ret = -1;
	// }
	ret = set_manual_xshut_state(tof_data, XHUT_GPIO_HIGH);
	if (ret)
		goto restore_page;

	if (current_mode == XRISP_TOF_POWER_MODE_ULP) {
		ret = set_regulators(tof_data, REGULATOR_ENABLE, REGULATOR_ENABLE);
		if (ret)
			goto restore_page;
	}

	ret = wait_mcu_boot(tof_data, FW_BOOT_STATE_HIGH, 5, 0);
	if (ret) {
		XRISP_PR_ERROR("Set power mode to HP failed.");
		goto restore_page;
	} else {
		tof_data->power_mode = XRISP_TOF_POWER_MODE_HP;
		XRISP_PR_INFO("Set power mode to HP successfully.");
	}

	ret = tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, DEFAULT_PAGE);
	if (ret)
		goto exit;

	if (current_mode == XRISP_TOF_POWER_MODE_ULP)
		ret = tof_load_firmware(tof_data);

	return ret;

restore_page:
	(void)tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, DEFAULT_PAGE);
exit:
	return ret;
}

static int set_power_to_lp(struct xrisp_tof_data *tof_data)
{
	int ret = 0;
	enum xrisp_tof_power_mode current_mode = tof_data->power_mode;

	if (current_mode == XRISP_TOF_POWER_MODE_LP) {
		XRISP_PR_INFO("The current power mode is already XRISP_TOF_POWER_MODE_LP");
		return ret;
	} else if (current_mode != XRISP_TOF_POWER_MODE_HP) {
		ret = set_power_to_hp(tof_data);
		if (ret)
			goto exit;
	}

	ret = tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, GO2_PAGE);
	if (ret)
		goto exit;

	ret = set_manual_xshut_state(tof_data, XHUT_GPIO_LOW) ||
		wait_mcu_boot(tof_data, FW_BOOT_STATE_LOW, 5, 0);
	if (ret)
		XRISP_PR_ERROR("Set power mode to LP failed.");
	else {
		tof_data->power_mode = XRISP_TOF_POWER_MODE_LP;
		XRISP_PR_INFO("Set power mode to LP successfully.");
	}

	(void)tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, DEFAULT_PAGE);
exit:
	return ret;
}

static int set_power_to_ulp(struct xrisp_tof_data *tof_data)
{
	int ret = 0;
	enum xrisp_tof_power_mode current_mode = tof_data->power_mode;

	if (current_mode == XRISP_TOF_POWER_MODE_ULP) {
		XRISP_PR_INFO("The current power mode is already XRISP_TOF_POWER_MODE_ULP");
		return ret;
	} else if (current_mode != XRISP_TOF_POWER_MODE_HP) {
		ret = set_power_to_hp(tof_data);
		if (ret)
			goto exit;
	}

	ret = tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, GO2_PAGE);
	if (ret)
		goto exit;

	ret = set_manual_xshut_state(tof_data, XHUT_GPIO_LOW) ||
		wait_mcu_boot(tof_data, FW_BOOT_STATE_LOW, 5, 0) ||
		set_regulators(tof_data, REGULATOR_DISABLE, REGULATOR_DISABLE);
	if (ret)
		XRISP_PR_ERROR("Set power mode to ULP failed.");
	else {
		tof_data->power_mode = XRISP_TOF_POWER_MODE_ULP;
		XRISP_PR_INFO("Set power mode to ULP successfully.");
	}

	(void)tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, DEFAULT_PAGE);
exit:
	return ret;
}

static int wait_for_boot_complete_before_fw_load(struct xrisp_tof_data *tof_data)
{
	int ret = 0;
	uint32_t start_ms = 0;
	uint32_t current_ms = 0;
	uint8_t reg_val = 0;

	ret = tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, GO1_PAGE);
	if (ret)
		goto exit;
	ret = tof_write_byte(tof_data, 0x0006, 0x01);
	if (ret)
		goto exit;

	tof_get_tick_count(&start_ms);
	while ((reg_val & 0x04) == 0) {
		ret = tof_read_byte(tof_data, 0x0021, &reg_val);
		if (ret)
			goto exit;
		tof_get_tick_count(&current_ms);
		ret = tof_check_for_timeout(start_ms, current_ms, BOOT_COMPLETION_TIMEOUT_MS);
		if (ret) {
			XRISP_PR_ERROR("Wait for MCU timeout, reg_val=0x%x", reg_val);
			goto exit;
		}
	}

	ret = tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, GO2_PAGE);
	if (ret)
		goto exit;
	ret = tof_write_byte(tof_data, 0x000C, 0x01);
	if (ret)
		goto exit;

	ret = tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, GO2_PAGE);
	if (ret)
		goto exit;

	ret = tof_write_byte(tof_data, 0x0101, 0x00) ||
		tof_write_byte(tof_data, 0x0102, 0x00) ||
		tof_write_byte(tof_data, 0x010A, 0x00);
	if (ret)
		goto exit;

	ret = tof_write_byte(tof_data, 0x4002, 0x01) ||
		tof_write_byte(tof_data, 0x4002, 0x00);
	if (ret)
		goto exit;

	ret = tof_write_byte(tof_data, 0x0103, 0x01) ||
		tof_write_byte(tof_data, 0x010A, 0x03);

exit:
	return ret;
}

static int wait_for_boot_complete_after_fw_load(struct xrisp_tof_data *tof_data)
{
	int ret = 0;
	uint8_t reg_val[] = {0x00, 0x00, 0x42, 0x00};

	ret = tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, GO2_PAGE);
	if (ret)
		goto exit;

	ret = tof_write_byte(tof_data, 0x400F, 0x00) ||
		tof_write_byte(tof_data, 0x021A, 0x43) ||
		tof_write_byte(tof_data, 0x021A, 0x03) ||
		tof_write_byte(tof_data, 0x021A, 0x01) ||
		tof_write_byte(tof_data, 0x021A, 0x00);
	if (ret)
		goto exit;

	ret = tof_write_byte(tof_data, 0x0219, 0x00) ||
		tof_write_byte(tof_data, 0x021B, 0x00);
	if (ret)
		goto exit;

	ret = tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, GO2_PAGE);
	if (ret)
		goto exit;

	ret = tof_write_block(tof_data, 0x0114, reg_val, 4);
	if (ret)
		goto exit;

	ret = tof_write_byte(tof_data, 0x000B, 0x00) ||
		tof_write_byte(tof_data, 0x000C, 0x00) ||
		tof_write_byte(tof_data, 0x000B, 0x01);
	if (ret)
		goto exit;

	ret = wait_mcu_boot(tof_data, FW_BOOT_STATE_HIGH, 50, 0);
exit:
	return ret;
}

static uint32_t fwbuf_decode_uint32_t(uint8_t *buf)
{
	uint32_t value = 0x00;

	for (int i = 0; i < 4; i++)
		value = (value << 8) | (uint32_t)buf[3 - i];
	return value;
}

static int write_data_to_ram(struct xrisp_tof_data *tof_data, uint8_t *fw_buffer)
{
	int ret = 0;
	uint32_t count = 0;
	uint8_t *comms_buffer = NULL;
	uint32_t tcpm_offset = fwbuf_decode_uint32_t(fw_buffer + 32);
	uint32_t tcpm_size = fwbuf_decode_uint32_t(fw_buffer + 36);//39096
	uint32_t tcpm_page = fwbuf_decode_uint32_t(fw_buffer + 40);//9
	// uint32_t tcpm_page_offset = fwbuf_decode_uint32_t(fw_buffer + 44);

	comms_buffer = kzalloc(COMMS_BUFFER_SIZE_BYTES, GFP_KERNEL);
	if (!comms_buffer) {
		XRISP_PR_INFO("Failed to alloc comms_buffer's memory.");
		return -ENOMEM;
	}

	for (uint32_t page = tcpm_page, page_size = 0; page < 12; page++) {
		ret = tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, (uint8_t)page);
		if (ret)
			goto exit;

		if (page == 9 || page == 10) // page9:32768, page10:6328
			page_size = 0x8000;
		else if (page == 11)
			page_size = 0x5000;
		if (tcpm_size - count < page_size)
			page_size = tcpm_size - count;

		for (uint32_t page_offset = 0; page_offset < page_size;
			page_offset += COMMS_BUFFER_SIZE_BYTES) {
			uint32_t len = page_offset + COMMS_BUFFER_SIZE_BYTES < page_size ?
				COMMS_BUFFER_SIZE_BYTES : page_size - page_offset;

			ret = tof_write_block(tof_data, page_offset,
				fw_buffer + tcpm_offset + count, len);
			if (ret)
				goto exit;
			count += len;
		}

		if (count == tcpm_size)
			break;
	}

exit:
	kfree(comms_buffer);
	return ret;
}

static int write_patch_code(struct xrisp_tof_data *tof_data, uint8_t *fw_buffer)
{
	int ret = 0;
	uint32_t offset = fwbuf_decode_uint32_t(fw_buffer + 112);
	uint32_t size = fwbuf_decode_uint32_t(fw_buffer + 116);// 4
	uint32_t page = fwbuf_decode_uint32_t(fw_buffer + 120);
	uint32_t page_offset = fwbuf_decode_uint32_t(fw_buffer + 124);

	ret = tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, page);
	if (ret)
		goto exit;

	ret = tof_write_block(tof_data, page_offset, fw_buffer + offset, size);
exit:
	return ret;
}

static int write_checksum_en(struct xrisp_tof_data *tof_data, uint8_t *fw_buffer)
{
	int ret = 0;
	uint32_t offset = fwbuf_decode_uint32_t(fw_buffer + 96);
	uint32_t size = fwbuf_decode_uint32_t(fw_buffer + 100);// 4
	uint32_t page = fwbuf_decode_uint32_t(fw_buffer + 104);
	uint32_t page_offset = fwbuf_decode_uint32_t(fw_buffer + 108);

	ret = tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, page);
	if (ret)
		goto exit;

	ret = tof_write_block(tof_data, page_offset, fw_buffer + offset, size);
exit:
	return ret;
}

static int check_fw_checksum(struct xrisp_tof_data *tof_data, uint8_t *fw_buffer)
{
	int ret = 0;
	uint32_t checksum = fwbuf_decode_uint32_t(fw_buffer + 28);// 0x29F61EEB
	uint32_t real_checksum = 0;
	uint16_t reg_addr = 0x812FFC & 0xFFFF;
	uint8_t reg_val[4] = {0};

	ret = tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, DEFAULT_PAGE);
	if (ret)
		goto exit;

	ret = tof_read_block(tof_data, reg_addr, reg_val, 4);
	if (ret)
		goto exit;
	real_checksum = (uint32_t)((reg_val[0] << 24) | (reg_val[1] << 16) |
		(reg_val[2] << 8) | reg_val[3]);

	if (real_checksum != checksum) {
		XRISP_PR_ERROR("expected checksum=0x%x, real checksum=0x%x",
			checksum, real_checksum);
		ret = -1;
	}

exit:
	return ret;
}

int tof_read_chip_info(struct xrisp_tof_data *tof_data)
{
	int ret = 0;

	ret = tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, GO2_PAGE);
	if (ret)
		goto exit;

	ret = tof_read_byte(tof_data, DEVICE_ID_REG_ADDR,
		&tof_data->firmware.device_id);
	if (ret)
		goto restore_page;
	else
		XRISP_PR_INFO("Read device_id=0x%X.", tof_data->firmware.device_id);

	ret = tof_read_byte(tof_data, REVISION_ID_REG_ADDR,
			&tof_data->firmware.revision_id);
	if (ret)
		goto restore_page;
	else
		XRISP_PR_INFO("Read revision_id=0x%X.", tof_data->firmware.revision_id);

	if ((tof_data->firmware.device_id != DEVICE_ID) ||
		(tof_data->firmware.revision_id != REVISION_ID)) {
		XRISP_PR_ERROR("device_id or revision_id is invalid.");
		ret = -1;
	}

restore_page:
	(void)tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, DEFAULT_PAGE);
exit:
	return ret;
}

int tof_set_power_mode(struct xrisp_tof_data *tof_data, enum xrisp_tof_power_mode mode)
{
	int ret = 0;

	switch (mode) {
	case XRISP_TOF_POWER_MODE_HP:
		ret = set_power_to_hp(tof_data);
		break;
	case XRISP_TOF_POWER_MODE_LP:
		ret = set_power_to_lp(tof_data);
		break;
	case XRISP_TOF_POWER_MODE_ULP:
		ret = set_power_to_ulp(tof_data);
		break;
	case XRISP_TOF_POWER_MODE_RANGING:
	case XRISP_TOF_POWER_MODE_OFF:
	default:
		XRISP_PR_ERROR("power mode is invalid.");
		break;
	}

	return ret;
}

int check_rom_firmware_boot(struct xrisp_tof_data *tof_data)
{
	int ret = 0;

	ret = tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, GO2_PAGE);
	if (ret)
		goto exit;

	tof_data->firmware.mcu_reg1.bytes = 0x00;
	ret = tof_read_byte(tof_data, MCU_STATUS_1_REG_ADDR,
		&tof_data->firmware.mcu_reg1.bytes);
	if (ret)
		goto restore_page;

	tof_data->firmware.device_booted = tof_data->firmware.mcu_reg1.mcu_spare0 ?
		true : false;

	if (tof_data->firmware.device_booted)
		goto restore_page;

	ret = wait_mcu_boot(tof_data, FW_BOOT_STATE_HIGH, 5, 0);
	(void)tof_write_byte(tof_data, 0x000E, 0x01);

restore_page:
	(void)tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, DEFAULT_PAGE);
exit:
	return ret;
}

int tof_load_firmware(struct xrisp_tof_data *tof_data)
{
	int ret = 0;
	uint8_t *fw_buffer = NULL;
	const struct firmware *fw = NULL;
	const struct fw_header *header = NULL;

	ret = request_firmware(&fw, tof_data->firmware.fw_name,
		&tof_data->device->dev);
	if (ret || !fw) {
		XRISP_PR_ERROR("Failed to request firmware %s.",
			tof_data->firmware.fw_name);
		return -1;
	}

	header = (struct fw_header *)(fw->data);
	XRISP_PR_INFO("Firmware version info: major=%d, minor=%d, build=%d, revision=%d",
		header->ver_major,
		header->ver_minor,
		header->ver_build,
		header->ver_revision);

	fw_buffer = kzalloc(header->fw_size, GFP_KERNEL);
	if (!fw_buffer) {
		XRISP_PR_ERROR("Failed to alloc fw_buffer's memory.");
		ret = -ENOMEM;
		goto exit;
	}
	memcpy(fw_buffer, &(fw->data)[header->fw_offset], header->fw_size);

	XRISP_PR_INFO("Start to load firmware.");
	ret = wait_for_boot_complete_before_fw_load(tof_data);
	if (ret)
		goto restore_page;

	ret = write_data_to_ram(tof_data, fw_buffer);
	if (ret)
		goto restore_page;

	ret = write_patch_code(tof_data, fw_buffer);
	if (ret)
		goto restore_page;

	ret = write_checksum_en(tof_data, fw_buffer);
	if (ret)
		goto restore_page;

	ret = wait_for_boot_complete_after_fw_load(tof_data);
	if (ret)
		goto restore_page;

	ret = check_fw_checksum(tof_data, fw_buffer);
	tof_data->firmware.fw_loaded = ret ? false : true;
	if (ret)
		goto restore_page;
	XRISP_PR_INFO("Load firmware '%s' successfully.", tof_data->firmware.fw_name);

restore_page:
	(void)tof_write_byte(tof_data, PAGE_SELECT_REG_ADDR, DEFAULT_PAGE);
	kfree(fw_buffer);
exit:
	release_firmware(fw);
	return ret;
}
