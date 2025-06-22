// SPDX-License-Identifier: GPL-2.0-or-later
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

#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/cacheflush.h>
#include "xrp_kernel_defs.h"
#include "xrp_hw.h"
#include <linux/regulator/consumer.h>
#include <linux/kernel.h>
#include "xrp_internal.h"
#include <linux/firmware.h>
#include <linux/dma-buf.h>
#include "soc/xring/xring_smmu_wrapper.h"
#include <linux/dma-buf.h>
#include <linux/dma-mapping.h>
#include <linux/dma-heap.h>


MODULE_IMPORT_NS(DMA_BUF);
#define DRIVER_NAME "xrp-boot-simple"
#define VDSP_IOCTL_BOOT _IOWR('B', 1, struct vdsp_boot_mem)
#define VDSP_IOCTL_SHUTDOWN  _IOWR('B', 2, struct vdsp_boot_mem)

#if IS_ENABLED(CONFIG_XRING_DEBUG)
int log_level = LOG_LEVEL_INFO;
module_param(log_level, int, 0644);
MODULE_PARM_DESC(log_level, "Control log level (0=none, 1=info, ...)");
#endif

static struct sys_nocache_heap_data sram_data = {
									VDSP_SRAM_ADDR,
									VDSP_SRAM_SIZE,
									SRAM_HEAP
									};

static struct sys_nocache_heap_data srom_data = {
									VDSP_SROM_ADDR,
									VDSP_SROM_SIZE,
									SROM_HEAP
									};

struct vdsp_boot_mem {
	struct vdsp_memmap_info  vdsp_sram;
	struct vdsp_memmap_info  vdsp_srom0;
};

enum image_type {
	DDR_SYSRAM,
	DDR_SYSROM,
	NON_DDR,
};

struct firmware_boot_info {
	const char *fw_name;
	enum image_type type;
	void *map_ram;
	void *map_rom;
};

struct firmware_boot_info fw_infos[] = {
	{"sram.bin", DDR_SYSRAM, NULL, NULL},
	{"srom0.bin", DDR_SYSROM, NULL, NULL},
	{"dram0.bin", NON_DDR, NULL, NULL},
};

struct vdsp_mem_info {
	// struct vdsp_memmap_info  map_info;
	struct device *smmu_dev;
	int size;
	void *dmabuf;
	void *kernel_vaddr;
	dma_addr_t dma_handle;
};

struct vdsp_hw_simple {
	struct device *dev;
	struct platform_device *pdev;
	struct xvp *xrp;
	phys_addr_t regs_phys;
	void __iomem *regs;
	struct iommu_domain *domain;
	struct regulator *vdsp_regulator;
	struct regulator *npu_subsys_regulator;
	struct vdsp_mem_info sram_info;
	struct vdsp_mem_info srom0_info;
	struct xrp_task_device *xrp_task_dev;
	struct xboot_dev *xbdev;
};

static int vdsp_halt_on_process(void *hw_arg)
{
	int stat = 0;

	stat = hpc_smc_xrp0_halt();
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to hpc_smc_xrp0_halt\n");
		return stat;
	}
	return stat;
}

static void xrp_vdsp_halt(void *hw_arg)
{
	vdsp_halt_on_process(hw_arg);
}


static int vdsp_release_on_process(void *hw_arg)
{
	int stat = 0;

	stat = hpc_smc_xrp0_release();
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to hpc_smc_xrp0_release\n");
		return stat;
	}
	return stat;
}

static void xrp_vdsp_release(void *hw_arg)
{
	vdsp_release_on_process(hw_arg);
}

static int vdsp_reset_on_process(void *hw_arg)
{
	int stat = 0;

	stat = hpc_smc_xrp0_reset();
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to hpc_smc_xrp0_reset\n");
		return stat;
	}
	return stat;
}

static void xrp_vdsp_reset(void *hw_arg)
{
	vdsp_reset_on_process(hw_arg);
}


static int vdsp_disreset_on_process(void *hw_arg)
{
	int stat = 0;

	stat = hpc_smc_xrp0_disreset();
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to hpc_smc_xrp0_disreset\n");
		return stat;
	}
	return stat;
}

static void xrp_vdsp_disreset(void *hw_arg)
{
	vdsp_disreset_on_process(hw_arg);
}



static int xrp_vdsp_enable(void *hw_arg)
{
	int stat = 0;
	struct vdsp_hw_simple *hw = (struct vdsp_hw_simple *)hw_arg;
	struct device *parent_dev = hw->dev->parent;
	struct xboot_dev *xbdev = hw->xbdev;

	if (atomic_inc_return(&xbdev->power) > 1) {
		xrp_print(LOG_LEVEL_ERROR, "error: vdsp repeated boot\n");
		return -EBUSY;
	}

#ifdef XRP_BOOT_FROM_ATF
	stat = hpc_smc_xrp0_pwon();
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to hpc_smc_xrp0_pwon\n");
		return stat;
	}
#else
	stat = regulator_enable(hw->npu_subsys_regulator);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to npu_subsys_regulator enable\n");
		return stat;
	}
	stat = regulator_enable(hw->vdsp_regulator);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to vdsp_regulator enable\n");
		return stat;
	}
#endif
	stat = xring_smmu_tcu_ctrl(parent_dev, XRING_SMMU_TCU_POWER_ON);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to power on the TCU\n");
		return stat;
	}
	stat = xring_smmu_tbu_ctrl(parent_dev, "npu_smmu_tbu8", XRING_SMMU_TBU_POWER_ON);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to power on the TBU 'npu_smmu_tbu8'\n");
		return stat;
	}
	stat = xring_smmu_tbu_ctrl(parent_dev, "npu_smmu_tbu9", XRING_SMMU_TBU_POWER_ON);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to power on the TBU 'npu_smmu_tbu9'\n");
		return stat;
	}

	set_vdsp_pw_true();

	return stat;
}


static int vdsp_disable_on_process(void *hw_arg)
{
	int stat = 0;
	struct vdsp_hw_simple *hw = (struct vdsp_hw_simple *)hw_arg;
	struct device *parent_dev = hw->dev->parent;
	struct xboot_dev *xbdev = hw->xbdev;

	if (atomic_read(&xbdev->power) == 0) {
		xrp_print(LOG_LEVEL_ERROR, "%s power_atomic is 0\n", __func__);
		return -EBUSY;
	}

	if (!atomic_dec_and_test(&xbdev->power)) {
		xrp_print(LOG_LEVEL_ERROR, "%s power_atomic dec_and_test is false\n", __func__);
		return -EBUSY;
	}

	stat = xring_smmu_tbu_ctrl(parent_dev, "npu_smmu_tbu8", XRING_SMMU_TBU_POWER_OFF);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to power off the TBU 'npu_smmu_tbu8'\n");
		return stat;
	}
	stat = xring_smmu_tbu_ctrl(parent_dev, "npu_smmu_tbu9", XRING_SMMU_TBU_POWER_OFF);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to power off the TBU 'npu_smmu_tbu9'\n");
		return stat;
	}
	stat = xring_smmu_tcu_ctrl(parent_dev, XRING_SMMU_TCU_POWER_OFF);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to power off the TCU\n");
		return stat;
	}

#ifdef XRP_BOOT_FROM_ATF
	stat = hpc_smc_xrp0_pwoff();
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to hpc_smc_xrp0_pwoff\n");
		return stat;
	}
#else
	stat = regulator_disable(hw->vdsp_regulator);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to vdsp_regulator disable\n");
		return stat;
	}

	stat = regulator_disable(hw->npu_subsys_regulator);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to npu_subsys_regulator disable\n");
		return stat;
	}
#endif

	set_vdsp_pw_false();

	return stat;
}




static void xrp_vdsp_disable(void *hw_arg)
{
	vdsp_disable_on_process(hw_arg);
}



static int load_image(const struct firmware_boot_info *fw_info)
{
	const struct firmware *fw;
	void *fw_mapped_addr;
	struct vdsp_mem_info *map_tmp;
	int ret = 0;

	ret = request_firmware(&fw, fw_info->fw_name, NULL);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to request firmware %s\n", fw_info->fw_name);
		return -ENOENT;
	}

	switch (fw_info->type) {
	case DDR_SYSRAM:
		map_tmp = (struct vdsp_mem_info *)fw_info->map_ram;
		fw_mapped_addr = map_tmp->kernel_vaddr;
		xrp_print(LOG_LEVEL_DEBUG, "%s loaded addr is %pK\n", fw_info->fw_name, fw_mapped_addr);
		memcpy(fw_mapped_addr, fw->data, fw->size);
		break;
	case DDR_SYSROM:
		map_tmp = (struct vdsp_mem_info *)fw_info->map_rom;
		fw_mapped_addr = map_tmp->kernel_vaddr;
		xrp_print(LOG_LEVEL_DEBUG, "%s loaded addr is %pK\n", fw_info->fw_name, fw_mapped_addr);
		memcpy(fw_mapped_addr, fw->data, fw->size);
		break;
	case NON_DDR:
		{
			phys_addr_t fw_phys_addr = VDSP_BOOT_ADDR;

			fw_mapped_addr = ioremap(fw_phys_addr, fw->size);
			if (!fw_mapped_addr) {
				xrp_print(LOG_LEVEL_ERROR, "Failed to remap firmware to kernel space\n");
				release_firmware(fw);
				return -ENOENT;
			}
			memcpy_toio(fw_mapped_addr, fw->data, fw->size);
			iounmap(fw_mapped_addr);
			break;
		}
	default:
		xrp_print(LOG_LEVEL_ERROR, "Unknown firmware type\n");
		return -EINVAL;
	}
	release_firmware(fw);
	xrp_print(LOG_LEVEL_DEBUG, "Firmware loaded successfully\n");
	return ret;
}

static int load_multiple_images(void *map_ram, void *map_rom)
{
	int i;
	int ret = 0;
	int num_fw = sizeof(fw_infos) / sizeof(struct firmware_boot_info);

	for (i = 0; i < num_fw; i++) {
		fw_infos[i].map_ram = map_ram;
		fw_infos[i].map_rom = map_rom;
		xrp_print(LOG_LEVEL_DEBUG, "Loading firmware %s...\n", fw_infos[i].fw_name);
		ret = load_image(&fw_infos[i]);
		if (ret != 0) {
			xrp_print(LOG_LEVEL_ERROR, "Failed to load firmware %s, error: %d\n", fw_infos[i].fw_name, ret);
			return -ENOENT;
		}
	}
	return ret;
}



static long xrp_vdsp_free_fwmap(void *hw_arg,
				const void *image,
				Elf32_Phdr *phdr)
{
	struct vdsp_hw_simple *hw = (struct vdsp_hw_simple *)hw_arg;
	struct xrp_task_device *xrp_task_dev = hw->xrp_task_dev;

	sys_nocache_unmap(xrp_task_dev, &sram_data);
	sys_nocache_unmap(xrp_task_dev, &srom_data);
	return 0;
}

static long xrp_vdsp_loadfw(void *hw_arg,
				const void *image,
				Elf32_Phdr *phdr)
{
	int ret = 0;
	struct vdsp_hw_simple *hw = (struct vdsp_hw_simple *)hw_arg;
	struct device *parent_dev = hw->dev->parent;
	struct vdsp_mem_info *sram_info = &hw->sram_info;
	struct vdsp_mem_info *srom0_info = &hw->srom0_info;
	struct sys_nocache_dma_buf *xrp_sram_sys_dma_buf = NULL;
	struct sys_nocache_dma_buf *xrp_srom_sys_dma_buf = NULL;

	struct xrp_task_device *xrp_task_dev = hw->xrp_task_dev;

	sram_info->smmu_dev = parent_dev;
	sram_info->size = VDSP_SRAM_SIZE;
	xrp_print(LOG_LEVEL_DEBUG, "begin malloc and map sram memory, addr is %pK, size is %08x\n", (void *)VDSP_SRAM_ADDR, sram_info->size);
	xrp_sram_sys_dma_buf = sys_nocache_map(xrp_task_dev, &sram_data);
	sram_info->kernel_vaddr = xrp_sram_sys_dma_buf->buf;
	if (!sram_info->kernel_vaddr) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to allocate and map sram memory\n");
		return ret;
	}
	xrp_print(LOG_LEVEL_DEBUG, "sram_info->kernel_vaddr= %p\n", sram_info->kernel_vaddr);

	srom0_info->smmu_dev = parent_dev;
	srom0_info->size = VDSP_SROM_SIZE;
	xrp_print(LOG_LEVEL_DEBUG, "begin malloc and map srom0 memory, addr is %pK, size is %08x\n", (void *)VDSP_SROM_ADDR, srom0_info->size);
	xrp_srom_sys_dma_buf = sys_nocache_map(xrp_task_dev, &srom_data);
	srom0_info->kernel_vaddr = xrp_srom_sys_dma_buf->buf;
	if (!srom0_info->kernel_vaddr) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to allocate and map srom memory\n");
		return ret;
	}
	xrp_print(LOG_LEVEL_DEBUG, "srom0_info->kernel_vaddr= %p\n", srom0_info->kernel_vaddr);

	ret = load_multiple_images((void *)sram_info, (void *)srom0_info);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "load_multiple_images failed: %d\n", ret);
		return ret;
	}
	return ret;
}




static int xrp_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	struct xboot_dev *xbdev = container_of(filp->private_data, struct xboot_dev, miscdev);

	filp->private_data = xbdev;
	return ret;
}

static int xrp_release(struct inode *inode, struct file *filp)
{
	return 0;
}


static long vdsp_topreset(struct device *dev, void *regul_arg)
{
	int stat = 0;
	struct regulator *regulator = (struct regulator *)regul_arg;
	struct device *parent_dev = dev->parent;
	struct regulator *regulator1 = devm_regulator_get(dev, "npu_subsys");

#ifdef XRP_BOOT_FROM_ATF
	stat = hpc_smc_xrp0_pwon();
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to hpc_smc_xrp0_pwon\n");
		return stat;
	}
#else
	stat = regulator_enable(regulator1);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to regulator_enable\n");
		return stat;
	}
	stat = regulator_enable(regulator);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to regulator_enable\n");
		return stat;
	}
#endif
	stat = hpc_smc_xrp0_halt();
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to hpc_smc_xrp0_halt\n");
		return stat;
	}

	stat = xring_smmu_tcu_ctrl(parent_dev, XRING_SMMU_TCU_POWER_ON);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to power on the TCU\n");
		return stat;
	}
	stat = xring_smmu_tbu_ctrl(parent_dev, "npu_smmu_tbu8", XRING_SMMU_TBU_POWER_ON);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to power on the TBU 'npu_smmu_tbu8'\n");
		return stat;
	}
	stat = xring_smmu_tbu_ctrl(parent_dev, "npu_smmu_tbu9", XRING_SMMU_TBU_POWER_ON);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to power on the TBU 'npu_smmu_tbu9'\n");
		return stat;
	}

	stat = hpc_smc_xrp0_disreset();
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to hpc_smc_xrp0_disreset\n");
		return stat;
	}
	return stat;
}

static int vdsp_release(void)
{
	int stat = 0;

	stat = hpc_smc_xrp0_release();
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to hpc_smc_xrp0_release\n");
		return stat;
	}
	return stat;
}

static int vdsp_disable(struct device *dev, void *regul_arg)
{
	int stat = 0;
	struct regulator *regulator = (struct regulator *)regul_arg;
	struct device *parent_dev = dev->parent;

	stat = hpc_smc_xrp0_reset();
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to hpc_smc_xrp0_reset\n");
		//return stat;
	}

	stat = xring_smmu_tbu_ctrl(parent_dev, "npu_smmu_tbu8", XRING_SMMU_TBU_POWER_OFF);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to power off the TBU 'npu_smmu_tbu8'\n");
		return stat;
	}
	stat = xring_smmu_tbu_ctrl(parent_dev, "npu_smmu_tbu9", XRING_SMMU_TBU_POWER_OFF);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to power off the TBU 'npu_smmu_tbu9'\n");
		return stat;
	}
	stat = xring_smmu_tcu_ctrl(parent_dev, XRING_SMMU_TCU_POWER_OFF);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to power off the TCU\n");
		return stat;
	}

#ifdef XRP_BOOT_FROM_ATF
	stat = hpc_smc_xrp0_pwoff();
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to hpc_smc_xrp0_pwoff\n");
		return stat;
	}
#else
	stat = regulator_disable(regulator);
	if (stat != 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to regulator_disable\n");
		return stat;
	}
#endif
	return stat;
}



void print_boot_kernel_mem_info(struct boot_kernel_mem_map_info *info)
{
	// return;
	xrp_print(LOG_LEVEL_DEBUG, "fd: %d\n", info->map_info.da);
	xrp_print(LOG_LEVEL_DEBUG, "size: %d\n", info->map_info.size);
	xrp_print(LOG_LEVEL_DEBUG, "type: %d\n", info->map_info.type);
	xrp_print(LOG_LEVEL_DEBUG, "vdsp_image.flag: %d\n", info->map_info.flag);
	xrp_print(LOG_LEVEL_DEBUG, "vdsp_image.prot: %d\n", info->map_info.prot);
	xrp_print(LOG_LEVEL_DEBUG, "vdsp_image.da: %u\n\n", info->map_info.da);
	xrp_print(LOG_LEVEL_DEBUG, "vdsp_image.dmabuf: %pK\n\n", info->dmabuf);
	xrp_print(LOG_LEVEL_DEBUG, "vdsp_image.kernel_vaddr: %pK\n\n", info->kernel_vaddr);
}

static int vdsp_power_on(struct boot_kernel_mem_map_info *sram_info,
	struct boot_kernel_mem_map_info *srom0_info, struct xboot_dev *xbdev)
{
	int ret = 0;

	if (atomic_inc_return(&xbdev->power) > 1) {
		xrp_print(LOG_LEVEL_ERROR, "error: vdsp repeated boot\n");
		return -EBUSY;
	}

	ret = vdsp_task_memmap(&xbdev->pdev->dev, sram_info);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "vdsp_task_memmap sram failed: %d\n", ret);
		return ret;
	}
	print_boot_kernel_mem_info(sram_info);


	ret = vdsp_task_memmap(&xbdev->pdev->dev, srom0_info);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "vdsp_task_memmap srom0 failed: %d\n", ret);
		return ret;
	}
	print_boot_kernel_mem_info(srom0_info);

	ret = vdsp_topreset(&xbdev->pdev->dev, xbdev->vdsp_regulator);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "vdsp_topreset failed: %d\n", ret);
		return ret;
	}

	ret = load_multiple_images((void *)sram_info, (void *)srom0_info);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "load_multiple_images failed: %d\n", ret);
		return ret;
	}

	ret = vdsp_release();
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "vdsp_release failed: %d\n", ret);
		return ret;
	}

	return ret;
}

static int vdsp_power_off(struct boot_kernel_mem_map_info *sram_info,
	struct boot_kernel_mem_map_info *srom0_info, struct xboot_dev *xbdev)
{
	int ret = 0;

	if (atomic_read(&xbdev->power) == 0) {
		xrp_print(LOG_LEVEL_ERROR, "%s power_atomic is 0\n", __func__);
		return -EBUSY;
	}

	if (!atomic_dec_and_test(&xbdev->power)) {
		xrp_print(LOG_LEVEL_ERROR, "%s power_atomic dec_and_test is false\n", __func__);
		return -EBUSY;
	}

	ret = vdsp_disable(&xbdev->pdev->dev, xbdev->vdsp_regulator);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "vdsp_disable failed: %d\n", ret);
		//return ret;
	}

	ret = vdsp_task_memunmap(&xbdev->pdev->dev, &sram_info->map_info);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "vdsp_task_memunmap sram failed: %d\n", ret);
		return ret;
	}

	ret = vdsp_task_memunmap(&xbdev->pdev->dev, &srom0_info->map_info);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "vdsp_task_memunmap srom0 failed: %d\n", ret);
		return ret;
	}
	return ret;
}



static long xrp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	struct vdsp_boot_mem *boot_para = (struct vdsp_boot_mem *)argp;
	struct boot_kernel_mem_map_info sram_info = {0}, srom0_info = {0};
	struct xboot_dev *xbdev = filp->private_data;

	ret = copy_from_user(&sram_info.map_info,
						&boot_para->vdsp_sram,
						sizeof(struct vdsp_memmap_info));
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "copy sram_info failed: %d\n", ret);
		return ret;
	}

	ret = copy_from_user(&srom0_info.map_info,
						&boot_para->vdsp_srom0,
						sizeof(struct vdsp_memmap_info));
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "copy srom0_info failed: %d\n", ret);
		return ret;
	}

	switch (cmd) {
	case VDSP_IOCTL_BOOT:
		ret = task_first_open_action(false);
		if (ret != 0)
			return ret;
		ret = vdsp_power_on(&sram_info, &srom0_info, xbdev);
		if (ret != 0) {
			xrp_print(LOG_LEVEL_ERROR, "vdsp_power_on failed: %d\n", ret);
			return ret;
		}
		(void)vdsp_poweron_done_action(true);
		break;
	case VDSP_IOCTL_SHUTDOWN:
		ret = task_last_close_action(false);
		if (ret != 0)
			return ret;
		ret = vdsp_power_off(&sram_info, &srom0_info, xbdev);
		if (ret != 0) {
			xrp_print(LOG_LEVEL_ERROR, "vdsp_power_off failed: %d\n", ret);
			//return ret;
		}
		(void)vdsp_poweroff_done_action(true);
		break;
	default:
		xrp_print(LOG_LEVEL_INFO, "Unsupported ioctl!!!\n");
		ret = -ENOTTY;
	}

	return ret;
}

static const struct xrp_hw_ops hw_ops = {
	.enable = xrp_vdsp_enable,
	.halt = xrp_vdsp_halt,
	.release = xrp_vdsp_release,
	.reset = xrp_vdsp_reset,
	.disreset = xrp_vdsp_disreset,
	.disable = xrp_vdsp_disable,
	.load_fw_segment = xrp_vdsp_loadfw,
	.free_fw_segment = xrp_vdsp_free_fwmap,
};

static const struct file_operations xbdev_fops = {
	.owner = THIS_MODULE,
	.open = xrp_open,
	.release = xrp_release,
	.unlocked_ioctl = xrp_ioctl,
};


static int xrp_boot_probe(struct platform_device *pdev)
{
	struct xboot_dev *xbdev;
	struct vdsp_hw_simple *hw =
		devm_kzalloc(&pdev->dev, sizeof(*hw), GFP_KERNEL);
	int ret = 0;
	struct device *parent_dev;
	struct platform_device *parent_pdev;
	struct device *child_dev = &pdev->dev;
	struct xrp_task_device *xrp_task_dev;

	xrp_print(LOG_LEVEL_INFO, "enter %s miscdev\n", __func__);

	parent_dev = child_dev->parent;

	parent_pdev = to_platform_device(parent_dev);
	xrp_task_dev = platform_get_drvdata(parent_pdev);

	hw->xrp_task_dev = xrp_task_dev;
	xbdev = devm_kzalloc(&pdev->dev, sizeof(*xbdev), GFP_KERNEL);
	if (!xbdev)
		return -ENOMEM;

	xbdev->pdev = pdev;

	xbdev->vdsp_regulator = devm_regulator_get(&pdev->dev, "xrp");
	if (!(xbdev->vdsp_regulator)) {
		xrp_print(LOG_LEVEL_ERROR, "%s get regulator failed\n", dev_name(&pdev->dev));
		return -ENODEV;
	}

	hw->vdsp_regulator = xbdev->vdsp_regulator;
	hw->npu_subsys_regulator = devm_regulator_get(&pdev->dev, "npu_subsys");
	if (!(hw->npu_subsys_regulator)) {
		xrp_print(LOG_LEVEL_ERROR, "%s get hw->npu_subsys_regulator failed\n", dev_name(&pdev->dev));
		return -ENODEV;
	}
	xbdev->npu_subsys_regulator = hw->npu_subsys_regulator;
	hw->dev = &pdev->dev;
	hw->pdev = pdev;
	hw->xbdev = xbdev;

	xbdev->miscdev.parent = &pdev->dev;
	xbdev->miscdev.minor = MISC_DYNAMIC_MINOR;
	xbdev->miscdev.name = "xvp_boot";
	xbdev->miscdev.fops = &xbdev_fops;

	set_hw_ops(&pdev->dev, (const void *)&hw_ops, (void *)hw);

	platform_set_drvdata(pdev, xbdev);
	atomic_set(&xbdev->power, 0);
	xrp_print(LOG_LEVEL_INFO, "xbdev is %pK, line is %d\n", xbdev, __LINE__);

	xrp_task_dev->xbdev = xbdev;
	xrp_print(LOG_LEVEL_INFO, "xrp_task_dev->xbdev is %pK, line is %d\n", xrp_task_dev->xbdev, __LINE__);
	return ret;
}

static int xrp_boot_remove(struct platform_device *pdev)
{
	struct xboot_dev *xbdev;
	int ret = 0;

	xbdev  = platform_get_drvdata(pdev);

	/*
	 *
	if (regulator_is_enabled(xbdev->vdsp_regulator)) {
		ret = regulator_disable(xbdev->vdsp_regulator);
		if (ret != 0) {
			xrp_print(LOG_LEVEL_ERROR, "vdsp_regulator_disable failed: %d\n", ret);
			return ret;
		}
	}

	if (regulator_is_enabled(xbdev->npu_subsys_regulator)) {
		ret = regulator_disable(xbdev->npu_subsys_regulator);
		if (ret != 0) {
			xrp_print(LOG_LEVEL_ERROR, "npu_subsys_regulator_disable failed: %d\n", ret);
			return ret;
		}
	}
	*/

	atomic_set(&xbdev->power, 0);

	return ret;
}

static const struct of_device_id xrp_boot_table[] = {
	{ .compatible = "xring,vdsp-boot", },
	{},
	};
MODULE_DEVICE_TABLE(of, xrp_boot_table);


static struct platform_driver xrp_boot_simple_driver = {
	.probe = xrp_boot_probe,
	.remove = xrp_boot_remove,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(xrp_boot_table),
	},
};

module_platform_driver(xrp_boot_simple_driver);

MODULE_AUTHOR("High Performance Computing Group");
MODULE_DESCRIPTION("Vsdp Boot Management Driver");
MODULE_LICENSE("GPL v2");

