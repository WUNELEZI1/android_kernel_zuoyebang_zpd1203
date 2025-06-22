// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/pci_regs.h>
#include "pcie-xring-interface.h"

#define PCI_VENDER_ID_XRING		0x16c3
#define PCI_DEVICE_ID_XRING		0xaaaa

#define MSI_VEC_MIN			1
#define MSI_VEC_MAX			32
#define MSIX_VEC_MAX			2048

#define BAR_SIZE			SZ_4K
#define OFFSET_04			(4)
#define OFFSET_08                       (8)
#define OFFSET_12                       (12)
#define OFFSET_16                       (16)

struct xring_pcie_device {
	struct pci_dev	*pdev;
	int		num_irq;
	void __iomem	*bar[PCI_STD_NUM_BARS];
};

enum pci_bar_num {
	BAR_0,
	BAR_1,
	BAR_2,
	BAR_3,
	BAR_4,
	BAR_5,
};

static irqreturn_t xring_pcie_irq_handler(int irq, void *arg)
{
	struct pci_dev *pdev = arg;
	struct device *dev = &pdev->dev;

	dev_err(dev, "Irq is triggered, irq_num: %d\n", irq);

	return IRQ_HANDLED;
}

static void msi_alloc_request(struct pci_dev *pdev, int mode, int irq_num)
{
	struct xring_pcie_device *xring_pcie_device = pci_get_drvdata(pdev);
	struct device *dev = &pdev->dev;
	int irq = -1;
	int i, j, ret;

	irq = pci_alloc_irq_vectors(pdev, MSI_VEC_MIN, irq_num, mode);
	if (irq < 0) {
		dev_err(dev, "Failed to get MSI interrupts\n");
		return;
	}

	dev_info(dev, "the num of msi(x) irqs is %d\n", irq);
	xring_pcie_device->num_irq = irq;

	for (i = 0; i < irq; i++) {
		ret = devm_request_irq(dev, pci_irq_vector(pdev, i),
				       xring_pcie_irq_handler,
				       IRQF_SHARED, "xring-pcie-device", pdev);
		if (ret) {
			dev_err(dev, "Failed to request IRQ %d for MSI %d\n",
				pci_irq_vector(pdev, i), i + 1);

			for (j = 0; j < i; j++)
				devm_free_irq(dev, pci_irq_vector(pdev, j), pdev);

			pci_free_irq_vectors(pdev);
			return;
		}
	}
}

static ssize_t msi_alloc_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	dev_info(dev, "Usage: echo [arguments] > msi_alloc\n");
	dev_info(dev, "arguments:\n");
	dev_info(dev, "	0:free irqs\n");
	dev_info(dev, "	1:alloc msi\n");
	dev_info(dev, "	2:alloc msix\n");

	return 0;
}

static ssize_t msi_alloc_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct xring_pcie_device *xring_pcie_device = pci_get_drvdata(pdev);
	unsigned long val;
	int mode, irq_num;
	int i;

	if (kstrtoul(buf, 0, &val) < 0)
		return -EINVAL;

	if (val == 1) {
		mode = PCI_IRQ_MSI;
		irq_num = MSI_VEC_MAX;
		dev_info(dev, "alloc msi...\n");
		msi_alloc_request(pdev, mode, irq_num);

	} else if (val == 2) {
		mode = PCI_IRQ_MSIX;
		irq_num = MSIX_VEC_MAX;
		dev_info(dev, "alloc msix...\n");
		msi_alloc_request(pdev, mode, irq_num);

	} else if (val == 0) {
		dev_info(dev, "free irq vectors...\n");

		for (i = 0; i < xring_pcie_device->num_irq; i++)
			devm_free_irq(dev, pci_irq_vector(pdev, i), pdev);

		xring_pcie_device->num_irq = 0;
		pci_free_irq_vectors(pdev);

	} else {
		dev_err(dev, "invalid argument\n");
	}

	return count;
}

static DEVICE_ATTR_RW(msi_alloc);

static ssize_t set_speed_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct pci_dev *pci_dev = to_pci_dev(dev);
	int target_speed;

	if (kstrtoint(buf, 0, &target_speed) < 0)
		return -EINVAL;

	xring_pcie_set_speed(pci_dev, target_speed);

	return count;
}

static DEVICE_ATTR_WO(set_speed);

static void show_4k_bar(struct pci_dev *pdev, int bar)
{
	struct device *dev = &pdev->dev;
	struct xring_pcie_device *xring_pcie_device = pci_get_drvdata(pdev);
	int offset = 0;

	dev_info(dev, "\nbar%d:\n", bar);
	while (offset < BAR_SIZE) {
		dev_info(dev, "0x%04x: 0x%08x 0x%08x 0x%08x 0x%08x\n", offset,
			ioread32(xring_pcie_device->bar[bar] + offset),
			ioread32(xring_pcie_device->bar[bar] + offset + OFFSET_04),
			ioread32(xring_pcie_device->bar[bar] + offset + OFFSET_08),
			ioread32(xring_pcie_device->bar[bar] + offset + OFFSET_12));

		offset = offset + OFFSET_16;
	}
}

static ssize_t read_bar_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct xring_pcie_device *xring_pcie_device = pci_get_drvdata(pdev);
	int bar = 0;

	for (bar = 0; bar < BAR_4; bar++) {
		if (xring_pcie_device->bar[bar])
			show_4k_bar(pdev, bar);
	}

	return 0;
}

static DEVICE_ATTR_RO(read_bar);

static int xring_pcie_device_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct xring_pcie_device *xring_pcie_device;
	struct device *dev = &pdev->dev;
	enum pci_bar_num bar;
	void __iomem *base;
	int ret;

	xring_pcie_device = devm_kzalloc(dev, sizeof(*xring_pcie_device), GFP_KERNEL);
	if (!xring_pcie_device)
		return -ENOMEM;

	xring_pcie_device->pdev = pdev;

	ret = pci_enable_device(pdev);
	if (ret) {
		dev_err(dev, "Cannot enable PCI device\n");
		return ret;
	}

	pci_set_master(pdev);

	for (bar = 0; bar < PCI_STD_NUM_BARS; bar++) {
		if (pci_resource_flags(pdev, bar) & (IORESOURCE_MEM | IORESOURCE_IO)) {
			base = pci_iomap(pdev, bar, 0);
			if (!base)
				dev_err(dev, "Failed to read BAR%d\n", bar);

			xring_pcie_device->bar[bar] = base;
		}
	}

	pci_set_drvdata(pdev, xring_pcie_device);

	if (device_create_file(dev, &dev_attr_msi_alloc) < 0)
		dev_err(dev, "Failed to create file msi_alloc\n");

	if (device_create_file(dev, &dev_attr_read_bar) < 0)
		dev_err(dev, "Failed to create file read_bar\n");

	if (device_create_file(dev, &dev_attr_set_speed) < 0)
		dev_err(dev, "Failed to create file set_speed\n");

	return 0;
}

static void xring_pcie_device_remove(struct pci_dev *pdev)
{
	struct xring_pcie_device *xring_pcie_device = pci_get_drvdata(pdev);
	struct device *dev = &pdev->dev;
	int bar, i;

	for (bar = 0; bar < PCI_STD_NUM_BARS; bar++) {
		if (xring_pcie_device->bar[bar])
			pci_iounmap(pdev, xring_pcie_device->bar[bar]);
	}

	device_remove_file(dev, &dev_attr_msi_alloc);
	device_remove_file(dev, &dev_attr_read_bar);
	device_remove_file(dev, &dev_attr_set_speed);

	for (i = 0; i < xring_pcie_device->num_irq; i++)
		devm_free_irq(dev, pci_irq_vector(pdev, i), pdev);

	xring_pcie_device->num_irq = 0;
	pci_free_irq_vectors(pdev);
	pci_disable_device(pdev);
}

static const struct pci_device_id xring_pcie_device_tbl[] = {
	{ PCI_DEVICE(PCI_VENDER_ID_XRING, PCI_DEVICE_ID_XRING) },
	{ 0, }	/* end of table */
};
MODULE_DEVICE_TABLE(pci, xring_pcie_device_tbl);

static struct pci_driver xring_pcie_device_driver = {
	.name		= "xring-pcie-device",
	.id_table	= xring_pcie_device_tbl,
	.probe		= xring_pcie_device_probe,
	.remove		= xring_pcie_device_remove,
};
module_pci_driver(xring_pcie_device_driver);
MODULE_LICENSE("GPL v2");
