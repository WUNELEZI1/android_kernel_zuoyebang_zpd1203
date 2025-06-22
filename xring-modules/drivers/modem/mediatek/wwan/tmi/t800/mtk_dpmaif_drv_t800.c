// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2022, MediaTek Inc.
 */

#define pr_fmt(fmt) "DATA_DRV: " fmt

#include <linux/delay.h>
#include <linux/random.h>
#include "mtk_common.h"
#include "mtk_data_plane.h"
#include "mtk_debug.h"
#include "mtk_dev.h"
#include "mtk_dpmaif_drv.h"
#include "mtk_dpmaif_reg_t800.h"
#include "mtk_reg.h"

#ifdef CONFIG_TX00_UT_DPMAIF_DRV_T800
#include "ut_dpmaif_drv_t800_fake.h"
#endif

#define TAG "DATA_DRV"

#define DRV_TO_MDEV(__drv_info) ((__drv_info)->mdev)

/* 2ms -> 2 * 1000 / 10 = 200 */
#define POLL_MAX_TIMES		200
#define POLL_INTERVAL_US	10
#define REWRITE_INTERVAL	20

static void mtk_dpmaif_drv_dump_t800(struct dpmaif_drv_info *drv_info);

static void mtk_dpmaif_drv_reset(struct dpmaif_drv_info *drv_info)
{
	int val, cnt = 0;

	/*Set DPMAIF infra CG en */
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AP_INFRA_CG_SET, DPMAIF_AP_INFRA_CG_BIT);
	udelay(2);
	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AP_INFRA_CG_VAL);
	do {
		if ((mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AP_INFRA_CG_VAL)
		 	& DPMAIF_AP_INFRA_CG_BIT))
				break;
		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	/* Glitch protect on */
	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AP_INFRA_CLITCH_PORT);
	val &= ~(1 << 15);
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AP_INFRA_CLITCH_PORT, val);
	udelay(2);

	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AP_AO_RGU_ASSERT, DPMAIF_AP_AO_RST_BIT);
	/* Delay 2 us to wait for hardware ready. */
	udelay(2);
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AP_RGU_ASSERT, DPMAIF_AP_RST_BIT);
	udelay(2);
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AP_AO_RGU_DEASSERT, DPMAIF_AP_AO_RST_BIT);
	udelay(2);
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AP_RGU_DEASSERT, DPMAIF_AP_RST_BIT);
	udelay(2);

	/* Glitch protect off */
	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AP_INFRA_CLITCH_PORT);
	val |= (1 << 15);
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AP_INFRA_CLITCH_PORT, val);
	udelay(2);

	/* Set DPMAIF infra CG off*/
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AP_INFRA_CG_CLR, DPMAIF_AP_INFRA_CG_BIT);
	udelay(2);
	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AP_INFRA_CG_VAL);
	
	cnt = 0;
	do {
		if (!(mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AP_INFRA_CG_VAL) &
			 DPMAIF_AP_INFRA_CG_BIT))
				break;
		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);
}

static bool mtk_dpmaif_drv_sram_init(struct dpmaif_drv_info *drv_info)
{
	u32 val, cnt = 0;
	bool ret = true;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AP_MISC_RSTR_CLR);
	val |= DPMAIF_MEM_CLR_MASK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AP_MISC_RSTR_CLR, val);

	do {
		if (!(mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AP_MISC_RSTR_CLR) &
		     DPMAIF_MEM_CLR_MASK))
			break;

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to initialize sram.\n");
		return false;
	}
	return ret;
}

static bool mtk_dpmaif_drv_config(struct dpmaif_drv_info *drv_info)
{
	u32 val;

	/* Reset dpmaif HW setting. */
	mtk_dpmaif_drv_reset(drv_info);

	/* Initialize dpmaif sram. */
	if (!mtk_dpmaif_drv_sram_init(drv_info))
		return false;

	/* Set DPMAIF AP port mode. */
	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_RDY_CHK_THRES);
	val &= ~DPMAIF_PORT_MODE_MSK;
	val |= DPMAIF_PORT_MODE_PCIE;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_RDY_CHK_THRES, val);

	/* Set CG enable. */
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AP_MISC_CG_EN, 0x7f);
	return true;
}

static bool mtk_dpmaif_drv_init_intr(struct dpmaif_drv_info *drv_info)
{
	struct dpmaif_drv_irq_en_mask *irq_en_mask;
	u32 cnt = 0, cfg;

	irq_en_mask = &drv_info->drv_irq_en_mask;

	/* Set SW UL interrupt. */
	irq_en_mask->ap_ul_l2intr_en_mask = DPMAIF_AP_UL_L2INTR_EN_MASK;

	/* Clear dummy status. */
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TISAR0, 0xFFFFFFFF);

	/* Set HW UL interrupt enable mask. */
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TICR0,
		       irq_en_mask->ap_ul_l2intr_en_mask);
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TISR0,
		       ~(irq_en_mask->ap_ul_l2intr_en_mask));
	mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TISR0);

	/* Check UL interrupt mask set done. */
	do {
		if (!((mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TIMR0) &
		     irq_en_mask->ap_ul_l2intr_en_mask) == irq_en_mask->ap_ul_l2intr_en_mask))
			break;

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to set UL interrupt mask.\n");
		return false;
	}

	/* Set SW DL interrupt. */
	irq_en_mask->ap_dl_l2intr_en_mask = DPMAIF_AP_DL_L2INTR_EN_MASK;

	/* Clear dummy status. */
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TISAR0, 0xFFFFFFFF);

	/* Set HW DL interrupt enable mask. */
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TISR0,
		       ~(irq_en_mask->ap_dl_l2intr_en_mask));
	mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TISR0);

	/* Check DL interrupt mask set done. */
	cnt = 0;
	do {
		if (!((mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TIMR0) &
		    irq_en_mask->ap_dl_l2intr_en_mask) == irq_en_mask->ap_dl_l2intr_en_mask))
			break;

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to set DL interrupt mask\n");
		return false;
	}

	/* Set SW AP IP busy. */
	irq_en_mask->ap_udl_ip_busy_en_mask = DPMAIF_AP_UDL_IP_BUSY_EN_MASK;

	/* Clear dummy status. */
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_IP_BUSY, 0xFFFFFFFF);

	/* Set HW IP busy mask. */
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DLUL_IP_BUSY_MASK,
		       irq_en_mask->ap_udl_ip_busy_en_mask);

	/* DLQ HPC setting. */
	cfg = mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_UL_AP_L1TIMR0);
	cfg |= DPMAIF_DL_INT_Q2APTOP_MSK | DPMAIF_DL_INT_Q2TOQ1_MSK | DPMAIF_UL_TOP0_INT_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_UL_AP_L1TIMR0, cfg);
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_HPC_INTR_MASK, 0xffff);

	MTK_INFO(DRV_TO_MDEV(drv_info),
		 "ul_mask=0x%08x, dl_mask=0x%08x, busy_mask=0x%08x\n",
		 irq_en_mask->ap_ul_l2intr_en_mask,
		 irq_en_mask->ap_dl_l2intr_en_mask,
		 irq_en_mask->ap_udl_ip_busy_en_mask);
	return true;
}

static void mtk_dpmaif_drv_set_property(struct dpmaif_drv_info *drv_info,
					struct dpmaif_drv_cfg *drv_cfg)
{
	struct dpmaif_drv_property *drv_property = &drv_info->drv_property;
	struct dpmaif_drv_data_ring *ring;
	struct dpmaif_drv_dlq *dlq;
	struct dpmaif_drv_ulq *ulq;
	u32 i;

	drv_property->features = drv_cfg->features;

	BUILD_BUG_ON(DPMAIF_DLQ_NUM > DPMAIF_RXQ_CNT_MAX);
	for (i = 0; i < DPMAIF_DLQ_NUM; i++) {
		dlq = &drv_property->dlq[i];
		dlq->pit_base = drv_cfg->pit_base[i];
		dlq->pit_size = drv_cfg->pit_cnt[i];
		dlq->q_started = true;
	}

	BUILD_BUG_ON(DPMAIF_ULQ_NUM > DPMAIF_TXQ_CNT_MAX);
	for (i = 0; i < DPMAIF_ULQ_NUM; i++) {
		ulq = &drv_property->ulq[i];
		ulq->drb_base = drv_cfg->drb_base[i];
		ulq->drb_size = drv_cfg->drb_cnt[i];
		ulq->q_started = true;
	}

	ring = &drv_property->ring;

	/* Normal bat setting. */
	ring->normal_bat_base = drv_cfg->normal_bat_base;
	ring->normal_bat_size = drv_cfg->normal_bat_cnt;
	ring->normal_bat_pkt_bufsz = drv_cfg->normal_bat_buf_size;
	ring->normal_bat_remain_size = DPMAIF_HW_BAT_REMAIN;
	ring->normal_bat_rsv_length = DPMAIF_HW_BAT_RSVLEN;
	ring->chk_normal_bat_num = DPMAIF_HW_CHK_BAT_NUM;

	/* Frag bat setting. */
	if (drv_property->features & DATA_HW_F_FRAG) {
		ring->frag_bat_base = drv_cfg->frag_bat_base;
		ring->frag_bat_size = drv_cfg->frag_bat_cnt;
		ring->frag_bat_pkt_bufsz = drv_cfg->frag_bat_buf_size;
		ring->chk_frag_bat_num = DPMAIF_HW_CHK_FRG_NUM;
	}

	ring->mtu = drv_cfg->max_mtu;
	ring->pkt_bid_max_cnt = DPMAIF_HW_PKT_BIDCNT;
	ring->pkt_alignment = DPMAIF_HW_PKT_ALIGN;
	ring->chk_pit_num = DPMAIF_HW_CHK_PIT_NUM;
}

static void mtk_dpmaif_drv_init_common_hw(struct dpmaif_drv_info *drv_info)
{
	u32 val;

	/* Config PCIe mode. */
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_UL_RESERVE_AO_RW,
		       DPMAIF_PCIE_MODE_SET_VALUE);

	/* Bat cache enable. */
	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_INIT_CON1);
	val |= DPMAIF_DL_BAT_CACHE_PRI;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_INIT_CON1, val);

	/* Pit burst enable. */
	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_RDY_CHK_THRES);
	val |= DPMAIF_DL_BURST_PIT_EN;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_RDY_CHK_THRES, val);
}

static void mtk_dpmaif_drv_set_hpc_cntl(struct dpmaif_drv_info *drv_info)
{
	u32 cfg = 0;

	cfg = (DPMAIF_HPC_LRO_PATH_DF & 0x3) << 0;
	cfg |= (DPMAIF_HPC_ADD_MODE_DF & 0x3) << 2;
	cfg |= (DPMAIF_HASH_PRIME_DF & 0xf) << 4;
	cfg |= (DPMAIF_HPC_TOTAL_NUM & 0xff) << 8;

	/* Configuration include hpc dlq path,
	 * hpc add mode, hash prime, hpc total number.
	 */
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_DL_HPC_CNTL, cfg);
}

static void mtk_dpmaif_drv_set_agg_cfg(struct dpmaif_drv_info *drv_info)
{
	u32 cfg;

	cfg = (DPMAIF_AGG_MAX_LEN_DF & 0xffff) << 0;
	cfg |= (DPMAIF_AGG_TBL_ENT_NUM_DF & 0xffff) << 16;

	/* Configuration include agg max length, agg table number. */
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_DL_LRO_AGG_CFG, cfg);

	/* enable/disable AGG */
	cfg = mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_DL_RDY_CHK_FRG_THRES);
	if (drv_info->drv_property.features & DATA_HW_F_LRO)
		mtk_hw_write32(DRV_TO_MDEV(drv_info),
			       NRL2_DPMAIF_AO_DL_RDY_CHK_FRG_THRES, cfg | (0xff << 20));
	else
		mtk_hw_write32(DRV_TO_MDEV(drv_info),
			       NRL2_DPMAIF_AO_DL_RDY_CHK_FRG_THRES, cfg & 0xf00fffff);
}

static void mtk_dpmaif_drv_set_hash_bit_choose(struct dpmaif_drv_info *drv_info)
{
	u32 cfg;

	cfg = (DPMAIF_LRO_HASH_BIT_CHOOSE_DF & 0x7) << 0;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_DL_LROPIT_INIT_CON5, cfg);
}

static void mtk_dpmaif_drv_set_mid_pit_timeout_threshold(struct dpmaif_drv_info *drv_info)
{
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_DL_LROPIT_TIMEOUT0,
		       DPMAIF_MID_TIMEOUT_THRES_DF);
}

static void mtk_dpmaif_drv_set_dlq_timeout_threshold(struct dpmaif_drv_info *drv_info)
{
	u32 val, i;

	for (i = 0; i < DPMAIF_HPC_MAX_TOTAL_NUM; i++) {
		val = mtk_hw_read32(DRV_TO_MDEV(drv_info),
				    NRL2_DPMAIF_AO_DL_LROPIT_TIMEOUT1 + 4 * (i / 2));

		if (i % 2)
			val = (val & 0xFFFF) | (DPMAIF_LRO_TIMEOUT_THRES_DF << 16);
		else
			val = (val & 0xFFFF0000) | (DPMAIF_LRO_TIMEOUT_THRES_DF);

		mtk_hw_write32(DRV_TO_MDEV(drv_info),
			       NRL2_DPMAIF_AO_DL_LROPIT_TIMEOUT1 + (4 * (i / 2)), val);
	}
}

static void mtk_dpmaif_drv_clr_dlq_timeout_threshold(struct dpmaif_drv_info *drv_info)
{
	u32 val, i;

	for (i = 0; i < DPMAIF_HPC_MAX_TOTAL_NUM; i++) {
		val = mtk_hw_read32(DRV_TO_MDEV(drv_info),
				    NRL2_DPMAIF_AO_DL_LROPIT_TIMEOUT1 + 4 * (i / 2));

		if (i % 2)
			val = (val & 0xFFFF) | (1 << 16);
		else
			val = (val & 0xFFFF0000) | 1;

		mtk_hw_write32(DRV_TO_MDEV(drv_info),
			       NRL2_DPMAIF_AO_DL_LROPIT_TIMEOUT1 + (4 * (i / 2)), val);
	}
}

static void mtk_dpmaif_drv_set_dlq_start_prs_threshold(struct dpmaif_drv_info *drv_info)
{
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_DL_LROPIT_TRIG_THRES,
		       DPMAIF_LRO_PRS_THRES_DF & 0x3FFFF);
}

static void mtk_dpmaif_drv_toeplitz_hash_enable(struct dpmaif_drv_info *drv_info, u32 enable)
{
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_REG_TOE_HASH_EN, enable);
}

static void mtk_dpmaif_drv_hash_default_value_set(struct dpmaif_drv_info *drv_info, u32 hash)
{
	u32 val = mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_REG_HASH_CFG_CON);

	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_REG_HASH_CFG_CON,
		       (val & DPMAIF_HASH_DEFAULT_V_MASK) | hash);
}

static int mtk_dpmaif_drv_hash_sec_key_set(struct dpmaif_drv_info *drv_info, u8 *hash_key)
{
	u32 i, cnt = 0;
	u32 index;
	u32 val;

	for (i = 0; i < DPMAIF_HASH_SEC_KEY_NUM / 4; i++) {
		index = i << 2;
		val = hash_key[index] << 24 | hash_key[index + 1] << 16 |
			hash_key[index + 2] << 8 | hash_key[index + 3];
		mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_REG_HASH_SEC_KEY_0 + index, val);
	}

	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_REG_HASH_SEC_KEY_UPD, 1);

	do {
		if (!(mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_REG_HASH_SEC_KEY_UPD)))
			break;

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES)
		return -DATA_HW_REG_TIMEOUT;

	return 0;
}

static int mtk_dpmaif_drv_hash_sec_key_get(struct dpmaif_drv_info *drv_info, u8 *hash_key)
{
	u32 index;
	u32 val;
	u32 i;

	for (i = 0; i < DPMAIF_HASH_SEC_KEY_NUM / 4; i++) {
		index = i << 2;
		val = mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_REG_HASH_SEC_KEY_0 + index);
		hash_key[index] = val >> 24 & 0xff;
		hash_key[index + 1] = val >> 16 & 0xff;
		hash_key[index + 2] = val >> 8 & 0xff;
		hash_key[index + 3] = val & 0xff;
	}

	return 0;
}

static void mtk_dpmaif_drv_hash_bit_mask_set(struct dpmaif_drv_info *drv_info, u32 mask)
{
	u32 val = mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_REG_HASH_CFG_CON);

	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_REG_HASH_CFG_CON,
		       (val & DPMAIF_HASH_BIT_MASK) | (mask << 8));
}

static void mtk_dpmaif_drv_hash_indir_mask_set(struct dpmaif_drv_info *drv_info, u32 mask)
{
	u32 val = mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_REG_HASH_CFG_CON);

	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_REG_HASH_CFG_CON,
		       (val & DPMAIF_HASH_INDR_MASK) | (mask << 16));
}

static u32 mtk_dpmaif_drv_hash_indir_mask_get(struct dpmaif_drv_info *drv_info)
{
	u32 val = mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_REG_HASH_CFG_CON);

	return (val & (~DPMAIF_HASH_INDR_MASK)) >> 16;
}

static void mtk_dpmaif_drv_hpc_stats_thres_set(struct dpmaif_drv_info *drv_info, u32 thres)
{
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_REG_HPC_STATS_THRES, thres);
}

static void mtk_dpmaif_drv_hpc_stats_time_cfg_set(struct dpmaif_drv_info *drv_info, u32 time_cfg)
{
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_REG_HPC_STATS_TIMER_CFG, time_cfg);
}

static void mtk_dpmaif_drv_init_dl_hpc_hw(struct dpmaif_drv_info *drv_info)
{
	u8 hash_key[DPMAIF_HASH_SEC_KEY_NUM];

	mtk_dpmaif_drv_set_hpc_cntl(drv_info);
	mtk_dpmaif_drv_set_agg_cfg(drv_info);
	mtk_dpmaif_drv_set_hash_bit_choose(drv_info);
	mtk_dpmaif_drv_set_mid_pit_timeout_threshold(drv_info);
	mtk_dpmaif_drv_set_dlq_timeout_threshold(drv_info);
	mtk_dpmaif_drv_set_dlq_start_prs_threshold(drv_info);
	mtk_dpmaif_drv_toeplitz_hash_enable(drv_info, DPMAIF_TOEPLITZ_HASH_EN);
	mtk_dpmaif_drv_hash_default_value_set(drv_info, DPMAIF_HASH_DEFAULT_VALUE);
	get_random_bytes(hash_key, sizeof(hash_key));
	mtk_dpmaif_drv_hash_sec_key_set(drv_info, hash_key);
	mtk_dpmaif_drv_hash_bit_mask_set(drv_info, DPMAIF_HASH_BIT_MASK_DF);
	mtk_dpmaif_drv_hash_indir_mask_set(drv_info, DPMAIF_HASH_INDR_MASK_DF);
	mtk_dpmaif_drv_hpc_stats_thres_set(drv_info, DPMAIF_HPC_STATS_THRESHOLD);
	mtk_dpmaif_drv_hpc_stats_time_cfg_set(drv_info, DPMAIF_HPC_STATS_TIMER_CFG);
}

static void mtk_dpmaif_drv_dl_set_ao_remain_minsz(struct dpmaif_drv_info *drv_info, u32 sz)
{
	u32 val;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_PKTINFO_CONO);
	val &= ~DPMAIF_BAT_REMAIN_MINSZ_MSK;
	val |= ((sz / DPMAIF_BAT_REMAIN_SZ_BASE) << 8) & DPMAIF_BAT_REMAIN_MINSZ_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_PKTINFO_CONO, val);
}

static void mtk_dpmaif_drv_dl_set_ao_bat_bufsz(struct dpmaif_drv_info *drv_info, u32 buf_sz)
{
	u32 val;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_PKTINFO_CON2);
	val &= ~DPMAIF_BAT_BUF_SZ_MSK;
	val |= ((buf_sz / DPMAIF_BAT_BUFFER_SZ_BASE) << 8) & DPMAIF_BAT_BUF_SZ_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_PKTINFO_CON2, val);
}

static void mtk_dpmaif_drv_dl_set_ao_bat_rsv_length(struct dpmaif_drv_info *drv_info, u32 length)
{
	u32 val;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_PKTINFO_CON2);
	val &= ~DPMAIF_BAT_RSV_LEN_MSK;
	val |= length & DPMAIF_BAT_RSV_LEN_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_PKTINFO_CON2, val);
}

static void mtk_dpmaif_drv_dl_set_ao_bid_maxcnt(struct dpmaif_drv_info *drv_info, u32 cnt)
{
	u32 val;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_PKTINFO_CONO);
	val &= ~DPMAIF_BAT_BID_MAXCNT_MSK;
	val |= (cnt << 16) & DPMAIF_BAT_BID_MAXCNT_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_PKTINFO_CONO, val);
}

static void mtk_dpmaif_drv_dl_set_pkt_alignment(struct dpmaif_drv_info *drv_info,
						bool enable, u32 mode)
{
	u32 val;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_RDY_CHK_THRES);
	val &= ~DPMAIF_PKT_ALIGN_MSK;
	if (enable) {
		val |= DPMAIF_PKT_ALIGN_EN;
		val |= (mode << 22) & DPMAIF_PKT_ALIGN_MSK;
	}
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_RDY_CHK_THRES, val);
}

static void mtk_dpmaif_drv_dl_set_pit_seqnum(struct dpmaif_drv_info *drv_info, u32 seq)
{
	u32 val;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_DL_PIT_SEQ_END);
	val &= ~DPMAIF_DL_PIT_SEQ_MSK;
	val |= seq & DPMAIF_DL_PIT_SEQ_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_DL_PIT_SEQ_END, val);
}

static void mtk_dpmaif_drv_dl_set_ao_mtu(struct dpmaif_drv_info *drv_info, u32 mtu_sz)
{
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_PKTINFO_CON1, mtu_sz);
}

static void mtk_dpmaif_drv_dl_set_ao_pit_chknum(struct dpmaif_drv_info *drv_info, u32 number)
{
	u32 val;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_PKTINFO_CON2);
	val &= ~DPMAIF_PIT_CHK_NUM_MSK;
	val |= (number << 24) & DPMAIF_PIT_CHK_NUM_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_PKTINFO_CON2, val);
}

static void mtk_dpmaif_drv_dl_set_ao_bat_check_threshold(struct dpmaif_drv_info *drv_info, u32 size)
{
	u32 val;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_RDY_CHK_THRES);
	val &= ~DPMAIF_BAT_CHECK_THRES_MSK;
	val |= (size << 16) & DPMAIF_BAT_CHECK_THRES_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_RDY_CHK_THRES, val);
}

static void mtk_dpmaif_drv_dl_frg_ao_en(struct dpmaif_drv_info *drv_info, bool enable)
{
	u32 val;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_FRG_CHK_THRES);
	if (enable)
		val |= DPMAIF_FRG_EN_MSK;
	else
		val &= ~DPMAIF_FRG_EN_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_FRG_CHK_THRES, val);
}

static void mtk_dpmaif_drv_dl_set_ao_frg_bufsz(struct dpmaif_drv_info *drv_info, u32 buf_sz)
{
	u32 val;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_FRG_CHK_THRES);
	val &= ~DPMAIF_FRG_BUF_SZ_MSK;
	val |= ((buf_sz / DPMAIF_FRG_BUFFER_SZ_BASE) << 8) & DPMAIF_FRG_BUF_SZ_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_FRG_CHK_THRES, val);
}

static void mtk_dpmaif_drv_dl_set_ao_frg_check_threshold(struct dpmaif_drv_info *drv_info, u32 size)
{
	u32 val;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_FRG_CHK_THRES);
	val &= ~DPMAIF_FRG_CHECK_THRES_MSK;
	val |= size & DPMAIF_FRG_CHECK_THRES_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_FRG_CHK_THRES, val);
}

static void mtk_dpmaif_drv_dl_set_bat_base_addr(struct dpmaif_drv_info *drv_info, u64 addr)
{
	u32 lb_addr = (u32)(addr & 0xFFFFFFFF);
	u32 hb_addr = (u32)(addr >> 32);

	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_INIT_CON0, lb_addr);
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_INIT_CON3, hb_addr);
}

static void mtk_dpmaif_drv_dl_set_bat_size(struct dpmaif_drv_info *drv_info, u32 size)
{
	u32 val;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_INIT_CON1);
	val &= ~DPMAIF_BAT_SIZE_MSK;
	val |= size & DPMAIF_BAT_SIZE_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_INIT_CON1, val);
}

static void mtk_dpmaif_drv_dl_bat_en(struct dpmaif_drv_info *drv_info, bool enable)
{
	u32 val;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_INIT_CON1);
	if (enable)
		val |= DPMAIF_BAT_EN_MSK;
	else
		val &= ~DPMAIF_BAT_EN_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_INIT_CON1, val);
}

static void mtk_dpmaif_drv_dl_bat_init_done(struct dpmaif_drv_info *drv_info, bool frag_en)
{
	u32 cnt = 0, dl_bat_init;

	dl_bat_init = DPMAIF_DL_BAT_INIT_ALLSET;
	dl_bat_init |= DPMAIF_DL_BAT_INIT_EN;

	if (frag_en)
		dl_bat_init |= DPMAIF_DL_BAT_FRG_INIT;

	do {
		if (!(mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_INIT) &
		      DPMAIF_DL_BAT_INIT_NOT_READY)) {
			mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_INIT, dl_bat_init);
			break;
		}
		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to initialize bat.\n");
		return;
	}

	cnt = 0;
	do {
		if (!((mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_INIT) &
		    DPMAIF_DL_BAT_INIT_NOT_READY) == DPMAIF_DL_BAT_INIT_NOT_READY))
			break;

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES)
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to initialize bat done-2\n");
}

static void mtk_dpmaif_drv_dl_set_pit_base_addr(struct dpmaif_drv_info *drv_info, u64 addr)
{
	u32 lb_addr = (u32)(addr & 0xFFFFFFFF);
	u32 hb_addr = (u32)(addr >> 32);

	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DL_LROPIT_INIT_CON0, lb_addr);
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DL_LROPIT_INIT_CON4, hb_addr);
}

static void mtk_dpmaif_drv_dl_set_pit_size(struct dpmaif_drv_info *drv_info, u32 size)
{
	u32 val;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DL_LROPIT_INIT_CON1);
	val &= ~DPMAIF_PIT_SIZE_MSK;
	val |= size & DPMAIF_PIT_SIZE_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DL_LROPIT_INIT_CON1, val);

	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DL_LROPIT_INIT_CON2, 0);
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DL_LROPIT_INIT_CON3, 0);
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DL_LROPIT_INIT_CON5, 0);
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DL_LROPIT_INIT_CON6, 0);
}

static void mtk_dpmaif_drv_dl_pit_en(struct dpmaif_drv_info *drv_info, bool enable)
{
	u32 val;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DL_LROPIT_INIT_CON3);
	if (enable)
		val |= DPMAIF_LROPIT_EN_MSK;
	else
		val &= ~DPMAIF_LROPIT_EN_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DL_LROPIT_INIT_CON3, val);
}

static void mtk_dpmaif_drv_dl_pit_init_done(struct dpmaif_drv_info *drv_info, u32 pit_idx)
{
	int cnt = 0, dl_pit_init;

	dl_pit_init = DPMAIF_DL_PIT_INIT_ALLSET;
	dl_pit_init |= pit_idx << DPMAIF_LROPIT_CHAN_OFS;
	dl_pit_init |= DPMAIF_DL_PIT_INIT_EN;

	do {
		if (!(mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DL_LROPIT_INIT) &
			DPMAIF_DL_PIT_INIT_NOT_READY)) {
			mtk_hw_write32(DRV_TO_MDEV(drv_info),
				       NRL2_DPMAIF_DL_LROPIT_INIT, dl_pit_init);
			break;
		}

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to initialize pit.");
		return;
	}

	cnt = 0;
	do {
		if (!((mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DL_LROPIT_INIT) &
		    DPMAIF_DL_PIT_INIT_NOT_READY) == DPMAIF_DL_PIT_INIT_NOT_READY))
			break;

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to initialize pit done-2\n");
		return;
	}
}

static void mtk_dpmaif_drv_config_dlq_pit_hw(struct dpmaif_drv_info *drv_info, u8 q_num,
					     struct dpmaif_drv_dlq *dlq)
{
	mtk_dpmaif_drv_dl_set_pit_base_addr(drv_info, (u64)dlq->pit_base);
	mtk_dpmaif_drv_dl_set_pit_size(drv_info, dlq->pit_size);
	mtk_dpmaif_drv_dl_pit_en(drv_info, true);
	mtk_dpmaif_drv_dl_pit_init_done(drv_info, q_num);
}

static int mtk_dpmaif_drv_dlq_all_en(struct dpmaif_drv_info *drv_info, bool enable)
{
	u32 val, dl_bat_init, cnt = 0;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_INIT_CON1);

	if (enable)
		val |= DPMAIF_BAT_EN_MSK;
	else
		val &= ~DPMAIF_BAT_EN_MSK;

	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_INIT_CON1, val);
	mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_INIT_CON1);

	dl_bat_init = DPMAIF_DL_BAT_INIT_ONLY_ENABLE_BIT;
	dl_bat_init |= DPMAIF_DL_BAT_INIT_EN;

	/* Update DL bat setting to HW */
	do {
		if (!(mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_INIT) &
			DPMAIF_DL_BAT_INIT_NOT_READY)) {
			mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_INIT, dl_bat_init);
			break;
		}

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to enable all dl queue.\n");
		return -DATA_HW_REG_TIMEOUT;
	}

	/* Wait HW update done */
	cnt = 0;
	do {
		if (!((mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_INIT) &
		    DPMAIF_DL_BAT_INIT_NOT_READY) == DPMAIF_DL_BAT_INIT_NOT_READY))
			break;

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to enable all dl queue-2\n");
		return -DATA_HW_REG_TIMEOUT;
	}

	return 0;
}

static bool mtk_dpmaif_drv_dl_idle_check(struct dpmaif_drv_info *drv_info)
{
	bool is_idle = false;
	u32 dl_dbg_sta;

	dl_dbg_sta = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_DBG_STA1);

	/* If all the queues are idle, DL idle is true. */
	if ((dl_dbg_sta & DPMAIF_DL_IDLE_STS) == DPMAIF_DL_IDLE_STS)
		is_idle = true;

	return is_idle;
}

static u32 mtk_dpmaif_drv_dl_get_wridx(struct dpmaif_drv_info *drv_info)
{
	return ((mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_PIT_STA3)) &
		DPMAIF_DL_PIT_WRIDX_MSK);
}

static u32 mtk_dpmaif_drv_dl_get_pit_ridx(struct dpmaif_drv_info *drv_info)
{
	return ((mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_PIT_STA2)) &
		DPMAIF_DL_PIT_WRIDX_MSK);
}

static void mtk_dpmaif_drv_dl_set_pkt_checksum(struct dpmaif_drv_info *drv_info)
{
	u32 val;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_RDY_CHK_THRES);
	val |= DPMAIF_DL_PKT_CHECKSUM_EN;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_RDY_CHK_THRES, val);
}

static void mtk_dpmaif_drv_dl_clr_pkt_checksum(struct dpmaif_drv_info *drv_info)
{
	u32 val;

	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_RDY_CHK_THRES);
	val &= ~DPMAIF_DL_PKT_CHECKSUM_EN;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_RDY_CHK_THRES, val);
}

static bool mtk_dpmaif_drv_config_dlq_hw(struct dpmaif_drv_info *drv_info)
{
	struct dpmaif_drv_property *drv_property = &drv_info->drv_property;
	struct dpmaif_drv_data_ring *ring = &drv_property->ring;
	struct dpmaif_drv_dlq *dlq;
	u32 i;

	mtk_dpmaif_drv_init_dl_hpc_hw(drv_info);
	mtk_dpmaif_drv_dl_set_ao_remain_minsz(drv_info, ring->normal_bat_remain_size);
	mtk_dpmaif_drv_dl_set_ao_bat_bufsz(drv_info, ring->normal_bat_pkt_bufsz);
	mtk_dpmaif_drv_dl_set_ao_bat_rsv_length(drv_info, ring->normal_bat_rsv_length);
	mtk_dpmaif_drv_dl_set_ao_bid_maxcnt(drv_info, ring->pkt_bid_max_cnt);

	if (ring->pkt_alignment == 64)
		mtk_dpmaif_drv_dl_set_pkt_alignment(drv_info, true, DPMAIF_PKT_ALIGN64_MODE);
	else if (ring->pkt_alignment == 128)
		mtk_dpmaif_drv_dl_set_pkt_alignment(drv_info, true, DPMAIF_PKT_ALIGN128_MODE);
	else
		mtk_dpmaif_drv_dl_set_pkt_alignment(drv_info, false, 0);

	mtk_dpmaif_drv_dl_set_pit_seqnum(drv_info, DPMAIF_PIT_SEQ_MAX);
	mtk_dpmaif_drv_dl_set_ao_mtu(drv_info, ring->mtu);
	mtk_dpmaif_drv_dl_set_ao_pit_chknum(drv_info, ring->chk_pit_num);
	mtk_dpmaif_drv_dl_set_ao_bat_check_threshold(drv_info, ring->chk_normal_bat_num);

	/* Initialize frag bat. */
	if (drv_property->features & DATA_HW_F_FRAG) {
		mtk_dpmaif_drv_dl_frg_ao_en(drv_info, true);
		mtk_dpmaif_drv_dl_set_ao_frg_bufsz(drv_info, ring->frag_bat_pkt_bufsz);
		mtk_dpmaif_drv_dl_set_ao_frg_check_threshold(drv_info, ring->chk_frag_bat_num);
		mtk_dpmaif_drv_dl_set_bat_base_addr(drv_info, (u64)ring->frag_bat_base);
		mtk_dpmaif_drv_dl_set_bat_size(drv_info, ring->frag_bat_size);
		mtk_dpmaif_drv_dl_bat_en(drv_info, true);
		mtk_dpmaif_drv_dl_bat_init_done(drv_info, true);
	}

	/* Initialize normal bat. */
	mtk_dpmaif_drv_dl_set_bat_base_addr(drv_info, (u64)ring->normal_bat_base);
	mtk_dpmaif_drv_dl_set_bat_size(drv_info, ring->normal_bat_size);
	mtk_dpmaif_drv_dl_bat_en(drv_info, false);
	mtk_dpmaif_drv_dl_bat_init_done(drv_info, false);

	/* Initialize pit information. */
	for (i = 0; i < DPMAIF_DLQ_NUM; i++) {
		dlq = &drv_property->dlq[i];
		mtk_dpmaif_drv_config_dlq_pit_hw(drv_info, i, dlq);
	}

	if (mtk_dpmaif_drv_dlq_all_en(drv_info, true))
		return false;
	mtk_dpmaif_drv_dl_set_pkt_checksum(drv_info);
	return true;
}

static void mtk_dpmaif_drv_ul_update_drb_size(struct dpmaif_drv_info *drv_info, u8 q_num, u32 size)
{
	u32 old_size;
	u64 addr;

	addr = DPMAIF_UL_DRBSIZE_ADDRH_N(q_num);

	old_size = mtk_hw_read32(DRV_TO_MDEV(drv_info), addr);
	old_size &= ~DPMAIF_DRB_SIZE_MSK;
	old_size |= size & DPMAIF_DRB_SIZE_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), addr, old_size);
}

static void mtk_dpmaif_drv_ul_update_drb_base_addr(struct dpmaif_drv_info *drv_info,
						   u8 q_num, u64 addr)
{
	u32 lb_addr = (u32)(addr & 0xFFFFFFFF);
	u32 hb_addr = (u32)(addr >> 32);

	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_ULQSAR_N(q_num), lb_addr);
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_UL_DRB_ADDRH_N(q_num), hb_addr);
}

static void mtk_dpmaif_drv_ul_rdy_en(struct dpmaif_drv_info *drv_info, u8 q_num, bool ready)
{
	u32 ul_rdy_en;

	ul_rdy_en = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_UL_CHNL_ARB0);
	if (ready)
		ul_rdy_en |= (1 << q_num);
	else
		ul_rdy_en &= ~(1 << q_num);

	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_UL_CHNL_ARB0, ul_rdy_en);
}

static void mtk_dpmaif_drv_ul_arb_en(struct dpmaif_drv_info *drv_info, u8 q_num, bool enable)
{
	u32 ul_arb_en;

	ul_arb_en = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_UL_CHNL_ARB0);
	if (enable)
		ul_arb_en |= (1 << (q_num + 8));
	else
		ul_arb_en &= ~(1 << (q_num + 8));

	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_UL_CHNL_ARB0, ul_arb_en);
}

static void mtk_dpmaif_drv_config_ulq_hw(struct dpmaif_drv_info *drv_info)
{
	struct dpmaif_drv_ulq *ulq;
	u32 i;

	for (i = 0; i < DPMAIF_ULQ_NUM; i++) {
		ulq = &drv_info->drv_property.ulq[i];
		mtk_dpmaif_drv_ul_update_drb_size(drv_info, i,
						  (ulq->drb_size * DPMAIF_UL_DRB_ENTRY_WORD));
		mtk_dpmaif_drv_ul_update_drb_base_addr(drv_info, i, (u64)ulq->drb_base);
		mtk_dpmaif_drv_ul_rdy_en(drv_info, i, true);
		mtk_dpmaif_drv_ul_arb_en(drv_info, i, true);
	}
}

static bool mtk_dpmaif_drv_init_done(struct dpmaif_drv_info *drv_info)
{
	u32 val, cnt = 0;

	/* Sync default value to SRAM. */
	val = mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AP_MISC_OVERWRITE_CFG);
	val |= DPMAIF_SRAM_SYNC_MASK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AP_MISC_OVERWRITE_CFG, val);
	do {
		if (!(mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AP_MISC_OVERWRITE_CFG) &
		    DPMAIF_SRAM_SYNC_MASK))
			break;

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to sync default value to sram\n");
		return false;
	}

	/* UL configure done. */
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_UL_INIT_SET, DPMAIF_UL_INIT_DONE_MASK);

	/* DL configure done. */
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_DL_INIT_SET, DPMAIF_DL_INIT_DONE_MASK);
	return true;
}

static bool mtk_dpmaif_drv_cfg_hw(struct dpmaif_drv_info *drv_info)
{
	mtk_dpmaif_drv_init_common_hw(drv_info);
	if (!mtk_dpmaif_drv_config_dlq_hw(drv_info))
		return false;
	mtk_dpmaif_drv_config_ulq_hw(drv_info);
	if (!mtk_dpmaif_drv_init_done(drv_info))
		return false;

	drv_info->ulq_all_enable = true;
	drv_info->dlq_all_enable = true;

	return true;
}

static void mtk_dpmaif_drv_clr_ul_all_intr(struct dpmaif_drv_info *drv_info)
{
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TISAR0, 0xFFFFFFFF);
}

static void mtk_dpmaif_drv_clr_dl_all_intr(struct dpmaif_drv_info *drv_info)
{
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TISAR0, 0xFFFFFFFF);
}

static int mtk_dpmaif_drv_init_t800(struct dpmaif_drv_info *drv_info, void *data)
{
	struct dpmaif_drv_cfg *drv_cfg = data;

	if (!drv_cfg) {
		MTK_ERR(DRV_TO_MDEV(drv_info), "Invalid parameter\n");
		return -DATA_FLOW_CHK_ERR;
	}

	/* Initialize port mode and clock. */
	if (!mtk_dpmaif_drv_config(drv_info))
		return DATA_HW_REG_CHK_FAIL;

	/* Initialize dpmaif interrupt. */
	if (!mtk_dpmaif_drv_init_intr(drv_info))
		return DATA_HW_REG_CHK_FAIL;

	/* Get initialization information from trans layer. */
	mtk_dpmaif_drv_set_property(drv_info, drv_cfg);

	/* Configure HW queue setting. */
	if (!mtk_dpmaif_drv_cfg_hw(drv_info))
		return DATA_HW_REG_CHK_FAIL;

	/* Clear all interrupt status. */
	mtk_dpmaif_drv_clr_ul_all_intr(drv_info);
	mtk_dpmaif_drv_clr_dl_all_intr(drv_info);

	return 0;
}

static int mtk_dpmaif_drv_ulq_all_en(struct dpmaif_drv_info *drv_info, bool enable)
{
	u32 ul_arb_en;

	ul_arb_en = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_UL_CHNL_ARB0);
	if (enable)
		ul_arb_en |= DPMAIF_UL_ALL_QUE_ARB_EN;
	else
		ul_arb_en &= ~DPMAIF_UL_ALL_QUE_ARB_EN;

	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_UL_CHNL_ARB0, ul_arb_en);
	mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_UL_CHNL_ARB0);

	return 0;
}

static bool mtk_dpmaif_drv_ul_all_idle_check(struct dpmaif_drv_info *drv_info)
{
	bool is_idle = false;
	u32 ul_dbg_sta;

	ul_dbg_sta = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_UL_DBG_STA2);
	/* If all the queues are idle, UL idle is true. */
	if ((ul_dbg_sta & DPMAIF_UL_IDLE_STS_MSK) == DPMAIF_UL_IDLE_STS)
		is_idle = true;

	return is_idle;
}

static int mtk_dpmaif_drv_unmask_ulq_intr(struct dpmaif_drv_info *drv_info, u32 q_num)
{
	u32 ui_que_done_mask;

	ui_que_done_mask = (1 << (q_num + DP_UL_INT_DONE_OFFSET)) & DPMAIF_UL_INT_QDONE_MSK;
	drv_info->drv_irq_en_mask.ap_ul_l2intr_en_mask |= ui_que_done_mask;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TICR0, ui_que_done_mask);

	return 0;
}

static int mtk_dpmaif_drv_ul_unmask_all_tx_done_intr(struct dpmaif_drv_info *drv_info)
{
	int ret;
	u8 i;

	for (i = 0; i < DPMAIF_ULQ_NUM; i++) {
		ret = mtk_dpmaif_drv_unmask_ulq_intr(drv_info, i);
		if (ret < 0)
			break;
	}

	return ret;
}

static u32 mtk_dpmaif_drv_get_dl_intr_mask(struct dpmaif_drv_info *drv_info)
{
	return mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TIMR0);
}

static int mtk_dpmaif_drv_dl_unmask_rx_done_intr(struct dpmaif_drv_info *drv_info, u8 qno)
{
	u32 di_que_done_mask;

	if (qno == DPMAIF_DLQ0)
		di_que_done_mask = DPMAIF_DL_INT_DLQ0_QDONE_MSK;
	else
		di_que_done_mask = DPMAIF_DL_INT_DLQ1_QDONE_MSK;

	drv_info->drv_irq_en_mask.ap_dl_l2intr_en_mask |= di_que_done_mask;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TICR0, di_que_done_mask);

	mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TICR0);

	return 0;
}

static int mtk_dpmaif_drv_dl_unmask_all_rx_done_intr(struct dpmaif_drv_info *drv_info)
{
	int ret;
	u8 i;

	for (i = 0; i < DPMAIF_DLQ_NUM; i++) {
		ret = mtk_dpmaif_drv_dl_unmask_rx_done_intr(drv_info, i);
		if (ret < 0)
			break;
	}

	return ret;
}

static int mtk_dpmaif_drv_dlq_mask_rx_done_intr(struct dpmaif_drv_info *drv_info, u8 qno)
{
	u32 cnt = 0, di_que_done_mask;

	if (qno == DPMAIF_DLQ0)
		di_que_done_mask = DPMAIF_DL_INT_DLQ0_QDONE_MSK;
	else
		di_que_done_mask = DPMAIF_DL_INT_DLQ1_QDONE_MSK;
	//mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TISR0, di_que_done_mask);
	//mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TISR0);

	///* Check mask status. */
	/* mask interrupt and check mask status. */
	do {
		if (!(cnt++ % REWRITE_INTERVAL)) {
			mtk_hw_write32(DRV_TO_MDEV(drv_info),
				       DPMAIF_PD_AP_DL_L2TISR0, di_que_done_mask);
			mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TISR0);
		}

		if (!((mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TIMR0) &
		    di_que_done_mask) != di_que_done_mask))
			break;

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to mask dlq%u interrupt :0x%08x\n",
			qno, mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TIMR0));
		if (mtk_hw_mmio_check(DRV_TO_MDEV(drv_info))) {
			MTK_INFO(DRV_TO_MDEV(drv_info), "Failed to access mmio\n");
			mtk_except_report_evt(DRV_TO_MDEV(drv_info), EXCEPT_LINK_ERR);
		} else {
			mtk_dpmaif_drv_dump_t800(drv_info);
			mtk_hw_send_ext_evt(DRV_TO_MDEV(drv_info), EXT_EVT_H2D_RESERVED_FOR_DPMAIF);
		}
		//MTK_BUG(DRV_TO_MDEV(drv_info));
		return -DATA_HW_REG_TIMEOUT;
	}

	drv_info->drv_irq_en_mask.ap_dl_l2intr_en_mask &= ~di_que_done_mask;

	return 0;
}

static int mtk_dpmaif_drv_dl_mask_all_rx_done_intr(struct dpmaif_drv_info *drv_info)
{
	int ret;
	u8 i;

	for (i = 0; i < DPMAIF_DLQ_NUM; i++) {
		ret = mtk_dpmaif_drv_dlq_mask_rx_done_intr(drv_info, i);
		if (ret < 0)
			break;
	}

	return ret;
}

static void mtk_dpmaif_drv_mask_dl_batcnt_len_err_intr(struct dpmaif_drv_info *drv_info, u32 q_num)
{
	drv_info->drv_irq_en_mask.ap_dl_l2intr_en_mask &= ~DPMAIF_DL_INT_BATCNT_LEN_ERR_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TISR0,
		       DPMAIF_DL_INT_BATCNT_LEN_ERR_MSK);
	mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TISR0);
}

static void mtk_dpmaif_drv_unmask_dl_batcnt_len_err_intr(struct dpmaif_drv_info *drv_info)
{
	drv_info->drv_irq_en_mask.ap_dl_l2intr_en_mask |= DPMAIF_DL_INT_BATCNT_LEN_ERR_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TICR0,
		       DPMAIF_DL_INT_BATCNT_LEN_ERR_MSK);
}

static int mtk_dpmaif_drv_mask_dl_frgcnt_len_err_intr(struct dpmaif_drv_info *drv_info, u32 q_num)
{
	drv_info->drv_irq_en_mask.ap_dl_l2intr_en_mask &= ~DPMAIF_DL_INT_FRG_LEN_ERR_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TISR0,
		       DPMAIF_DL_INT_FRG_LEN_ERR_MSK);
	mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TISR0);

	return 0;
}

static void mtk_dpmaif_drv_unmask_dl_frgcnt_len_err_intr(struct dpmaif_drv_info *drv_info)
{
	drv_info->drv_irq_en_mask.ap_dl_l2intr_en_mask |= DPMAIF_DL_INT_FRG_LEN_ERR_MSK;
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TICR0,
		       DPMAIF_DL_INT_FRG_LEN_ERR_MSK);
}

static int mtk_dpmaif_drv_dlq_mask_pit_cnt_len_err_intr(struct dpmaif_drv_info *drv_info, u8 qno)
{
	if (qno == DPMAIF_DLQ0)
		mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_UL_APDL_L2TIMSR0,
			       DPMAIF_DL_INT_DLQ0_PITCNT_LEN_ERR_MSK);
	else
		mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_UL_APDL_L2TIMSR0,
			       DPMAIF_DL_INT_DLQ1_PITCNT_LEN_ERR_MSK);

	mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_UL_APDL_L2TIMSR0);

	return 0;
}

static int mtk_dpmaif_drv_dlq_unmask_pit_cnt_len_err_intr(struct dpmaif_drv_info *drv_info, u8 qno)
{
	if (qno == DPMAIF_DLQ0)
		mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_UL_APDL_L2TIMCR0,
			       DPMAIF_DL_INT_DLQ0_PITCNT_LEN_ERR_MSK);
	else
		mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_UL_APDL_L2TIMCR0,
			       DPMAIF_DL_INT_DLQ1_PITCNT_LEN_ERR_MSK);

	return 0;
}

static int mtk_dpmaif_drv_start_queue_t800(struct dpmaif_drv_info *drv_info,
					   enum dpmaif_drv_dir dir)
{
	int ret;

	if (dir == DPMAIF_TX) {
		if (unlikely(drv_info->ulq_all_enable)) {
			MTK_DBG(DRV_TO_MDEV(drv_info),
				MTK_DBG_DPMF, MTK_MEMLOG_RG_0, "ulq all enabled\n");
			return 0;
		}

		ret = mtk_dpmaif_drv_ulq_all_en(drv_info, true);
		if (ret < 0)
			return ret;

		ret = mtk_dpmaif_drv_ul_unmask_all_tx_done_intr(drv_info);
		if (ret < 0)
			return ret;

		drv_info->ulq_all_enable = true;
	} else {
		if (unlikely(drv_info->dlq_all_enable)) {
			MTK_DBG(DRV_TO_MDEV(drv_info),
				MTK_DBG_DPMF, MTK_MEMLOG_RG_0, "dlq all enabled\n");
			return 0;
		}

		MTK_INFO(DRV_TO_MDEV(drv_info), "before enable bat_status=0x%x\n",
			 mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_DL_BAT_STA1));
		ret = mtk_dpmaif_drv_dlq_all_en(drv_info, true);
		if (ret < 0)
			return ret;

		ret = mtk_dpmaif_drv_dl_unmask_all_rx_done_intr(drv_info);
		if (ret < 0)
			return ret;

		drv_info->dlq_all_enable = true;
	}
	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_UL_RESERVE_AO_RW,
		       DPMAIF_PCIE_MODE_RESUME_VALUE);
	MTK_INFO(DRV_TO_MDEV(drv_info), "Send message=0x%x\n", DPMAIF_PCIE_MODE_RESUME_VALUE);

	return 0;
}

static int mtk_dpmaif_drv_stop_ulq(struct dpmaif_drv_info *drv_info)
{
	int cnt = 0;

	/* Disable HW arb and check idle. */
	mtk_dpmaif_drv_ulq_all_en(drv_info, false);
	do {
		if (++cnt >= POLL_MAX_TIMES) {
			extern void t800_send_platform_uevent(int id, int sub_id, int value1, int value2, int value3);
			t800_send_platform_uevent(5/*MTK_UEVENT_MODEM_MONITOR*/, 8/*MODEM_MONITOR_865CREASON*/, 3, 0, 0);
			MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to stop ul queue, 0x%x\n",
				mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_UL_DBG_STA2));
			return -DATA_HW_REG_TIMEOUT;
		}
		udelay(POLL_INTERVAL_US);
	} while (!mtk_dpmaif_drv_ul_all_idle_check(drv_info));

	return 0;
}

static int mtk_dpmaif_drv_mask_ulq_intr(struct dpmaif_drv_info *drv_info, u32 q_num)
{
	u32 cnt = 0, ui_que_done_mask;

	ui_que_done_mask = (1 << (q_num + DP_UL_INT_DONE_OFFSET)) & DPMAIF_UL_INT_QDONE_MSK;

	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TISR0, ui_que_done_mask);
	mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TISR0);

	do {
		if (!((mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TIMR0) &
		    ui_que_done_mask) != ui_que_done_mask))
			break;

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to mask ul%u interrupt done-0x%08x\n", q_num,
			mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TIMR0));
		MTK_BUG(DRV_TO_MDEV(drv_info));
		return -DATA_HW_REG_TIMEOUT;
	}
	drv_info->drv_irq_en_mask.ap_ul_l2intr_en_mask &= ~ui_que_done_mask;

	return 0;
}

static void mtk_dpmaif_drv_ul_mask_multi_tx_done_intr(struct dpmaif_drv_info *drv_info, u8 q_mask)
{
	u32 i;

	for (i = 0; i < DPMAIF_ULQ_NUM; i++) {
		if (q_mask & (1 << i)) {
			mtk_dpmaif_drv_mask_ulq_intr(drv_info, i);
			mtk_pm_ds_try_lock(DRV_TO_MDEV(drv_info), MTK_USER_DPMAIF);
		}
	}
}

static int mtk_dpmaif_drv_ul_mask_all_tx_done_intr(struct dpmaif_drv_info *drv_info)
{
	int ret;
	u8 i;

	for (i = 0; i < DPMAIF_ULQ_NUM; i++) {
		ret = mtk_dpmaif_drv_mask_ulq_intr(drv_info, i);
		if (ret < 0)
			break;
	}

	return ret;
}

static int mtk_dpmaif_drv_stop_dlq(struct dpmaif_drv_info *drv_info)
{
	u32 cnt = 0, wridx, ridx;

	/* Disable HW arb and check idle. */
	mtk_dpmaif_drv_dlq_all_en(drv_info, false);
	do {
		if (++cnt >= (POLL_MAX_TIMES * 10)) {
			extern void t800_send_platform_uevent(int id, int sub_id, int value1, int value2, int value3);
			t800_send_platform_uevent(5/*MTK_UEVENT_MODEM_MONITOR*/, 8/*MODEM_MONITOR_865CREASON*/, 4, 0, 0);
			// MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to stop dl queue, 0x%x\n",
			// 	mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_DBG_STA1));
			MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to stop dl queue, 0x%x, 0x%x\n",
				mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_DBG_STA1),
				mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_DL_BAT_STA1));
			
			if (cnt % 200 == 0)
				MTK_INFO(DRV_TO_MDEV(drv_info), "dl queue is not idle\n");
			return -DATA_HW_REG_TIMEOUT;
		}
		udelay(POLL_INTERVAL_US);
	} while (!mtk_dpmaif_drv_dl_idle_check(drv_info));

	/* Check middle pit sync done. */
	cnt = 0;
	do {
		wridx = mtk_dpmaif_drv_dl_get_wridx(drv_info);
		ridx = mtk_dpmaif_drv_dl_get_pit_ridx(drv_info);
		if (wridx == ridx)
			break;

		cnt++;
		udelay(POLL_INTERVAL_US);
		if(cnt % 100 == 0) {
			MTK_INFO(DRV_TO_MDEV(drv_info), "pit wridx:%d ridx:%d\n", wridx, ridx);
		}
	} while (cnt < POLL_MAX_TIMES * 10);

	if (cnt >= POLL_MAX_TIMES) {
		extern void t800_send_platform_uevent(int id, int sub_id, int value1, int value2, int value3);
		t800_send_platform_uevent(5/*MTK_UEVENT_MODEM_MONITOR*/, 8/*MODEM_MONITOR_865CREASON*/, 1, 0, 0);
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to check middle pit sync\n");
		return -DATA_HW_REG_TIMEOUT;
	}

	return 0;
}

static u32 mtk_dpmaif_drv_get_dl_lv2_sts(struct dpmaif_drv_info *drv_info)
{
	return mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TISAR0);
}

static u32 mtk_dpmaif_drv_get_ul_lv2_sts(struct dpmaif_drv_info *drv_info)
{
	return mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TISAR0);
}

static u32 mtk_dpmaif_drv_get_ul_intr_mask(struct dpmaif_drv_info *drv_info)
{
	return mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TIMR0);
}

static bool mtk_dpmaif_drv_check_clr_ul_done_status(struct dpmaif_drv_info *drv_info, u8 qno)
{
	u32 val, l2tisar0;
	bool ret = false;
	/* get TX interrupt status. */
	l2tisar0 = mtk_dpmaif_drv_get_ul_lv2_sts(drv_info);
	val = l2tisar0 & DPMAIF_UL_INT_QDONE  & (1 << (DP_UL_INT_DONE_OFFSET + qno));

	/* ulq status. */
	if (val) {
		/* clear ulq done status */
		mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TISAR0, val);
		ret = true;
	}

	return ret;
}

static u32 mtk_dpmaif_drv_irq_src0_dl_filter(struct dpmaif_drv_info *drv_info, u32 l2risar0,
					     u32 l2rimr0)
{
	if (l2rimr0 & DPMAIF_DL_INT_DLQ0_QDONE_MSK)
		l2risar0 &= ~DPMAIF_DL_INT_DLQ0_QDONE;

	if (l2rimr0 & DPMAIF_DL_INT_DLQ0_PITCNT_LEN_ERR_MSK)
		l2risar0 &= ~DPMAIF_DL_INT_DLQ0_PITCNT_LEN_ERR;

	if (l2rimr0 & DPMAIF_DL_INT_FRG_LEN_ERR_MSK)
		l2risar0 &= ~DPMAIF_DL_INT_FRG_LEN_ERR;

	if (l2rimr0 & DPMAIF_DL_INT_BATCNT_LEN_ERR_MSK)
		l2risar0 &= ~DPMAIF_DL_INT_BATCNT_LEN_ERR;

	return l2risar0;
}

static u32 mtk_dpmaif_drv_irq_src1_dl_filter(struct dpmaif_drv_info *drv_info, u32 l2risar0,
					     u32 l2rimr0)
{
	if (l2rimr0 & DPMAIF_DL_INT_DLQ1_QDONE_MSK)
		l2risar0 &= ~DPMAIF_DL_INT_DLQ1_QDONE;

	if (l2rimr0 & DPMAIF_DL_INT_DLQ1_PITCNT_LEN_ERR_MSK)
		l2risar0 &= ~DPMAIF_DL_INT_DLQ1_PITCNT_LEN_ERR;

	return l2risar0;
}

static int mtk_dpmaif_drv_stop_queue_t800(struct dpmaif_drv_info *drv_info, enum dpmaif_drv_dir dir)
{
	u32 dl_status, dl_mask;
	u32 ul_status, ul_mask;
	int ret;

	mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_UL_RESERVE_AO_RW,
		       DPMAIF_PCIE_MODE_SUSPEND_VALUE);
	MTK_INFO(DRV_TO_MDEV(drv_info), "Send message=0x%x\n", DPMAIF_PCIE_MODE_SUSPEND_VALUE);

	if (dir == DPMAIF_TX) {
		ul_status = mtk_dpmaif_drv_get_ul_lv2_sts(drv_info);
		ul_mask = mtk_dpmaif_drv_get_ul_intr_mask(drv_info);
		MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_0,
			"ul interrupt:0x%08x,0x%08x\n", ul_status, ul_mask);

		if (unlikely(!drv_info->ulq_all_enable)) {
			MTK_DBG(DRV_TO_MDEV(drv_info),
				MTK_DBG_DPMF, MTK_MEMLOG_RG_0, "ulq all disabled\n");
			return 0;
		}

		ret = mtk_dpmaif_drv_stop_ulq(drv_info);
		if (ret < 0)
			return ret;

		ret = mtk_dpmaif_drv_ul_mask_all_tx_done_intr(drv_info);
		if (ret < 0)
			return ret;

		drv_info->ulq_all_enable = false;
	} else {
		dl_status = mtk_dpmaif_drv_get_dl_lv2_sts(drv_info);
		dl_mask = mtk_dpmaif_drv_get_dl_intr_mask(drv_info);
		MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_0,
			"dl interrupt:0x%08x,0x%08x\n", dl_status, dl_mask);

		if (unlikely(!drv_info->dlq_all_enable)) {
			MTK_DBG(DRV_TO_MDEV(drv_info),
				MTK_DBG_DPMF, MTK_MEMLOG_RG_0, "dlq all disabled\n");
			return 0;
		}

		ret = mtk_dpmaif_drv_stop_dlq(drv_info);
		if (ret < 0)
			return ret;

		ret = mtk_dpmaif_drv_dl_mask_all_rx_done_intr(drv_info);
		if (ret < 0)
			return ret;

		drv_info->dlq_all_enable = false;
	}

	return 0;
}

static int mtk_dpmaif_drv_clr_ip_busy_sts(struct dpmaif_drv_info *drv_info)
{
	u32 ip_busy_sts;

	/* Get AP IP busy status. */
	ip_busy_sts = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_IP_BUSY);

	/* Clear AP IP busy. */
	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_IP_BUSY, ip_busy_sts);

	return 0;
}

static int mtk_dpmaif_drv_irq_src0(struct dpmaif_drv_info *drv_info,
				   struct dpmaif_drv_intr_info *intr_info)
{
	u32 val, ori_l2risar0, l2risar0, l2rimr0;

	ori_l2risar0 = mtk_dpmaif_drv_get_dl_lv2_sts(drv_info);
	l2rimr0 = mtk_dpmaif_drv_get_dl_intr_mask(drv_info);

	l2risar0 = ori_l2risar0 & DPMAIF_SRC0_DL_STATUS_MASK;
	if (l2risar0) {
		/* Filter to get DL unmasked interrupts */
		l2risar0 = mtk_dpmaif_drv_irq_src0_dl_filter(drv_info, l2risar0, l2rimr0);

		val = l2risar0 & DPMAIF_DL_INT_BATCNT_LEN_ERR;
		if (val) {
			intr_info->intr_types[intr_info->intr_cnt] = DPMAIF_INTR_DL_BATCNT_LEN_ERR;
			intr_info->intr_queues[intr_info->intr_cnt] = DPMAIF_DLQ0;
			intr_info->intr_cnt++;
			mtk_dpmaif_drv_mask_dl_batcnt_len_err_intr(drv_info, DPMAIF_DLQ0);
		}

		val = l2risar0 & DPMAIF_DL_INT_FRG_LEN_ERR;
		if (val) {
			intr_info->intr_types[intr_info->intr_cnt] = DPMAIF_INTR_DL_FRGCNT_LEN_ERR;
			intr_info->intr_queues[intr_info->intr_cnt] = DPMAIF_DLQ0;
			intr_info->intr_cnt++;
			mtk_dpmaif_drv_mask_dl_frgcnt_len_err_intr(drv_info, DPMAIF_DLQ0);
		}

		val = l2risar0 & DPMAIF_DL_INT_DLQ0_PITCNT_LEN_ERR;
		if (val) {
			intr_info->intr_types[intr_info->intr_cnt] = DPMAIF_INTR_DL_PITCNT_LEN_ERR;
			intr_info->intr_queues[intr_info->intr_cnt] = 0x01 << DPMAIF_DLQ0;
			intr_info->intr_cnt++;
			mtk_dpmaif_drv_dlq_mask_pit_cnt_len_err_intr(drv_info, DPMAIF_DLQ0);
		}

		val = l2risar0 & DPMAIF_DL_INT_DLQ0_QDONE;
		if (val) {
			if (!mtk_dpmaif_drv_dlq_mask_rx_done_intr(drv_info, DPMAIF_DLQ0)) {
				mtk_pm_ds_try_lock(DRV_TO_MDEV(drv_info), MTK_USER_DPMAIF);
				intr_info->intr_types[intr_info->intr_cnt] = DPMAIF_INTR_DL_DONE;
				intr_info->intr_queues[intr_info->intr_cnt] = 0x01 << DPMAIF_DLQ0;
				intr_info->intr_cnt++;
			}
		}

		/* Clear interrupt status. */
		mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TISAR0, l2risar0);
	}
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DATA_IRQ, MTK_DATA_IRQ_MEMLOG_RG(0),
		"src0:0x%08x,0x%08x,0x%08x\n", ori_l2risar0, l2risar0, l2rimr0);

	return 0;
}

static int mtk_dpmaif_drv_irq_src1(struct dpmaif_drv_info *drv_info,
				   struct dpmaif_drv_intr_info *intr_info)
{
	u32 val, ori_l2risar0, l2risar0, l2rimr0;

	ori_l2risar0 = mtk_dpmaif_drv_get_dl_lv2_sts(drv_info);
	l2rimr0 = mtk_dpmaif_drv_get_dl_intr_mask(drv_info);

	/* Check and process interrupt. */
	l2risar0 = ori_l2risar0 & DPMAIF_SRC1_DL_STATUS_MASK;
	if (l2risar0) {
		/* Filter to get DL unmasked interrupts */
		l2risar0 = mtk_dpmaif_drv_irq_src1_dl_filter(drv_info, l2risar0, l2rimr0);

		val = l2risar0 & DPMAIF_DL_INT_DLQ1_PITCNT_LEN_ERR;
		if (val) {
			intr_info->intr_types[intr_info->intr_cnt] = DPMAIF_INTR_DL_PITCNT_LEN_ERR;
			intr_info->intr_queues[intr_info->intr_cnt] = 0x01 << DPMAIF_DLQ1;
			intr_info->intr_cnt++;
			mtk_dpmaif_drv_dlq_mask_pit_cnt_len_err_intr(drv_info, DPMAIF_DLQ1);
		}

		val = l2risar0 & DPMAIF_DL_INT_DLQ1_QDONE;
		if (val) {
			if (!mtk_dpmaif_drv_dlq_mask_rx_done_intr(drv_info, DPMAIF_DLQ1)) {
				mtk_pm_ds_try_lock(DRV_TO_MDEV(drv_info), MTK_USER_DPMAIF);
				intr_info->intr_types[intr_info->intr_cnt] = DPMAIF_INTR_DL_DONE;
				intr_info->intr_queues[intr_info->intr_cnt] = 0x01 << DPMAIF_DLQ1;
				intr_info->intr_cnt++;
			}
		}

		/* Clear interrupt status. */
		mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_DL_L2TISAR0, l2risar0);
	}
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DATA_IRQ, MTK_DATA_IRQ_MEMLOG_RG(1),
		"src1:0x%08x,0x%08x,0x%08x\n", ori_l2risar0, l2risar0, l2rimr0);

	return 0;
}

static void mtk_dpmaif_drv_ul_mask_intr(struct dpmaif_drv_info *drv_info, u32 mask)
{
	u32 cnt = 0;

	mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TISR0, mask);
	mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TISR0);

	while ((mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TIMR0) &
		mask) != mask) {
		if (++cnt >= POLL_MAX_TIMES) {
			MTK_ERR(DRV_TO_MDEV(drv_info),
				"Failed to mask interrupt done-0x%08x\n",
				mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TIMR0));
			MTK_BUG(DRV_TO_MDEV(drv_info));
			return;
		}
		udelay(POLL_INTERVAL_US);
	}
}

static int mtk_dpmaif_drv_irq_src2(struct dpmaif_drv_info *drv_info,
				   struct dpmaif_drv_intr_info *intr_info)
{
	u32 ori_l2tisar0, l2tisar0, l2timr0;
	u8 q_mask;
	u32 val;
	static u32 cnt;

	ori_l2tisar0 = mtk_dpmaif_drv_get_ul_lv2_sts(drv_info);
	l2timr0 = mtk_dpmaif_drv_get_ul_intr_mask(drv_info);

	/* Check and process interrupt. */
	l2tisar0 = ori_l2tisar0 & (~l2timr0);
	if (l2tisar0) {
		cnt = 0;
		val = l2tisar0 & DPMAIF_UL_INT_QDONE;
		if (val) {
			q_mask = val >> DP_UL_INT_DONE_OFFSET & DPMAIF_ULQS;
			mtk_dpmaif_drv_ul_mask_multi_tx_done_intr(drv_info, q_mask);
			intr_info->intr_types[intr_info->intr_cnt] = DPMAIF_INTR_UL_DONE;
			intr_info->intr_queues[intr_info->intr_cnt] = val >> DP_UL_INT_DONE_OFFSET;
			intr_info->intr_cnt++;
		} else {
			mtk_dpmaif_drv_ul_mask_intr(drv_info, l2tisar0);
		}

		/* clear interrupt status */
		mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_AP_UL_L2TISAR0, l2tisar0);
	}
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DATA_IRQ, MTK_DATA_IRQ_MEMLOG_RG(2),
		"src2:0x%08x,0x%08x,0x%08x\n", ori_l2tisar0, l2tisar0, l2timr0);

	return 0;
}

static int mtk_dpmaif_drv_intr_handle_t800(struct dpmaif_drv_info *drv_info, void *data, u8 irq_id)
{
	switch (irq_id) {
	case MTK_IRQ_SRC_DPMAIF:
		mtk_dpmaif_drv_irq_src0(drv_info, data);
		break;
	case MTK_IRQ_SRC_DPMAIF2:
		mtk_dpmaif_drv_irq_src1(drv_info, data);
		break;
	case MTK_IRQ_SRC_DPMAIF3:
		mtk_dpmaif_drv_irq_src2(drv_info, data);
		break;
	default:
		break;
	}

	mtk_dpmaif_drv_clr_ip_busy_sts(drv_info);

	return 0;
}

static int mtk_dpmaif_drv_intr_complete_t800(struct dpmaif_drv_info *drv_info,
					     enum dpmaif_drv_intr_type type, u8 q_id, u64 data)
{
	int ret = 0;

	switch (type) {
	case DPMAIF_INTR_UL_DONE:
		if (data == DPMAIF_CLEAR_INTR) {
			mtk_dpmaif_drv_check_clr_ul_done_status(drv_info, q_id);
		} else {
			ret = mtk_dpmaif_drv_unmask_ulq_intr(drv_info, q_id);
			mtk_pm_ds_unlock(DRV_TO_MDEV(drv_info), MTK_USER_DPMAIF, false);
		}
		break;
	case DPMAIF_INTR_DL_BATCNT_LEN_ERR:
		mtk_dpmaif_drv_unmask_dl_batcnt_len_err_intr(drv_info);
		break;
	case DPMAIF_INTR_DL_FRGCNT_LEN_ERR:
		mtk_dpmaif_drv_unmask_dl_frgcnt_len_err_intr(drv_info);
		break;
	case DPMAIF_INTR_DL_PITCNT_LEN_ERR:
		ret = mtk_dpmaif_drv_dlq_unmask_pit_cnt_len_err_intr(drv_info, q_id);
		break;
	case DPMAIF_INTR_DL_DONE:
		ret = mtk_dpmaif_drv_dl_unmask_rx_done_intr(drv_info, q_id);
		mtk_pm_ds_unlock(DRV_TO_MDEV(drv_info), MTK_USER_DPMAIF, false);
		break;
	default:
		break;
	}

	return ret;
}

static int mtk_dpmaif_drv_dl_add_pit_cnt(struct dpmaif_drv_info *drv_info,
					 u32 qno, u32 pit_remain_cnt)
{
	u32 cnt = 0, dl_update;

	dl_update = pit_remain_cnt & 0x0003ffff;
	dl_update |= DPMAIF_DL_ADD_UPDATE | (qno << DPMAIF_ADD_LRO_PIT_CHAN_OFS);

	do {
		if ((mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DL_LROPIT_ADD) &
			DPMAIF_DL_ADD_NOT_READY) == 0) {
			mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DL_LROPIT_ADD, dl_update);
			break;
		}

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info),
			"Failed to add dlq%u pit-1, cnt=%u\n", qno, pit_remain_cnt);
		return -DATA_HW_REG_TIMEOUT;
	}

	cnt = 0;
	do {
		if (!((mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DL_LROPIT_ADD) &
		     DPMAIF_DL_ADD_NOT_READY) == DPMAIF_DL_ADD_NOT_READY))
			break;

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to add dlq%u pit-2, cnt=%u\n",
			qno, pit_remain_cnt);
		return -DATA_HW_REG_TIMEOUT;
	}

	return 0;
}

static int mtk_dpmaif_drv_dl_add_bat_cnt(struct dpmaif_drv_info *drv_info, u32 bat_entry_cnt)
{
	u32 cnt = 0, dl_bat_update;

	dl_bat_update = bat_entry_cnt & 0xffff;
	dl_bat_update |= DPMAIF_DL_ADD_UPDATE;
	do {
		if ((mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_ADD) &
			DPMAIF_DL_ADD_NOT_READY) == 0) {
			mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_ADD, dl_bat_update);
			break;
		}

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info),
			"Failed to add bat-1, cnt=%u\n", bat_entry_cnt);
		return -DATA_HW_REG_TIMEOUT;
	}

	cnt = 0;
	do {
		if (!((mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_ADD) &
		       DPMAIF_DL_ADD_NOT_READY) == DPMAIF_DL_ADD_NOT_READY))
			break;

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to add bat-2, cnt=%u\n",
			bat_entry_cnt);
		return -DATA_HW_REG_TIMEOUT;
	}
	return 0;
}

static int mtk_dpmaif_drv_dl_add_frg_cnt(struct dpmaif_drv_info *drv_info, u32 frg_entry_cnt)
{
	u32 cnt = 0, dl_frg_update;
	int ret = 0;

	dl_frg_update = frg_entry_cnt & 0xffff;
	dl_frg_update |= DPMAIF_DL_FRG_ADD_UPDATE;
	dl_frg_update |= DPMAIF_DL_ADD_UPDATE;

	do {
		if (!(mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_ADD)
		     & DPMAIF_DL_ADD_NOT_READY)) {
			mtk_hw_write32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_ADD, dl_frg_update);
			break;
		}

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to add frag bat-1, cnt=%u\n",
			frg_entry_cnt);
		return -DATA_HW_REG_TIMEOUT;
	}

	cnt = 0;
	do {
		if (!((mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_PD_DL_BAT_ADD) &
		       DPMAIF_DL_ADD_NOT_READY) == DPMAIF_DL_ADD_NOT_READY))
			break;

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to add frag bat-2, cnt=%u\n",
			frg_entry_cnt);
		return -DATA_HW_REG_TIMEOUT;
	}
	return ret;
}

static int mtk_dpmaif_drv_ul_add_drb(struct dpmaif_drv_info *drv_info, u8 q_num, u32 drb_cnt)
{
	u32 drb_entry_cnt = drb_cnt * DPMAIF_UL_DRB_ENTRY_WORD;
	u32 cnt = 0, ul_update;
	u64 addr;

	ul_update = drb_entry_cnt & 0x0000ffff;
	ul_update |= DPMAIF_UL_ADD_UPDATE;

	if (q_num == 4)
		addr = NRL2_DPMAIF_UL_ADD_DESC_CH4;
	else
		addr = DPMAIF_ULQ_ADD_DESC_CH_N(q_num);

	do {
		if (!(mtk_hw_read32(DRV_TO_MDEV(drv_info), addr) & DPMAIF_UL_ADD_NOT_READY)) {
			mtk_hw_write32(DRV_TO_MDEV(drv_info), addr, ul_update);
			break;
		}

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to add ulq%u drb-1, cnt=%u\n",
			q_num, drb_cnt);
		return -DATA_HW_REG_TIMEOUT;
	}

	cnt = 0;
	do {
		if (!((mtk_hw_read32(DRV_TO_MDEV(drv_info), addr) &
		       DPMAIF_UL_ADD_NOT_READY) == DPMAIF_UL_ADD_NOT_READY))
			break;

		cnt++;
		udelay(POLL_INTERVAL_US);
	} while (cnt < POLL_MAX_TIMES);

	if (cnt >= POLL_MAX_TIMES) {
		MTK_ERR(DRV_TO_MDEV(drv_info), "Failed to add ulq%u drb-2, cnt=%u\n",
			q_num, drb_cnt);
		return -DATA_HW_REG_TIMEOUT;
	}
	return 0;
}

static int mtk_dpmaif_drv_send_doorbell_t800(struct dpmaif_drv_info *drv_info,
					     enum dpmaif_drv_ring_type type,
					     u8 q_id, u32 cnt)
{
	int ret = 0;

	switch (type) {
	case DPMAIF_PIT:
		ret = mtk_dpmaif_drv_dl_add_pit_cnt(drv_info, q_id, cnt);
		break;
	case DPMAIF_BAT:
		ret = mtk_dpmaif_drv_dl_add_bat_cnt(drv_info, cnt);
		break;
	case DPMAIF_FRAG:
		ret = mtk_dpmaif_drv_dl_add_frg_cnt(drv_info, cnt);
		break;
	case DPMAIF_DRB:
		ret = mtk_dpmaif_drv_ul_add_drb(drv_info, q_id, cnt);
		break;
	default:
		break;
	}

	return ret;
}

static int mtk_dpmaif_drv_dl_get_pit_wridx(struct dpmaif_drv_info *drv_info, u32 qno)
{
	u32 pit_wridx;

	pit_wridx = (mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_DL_LRO_STA5 + qno * 0x20))
		    & DPMAIF_DL_PIT_WRIDX_MSK;
	if (unlikely(pit_wridx >= drv_info->drv_property.dlq[qno].pit_size))
		return -DATA_HW_REG_CHK_FAIL;

	return pit_wridx;
}

static int mtk_dpmaif_drv_dl_get_pit_rdidx(struct dpmaif_drv_info *drv_info, u32 qno)
{
	u32 pit_rdidx;

	pit_rdidx = (mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_DL_LRO_STA6 + qno * 0x20))
		    & DPMAIF_DL_PIT_WRIDX_MSK;
	if (unlikely(pit_rdidx >= drv_info->drv_property.dlq[qno].pit_size))
		return -DATA_HW_REG_CHK_FAIL;

	return pit_rdidx;
}

static int mtk_dpmaif_drv_dl_get_bat_ridx(struct dpmaif_drv_info *drv_info)
{
	u32 bat_ridx;

	bat_ridx = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_BAT_STA2) &
		   DPMAIF_DL_BAT_WRIDX_MSK;

	if (unlikely(bat_ridx >= drv_info->drv_property.ring.normal_bat_size))
		return -DATA_HW_REG_CHK_FAIL;

	return bat_ridx;
}

static int mtk_dpmaif_drv_dl_get_bat_wridx(struct dpmaif_drv_info *drv_info)
{
	u32 bat_wridx;

	bat_wridx = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_BAT_STA3) &
		    DPMAIF_DL_BAT_WRIDX_MSK;
	if (unlikely(bat_wridx >= drv_info->drv_property.ring.normal_bat_size))
		return -DATA_HW_REG_CHK_FAIL;

	return bat_wridx;
}

static int mtk_dpmaif_drv_dl_get_frg_ridx(struct dpmaif_drv_info *drv_info)
{
	u32 frg_ridx;

	frg_ridx = mtk_hw_read32(DRV_TO_MDEV(drv_info), DPMAIF_AO_DL_FRG_STA2) &
		   DPMAIF_DL_FRG_WRIDX_MSK;
	if (unlikely(frg_ridx >= drv_info->drv_property.ring.frag_bat_size))
		return -DATA_HW_REG_CHK_FAIL;

	return frg_ridx;
}

static int mtk_dpmaif_drv_ul_get_drb_ridx(struct dpmaif_drv_info *drv_info, u8 q_num)
{
	u32 drb_ridx;
	u64 addr;

	addr = DPMAIF_ULQ_STA0_N(q_num);

	drb_ridx = mtk_hw_read32(DRV_TO_MDEV(drv_info), addr) >> 16;
	drb_ridx = drb_ridx / DPMAIF_UL_DRB_ENTRY_WORD;

	if (unlikely(drb_ridx >= drv_info->drv_property.ulq[q_num].drb_size))
		return -DATA_HW_REG_CHK_FAIL;

	return drb_ridx;
}

static int mtk_dpmaif_drv_get_ring_idx_t800(struct dpmaif_drv_info *drv_info,
					    enum dpmaif_drv_ring_idx index, u8 q_id)
{
	int ret = 0;

	switch (index) {
	case DPMAIF_PIT_WIDX:
		ret = mtk_dpmaif_drv_dl_get_pit_wridx(drv_info, q_id);
		break;
	case DPMAIF_PIT_RIDX:
		ret = mtk_dpmaif_drv_dl_get_pit_rdidx(drv_info, q_id);
		break;
	case DPMAIF_BAT_WIDX:
		ret = mtk_dpmaif_drv_dl_get_bat_wridx(drv_info);
		break;
	case DPMAIF_BAT_RIDX:
		ret = mtk_dpmaif_drv_dl_get_bat_ridx(drv_info);
		break;

	case DPMAIF_FRAG_WIDX:
		break;

	case DPMAIF_FRAG_RIDX:
		ret = mtk_dpmaif_drv_dl_get_frg_ridx(drv_info);
		break;

	case DPMAIF_DRB_WIDX:
		break;

	case DPMAIF_DRB_RIDX:
		ret = mtk_dpmaif_drv_ul_get_drb_ridx(drv_info, q_id);
		break;
	default:
		break;
	}

	return ret;
}

static u32 mtk_dpmaif_drv_hash_indir_get(struct dpmaif_drv_info *drv_info, u32 *indir)
{
	u32 val = mtk_dpmaif_drv_hash_indir_mask_get(drv_info);
	u8 i;

	for (i = 0; i < DPMAIF_HASH_INDR_SIZE; i++) {
		if (val & (0x01 << i))
			indir[i] = 1;
		else
			indir[i] = 0;
	}

	return 0;
}

static u32 mtk_dpmaif_drv_hash_indir_set(struct dpmaif_drv_info *drv_info, u32 *indir)
{
	u32 val = 0;
	u8 i;

	for (i = 0; i < DPMAIF_HASH_INDR_SIZE; i++) {
		if (indir[i])
			val |= (0x01 << i);
	}
	mtk_dpmaif_drv_hash_indir_mask_set(drv_info, val);

	return 0;
}

static void mtk_dpmaif_drv_agg_cfg(struct dpmaif_drv_info *drv_info, bool enable)
{
	u32 agg_max_len;
	u32 threshold;
	u32 cfg;

	cfg = mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_DL_RDY_CHK_FRG_THRES);

	/* enable/disable AGG cfg */
	if (enable) {
		threshold = cfg | (0xff << 20);
		agg_max_len = (DPMAIF_AGG_TBL_ENT_NUM_DF & 0xffff) << 16;
	} else {
		threshold = cfg & 0xf00fffff;
		agg_max_len = (1 & 0xffff) << 16;
	}

	mtk_hw_write32(DRV_TO_MDEV(drv_info),
		       NRL2_DPMAIF_AO_DL_RDY_CHK_FRG_THRES, threshold);
	cfg = DPMAIF_AGG_MAX_LEN_DF & 0xffff;
	cfg |= agg_max_len;

	/* Configuration include agg max length, agg table number. */
	mtk_hw_write32(DRV_TO_MDEV(drv_info),
		       NRL2_DPMAIF_AO_DL_LRO_AGG_CFG, cfg);
}

static u32 mtk_dpmaif_drv_lro_set(struct dpmaif_drv_info *drv_info, void *data)
{
	bool dpmaif_lro_enable = *(bool *)data;
	u32 cfg;

	cfg = mtk_hw_read32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_DL_RDY_CHK_FRG_THRES);

	if (dpmaif_lro_enable && !(drv_info->drv_property.features & DATA_HW_F_LRO)) {
		drv_info->drv_property.features |= DATA_HW_F_LRO;
		mtk_dpmaif_drv_agg_cfg(drv_info, dpmaif_lro_enable);
		mtk_dpmaif_drv_set_dlq_timeout_threshold(drv_info);
	} else if (!dpmaif_lro_enable && drv_info->drv_property.features & DATA_HW_F_LRO) {
		drv_info->drv_property.features &= ~DATA_HW_F_LRO;
		mtk_dpmaif_drv_agg_cfg(drv_info, dpmaif_lro_enable);
		mtk_dpmaif_drv_clr_dlq_timeout_threshold(drv_info);
	}

	return 0;
}

static u32 mtk_dpmaif_drv_rxcsum_set(struct dpmaif_drv_info *drv_info, void *data)
{
	bool dpmaif_rxcsum_enable = *(bool *)data;

	if (dpmaif_rxcsum_enable && !(drv_info->drv_property.features & DATA_HW_F_RXCSUM)) {
		drv_info->drv_property.features |= DATA_HW_F_RXCSUM;
		mtk_dpmaif_drv_dl_set_pkt_checksum(drv_info);
	} else if (!dpmaif_rxcsum_enable && drv_info->drv_property.features & DATA_HW_F_RXCSUM) {
		drv_info->drv_property.features &= ~DATA_HW_F_RXCSUM;
		mtk_dpmaif_drv_dl_clr_pkt_checksum(drv_info);
	}

	return 0;
}

static u32 mtk_dpmaif_drv_txcsum_set(struct dpmaif_drv_info *drv_info, void *data)
{
	bool dpmaif_txcsum_enable = *(bool *)data;

	if (dpmaif_txcsum_enable && !(drv_info->drv_property.features & DATA_HW_F_TXCSUM))
		drv_info->drv_property.features |= DATA_HW_F_TXCSUM;
	else if (!dpmaif_txcsum_enable && drv_info->drv_property.features & DATA_HW_F_TXCSUM)
		drv_info->drv_property.features &= ~DATA_HW_F_TXCSUM;

	return 0;
}

static u32 mtk_dpmaif_drv_features_get(struct dpmaif_drv_info *drv_info, void *data)
{
	*(u32  *)data = drv_info->drv_property.features;

	return 0;
}

static int mtk_dpmaif_drv_ul_set_delay_intr(struct dpmaif_drv_info *drv_info,
					    u8 q_num, u8 mode, u32 time_us, u32 pkt_cnt)
{
	u32 ret = 0, cfg;

	cfg = ((mode & 0x3) << 30) | ((pkt_cnt & 0x3fff) << 16) | (time_us & 0xffff);

	switch (q_num) {
	case 0:
		mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DLY_IRQ_TIMER3, cfg);
		break;
	case 1:
		mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DLY_IRQ_TIMER4, cfg);
		break;
	case 2:
		mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DLY_IRQ_TIMER5, cfg);
		break;
	case 3:
		mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DLY_IRQ_TIMER6, cfg);
		break;
	case 4:
		mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_DLY_IRQ_TIMER7, cfg);
		break;
	default:
		MTK_WARN(DRV_TO_MDEV(drv_info), "Invalid ulq=%d!\n", q_num);
		ret = -EINVAL;
	}

	return ret;
}

static int mtk_dpmaif_drv_dl_set_delay_intr(struct dpmaif_drv_info *drv_info,
					    u8 q_num, u8 mode, u32 time_us, u32 pkt_cnt)
{
	int ret = 0;
	u32 cfg = 0;

	cfg = ((mode & 0x3) << 30) | ((pkt_cnt & 0x3fff) << 16) | (time_us & 0xffff);

	switch (q_num) {
	case 0:
		mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_DL_DLY_IRQ_TIMER1, cfg);
		break;
	case 1:
		mtk_hw_write32(DRV_TO_MDEV(drv_info), NRL2_DPMAIF_AO_DL_DLY_IRQ_TIMER2, cfg);
		break;
	default:
		MTK_WARN(DRV_TO_MDEV(drv_info), "Invalid dlq=%d!\n", q_num);
		ret = -EINVAL;
	}

	return ret;
}

static int mtk_dpmaif_drv_intr_coalesce_set(struct dpmaif_drv_info *drv_info,
					    struct dpmaif_drv_intr *intr)
{
	u8 i;

	if (intr->dir == DPMAIF_TX) {
		for (i = 0; i < DPMAIF_ULQ_NUM; i++) {
			if (intr->q_mask & (1 << i))
				mtk_dpmaif_drv_ul_set_delay_intr(drv_info, i, intr->mode,
								 intr->time_threshold,
								 intr->pkt_threshold);
		}
	} else {
		for (i = 0; i < DPMAIF_DLQ_NUM; i++) {
			if (intr->q_mask & (1 << i))
				mtk_dpmaif_drv_dl_set_delay_intr(drv_info, i, intr->mode,
								 intr->time_threshold,
								 intr->pkt_threshold);
		}
	}

	return 0;
}

static struct dpmaif_res_cfg res_cfg = {
	.tx_srv_cnt = 4,
	.tx_vq_cnt = DPMAIF_ULQ_NUM,
	.tx_vq_srv_map = {3, 1, 2, 0, 3},
	.srv_prio_tbl = {-20, -15, -10, -5},
#ifdef CONFIG_REG_ACCESS_OPTIMIZE
	.txq_doorbell_delay = {10, 2, 0, 0, 10},
	.txq_burst_pkts = {128, 32, 0, 0, 128},
#else
	.txq_doorbell_delay = {0},
#endif
	.irq_cnt = 3,
	.irq_src = {MTK_IRQ_SRC_DPMAIF, MTK_IRQ_SRC_DPMAIF2, MTK_IRQ_SRC_DPMAIF3},
	.txq_cnt = DPMAIF_ULQ_NUM,
	.rxq_cnt = DPMAIF_DLQ_NUM,
	.normal_bat_cnt = 16384,
	.frag_bat_cnt = 8192,
	.pit_cnt = {16384, 16384},
	.drb_cnt = {6144, 6144, 6144, 6144, 6144},
	.cap = DATA_HW_F_LRO | DATA_HW_F_INDR_TBL | DATA_HW_F_INTR_COALESCE |
	DATA_HW_F_RXCSUM | DATA_HW_F_TXCSUM,
};

static int mtk_dpmaif_drv_cfg_get(struct dpmaif_drv_info *drv_info, void *data)
{
	*(struct dpmaif_res_cfg **)data = &res_cfg;

	return 0;
}

static int mtk_dpmaif_drv_txq_get(struct dpmaif_drv_info *drv_info, void *data)
{
	struct dpmaif_drv_pkt_info *pkt_info = (struct dpmaif_drv_pkt_info *)data;
	int q_id;

	switch (pkt_info->prio) {
	case PRIORITY_1:
		q_id = 2;
		break;
	case PRIORITY_2:
		q_id = 1;
		break;
	case PRIORITY_3:
		q_id = 3;
		break;
	case PRIORITY_0:
		fallthrough;
	default:
		q_id = (pkt_info->skb_hash & 0x01) ? 0 : 4;
		break;
	}
	return q_id;
}

static int mtk_dpmaif_drv_feature_cmd_t800(struct dpmaif_drv_info *drv_info,
					   enum dpmaif_drv_cmd cmd, void *data)
{
	int ret = 0;

	switch (cmd) {
	case DATA_HW_INTR_COALESCE_SET:
		ret = mtk_dpmaif_drv_intr_coalesce_set(drv_info, data);
		break;
	case DATA_HW_HASH_GET:
		ret = mtk_dpmaif_drv_hash_sec_key_get(drv_info, data);
		break;
	case DATA_HW_HASH_SET:
		ret = mtk_dpmaif_drv_hash_sec_key_set(drv_info, data);
		break;
	case DATA_HW_HASH_KEY_SIZE_GET:
		*(u32 *)data = DPMAIF_HASH_SEC_KEY_NUM;
		break;
	case DATA_HW_INDIR_GET:
		ret = mtk_dpmaif_drv_hash_indir_get(drv_info, data);
		break;
	case DATA_HW_INDIR_SET:
		ret = mtk_dpmaif_drv_hash_indir_set(drv_info, data);
		break;
	case DATA_HW_INDIR_SIZE_GET:
		*(u32 *)data = DPMAIF_HASH_INDR_SIZE;
		break;
	case DATA_HW_LRO_SET:
		ret = mtk_dpmaif_drv_lro_set(drv_info, data);
		break;
	case DATA_HW_RXCSUM_SET:
		ret = mtk_dpmaif_drv_rxcsum_set(drv_info, data);
		break;
	case DATA_HW_TXCSUM_SET:
		ret = mtk_dpmaif_drv_txcsum_set(drv_info, data);
		break;
	case DATA_HW_FEATURES_GET:
		ret = mtk_dpmaif_drv_features_get(drv_info, data);
		break;
	case DATA_HW_CFG_GET:
		ret = mtk_dpmaif_drv_cfg_get(drv_info, data);
		break;
	case DATA_HW_TXQ_GET:
		ret = mtk_dpmaif_drv_txq_get(drv_info, data);
		break;

	default:
		MTK_WARN(DRV_TO_MDEV(drv_info), "Unsupport cmd=%d\n", cmd);
		ret = -EOPNOTSUPP;
		break;
	}

	return ret;
}

static void mtk_dpmaif_drv_dump_t800(struct dpmaif_drv_info *drv_info)
{
	unsigned int delay_us = 50;
	unsigned long base_addr;
	int len;

	MTK_DBG(DRV_TO_MDEV(drv_info),
		MTK_DBG_DPMF, MTK_MEMLOG_RG_5, "===start dpmaif t800 drv dump===\n");
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"interrupt mask:0x%08x,0x%08x,0x%08x\n",
		drv_info->drv_irq_en_mask.ap_ul_l2intr_en_mask,
		drv_info->drv_irq_en_mask.ap_dl_l2intr_en_mask,
		drv_info->drv_irq_en_mask.ap_udl_ip_busy_en_mask);

	MTK_DBG(DRV_TO_MDEV(drv_info),
		MTK_DBG_DPMF, MTK_MEMLOG_RG_5, "==dpmaif regs dump ==\n");
	base_addr = 0x10011000;
	len = 0x9c;
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"confg_dpmaif_ao_ul_mmw_upd: 0x%lx, len=0x%x\n", base_addr, len);
	MTK_REGS_DUMP(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		      NULL, base_addr, len);
	mtk_drv_delay(delay_us);

	base_addr = 0x10011400;
	len = 0x74;
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"confg_dpmaif_ao_dl_mmw_upd: 0x%lx, len=0x%x\n", base_addr, len);
	MTK_REGS_DUMP(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		      NULL, base_addr, len);
	mtk_drv_delay(delay_us);

	base_addr = 0x10011800;
	len = 0x64;
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"nrl2_dpmaif_misc_ao_cfg_mmw: 0x%lx, len=0x%x\n", base_addr, len);
	MTK_REGS_DUMP(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		      NULL, base_addr, len);
	mtk_drv_delay(delay_us);

	base_addr = 0x1022C000;
	len = 0x54;
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"nrl2_dpmaif_md_misc_cfg: 0x%lx, len=0x%x\n", base_addr, len);
	MTK_REGS_DUMP(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		      NULL, base_addr, len);
	mtk_drv_delay(delay_us);

	base_addr = 0x1022C100;
	len = 0xDC;
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"nrl2_dpmaif_md_dl_reorder_cfg: 0x%lx, len=0x%x\n", base_addr, len);
	MTK_REGS_DUMP(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		      NULL, base_addr, len);
	mtk_drv_delay(delay_us);

	base_addr = 0x1022D000;
	len = 0xE4;
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"nrl2_dpmaif_ul: 0x%lx, len=0x%x\n", base_addr, len);
	MTK_REGS_DUMP(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		      NULL, base_addr, len);
	mtk_drv_delay(delay_us);

	base_addr = 0x1022D100;
	len = 0xFC;
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"mmw_dpmaif_dl_cfg: 0x%lx, len=0x%x\n", base_addr, len);
	MTK_REGS_DUMP(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		      NULL, base_addr, len);
	mtk_drv_delay(delay_us);

	base_addr = 0x1022D400;
	len = 0xFC;
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"confg_dpmaif_ap_misc: 0x%lx, len=0x%x\n", base_addr, len);
	MTK_REGS_DUMP(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		      NULL, base_addr, len);
	mtk_drv_delay(delay_us);

	base_addr = 0x1022D500;
	len = 0xFC;
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"confg_dpmaif_dl_reorder_idx: 0x%lx, len=0x%x\n", base_addr, len);
	MTK_REGS_DUMP(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		      NULL, base_addr, len);
	mtk_drv_delay(delay_us);

	base_addr = 0x1022D600;
	len = 0x1FC;
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"mmw_dpmaif_hpc: 0x%lx, len=0x%x\n", base_addr, len);
	MTK_REGS_DUMP(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		      NULL, base_addr, len);
	mtk_drv_delay(delay_us);

	base_addr = 0x1022D900;
	len = 0xFC;
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"confg_dpmaif_dl_lro_removeao_idx: 0x%lx, len=0x%x\n", base_addr, len);
	MTK_REGS_DUMP(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		      NULL, base_addr, len);
	mtk_drv_delay(delay_us);

	base_addr = 0x1022DC00;
	len = 0xFC;
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"confg_dpmaif_mmw_pd_sram_dl: 0x%lx, len=0x%x\n", base_addr, len);
	MTK_REGS_DUMP(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		      NULL, base_addr, len);
	mtk_drv_delay(delay_us);

	base_addr = 0x1022DD00;
	len = 0xFC;
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"confg_dpmaif_mmw_pd_sram_ul: 0x%lx, len=0x%x\n", base_addr, len);
	MTK_REGS_DUMP(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		      NULL, base_addr, len);
	mtk_drv_delay(delay_us);

	base_addr = 0x1022DE00;
	len = 0x5C;
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"confg_dpmaif_mmw_pd_sram_misc: 0x%lx, len=0x%x\n", base_addr, len);
	MTK_REGS_DUMP(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		      NULL, base_addr, len);
	mtk_drv_delay(delay_us);

	base_addr = 0x1022DF00;
	len = 0xFC;
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"config_dpmaif_mmw_pd_sram_misc2: 0x%lx, len=0x%x\n", base_addr, len);
	MTK_REGS_DUMP(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		      NULL, base_addr, len);
	mtk_drv_delay(delay_us);

	base_addr = 0x1022D200;
	len = 0xFC;
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"nrl2_cldma_rdma_cfg: 0x%lx, len=0x%x\n", base_addr, len);
	MTK_REGS_DUMP(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		      NULL, base_addr, len);
	mtk_drv_delay(delay_us);

	base_addr = 0x1022D300;
	len = 0xFC;
	MTK_DBG(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		"nrl2_cldma_dma_wr_config: 0x%lx, len=0x%x\n", base_addr, len);
	MTK_REGS_DUMP(DRV_TO_MDEV(drv_info), MTK_DBG_DPMF, MTK_MEMLOG_RG_5,
		      NULL, base_addr, len);
	mtk_drv_delay(delay_us);

	MTK_DBG(DRV_TO_MDEV(drv_info),
		MTK_DBG_DPMF, MTK_MEMLOG_RG_5, "===end dpmaif t800 drv dump===\n");
}

struct dpmaif_drv_ops dpmaif_drv_ops_t800 = {
	.init = mtk_dpmaif_drv_init_t800,
	.start_queue = mtk_dpmaif_drv_start_queue_t800,
	.stop_queue = mtk_dpmaif_drv_stop_queue_t800,
	.intr_handle = mtk_dpmaif_drv_intr_handle_t800,
	.intr_complete = mtk_dpmaif_drv_intr_complete_t800,
	.send_doorbell = mtk_dpmaif_drv_send_doorbell_t800,
	.get_ring_idx = mtk_dpmaif_drv_get_ring_idx_t800,
	.feature_cmd = mtk_dpmaif_drv_feature_cmd_t800,
	.dump = mtk_dpmaif_drv_dump_t800,
};
