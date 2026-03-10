/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
 */
#ifndef __BQ25960_CHARGER_IIO_H_
#define __BQ25960_CHARGER_IIO_H_

#include <linux/iio/iio.h>
#include <dt-bindings/iio/qti_power_supply_iio.h>

struct bq25960_iio_channels {
    const char *datasheet_name;
    int channel_num;
    enum iio_chan_type type;
    long info_mask;
};

#define BQ25960_IIO_CHAN(_name, _num, _type, _mask)		\
    {						\
        .datasheet_name = _name,		\
        .channel_num = _num,			\
        .type = _type,				\
        .info_mask = _mask,			\
    },
#define BQ25960_CHAN_VOLT(_name, _num)			\
    BQ25960_IIO_CHAN(_name, _num, IIO_VOLTAGE,		\
        BIT(IIO_CHAN_INFO_PROCESSED))
#define BQ25960_CHAN_CUR(_name, _num)			\
    BQ25960_IIO_CHAN(_name, _num, IIO_CURRENT,		\
        BIT(IIO_CHAN_INFO_PROCESSED))
#define BQ25960_CHAN_ENERGY(_name, _num)			\
    BQ25960_IIO_CHAN(_name, _num, IIO_ENERGY,		\
        BIT(IIO_CHAN_INFO_PROCESSED))

static const struct bq25960_iio_channels bq25960_iio_psy_channels[] = {
    BQ25960_CHAN_ENERGY("bq_present", PSY_IIO_SC_PRESENT)
    BQ25960_CHAN_ENERGY("bq_charging_enabled", PSY_IIO_SC_CHARGING_ENABLED)
    BQ25960_CHAN_VOLT("bq_bus_voltage", PSY_IIO_SC_BUS_VOLTAGE)
    BQ25960_CHAN_CUR("bq_bus_current", PSY_IIO_SC_BUS_CURRENT)
    BQ25960_CHAN_ENERGY("bq_adc_ctrol", PSY_IIO_SC_ADC_CONTROL)
    BQ25960_CHAN_ENERGY("bq_otg_ctrol", PSY_IIO_SC_OTG_CONTROL)
    BQ25960_CHAN_ENERGY("bq_status", PSY_IIO_SC_STATUS)
    BQ25960_CHAN_ENERGY("bq_manufacturer", PSY_IIO_SC_CHIP_VENDOR)
    BQ25960_CHAN_ENERGY("bq_ovp_gate_ctrol", PSY_IIO_SC_OVP_GATE_CONTROL)
};

#endif /* __BQ25960_CHARGER_IIO_H_ */