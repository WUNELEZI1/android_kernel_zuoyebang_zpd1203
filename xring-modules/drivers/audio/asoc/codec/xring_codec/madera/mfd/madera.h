/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * MFD internals for Cirrus Logic Madera codecs
 *
 * Copyright 2015-2018 Cirrus Logic
 */

#ifndef MADERA_MFD_H
#define MADERA_MFD_H

#include <linux/of.h>
#include <linux/pm.h>

#include "../include/mfd/madera/core.h"

//#define MADERA_FPGA_DEBUG

struct madera;

extern const struct dev_pm_ops madera_pm_ops;
extern const struct of_device_id madera_of_match[];

int madera_dev_init(struct madera *madera);
int madera_dev_exit(struct madera *madera);
int __maybe_unused madera_runtime_resume(struct device *dev);
int __maybe_unused madera_runtime_suspend(struct device *dev);

const char *madera_name_from_type(enum madera_type type);

extern const struct regmap_config cs47l15_16bit_spi_regmap;
extern const struct regmap_config cs47l15_32bit_spi_regmap;
extern const struct regmap_config cs47l15_16bit_i2c_regmap;
extern const struct regmap_config cs47l15_32bit_i2c_regmap;
int cs47l15_patch(struct madera *madera);

extern const struct regmap_config cs47l35_16bit_spi_regmap;
extern const struct regmap_config cs47l35_32bit_spi_regmap;
extern const struct regmap_config cs47l35_16bit_i2c_regmap;
extern const struct regmap_config cs47l35_32bit_i2c_regmap;
int cs47l35_patch(struct madera *madera);

extern const struct regmap_config cs47l85_16bit_spi_regmap;
extern const struct regmap_config cs47l85_32bit_spi_regmap;
extern const struct regmap_config cs47l85_16bit_i2c_regmap;
extern const struct regmap_config cs47l85_32bit_i2c_regmap;
int cs47l85_patch(struct madera *madera);

extern const struct regmap_config cs47l90_16bit_spi_regmap;
extern const struct regmap_config cs47l90_32bit_spi_regmap;
extern const struct regmap_config cs47l90_16bit_i2c_regmap;
extern const struct regmap_config cs47l90_32bit_i2c_regmap;
int cs47l90_patch(struct madera *madera);

extern const struct regmap_config cs47l92_16bit_spi_regmap;
extern const struct regmap_config cs47l92_32bit_spi_regmap;
extern const struct regmap_config cs47l92_16bit_i2c_regmap;
extern const struct regmap_config cs47l92_32bit_i2c_regmap;
int cs47l92_patch(struct madera *madera);
int codec_reg_debugs_write(unsigned int reg, unsigned int val);
int codec_reg_debugs_read(unsigned int reg);
void madera_configure_power_fpga(struct madera *madera);
void madera_disable_power_fpga(struct madera *madera);
int madera_i2c_init(struct i2c_client *i2c, const struct i2c_device_id *id);
int madera_i2c_deinit(struct i2c_client *i2c);

#endif
