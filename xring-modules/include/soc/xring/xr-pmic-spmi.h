// SPDX-License-Identifier: GPL-2.0-only

#ifndef XR_PMIC_SPMI_HEADER_H
#define XR_PMIC_SPMI_HEADER_H

extern int xr_pmic_reg_read(unsigned int reg, unsigned int *val);

extern int xr_pmic_reg_write(unsigned int reg, unsigned int val);

extern int xr_pmic_reg_bulk_read(unsigned int reg, void *val, size_t val_count);

extern int xr_pmic_reg_bulk_write(unsigned int reg, const void *val, size_t val_count);

#endif