/* SPDX-License-Identifier: GPL-2.0 */
/*
 * SMMU pagetable dump.
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 */

#ifndef __XRING_SMMU_DEBUGFS_H__
#define __XRING_SMMU_DEBUGFS_H__

#include "xring_smmu.h"
#include "xring_smmu_wrapper_reg.h"

#define SMMU_INFO_MAX		4
#define SMMU_NAME_LEN		256

enum dump_type {
	DUMP_STE = 1,
	DUMP_CD = 2,
	DUMP_STRTAB = 4,
};

struct xring_smmu_debugfs_info {
	struct arm_smmu_device *smmu;
	char name[SMMU_NAME_LEN];
};

struct xring_iommu_group {
	struct kobject kobj;
	struct kobject *devices_kobj;
	struct list_head devices;
	struct mutex mutex;
	struct blocking_notifier_head notifier;
	void *iommu_data;
	void (*iommu_data_release)(void *iommu_data);
	char *name;
	int id;
	struct iommu_domain *default_domain;
	struct iommu_domain *domain;
	struct list_head entry;
};

int xring_smmu_debugfs_init(struct arm_smmu_device *smmu);
void xring_smmu_debugfs_exit(void);

#endif /* __XRING_SMMU_DEBUGFS_H__ */
