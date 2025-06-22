/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __XRING_SMMU_WRAPPER_H__
#define __XRING_SMMU_WRAPPER_H__

#include <linux/dma-buf.h>

enum xring_smmu_tcu_option {
	XRING_SMMU_TCU_POWER_ON = 0,
	XRING_SMMU_TCU_POWER_OFF,
};

enum xring_smmu_tbu_option {
	XRING_SMMU_TBU_POWER_ON = 0,
	XRING_SMMU_TBU_POWER_OFF,
};

int xring_smmu_tcu_ctrl(struct device *dev, enum xring_smmu_tcu_option option);
int xring_smmu_tbu_ctrl(struct device *dev, char *tbu_name, enum xring_smmu_tbu_option option);

#ifdef CONFIG_XRING_SMMU_DOMAIN_SHARE
int xring_smmu_sid_set(struct device *dev, char *sid_name, int sid, int ssid, int ssidv);
#else
int xring_smmu_sid_set(struct device *dev, char *sid_name, int sid);
#endif

#endif
