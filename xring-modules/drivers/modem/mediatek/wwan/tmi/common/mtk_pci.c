// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2022, MediaTek Inc.
 */

#include <linux/acpi.h>
#include <linux/aer.h>
#include <linux/bitfield.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched/clock.h>

#include "mtk_common.h"
#include "mtk_pci.h"
#include "mtk_port.h"
#include "mtk_port_io.h"
#include "mtk_pci_dev_cfg.h"
#ifdef CONFIG_TX00_UT_PCI
#include "ut_pci_fake.h"
#endif

#if IS_ENABLED(CONFIG_MTK_WWAN_PWRCTL_SUPPORT)
#include "mtk_pwrctl.h"
//#include "pcie-mediatek-gen3.h"
#endif

#define TAG "PCI"
#define BAR_NUM		6

#define MTK_PCI_TRANSPARENT_ATR_SIZE  (0x3F)

struct sock *mtk_netlink_sock;

static bool mtk_pci_ds_lock_debug;
extern bool mtk_cldma_irq_registed;
static int mtk_pci_irq_cnt_max = MTK_IRQ_CNT_MAX;
static int mtk_pci_irq_type = PCI_IRQ_MSIX | PCI_IRQ_LEGACY;

extern int xring_pcie_disable_data_trans(int port);

/* This table records which bits of the interrupt status register each interrupt corresponds to
 * when there are different numbers of msix interrupts.
 */
static const u32 mtk_msix_bits_map[MTK_IRQ_CNT_MAX / 2][5] = {
	{0xFFFFFFFF, 0x55555555, 0x11111111, 0x01010101, 0x00010001},
	{0x00000000, 0xAAAAAAAA, 0x22222222, 0x02020202, 0x00020002},
	{0x00000000, 0x00000000, 0x44444444, 0x04040404, 0x00040004},
	{0x00000000, 0x00000000, 0x88888888, 0x08080808, 0x00080008},
	{0x00000000, 0x00000000, 0x00000000, 0x10101010, 0x00100010},
	{0x00000000, 0x00000000, 0x00000000, 0x20202020, 0x00200020},
	{0x00000000, 0x00000000, 0x00000000, 0x40404040, 0x00400040},
	{0x00000000, 0x00000000, 0x00000000, 0x80808080, 0x00800080},
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x01000100},
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x02000200},
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000400},
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x08000800},
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10001000},
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x20002000},
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x40004000},
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x80008000},
};

u32 mtk_pci_mac_read32(struct mtk_pci_priv *priv, u64 addr)
{
	return ioread32(priv->mac_reg_base + addr);
}

void mtk_pci_mac_write32(struct mtk_pci_priv *priv, u64 addr, u32 val)
{
	iowrite32(val, priv->mac_reg_base + addr);
}

u32 mtk_pci_read32(struct mtk_md_dev *mdev, u64 addr)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	return ioread32(priv->ext_reg_base + addr);
}

void mtk_pci_write32(struct mtk_md_dev *mdev, u64 addr, u32 val)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	iowrite32(val, priv->ext_reg_base + addr);
}

int mtk_pci_setup_atr(struct mtk_md_dev *mdev, struct mtk_atr_cfg *cfg)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	u32 addr, val, size_h, size_l;
	int atr_size, pos, offset;

	if (cfg->transparent) {
		atr_size = MTK_PCI_TRANSPARENT_ATR_SIZE; /* No address conversion is performed */
	} else {
		if (cfg->src_addr & (cfg->size - 1)) {
			dev_err(mdev->dev, "Invalid atr src addr is not aligned to size\n");
			return -EFAULT;
		}
		if (cfg->trsl_addr & (cfg->size - 1)) {
			dev_err(mdev->dev, "Invalid atr trsl addr is not aligned to size, %llx, %llx\n",
				cfg->trsl_addr, cfg->size - 1);
			return -EFAULT;
		}
		size_l = FIELD_GET(GENMASK_ULL(31, 0), cfg->size);
		size_h = FIELD_GET(GENMASK_ULL(63, 32), cfg->size);
		pos = ffs(size_l);
		if (pos) {
			/* Address Translate Space Size is equal to 2^(atr_size+1).
			 * "-2" means "-1-1", the first "-1" is because of the atr_size register,
			 * the second is because of the ffs() will increase by one.
			 */
			atr_size = pos - 2;
		} else {
			pos = ffs(size_h);
			/* "+30" means "+32-1-1", the meaning of "-1-1" is same as above,
			 * "+32" is because atr_size is large, exceeding 32-bits.
			 */
			atr_size = pos + 30;
		}
	}

	/* Calculate table offset */
	offset = ATR_PORT_OFFSET * cfg->port + ATR_TABLE_OFFSET * cfg->table;
	/* SRC_ADDR_H */
	addr = REG_ATR_PCIE_WIN0_T0_SRC_ADDR_MSB + offset;
	val = (u32)(cfg->src_addr >> 32);
	mtk_pci_mac_write32(priv, addr, val);
	/* SRC_ADDR_L */
	addr = REG_ATR_PCIE_WIN0_T0_SRC_ADDR_LSB + offset;
	val = (u32)(cfg->src_addr & 0xFFFFF000) | (atr_size << 1) | 0x1;
	mtk_pci_mac_write32(priv, addr, val);

	/* TRSL_ADDR_H */
	addr = REG_ATR_PCIE_WIN0_T0_TRSL_ADDR_MSB + offset;
	val = (u32)(cfg->trsl_addr >> 32);
	mtk_pci_mac_write32(priv, addr, val);
	/* TRSL_ADDR_L */
	addr = REG_ATR_PCIE_WIN0_T0_TRSL_ADDR_LSB + offset;
	val = (u32)(cfg->trsl_addr & 0xFFFFF000);
	mtk_pci_mac_write32(priv, addr, val);

	/* TRSL_PARAM */
	addr = REG_ATR_PCIE_WIN0_T0_TRSL_PARAM + offset;
	val = (cfg->trsl_param << 16) | cfg->trsl_id;
	mtk_pci_mac_write32(priv, addr, val);

	return 0;
}

void mtk_pci_atr_disable(struct mtk_pci_priv *priv)
{
	int port, tbl, offset;
	u32 val;

	/* Disable all ATR table for all ports */
	for (port = ATR_SRC_PCI_WIN0; port <= ATR_SRC_AXIS_3; port++)
		for (tbl = 0; tbl < ATR_TABLE_NUM_PER_ATR; tbl++) {
			/* Calculate table offset */
			offset = ATR_PORT_OFFSET * port + ATR_TABLE_OFFSET * tbl;
			val = mtk_pci_mac_read32(priv, REG_ATR_PCIE_WIN0_T0_SRC_ADDR_LSB + offset);
			val = val & (~BIT(0));
			/* Disable table by SRC_ADDR_L */
			mtk_pci_mac_write32(priv, REG_ATR_PCIE_WIN0_T0_SRC_ADDR_LSB + offset, val);
		}
}

void mtk_pci_flush_work(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	flush_work(&priv->mhccif_work);
}

static void mtk_pci_set_msi_merged(struct mtk_pci_priv *priv, int irq_cnt)
{
	u32 reg;

	reg = mtk_pci_mac_read32(priv, REG_PCIE_CFG_MSI_MERGED);
	reg = reg & (~MTK_PCI_MSI_MESSAGE_CONTROL);
	reg |= ((ffs(irq_cnt) - 1) << 4);
	mtk_pci_mac_write32(priv, REG_PCIE_CFG_MSI_MERGED, reg);
}

static void mtk_pci_set_msix_merged(struct mtk_pci_priv *priv, int irq_cnt)
{
	mtk_pci_mac_write32(priv, REG_PCIE_CFG_MSIX, ffs(irq_cnt) * 2 - 1);
}

static u32 mtk_pci_get_dev_state(struct mtk_md_dev *mdev)
{
	return mtk_pci_mac_read32(mdev->hw_priv, REG_PCIE_DEBUG_DUMMY_7);
}

static void mtk_pci_ack_dev_state(struct mtk_md_dev *mdev, u32 state)
{
	mtk_pci_mac_write32(mdev->hw_priv, REG_PCIE_DEBUG_DUMMY_7, state);
}

static void mtk_pci_force_mac_active(struct mtk_md_dev *mdev, bool enable, u8 ctrl)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	u32 reg;
	u8 sts;

	sts = priv->mac_active_ctrl;
	if (enable)
		priv->mac_active_ctrl |= ctrl;
	else
		priv->mac_active_ctrl &= ~ctrl;

	if (!sts && priv->mac_active_ctrl) {
		reg = mtk_pci_mac_read32(priv, REG_PCIE_MISC_CTRL);
		reg |= MTK_FORCE_MAC_ACTIVE_BIT;
		mtk_pci_mac_write32(priv, REG_PCIE_MISC_CTRL, reg);
	}

	if (sts && !priv->mac_active_ctrl) {
		reg = mtk_pci_mac_read32(priv, REG_PCIE_MISC_CTRL);
		reg &= ~MTK_FORCE_MAC_ACTIVE_BIT;
		mtk_pci_mac_write32(priv, REG_PCIE_MISC_CTRL, reg);
	}
}

static u32 mtk_pci_get_ds_status(struct mtk_md_dev *mdev)
{
	u32 reg;

	reg = mtk_pci_mac_read32(mdev->hw_priv, REG_PCIE_RESOURCE_STATUS);

	return reg;
}

static void mtk_pci_enable_intr(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	u32 reg;

	reg = mtk_pci_mac_read32(priv, priv->cfg->istatus_host_ctrl_addr);
	reg &= ~MTK_ENABLE_INTR_BIT;
	mtk_pci_mac_write32(priv, priv->cfg->istatus_host_ctrl_addr, reg);
}

static void mtk_pci_ds_lock(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	unsigned long flags;
	u32 reg;

	if (mdev->hw_ver != 0x4d75 && mdev->hw_ver != 0x4d80) {
		mtk_pci_mac_write32(mdev->hw_priv, REG_PCIE_PEXTP_MAC_SLEEP_CTRL,
				    MTK_DISABLE_DS_BIT(0));
	} else {
		reg = mtk_pci_mac_read32(priv, REG_PCIE_MISC_CTRL);
		reg |= MTK_DS_LOCK_REG_BIT;
		mtk_pci_mac_write32(priv, REG_PCIE_MISC_CTRL, reg);
	}
	if (priv->cfg->flag & MTK_CFG_FORCE_MAC_ACTIVE) {
		spin_lock_irqsave(&priv->mac_active_lock, flags);
		mtk_pci_force_mac_active(mdev, true, DS_LOCK_MAC_ACTIVE);
		spin_unlock_irqrestore(&priv->mac_active_lock, flags);
	}

	if (mtk_pci_ds_lock_debug)
		MTK_INFO(mdev, "PCI ds lock\n");
}

static void mtk_pci_ds_unlock(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	unsigned long flags;
	u32 reg;

	if (mdev->hw_ver != 0x4d75 && mdev->hw_ver != 0x4d80) {
		mtk_pci_mac_write32(mdev->hw_priv, REG_PCIE_PEXTP_MAC_SLEEP_CTRL,
				    MTK_ENABLE_DS_BIT(0));
	} else {
		reg = mtk_pci_mac_read32(priv, REG_PCIE_MISC_CTRL);
		reg &= ~MTK_DS_LOCK_REG_BIT;
		mtk_pci_mac_write32(priv, REG_PCIE_MISC_CTRL, reg);
	}
	if (priv->cfg->flag & MTK_CFG_FORCE_MAC_ACTIVE) {
		spin_lock_irqsave(&priv->mac_active_lock, flags);
		mtk_pci_force_mac_active(mdev, false, DS_LOCK_MAC_ACTIVE);
		spin_unlock_irqrestore(&priv->mac_active_lock, flags);
	}

	if (mtk_pci_ds_lock_debug)
		MTK_INFO(mdev, "PCI ds unlock\n");
}

static void mtk_pci_set_l1ss(struct mtk_md_dev *mdev, u32 type, bool enable, enum mtk_l1ss_grp user)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	u32 addr = REG_DIS_ASPM_LOWPWR_SET_0;
	unsigned long flags;

	if (enable) {
		addr = REG_DIS_ASPM_LOWPWR_CLR_0;
		if (priv->cfg->flag & MTK_CFG_FORCE_MAC_ACTIVE) {
			spin_lock_irqsave(&priv->mac_active_lock, flags);
			priv->l1ss_user &= ~BIT(user);
			if (!priv->l1ss_user)
				mtk_pci_force_mac_active(mdev, false, L1SS_MAC_ACTIVE);
			spin_unlock_irqrestore(&priv->mac_active_lock, flags);
		}
	} else {
		if (priv->cfg->flag & MTK_CFG_FORCE_MAC_ACTIVE) {
			spin_lock_irqsave(&priv->mac_active_lock, flags);
			if (!priv->l1ss_user)
				mtk_pci_force_mac_active(mdev, true, L1SS_MAC_ACTIVE);
			priv->l1ss_user |= BIT(user);
			spin_unlock_irqrestore(&priv->mac_active_lock, flags);
		}
	}

	mtk_pci_mac_write32(priv, addr, type);
}

static int mtk_pci_get_irq_id(struct mtk_md_dev *mdev, enum mtk_irq_src irq_src)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	const int *irq_tbl = priv->cfg->irq_tbl;
	int irq_id = -EINVAL;

	if (irq_src > MTK_IRQ_SRC_MIN && irq_src < MTK_IRQ_SRC_MAX) {
		irq_id = irq_tbl[irq_src];
		if (unlikely(irq_id < 0 || irq_id >= MTK_IRQ_CNT_MAX))
			irq_id = -EINVAL;
	}

	return irq_id;
}

static int mtk_pci_get_virq_id(struct mtk_md_dev *mdev, int irq_id)
{
	struct pci_dev *pdev = to_pci_dev(mdev->dev);
	int nr = 0;

	if (pdev->msix_enabled)
		nr = irq_id % mdev->msi_nvecs;

	return pci_irq_vector(pdev, nr);
}

static int mtk_pci_register_irq(struct mtk_md_dev *mdev, int irq_id,
				int (*irq_cb)(int irq_id, void *data), void *data)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	if (unlikely((irq_id < 0 || irq_id >= MTK_IRQ_CNT_MAX) || !irq_cb))
		return -EINVAL;

	if (priv->irq_cb_list[irq_id]) {
		MTK_ERR(mdev,
			"Unable to register irq, irq_id=%d, it's already been register by %ps.\n",
			irq_id, priv->irq_cb_list[irq_id]);
		return -EFAULT;
	}
	priv->irq_cb_list[irq_id] = irq_cb;
	priv->irq_cb_data[irq_id] = data;
	MTK_INFO(mdev, "Register irq: irq_id=%d irq_cb=%ps data=%p\n",
		 irq_id, irq_cb, data);

	return 0;
}

static int mtk_pci_unregister_irq(struct mtk_md_dev *mdev, int irq_id)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	if (unlikely(irq_id < 0 || irq_id >= MTK_IRQ_CNT_MAX))
		return -EINVAL;

	if (!priv->irq_cb_list[irq_id]) {
		MTK_ERR(mdev, "irq_id=%d has not been registered\n", irq_id);
		return -EFAULT;
	}
	priv->irq_cb_list[irq_id] = NULL;
	priv->irq_cb_data[irq_id] = NULL;
	MTK_INFO(mdev, "Unregister irq: irq_id=%d\n", irq_id);

	return 0;
}

static int mtk_pci_mask_irq(struct mtk_md_dev *mdev, int irq_id)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	if (unlikely((irq_id < 0 || irq_id >= MTK_IRQ_CNT_MAX) || priv->irq_type == 0)) {
		MTK_ERR(mdev, "Failed to mask irq: input irq_id=%d\n", irq_id);
		return -EINVAL;
	}

	if (likely(priv->irq_type == PCI_IRQ_MSIX))
		mtk_pci_mac_write32(priv, REG_IMASK_HOST_MSIX_CLR_GRP0_0, BIT(irq_id));
	else
		mtk_pci_mac_write32(priv, REG_INT_ENABLE_HOST_CLR, BIT(irq_id));

	return 0;
}

static int mtk_pci_unmask_irq(struct mtk_md_dev *mdev, int irq_id)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	if (unlikely((irq_id < 0 || irq_id >= MTK_IRQ_CNT_MAX) || priv->irq_type == 0)) {
		MTK_ERR(mdev, "Failed to unmask irq: input irq_id=%d\n", irq_id);
		return -EINVAL;
	}

	if (likely(priv->irq_type == PCI_IRQ_MSIX))
		mtk_pci_mac_write32(priv, REG_IMASK_HOST_MSIX_SET_GRP0_0, BIT(irq_id));
	else
		mtk_pci_mac_write32(priv, REG_INT_ENABLE_HOST_SET, BIT(irq_id));

	return 0;
}

static int mtk_pci_clear_irq(struct mtk_md_dev *mdev, int irq_id)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	if (unlikely((irq_id < 0 || irq_id >= MTK_IRQ_CNT_MAX) || priv->irq_type == 0)) {
		MTK_ERR(mdev, "Failed to clear irq: input irq_id=%d\n", irq_id);
		return -EINVAL;
	}

	if (likely(priv->irq_type == PCI_IRQ_MSIX))
		mtk_pci_mac_write32(priv, REG_MSIX_ISTATUS_HOST_GRP0_0, BIT(irq_id));
	else
		mtk_pci_mac_write32(priv, REG_ISTATUS_HOST, BIT(irq_id));

	return 0;
}

static int mtk_pci_reset_sys_irq(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	int rgu_virq_id, mhccif_virq_id;

	rgu_virq_id = mtk_pci_get_virq_id(mdev, priv->rgu_irq_id);
	mhccif_virq_id = mtk_pci_get_virq_id(mdev, priv->mhccif_irq_id);

	mtk_pci_mask_irq(mdev, priv->rgu_irq_id);
	synchronize_irq(rgu_virq_id);
	cancel_delayed_work_sync(&priv->rgu_work);

	mtk_pci_mask_irq(mdev, priv->mhccif_irq_id);
	synchronize_irq(mhccif_virq_id);
	cancel_work_sync(&priv->mhccif_work);

	if (priv->mac_active_ctrl || priv->l1ss_user)
		MTK_WARN(mdev, "PCIe mac active ctrl=0x%x, l1ss_user=0x%x!\n",
			 priv->mac_active_ctrl, priv->l1ss_user);

	return 0;
}

static void mtk_pci_send_sw_evt(struct mtk_md_dev *mdev, enum mtk_h2d_sw_evt evt)
{
	mtk_pci_mac_write32(mdev->hw_priv, REG_RC2EP_SW_TRIG_LOCAL_INTR_SET, BIT(evt));
	mtk_pci_mac_read32(mdev->hw_priv, REG_RC2EP_SW_TRIG_LOCAL_INTR_STAT);
}

static void mtk_pci_clear_sw_evt(struct mtk_md_dev *mdev, enum mtk_d2h_sw_evt evt)
{
	mtk_pci_mac_write32(mdev->hw_priv, REG_SW_TRIG_INTR_CLR, BIT(evt));
}

static int mtk_mhccif_register_evt(struct mtk_md_dev *mdev, u32 chs,
				   int (*evt_cb)(u32 status, void *data), void *data)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	struct mtk_mhccif_cb *cb;
	unsigned long flag;
	int ret = 0;

	if (!chs || !evt_cb)
		return -EINVAL;

	spin_lock_irqsave(&priv->mhccif_lock, flag);
	list_for_each_entry(cb, &priv->mhccif_cb_list, entry) {
		if (cb->chs & chs) {
			ret = -EFAULT;
			MTK_ERR(mdev,
				"Unable to register evt, intersection: chs=0x%08x&0x%08x registered_cb=%ps\n",
				chs, cb->chs, cb->evt_cb);
			goto err;
		}
	}
	cb = devm_kzalloc(mdev->dev, sizeof(*cb), GFP_ATOMIC);
	if (!cb) {
		ret = -ENOMEM;
		goto err;
	}
	cb->evt_cb = evt_cb;
	cb->data = data;
	cb->chs = chs;
	list_add_tail(&cb->entry, &priv->mhccif_cb_list);
	MTK_INFO(mdev, "Register mhccif: chs=0x%08x evt_cb=%ps data=%p\n",
		 chs, evt_cb, data);
err:
	spin_unlock_irqrestore(&priv->mhccif_lock, flag);

	return ret;
}

static int mtk_mhccif_unregister_evt(struct mtk_md_dev *mdev, u32 chs)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	struct mtk_mhccif_cb *cb, *next;
	unsigned long flag;
	int ret = 0;

	if (!chs)
		return -EINVAL;

	spin_lock_irqsave(&priv->mhccif_lock, flag);
	list_for_each_entry_safe(cb, next, &priv->mhccif_cb_list, entry) {
		if (cb->chs == chs) {
			list_del(&cb->entry);
			devm_kfree(mdev->dev, cb);
			MTK_INFO(mdev, "Unregister mhccif: chs=0x%08x\n", chs);
			goto out;
		}
	}
	ret = -EFAULT;
	MTK_WARN(mdev, "Unable to unregister evt, no chs=0x%08x has been registered.\n", chs);
out:
	spin_unlock_irqrestore(&priv->mhccif_lock, flag);

	return ret;
}

static void mtk_mhccif_mask_evt(struct mtk_md_dev *mdev, u32 chs)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	mtk_pci_write32(mdev, priv->cfg->mhccif_rc_base_addr +
			MHCCIF_EP2RC_SW_INT_EAP_MASK_SET, chs);
}

static void mtk_mhccif_unmask_evt(struct mtk_md_dev *mdev, u32 chs)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	mtk_pci_write32(mdev, priv->cfg->mhccif_rc_base_addr +
			MHCCIF_EP2RC_SW_INT_EAP_MASK_CLR, chs);
}

static void mtk_mhccif_clear_evt(struct mtk_md_dev *mdev, u32 chs)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	mtk_pci_write32(mdev, priv->cfg->mhccif_rc_base_addr +
			MHCCIF_EP2RC_SW_INT_ACK, chs);
}

static int mtk_mhccif_send_evt(struct mtk_md_dev *mdev, u32 ch)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	u32 rc_base;

	rc_base = priv->cfg->mhccif_rc_base_addr;

	/* Only allow one ch to be triggered at a time */
	if (!is_power_of_2(ch)) {
		MTK_ERR(mdev, "Unsupported ext evt ch=0x%08x\n", ch);
		return -EINVAL;
	}

	mtk_pci_write32(mdev, rc_base + MHCCIF_RC2EP_SW_BSY, ch);
	mtk_pci_write32(mdev, rc_base + MHCCIF_RC2EP_SW_TCHNUM, ffs(ch) - 1);
	return 0;
}

static u32 mtk_mhccif_get_evt_status(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	return mtk_pci_read32(mdev, priv->cfg->mhccif_rc_base_addr + MHCCIF_EP2RC_SW_INT_STS);
}

static u32 mtk_mhccif_get_ext_traffic_cfg(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	return mtk_pci_read32(mdev, priv->cfg->mhccif_rc_base_addr + MHCCIF_RC2EP_TRAFFIC_CFG);
}

static u32 mtk_mhccif_get_pm_req_status(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	return mtk_pci_read32(mdev, priv->cfg->mhccif_rc_base_addr + MHCCIF_EP2RC_SPARE_REG_1);
}

static u32 mtk_mhccif_get_pm_resume_user(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	return mtk_pci_read32(mdev, priv->cfg->mhccif_rc_base_addr + MHCCIF_EP2RC_SPARE_REG_5);
}

#if IS_ENABLED(CONFIG_MTK_WWAN_PWRCTL_SUPPORT)
static int mtk_pci_fldr(struct mtk_md_dev *mdev)
{
	return mtk_pwrctl_fldr();
}

static int mtk_pci_pldr(struct mtk_md_dev *mdev)
{
	return mtk_pwrctl_pldr();
}
#else
static int mtk_pci_fldr(struct mtk_md_dev *mdev)
{
#ifdef CONFIG_ACPI
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };
	acpi_status acpi_ret;
	acpi_handle handle;
	int ret = 0;

	if (acpi_disabled) {
		MTK_ERR(mdev, "Unsupported, acpi function isn't enable\n");
		ret = -ENODEV;
		goto err;
	}
	handle = ACPI_HANDLE(mdev->dev);
	if (!handle) {
		MTK_ERR(mdev, "Unsupported, acpi handle isn't found\n");
		ret = -ENODEV;
		goto err;
	}
	if (!acpi_has_method(handle, "_RST")) {
		MTK_ERR(mdev, "Unsupported, _RST method isn't found\n");
		ret = -ENODEV;
		goto err;
	}
	acpi_ret = acpi_evaluate_object(handle, "_RST", NULL, &buffer);
	if (ACPI_FAILURE(acpi_ret)) {
		MTK_ERR(mdev, "Failed to execute _RST method: %s\n",
			acpi_format_exception(acpi_ret));
		ret = -EFAULT;
		goto err;
	}
	MTK_INFO(mdev, "FLDR DONE\n");
	acpi_os_free(buffer.pointer);
err:
	return ret;
#else
	MTK_ERR(mdev, "Unsupported, CONFIG ACPI hasn't been set to 'y'\n");

	return -ENODEV;
#endif
}

static int mtk_pci_pldr(struct mtk_md_dev *mdev)
{
#ifdef CONFIG_ACPI
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };
	struct pci_dev *bridge;
	acpi_status acpi_ret;
	acpi_handle handle;
	int ret = 0;

	if (acpi_disabled) {
		MTK_ERR(mdev, "Unsupported, acpi function isn't enable\n");
		ret = -ENODEV;
		goto err;
	}

	bridge = pci_upstream_bridge(to_pci_dev(mdev->dev));
	if (!bridge) {
		MTK_ERR(mdev, "Unable to find bridge\n");
		ret = -ENODEV;
		goto err;
	}

	handle = ACPI_HANDLE(&bridge->dev);
	if (!handle) {
		MTK_ERR(mdev, "Unsupported, acpi handle isn't found\n");
		ret = -ENODEV;
		goto err;
	}
	if (!acpi_has_method(handle, "PXP._OFF") ||
	    !acpi_has_method(handle, "PXP._ON")) {
		MTK_ERR(mdev, "Unsupported, pldr method isn't supported\n");
		ret = -ENODEV;
		goto err;
	}
	acpi_ret = acpi_evaluate_object(handle, "PXP._OFF", NULL, &buffer);
	if (ACPI_FAILURE(acpi_ret)) {
		MTK_ERR(mdev, "Failed to execute _OFF method: %s\n",
			acpi_format_exception(acpi_ret));
		ret = -EFAULT;
		goto err;
	}
	msleep(500);
	acpi_ret = acpi_evaluate_object(handle, "PXP._ON", NULL, &buffer);
	if (ACPI_FAILURE(acpi_ret)) {
		MTK_ERR(mdev, "Failed to execute _ON method: %s\n",
			acpi_format_exception(acpi_ret));
		ret = -EFAULT;
		goto err;
	}
	MTK_INFO(mdev, "PLDR DONE\n");
	acpi_os_free(buffer.pointer);
err:
	return ret;
#else
	MTK_ERR(mdev, "Unsupported, CONFIG ACPI hasn't been set to 'y'\n");

	return -ENODEV;
#endif
}
#endif /* CONFIG_MTK_WWAN_PWRCTL_SUPPORT */

/*static void mtk_pci_intr_sts_dump(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	MTK_INFO(mdev, "Intr STS: REG_ISTATUS_LOCAL: 0x%x\n"
		"REG_INT_ENABLE_HOST: 0x%x REG_ISTATUS_HOST: 0x%x\n"
		"REG_IMASK_LOCAL: 0x%x REG_MSIX_ISTATUS_HOST_GRP0_0: 0x%x\n"
		"REG_IMASK_HOST_MSIX_GRP0_0: 0x%x REG_ISTATUS_HOST_CTRL: 0x%x\n",
		mtk_pci_mac_read32(priv, REG_ISTATUS_LOCAL),
		mtk_pci_mac_read32(priv, REG_INT_ENABLE_HOST),
		mtk_pci_mac_read32(priv, REG_ISTATUS_HOST),
		mtk_pci_mac_read32(priv, REG_IMASK_LOCAL),
		mtk_pci_mac_read32(priv, REG_MSIX_ISTATUS_HOST_GRP0_0),
		mtk_pci_mac_read32(priv, REG_IMASK_HOST_MSIX_GRP0_0),
		mtk_pci_mac_read32(priv, priv->cfg->istatus_host_ctrl_addr));
}
*/

static u32 mtk_pci_get_dev_cfg(struct mtk_md_dev *mdev)
{
	u32 val;

	val = mtk_pci_mac_read32(mdev->hw_priv, REG_PCIE_DEBUG_DUMMY_4);
	return (val >> MTK_CFG_INFO_BIT_SHIFT);
}

static u32 mtk_pci_get_dev_info(struct mtk_md_dev *mdev)
{
	return mtk_pci_mac_read32(mdev->hw_priv, REG_PCIE_DEBUG_DUMMY_2);
}

static int mtk_pci_reset(struct mtk_md_dev *mdev, enum mtk_reset_type type)
{
	switch (type) {
	case RESET_RGU:
		return mtk_mhccif_send_evt(mdev, EXT_EVT_H2D_DEVICE_RESET);
	case RESET_FLDR:
		return mtk_pci_fldr(mdev);
	case RESET_PLDR:
		return mtk_pci_pldr(mdev);
	default:
		break;
	}

	return -EINVAL;
}

static int mtk_pci_get_dev_log(struct mtk_md_dev *mdev,
			       void *buf, size_t count, enum mtk_dev_log_type type)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	if (priv->cfg->get_dev_log)
		return priv->cfg->get_dev_log(mdev, buf, count, type);

	return -ENODEV;
}

static int mtk_pci_get_log_region_size(struct mtk_md_dev *mdev, enum mtk_dev_log_type type)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	if (priv->cfg->get_log_region_size)
		return priv->cfg->get_log_region_size(mdev, type);

	return -ENODEV;
}

static int mtk_pci_dev_log_buff_init(struct mtk_md_dev *mdev, bool enable)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	if (priv->cfg->dev_log_buff_init)
		return priv->cfg->dev_log_buff_init(mdev, enable);

	return 0;
}

static int mtk_pci_dev_log_buff_exit(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	if (priv->cfg->dev_log_buff_exit)
		return priv->cfg->dev_log_buff_exit(mdev);

	return 0;
}

static void mtk_pci_bridge_configure(struct mtk_md_dev *mdev)
{
	struct pci_dev *pdev, *bridge;
	struct mtk_pci_priv *priv;
	int dpc_cap;
	u16 cap, ctl;

	priv = mdev->hw_priv;
	pdev = to_pci_dev(mdev->dev);

	/* check hotplug capability */
	bridge = pci_upstream_bridge(pdev);
	if (!bridge) {
		MTK_ERR(mdev, "Unable to find bridge\n");
		return;
	}

	priv->rc_hp_on = bridge->is_hotplug_bridge;

	/* Disable DPC Capability */
	dpc_cap = pci_find_ext_capability(bridge, PCI_EXT_CAP_ID_DPC);
	if (dpc_cap) {
		pci_read_config_word(bridge, dpc_cap + PCI_EXP_DPC_CAP, &cap);
		pci_read_config_word(bridge, dpc_cap + PCI_EXP_DPC_CTL, &ctl);
		if (ctl & (PCI_EXP_DPC_CTL_INT_EN | 0x3)) {
			ctl &= ~(PCI_EXP_DPC_CTL_INT_EN | 0x3);
			pci_write_config_word(bridge, dpc_cap + PCI_EXP_DPC_CTL, ctl);
		}
	}

#ifdef CONFIG_PCIEASPM
	/* Re-configure ltr to satisfy remove-rescan case */
	if (bridge->ltr_path) {
		pcie_capability_read_word(bridge, PCI_EXP_DEVCTL2, &ctl);
		if (!(ctl & PCI_EXP_DEVCTL2_LTR_EN))
			pcie_capability_set_word(bridge, PCI_EXP_DEVCTL2, PCI_EXP_DEVCTL2_LTR_EN);
	}
	pcie_capability_read_word(pdev, PCI_EXP_DEVCTL2, &ctl);
	MTK_INFO(mdev, "Called by %ps:Hotplug [%s], LTR_Path[%x/%x], Device CTL2[0x%x] LTR[0x%x]\n",
		 __builtin_return_address(0), priv->rc_hp_on ? "on" : "off", bridge->ltr_path,
		 pdev->ltr_path, ctl, ctl & PCI_EXP_DEVCTL2_LTR_EN);
#endif
}

static int mtk_pci_reinit(struct mtk_md_dev *mdev, enum mtk_reinit_type type)
{
	struct pci_dev *pdev = to_pci_dev(mdev->dev);
	struct mtk_pci_priv *priv = mdev->hw_priv;
	int ret, ltr, l1ss, val;
	u32 bar[BAR_NUM];
	int i;

	mtk_pci_bridge_configure(mdev);

	for (i = 0; i < BAR_NUM; i++)
		pci_read_config_dword(to_pci_dev(mdev->dev),
				      PCI_BASE_ADDRESS_0 + (i << 2), bar + i);

	MTK_INFO(mdev, "BAR0/1/2/3: 0x%08x/0x%08x/0x%08x/0x%08x",
		 bar[0], bar[1], bar[2], bar[3]);

	if (type == REINIT_TYPE_EXP) {
		/* We have saved it in probe() */
		pci_load_saved_state(pdev, priv->saved_state);
		pci_restore_state(pdev);
		val = mtk_pci_get_dev_cfg(mdev);
		MTK_INFO(mdev, "0x%x reinit reboot reason is 0x%x, devdbg infor is 0x%x\n",
			 val, (val & 0x1F), mtk_pci_get_dev_info(mdev));
	}

	/* restore ltr */
	ltr = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_LTR);
	if (ltr) {
		pci_write_config_word(pdev, ltr + PCI_LTR_MAX_SNOOP_LAT,
				      priv->ltr_max_snoop_lat);
		pci_write_config_word(pdev, ltr + PCI_LTR_MAX_NOSNOOP_LAT,
				      priv->ltr_max_nosnoop_lat);
	}
	/* restore l1ss */
	l1ss = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_L1SS);
	if (l1ss) {
		pci_write_config_dword(pdev, l1ss + PCI_L1SS_CTL1, priv->l1ss_ctl1);
		pci_write_config_dword(pdev, l1ss + PCI_L1SS_CTL2, priv->l1ss_ctl2);
	}

	ret = priv->cfg->atr_init(mdev);
	if (ret)
		return ret;

	if (priv->irq_type == PCI_IRQ_MSIX) {
		if (priv->irq_cnt != MTK_IRQ_CNT_MAX)
			mtk_pci_set_msix_merged(priv, priv->irq_cnt);
		if (priv->cfg->flag & MTK_CFG_IRQ_DFLT_MASK) /* mask all L1 level interrupts */
			mtk_pci_mac_write32(priv, REG_IMASK_HOST_MSIX_CLR_GRP0_0, U32_MAX);
	} else if (priv->irq_type == PCI_IRQ_MSI) {
		mtk_pci_set_msi_merged(priv, priv->irq_cnt);
	}

	if (type == REINIT_TYPE_EXP)
		mtk_pci_clear_irq(mdev, priv->rgu_irq_id);

	mtk_pci_unmask_irq(mdev, priv->rgu_irq_id);
	mtk_pci_unmask_irq(mdev, priv->mhccif_irq_id);

	/* In L2 resume, device would disable PCIe interrupt,
	 * and this step would re-enable PCIe interrupt.
	 * For L3, just do this with no effect.
	 */
	if (type == REINIT_TYPE_RESUME)
		mtk_pci_enable_intr(mdev);

	mtk_pcimsg_send_msg_to_user(mdev, MTK_PCIMSG_H2C_READY);

	ret = mtk_pci_dev_log_buff_init(mdev, false);
	if (ret) {
		MTK_ERR(mdev, "Failed to init dev log buff, ret=%d\n", ret);
		return ret;
	}

	MTK_INFO(mdev, "PCIe reinit type=%d, invoked by %ps\n", type, __builtin_return_address(0));

	return 0;
}

static bool mtk_pci_link_check(struct mtk_md_dev *mdev)
{
	u32 vendor_id;
	bool present;

	pci_read_config_dword(to_pci_dev(mdev->dev), PCI_VENDOR_ID, &vendor_id);
	present = pci_device_is_present(to_pci_dev(mdev->dev));
	MTK_INFO(mdev, "vendor id[%x], device present[%d]\n", vendor_id, present);

	return !present;
}

static bool mtk_pci_mmio_check(struct mtk_md_dev *mdev)
{
	struct pci_dev *pdev;
	u16 cmd;
	u32 val;

	pdev = to_pci_dev(mdev->dev);

	pci_read_config_word(pdev, PCI_COMMAND, &cmd);
	if (!(cmd & PCI_COMMAND_MASTER)) {
		MTK_INFO(mdev, "mmio link status :cmd = 0x%x\n", cmd);
		return true;
	}
	val = mtk_pci_mac_read32(mdev->hw_priv, REG_ATR_PCIE_WIN0_T0_SRC_ADDR_LSB);
	if (val == 0xffffffff || val == 0x00000000) {
		MTK_INFO(mdev, "mmio link status cmd = 0x%x, val = 0x%x\n", cmd, val);
		return true;
	}
	return false;
}

static int mtk_pci_get_hp_status(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	return priv->rc_hp_on;
}

static void mtk_pci_dump_msix_tbl(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	struct pci_dev *pdev;
	u32 offset;
	u8 bar_idx;
	int i;

	pdev = to_pci_dev(mdev->dev);
	pci_read_config_dword(pdev, pdev->msix_cap + PCI_MSIX_TABLE, &offset);
	offset &= PCI_MSIX_TABLE_OFFSET;
	bar_idx = (u8)(offset & PCI_MSIX_TABLE_BIR);
	if (bar_idx != 0) {
		MTK_ERR(mdev, "Fail to get MSIX table\n");
		return;
	}

	MTK_INFO(mdev, "MSIX tbl Dump\n");
	for (i = 0; i < 32; i++) {
		MTK_INFO(mdev, "0x%04x: %08x %08x %08x %08x\n", offset,
			 mtk_pci_mac_read32(priv, offset + 0),
			 mtk_pci_mac_read32(priv, offset + 4),
			 mtk_pci_mac_read32(priv, offset + 8),
			 mtk_pci_mac_read32(priv, offset + 12));
		offset += 16;
	}

	// read MSI-X pending bit arrays
	pci_read_config_dword(pdev, pdev->msix_cap + PCI_MSIX_PBA, &offset);
	offset &= PCI_MSIX_PBA_OFFSET;
	bar_idx = (u8)(offset & PCI_MSIX_PBA_BIR);
	if (bar_idx != 0) {
		MTK_ERR(mdev, "Fail to get MSIX PBA\n");
		return;
	}

	MTK_INFO(mdev, "MSIX pending bit array Dump\n");
	MTK_INFO(mdev, "0x%04x: %08x %08x %08x %08x\n", offset,
		mtk_pci_mac_read32(priv, offset + 0),
		mtk_pci_mac_read32(priv, offset + 4),
		mtk_pci_mac_read32(priv, offset + 8),
		mtk_pci_mac_read32(priv, offset + 12));
}

static void mtk_pci_dbg_dump(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	struct pci_dev *pdev;
	u32 l1ss_ctl1 = 0;
	u16 l1ss_cap_ptr;
	u32 bar[BAR_NUM];
	u16 lnkctl;
	u32 i, reg;

	pdev = to_pci_dev(mdev->dev);

	for (i = 0; i < BAR_NUM; i++) {
		reg = PCI_BASE_ADDRESS_0 + (i << 2);
		pci_read_config_dword(pdev, reg, bar + i);
	}

	pci_read_config_word(pdev, pci_pcie_cap(pdev) + PCI_EXP_LNKCTL, &lnkctl);
	l1ss_cap_ptr = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_L1SS);
	if (!l1ss_cap_ptr)
		pci_read_config_dword(pdev, l1ss_cap_ptr + PCI_L1SS_CTL1, &l1ss_ctl1);

	MTK_INFO(mdev, "Start to dump HW infomartion:\n");
	MTK_INFO(mdev, "MAC_REG_Base: 0x%x\n", priv->mac_reg_base);
	MTK_INFO(mdev, "REG_PCIE_LTSSM_STATUS: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_PCIE_LTSSM_STATUS));

	MTK_INFO(mdev, "REG_ISTATUS_LOCAL: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_ISTATUS_LOCAL));

	MTK_INFO(mdev, "REG_RC2EP_SW_TRIG_LOCAL_INTR_MASK: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_RC2EP_SW_TRIG_LOCAL_INTR_MASK));

	MTK_INFO(mdev, "REG_INT_ENABLE_HOST: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_INT_ENABLE_HOST));

	MTK_INFO(mdev, "REG_ISTATUS_HOST: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_ISTATUS_HOST));

	MTK_INFO(mdev, "REG_ISTATUS_PENDING_ADT: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_ISTATUS_PENDING_ADT));

	MTK_INFO(mdev, "REG_IMASK_LOCAL: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_IMASK_LOCAL));

	MTK_INFO(mdev, "REG_DIS_ASPM_LOWPWR_STS_0: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_DIS_ASPM_LOWPWR_STS_0));

	MTK_INFO(mdev, "REG_DIS_ASPM_LOWPWR_STS_1: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_DIS_ASPM_LOWPWR_STS_1));

	MTK_INFO(mdev,	"REG_MSIX_ISTATUS_HOST_GRP0_0: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_MSIX_ISTATUS_HOST_GRP0_0));

	MTK_INFO(mdev,	"REG_IMASK_HOST_MSIX_GRP0_0: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_IMASK_HOST_MSIX_GRP0_0));

	MTK_INFO(mdev,	"REG_ISTATUS_HOST_CTRL: 0x%x\n",
		 mtk_pci_mac_read32(priv, priv->cfg->istatus_host_ctrl_addr));

	MTK_INFO(mdev,	"MHCCIF_EP2RC_SW_INT_STS: 0x%x\n",
		 mtk_mhccif_get_evt_status(mdev));

	MTK_INFO(mdev,	"MHCCIF_EP2RC_SPARE_REG_1: 0x%x\n",
		 mtk_mhccif_get_pm_req_status(mdev));

	MTK_INFO(mdev,	"PCI_EXP_LNKCTL: 0x%x\n", lnkctl);

	MTK_INFO(mdev,	"PCI_L1SS_CTL1: 0x%x\n", l1ss_ctl1);

	MTK_INFO(mdev,	"REG_PCIE_MISC_CTRL: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_PCIE_MISC_CTRL));

	MTK_INFO(mdev,	"REG_PCIE_LOW_POWER_CTRL: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_PCIE_LOW_POWER_CTRL));

	MTK_INFO(mdev,	"REG_PCIE_PEXTP_MAC_SLEEP_CTRL: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_PCIE_PEXTP_MAC_SLEEP_CTRL));

	MTK_INFO(mdev,	"REG_PCIE_AER_UNC_STATUS: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_PCIE_AER_UNC_STATUS));

	MTK_INFO(mdev,	"REG_PCIE_AER_CO_STATUS: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_PCIE_AER_CO_STATUS));

	MTK_INFO(mdev,	"REG_PCIE_AER_CAPCTL: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_PCIE_AER_CAPCTL));

	MTK_INFO(mdev,	"REG_PCIE_ERR_ADDR_L: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_PCIE_ERR_ADDR_L));

	MTK_INFO(mdev,	"REG_PCIE_ERR_ADDR_H: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_PCIE_ERR_ADDR_H));

	MTK_INFO(mdev,	"REG_PCIE_ERR_INFO: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_PCIE_ERR_INFO));
	MTK_INFO(mdev,	"MAC_REG_Base: 0x%llx, id=%d\n", priv->mac_reg_base, REG_PCIE_DEBUG_DUMMY_7);
	MTK_INFO(mdev,	"dummy7 io addr: 0x%llx\n", priv->mac_reg_base+REG_PCIE_DEBUG_DUMMY_7);
	MTK_INFO(mdev,	"EXT_REG_Base: 0x%llx\n", priv->ext_reg_base);

	MTK_INFO(mdev,	"REG_PCIE_DEBUG_DUMMY_7: 0x%x\n",
		 mtk_pci_mac_read32(priv, REG_PCIE_DEBUG_DUMMY_7));


	MTK_INFO(mdev,	"BAR0/1/2/3/4/5: 0x%08x/0x%08x/0x%08x/0x%08x/0x%08x/0x%08x",
		 bar[0], bar[1], bar[2], bar[3], bar[4], bar[5]);
	if (pdev->msix_enabled)
		mtk_pci_dump_msix_tbl(mdev);
}

static void mtk_pci_write_pm_cnt(struct mtk_md_dev *mdev, u32 val)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	mtk_pci_write32(mdev, priv->cfg->mhccif_rc_base_addr
		+ priv->cfg->mhccif_rc2ep_pcie_pm_counter, val);
}

static u32 mtk_pci_get_resume_state(struct mtk_md_dev *mdev)
{
	return mtk_pci_mac_read32(mdev->hw_priv, REG_PCIE_DEBUG_DUMMY_3);
}

static const struct mtk_hw_ops mtk_pci_ops = {
	.read32                = mtk_pci_read32,
	.write32               = mtk_pci_write32,
	.get_dev_state         = mtk_pci_get_dev_state,
	.ack_dev_state         = mtk_pci_ack_dev_state,
	.get_ds_status         = mtk_pci_get_ds_status,
	.ds_lock               = mtk_pci_ds_lock,
	.ds_unlock             = mtk_pci_ds_unlock,
	.set_l1ss              = mtk_pci_set_l1ss,
	.get_resume_state      = mtk_pci_get_resume_state,
	.get_dev_cfg           = mtk_pci_get_dev_cfg,
	.get_irq_id            = mtk_pci_get_irq_id,
	.get_virq_id           = mtk_pci_get_virq_id,
	.register_irq          = mtk_pci_register_irq,
	.unregister_irq        = mtk_pci_unregister_irq,
	.mask_irq              = mtk_pci_mask_irq,
	.unmask_irq            = mtk_pci_unmask_irq,
	.clear_irq             = mtk_pci_clear_irq,
	.reset_sys_irq         = mtk_pci_reset_sys_irq,
	.send_sw_evt           = mtk_pci_send_sw_evt,
	.clear_sw_evt          = mtk_pci_clear_sw_evt,
	.register_ext_evt      = mtk_mhccif_register_evt,
	.unregister_ext_evt    = mtk_mhccif_unregister_evt,
	.mask_ext_evt          = mtk_mhccif_mask_evt,
	.unmask_ext_evt        = mtk_mhccif_unmask_evt,
	.clear_ext_evt         = mtk_mhccif_clear_evt,
	.send_ext_evt          = mtk_mhccif_send_evt,
	.get_ext_evt_status    = mtk_mhccif_get_evt_status,
	.get_ext_traffic_cfg   = mtk_mhccif_get_ext_traffic_cfg,
	.reset                 = mtk_pci_reset,
	.reinit                = mtk_pci_reinit,
	.link_check            = mtk_pci_link_check,
	.mmio_check            = mtk_pci_mmio_check,
	.get_hp_status         = mtk_pci_get_hp_status,
	.dbg_dump              = mtk_pci_dbg_dump,
	.write_pm_cnt          = mtk_pci_write_pm_cnt,
	.get_dev_log           = mtk_pci_get_dev_log,
	.get_log_region_size   = mtk_pci_get_log_region_size,
	.get_pm_req_status     = mtk_mhccif_get_pm_req_status,
	.get_pm_resume_user    = mtk_mhccif_get_pm_resume_user,
};

static void mtk_mhccif_isr_work(struct work_struct *work)
{
	struct mtk_pci_priv *priv = container_of(work, struct mtk_pci_priv, mhccif_work);
	struct mtk_md_dev *mdev = priv->irq_desc->mdev;
	struct mtk_mhccif_cb *cb;
	u32 stat, mask;

	if (priv->cfg->flag & MTK_CFG_DFLT_DISABLE_L1SS)
		mtk_pci_set_l1ss(mdev,
				 L1SS_BIT_L1_1(L1SS_EXT_EVT) | L1SS_BIT_L1_2(L1SS_EXT_EVT),
				 false, L1SS_EXT_EVT);

	stat = mtk_mhccif_get_evt_status(mdev);
	mask = mtk_pci_read32(mdev, priv->cfg->mhccif_rc_base_addr
		+ MHCCIF_EP2RC_SW_INT_EAP_MASK);
	MTK_INFO(mdev, "External events: mhccif_stat=0x%08x mask=0x%08x\n", stat, mask);

	if (unlikely(stat == U32_MAX && mtk_pci_link_check(mdev))) {
		/* When link failed, we don't need to unmask/clear. */
		MTK_ERR(mdev, "Failed to check link in MHCCIF handler.\n");
		mtk_except_report_evt(mdev, EXCEPT_LINK_ERR);
		return;
	}

	stat &= ~mask;
	spin_lock_bh(&priv->mhccif_lock);
	MTK_INFO(mdev, "mtk_mhccif_isr_work list_for_each_entry start");
	list_for_each_entry(cb, &priv->mhccif_cb_list, entry) {
		if (cb->chs & stat)
			cb->evt_cb(cb->chs & stat, cb->data);
	}
	MTK_INFO(mdev, "mtk_mhccif_isr_work list_for_each_entry end\n");
	spin_unlock_bh(&priv->mhccif_lock);

	if (priv->cfg->flag & MTK_CFG_DFLT_DISABLE_L1SS) {
		/* We must use the 1 bit to not conflict with low power */
		mtk_pci_set_l1ss(mdev, L1SS_BIT_L1(L1SS_EXT_EVT), true, L1SS_EXT_EVT);
		stat = mtk_mhccif_get_evt_status(mdev);
		/* At this point, we read MHCCIF not for handling the channels.
		 * So not checking link status couldn't cause critical issue in some corner case.
		 */
		mask = mtk_pci_read32(mdev, priv->cfg->mhccif_rc_base_addr
			+ MHCCIF_EP2RC_SW_INT_EAP_MASK);
		stat &= ~mask;
		if (!stat)
			mtk_pci_set_l1ss(mdev, L1SS_BIT_L1_1(L1SS_EXT_EVT) |
				L1SS_BIT_L1_2(L1SS_EXT_EVT), true, L1SS_EXT_EVT);
	}

	mtk_pci_clear_irq(mdev, priv->mhccif_irq_id);
	mtk_pci_unmask_irq(mdev, priv->mhccif_irq_id);
}

MODULE_DEVICE_TABLE(pci, mtk_pci_ids);

static int mtk_pci_bar_init(struct mtk_md_dev *mdev)
{
	struct pci_dev *pdev = to_pci_dev(mdev->dev);
	struct mtk_pci_priv *priv = mdev->hw_priv;
	u32 bar[BAR_NUM];
	int i, ret;

	for (i = 0; i < BAR_NUM; i++)
		pci_read_config_dword(to_pci_dev(mdev->dev),
				      PCI_BASE_ADDRESS_0 + (i << 2), bar + i);

	ret = pcim_iomap_regions(pdev, MTK_REQUESTED_BARS, mdev->dev_str);
	if (ret) {
		MTK_ERR(mdev, "Failed to init MMIO. ret=%d\n", ret);
		return ret;
	}

	/* get ioremapped memory */
	priv->mac_reg_base = pcim_iomap_table(pdev)[MTK_BAR_0_1_IDX];
	priv->bar23_addr = pcim_iomap_table(pdev)[MTK_BAR_2_3_IDX];
	priv->bar45_addr = pcim_iomap_table(pdev)[MTK_BAR_4_5_IDX];
	if (!priv->mac_reg_base || !priv->bar23_addr || !priv->bar45_addr) {
		MTK_ERR(mdev, "Failed to init BAR.\n");
		return -EINVAL;
	}
	MTK_INFO(mdev, "BAR Addr 0/1:0x%llx, BAR2/3 Addr=0x%llx, BAR4/5 Addr=0x%llx\n",
		 priv->mac_reg_base, priv->bar23_addr, priv->bar45_addr);
	MTK_INFO(mdev, "BAR0~5: 0x%08x/0x%08x/0x%08x/0x%08x/0x%08x/0x%08x\n",
		 bar[0], bar[1], bar[2], bar[3], bar[4], bar[5]);

	/* We use MD view base address "0" to observe registers */
	priv->ext_reg_base = priv->bar23_addr - ATR_PCIE_REG_TRSL_ADDR;

	return 0;
}

static void mtk_pci_bar_exit(struct mtk_md_dev *mdev)
{
	pcim_iounmap_regions(to_pci_dev(mdev->dev), MTK_REQUESTED_BARS);
}

static int mtk_mhccif_irq_cb(int irq_id, void *data)
{
	struct mtk_md_dev *mdev = data;
	struct mtk_pci_priv *priv;

	priv = mdev->hw_priv;
	queue_work(system_highpri_wq, &priv->mhccif_work);

	return 0;
}

static int mtk_mhccif_init(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	u32 tmp_mask;
	int ret;

	INIT_LIST_HEAD(&priv->mhccif_cb_list);
	spin_lock_init(&priv->mhccif_lock);
	INIT_WORK(&priv->mhccif_work, mtk_mhccif_isr_work);

	ret = mtk_pci_get_irq_id(mdev, MTK_IRQ_SRC_MHCCIF);
	if (ret < 0) {
		MTK_ERR(mdev, "Failed to get mhccif_irq_id. ret=%d\n", ret);
		goto err;
	}
	priv->mhccif_irq_id = ret;

	ret = mtk_pci_register_irq(mdev, priv->mhccif_irq_id, mtk_mhccif_irq_cb, mdev);
	if (ret) {
		MTK_ERR(mdev, "Failed to register mhccif_irq callback\n");
		goto err;
	}

	/* To check if the device rebooted.
	 * The reboot of some PC doesn't cause the device power cycle.
	 */
	tmp_mask = mtk_pci_read32(mdev, priv->cfg->mhccif_rc_base_addr +
				   MHCCIF_EP2RC_SW_INT_EAP_MASK);
err:
	return ret;
}

static void mtk_mhccif_exit(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	mtk_pci_unregister_irq(mdev, priv->mhccif_irq_id);
	cancel_work_sync(&priv->mhccif_work);
}

static void mtk_rgu_work(struct work_struct *work)
{
	struct mtk_pci_priv *priv;
	struct mtk_md_dev *mdev;
	struct pci_dev *pdev;
	int ret;

	priv = container_of(to_delayed_work(work), struct mtk_pci_priv, rgu_work);
	mdev = priv->mdev;
	pdev = to_pci_dev(mdev->dev);

	MTK_INFO(mdev, "RGU work\n");

	msleep(300);
	MTK_INFO(mdev, "wait 300ms after RGU work\n");

	mtk_pci_mask_irq(mdev, priv->rgu_irq_id);
	mtk_pci_clear_irq(mdev, priv->rgu_irq_id);

	ret = mtk_except_report_evt(mdev, EXCEPT_RGU);
	if (ret)
		MTK_ERR(mdev, "Failed to report exception with EXCEPT_RGU\n");

	if (priv->cfg->flag & MTK_CFG_RGU_L2_AUTO_ACK &&
	    pdev->msix_enabled)
		mtk_pci_unmask_irq(mdev, priv->rgu_irq_id);
}

static int mtk_rgu_irq_cb(int irq_id, void *data)
{
	struct mtk_md_dev *mdev = data;
	struct mtk_pci_priv *priv;
	struct pci_dev *pdev;

	priv = mdev->hw_priv;
	pdev = to_pci_dev(mdev->dev);

	if (delayed_work_pending(&priv->rgu_work))
		goto exit;

	schedule_delayed_work(&priv->rgu_work, msecs_to_jiffies(1));

exit:
	return 0;
}

static int mtk_rgu_init(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	int ret;

	ret = mtk_pci_get_irq_id(mdev, MTK_IRQ_SRC_SAP_RGU);
	if (ret < 0) {
		MTK_ERR(mdev, "Failed to get rgu_irq_id. ret=%d\n", ret);
		goto err;
	}
	priv->rgu_irq_id = ret;

	INIT_DELAYED_WORK(&priv->rgu_work, mtk_rgu_work);

	mtk_pci_mask_irq(mdev, priv->rgu_irq_id);
	mtk_pci_clear_irq(mdev, priv->rgu_irq_id);

	ret = mtk_pci_register_irq(mdev, priv->rgu_irq_id, mtk_rgu_irq_cb, mdev);
	if (ret) {
		MTK_ERR(mdev, "Failed to register rgu_irq callback\n");
		goto err;
	}

	mtk_pci_unmask_irq(mdev, priv->rgu_irq_id);

err:
	return ret;
}

static void mtk_rgu_exit(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	mtk_pci_unregister_irq(mdev, priv->rgu_irq_id);
	cancel_delayed_work_sync(&priv->rgu_work);
}

static irqreturn_t mtk_pci_irq_handler(struct mtk_md_dev *mdev, u32 irq_state)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	int irq_id;

	/* Check whether each set bit has a callback, if has, call it */
	do {
		irq_id = fls(irq_state) - 1;
		irq_state &= ~BIT(irq_id);

		if (irq_id == 0)
			mdev->irq_timestamp = local_clock();
		if (likely(priv->irq_cb_list[irq_id])) {
			priv->irq_cb_list[irq_id](irq_id, priv->irq_cb_data[irq_id]);
		} else {
			if (mtk_cldma_irq_registed) {
				MTK_ERR(mdev, "Unhandled irq_id=%d, no callback for it.\n", irq_id);
				MTK_BUG(mdev);
			} else {
				mtk_pci_mask_irq(mdev, irq_id);
				mtk_pci_clear_irq(mdev, irq_id);
				MTK_ERR(mdev, "Unhandled irq_id=%d in unregisted flow, ignore it.\n", irq_id);
			}
		}
	} while (irq_state);

	return IRQ_HANDLED;
}

static irqreturn_t mtk_pci_irq_legacy(int irq, void *data)
{
	struct mtk_md_dev *mdev = data;
	struct mtk_pci_priv *priv = mdev->hw_priv;
	u32 irq_state, irq_enable;

	irq_state = mtk_pci_mac_read32(priv, REG_ISTATUS_HOST);
	irq_enable = mtk_pci_mac_read32(priv, REG_INT_ENABLE_HOST);
	irq_state &= irq_enable;

	if (unlikely(!irq_state))
		return IRQ_NONE;

	/* Mask the bit and user needs to unmask by itself */
	mtk_pci_mac_write32(priv, REG_INT_ENABLE_HOST_CLR, irq_state);

	return mtk_pci_irq_handler(mdev, irq_state);
}

static irqreturn_t mtk_pci_irq_msi(int irq, void *data)
{
	struct mtk_pci_irq_desc *irq_desc = data;
	struct mtk_md_dev *mdev = irq_desc->mdev;
	struct mtk_pci_priv *priv;
	u32 irq_state, irq_enable;

	priv = mdev->hw_priv;
	irq_state = mtk_pci_mac_read32(priv, REG_ISTATUS_HOST);
	irq_enable = mtk_pci_mac_read32(priv, REG_INT_ENABLE_HOST);

	irq_state &= irq_enable & irq_desc->msix_bits;

	if (unlikely(!irq_state))
		return IRQ_NONE;

	/* Mask the bit and user needs to unmask by itself */
	mtk_pci_mac_write32(priv, REG_INT_ENABLE_HOST_CLR, irq_state);

	return mtk_pci_irq_handler(mdev, irq_state);
}

static irqreturn_t mtk_pci_irq_msix(int irq, void *data)
{
	struct mtk_pci_irq_desc *irq_desc = data;
	struct mtk_md_dev *mdev = irq_desc->mdev;
	struct mtk_pci_priv *priv;
	u32 irq_state, irq_enable;

	priv = mdev->hw_priv;
	irq_state = mtk_pci_mac_read32(priv, REG_MSIX_ISTATUS_HOST_GRP0_0);
	irq_enable = mtk_pci_mac_read32(priv, REG_IMASK_HOST_MSIX_GRP0_0);

	irq_state &= irq_enable & irq_desc->msix_bits;

	if (unlikely(!irq_state))
		return IRQ_NONE;

	/* Mask the bit and user needs to unmask by itself */
	mtk_pci_mac_write32(priv, REG_IMASK_HOST_MSIX_CLR_GRP0_0, irq_state & ~BIT(30));

	return mtk_pci_irq_handler(mdev, irq_state);
}

static int mtk_pci_request_irq_legacy(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	struct mtk_pci_irq_desc *irq_desc = priv->irq_desc;
	struct pci_dev *pdev = to_pci_dev(mdev->dev);
	int ret;

	snprintf(irq_desc->name, MTK_IRQ_NAME_LEN, "legacy-%s", mdev->dev_str);
	ret = pci_request_irq(pdev, 0, mtk_pci_irq_legacy, NULL,
			      mdev, irq_desc->name);

	if (ret) {
		MTK_ERR(mdev, "Failed to request legacy irq: ret=%d\n", ret);
		return ret;
	}
	irq_desc->mdev = mdev;
	priv->irq_type = PCI_IRQ_LEGACY;
	priv->irq_cnt = MTK_IRQ_CNT_MIN;

	return 0;
}

static int mtk_pci_request_irq_msi(struct mtk_md_dev *mdev, int irq_cnt_allocated)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	struct mtk_pci_irq_desc *irq_desc;
	struct pci_dev *pdev;
	int irq_cnt;
	int ret, i;

	/* calculate the nearest 2's power number */
	irq_cnt = BIT(fls(irq_cnt_allocated) - 1);
	pdev = to_pci_dev(mdev->dev);
	irq_desc = priv->irq_desc;
	if (irq_cnt != irq_cnt_allocated) {
		MTK_INFO(mdev, "%d irqs have been allocated, but only %d irqs are used\n",
			 irq_cnt_allocated, irq_cnt);
	}

	for (i = 0; i < irq_cnt; i++) {
		irq_desc[i].mdev = mdev;
		if (irq_cnt == MTK_IRQ_CNT_MAX)
			irq_desc[i].msix_bits = BIT(i);
		else
			irq_desc[i].msix_bits = mtk_msix_bits_map[i][ffs(irq_cnt) - 1];
		snprintf(irq_desc[i].name, MTK_IRQ_NAME_LEN, "msi%d-%s", i, mdev->dev_str);
		ret = pci_request_irq(pdev, i, mtk_pci_irq_msi, NULL,
				      &irq_desc[i], irq_desc[i].name);
		if (ret) {
			MTK_ERR(mdev, "Failed to request %s: ret=%d\n", irq_desc[i].name, ret);
			for (i--; i >= 0; i--)
				pci_free_irq(pdev, i, &irq_desc[i]);
			return ret;
		}
	}

	priv->irq_cnt = irq_cnt;
	priv->irq_type = PCI_IRQ_MSI;

	if (irq_cnt != MTK_IRQ_CNT_MAX)
		mtk_pci_set_msi_merged(priv, irq_cnt);

	return 0;
}

static int mtk_pci_request_irq_msix(struct mtk_md_dev *mdev, int irq_cnt_allocated)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	struct mtk_pci_irq_desc *irq_desc;
	struct pci_dev *pdev;
	int irq_cnt;
	int ret, i;

	/* calculate the nearest 2's power number */
	irq_cnt = BIT(fls(irq_cnt_allocated) - 1);
	pdev = to_pci_dev(mdev->dev);
	irq_desc = priv->irq_desc;
	if (irq_cnt != irq_cnt_allocated) {
		MTK_INFO(mdev, "%d irqs have been allocated, but only %d irqs are used\n",
			 irq_cnt_allocated, irq_cnt);
	}

	for (i = 0; i < irq_cnt; i++) {
		irq_desc[i].mdev = mdev;
		if (irq_cnt == MTK_IRQ_CNT_MAX)
			irq_desc[i].msix_bits = BIT(i);
		else
			irq_desc[i].msix_bits = mtk_msix_bits_map[i][ffs(irq_cnt) - 1];
		snprintf(irq_desc[i].name, MTK_IRQ_NAME_LEN, "msix%d-%s", i, mdev->dev_str);
		ret = pci_request_irq(pdev, i, mtk_pci_irq_msix, NULL,
				      &irq_desc[i], irq_desc[i].name);
		if (ret) {
			MTK_ERR(mdev, "Failed to request %s: ret=%d\n", irq_desc[i].name, ret);
			for (i--; i >= 0; i--)
				pci_free_irq(pdev, i, &irq_desc[i]);
			return ret;
		}
	}
	priv->irq_cnt = irq_cnt;
	priv->irq_type = PCI_IRQ_MSIX;

	if (irq_cnt != MTK_IRQ_CNT_MAX)
		mtk_pci_set_msix_merged(priv, irq_cnt);

	return 0;
}

static int mtk_pci_request_irq(struct mtk_md_dev *mdev, int max_irq_cnt, int irq_type)
{
	struct pci_dev *pdev = to_pci_dev(mdev->dev);
	int irq_cnt;
	int ret;

	if (max_irq_cnt < MTK_IRQ_CNT_MIN || max_irq_cnt > MTK_IRQ_CNT_MAX) {
		ret = -EINVAL;
		goto err;
	}

	irq_type &= PCI_IRQ_MSIX | PCI_IRQ_MSI | PCI_IRQ_LEGACY;

	if (!irq_type) {
		MTK_ERR(mdev, "Invalid irq type\n");
		ret = -EINVAL;
		goto err;
	}

	irq_cnt = pci_alloc_irq_vectors(pdev, MTK_IRQ_CNT_MIN, max_irq_cnt, irq_type);
	mdev->msi_nvecs = irq_cnt;

	if (irq_cnt < MTK_IRQ_CNT_MIN) {
		MTK_ERR(mdev,
			"Unable to alloc pci irq vectors. ret=%d maxirqcnt=%d irqtype=0x%x",
			irq_cnt, max_irq_cnt, irq_type);
		ret = -EFAULT;
		goto err;
	}

	if (pdev->msix_enabled)
		ret = mtk_pci_request_irq_msix(mdev, irq_cnt);
	else if (pdev->msi_enabled)
		ret = mtk_pci_request_irq_msi(mdev, irq_cnt);
	else
		ret = mtk_pci_request_irq_legacy(mdev);

err:
	return ret;
}

static void mtk_pci_free_irq(struct mtk_md_dev *mdev)
{
	struct pci_dev *pdev = to_pci_dev(mdev->dev);
	struct mtk_pci_priv *priv = mdev->hw_priv;
	int i;

	if (priv->irq_type == PCI_IRQ_MSIX || priv->irq_type == PCI_IRQ_MSI)
		for (i = 0; i < priv->irq_cnt; i++)
			pci_free_irq(pdev, i, &priv->irq_desc[i]);
	else
		pci_free_irq(pdev, 0, mdev);

	pci_free_irq_vectors(pdev);
}

static int __mtk_str_begin_with(const char *str, const char *begin)
{
	return !strncmp(str, begin, strlen(begin));
}

static ssize_t mtk_pci_dbg_write_reset(void *data, const char *buf, ssize_t cnt)
{
	struct mtk_md_dev *mdev = data;
	enum mtk_reset_type type;
	int ret;

	if (__mtk_str_begin_with(buf, "rgu"))
		type = RESET_RGU;
	else if (__mtk_str_begin_with(buf, "fldr"))
		type = RESET_FLDR;
	else if (__mtk_str_begin_with(buf, "pldr"))
		type = RESET_PLDR;
	else
		return -EINVAL;

	ret = mtk_pci_reset(mdev, type);
	MTK_INFO(mdev, "Reset done, buf=%s, ret=%d\n", buf, ret);
	if (ret)
		return ret;

	return cnt;
}

MTK_DBGFS(reset, NULL, mtk_pci_dbg_write_reset); /* note that */

static ssize_t mtk_pci_dbg_write_reinit(void *data, const char *buf, ssize_t cnt)
{
	struct mtk_md_dev *mdev = data;
	enum mtk_reinit_type type;
	int ret;

	if (__mtk_str_begin_with(buf, "exp"))
		type = REINIT_TYPE_EXP;
	else if (__mtk_str_begin_with(buf, "resume"))
		type = REINIT_TYPE_RESUME;
	else
		return -EINVAL;

	ret = mtk_pci_reinit(mdev, type);
	MTK_INFO(mdev, "Reinit done, buf=%s, ret = %d\n", buf, ret);
	if (ret)
		return ret;

	return cnt;
}

MTK_DBGFS(reinit, NULL, mtk_pci_dbg_write_reinit); /* note that */

static ssize_t mtk_pci_dbg_read_link(void *data, char *buf, ssize_t max_cnt)
{
	struct mtk_md_dev *mdev = data;
	struct pci_dev *pdev = to_pci_dev(mdev->dev);
	bool ret, tmp;
	u16 vend;

	pci_read_config_word(pdev, PCI_VENDOR_ID, &vend);

	ret = mtk_pci_link_check(mdev);
	tmp = vend != pdev->vendor;

	MTK_INFO(mdev, "ret=%d tmp=%d vend=0x%04X mdev=%p\n", ret, tmp, vend, mdev);

	return snprintf(buf, max_cnt, "link %s\n", ret ? "down" : "ok");
}

MTK_DBGFS(link, mtk_pci_dbg_read_link, NULL); /* note that */

static ssize_t mtk_pci_dbg_read_hp(void *data, char *buf, ssize_t max_cnt)
{
	struct mtk_md_dev *mdev = data;
	struct mtk_pci_priv *priv = mdev->hw_priv;

	return snprintf(buf, max_cnt, "%s\n", priv->rc_hp_on ? "on" : "off");
}

MTK_DBGFS(hp, mtk_pci_dbg_read_hp, NULL); /* note that */

static ssize_t mtk_pci_dbg_write_dump(void *data, const char *buf, ssize_t cnt)
{
	struct mtk_md_dev *mdev = data;

	if (__mtk_str_begin_with(buf, "dump"))
		mtk_pci_dbg_dump(mdev);

	return cnt;
}

MTK_DBGFS(pci_dbg, NULL, mtk_pci_dbg_write_dump); /* note that */

static ssize_t mtk_pci_dbg_link_ctrl(void *data, const char *buf, ssize_t cnt)
{
	struct mtk_md_dev *mdev = data;
	struct mtk_pci_priv *priv;

	/* trigger EP link failure */
	if (__mtk_str_begin_with(buf, "linkdown")) {
		priv = mdev->hw_priv;
		iowrite32(0xF, priv->mac_reg_base + (u64)0x148);
		MTK_INFO(mdev, "trigger link down done\n");
	}

	return cnt;
}

MTK_DBGFS(link_ctrl, NULL, mtk_pci_dbg_link_ctrl); /* note that */

static void mtk_pci_dbgfs_init(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	struct dentry *dentry;

	dentry = mtk_dbgfs_create_dir(mtk_get_dev_dentry(mdev), "pci");
	if (IS_ERR_OR_NULL(dentry))
		return;
	priv->dentry = dentry;

	mtk_dbgfs_create_file(dentry, &mtk_dbgfs_reset, mdev);
	mtk_dbgfs_create_file(dentry, &mtk_dbgfs_reinit, mdev);
	mtk_dbgfs_create_file(dentry, &mtk_dbgfs_link, mdev);
	mtk_dbgfs_create_file(dentry, &mtk_dbgfs_hp, mdev);
	mtk_dbgfs_create_file(dentry, &mtk_dbgfs_pci_dbg, mdev);
	mtk_dbgfs_create_file(dentry, &mtk_dbgfs_link_ctrl, mdev);
}

static void mtk_pci_dbgfs_exit(struct mtk_md_dev *mdev)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;

	mtk_dbgfs_remove(priv->dentry);
}

static void mtk_pci_save_state(struct mtk_md_dev *mdev)
{
	struct pci_dev *pdev = to_pci_dev(mdev->dev);
	struct mtk_pci_priv *priv = mdev->hw_priv;
	int ltr, l1ss;

	pci_save_state(pdev);
	/* save ltr */
	ltr = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_LTR);
	if (ltr) {
		pci_read_config_word(pdev, ltr + PCI_LTR_MAX_SNOOP_LAT,
				     &priv->ltr_max_snoop_lat);
		pci_read_config_word(pdev, ltr + PCI_LTR_MAX_NOSNOOP_LAT,
				     &priv->ltr_max_nosnoop_lat);
	}
	/* save l1ss */
	l1ss = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_L1SS);
	if (l1ss) {
		pci_read_config_dword(pdev, l1ss + PCI_L1SS_CTL1, &priv->l1ss_ctl1);
		pci_read_config_dword(pdev, l1ss + PCI_L1SS_CTL2, &priv->l1ss_ctl2);
	}
}

static void mtk_pci_restore_aspm_l1ss(struct mtk_md_dev *mdev)
{
	struct pci_dev *pdev = to_pci_dev(mdev->dev);
	u32 up_cap_ptr, up_cap, up_l1ss_ctl1;
	u32 dw_cap_ptr, dw_cap, dw_l1ss_ctl1;
	struct pci_dev *bridge;
	u32 l1ss_enable;

	bridge = pci_upstream_bridge(pdev);
	if (!bridge) {
		MTK_ERR(mdev, "Unable to find bridge!\n");
		return;
	}

	up_cap_ptr = pci_find_ext_capability(bridge, PCI_EXT_CAP_ID_L1SS);
	dw_cap_ptr = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_L1SS);
	if (!up_cap_ptr || !dw_cap_ptr)
		return;

	pci_read_config_dword(bridge, up_cap_ptr + PCI_L1SS_CAP, &up_cap);
	pci_read_config_dword(pdev, dw_cap_ptr + PCI_L1SS_CAP, &dw_cap);
	pci_read_config_dword(bridge, up_cap_ptr + PCI_L1SS_CTL1, &up_l1ss_ctl1);
	pci_read_config_dword(pdev, dw_cap_ptr + PCI_L1SS_CTL1, &dw_l1ss_ctl1);

	l1ss_enable = PCI_L1SS_CTL1_L1SS_MASK & up_cap & dw_cap & up_l1ss_ctl1;
	dw_l1ss_ctl1 |= l1ss_enable;

	pci_write_config_dword(pdev, dw_cap_ptr + PCI_L1SS_CTL1, dw_l1ss_ctl1);
}

#ifdef CONFIG_PCIEASPM
#ifndef PCI_EXP_LNKCAP_ASPM_L1
#define PCI_EXP_LNKCAP_ASPM_L1    (0x00000800) /* ASPM L1 support */
#endif
#ifndef PCI_EXP_LNKCAP_ASPM_L0S
#define PCI_EXP_LNKCAP_ASPM_L0S   (0x00000400) /* ASPM L0S support */
#endif

static void mtk_pci_set_aspm(struct pci_dev *dev, int config, bool is_l1)
{
	u16 lnkctl;

	pci_read_config_word(dev, pci_pcie_cap(dev) + PCI_EXP_LNKCTL, &lnkctl);
	if (is_l1)
		lnkctl &= ~PCI_EXP_LNKCTL_ASPM_L1;
	else
		lnkctl &= ~PCI_EXP_LNKCTL_ASPM_L0S;
	lnkctl |= config;
	pci_write_config_word(dev, pci_pcie_cap(dev) + PCI_EXP_LNKCTL, lnkctl);
}

static void mtk_pci_config_aspm(struct mtk_md_dev *mdev, int config, bool is_l1)
{
	u32 child_lnkcap = 0, parent_lnkcap;
	struct pci_dev *child, *parent;

	child = to_pci_dev(mdev->dev);
	parent = pci_upstream_bridge(child);

	if (!parent) {
		MTK_ERR(mdev, "Upstream bridge is NULL\n");
		return;
	}

	pci_read_config_dword(parent, pci_pcie_cap(parent) + PCI_EXP_LNKCAP, &parent_lnkcap);
	if (!is_l1 && (parent_lnkcap & PCI_EXP_LNKCAP_ASPM_L0S)) {
		pci_read_config_dword(child, pci_pcie_cap(child) + PCI_EXP_LNKCAP, &child_lnkcap);
		if (child_lnkcap & PCI_EXP_LNKCAP_ASPM_L0S) {
			mtk_pci_set_aspm(parent, config, false);
			mtk_pci_set_aspm(child, config, false);
			return;
		}
	} else if (is_l1 && (parent_lnkcap & PCI_EXP_LNKCAP_ASPM_L1)) {
		pci_read_config_dword(child, pci_pcie_cap(child) + PCI_EXP_LNKCAP, &child_lnkcap);
		if (child_lnkcap & PCI_EXP_LNKCAP_ASPM_L1) {
			mtk_pci_set_aspm(parent, config, true);
			mtk_pci_set_aspm(child, config, true);
			return;
		}
	}
	MTK_WARN(mdev, "RP Link cap:0x%x, EP Link cap:0x%x\n", parent_lnkcap, child_lnkcap);
}

static bool mtk_pci_set_aspm_l1ss(struct pci_dev *dev, int config)
{
	u16 l1ss_cap_ptr;
	u32 l1ss_ctl1 = 0;

	l1ss_cap_ptr = pci_find_ext_capability(dev, PCI_EXT_CAP_ID_L1SS);
	if (!l1ss_cap_ptr)
		return false;

	pci_read_config_dword(dev, l1ss_cap_ptr + PCI_L1SS_CTL1, &l1ss_ctl1);
	l1ss_ctl1 &= ~PCI_L1SS_CTL1_L1SS_MASK;
	l1ss_ctl1 |= config;
	pci_write_config_dword(dev, l1ss_cap_ptr + PCI_L1SS_CTL1, l1ss_ctl1);

	return true;
}

static bool mtk_pci_check_aspm_l1ss(struct pci_dev *dev, int config)
{
	u16 l1ss_cap_ptr;
	u32 l1ss_cap;

	l1ss_cap_ptr = pci_find_ext_capability(dev, PCI_EXT_CAP_ID_L1SS);
	if (!l1ss_cap_ptr)
		return false;

	pci_read_config_dword(dev, l1ss_cap_ptr + PCI_L1SS_CAP, &l1ss_cap);

	if (config) {
		if (config & PCI_L1SS_CTL1_ASPM_L1_1) {
			if (!(l1ss_cap & PCI_L1SS_CAP_ASPM_L1_1))
				return false;
		}

		if (config & PCI_L1SS_CTL1_ASPM_L1_2) {
			if (!(l1ss_cap & PCI_L1SS_CAP_ASPM_L1_2))
				return false;
		}
	}

	return true;
}

static bool mtk_pci_config_aspm_l1ss(struct mtk_md_dev *mdev, int config)
{
	struct pci_dev *child = to_pci_dev(mdev->dev);
	struct pci_dev *parent = pci_upstream_bridge(child);

	if (!parent) {
		MTK_ERR(mdev, "Upstream bridge is NULL\n");
		return false;
	}

	if (mtk_pci_check_aspm_l1ss(parent, config)) {
		if (mtk_pci_check_aspm_l1ss(child, config)) {
			mtk_pci_set_aspm_l1ss(parent, config);
			mtk_pci_set_aspm_l1ss(child, config);
			return true;
		}
	}
	MTK_WARN(mdev, "ASPM L1ss RP support:%d, EP support:%d\n",
		 mtk_pci_check_aspm_l1ss(parent, config),
		 mtk_pci_check_aspm_l1ss(child, config));

	return false;
}
#endif


extern void xiaomi_modem_power_irq_register(void);
extern void xiaomi_modem_power_irq_unregister(void);
extern void t800_send_platform_uevent(int id, int sub_id, int value1, int value2, int value3);
struct mtk_md_dev *g_t800_mdev;

static inline void mtk_pci_enable_aspm(struct mtk_md_dev *mdev)
{
#ifdef CONFIG_PCIEASPM
	mtk_pci_config_aspm(mdev, PCI_EXP_LNKCTL_ASPM_L0S, false);
	mtk_pci_config_aspm(mdev, 0, true);
	mtk_pci_config_aspm_l1ss(mdev, PCI_L1SS_CTL1_ASPM_L1_1 | PCI_L1SS_CTL1_ASPM_L1_2);
	mtk_pci_config_aspm(mdev, PCI_EXP_LNKCTL_ASPM_L1, true);
#endif
}

static int mtk_pci_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct device *dev = &pdev->dev;
	struct mtk_pci_priv *priv;
	struct mtk_md_dev *mdev;
	int ret;
	xiaomi_modem_power_irq_register();
	mdev = devm_kzalloc(dev, sizeof(*mdev), GFP_KERNEL);
	if (!mdev) {
		ret = -ENOMEM;
		goto exit;
	}

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		ret = -ENOMEM;
		goto free_cntx_data;
	}
       g_t800_mdev = mdev; //add by xiaomi for optimize
	pci_set_drvdata(pdev, mdev);
	priv->cfg = (void *)id->driver_data;
	priv->mdev = mdev;
	mdev->hw_ver  = pdev->device;
	mdev->hw_ops  = &mtk_pci_ops;
	mdev->hw_priv = priv;
	mdev->dev     = dev;
	snprintf(mdev->dev_str, MTK_DEV_STR_LEN, "%02x%02x%d",
		 pdev->bus->number, PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn));
	spin_lock_init(&priv->mac_active_lock);

	mtk_pci_bridge_configure(mdev);

	MTK_INFO(mdev, "Start probe 0x%x, state_saved[%d]\n",
		 mdev->hw_ver, pdev->state_saved);

	if (pdev->state_saved) {
		MTK_INFO(mdev, "Restoring configuration space\n"); /* note that */
		mtk_pci_restore_aspm_l1ss(mdev);
		pci_restore_state(pdev);
	}

	/* enable ASPM L0s L1 L1ss */
	mtk_pci_enable_aspm(mdev);

	/* enable host to device access. */
	ret = pcim_enable_device(pdev);
	if (ret) {
		MTK_ERR(mdev, "Failed to enable pci device.\n");
		goto free_priv_data;
	}

	ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
	if (ret) {
		dev_err(mdev->dev, "Failed to set DMA Mask and Coherent. (ret=%d)\n", ret);
		goto disable_device;
	}

	ret = mtk_pci_bar_init(mdev);
	if (ret)
		goto disable_device;

	ret = priv->cfg->atr_init(mdev);
	if (ret)
		goto free_bar;

	ret = mtk_mhccif_init(mdev);
	if (ret)
		goto free_bar;

	/* mask all irqs */
	if (priv->cfg->flag & MTK_CFG_IRQ_DFLT_MASK)
		mtk_pci_mac_write32(priv, REG_IMASK_HOST_MSIX_CLR_GRP0_0, U32_MAX);

	ret = mtk_pci_request_irq(mdev, mtk_pci_irq_cnt_max, mtk_pci_irq_type);
	if (ret)
		goto free_mhccif;

	ret = mtk_dev_init(mdev);
	if (ret) {
		MTK_ERR(mdev, "Failed to init dev.\n");
		goto free_irq;
	}

	ret = mtk_rgu_init(mdev);
	if (ret)
		goto free_device;

	/* enable device to host interrupt. */
	pci_set_master(pdev);

	mtk_pci_unmask_irq(mdev, priv->mhccif_irq_id);

	mtk_pci_dbgfs_init(mdev);

	//pci_enable_pcie_error_reporting(pdev);
	mtk_pci_save_state(mdev);
	priv->saved_state = pci_store_saved_state(pdev);
	if (!priv->saved_state) {
		ret = -EFAULT;
		goto clear_master_and_rgu;
	}

	ret = mtk_dev_start(mdev);
	if (ret) {
		MTK_ERR(mdev, "Failed to start dev.\n");
		goto free_saved_state;
	}

	ret = mtk_pcimsg_messenger_init(mdev);
	if (ret)
		MTK_ERR(mdev, "Failed to init messenger\n");

	ret = mtk_pci_dev_log_buff_init(mdev, false);
	if (ret) {
		MTK_ERR(mdev, "Failed to init dev log buff, ret=%d\n");
		goto free_saved_state;
	}

	MTK_INFO(mdev, "Probe done hw_ver=0x%x with irq type %d\n", mdev->hw_ver, mtk_pci_irq_type);

	return 0;

free_saved_state:
	pci_load_and_free_saved_state(pdev, &priv->saved_state);
clear_master_and_rgu:
	mtk_pci_dbgfs_exit(mdev);
	pci_clear_master(pdev);
	mtk_rgu_exit(mdev);
free_device:
	mtk_dev_exit(mdev);
free_irq:
	mtk_pci_free_irq(mdev);
free_mhccif:
	mtk_mhccif_exit(mdev);
free_bar:
	mtk_pci_bar_exit(mdev);
disable_device:
	pci_disable_device(pdev);
free_priv_data:
	devm_kfree(dev, priv);
free_cntx_data:
	devm_kfree(dev, mdev);
exit:
	dev_err(dev, "Failed to probe device, ret=%d\n", ret);
/*********************add by xiaomi for optimize start*******************************/
	g_t800_mdev = NULL; //add by xiaomi for optimize
	extern bool g_t800_recovery_ongoing_flag;
	if(!g_t800_recovery_ongoing_flag) {
		t800_send_platform_uevent(4/*MTK_UEVENT_RECOVERY*/, 6/*MODEM_RECOVERY_PROBE_FAIL*/, 6, 0, 0);
	}
/*********************add by xiaomi for optimize end*******************************/
	return ret;
}

static void mtk_pci_remove(struct pci_dev *pdev)
{
	struct mtk_md_dev *mdev = pci_get_drvdata(pdev);
	struct mtk_pci_priv *priv = mdev->hw_priv;
	struct device *dev = &pdev->dev;
	//xiaomi_modem_power_irq_unregister();  // move it from pcie host drvier to pwrctl for only unregister irq in power off status
	g_t800_mdev = NULL; //add by xiaomi for optimize
	mtk_pci_mask_irq(mdev, priv->rgu_irq_id);
	mtk_pci_mask_irq(mdev, priv->mhccif_irq_id);
	mtk_pci_dbgfs_exit(mdev);
	mtk_pci_dev_log_buff_exit(mdev);
	mtk_pcimsg_messenger_exit(mdev);
	mtk_dev_exit(mdev);

#if !defined(CONFIG_MTK_WWAN_PWRCTL_SUPPORT)
	if (priv->cfg->flag & MTK_CFG_DRM_DISABLE_AP) {
		mtk_mhccif_send_evt(mdev, EXT_EVT_H2D_DRM_DISABLE_AP);
		/* Sleep to wait the DRM disable takes effect. */
		msleep(200);
	}

	if (mtk_pci_pldr(mdev)) {
		dev_info(dev, "Failed to execute PLDR , try external event\n");
		mtk_mhccif_send_evt(mdev, EXT_EVT_H2D_DEVICE_RESET);
	}
#endif

	pci_clear_master(pdev);
	mtk_rgu_exit(mdev);
	mtk_mhccif_exit(mdev);
	mtk_pci_free_irq(mdev);
	mtk_pci_bar_exit(mdev);
	pci_disable_device(pdev);
	pci_load_and_free_saved_state(pdev, &priv->saved_state);
	devm_kfree(dev, priv);
	devm_kfree(dev, mdev);
	dev_info(dev, "Remove done, state_saved[%d]\n", pdev->state_saved);
}

#if !defined(CONFIG_MTK_WWAN_PWRCTL_SUPPORT)
static void mtk_pci_ltr_dbg(struct pci_dev *pdev)
{
#ifdef CONFIG_PCIEASPM
	struct mtk_md_dev *mdev = pci_get_drvdata(pdev);
	struct pci_dev *bridge;
	u16 ctl;

	bridge = pci_upstream_bridge(pdev);
	if (!bridge) {
		MTK_ERR(mdev, "Unable to find bridge\n");
		return;
	}
	if (bridge->ltr_path) {
		pcie_capability_read_word(bridge, PCI_EXP_DEVCTL2, &ctl);
		if (!(ctl & PCI_EXP_DEVCTL2_LTR_EN))
			pcie_capability_set_word(bridge, PCI_EXP_DEVCTL2, PCI_EXP_DEVCTL2_LTR_EN);
		MTK_DBG(mdev, MTK_DBG_PCIE, MTK_MEMLOG_RG_0, "DEVCTL2=0x%x, LTR_En=0x%x\n",
			ctl, ctl & PCI_EXP_DEVCTL2_LTR_EN);
	}
	pcie_capability_read_word(pdev, PCI_EXP_DEVCTL2, &ctl);
	MTK_INFO(mdev, "Bridge LTR_Path %x/%x, Device DEVCTL2=0x%x, LTR_En=0x%x\n",
		 bridge->ltr_path, pdev->ltr_path, ctl, ctl & PCI_EXP_DEVCTL2_LTR_EN);
#endif
}

static pci_ers_result_t mtk_pci_slot_reset(struct pci_dev *pdev)
{
	struct mtk_md_dev *mdev = pci_get_drvdata(pdev);
	int ret;

	ret = mtk_except_report_evt(mdev, EXCEPT_AER_RESET);
	if (ret)
		MTK_ERR(mdev, "Failed to call excpetion report API with EXCEPT_AER_RESET!\n");

	return PCI_ERS_RESULT_RECOVERED;
}

static void mtk_pci_io_resume(struct pci_dev *pdev)
{
	struct mtk_md_dev *mdev = pci_get_drvdata(pdev);
	int ret;

	ret = mtk_except_report_evt(mdev, EXCEPT_AER_RESUME);
	if (ret)
		MTK_ERR(mdev, "Failed to call excpetion report API with EXCEPT_AER_RESUME!\n");
}
#endif

static pci_ers_result_t mtk_pci_error_detected(struct pci_dev *pdev,
					       pci_channel_state_t state)
{
#if !defined(CONFIG_MTK_WWAN_PWRCTL_SUPPORT)
	struct mtk_md_dev *mdev = pci_get_drvdata(pdev);
	int ret;

	ret = mtk_except_report_evt(mdev, EXCEPT_AER_DETECTED);
	if (ret)
		MTK_ERR(mdev, "Failed to call excpetion report API with EXCEPT_AER_DETECTED!\n");
	MTK_INFO(mdev, "AER detected: pci_channel_state_t=%d\n", state);

	mtk_pci_ltr_dbg(pdev);
#else
	struct mtk_md_dev *mdev = pci_get_drvdata(pdev);

	MTK_ERR(mdev, "AER detected: pci_channel_state_t=%d\n", state);
	//mtk_pcie_disable_data_trans(MTK_PCIE_PORT_NUM);
	xring_pcie_disable_data_trans(0);
#endif
	/* Request a slot reset. */
	return PCI_ERS_RESULT_NEED_RESET;
}

static const struct pci_error_handlers mtk_pci_err_handler = {
	.error_detected = mtk_pci_error_detected,
#if !defined(CONFIG_MTK_WWAN_PWRCTL_SUPPORT)
	.slot_reset = mtk_pci_slot_reset,
	.resume = mtk_pci_io_resume,
#endif
};

static bool mtk_pci_check_init_status(struct mtk_md_dev *mdev)
{
	if (mtk_pci_mac_read32(mdev->hw_priv, REG_ATR_PCIE_WIN0_T0_SRC_ADDR_LSB) ==
		ATR_WIN0_SRC_ADDR_LSB_DEFT)
		/* Device reboots and isn't configured ATR, so it is default value. */
		return TRUE;
	return FALSE;
}

static int __maybe_unused mtk_pci_pm_prepare(struct device *dev)
{
	return mtk_pm_prepare(dev);
}

static int __maybe_unused mtk_pci_pm_suspend(struct device *dev)
{
	return mtk_pm_suspend(dev);
}

static int __maybe_unused mtk_pci_pm_suspend_noirq(struct device *dev)
{
	return mtk_pm_suspend_noirq(dev);
}

static int __maybe_unused mtk_pci_pm_resume(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct mtk_md_dev *mdev;
	bool atr_init;

	mdev = pci_get_drvdata(pdev);
	atr_init = mtk_pci_check_init_status(mdev);

	return mtk_pm_resume(dev, atr_init);
}

static int __maybe_unused mtk_pci_pm_resume_noirq(struct device *dev)
{
	return mtk_pm_resume_noirq(dev);
}

static int __maybe_unused mtk_pci_pm_freeze(struct device *dev)
{
	return mtk_pm_freeze(dev);
}

static int __maybe_unused mtk_pci_pm_freeze_noirq(struct device *dev)
{
	return mtk_pm_freeze_noirq(dev);
}

static int __maybe_unused mtk_pci_pm_thaw(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct mtk_md_dev *mdev;
	bool atr_init;

	mdev = pci_get_drvdata(pdev);
	atr_init = mtk_pci_check_init_status(mdev);

	return mtk_pm_thaw(dev, atr_init);
}

static int __maybe_unused mtk_pci_pm_thaw_noirq(struct device *dev)
{
	return mtk_pm_thaw_noirq(dev);
}

static int __maybe_unused mtk_pci_pm_poweroff(struct device *dev)
{
	return mtk_pm_poweroff(dev);
}

static int __maybe_unused mtk_pci_pm_restore(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct mtk_md_dev *mdev;
	bool atr_init;

	mdev = pci_get_drvdata(pdev);
	atr_init = mtk_pci_check_init_status(mdev);

	return mtk_pm_restore(dev, atr_init);
}

static int __maybe_unused mtk_pci_pm_runtime_suspend(struct device *dev)
{
	return mtk_pm_runtime_suspend(dev);
}

static int __maybe_unused mtk_pci_pm_runtime_resume(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct mtk_md_dev *mdev;
	bool atr_init;

	mdev = pci_get_drvdata(pdev);
	atr_init = mtk_pci_check_init_status(mdev);

	return mtk_pm_runtime_resume(dev, atr_init);
}

static int __maybe_unused mtk_pci_pm_runtime_idle(struct device *dev)
{
	return mtk_pm_runtime_idle(dev);
}

static void mtk_pci_pm_shutdown(struct pci_dev *pdev)
{
	struct mtk_md_dev *mdev;

	mdev = pci_get_drvdata(pdev);

	return mtk_pm_shutdown(mdev);
}

static const struct dev_pm_ops mtk_pci_pm_ops = {
	.prepare = mtk_pci_pm_prepare,
	.suspend = mtk_pci_pm_suspend,
	.suspend_noirq = mtk_pci_pm_suspend_noirq,
	.resume = mtk_pci_pm_resume,
	.resume_noirq = mtk_pci_pm_resume_noirq,
	.freeze = mtk_pci_pm_freeze,
	.freeze_noirq = mtk_pci_pm_freeze_noirq,
	.restore = mtk_pci_pm_restore,

	SET_RUNTIME_PM_OPS(mtk_pci_pm_runtime_suspend, mtk_pci_pm_runtime_resume,
			   mtk_pci_pm_runtime_idle)
};

static struct pci_driver mtk_pci_drv = {
	.name = "mtk_pci_drv",
	.id_table = mtk_pci_ids,

	.probe =    mtk_pci_probe,
	.remove =   mtk_pci_remove,
	.driver.pm = &mtk_pci_pm_ops,
	.shutdown = mtk_pci_pm_shutdown,

	.err_handler = &mtk_pci_err_handler
};

static int __init mtk_drv_init(void)
{
	int ret;

	ret = mtk_port_io_init();
	if (ret)
		goto exit;

	mtk_drv_dbgfs_init();
	mtk_netlink_init();

	ret = pci_register_driver(&mtk_pci_drv);
	if (ret)
		goto free_and_exit;

	return 0;
free_and_exit:
	mtk_netlink_uninit();
	mtk_drv_dbgfs_exit();
	mtk_port_io_exit();
exit:

	return ret;
}
module_init(mtk_drv_init);
extern void xiaomi_release_dpmaif_memory(void);
static void __exit mtk_drv_exit(void)
{
	pci_unregister_driver(&mtk_pci_drv);
	mtk_netlink_uninit();
	mtk_drv_dbgfs_exit();
	mtk_port_io_exit();
	mtk_port_stale_list_grp_cleanup();
	xiaomi_release_dpmaif_memory();
}
module_exit(mtk_drv_exit);

module_param(mtk_pci_irq_cnt_max, int, 0644);
MODULE_PARM_DESC(mtk_pci_irq_cnt_max, "The maximum number of irqs requested");

module_param(mtk_pci_irq_type, int, 0644);
MODULE_PARM_DESC(mtk_pci_irq_type, "The type of irqs requested");

module_param(mtk_pci_ds_lock_debug, bool, 0644);
MODULE_PARM_DESC(mtk_pci_ds_lock_debug, "Enable PCIe deep sleep debug information");
MODULE_LICENSE("GPL");
