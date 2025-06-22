// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/io.h>
#include <linux/version.h>
#include <dt-bindings/xring/platform-specific/tsensorc.h>
#include <dt-bindings/xring/platform-specific/common/tsens/tsens_id.h>
#include "xr_soc_tm.h"

#define TSENS_VENDOR_OFFSET	(0x2000)
#define TSENS_MSG_FAULTN_BIT	(31UL)
#define TSENS_MSG_BITS		(9UL)
#define TSENS_MSG_SHIFT		(14UL)
#define TSENS_MSG_REG_OFFSET	(TSENS_VENDOR_OFFSET + TSENSORC_DTS_SENSOR1_MSG)
#define TSENS_MMSG_REG_OFFSET	(TSENS_VENDOR_OFFSET + TSENSORC_MONITOR_MSG0)
#define TSENS_MMSG_MAX_SHIFT	(TSENSORC_MONITOR_MSG0_MAX_TEMP_MSG_SHIFT)
#define TSENS_MMSG_MIN_SHIFT	(TSENSORC_MONITOR_MSG0_MIN_TEMP_MSG_SHIFT)
#define BITS_MASK(bits)		((1UL << (bits)) - 1UL)

static inline int sign_extend(uint32_t val, uint32_t shift, uint32_t bits)
{
	uint32_t rval;
	uint32_t mask;

	mask = BITS_MASK(bits);
	rval = val >> shift;
	rval &= mask;
	if (rval & BIT(bits - 1UL))
		rval |= (~mask);
	return (int)rval;
}

static int tsens_probe_v1_0(struct xr_thermal_data *data)
{
	return 0;
}

static int tsens_read_temp_v1_0(struct xr_thermal_data *data,
				int chann, int *temp)
{
	uint32_t offset;
	uint32_t vreg;
	uint32_t vshift;
	uint32_t vbits;

	if ((chann > TSENS_ID_MIN) && (chann < TSENS_ID_MAX))
		offset = TSENS_MSG_REG_OFFSET + ((uint32_t)chann << 2);
	else
		offset = TSENS_MMSG_REG_OFFSET;

	vreg = readl(data->regs + offset);
	if (chann <= TSENS_ID_MIN) {
		vshift = TSENS_MMSG_MIN_SHIFT;
		vbits = TSENS_MSG_BITS;
	} else if (chann >= TSENS_ID_MAX) {
		vshift = TSENS_MMSG_MAX_SHIFT;
		vbits = TSENS_MSG_BITS;
	} else {
		vshift = TSENS_MSG_SHIFT;
		vbits = TSENS_MSG_BITS;
		if ((vreg & BIT(TSENS_MSG_FAULTN_BIT)) == 0)
			return -EINVAL;
	}
	*temp = sign_extend(vreg, vshift, vbits);
	*temp *= 1000;
	return 0;
}

struct xr_thermal_tsens_ops tsens_data_v1 = {
	.read_temp	= tsens_read_temp_v1_0,
	.probe		= tsens_probe_v1_0
};
