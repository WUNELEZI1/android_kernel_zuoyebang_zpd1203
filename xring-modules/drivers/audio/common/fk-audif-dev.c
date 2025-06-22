// SPDX-License-Identifier: GPL-2.0-only
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

#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/kern_levels.h>
#include <linux/mod_devicetable.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/amba/bus.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/init.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>

#include "fk-audif-dev.h"
#include "fk-audio-simulate.h"
#include "../adsp/adsp_boot_driver.h"
#define DRV_NAME "xr-audif"
#define U32_DATA_LEN	2
#define AUD_IF_BASE_ADDR_VIR

struct xr_audif_dev g_xr_audif;

static inline unsigned int audif_reg_read16(const void __iomem *reg)
{
	return __raw_readw(reg);
}

static inline unsigned int audif_reg_read32(const void __iomem *reg)
{
	return __raw_readl(reg);
}

static inline void audif_reg_write32(void __iomem *reg, int val)
{
	__raw_writel(val, reg);
}

void audif_bit_set(void __iomem *reg, int n)
{
	uint32_t ret;

	ret = audif_reg_read32(reg);
	ret |= BIT(n);
	audif_reg_write32(reg, ret);
}

void audif_bit_clr(void __iomem *reg, int n)
{
	uint32_t ret;

	ret = audif_reg_read32(reg);
	ret &= ~(BIT(n));
	audif_reg_write32(reg, ret);
}

uint32_t audif_bit_read(void __iomem *reg, int bit)
{
	uint32_t ret;

	ret = audif_reg_read32(reg);
	ret &= (1 << bit);
	return ret;
}

void audif_reg_set(void __iomem *reg, int val, int bit)
{
	uint32_t ret;

	ret = audif_reg_read32(reg);
	ret |= (val << bit);
	audif_reg_write32(reg, ret);
}


void audif_reg_clr(void __iomem *reg, int width, int bit)
{
	uint32_t ret;

	ret = audif_reg_read32(reg);
	ret &= ~(((1 << width) - 1) << bit);
	audif_reg_write32(reg, ret);
}

void audif_reg_val_write(void __iomem *reg, uint16_t bit, uint16_t width, uint16_t val)
{
	uint32_t ret;

	ret = audif_reg_read32(reg);
	ret &= ~(((1 << width) - 1) << bit);
	ret |= (val << bit);
	audif_reg_write32(reg, ret);
}

#ifndef SIMULATE_COMMON
static int audif_all_slot_update(void)
{
	audif_reg_val_write(ALL_SLOT_SEL_UPDATE, ALL_SLOT_UPDATE, BIT_WIDTH_1, 1);

	return 0;
}
#endif

int audif_fifo_empty_watermark_ap_set(uint16_t ap_empty_watermark)
{
#ifdef SIMULATE_COMMON
	AUD_LOG_INFO(AUD_COMM, "%s: ap_empty_watermark = %d",
		__func__, ap_empty_watermark);
#else
	audif_reg_val_write(FIFO_CONTROL_AP, FIFO_EMPTY_WATERMARK_AP,
				BIT_WIDTH_10, ap_empty_watermark);
#endif
	return 0;
}
EXPORT_SYMBOL(audif_fifo_empty_watermark_ap_set);

int audif_fifo_full_watermark_ar_set(uint16_t ar_full_watermark)
{
#ifdef SIMULATE_COMMON
	AUD_LOG_INFO(AUD_COMM, "%s: ar_full_watermark = %d",
		__func__, ar_full_watermark);
#else
	audif_reg_val_write(FIFO_CONTROL_AR, FIFO_FULL_WATERMARK_AR,
				BIT_WIDTH_10, ar_full_watermark);
#endif
	return 0;
}
EXPORT_SYMBOL(audif_fifo_full_watermark_ar_set);

int audif_aempty_ap_mask_enable(bool enable)
{
#ifdef SIMULATE_COMMON
	if (enable)
		g_xr_audif.ap_mask_flag = 1;
	else
		g_xr_audif.ap_mask_flag = 0;

	AUD_LOG_INFO(AUD_COMM, "%s: ap_mask_enable = %d",
		__func__, g_xr_audif.ap_mask_flag);
#else
	uint16_t aempty_ap_mask_l;
	uint16_t aempty_ap_mask_r;

	if (enable) {
		aempty_ap_mask_l = 0x0;
		aempty_ap_mask_r = 0x0;
	} else {
		aempty_ap_mask_l = 0x1;
		aempty_ap_mask_r = 0x1;
	}

	audif_reg_val_write(AFULL_AEMPTY_INTR_MASK, AEMPTY_AP_L_MASK,
				BIT_WIDTH_1, aempty_ap_mask_l);
	audif_reg_val_write(AFULL_AEMPTY_INTR_MASK, AEMPTY_AP_R_MASK,
				BIT_WIDTH_1, aempty_ap_mask_r);
	audif_all_slot_update();
#endif
	return 0;
}
EXPORT_SYMBOL(audif_aempty_ap_mask_enable);

int audif_afull_ar_mask_enable(bool enable)
{
#ifdef SIMULATE_COMMON
	if (enable)
		g_xr_audif.ar_mask_flag = 1;
	else
		g_xr_audif.ar_mask_flag = 0;

	AUD_LOG_INFO(AUD_COMM, "%s: ar_mask_enable = %d",
		__func__, g_xr_audif.ar_mask_flag);
#else
	uint16_t afull_ar_mask_l;
	uint16_t afull_ar_mask_r;

	if (enable) {
		afull_ar_mask_l = 0x0;
		afull_ar_mask_r = 0x0;
	} else {
		afull_ar_mask_l = 0x1;
		afull_ar_mask_r = 0x1;
	}

	audif_reg_val_write(AFULL_AEMPTY_INTR_MASK, AFULL_AR_L_MASK,
				BIT_WIDTH_1, afull_ar_mask_l);
	audif_reg_val_write(AFULL_AEMPTY_INTR_MASK, AFULL_AR_R_MASK,
				BIT_WIDTH_1, afull_ar_mask_r);
	audif_all_slot_update();
#endif
	return 0;
}
EXPORT_SYMBOL(audif_afull_ar_mask_enable);

int audif_fifo_clr_ap_enable(void)
{
#ifdef SIMULATE_COMMON
#else
	audif_bit_clr(FIFO_PTR_CLR, FIFO_CLR_AP_L);
	audif_bit_clr(FIFO_PTR_CLR, FIFO_CLR_AP_R);
	audif_all_slot_update();
#endif
	return 0;
}
EXPORT_SYMBOL(audif_fifo_clr_ap_enable);

int audif_ad_fifo_enable_ar(bool enable)
{
#ifdef SIMULATE_COMMON
	AUD_LOG_INFO(AUD_COMM, "%s: ar_fifo_enable = %d",
		__func__, enable);
#else
	uint16_t ar_fifo_enable;

	if (enable)
		ar_fifo_enable = SW_ON;
	else
		ar_fifo_enable = SW_OFF;

	audif_reg_val_write(AD_FIFO_ENABLE, AR_FIFO_ENABLE,
				BIT_WIDTH_1, ar_fifo_enable);
	audif_all_slot_update();
#endif
	return 0;
}
EXPORT_SYMBOL(audif_ad_fifo_enable_ar);

bool audif_ap_fifo_full(void)
{
	uint32_t ret = 0;

	ret = audif_bit_read(AFIFO_STATUS_AP_L, 31);
	if (ret) {
		AUD_LOG_DBG(AUD_COMM, "ap_l full = %x", ret);
		return true;
	}

	ret = audif_bit_read(AFIFO_STATUS_AP_R, 31);
	if (ret) {
		AUD_LOG_DBG(AUD_COMM, "ap_r full = %x", ret);
		return true;
	}

	return false;
}


#ifdef SIMULATE_COMMON
#else
static void audif_dac_src_module_config(enum audif_dac_src_type_t type,
			enum audif_dac_src_mode_t dac_src_mode, uint16_t src_sw)
{
	uint16_t mode = dac_src_mode;
	void __iomem *dac_fifo_rd_val, *period_num_a, *period_num_b, *period_num_c;
	uint16_t mode_bit = 0;
	uint32_t fs_mode_cfg;

	fs_mode_cfg = audif_reg_read32(AUDIF_FS_MODE_CFG);
	dac_fifo_rd_val = INIT_VALUE;
	period_num_a = INIT_VALUE;
	period_num_b = INIT_VALUE;
	period_num_c = INIT_VALUE;
	switch (type) {
	case AUDIF_AP_DAC_SRC:
		dac_fifo_rd_val = AP_FIFO_RD_VAL;
		period_num_a = PERIOD_NUM1;
		period_num_b = PERIOD_NUM2;
		period_num_c = PERIOD_NUM3;
		if (mode != AUDIF_DAC_SRC_BYPASS)
			audif_bit_set(AUDIF_DAC_SRC_CFG, AP_DAC_SRC_SW);
		mode_bit = AP_DAC_SRC_MODE;
		break;
	case AUDIF_DAC0_SRC:
		dac_fifo_rd_val = DA0_FIFO_RD_VAL;
		period_num_a = PERIOD_NUM4;
		period_num_b = PERIOD_NUM5;
		period_num_c = PERIOD_NUM6;
		if (mode != AUDIF_DAC_SRC_BYPASS)
			audif_bit_set(AUDIF_DAC_SRC_CFG, DAC0_SRC_SW);
		mode_bit = DAC0_SRC_MODE;
		break;
	case AUDIF_DAC1_SRC:
		dac_fifo_rd_val = DA1_FIFO_RD_VAL;
		period_num_a = PERIOD_NUM7;
		period_num_b = PERIOD_NUM8;
		period_num_c = PERIOD_NUM9;
		if (mode != AUDIF_DAC_SRC_BYPASS)
			audif_bit_set(AUDIF_DAC_SRC_CFG, DAC1_SRC_SW);
		mode_bit = DAC1_SRC_MODE;
		break;
	case AUDIF_DAC1_BT_SRC:
		dac_fifo_rd_val = BT_FIFO_RD_VAL;
		mode = AUDIF_DAC_SRC_BYPASS;
		//BIT_TRUE(AUDIF_DAC_SRC_CFG, DAC1_BT_SRC_SW);
		mode_bit = DAC1_BT_SRC_MODE;
		break;
	default:
		AUD_LOG_ERR(AUD_COMM, "ERROR");
	}

	if (src_sw == AUDIF_DAC_SRC_SW_ON) {
		audif_reg_val_write(AUDIF_DAC_SRC_CFG, mode_bit, BIT_WIDTH_4, mode);

		switch (mode) {
		case AUDIF_DAC_SRC_BYPASS:
			/*audif 96k mode*/
			if (audif_bit_read(AUDIF_FS_MODE_CFG, AUDIF_WORKING_FS_MODE))
				audif_reg_write32(dac_fifo_rd_val, 0x01e02fff);
			else
			/*audif 48k mode*/
				audif_reg_write32(dac_fifo_rd_val, 0x03ca2fff);

			if (type != AUDIF_DAC1_BT_SRC) {
				audif_reg_write32(period_num_a, 0x0);
				audif_reg_write32(period_num_b, 0x0);
				audif_reg_write32(period_num_c, 0x0);
			}
				else {
					if (fs_mode_cfg == 1)
						audif_reg_write32(dac_fifo_rd_val, 0x01e02fff);
				}
			break;
		case AUDIF_DAC_SRC_44_1K_2_48K:
			audif_reg_write32(dac_fifo_rd_val, 0x04262fff);
			if (type != AUDIF_DAC1_BT_SRC) {
				audif_reg_write32(period_num_a, 0xf0);
				audif_reg_write32(period_num_b, 0x28);
				audif_reg_write32(period_num_c, 0x1);
			}
			break;
		case AUDIF_DAC_SRC_32K_2_48K:
			audif_reg_write32(dac_fifo_rd_val, 0x05b82fff);
			if (type != AUDIF_DAC1_BT_SRC) {
				audif_reg_write32(period_num_a, 0x0);
				audif_reg_write32(period_num_b, 0x0);
				audif_reg_write32(period_num_c, 0x0);
			}
			break;
		case AUDIF_DAC_SRC_24K_2_48K:
			audif_reg_write32(dac_fifo_rd_val, 0x07a02fff);
			if (type != AUDIF_DAC1_BT_SRC) {
				audif_reg_write32(period_num_a, 0x0);
				audif_reg_write32(period_num_b, 0x0);
				audif_reg_write32(period_num_c, 0x0);
			}
			break;
		case AUDIF_DAC_SRC_22_05K_2_48K:
			audif_reg_write32(dac_fifo_rd_val, 0x084c2fff);
			if (type != AUDIF_DAC1_BT_SRC) {
				audif_reg_write32(period_num_a, 0x1e0);
				audif_reg_write32(period_num_b, 0x50);
				audif_reg_write32(period_num_c, 0x1);
			}
			break;
		case AUDIF_DAC_SRC_16K_2_48K:
			audif_reg_write32(dac_fifo_rd_val, 0x0b702fff);
			if (type != AUDIF_DAC1_BT_SRC) {
				audif_reg_write32(period_num_a, 0x0);
				audif_reg_write32(period_num_b, 0x0);
				audif_reg_write32(period_num_c, 0x0);
			}
			break;
		case AUDIF_DAC_SRC_12K_2_48K:
			audif_reg_write32(dac_fifo_rd_val, 0x0f402fff);
			if (type != AUDIF_DAC1_BT_SRC) {
				audif_reg_write32(period_num_a, 0x0);
				audif_reg_write32(period_num_b, 0x0);
				audif_reg_write32(period_num_c, 0x0);
			}
			break;
		case AUDIF_DAC_SRC_11_025K_2_48K:
			audif_reg_write32(dac_fifo_rd_val, 0x10982fff);
			if (type != AUDIF_DAC1_BT_SRC) {
				audif_reg_write32(period_num_a, 0x3c0);
				audif_reg_write32(period_num_b, 0xa0);
				audif_reg_write32(period_num_c, 0x1);
			}
			break;
		case AUDIF_DAC_SRC_8K_2_48K:
			audif_reg_write32(dac_fifo_rd_val,  0x16e02fff);
			if (type != AUDIF_DAC1_BT_SRC) {
				audif_reg_write32(period_num_a, 0x0);
				audif_reg_write32(period_num_b, 0x0);
				audif_reg_write32(period_num_c, 0x0);
			}
			break;
		case AUDIF_DAC_SRC_96K_2_48K:
				audif_reg_write32(dac_fifo_rd_val,  0x01e02fff);
				if (type != AUDIF_DAC1_BT_SRC) {
					audif_reg_write32(period_num_a, 0x0);
					audif_reg_write32(period_num_b, 0x0);
					audif_reg_write32(period_num_c, 0x0);
				}
				break;
		default:
			AUD_LOG_ERR(AUD_COMM, "ERROR");
		}
	} else {
		if (fs_mode_cfg == 1)
			audif_reg_write32(dac_fifo_rd_val, 0x01e02fff);
		else
			audif_reg_write32(dac_fifo_rd_val, 0x03ca2fff);

		audif_bit_clr(AUDIF_DAC_SRC_CFG, type << BIT_WIDTH_3);
	}
}
#endif

void audif_dac_format_set(struct data_type *playback_data_type)
{
	int ap_format;

	ap_format = playback_data_type->data_format;

	switch (ap_format) {
	case FORMAT_16BIT:
		audif_reg_val_write(AUDIF_DAC_FORMAT_CFG, AP_DAC_FORMAT,
				BIT_WIDTH_2, LOW_16BIT);
		break;
	case FORMAT_24BIT:
		audif_reg_val_write(AUDIF_DAC_FORMAT_CFG, AP_DAC_FORMAT,
				BIT_WIDTH_2, HIGH_24BIT);
		break;
	case FORMAT_32BIT:
		audif_reg_val_write(AUDIF_DAC_FORMAT_CFG, AP_DAC_FORMAT,
				BIT_WIDTH_2, HIGH_24BIT);
		break;
	default:
		AUD_LOG_INFO(AUD_COMM, "unsupport format cfg");
	}
}

void audif_src_daap_set(bool enable, uint16_t ap_src_mode)
{
#ifdef SIMULATE_COMMON
	AUD_LOG_INFO(AUD_COMM, "src_daap_enable = %d, ap_src_mode = %d",
				enable, ap_src_mode);
#else
/* AP SRC  0:48000->48000;1:44100->48000;2:32000->48000;3:24000->48000;
 * 4:22050->48000;5:16000->48000;6:12000->48000;7:11025->48000;8:8000->48000;
 */
	uint16_t ap_dac_src_sw;

	if (enable == TRUE)
		ap_dac_src_sw = SW_ON;
	else
		ap_dac_src_sw = SW_OFF;

	audif_reg_val_write(AUDIF_DAC_SRC_CFG, AP_DAC_SRC_SW,
				BIT_WIDTH_1, ap_dac_src_sw);
	audif_reg_val_write(AUDIF_DAC_SRC_CFG, AP_DAC_SRC_MODE,
				BIT_WIDTH_4, ap_src_mode);
	audif_dac_src_module_config(AUDIF_AP_DAC_SRC, ap_src_mode, ap_dac_src_sw);
#endif
}

void audif_ar_control_mux_src_set(bool enable, uint16_t mux_ar,
			uint16_t ar_src_mode)
{
#ifdef SIMULATE_COMMON
	AUD_LOG_INFO(AUD_COMM, "src_ar_enable = %d, mux_ar = %d, ar_src_mode = %d",
				enable, mux_ar, ar_src_mode);
#else
//MUX_AR 0:0,1:adc0_l,2:adc1_l,3:adc2_l
/*
 * AR_SRC 0:48000->48000;  1:48000->44100;	2:48000->32000;	3:48000->24000;
 * 4:48000->22050;	5:48000->16000;	6:48000->12000;   7:48000->11025;	 8:48000->8000;
 */
	uint16_t ar_mux_l, ar_mux_r, ar_src_sw;

	if (enable == TRUE)
		ar_src_sw = SW_ON;
	else
		ar_src_sw = SW_OFF;

	ar_mux_l = mux_ar;
	ar_mux_r = mux_ar;

	audif_reg_val_write(AUDIF_AR_CFG, AR_MUX_L, BIT_WIDTH_2, ar_mux_l);
	audif_reg_val_write(AUDIF_AR_CFG, AR_MUX_R, BIT_WIDTH_2, ar_mux_r);
	audif_reg_val_write(AUDIF_AR_CFG, AR_SRC_MODE, BIT_WIDTH_4, ar_src_mode);
	audif_reg_val_write(AUDIF_AR_CFG, AR_SRC_SW, BIT_WIDTH_1, ar_src_sw);
	audif_all_slot_update();
#endif
}

void audif_daap_dg_control_set(bool enable, uint16_t daap_dg_l, uint16_t daap_dg_r)
{
#ifdef SIMULATE_COMMON
	AUD_LOG_INFO(AUD_COMM, "daap_dg_enable = %d, daap_dg_l = %d, daap_dg_l = %d",
				enable, daap_dg_l, daap_dg_l);
#else
	uint16_t daap_dg_en;

	if (enable == TRUE)
		daap_dg_en = SW_ON;
	else
		daap_dg_en = SW_OFF;

	audif_reg_val_write(AUDIF_AP_DG_CFG, DAC_AP_DG_L, BIT_WIDTH_8, daap_dg_l);
	audif_reg_val_write(AUDIF_AP_DG_CFG, DAC_AP_DG_R, BIT_WIDTH_8, daap_dg_r);
	audif_reg_val_write(AUDIF_AP_DG_CFG, DAC_AP_DG_EN, BIT_WIDTH_1, daap_dg_en);
	audif_all_slot_update();
#endif
}
EXPORT_SYMBOL(audif_daap_dg_control_set);

#ifdef SIMULATE_COMMON
#else
uint16_t audif_fifo_status_ap_l_get(void)
{
	uint16_t size;

	size = (audif_reg_read32(AFIFO_STATUS_AP_L) >> BIT_WIDTH_18) & 0x3FF;
	return size;
}

uint16_t audif_fifo_status_ar_l_get(void)
{
	uint16_t size;

	size = (audif_reg_read32(AFIFO_STATUS_AR_L) >> BIT_WIDTH_18) & 0x3FF;
	return size;
}
#endif

int audif_clk_status_read(void)
{
	void __iomem *audsys_poweroff_state;
	void __iomem *audif_clk_state;
	int dsp_power_st = 0;
	int audif_clk_st = 0;

	audsys_poweroff_state = (void __iomem *)g_xr_audif.audsys_poweroff_state;
	if (!audsys_poweroff_state)
		return -EINVAL;

	dsp_power_st = audif_reg_read32(audsys_poweroff_state);
	if ((dsp_power_st == AUDSYS_POWERON_STATE)
		|| (dsp_power_st == AUDSYS_PREPARE_POWERDOWN)) {
		audif_clk_state = (void __iomem *)g_xr_audif.audif_clk_state;

		audif_clk_st = audif_reg_read32(audif_clk_state)
			& AUDIO_CRG_CLKGT0_RO_GT_CLK_AUDIF_MASK;
		if (audif_clk_st)
			return 0;
		else
			return -EINVAL;
	} else
		return -EINVAL;
}
EXPORT_SYMBOL(audif_clk_status_read);

static void audif_ap_params_set(audif_data_cb cb, void *priv,
			struct data_type *playback_data_type)
{
	struct data_type *ap_data_type = &g_xr_audif.playback_data_type;

	g_xr_audif.ap_callback = cb;
	g_xr_audif.ap_strm = priv;
	memcpy(ap_data_type, playback_data_type, sizeof(struct data_type));
}

int audif_ap_fifo_write_register(audif_data_cb cb, void *priv,
			struct data_type *playback_data_type)
{
	int ap_sample;

	if (!cb) {
		AUD_LOG_ERR(AUD_COMM, "invalid cb");
		return -EINVAL;
	}

	AUD_LOG_DBG_LIM(AUD_COMM, "audif");
	audif_ap_params_set(cb, priv, playback_data_type);
	ap_sample = playback_data_type->data_sample;

	switch (ap_sample) {
	case SAMPLE_8000:
		audif_src_daap_set(TRUE, SRC_8000_TO_48000);
		break;
	case SAMPLE_12000:
		audif_src_daap_set(TRUE, SRC_12000_TO_48000);
		break;
	case SAMPLE_16000:
		audif_src_daap_set(TRUE, SRC_16000_TO_48000);
		break;
	case SAMPLE_24000:
		audif_src_daap_set(TRUE, SRC_24000_TO_48000);
		break;
	case SAMPLE_32000:
		audif_src_daap_set(TRUE, SRC_32000_TO_48000);
		break;
	case SAMPLE_48000:
		audif_src_daap_set(TRUE, SRC_48000_TO_48000);
		break;
	default:
		AUD_LOG_INFO(AUD_COMM, "unsupport sample");
		audif_src_daap_set(FALSE, SRC_48000_TO_48000);
		break;
	}

	audif_dac_format_set(playback_data_type);

	if (!g_xr_audif.irq_ap_enable) {
		enable_irq(g_xr_audif.irqno_ap);
		mutex_lock(&g_xr_audif.ap_lock);
		g_xr_audif.irq_ap_enable = 1;
		mutex_unlock(&g_xr_audif.ap_lock);
	}

	return 0;
}
EXPORT_SYMBOL(audif_ap_fifo_write_register);

int audif_ap_fifo_write_unregister(void)
{
	AUD_LOG_DBG_LIM(AUD_COMM, "audif");
	g_xr_audif.ap_callback = NULL;
	g_xr_audif.ap_strm = NULL;
	if (g_xr_audif.irq_ap_enable) {
		disable_irq_nosync(g_xr_audif.irqno_ap);
		mutex_lock(&g_xr_audif.ap_lock);
		g_xr_audif.irq_ap_enable = 0;
		mutex_unlock(&g_xr_audif.ap_lock);
	}

	return 0;
}
EXPORT_SYMBOL(audif_ap_fifo_write_unregister);

static void audif_ar_params_set(audif_data_cb cb, void *priv,
			struct data_type *capture_data_type)
{
	struct data_type *ar_data_type = &g_xr_audif.capture_data_type;

	g_xr_audif.ar_callback = cb;
	g_xr_audif.ar_strm = priv;
	memcpy(ar_data_type, capture_data_type, sizeof(struct data_type));
}

int audif_ar_fifo_read_register(audif_data_cb cb, void *priv,
			struct data_type *capture_data_type)
{
	int ar_sample;
	uint16_t ar_mux = 1;

	if (!cb) {
		AUD_LOG_ERR(AUD_COMM, "invalid cb");
		return -EINVAL;
	}

	AUD_LOG_DBG_LIM(AUD_COMM, "audif");
	audif_ar_params_set(cb, priv, capture_data_type);
	ar_sample = capture_data_type->data_sample;

	switch (ar_sample) {
	case SAMPLE_8000:
		audif_ar_control_mux_src_set(TRUE, ar_mux, SRC_48000_TO_8000);
		break;
	case SAMPLE_12000:
		audif_ar_control_mux_src_set(TRUE, ar_mux, SRC_48000_TO_12000);
		break;
	case SAMPLE_16000:
		audif_ar_control_mux_src_set(TRUE, ar_mux, SRC_48000_TO_16000);
		break;
	case SAMPLE_24000:
		audif_ar_control_mux_src_set(TRUE, ar_mux, SRC_48000_TO_24000);
		break;
	case SAMPLE_32000:
		audif_ar_control_mux_src_set(TRUE, ar_mux, SRC_48000_TO_32000);
		break;
	case SAMPLE_48000:
		audif_ar_control_mux_src_set(FALSE, ar_mux, SRC_AR_48000_TO_48000);
		break;
	default:
		AUD_LOG_INFO(AUD_COMM, "unsupport sample");
		audif_ar_control_mux_src_set(FALSE, ar_mux, SRC_AR_48000_TO_48000);
		break;
	}

	/* FPGA stage: ADC DG set 18db */
	audif_reg_write32(AUDIF_ADC0_DG_CFG, 0x1);

	if (!g_xr_audif.irq_ar_enable) {
		enable_irq(g_xr_audif.irqno_ar);
		mutex_lock(&g_xr_audif.ar_lock);
		g_xr_audif.irq_ar_enable = 1;
		mutex_unlock(&g_xr_audif.ar_lock);
	}

	return 0;
}
EXPORT_SYMBOL(audif_ar_fifo_read_register);

int audif_ar_fifo_read_unregister(void)
{
	AUD_LOG_DBG_LIM(AUD_COMM, "audif");
	g_xr_audif.ar_callback = NULL;
	g_xr_audif.ar_strm = NULL;
	if (g_xr_audif.irq_ar_enable) {
		disable_irq_nosync(g_xr_audif.irqno_ar);
		mutex_lock(&g_xr_audif.ar_lock);
		g_xr_audif.irq_ar_enable = 0;
		mutex_unlock(&g_xr_audif.ar_lock);
	}

	return 0;
}
EXPORT_SYMBOL(audif_ar_fifo_read_unregister);

#ifndef SIMULATE_COMMON
static int audif_playback_data_frame_get(uint32_t size)
{
	int play_format, play_channel;
	uint32_t playback_data_frame = 0;

	play_format = g_xr_audif.playback_data_type.data_format;
	play_channel = g_xr_audif.playback_data_type.data_channel;

	switch (play_format) {
	case FORMAT_16BIT:
		playback_data_frame = size / (2 * play_channel);
		break;
	case FORMAT_24BIT:
		playback_data_frame = size / (3 * play_channel);
		break;
	case FORMAT_32BIT:
		playback_data_frame = size / (4 * play_channel);
		break;
	default:
		AUD_LOG_INFO(AUD_COMM, "unsupport format");
		playback_data_frame = size / (2 * play_channel);
		break;
	}

	return playback_data_frame;
}
#endif

static inline uint32_t audif_playback_data_l_get(void *data_addr, u64 phy_addr,
			uint32_t count)
{
	uint32_t data_l = 0;
	int play_format, play_channel;

	play_format = g_xr_audif.playback_data_type.data_format;
	play_channel = g_xr_audif.playback_data_type.data_channel;

	if ((play_format == FORMAT_16BIT) && (play_channel == CHANNEL_1))
		data_l = *((uint16_t *)data_addr + count);
	else if ((play_format == FORMAT_16BIT) && (play_channel == CHANNEL_2))
		data_l = ((*((uint32_t *)data_addr + count)) & 0xFFFF0000) >> BIT_WIDTH_16;
	else if ((play_format == FORMAT_24BIT) && (play_channel == CHANNEL_1))
		data_l = *((uint8_t *)data_addr + count * 3);
	else if ((play_format == FORMAT_24BIT) && (play_channel == CHANNEL_2))
		data_l = *((uint8_t *)data_addr + count * 6);
	else if ((play_format == FORMAT_32BIT) && (play_channel == CHANNEL_1))
		data_l = *((uint32_t *)data_addr + count);
	else if ((play_format == FORMAT_32BIT) && (play_channel == CHANNEL_2))
		data_l = *((uint32_t *)data_addr + count * 2);
	else
		data_l = 0;

	return data_l;
}

static inline uint32_t audif_playback_data_r_get(void *data_addr, u64 phy_addr,
			uint32_t count)
{
	uint32_t data_r = 0;
	int play_format, play_channel;

	play_format = g_xr_audif.playback_data_type.data_format;
	play_channel = g_xr_audif.playback_data_type.data_channel;

	if ((play_format == FORMAT_16BIT) && (play_channel == CHANNEL_1))
		data_r = *((uint16_t *)data_addr + count);
	else if ((play_format == FORMAT_16BIT) && (play_channel == CHANNEL_2))
		data_r = (*((uint32_t *)data_addr + count)) & 0x0000FFFF;
	else if ((play_format == FORMAT_24BIT) && (play_channel == CHANNEL_1))
		data_r = *((uint8_t *)data_addr + count * 3);
	else if ((play_format == FORMAT_24BIT) && (play_channel == CHANNEL_2))
		data_r = *((uint8_t *)data_addr + count * 6 + 3);
	else if ((play_format == FORMAT_32BIT) && (play_channel == CHANNEL_1))
		data_r = *((uint32_t *)data_addr + count);
	else if ((play_format == FORMAT_32BIT) && (play_channel == CHANNEL_2))
		data_r = *((uint32_t *)data_addr + count * 2 + 1);

	return data_r;
}

/* write ap fifo */
int audif_ap_fifo_write_data(void *vir_addr, u64 phy_addr, size_t size)
{
#ifdef SIMULATE_COMMON
	AUD_LOG_INFO(AUD_COMM, "audif");
	fk_simulate_core_write_file(vir_addr, size);
#else
	/* 32bit 2ch high 24bit */
	uint32_t data_l, data_r, index, fifo_size;
	uint16_t ap_l_size;

	if (!vir_addr)
		return 0;

	ap_l_size = audif_fifo_status_ap_l_get();
	if (ap_l_size == 0)
		AUD_LOG_INFO(AUD_COMM, "ap fifo is empty, insufficient performance");

	fifo_size = audif_playback_data_frame_get(size);

	for (index = 0; index < fifo_size; index++) {
		if (audif_ap_fifo_full() == true)
			udelay(5);
		data_l = audif_playback_data_l_get(vir_addr, phy_addr, index);
		data_r = audif_playback_data_r_get(vir_addr, phy_addr, index);
		audif_reg_write32(AP_FIFO_L, (uint32_t)data_l);
		audif_reg_write32(AP_FIFO_R, (uint32_t)data_r);
	}

#endif
	return 0;
}
EXPORT_SYMBOL(audif_ap_fifo_write_data);

#ifndef SIMULATE_COMMON
static int audif_capture_data_frame_get(uint32_t size)
{
	int cap_format, cap_channel;
	uint32_t capture_data_frame = 0;

	cap_format = g_xr_audif.capture_data_type.data_format;
	cap_channel = g_xr_audif.capture_data_type.data_channel;

	switch (cap_format) {
	case FORMAT_16BIT:
		capture_data_frame = size / (2 * cap_channel);
		break;
	case FORMAT_24BIT:
		capture_data_frame = size / (3 * cap_channel);
		break;
	case FORMAT_32BIT:
		capture_data_frame = size / (4 * cap_channel);
		break;
	default:
		AUD_LOG_INFO(AUD_COMM, "unsupport format");
		capture_data_frame = size / (2 * cap_channel);
		break;
	}

	AUD_LOG_DBG_LIM(AUD_COMM, "frame = %d", capture_data_frame);
	return capture_data_frame;
}
#endif

static inline void audif_capture_data_set(void *data_addr, u64 phy_addr,
			uint32_t count, uint32_t l_data, uint32_t r_data)
{
	uint32_t data_lr = 0;
	uint32_t data_l = 0;
	uint32_t data_r = 0;
	int cap_format, cap_channel;

	cap_format = g_xr_audif.capture_data_type.data_format;
	cap_channel = g_xr_audif.capture_data_type.data_channel;

	if ((cap_format == FORMAT_16BIT) && (cap_channel == CHANNEL_1)) {
		data_lr = (l_data & 0xFFFF00) >> BIT_WIDTH_8;
		*((uint16_t *)data_addr + count) = (uint16_t)data_lr;
	} else if ((cap_format == FORMAT_16BIT) && (cap_channel == CHANNEL_2)) {
		data_lr = ((l_data & 0xFFFF00) << BIT_WIDTH_8) |
			((r_data & 0xFFFF00) >> BIT_WIDTH_8);
		*((uint32_t *)data_addr + count) = data_lr;
	} else if ((cap_format == FORMAT_24BIT) && (cap_channel == CHANNEL_1)) {
		data_lr = l_data << BIT_WIDTH_8;
		*((uint8_t *)data_addr + count * 3) = data_lr;
	} else if ((cap_format == FORMAT_24BIT) && (cap_channel == CHANNEL_2)) {
		data_l = l_data << BIT_WIDTH_8;
		data_r = r_data << BIT_WIDTH_8;
		*((uint8_t *)data_addr + count * 6) = data_l;
		*((uint8_t *)data_addr + count * 6 + 3) = data_r;
	} else if ((cap_format == FORMAT_32BIT) && (cap_channel == CHANNEL_1)) {
		data_lr = l_data << BIT_WIDTH_8;
		*((uint32_t *)data_addr + count) = data_lr;
	} else if ((cap_format == FORMAT_32BIT) && (cap_channel == CHANNEL_2)) {
		data_l = l_data << BIT_WIDTH_8;
		data_r = r_data << BIT_WIDTH_8;
		*((uint32_t *)data_addr + count * 2) = data_l;
		*((uint32_t *)data_addr + count * 2 + 1) = data_r;
	}

}

/* read ar fifo */
int audif_ar_fifo_read_data(void *vir_addr, u64 phy_addr, size_t size)
{
#ifdef SIMULATE_COMMON
	int frame_no = 0;

	AUD_LOG_INFO_LIM(AUD_COMM, "audif");
	memset(vir_addr, frame_no, size);
	return size;
#else
	/* TODO : read data */
	uint32_t data_l, data_r;
	uint16_t index, ar_l_size, a_size, frame_count;

	if (!vir_addr)
		return 0;

	a_size = audif_capture_data_frame_get(size);
	ar_l_size = audif_fifo_status_ar_l_get();
	if (ar_l_size == 512)
		AUD_LOG_INFO(AUD_COMM, "ar fifo is full");

	frame_count = (a_size <= ar_l_size) ? a_size : ar_l_size;
	for (index = 0; index < frame_count; index++) {
		data_l = audif_reg_read32(AR_FIFO_L);
		data_r = audif_reg_read32(AR_FIFO_R);

		audif_capture_data_set(vir_addr, phy_addr, index, data_l, data_r);
	}

	return frame_count;
#endif
}
EXPORT_SYMBOL(audif_ar_fifo_read_data);

void audif_ap_fifo_workqueue(struct work_struct *work)
{
	if (g_xr_audif.ap_callback) {
		g_xr_audif.ap_callback((void *)g_xr_audif.ap_strm);
		audif_aempty_ap_mask_enable(TRUE);
	}
}

void audif_ar_fifo_workqueue(struct work_struct *work)
{
	if (g_xr_audif.ar_callback) {
		g_xr_audif.ar_callback((void *)g_xr_audif.ar_strm);
		audif_afull_ar_mask_enable(TRUE);
	}
}

irqreturn_t audif_ap_handler(int irqno, void *dev)
{
	audif_aempty_ap_mask_enable(FALSE);
	schedule_work_on(2, &g_xr_audif.audif_ap_write_work);

	return IRQ_HANDLED;
}

void audif_ap_schedule_work(void)
{
	schedule_work_on(2, &g_xr_audif.audif_ap_write_work);
}

irqreturn_t audif_ar_handler(int irqno, void *dev)
{
	audif_afull_ar_mask_enable(FALSE);
	schedule_work_on(2, &g_xr_audif.audif_ar_read_work);

	return IRQ_HANDLED;
}

void audif_ar_schedule_work(void)
{
	schedule_work_on(2, &g_xr_audif.audif_ar_read_work);
}

#ifdef SIMULATE_COMMON
static int audif_ap_fifo_thread(void *priv)
{
	AUD_LOG_INFO(AUD_COMM, "audif");
	for (;;) {
		msleep(20);
		if (!g_xr_audif.ap_mask_flag)
			continue;
		else
			g_xr_audif.ap_callback((void *)g_xr_audif.ap_strm);
	}
	return 0;
}

static int audif_ar_fifo_thread(void *priv)
{
	AUD_LOG_INFO(AUD_COMM, "audif");
	for (;;) {
		msleep(20);
		if (!g_xr_audif.ar_mask_flag)
			continue;
		else
			g_xr_audif.ar_callback((void *)g_xr_audif.ar_strm);
	}
	return 0;
}

int audif_dev_thread_init(void *priv)
{
	AUD_LOG_INFO(AUD_COMM, "audif");
	g_xr_audif.ap_fifo_thrd = kthread_run(audif_ap_fifo_thread, NULL, "pdata%d", 1);
	g_xr_audif.ar_fifo_thrd = kthread_run(audif_ar_fifo_thread, NULL, "cdata%d", 1);
	return 0;
}
#endif

static int audif_dev_probe(struct platform_device *pdev)
{
	int ret = 0;
#ifdef SIMULATE_COMMON

#else
	struct xr_audif_dev *adev = (struct xr_audif_dev *)&g_xr_audif;
	struct resource *mem = NULL;

	AUD_LOG_INFO(AUD_COMM, "dev name %s", dev_name(&pdev->dev));

	adev->reg = devm_platform_get_and_ioremap_resource(pdev, 0, &mem);
	if (!adev->reg) {
		AUD_LOG_ERR(AUD_COMM, "iomap error");
		ret = -ENODEV;
	}

	g_xr_audif.irqno_ap = irq_of_parse_and_map(pdev->dev.of_node, 0);
	g_xr_audif.irqno_ar = irq_of_parse_and_map(pdev->dev.of_node, 1);

	mutex_init(&g_xr_audif.ap_lock);
	mutex_init(&g_xr_audif.ar_lock);

	INIT_WORK(&g_xr_audif.audif_ap_write_work, audif_ap_fifo_workqueue);
	INIT_WORK(&g_xr_audif.audif_ar_read_work, audif_ar_fifo_workqueue);

	if (request_irq(g_xr_audif.irqno_ap, audif_ap_handler, IRQF_TRIGGER_HIGH,
			"audif ap dev", (void *)&g_xr_audif.dev)) {
		AUD_LOG_ERR(AUD_COMM, "request_irq error");
		return -EINVAL;
	}

	if (request_irq(g_xr_audif.irqno_ar, audif_ar_handler, IRQF_TRIGGER_HIGH,
			"audif ar dev", (void *)&g_xr_audif.dev)) {
		AUD_LOG_ERR(AUD_COMM, "request_irq error");
		return -EINVAL;
	}
	g_xr_audif.irq_ap_enable = 1;
	g_xr_audif.irq_ar_enable = 1;

	adev->audsys_poweroff_state = ioremap(AUDSYS_POWER_STATE, 0x4);
	if (!adev->audsys_poweroff_state) {
		AUD_LOG_ERR(AUD_COMM, "can't remap AUDSYS_POWEROFF_STATE");
		return -EINVAL;
	}

	adev->audif_clk_state = ioremap((ACPU_ASP_CRG + AUDIO_CRG_CLKGT0_RO), 0x4);
	if (!adev->audif_clk_state) {
		AUD_LOG_ERR(AUD_COMM, "can't remap ACPU_ASP_CRG AUDIO_CRG_CLKGT0_RO");
		iounmap(adev->audsys_poweroff_state);
		return -EINVAL;
	}
#endif
	return ret;
}

static int audif_dev_remove(struct platform_device *pdev)
{
	free_irq(g_xr_audif.irqno_ap, &g_xr_audif.dev);
	free_irq(g_xr_audif.irqno_ar, &g_xr_audif.dev);

	if (g_xr_audif.audsys_poweroff_state)
		iounmap(g_xr_audif.audsys_poweroff_state);
	if (g_xr_audif.audif_clk_state)
		iounmap(g_xr_audif.audif_clk_state);

	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static const struct of_device_id xr_audif_dt_match[] = {
	{ .compatible = "xring,xr-audif", },
	{},
};

static struct platform_driver xr_audif_driver = {
	.driver = {
		.name = "xr-audif",
		.owner = THIS_MODULE,
		.of_match_table = xr_audif_dt_match,
		.suppress_bind_attrs = true,
	},
	.probe = audif_dev_probe,
	.remove = audif_dev_remove,
};

int __init xr_audif_init(void)
{
#ifdef SIMULATE_COMMON
	int ret = 0;

	ret = audif_dev_thread_init(NULL);
#endif
	return platform_driver_register(&xr_audif_driver);
}

void xr_audif_exit(void)
{
	platform_driver_unregister(&xr_audif_driver);
}

MODULE_DESCRIPTION("XRING AUDIO COMMON AUDIF");
MODULE_LICENSE("Dual BSD/GPL");
