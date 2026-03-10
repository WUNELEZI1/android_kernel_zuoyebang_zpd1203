/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
 */
#ifndef __SC853x_IIO_H
#define __SC853x_IIO_H

#include <linux/iio/iio.h>
#include <dt-bindings/iio/qti_power_supply_iio.h>

struct SC853x_iio_channels {
    const char *datasheet_name;
    int channel_num;
    enum iio_chan_type type;
    long info_mask;
};

#define SC853x_IIO_CHAN(_name, _num, _type, _mask)		\
    {						\
        .datasheet_name = _name,		\
        .channel_num = _num,			\
        .type = _type,				\
        .info_mask = _mask,			\
    },
#define SC853x_CHAN_VOLT(_name, _num)			\
    SC853x_IIO_CHAN(_name, _num, IIO_VOLTAGE,		\
        BIT(IIO_CHAN_INFO_PROCESSED))
#define SC853x_CHAN_CUR(_name, _num)			\
    SC853x_IIO_CHAN(_name, _num, IIO_CURRENT,		\
        BIT(IIO_CHAN_INFO_PROCESSED))
#define SC853x_CHAN_ENERGY(_name, _num)			\
    SC853x_IIO_CHAN(_name, _num, IIO_ENERGY,		\
        BIT(IIO_CHAN_INFO_PROCESSED))

static const struct SC853x_iio_channels sc853x_iio_psy_channels[] = {
    SC853x_CHAN_ENERGY("sc_present", PSY_IIO_SC_PRESENT)
    SC853x_CHAN_ENERGY("sc_charging_enabled", PSY_IIO_SC_CHARGING_ENABLED)
    SC853x_CHAN_VOLT("sc_bus_voltage", PSY_IIO_SC_BUS_VOLTAGE)
    SC853x_CHAN_CUR("sc_bus_current", PSY_IIO_SC_BUS_CURRENT)
    SC853x_CHAN_ENERGY("sc_adc_ctrol", PSY_IIO_SC_ADC_CONTROL)
    SC853x_CHAN_ENERGY("sc_otg_ctrol", PSY_IIO_SC_OTG_CONTROL)
    SC853x_CHAN_ENERGY("sc_status", PSY_IIO_SC_STATUS)
    SC853x_CHAN_ENERGY("sc_manufacturer", PSY_IIO_SC_CHIP_VENDOR)
    SC853x_CHAN_ENERGY("sc_ovp_gate_ctrol", PSY_IIO_SC_OVP_GATE_CONTROL)
};

#endif