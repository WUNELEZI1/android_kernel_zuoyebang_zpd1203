/*
 * SPDX_license-Identifier: GPL-2.0  OR BSD-3-Clause
 * Copyright (c) 2015, Verisilicon Inc. - All Rights Reserved
 *
 ********************************************************************************
 *
 * GPL-2.0
 *
 ********************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
 * Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 ********************************************************************************
 *
 * Alternatively, This software may be distributed under the terms of
 * BSD-3-Clause, in which case the following provisions apply instead of the ones
 * mentioned above :
 *
 ********************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ********************************************************************************
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/version.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/resource.h>
#include <linux/pm_runtime.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
#include "vcx_driver.h"
#else
#include <dt-bindings/xring/platform-specific/vpu/venc/vcx_driver.h>
#endif
#include "vcx_vcmd_priv.h"
#include <soc/xring/xring_smmu_wrapper.h>
#include <linux/regulator/consumer.h>
#ifdef FPGA_POWER_ON
#include "power_on.h"
#endif

/* for dma_alloc_coherent to allocate mmu &
 * vcmd linear buffers for non-pcie env
 */
#if KERNEL_VERSION(5, 10, 0) > LINUX_VERSION_CODE
#include <linux/dma-contiguous.h>
#else
#include <linux/dma-map-ops.h>
#endif
#include <linux/mod_devicetable.h>
#include <linux/dma-buf.h>

#include "venc_power_common.h"
#include "venc_clk.h"
#include "venc_power_manager.h"
#include <linux/kstrtox.h>
#include <linux/moduleparam.h>
#include "venc_mdr.h"

#include "venc_clk_manager.h"

#define HANTROENC_SLEEP_DELAY_MS 1 /* Autosleep delay */
/****************************************************************
 * global variables declarations
 ***************************************************************/
static u32 vcmd_supported = 1;
unsigned long vcmd_isr_polling;
int vsi_kloglvl = LOGLVL_WARNING;
ulong vsi_klogMask = VENC_LOGMASK_DEFAULT;
int vsi_dumplvl = DUMP_REGISTER_LEVEL_BRIEF_ONERROR;
char *enc_dev_n = "xring_venc";
unsigned long ddr_offset;
u32 arbiter_weight = 0x1d;
u32 arbiter_urgent;
u32 arbiter_timewindow = 0x1d;
u32 arbiter_bw_overflow;
unsigned long sw_timeout_time = SW_TIMEOUT_TIME_FOR_ARBITER;

#ifdef CONFIG_ENC_PM
u32 always_on;
#ifdef CONFIG_ENC_PM_RUNTIME
u32 runtime_pm_enable;
#endif
#ifdef CONFIG_ENC_CM_RUNTIME
u32 runtime_cm_enable = 1;
#endif
#endif
/***************************************************************
 * hantroenc driver functions declarations
 ***************************************************************/
int __init hantroenc_normal_init(void **_hantroenc_data);
int __init hantroenc_vcmd_init(void **_vcmd_mgr);
void __exit hantroenc_normal_cleanup(void);
void __exit hantroenc_vcmd_cleanup(void);
#ifdef CONFIG_ENC_PM
int vcmd_pm_suspend(void *handler);
int vcmd_pm_resume(void *handler);
int enc_pm_suspend(void *handler);
int enc_pm_resume(void *handler);
#endif

/******************************************************************
 * platform device and driver related declarations
 ******************************************************************/
#define DRIVER_NAME "vsi_vcx"
struct platform_device *platformdev;

static const struct platform_device_info hantro_platform_info = {
	.name = DRIVER_NAME,
	.id = -1,
	.dma_mask = DMA_BIT_MASK(32),
};

/* struct used for matching a device */
static const struct of_device_id of_hantroenc_match[] = {
	{
		.compatible = "xring,vpu-enc", // used for matching device and driver
	},
	{ /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, of_hantroenc_match);

void vce_reset_asic(void *_vcmd_mgr)
{
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)_vcmd_mgr;
	volatile void *hwregs;

	hwregs = vcmd_mgr->dev_ctx->subsys_info->hwregs[SUB_MOD_MAIN];
	/*vc9000e
	bit 17 - bit 18: sram sd mode
	bit 19: sram power always on
	bit 26: asic inter h264 clock gating control enable
	bit 27: asic inter h265 clock gating control enable
	bit 28: asic inter clock gating control enable
	bit 29: asic encoder h264 clock gating control enable
	bit 30: asic encoder h265 clock gating control enable
	bit 31: asic encoder clock gating control enable*/
	iowrite32(0xFC060000, (void __iomem *)(hwregs + 0xC));
}

static int hantroenc_probe(struct platform_device *pdev)
{
	struct hantroenc_dev *enc_dev;
	void *handler = NULL;
	int ret = 0;

	venc_init_klog(LOGLVL_INFO, "called\n");

	enc_dev = devm_kzalloc(&pdev->dev, sizeof(*enc_dev), GFP_KERNEL);
	if (!enc_dev)
		return -ENOMEM;

	enc_dev->dev = &pdev->dev;

	enc_dev->pm = hantroenc_power_mgr_init(&pdev->dev);
	if (IS_ERR_OR_NULL(enc_dev->pm)) {
		venc_init_klog(LOGLVL_ERROR, "Failed to init power manager: %p!\n", enc_dev->pm);
		return -EFAULT;
	}

	enc_dev->cm = venc_clk_mgr_init(enc_dev->pm->venc_clk->clk);
	if (IS_ERR_OR_NULL(enc_dev->cm)) {
		venc_init_klog(LOGLVL_ERROR, "Failed to init clock manager: %p!\n", enc_dev->cm);
		goto enc_clk_mgr_init_fail;
	}

	ret = hantroenc_power_on(enc_dev->pm);
	if (ret) {
		venc_init_klog(LOGLVL_ERROR, "hantroenc_power_on failed:%d\n", ret);
		goto enc_power_on_fail;
	}

	ret = mdr_venc_exception_register(&pdev->dev);
	if (ret) {
		venc_init_klog(LOGLVL_ERROR, "Failed to register mdr exception of venc, %d\n", ret);
	}
	venc_init_klog(LOGLVL_INFO, "set enc platformdev.\n");
	platform_set_drvdata(pdev, enc_dev);
	platformdev = pdev;

	ret = venc_pdev_init(pdev, &enc_dev->priv_data);
	if (ret < 0)
		goto vcmd_init_fail;

#ifdef CONFIG_ENC_PM
#ifdef CONFIG_ENC_PM_RUNTIME
	pm_runtime_enable(&pdev->dev);
	pm_runtime_set_autosuspend_delay(&pdev->dev, HANTROENC_SLEEP_DELAY_MS);
	pm_runtime_use_autosuspend(&pdev->dev);
#endif
	if (!always_on)
		hantroenc_power_off(enc_dev->pm);
#endif

	return 0;

vcmd_init_fail:
	hantroenc_power_off(enc_dev->pm);
enc_power_on_fail:
	venc_clk_mgr_deinit(enc_dev->cm);
	enc_dev->cm = NULL;
enc_clk_mgr_init_fail:
	hantroenc_power_mgr_deinit(enc_dev->pm);
	enc_dev->pm = NULL;
	venc_klog(LOGLVL_INFO, "hantroenc: module insert fail\n");
	return ret;

}

#ifdef CONFIG_ENC_PM
static int hantroenc_pm_suspend(struct device *dev)
{
	struct hantroenc_dev *enc_dev;
	int ret = 0;

	enc_dev = dev_get_drvdata(dev);
	mutex_lock(&enc_dev->pm->pm_lock);
	if (vcmd_supported == 1)
		ret = vcmd_pm_suspend(enc_dev->priv_data);
	else
		ret = enc_pm_suspend(enc_dev->priv_data);
	venc_force_power_off(enc_dev->pm, POWER_VENC_SUBSYS);
	venc_force_power_off(enc_dev->pm, POWER_MEDIA2_SUBSYS);

	mutex_unlock(&enc_dev->pm->pm_lock);
	venc_pm_klog(LOGLVL_WARNING, "device suspend done!\n");
	return ret;
}

static int hantroenc_pm_resume(struct device *dev)
{
	struct hantroenc_dev *enc_dev;
	int ret = 0;

	enc_dev = dev_get_drvdata(dev);

	mutex_lock(&enc_dev->pm->pm_lock);

	ret = venc_force_power_on(enc_dev->pm, POWER_MEDIA2_SUBSYS);
	if (ret) {
		mutex_unlock(&enc_dev->pm->pm_lock);
		venc_pm_klog(LOGLVL_ERROR, "Failed to power on media2_subsys: %d\n", ret);
		return ret;
	}

	ret = venc_force_power_on(enc_dev->pm, POWER_VENC_SUBSYS);
	if (ret) {
		venc_pm_klog(LOGLVL_ERROR, "Failed to power on venc_subsys: %d\n", ret);
		goto venc_subsys_power_on_fail;
	}

	if (vcmd_supported == 1)
		ret = vcmd_pm_resume(enc_dev->priv_data);
	else
		ret = enc_pm_resume(enc_dev->priv_data);

	mutex_unlock(&enc_dev->pm->pm_lock);
	venc_pm_klog(LOGLVL_WARNING, "device resume done!\n");
	return ret;

venc_subsys_power_on_fail:
	venc_power_off(enc_dev->pm, POWER_MEDIA2_SUBSYS);
	mutex_unlock(&enc_dev->pm->pm_lock);
	return ret;
}

#ifdef CONFIG_ENC_PM_RUNTIME
static int hantroenc_pm_runtime_suspend(struct device *dev)
{
	/* Add Clk control */
	/* if vcmd mode, set the dev->state as power off before suspend end */
	struct hantroenc_dev *enc_dev;

	enc_dev = dev_get_drvdata(dev);
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)(enc_dev->priv_data);;
	venc_power_off(enc_dev->pm, POWER_VENC_SUBSYS);

	return 0;
}

static int hantroenc_pm_runtime_resume(struct device *dev)
{
	/* Add Clk control */
	/* if vcmd mode, set the dev->state as power on in resume start*/
	struct hantroenc_dev *enc_dev;
	int ret = 0;
	enc_dev = dev_get_drvdata(dev);
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)(enc_dev->priv_data);

	ret = venc_power_on(enc_dev->pm, POWER_VENC_SUBSYS);
	if (ret) {
		venc_pm_klog(LOGLVL_ERROR, "Failed to power on venc_subsys: %d\n", ret);
		goto venc_subsys_power_on_fail;
	}
	return 0;

venc_subsys_power_on_fail:
	venc_power_off(enc_dev->pm, POWER_MEDIA2_SUBSYS);
	return ret;
}

void hantroenc_pm_runtime_get(struct device *dev)
{
	pm_runtime_get_sync(dev);
}

void hantroenc_pm_runtime_put(struct device *dev)
{
	pm_runtime_put_sync(dev);
}

void hantroenc_pm_runtime_put_sync_autosuspend(struct device *dev)
{
	pm_runtime_put_sync_autosuspend(dev);
}
#endif

static const struct dev_pm_ops hantroenc_pm_ops = {
#ifdef CONFIG_ENC_PM_RUNTIME
	SET_RUNTIME_PM_OPS(hantroenc_pm_runtime_suspend, hantroenc_pm_runtime_resume, NULL)
#endif
	SET_SYSTEM_SLEEP_PM_OPS(hantroenc_pm_suspend, hantroenc_pm_resume)
};
#endif

static int hantroenc_remove(struct platform_device *pdev)
{
	int ret = 0;

	ret = venc_pdev_deinit(pdev);
	if (ret) {
		venc_klog(LOGLVL_ERROR, "Failed to deinit venc, %d\n", ret);
	}

	if (mdr_venc_exception_unregister(&pdev->dev)) {
		venc_klog(LOGLVL_ERROR, "Failed to unregister mdr exception of venc\n");
	}
	return ret;
}

static struct platform_driver hantroenc_driver = {
	.probe = hantroenc_probe,
	.remove = hantroenc_remove,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(of_hantroenc_match),
#ifdef CONFIG_ENC_PM
		.pm = &hantroenc_pm_ops,
#endif
	},
};

static int __init hantroenc_init(void)
{
	platform_driver_register(&hantroenc_driver);
	return 0;
}

static void __exit hantroenc_cleanup(void)
{
#ifdef CONFIG_ENC_PM
#ifdef CONFIG_ENC_PM_RUNTIME
	pm_runtime_disable(&platformdev->dev);
#endif
#endif
	platform_driver_unregister(&hantroenc_driver);
}

/**
 * @brief reset for hantro enc
 */
int hantroenc_reset(struct device *dev, bool vcmd_enc)
{
	int ret = 0;
	unsigned long flags;
	struct hantroenc_dev *enc_dev = dev_get_drvdata(dev);
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)(enc_dev->priv_data);
	struct hantrovcmd_dev *vdev = &vcmd_mgr->dev_ctx[0];
	venc_power_mgr *pm = enc_dev->pm;

	WARN_ON(!enc_dev);
	venc_klog(LOGLVL_WARNING, "venc reset start\n");

	if (atomic_read(&pm->power_count[POWER_VENC_SUBSYS]) &&
		regulator_is_enabled(pm->venc_rg)) {
		/* power off venc & media subsys */
		venc_force_power_off(pm, POWER_VENC_SUBSYS);
		venc_force_power_off(pm, POWER_MEDIA2_SUBSYS);

		/* power on venc & media subsys */
		ret = venc_force_power_on(pm, POWER_MEDIA2_SUBSYS);
		if (ret) {
			venc_klog(LOGLVL_ERROR, "Failed to power on media2_subsys: %d\n", ret);
			goto err;
		}

		ret = venc_force_power_on(pm, POWER_VENC_SUBSYS);
		if (ret != 0) {
			venc_klog(LOGLVL_ERROR, "Failed to power on venc_subsys: %d\n", ret);
			goto err;
		}

		if (vcmd_enc == 1)
			ret = vcmd_pm_resume(enc_dev->priv_data);
		else
			ret = enc_pm_resume(enc_dev->priv_data);
	} else {
		venc_klog(LOGLVL_ERROR, "venc has been powered off, no need to reset\n");
	}

	venc_klog(LOGLVL_WARNING, "venc reset finish, m2 power count: %d, "
			"venc power count: %d, dev state %d\n",
			atomic_read(&pm->power_count[POWER_MEDIA2_SUBSYS]),
			atomic_read(&pm->power_count[POWER_VENC_SUBSYS]),
			vdev->state);
	return ret;
err:
	venc_klog(LOGLVL_ERROR, "venc reset error\n");
	mdr_system_error(MODID_VPU_ENC_HANG_EXCEPTION, 0, 0);
	return ret;
}

module_init(hantroenc_init);
module_exit(hantroenc_cleanup);
/**
 * module param:
 *  vcmd_supported      - 0: vcmd driver;
 *                        1: normal driver
 *  vcmd_isr_polling    - the mode to wait for device being aborted
 *                        0: use IRQ mode
 *                        1: use polling ISR mode
 *  vsi_kloglvl         - kernel driver log level, default is LOGLVL_ERROR
 *  vsi_klogMask        - kernel driver log mask, default is VENC_LOGMASK_DEFAULT
 *  vsi_dumplvl         - dump register log level, default is DUMP_REGISTER_LEVEL_BRIEF_ONERROR
 *  enc_dev_n           - specify the device name
 *  ddr_offset          - the memory offset in ddr space
 *  arbiter_urgent      - 0: normal priority
 *                        1: urgent priority
 *  arbiter_weight      - normal priority, it indicates required bandwidth
 *                        urgent priority, it indicates urgent level
 *  arbiter_timewindow  - time window, 2^n cycles
 *  arbiter_bw_overflow - 0: can't overflow
 *                        1: can overflow
 *  sw_timeout_time     - VF timeout time after PF reset vcmd
 */
module_param(vcmd_supported, uint, 0);
module_param(vcmd_isr_polling, ulong, 0);
module_param(vsi_kloglvl, int, 0644);
module_param(vsi_klogMask, ulong, 0644);
module_param(vsi_dumplvl, int, 0644);
module_param(enc_dev_n, charp, 0644);
module_param(ddr_offset, ulong, 0);
module_param(arbiter_urgent, uint, 0);
module_param(arbiter_weight, uint, 0);
module_param(arbiter_timewindow, uint, 0);
module_param(arbiter_bw_overflow, uint, 0);
module_param(sw_timeout_time, ulong, 0);

#ifdef CONFIG_ENC_PM
#ifdef CONFIG_ENC_PM_DEBUG
static int always_on_set(const char *val, const struct kernel_param *kp)
{
	int ret;
	struct hantroenc_dev *enc_dev = dev_get_drvdata(&platformdev->dev);

	ret = kstrtouint(val, 10, &always_on);
	if (ret)
		return ret;

	mutex_lock(&enc_dev->pm->pm_lock);
	if (enc_dev->pm->always_on_curr_state == 0 && always_on == 1) {
		ret = venc_power_on(enc_dev->pm, POWER_MEDIA2_SUBSYS);
		if (ret) {
			mutex_unlock(&enc_dev->pm->pm_lock);
			venc_pm_klog(LOGLVL_ERROR, "Failed to power on media2_subsys: %d\n", ret);
			return ret;
		}

		ret = venc_power_on(enc_dev->pm, POWER_VENC_SUBSYS);
		if (ret) {
			venc_pm_klog(LOGLVL_ERROR, "Failed to power on venc_subsys: %d\n", ret);
			goto venc_subsys_power_on_fail;
		}

		enc_dev->pm->always_on_curr_state = 1;
	} else if (enc_dev->pm->always_on_curr_state == 1 && always_on == 0) {
		venc_power_off(enc_dev->pm, POWER_VENC_SUBSYS);
		venc_power_off(enc_dev->pm, POWER_MEDIA2_SUBSYS);
		enc_dev->pm->always_on_curr_state = 0;
	}

	ret = param_set_uint(val, kp);
	if (ret)
		goto param_set_uint_fail;

	mutex_unlock(&enc_dev->pm->pm_lock);
	return ret;

param_set_uint_fail:
	venc_power_off(enc_dev->pm, POWER_VENC_SUBSYS);
venc_subsys_power_on_fail:
	venc_power_off(enc_dev->pm, POWER_MEDIA2_SUBSYS);
	mutex_unlock(&enc_dev->pm->pm_lock);
	return ret;
}

static const struct kernel_param_ops always_on_ops = {
	.get = param_get_uint,
	.set = always_on_set,
};
module_param_cb(always_on, &always_on_ops, &always_on, 0644);
#endif

#ifdef CONFIG_ENC_PM_RUNTIME
module_param(runtime_pm_enable, uint, 0644);
#endif

#ifdef CONFIG_ENC_CM_RUNTIME
module_param(runtime_cm_enable, uint, 0644);
#endif
#endif
/* module description */
/*MODULE_LICENSE("Proprietary");*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Verisilicon");
MODULE_DESCRIPTION("VCX driver");
