// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/reset-controller.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <dt-bindings/xring/platform-specific/peri_crg.h>
#include <dt-bindings/xring/platform-specific/lpis_crg.h>
#include <dt-bindings/xring/platform-specific/lms_crg.h>
#include <dt-bindings/xring/platform-specific/xr_reset_resource.h>
#include "xr_reset_internal.h"

/* PERI Reset */
#define PERI_RST_OFF(n)         PERI_CRG_RST##n##_W1S
#define PERI_RST_SFT(n, field)  PERI_CRG_RST##n##_W1S_IP_##field##_N_SHIFT
#define PERI_RST_INIT(name, secured, n, field) \
		{#name, secured, PERI_RST_OFF(n), PERI_RST_SFT(n, field)}
static const struct xr_reset_config peri_resets[] = {
	[XR_PERI_PRST_SPINLOCK]  = PERI_RST_INIT(prst_peri_spinlock, false, 0, PRST_PERI_SPINLOCK),
	[XR_PERI_PRST_TIMER_NS]  = PERI_RST_INIT(prst_peri_timer_ns, false, 0, PRST_PERI_TIMER_NS),
	[XR_PERI_RST_TIMER0]     = PERI_RST_INIT(rst_peri_timer0, false, 0, RST_PERI_TIMER0),
	[XR_PERI_RST_TIMER1]     = PERI_RST_INIT(rst_peri_timer1, false, 0, RST_PERI_TIMER1),
	[XR_PERI_RST_TIMER2]     = PERI_RST_INIT(rst_peri_timer2, false, 0, RST_PERI_TIMER2),
	[XR_PERI_RST_TIMER3]     = PERI_RST_INIT(rst_peri_timer3, false, 0, RST_PERI_TIMER3),
	[XR_PERI_PRST_IPC1]      = PERI_RST_INIT(prst_peri_ipc1, false, 0, PRST_PERI_IPC1),
	[XR_PERI_PRST_IPC2]      = PERI_RST_INIT(prst_peri_ipc2, false, 0, PRST_PERI_IPC2),
	[XR_PERI_PRST_SPI4]      = PERI_RST_INIT(prst_spi4, false, 0, PRST_SPI4),
	[XR_PERI_PRST_SPI5]      = PERI_RST_INIT(prst_spi5, false, 0, PRST_SPI5),
	[XR_PERI_PRST_SPI6]      = PERI_RST_INIT(prst_spi6, false, 0, PRST_SPI6),
	[XR_PERI_RST_SPI4]       = PERI_RST_INIT(rst_spi4, false, 0, RST_SPI4),
	[XR_PERI_RST_SPI5]       = PERI_RST_INIT(rst_spi5, false, 0, RST_SPI5),
	[XR_PERI_RST_SPI6]       = PERI_RST_INIT(rst_spi6, false, 0, RST_SPI6),

	[XR_PERI_PRST_I2C0]      = PERI_RST_INIT(prst_i2c0, false, 1, PRST_I2C0),
	[XR_PERI_PRST_I2C1]      = PERI_RST_INIT(prst_i2c1, false, 1, PRST_I2C1),
	[XR_PERI_PRST_I2C2]      = PERI_RST_INIT(prst_i2c2, false, 1, PRST_I2C2),
	[XR_PERI_PRST_I2C3]      = PERI_RST_INIT(prst_i2c3, false, 1, PRST_I2C3),
	[XR_PERI_PRST_I2C4]      = PERI_RST_INIT(prst_i2c4, false, 1, PRST_I2C4),
	[XR_PERI_PRST_I2C5]      = PERI_RST_INIT(prst_i2c5, false, 1, PRST_I2C5),
	[XR_PERI_PRST_I2C6]      = PERI_RST_INIT(prst_i2c6, false, 1, PRST_I2C6),
	[XR_PERI_PRST_I2C9]      = PERI_RST_INIT(prst_i2c9, false, 1, PRST_I2C9),
	[XR_PERI_PRST_I2C10]     = PERI_RST_INIT(prst_i2c10, false, 1, PRST_I2C10),
	[XR_PERI_PRST_I2C11]     = PERI_RST_INIT(prst_i2c11, false, 1, PRST_I2C11),
	[XR_PERI_PRST_I2C12]     = PERI_RST_INIT(prst_i2c12, false, 1, PRST_I2C12),
	[XR_PERI_PRST_I2C13]     = PERI_RST_INIT(prst_i2c13, false, 1, PRST_I2C13),
	[XR_PERI_PRST_I2C20]     = PERI_RST_INIT(prst_i2c20, false, 1, PRST_I2C20),
	[XR_PERI_RST_DMA_NS]     = PERI_RST_INIT(rst_dma_ns, false, 1, RST_DMA_NS),
	[XR_PERI_RST_DMA_S]      = PERI_RST_INIT(rst_dma_s, false, 1, RST_DMA_S),
	[XR_PERI_RST_DMA_NS_TPC] = PERI_RST_INIT(rst_dma_ns_tpc, false, 1, RST_DMA_NS_TPC),
	[XR_PERI_RST_DMA_S_TPC]  = PERI_RST_INIT(rst_dma_s_tpc, false, 1, RST_DMA_S_TPC),

	[XR_PERI_RST_I2C0]       = PERI_RST_INIT(rst_i2c0, false, 2, RST_I2C0),
	[XR_PERI_RST_I2C1]       = PERI_RST_INIT(rst_i2c1, false, 2, RST_I2C1),
	[XR_PERI_RST_I2C2]       = PERI_RST_INIT(rst_i2c2, false, 2, RST_I2C2),
	[XR_PERI_RST_I2C3]       = PERI_RST_INIT(rst_i2c3, false, 2, RST_I2C3),
	[XR_PERI_RST_I2C4]       = PERI_RST_INIT(rst_i2c4, false, 2, RST_I2C4),
	[XR_PERI_RST_I2C5]       = PERI_RST_INIT(rst_i2c5, false, 2, RST_I2C5),
	[XR_PERI_RST_I2C6]       = PERI_RST_INIT(rst_i2c6, false, 2, RST_I2C6),
	[XR_PERI_RST_I2C9]       = PERI_RST_INIT(rst_i2c9, false, 2, RST_I2C9),
	[XR_PERI_RST_I2C10]      = PERI_RST_INIT(rst_i2c10, false, 2, RST_I2C10),
	[XR_PERI_RST_I2C11]      = PERI_RST_INIT(rst_i2c11, false, 2, RST_I2C11),
	[XR_PERI_RST_I2C12]      = PERI_RST_INIT(rst_i2c12, false, 2, RST_I2C12),
	[XR_PERI_RST_I2C13]      = PERI_RST_INIT(rst_i2c13, false, 2, RST_I2C13),
	[XR_PERI_RST_I2C20]      = PERI_RST_INIT(rst_i2c20, false, 2, RST_I2C20),
	[XR_PERI_RST_I3C0]       = PERI_RST_INIT(rst_i3c0, false, 2, RST_I3C0),
	[XR_PERI_PRST_I3C0]      = PERI_RST_INIT(prst_i3c0, false, 2, PRST_I3C0),
	[XR_PERI_RST_PWM0]       = PERI_RST_INIT(rst_pwm0, false, 2, RST_PWM0),
	[XR_PERI_RST_PWM1]       = PERI_RST_INIT(rst_pwm1, false, 2, RST_PWM1),
	[XR_PERI_RST_UART3]      = PERI_RST_INIT(rst_uart3, false, 2, RST_UART3),
	[XR_PERI_RST_UART6]      = PERI_RST_INIT(rst_uart6, false, 2, RST_UART6),

	[XR_PERI_RST_ONEWIRE0]     = PERI_RST_INIT(rst_onewire0, true, 4, RST_ONEWIRE0),
	[XR_PERI_RST_ONEWIRE1]     = PERI_RST_INIT(rst_onewire1, true, 4, RST_ONEWIRE1),
	[XR_PERI_RST_ONEWIRE2]     = PERI_RST_INIT(rst_onewire2, true, 4, RST_ONEWIRE2),
	[XR_PERI_PRST_ONEWIRE0]    = PERI_RST_INIT(prst_onewire0, true, 4, PRST_ONEWIRE0),
	[XR_PERI_PRST_ONEWIRE1]    = PERI_RST_INIT(prst_onewire1, true, 4, PRST_ONEWIRE1),
	[XR_PERI_PRST_ONEWIRE2]    = PERI_RST_INIT(prst_onewire2, true, 4, PRST_ONEWIRE2),
};

static struct xr_reset_desc peri_reset_desc = {
	.resets = peri_resets,
	.num_resets = ARRAY_SIZE(peri_resets),
};

/* LPIS Reset */
#define LPIS_RST_OFF(n)         LPIS_CRG_RST##n##_W1S
#define LPIS_RST_SFT(n, field)  LPIS_CRG_RST##n##_W1S_IP_##field##_N_SHIFT
#define LPIS_RST_INIT(name, secured, n, field) \
		{#name, secured, LPIS_RST_OFF(n), LPIS_RST_SFT(n, field)}
static const struct xr_reset_config lpis_resets[] = {
	[XR_LPIS_PRST_IPC_NS]   = LPIS_RST_INIT(prst_ipc_ns, false, 2, PRST_IPC_NS),
	[XR_LPIS_PRST_SPI_DMAC] = LPIS_RST_INIT(prst_lpis_spi_dmac, true, 0, PRST_LPIS_SPI_DMAC),
	[XR_LPIS_RST_SPI_DMAC]  = LPIS_RST_INIT(rst_spi_dmac, true, 0, RST_SPI_DMAC),
};

static struct xr_reset_desc lpis_reset_desc = {
	.resets = lpis_resets,
	.num_resets = ARRAY_SIZE(lpis_resets),
};

/* LMS Reset */
#define LMS_RST_OFF(n)         LMS_CRG_RST##n##_W1S
#define LMS_RST_SFT(n, field)  LMS_CRG_RST##n##_W1S_IP_##field##_N_SHIFT
#define LMS_RST_INIT(name, secured, n, field) \
		{#name, secured, LMS_RST_OFF(n), LMS_RST_SFT(n, field)}
static const struct xr_reset_config lms_resets[] = {
	[XR_LMS_RST_RTC1]     = LMS_RST_INIT(rst_lms_rtc1, false, 0, RST_LMS_RTC1),
	[XR_LMS_PRST_RTC1]    = LMS_RST_INIT(prst_lms_rtc1, false, 0, PRST_LMS_RTC1),
	[XR_LMS_PRST_TIMER2]  = LMS_RST_INIT(prst_lms_timer2, false, 0, PRST_LMS_TIMER2),
	[XR_LMS_RST_TIMER60]  = LMS_RST_INIT(rst_lms_timer60, false, 0, RST_LMS_TIMER60),
};

static struct xr_reset_desc lms_reset_desc = {
	.resets = lms_resets,
	.num_resets = ARRAY_SIZE(lms_resets),
};

static const struct of_device_id xring_o1_of_match_tbl[] = {
	{
		.compatible = "xring,peri_reset",
		.data = &peri_reset_desc,
	},
	{
		.compatible = "xring,lpis_reset",
		.data = &lpis_reset_desc,
	},
	{
		.compatible = "xring,lms_reset",
		.data = &lms_reset_desc,
	},
	{ /* end */ }

};

static int xring_o1_reset_probe(struct platform_device *pdev)
{
	const struct xr_reset_desc *desc;
	int ret;

	desc = of_device_get_match_data(&pdev->dev);
	if (!desc)
		return -EINVAL;

	ret = xr_reset_probe(pdev, desc);

	if (ret)
		dev_err(&pdev->dev, "probe failed\n");
	else
		dev_info(&pdev->dev, "probe success\n");

	return ret;
}

MODULE_DEVICE_TABLE(of, xring_o1_of_match_tbl);

static struct platform_driver xring_o1_reset_driver = {
	.probe = xring_o1_reset_probe,
	.driver  = {
		.name = "xring_o1_reset",
		.of_match_table = xring_o1_of_match_tbl,
	},
};

module_platform_driver(xring_o1_reset_driver);

MODULE_AUTHOR("Jinfei Weng <wengjinfei@xiaomi.com>");
MODULE_DESCRIPTION("X-Ring O1 Reset Driver");
MODULE_LICENSE("GPL v2");
