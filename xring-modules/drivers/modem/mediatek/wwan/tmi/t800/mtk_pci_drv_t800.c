// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2022, MediaTek Inc.
 */
#include "mtk_pci.h"
#include "mtk_pci_drv_t800.h"

#define TAG "PCI"

static int mtk_pci_atr_init_t800(struct mtk_md_dev *mdev)
{
	struct pci_dev *pdev = to_pci_dev(mdev->dev);
	struct mtk_pci_priv *priv = mdev->hw_priv;
	struct mtk_atr_cfg cfg;
	int port, ret;

	mtk_pci_atr_disable(priv);

	/* Config ATR for RC to access device's register */
	cfg.src_addr = pci_resource_start(pdev, MTK_BAR_2_3_IDX);
	cfg.size = ATR_PCIE_REG_SIZE;
	cfg.trsl_addr = ATR_PCIE_REG_TRSL_ADDR;
	cfg.type = ATR_PCI2AXI;
	cfg.port = ATR_PCIE_REG_PORT;
	cfg.table = ATR_PCIE_REG_TABLE_NUM;
	cfg.trsl_id = ATR_PCIE_REG_TRSL_PORT;
	cfg.trsl_param = 0x0;
	cfg.transparent = 0x0;
	ret = mtk_pci_setup_atr(mdev, &cfg);
	if (ret)
		return ret;

	if (priv->cfg->flag & MTK_CFG_MHCCIF_TRSL) {
		/* Config ATR for MHCCIF */
		cfg.src_addr = pci_resource_start(pdev, MTK_BAR_2_3_IDX);
		cfg.src_addr += priv->cfg->mhccif_rc_base_addr - ATR_PCIE_REG_TRSL_ADDR;
		cfg.size = priv->cfg->mhccif_trsl_size;
		cfg.trsl_addr = priv->cfg->mhccif_rc_reg_trsl_addr;
		cfg.type = ATR_PCI2AXI;
		cfg.port = ATR_PCIE_REG_PORT;
		cfg.table = ART_PCIE_REG_MHCCIF_TABLE_NUM;
		cfg.trsl_id = ATR_PCIE_REG_TRSL_PORT;
		cfg.trsl_param = 0x0;
		cfg.transparent = 0x0;
		ret = mtk_pci_setup_atr(mdev, &cfg);
		if (ret)
			return ret;
	}

	/* Config ATR for EP to access RC's memory */
	for (port = ATR_PCIE_DEV_DMA_PORT_START; port <= ATR_PCIE_DEV_DMA_PORT_END; port++) {
		cfg.src_addr = ATR_PCIE_DEV_DMA_SRC_ADDR;
		cfg.size = ATR_PCIE_DEV_DMA_SIZE;    // 0 -> 0x10_00000000
		cfg.trsl_addr = ATR_PCIE_DEV_DMA_TRSL_ADDR;
		cfg.type = ATR_AXI2PCI;
		cfg.port = port;
		cfg.table = ATR_PCIE_DEV_DMA_TABLE_NUM;
		cfg.trsl_id = ATR_DST_PCI_TRX;
		cfg.trsl_param = 0x0;
		/* Enable transparent translation */
		cfg.transparent = 0x0;    // ATR_PCIE_DEV_DMA_TRANSPARENT -> 0
		ret = mtk_pci_setup_atr(mdev, &cfg);
		if (ret)
			return ret;
	}

	if (priv->cfg->flag & MTK_CFG_HOST_GET_DEV_LOG) {
		/* Config ATR for BROM SRAM origin log */
		MTK_INFO(mdev, "Config ATR for BROM SRAM log origin\n");
		cfg.src_addr = pci_resource_start(pdev, MTK_BAR_4_5_IDX);
		cfg.src_addr += ATR_PCIE_BROM_SRAM_ORI_OFFSET;
		cfg.size = ATR_PCIE_BROM_SRAM_ORI_SIZE;
		cfg.trsl_addr = ATR_PCIE_BROM_SRAM_ORI_TRASL_ADDR;
		cfg.type = ATR_PCI2AXI;
		cfg.port = ATR_SRC_PCI_WIN1;
		cfg.table = ATR_PCIE_BROM_SRAM_ORI_TABLE_NUM;
		cfg.trsl_id = ATR_PCIE_REG_TRSL_PORT;
		cfg.trsl_param = 0x0;
		cfg.transparent = 0x0;
		ret = mtk_pci_setup_atr(mdev, &cfg);
		if (ret)
			return ret;

		/* Config ATR for BROM SRAM log */
		MTK_INFO(mdev, "Config ATR for BROM SRAM log\n");
		cfg.src_addr = pci_resource_start(pdev, MTK_BAR_4_5_IDX);
		cfg.src_addr += ATR_PCIE_BROM_SRAM_OFFSET;
		cfg.size = ATR_PCIE_BROM_SRAM_SIZE;
		cfg.trsl_addr = ATR_PCIE_BROM_SRAM_TRASL_ADDR;
		cfg.type = ATR_PCI2AXI;
		cfg.port = ATR_SRC_PCI_WIN1;
		cfg.table = ATR_PCIE_BROM_SRAM_TABLE_NUM;
		cfg.trsl_id = ATR_PCIE_REG_TRSL_PORT;
		cfg.trsl_param = 0x0;
		cfg.transparent = 0x0;
		ret = mtk_pci_setup_atr(mdev, &cfg);
		if (ret)
			return ret;

		/* Config ATR for PL SRAM log */
		MTK_INFO(mdev, "Config ATR for PL SRAM log\n");
		cfg.src_addr = pci_resource_start(pdev, MTK_BAR_4_5_IDX);
		cfg.src_addr += ATR_PCIE_PL_SRAM_OFFSET;
		cfg.size = ATR_PCIE_PL_SRAM_SIZE;
		cfg.trsl_addr = ATR_PCIE_PL_SRAM_TRASL_ADDR;
		cfg.type = ATR_PCI2AXI;
		cfg.port = ATR_SRC_PCI_WIN1;
		cfg.table = ATR_PCIE_PL_SRAM_TABLE_NUM;
		cfg.trsl_id = ATR_PCIE_REG_TRSL_PORT;
		cfg.trsl_param = 0x0;
		cfg.transparent = 0x0;
		ret = mtk_pci_setup_atr(mdev, &cfg);
		if (ret)
			return ret;

		/* Config ATR for PL DRAM log */
		MTK_INFO(mdev, "Config ATR for PL DRAM log\n");
		cfg.src_addr = pci_resource_start(pdev, MTK_BAR_4_5_IDX);
		cfg.src_addr += ATR_PCIE_PL_DRAM_OFFSET;
		cfg.size = ATR_PCIE_PL_DRAM_SIZE;
		cfg.trsl_addr = ATR_PCIE_PL_DRAM_TRASL_ADDR;
		cfg.type = ATR_PCI2AXI;
		cfg.port = ATR_SRC_PCI_WIN1;
		cfg.table = ATR_PCIE_PL_DRAM_TABLE_NUM;
		cfg.trsl_id = ATR_PCIE_REG_TRSL_PORT;
		cfg.trsl_param = 0x0;
		cfg.transparent = 0x0;
		ret = mtk_pci_setup_atr(mdev, &cfg);
		if (ret)
			return ret;

		/* Config ATR for ATF DRAM log */
		MTK_INFO(mdev, "Config ATR for ATF DRAM log\n");
		cfg.src_addr = pci_resource_start(pdev, MTK_BAR_4_5_IDX);
		cfg.src_addr += ATR_PCIE_ATF_DRAM_OFFSET;
		cfg.size = ATR_PCIE_ATF_DRAM_SIZE;
		cfg.trsl_addr = ATR_PCIE_ATF_DRAM_TRASL_ADDR;
		cfg.type = ATR_PCI2AXI;
		cfg.port = ATR_SRC_PCI_WIN1;
		cfg.table = ATR_PCIE_ATF_DRAM_TABLE_NUM;
		cfg.trsl_id = ATR_PCIE_REG_TRSL_PORT;
		cfg.trsl_param = 0x0;
		cfg.transparent = 0x0;
		ret = mtk_pci_setup_atr(mdev, &cfg);
		if (ret)
			return ret;
	}

	return 0;
}

static int mtk_pci_get_dev_log_t800(struct mtk_md_dev *mdev,
				    void *buf, size_t count, enum mtk_dev_log_type type)
{
	struct mtk_pci_priv *priv = mdev->hw_priv;
	const void __iomem *io_addr = NULL;
	int read_len = 0;
	int ret;

	MTK_INFO(mdev, "bar45_addr=0x%llx, brom_s:0x%x/0x%x, pl_s:0x%x/0x%x, atf_d:0x%x/0x%x\n",
		 priv->bar45_addr,
		 ATR_PCIE_BROM_SRAM_OFFSET, PCIE_BROM_SRAM_READ_OFFSET,
		 ATR_PCIE_PL_SRAM_OFFSET, PCIE_PL_SRAM_READ_OFFSET,
		 ATR_PCIE_ATF_DRAM_OFFSET, PCIE_ATF_DRAM_READ_OFFSET);

	switch (type) {
	case MTK_DEV_LOG_BROM_SRAM:
		io_addr = priv->bar45_addr + ATR_PCIE_BROM_SRAM_OFFSET +
			PCIE_BROM_SRAM_READ_OFFSET;
		read_len = PCIE_BROM_SRAM_LOG_SIZE;
		break;
	case MTK_DEV_LOG_BROM_SRAM_ORIGIN:
		io_addr = priv->bar45_addr + ATR_PCIE_BROM_SRAM_ORI_OFFSET +
			PCIE_BROM_SRAM_READ_ORI_OFFSET;
		read_len = PCIE_BROM_SRAM_LOG_SIZE;
		break;
	case MTK_DEV_LOG_PL_SRAM:
		io_addr = priv->bar45_addr + ATR_PCIE_PL_SRAM_OFFSET + PCIE_PL_SRAM_READ_OFFSET;
		read_len = PCIE_PL_SRAM_LOG_SIZE;
		break;
	case MTK_DEV_LOG_PL_DRAM:
		io_addr = priv->bar45_addr + ATR_PCIE_PL_DRAM_OFFSET + PCIE_PL_DRAM_READ_OFFSET;
		read_len = PCIE_PL_DRAM_LOG_SIZE;
		break;
	case MTK_DEV_LOG_ATF_DRAM:
		io_addr = priv->bar45_addr + ATR_PCIE_ATF_DRAM_OFFSET + PCIE_ATF_DRAM_READ_OFFSET;
		read_len = PCIE_ATF_DRAM_LOG_SIZE;
		break;
	default:
		ret = -EINVAL;
		goto exit;
	}

	if (!buf || count != read_len) {
		ret = -EINVAL;
		goto exit;
	}
	MTK_INFO(mdev, "Get device log, type=%d, io_addr=0x%llx, len=0x%x\n",
		 type, io_addr, read_len);

	memcpy_fromio(buf, io_addr, read_len);

	return read_len;

exit:
	MTK_ERR(mdev, "Invalid parameter, count=%d, type=%d\n", count, type);
	return ret;
}

static int mtk_pci_get_log_region_size_t800(struct mtk_md_dev *mdev, enum mtk_dev_log_type type)
{
	int size;

	switch (type) {
	case MTK_DEV_LOG_BROM_SRAM:
		size = PCIE_BROM_SRAM_LOG_SIZE;
		break;
	case MTK_DEV_LOG_BROM_SRAM_ORIGIN:
		size = PCIE_BROM_SRAM_LOG_SIZE;
		break;
	case MTK_DEV_LOG_PL_SRAM:
		size = PCIE_PL_SRAM_LOG_SIZE;
		break;
	case MTK_DEV_LOG_PL_DRAM:
		size = PCIE_PL_DRAM_LOG_SIZE;
		break;
	case MTK_DEV_LOG_ATF_DRAM:
		size = PCIE_ATF_DRAM_LOG_SIZE;
		break;
	default:
		MTK_ERR(mdev, "Invalid parameter, type=%d\n", type);
		return -EINVAL;
	}

	return size;
}

const struct mtk_pci_dev_cfg mtk_dev_cfg_0800 = {
	.flag = MTK_CFG_MHCCIF_TRSL | MTK_CFG_RGU_L2_AUTO_ACK | MTK_CFG_FORCE_MAC_ACTIVE |
		MTK_CFG_HOST_GET_DEV_LOG,
	.mhccif_rc_base_addr = 0x10012000,
	.mhccif_trsl_size = 0x2000,
	.mhccif_rc_reg_trsl_addr = 0x12020000,
	.mhccif_rc2ep_pcie_pm_counter = 0x14c,
	.istatus_host_ctrl_addr = REG_ISTATUS_HOST_CTRL_NEW,
	.irq_tbl = {
		[MTK_IRQ_SRC_AUDIO]   = 1,
		[MTK_IRQ_SRC_DPMAIF]  = 24,
		[MTK_IRQ_SRC_CLDMA0]  = 25,
		[MTK_IRQ_SRC_CLDMA1]  = 26,
		[MTK_IRQ_SRC_CLDMA2]  = 27,
		[MTK_IRQ_SRC_MHCCIF]  = 28,
		[MTK_IRQ_SRC_DPMAIF2] = 29,
		[MTK_IRQ_SRC_SAP_RGU] = 30,
		[MTK_IRQ_SRC_CLDMA3]  = 31,
		[MTK_IRQ_SRC_PM_LOCK] = 0,
		[MTK_IRQ_SRC_DPMAIF3] = 7,
		[MTK_IRQ_SRC_DPMAIF4] = MTK_INVAL_IRQ_SRC,
	},
	.atr_init = mtk_pci_atr_init_t800,
	.get_dev_log = mtk_pci_get_dev_log_t800,
	.get_log_region_size = mtk_pci_get_log_region_size_t800,
};
