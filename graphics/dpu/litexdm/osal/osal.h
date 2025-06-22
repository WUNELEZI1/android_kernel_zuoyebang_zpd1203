// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _OSAL_H_
#define _OSAL_H_

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/log2.h>
#else

#include <Library/DebugLib.h>
#include "Protocol/I2cProtocol.h"
#include "xr_ioc_user_def.h"

#define container_of(ptr, type, member) ({\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define bool    _Bool
#define true    1
#define false   0
#endif

#define DPU_DTS_PATH "/soc/dpu@E7000000"
#define DSI_DTS_PATH "/soc/dsi_display@E7051200"
#define PHY_DTS_PATH "/soc/dsi_phy@E8000000"

#define DPU_PHY_ADDR uint64_t
typedef uint32_t u32;
#define BIT(a) (1UL << (a))
#define dpu_read_reg(addr) (*(volatile uint32_t *)(uintptr_t)(addr))

struct dpu_power_mgr {
	struct dpu_panel_info *pinfo;
	u32 profile_id;
	u32 volt_level;
};

enum IO_OPS {
	PULL_DOWN = 0,
	PULL_UP,
};

/**
 * gpio_func_set - set function for this IO
 *
 * @gpio_id: The IO id
 * @func_id: The IO function id
 *
 * @return 0 on success, -1 on failure
 */
int gpio_func_set(uint32_t gpio_id, uint32_t func_id);

/**
 * gpio_pull_status_set - set pull status for this IO
 *
 * @gpio_id: The IO id
 * @pull_status: The IO pull status
 *
 * @return 0 on success, -1 on failure
 */
int gpio_pull_status_set(uint32_t gpio_id, uint8_t pull_status);

/**
 * gpio_pull_status_set - set pull status for this IO
 *
 * @gpio_id: The IO id
 * @func_id: The IO function id
 * @pull_status: The IO pull status
 *
 * @return 0 on success, -1 on failure
 */
int gpio_ioc_prepare(uint32_t gpio_id, uint32_t func_id, uint8_t pull_status);

/**
 * gpio_set_direction_output_value - gpio ops abstracted across os
 *
 * @gpio: gpio pins
 * @value: set pin value
 * @delay_ms: delay ms after gpio operation
 */
void gpio_set_direction_output_value(uint32_t gpio, int32_t value, uint32_t delay_ms);

/**
 * ldo_set_direction_output_value - ldo ops abstracted across os
 *
 * @ldo_num: ldo id
 * @value: set value
 * @delay_ms: delay ms after operation
 */
int ldo_set_direction_output_value(uint32_t ldo_num, enum IO_OPS value, uint32_t delay_ms);

/**
 * gpio_get_direction_input_value - gpio ops abstracted across os
 *
 * @gpio: gpio pins
 * @value: get pin value
 */
void gpio_get_direction_input_value(uint32_t gpio, int32_t *value);

/**
 * gpio_set_direction_input_value - gpio ops abstracted across os
 *
 * @gpio: gpio pins
 * @value: set pin value
 */
void gpio_set_direction_input_value(uint32_t gpio, int32_t value);

/**
 * dpu_power_on - media1 dpu p0 p1 p2 power up
 * @mgr: power and clk control manager
 */
int32_t dpu_power_on(struct dpu_power_mgr *mgr);

/**
 * dpu_power_off - dpu p2 p1 p0 media1 power down
 * @mgr: power and clk control manager
 */
int32_t dpu_power_off(struct dpu_power_mgr *mgr);

/**
 * dsi_power_on - dsi power on
 * @mgr: power and clk control manager
 */
int32_t dsi_power_on(struct dpu_power_mgr *mgr);

/**
 * dsi_power_off - dsi power off
 * @mgr: power and clk control manager
 */
int32_t dsi_power_off(struct dpu_power_mgr *mgr);


void dpu_mem_set(void *buf, uint32_t value, uint32_t size);

/**
 * dpu_mem_cpy - memcpy abstracted across os
 *
 * @dst: dest buf addr
 * @src: src buf addr
 * @size: buf size
 */
void dpu_mem_cpy(void *dst,  const void *src, uint32_t size);

/**
 * dpu_mem_alloc - malloc abstracted across os
 *
 * @size: buf size
 * @return void*
 */
void *dpu_mem_alloc(uint32_t size);

/**
 * dpu_mem_realloc - realloc abstracted across os
 *
 * @old_size: old buf size
 * @new_size: new buf size
 * @old_ptr: old pointer
 * @return new pointer
 */
void *dpu_mem_realloc(uint32_t old_size, uint32_t new_size, void *old_ptr);

/**
 * dpu_mem_free - buf free func abstracted across os
 *
 * @ptr: buf addr
 */
void dpu_mem_free(void *ptr);

/**
 * dpu_write_reg - write reg abstracted across os
 *
 * @addr: reg addr
 * @value: reg value
 */
void dpu_write_reg(uint64_t addr, uint32_t value);

/**
 * dpu_udelay - udelay abstracted across os
 *
 * @us: time
 */
void dpu_udelay(uint32_t us);

/**
 * dpu_mdelay - mdelay abstracted across os
 *
 * @ms: time
 */
void dpu_mdelay(uint32_t ms);

/**
 * dpu_str_cpy - strcpy abstracted across os
 *
 * @dest: dest addr
 * @src: src addr
 * @max_len: str len
 */
void dpu_str_cpy(char* dest, const char* src, uint32_t max_len);

/**
 * dpu_str_cmp - strcmp abstracted across os
 *
 * @str1: first string
 * @str2: second string
 *
 * @return 0: str1 is identical to str2.
 */
int32_t dpu_str_cmp(const char *str1, const char *str2);

/**
 * dpu_get_fdt - Get fdt root node pointer
 *
 * @return The fdt root node pointer
 */
void *dpu_get_fdt(void);

/**
 * dpu_get_subnode_offset - get offset of sub node
 *
 * @fdt: The fdt root node pointer
 * @parent_offset: offset of parent node
 * @name: prop name
 * @return 0 on success, -1 on failure
 */
int32_t dpu_get_subnode_offset(void *fdt, int parent_offset, const char *name);

/**
 * dpu_get_fdt_offset - get fdt node offset
 *
 * @fdt: The fdt root node pointer
 * @path: node path
 * @return FDT node offset
 */
int32_t dpu_get_fdt_offset(void *fdt, const char *path);

/**
 * dpu_str_n_cmp - strncmp abstracted across os
 *
 * @str1: first string
 * @str2: second string
 * @length: The maximum number of ASCII characters for compare.
 *
 * @return 0: the first n characters of str1 and str2 are identical
 */
int32_t dpu_str_n_cmp(const char *str1, const char *str2, uint32_t length);

/**
 * @brief check dpu hardware module whether existed
 *
 * @return true if dpu enable
 * @return false if dpu disabled
 */
bool dpu_check_existence(void);

/**
 * ilog2 - log 2
 *
 * @gray: log gray
 */
int32_t ilog2(int64_t gray);

/**
 * dpu_dts_parse_u8 - ops of parse u8 property
 *
 * @fdt: The fdt root node pointer
 * @offset: offset of parent node
 * @name: prop name
 * @out_data: out data
 * @return 0 on success, -1 on failure
 */
int dpu_dts_parse_u8(void *fdt, int32_t offset, const char *name, uint8_t *out_data);

/**
 * dpu_dts_get_u8_array_size - ops of get u8 property array size
 *
 * @fdt: The fdt root node pointer
 * @offset: offset of parent node
 * @name: prop name
 * @return -1 on failure, otherwise on valid
 */
int dpu_dts_get_u8_array_size(void *fdt, int32_t offset, const char *name);

/**
 * dpu_dts_parse_u8_array - ops of parse u8 property array
 *
 * @fdt: The fdt root node pointer
 * @offset: offset of parent node
 * @name: prop name
 * @out_data: out data
 * @return 0 on success, -1 on failure
 */
int dpu_dts_parse_u8_array(void *fdt, int32_t offset, const char *name, uint8_t *out_data);

/**
 * dpu_dts_parse_u32 - ops of parse u32 property
 *
 * @fdt: The fdt root node pointer
 * @offset: offset of parent node
 * @name: prop name
 * @out_data: out data
 * @return 0 on success, -1 on failure
 */
int dpu_dts_parse_u32(void *fdt, int32_t offset, const char *name, uint32_t *out_data);

/**
 * dpu_dts_get_u32_array_size - ops of get u32 property array size
 *
 * @fdt: The fdt root node pointer
 * @offset: offset of parent node
 * @name: prop name
 * @return -1 on failure, otherwise on valid
 */
int dpu_dts_get_u32_array_size(void *fdt, int32_t offset, const char *name);

/**
 * dpu_dts_parse_u32_array - ops of parse u32 property array
 *
 * @fdt: The fdt root node pointer
 * @offset: offset of parent node
 * @name: prop name
 * @out_data: out data
 * @return 0 on success, -1 on failure
 */
int dpu_dts_parse_u32_array(void *fdt, int32_t offset, const char *name, uint32_t *out_data);

/**
 * dpu_dts_parse_string - ops of parse string property
 *
 * @fdt: The fdt root node pointer
 * @offset: offset of parent node
 * @name: prop name
 * @out_string: out data
 * @return 0 on success, -1 on failure
 */
int dpu_dts_parse_string(void *fdt, int32_t offset, const char *name, const char **out_string);

/**
 * dpu_dts_get_string_array_size - ops of get string property array size
 *
 * @fdt: The fdt root node pointer
 * @offset: offset of parent node
 * @name: prop name
 * @return -1 on failure, others for success
 */
int dpu_dts_get_string_array_size(void *fdt, int32_t offset, const char *name);

/**
 * dpu_dts_parse_string_array - ops of parse string property array
 *
 * @fdt: The fdt root node pointer
 * @offset: offset of parent node
 * @name: prop name
 * @out_strs: output array of string pointers.
 * @size: number of array elements to read.
 * @return -1 on failure, others for success.
 */
int dpu_dts_parse_string_array(void *fdt, int32_t offset,
		const char *name, const char **out_strs, size_t size);

/**
 * dpu_dts_read_bool - ops of read bool property
 *
 * @fdt: The fdt root node pointer
 * @offset: offset of parent node
 * @name: prop name
 * @return true if the property exists false otherwise
 */
bool dpu_dts_read_bool(void *fdt, int32_t offset, const char *name);

/**
 * dpu_dts_update_string_prop - ops of set string property
 *
 * @fdt: The fdt root node pointer
 * @offset: offset of parent node
 * @prop_name: prop name
 * @new_value: set new value
 * @return 0 on success, -1 on failure
 */
void dpu_dts_update_string_prop(void *fdt, int32_t offset, const char *prop_name, const char *new_value);

/**
 * dpu_dts_get_offset_by_phandle - ops of get node offset by phandle
 *
 * @fdt: The fdt root node pointer
 * @phandle: phandle of node
 * @return 0 on success, -1 on failure
 */
int dpu_dts_get_offset_by_phandle(void *fdt, uint32_t phandle);

/**
 * i2c_write - I2C write
 *
 * @bus_id: I2C bus id
 * @addr: I2C slave address
 * @reg: Slave register
 * @reg_len: Slave register length
 * @buf: Write buffer
 * @data_len: Write data length
 */
int i2c_write(uint32_t bus_id, uint16_t addr, uint32_t reg, uint32_t reg_len,
		uint8_t *buf, uint32_t data_len);

/**
 * i2c_read - I2C read
 *
 * @bus_id: I2C bus id
 * @addr: I2C slave address
 * @reg: Slave register
 * @reg_len: Slave register length
 * @buf: Read buffer
 * @data_len: Read data length
 */
int i2c_read(uint32_t bus_id, uint16_t addr, uint32_t reg, uint32_t reg_len,
		uint8_t *buf, uint32_t data_len);

#endif /* _OSAL_H_ */
