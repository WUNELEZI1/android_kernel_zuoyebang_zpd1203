/* SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Copyright (c) 2022, MediaTek Inc.
 */

#ifndef __MTK_PCI_H__
#define __MTK_PCI_H__

#include <linux/pci.h>
#include "mtk_debug.h"
#include "mtk_debugfs.h"
#include "mtk_dev.h"
#include "mtk_reg.h"

enum mtk_atr_type {
	ATR_PCI2AXI = 0,
	ATR_AXI2PCI
};

enum mtk_atr_src_port {
	ATR_SRC_PCI_WIN0 = 0,
	ATR_SRC_PCI_WIN1,
	ATR_SRC_AXIS_0,
	ATR_SRC_AXIS_1,
	ATR_SRC_AXIS_2,
	ATR_SRC_AXIS_3
};

enum mtk_atr_dst_port {
	ATR_DST_PCI_TRX = 0,
	ATR_DST_AXIM_0 = 4,
	ATR_DST_AXIM_1,
	ATR_DST_AXIM_2,
	ATR_DST_AXIM_3
};

#define ATR_PORT_OFFSET                 0x100
#define ATR_TABLE_OFFSET                0x20
#define ATR_TABLE_NUM_PER_ATR           8
#define ATR_WIN0_SRC_ADDR_LSB_DEFT      0x0000007f
#define ATR_PCIE_REG_TRSL_ADDR          0x10000000
#define ATR_PCIE_REG_SIZE               0x00400000
#define ATR_PCIE_REG_PORT               ATR_SRC_PCI_WIN0
#define ATR_PCIE_REG_TABLE_NUM          1
#define ART_PCIE_REG_MHCCIF_TABLE_NUM	0
#define ATR_PCIE_REG_TRSL_PORT          ATR_DST_AXIM_0
#define ATR_PCIE_DEV_DMA_PORT_START     ATR_SRC_AXIS_0
#define ATR_PCIE_DEV_DMA_PORT_END       ATR_SRC_AXIS_2
#define ATR_PCIE_DEV_DMA_SRC_ADDR       0x00000000
#define ATR_PCIE_DEV_DMA_TRANSPARENT    1
#define ATR_PCIE_DEV_DMA_SIZE           0x1000000000 // original value is 0
#define ATR_PCIE_DEV_DMA_TABLE_NUM      0
#define ATR_PCIE_DEV_DMA_TRSL_ADDR      0x00000000

#define MTK_BAR_0_1_IDX        0
#define MTK_BAR_2_3_IDX        2
#define MTK_BAR_4_5_IDX        4

/* Only use BAR0/1 and 2/3, so we should input 0b0101 for the two bar,
 * Input 0xf would cause error.
 */
#define MTK_REQUESTED_BARS \
	((1 << MTK_BAR_0_1_IDX) | \
	 (1 << MTK_BAR_2_3_IDX) | \
	 (1 << MTK_BAR_4_5_IDX))

#define MTK_IRQ_CNT_MIN        1
#define MTK_IRQ_CNT_MAX        32
#define MTK_IRQ_NAME_LEN	20

#define MTK_INVAL_IRQ_SRC      -1

#define MTK_IRQSRC_EXCEPT_ATR_MASK 0xFF77FFFF

#define MTK_ENABLE_INTR_BIT	BIT(0)

#define MTK_FORCE_MAC_ACTIVE_BIT	BIT(6)
#define MTK_DS_LOCK_REG_BIT		BIT(7)

#define MTK_PCI_MSI_MESSAGE_CONTROL  GENMASK(6, 4)

/* mhccif registers */
#define MHCCIF_RC2EP_SW_BSY                0x4
#define MHCCIF_RC2EP_SW_INT_START          0x8
#define MHCCIF_RC2EP_SW_TCHNUM             0xC
#define MHCCIF_RC2EP_TRAFFIC_CFG           0x0134
#define MHCCIF_RC2EP_SPARE_REG_4           0x0150
#define MHCCIF_RC2EP_SPARE_REG_6           0x0158
#define MHCCIF_RC2EP_SPARE_REG_7           0x015C
#define MHCCIF_EP2RC_SPARE_REG_1           0x0104
#define MHCCIF_EP2RC_SPARE_REG_5           0x0114
#define MHCCIF_EP2RC_SW_INT_STS            0x10
#define MHCCIF_EP2RC_SW_INT_ACK            0x14
#define MHCCIF_EP2RC_SW_INT_EAP_MASK       0x20
#define MHCCIF_EP2RC_SW_INT_EAP_MASK_SET   0x30
#define MHCCIF_EP2RC_SW_INT_EAP_MASK_CLR   0x40

/* mac active ctrl */
#define L1SS_MAC_ACTIVE			BIT(0)
#define DS_LOCK_MAC_ACTIVE		BIT(1)

#define MTK_PCI_CLASS         0x0D4000
#define MTK_PCI_VENDOR_ID     0x14C3

#define MTK_DISABLE_DS_BIT(grp)	BIT(grp)
#define MTK_ENABLE_DS_BIT(grp)	(BIT(grp) << 8)

#define MTK_CFG_INFO_BIT_SHIFT	4

#define MTK_PCI_DEV_CFG(id, cfg) \
{ \
	PCI_DEVICE(MTK_PCI_VENDOR_ID, id), \
	MTK_PCI_CLASS, PCI_ANY_ID, \
	.driver_data = (kernel_ulong_t)&(cfg), \
}

#define MTK_CFG_IRQ_DFLT_MASK		BIT(0)
#define MTK_CFG_DFLT_DISABLE_L1SS	BIT(1)
#define MTK_CFG_DRM_DISABLE_AP		BIT(2)
#define MTK_CFG_MHCCIF_TRSL			BIT(3)
#define MTK_CFG_RGU_L2_AUTO_ACK		BIT(4)
#define MTK_CFG_FORCE_MAC_ACTIVE	BIT(5)
#define MTK_CFG_HOST_GET_DEV_LOG	BIT(6)

struct mtk_pci_dev_cfg {
	u32 flag;
	u32 mhccif_rc_base_addr;
	u32 mhccif_rc_reg_trsl_addr;
	u32 mhccif_trsl_size;
	u32 mhccif_rc2ep_pcie_pm_counter;
	u32 istatus_host_ctrl_addr;
	int irq_tbl[MTK_IRQ_SRC_MAX];
	int (*atr_init)(struct mtk_md_dev *mdev);
	int (*get_dev_log)(struct mtk_md_dev *mdev,
			   void *buf, size_t count, enum mtk_dev_log_type type);
	int (*get_log_region_size)(struct mtk_md_dev *mdev, enum mtk_dev_log_type type);
	int (*dev_log_buff_init)(struct mtk_md_dev *mdev, bool enable);
	int (*dev_log_buff_exit)(struct mtk_md_dev *mdev);
};

struct mtk_pci_irq_desc {
	struct mtk_md_dev *mdev;
	u32 msix_bits;
	char name[MTK_IRQ_NAME_LEN];
};

struct mtk_pci_priv {
	const struct mtk_pci_dev_cfg *cfg;
	void *mdev;
	void __iomem *bar23_addr;
	void __iomem *bar45_addr;
	void __iomem *mac_reg_base;
	void __iomem *ext_reg_base;
	int rc_hp_on; /* Bridge hotplug status */
	int rgu_irq_id;
	int irq_cnt;
	int irq_type;
	void *irq_cb_data[MTK_IRQ_CNT_MAX];

	int (*irq_cb_list[MTK_IRQ_CNT_MAX])(int irq_id, void *data);
	struct mtk_pci_irq_desc irq_desc[MTK_IRQ_CNT_MAX];
	struct list_head mhccif_cb_list;
	/* mhccif_lock: lock to protect mhccif_cb_list */
	spinlock_t mhccif_lock;
	/* mac_active_lock: lock to protect mac active */
	spinlock_t mac_active_lock;
	u8 mac_active_ctrl;
	u8 l1ss_user;
	struct work_struct mhccif_work;
	int mhccif_irq_id;
	struct delayed_work rgu_work;
	struct pci_saved_state *saved_state;
	u16 ltr_max_snoop_lat;
	u16 ltr_max_nosnoop_lat;
	u32 l1ss_ctl1;
	u32 l1ss_ctl2;
	void *log_region_cfg;

	struct dentry *dentry;
};

struct mtk_mhccif_cb {
	struct list_head entry;
	int (*evt_cb)(u32 status, void *data);
	void *data;
	u32 chs;
};

struct mtk_atr_cfg {
	u64 src_addr;
	u64 trsl_addr;
	u64 size;
	u32 type;      /* Port type */
	u32 port;      /* Port number */
	u32 table;     /* Table number (8 tables for each port) */
	u32 trsl_id;
	u32 trsl_param;
	u32 transparent;
};

u32 mtk_pci_read32(struct mtk_md_dev *mdev, u64 addr);
void mtk_pci_write32(struct mtk_md_dev *mdev, u64 addr, u32 val);
u32 mtk_pci_mac_read32(struct mtk_pci_priv *priv, u64 addr);
void mtk_pci_mac_write32(struct mtk_pci_priv *priv, u64 addr, u32 val);
int mtk_pci_setup_atr(struct mtk_md_dev *mdev, struct mtk_atr_cfg *cfg);
void mtk_pci_atr_disable(struct mtk_pci_priv *priv);
void mtk_pci_flush_work(struct mtk_md_dev *mdev);

#endif /* __MTK_PCI_H__ */
