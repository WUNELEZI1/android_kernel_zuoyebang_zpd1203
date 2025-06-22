// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2022, MediaTek Inc.
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/irq.h>

#if IS_ENABLED(CONFIG_MTK_AUDIODSP_SUPPORT)
#include <adsp_helper.h>
#include <adsp_ipi_queue.h>
#include <audio_ipi_platform_common.h>
#include <pcie-mediatek-gen3.h>
#endif

#include "mtk_common.h"
#include "mtk_dev.h"
#include "mtk_debug.h"
#include "mtk_fsm.h"
#include "mtk_pci.h"
#ifdef CONFIG_TX00_UT_PCIMSG
#include "ut_mtk_pcimsg_adsp.h"
#endif

#define TAG							("IPI")
//#define PCIE_FEATURE_ID			(31)
#define ADSP_IPI_PCIE				(32)
#define MAX_IPI_MSG_PAYLOAD_SIZE	(104)
#define ADSP_PHONE_CALL_STATE_REG	(0X112E0044)
#define ADSP_MEM_ID			(9)

struct ipi_pci_msg_t {
	u32 op_id;
	u32 payload_size;
	u8 payload[MAX_IPI_MSG_PAYLOAD_SIZE];
};

struct adsp_ipi_t {
	int ipi_id;
	struct mtk_md_dev *mdev;
	void __iomem *adsp_state_reg;
};

#if IS_ENABLED(CONFIG_MTK_AUDIODSP_SUPPORT)
static struct adsp_ipi_t *adsp_ipi_saved;

static void mtk_pcimsg_recv_msg_from_user(int ipi_id, void *data, unsigned int len)
{
	struct ipi_pci_msg_t *ipi_msg = data;

	if (!adsp_ipi_saved)
		return;

	MTK_INFO(adsp_ipi_saved->mdev, "recevice IPI message: 0x%x\n", ipi_msg->op_id);

	switch (ipi_msg->op_id) {
	case MTK_PCIMSG_C2H_REQ_BAR:
		mtk_pcimsg_send_msg_to_user(adsp_ipi_saved->mdev, MTK_PCIMSG_H2C_BAR);
		break;
	case MTK_PCIMSG_C2H_EXCEPT:
		mtk_pcimsg_send_msg_to_user(adsp_ipi_saved->mdev, MTK_PCIMSG_H2C_EXCEPT_ACK);
		mtk_except_report_evt(adsp_ipi_saved->mdev, EXCEPT_LINK_ERR);
		break;
	case MTK_PCIMSG_C2H_REQ_SMEM:
		mtk_pcimsg_send_msg_to_user(adsp_ipi_saved->mdev, MTK_PCIMSG_H2C_SMEM);
		break;
	default:
		break;
	}
}
#endif

int mtk_pcimsg_send_msg_to_user(struct mtk_md_dev *mdev, int msg_id)
{
#if IS_ENABLED(CONFIG_MTK_AUDIODSP_SUPPORT)
	struct pci_dev *pdev = to_pci_dev(mdev->dev);
	struct mtk_md_fsm *fsm = mdev->fsm;
	struct ipi_pci_msg_t ipi_msg;

	switch (msg_id) {
	case MTK_PCIMSG_H2C_READY:
	case MTK_PCIMSG_H2C_EXCEPT:
	case MTK_PCIMSG_H2C_EXCEPT_ACK:
		ipi_msg.op_id = msg_id;
		ipi_msg.payload_size = 0;
		break;
	case MTK_PCIMSG_H2C_BAR:
		ipi_msg.op_id = MTK_PCIMSG_H2C_BAR;
		ipi_msg.payload_size = 16;
		if (fsm->state == FSM_STATE_OFF) {
			*((uint64_t *)ipi_msg.payload) = INVALID_BAR_ADDR;
			*(((uint64_t *)ipi_msg.payload) + 1) = INVALID_BAR_ADDR;
		} else {
			*((uint64_t *)ipi_msg.payload) = pdev->resource[0].start;
			*(((uint64_t *)ipi_msg.payload) + 1) = pdev->resource[2].start;
			*(((uint64_t *)ipi_msg.payload) + 1) -= ATR_PCIE_REG_TRSL_ADDR;
		}
		break;
	case MTK_PCIMSG_H2C_SMEM:
		ipi_msg.op_id = MTK_PCIMSG_H2C_SMEM;
		ipi_msg.payload_size = 16;
		*((uint64_t *)ipi_msg.payload) = (uint64_t)adsp_get_reserve_mem_phys(ADSP_MEM_ID);
		*(((uint64_t *)ipi_msg.payload) + 1) =
			(uint64_t)adsp_get_reserve_mem_size(ADSP_MEM_ID);
		break;
	default:
		break;
	}

	adsp_register_feature(PCIE_FEATURE_ID);
	dsp_send_msg_to_queue(AUDIO_OPENDSP_USE_HIFI3_A, ADSP_IPI_PCIE,
			      &ipi_msg, sizeof(ipi_msg), 10);
	dsp_send_msg_to_queue(AUDIO_OPENDSP_USE_HIFI3_B, ADSP_IPI_PCIE,
			      &ipi_msg, sizeof(ipi_msg), 10);
	adsp_deregister_feature(PCIE_FEATURE_ID);

	MTK_INFO(mdev, "send IPI message: 0x%x\n", msg_id);
#endif

	return 0;
}

int mtk_pcimsg_messenger_init(struct mtk_md_dev *mdev)
{
	int err = 0;
#if IS_ENABLED(CONFIG_MTK_AUDIODSP_SUPPORT)
	int irqn;
	struct irq_data *data;
	struct adsp_ipi_t *adsp_ipi;

	adsp_ipi = devm_kzalloc(mdev->dev, sizeof(*adsp_ipi), GFP_KERNEL);
	if (!adsp_ipi) {
		MTK_ERR(mdev, "Failed to allocate memory for adsp ipi\n");
		return -ENOMEM;
	}

	adsp_ipi->ipi_id = ADSP_IPI_PCIE;
	adsp_ipi->mdev = mdev;
	adsp_ipi->adsp_state_reg = ioremap(ADSP_PHONE_CALL_STATE_REG, 4);
	if (!adsp_ipi->adsp_state_reg) {
		MTK_ERR(mdev, "Fialed to ioremap\n");
		err = -EIO;
		goto exit_free_mem;
	}

	err = adsp_ipi_registration(ADSP_IPI_PCIE,
				    mtk_pcimsg_recv_msg_from_user, "PCIe");
	if (err) {
		MTK_ERR(mdev, "Failed to register ipi message, err:%d\n", err);
		goto exit_iounmap;
	}

	adsp_ipi_saved = adsp_ipi;
	mdev->messenger = adsp_ipi;

	irqn = pci_irq_vector(to_pci_dev(mdev->dev), mtk_hw_get_irq_id(mdev, MTK_IRQ_SRC_CLDMA2));
	if (irqn < 0) {
		MTK_ERR(mdev, "Failed to get CLDMA2 IRQ number,irqn:%d\n", irqn);
		goto exit_iounmap;
	}

	data = irq_get_irq_data(irqn);
	if (!data) {
		MTK_ERR(mdev, "CLDMA2 IRQ data is NULL\n");
		goto exit_iounmap;
	}

	irq_chip_mask_parent(data);
	mtk_msi_unmask_to_other_mcu(data, 1);
	MTK_INFO(mdev, "Dispatch CLDMA2 IRQ, irq id:%d, irq vector:%d\n",
		 mtk_hw_get_irq_id(mdev, MTK_IRQ_SRC_CLDMA2), irqn);

	irqn = pci_irq_vector(to_pci_dev(mdev->dev), mtk_hw_get_irq_id(mdev, MTK_IRQ_SRC_AUDIO));
	if (irqn < 0) {
		MTK_ERR(mdev, "Failed to get AUDIO IRQ number,irqn:%d\n", irqn);
		goto exit_iounmap;
	}

	data = irq_get_irq_data(irqn);
	if (!data) {
		MTK_ERR(mdev, "AUDIO IRQ data is NULL\n");
		goto exit_iounmap;
	}

	irq_chip_mask_parent(data);
	mtk_msi_unmask_to_other_mcu(data, 1);
	MTK_INFO(mdev, "Dispatch AUDIO IRQ, irq id:%d, irq vector:%d\n",
		 mtk_hw_get_irq_id(mdev, MTK_IRQ_SRC_AUDIO), irqn);

	mtk_pcimsg_send_msg_to_user(mdev, MTK_PCIMSG_H2C_READY);

	return 0;

exit_iounmap:
	adsp_ipi_saved = NULL;
	mdev->messenger = NULL;
	iounmap(adsp_ipi->adsp_state_reg);
exit_free_mem:
	devm_kfree(mdev->dev, adsp_ipi);
#endif
	return err;
}

int mtk_pcimsg_messenger_exit(struct mtk_md_dev *mdev)
{
#if IS_ENABLED(CONFIG_MTK_AUDIODSP_SUPPORT)
	struct adsp_ipi_t *adsp_ipi = mdev->messenger;

	if (!adsp_ipi)
		return 0;

	adsp_ipi_unregistration(ADSP_IPI_PCIE);
	iounmap(adsp_ipi->adsp_state_reg);
	devm_kfree(mdev->dev, adsp_ipi);
	mdev->messenger = NULL;
	adsp_ipi_saved = NULL;
#endif

	return 0;
}

bool mtk_pcimsg_pci_user_is_busy(struct mtk_md_dev *mdev)
{
	bool is_busy = false;
#if IS_ENABLED(CONFIG_MTK_AUDIODSP_SUPPORT)
	u32 val = 0;
	struct adsp_ipi_t *adsp_ipi = mdev->messenger;

	if (!adsp_ipi)
		return false;
	val = ioread32(adsp_ipi->adsp_state_reg);
	is_busy = !!(val & BIT(7));
#endif

	return is_busy;
}
