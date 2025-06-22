// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2012-2015, 2017, 2021, The Linux Foundation. All rights reserved.
 */
#include <linux/bitmap.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/irqdomain.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spmi.h>
#include <dt-bindings/xring/platform-specific/spmi_per.h>
#include <dt-bindings/xring/platform-specific/pmic/top_reg.h>
/*
 * SPMI register addr
 */
#define SPMI_CHANNEL_OFFSET               0x20
#define SPMI_APB_SPMI_CMD_BASE_ADDR       SPMI_PER_APB_BURST0_ENTRY
#define SPMI_APB_SPMI_RDATA0_BASE_ADDR    SPMI_PER_BURST0_DATA0
#define SPMI_APB_SPMI_WDATA0_BASE_ADDR    SPMI_APB_SPMI_RDATA0_BASE_ADDR

/*
 * SPMI cmd register
 */
#define SPMI_APB_SPMI_CMD_EN                   BIT(31)
#define SPMI_APB_SPMI_CMD_SLAVEID_OFFSET       24
#define SPMI_APB_SPMI_CMD_SLAVE_ADDR_OFFSET    8
#define SPMI_APB_SPMI_CMD_TYPE_OFFSET          0
/*
 * SPMI status register
 */
#define SPMI_APB_TRANS_DONE    BIT(29)
#define SPMI_APB_TRANS_FAIL    BIT(30)

#define SPMI_CONTROLLER_TIMEOUT_US         1000
#define SPMI_CONTROLLER_WAIT_DONE_US       5
#define SPMI_CONTROLLER_MAX_TRANS_BYTES    16
#define SPMI_PER_DATAREG_BYTE              4

#define PMIC_LEVEL1_INT_COUNT            2
#define SPMI_SLAVEID                     0xf
#define PMIC_INTERRUPT_BASE_ADDR         TOP_REG_PMIC_LVL2INT_O1_IRQ_INT_RAW
#define PMIC_LEVEL1_INT_STATUS_ADDR(N)   (TOP_REG_PMIC_LVL1INT_1_SIRQ_INT_MASK + N * 2 + 1)
#define PMIC_LEVEL1_INT_MASK_ADDR(N)     (TOP_REG_PMIC_LVL1INT_1_SIRQ_INT_MASK + N * 2)
#define PMIC_LEVEL2_INT_STATUS_ADDR(N)   (PMIC_INTERRUPT_BASE_ADDR + N * 4 + 3)
#define PMIC_LEVEL2_INT_MASK_ADDR(N)     (PMIC_INTERRUPT_BASE_ADDR + N * 4 + 2)
#define PMIC_LEVEL2_INT_RAW_ADDR(N)      (PMIC_INTERRUPT_BASE_ADDR + N * 4)
#define POWER_KEY_RISING_EDGE 0x01
#define POWER_KEY_FALLING_EDGE 0x02
#define POWER_KEY_RISING_FALLING_EDGE 0x3

#define spec_to_hwirq(slave_id, first_irq_id, second_irq_id) \
	((((slave_id)     & 0xF)  << 8) | \
	(((first_irq_id)  & 0xF)  << 4) | \
	(((second_irq_id) & 0x7)  << 0))

#define hwirq_to_sid(hwirq)         (((hwirq) >> 8) & 0xF)
#define hwirq_to_first_irq(hwirq)   (((hwirq) >> 4) & 0xF)
#define hwirq_to_second_irq(hwirq)  (((hwirq) >> 0) & 0x7)



struct spmi_controller_dev {
	struct spmi_controller	*controller;
	struct device		*dev;
	void __iomem		*base;
	spinlock_t		lock;
	spinlock_t		irq_lock;
	u32			channel;
	int			irq;
	struct irq_domain	*domain;
};

static int spmi_controller_wait_for_done(struct device *dev,
					 struct spmi_controller_dev *ctrl_dev,
					 void __iomem *base, u8 sid, u16 addr)
{
	u32 timeout = SPMI_CONTROLLER_TIMEOUT_US / SPMI_CONTROLLER_WAIT_DONE_US;
	u32 status, offset;

	offset = SPMI_CHANNEL_OFFSET * ctrl_dev->channel + SPMI_APB_SPMI_CMD_BASE_ADDR;
	do {
		status = readl(base + offset);

		if (!(status & SPMI_APB_TRANS_FAIL) && !(status & SPMI_APB_TRANS_DONE)) {
			udelay(SPMI_CONTROLLER_WAIT_DONE_US);
		} else if (!(status & SPMI_APB_TRANS_FAIL) && (status & SPMI_APB_TRANS_DONE)) {
			status = readl(base + offset);
			if ((status & SPMI_APB_TRANS_FAIL) && !(status & SPMI_APB_TRANS_DONE))
				break;
			else if (!(status & SPMI_APB_TRANS_FAIL) && !(status & SPMI_APB_TRANS_DONE))
				return 0;
		} else if ((status & SPMI_APB_TRANS_FAIL) && !(status & SPMI_APB_TRANS_DONE)) {
			status = readl(base + offset);
			break;
		} else if ((status & SPMI_APB_TRANS_FAIL) && (status & SPMI_APB_TRANS_DONE))
			break;

	} while (--timeout);
	if (timeout) {
		dev_err(dev, "%s: transaction failed (0x%x)\n", __func__, status);
		return -EIO;
	}
	dev_err(dev, "%s: timeout, status 0x%x\n", __func__, status);
	return -ETIMEDOUT;
}

static int spmi_read_cmd(struct spmi_controller *ctrl,
			 u8 opc, u8 slave_id, u16 slave_addr, u8 *__buf, size_t bc)
{
	struct spmi_controller_dev *spmi_controller = dev_get_drvdata(&ctrl->dev);
	u32 chnl_ofst = SPMI_CHANNEL_OFFSET * spmi_controller->channel;
	unsigned long flags;
	u8 *buf = __buf;
	u32 cmd, data;
	int rc;
	u8 op_code, i;

	if (bc > SPMI_CONTROLLER_MAX_TRANS_BYTES) {
		dev_err(&ctrl->dev,
			"spmi_controller supports 1..%d bytes per trans, but:%zu requested\n",
			SPMI_CONTROLLER_MAX_TRANS_BYTES, bc);
		return  -EINVAL;
	}

	if (opc == SPMI_CMD_EXT_READ || opc == SPMI_CMD_EXT_READL)
		op_code = opc | (bc - 1);
	else if (opc == SPMI_CMD_READ)
		op_code = opc | (slave_addr & 0x1F);
	else {
		dev_err(&ctrl->dev, "invalid read cmd 0x%x\n", opc);
		return -EINVAL;
	}

	cmd = SPMI_APB_SPMI_CMD_EN |
		 ((slave_id & 0xf) << SPMI_APB_SPMI_CMD_SLAVEID_OFFSET)	|
		 ((slave_addr & 0xffff)  << SPMI_APB_SPMI_CMD_SLAVE_ADDR_OFFSET) |
		 (op_code << SPMI_APB_SPMI_CMD_TYPE_OFFSET);


	spin_lock_irqsave(&spmi_controller->lock, flags);

	writel(cmd, spmi_controller->base + chnl_ofst + SPMI_APB_SPMI_CMD_BASE_ADDR);

	rc = spmi_controller_wait_for_done(&ctrl->dev, spmi_controller,
					   spmi_controller->base, slave_id, slave_addr);
	if (rc)
		goto done;

	for (i = 0; bc > i * SPMI_PER_DATAREG_BYTE; i++) {
		data = readl(spmi_controller->base + chnl_ofst +
			     SPMI_APB_SPMI_RDATA0_BASE_ADDR +
			     i * SPMI_PER_DATAREG_BYTE);

		if ((bc - i * SPMI_PER_DATAREG_BYTE) >> 2) { /* multi byte cnt */
			memcpy(buf, &data, sizeof(data));
			buf += sizeof(data);
		} else {
			memcpy(buf, &data, bc % SPMI_PER_DATAREG_BYTE);
			buf += (bc % SPMI_PER_DATAREG_BYTE);
		}
	}

done:
	spin_unlock_irqrestore(&spmi_controller->lock, flags);
	if (rc)
		dev_err(&ctrl->dev,
			"spmi read wait timeout op:0x%x slave_id:%d slave_addr:0x%x bc:%zu\n",
			opc, slave_id, slave_addr, bc);
	else
		dev_dbg(&ctrl->dev, "%s: id:%d slave_addr:0x%x, read value: %*ph\n",
			__func__, slave_id, slave_addr, (int)bc, __buf);

	return rc;
}

static int spmi_write_cmd(struct spmi_controller *ctrl,
			  u8 opc, u8 slave_id, u16 slave_addr, const u8 *__buf, size_t bc)
{
	struct spmi_controller_dev *spmi_controller = dev_get_drvdata(&ctrl->dev);
	u32 chnl_ofst = SPMI_CHANNEL_OFFSET * spmi_controller->channel;
	const u8 *buf = __buf;
	unsigned long flags;
	u32 cmd, data;
	int rc;
	u8 op_code, i;

	if (bc > SPMI_CONTROLLER_MAX_TRANS_BYTES) {
		dev_err(&ctrl->dev,
			"spmi_controller supports 1..%d bytes per trans, but:%zu requested\n",
			SPMI_CONTROLLER_MAX_TRANS_BYTES, bc);
		return  -EINVAL;
	}

	if (opc == SPMI_CMD_EXT_WRITE || opc == SPMI_CMD_EXT_WRITEL)
		op_code = opc | (bc - 1);
	else if (opc == SPMI_CMD_WRITE)
		op_code = opc | (slave_addr & 0x1F);
	else if (opc == SPMI_CMD_ZERO_WRITE)
		op_code = opc | (buf[0] & 0x7F);
	else {
		dev_err(&ctrl->dev, "invalid write cmd 0x%x\n", opc);
		return -EINVAL;
	}

	cmd = SPMI_APB_SPMI_CMD_EN |
		 ((slave_id & 0xf) << SPMI_APB_SPMI_CMD_SLAVEID_OFFSET)	|
		 ((slave_addr & 0xffff)  << SPMI_APB_SPMI_CMD_SLAVE_ADDR_OFFSET) |
		 (op_code << SPMI_APB_SPMI_CMD_TYPE_OFFSET);

	/* Write data to FIFOs */
	spin_lock_irqsave(&spmi_controller->lock, flags);

	for (i = 0; bc > i * SPMI_PER_DATAREG_BYTE; i++) {
		data = 0;
		if ((bc - i * SPMI_PER_DATAREG_BYTE) >> 2) { /* multi byte cnt */
			memcpy(&data, buf, sizeof(data));
			buf += sizeof(data);
		} else {
			memcpy(&data, buf, bc % SPMI_PER_DATAREG_BYTE);
			buf += (bc % SPMI_PER_DATAREG_BYTE);
		}

		writel(data,
		       spmi_controller->base + chnl_ofst +
		       SPMI_APB_SPMI_WDATA0_BASE_ADDR +
		       SPMI_PER_DATAREG_BYTE * i);
	}

	/* Start the transaction */
	writel(cmd, spmi_controller->base + chnl_ofst + SPMI_APB_SPMI_CMD_BASE_ADDR);

	rc = spmi_controller_wait_for_done(&ctrl->dev, spmi_controller,
					   spmi_controller->base, slave_id,
					   slave_addr);
	spin_unlock_irqrestore(&spmi_controller->lock, flags);

	if (rc)
		dev_err(&ctrl->dev, "spmi write wait timeout op:0x%x slave_id:%d slave_addr:0x%x bc:%zu\n",
			opc, slave_id, slave_addr, bc);
	else
		dev_dbg(&ctrl->dev, "%s: id:%d slave_addr:0x%x, wrote value: %*ph\n",
			__func__, slave_id, slave_addr, (int)bc, __buf);

	return rc;
}

static void write_pmic_int_reg(struct irq_data *d, u16 reg, void *buf,
			       size_t len)
{
	struct spmi_controller_dev *spmi_controller = irq_data_get_irq_chip_data(d);
	u8 sid = hwirq_to_sid(d->hwirq);

	if (spmi_write_cmd(spmi_controller->controller,
					 SPMI_CMD_EXT_WRITEL, sid, reg, buf, len))
		dev_err(&spmi_controller->controller->dev, "failed irqchip transaction on %x\n",
				    d->irq);
}

static void read_pmic_int_reg(struct irq_data *d, u16 reg, void *buf, size_t len)
{
	struct spmi_controller_dev *spmi_controller = irq_data_get_irq_chip_data(d);
	u8 sid = hwirq_to_sid(d->hwirq);

	if (spmi_read_cmd(spmi_controller->controller,
					SPMI_CMD_EXT_READL, sid, reg, buf, len))
		dev_err(&spmi_controller->controller->dev, "failed irqchip transaction on %x\n",
				    d->irq);
}

static void cleanup_irq(struct spmi_controller_dev *spmi_controller,
				u8 first_irq_id, u8 second_irq_id)
{

	u16 raw_reg = PMIC_LEVEL2_INT_RAW_ADDR(first_irq_id);
	u8 data;

	data = BIT(second_irq_id);
	if (spmi_write_cmd(spmi_controller->controller,
			 SPMI_CMD_EXT_WRITEL, SPMI_SLAVEID, raw_reg, &data, 1))
		dev_err(&spmi_controller->controller->dev,
			"failed to clear irq, first_irq_id = %x,second_irq_id = %x\n",
			first_irq_id, second_irq_id);
}

static void periph_interrupt(struct spmi_controller_dev *spmi_controller,
				 u8 first_irq_id, u8 status)
{
	unsigned int irq;
	u8 second_irq_id;
	u8 temp;

	while (status) {
		if ((status & POWER_KEY_RISING_FALLING_EDGE) ==
				POWER_KEY_RISING_FALLING_EDGE)
			status &= ~POWER_KEY_RISING_EDGE;
		second_irq_id = ffs(status) - 1;
		status &= ~BIT(second_irq_id);
		cleanup_irq(spmi_controller, first_irq_id, second_irq_id);
		if (spmi_read_cmd(spmi_controller->controller, SPMI_CMD_EXT_READL,
				SPMI_SLAVEID, PMIC_INTERRUPT_BASE_ADDR, &temp, 1))
			dev_err(&spmi_controller->controller->dev,
				"failed to read level1 int status\n");
		irq = irq_find_mapping(spmi_controller->domain,
					spec_to_hwirq(SPMI_SLAVEID, first_irq_id, second_irq_id));
		if (!irq) {
			dev_err(&spmi_controller->controller->dev, "unmapped irq %d\n", irq);
			continue;
		}
		generic_handle_irq(irq);
	}
}

static void pmic_chained_irq(struct irq_desc *desc)
{
	struct spmi_controller_dev *spmi_controller = irq_desc_get_handler_data(desc);
	struct irq_chip *chip = irq_desc_get_chip(desc);
	u8 level1_int_status, level2_int_status, first_irq_id;
	u16 reg;
	int i;

	chained_irq_enter(chip, desc);

	for (i = 0; i < PMIC_LEVEL1_INT_COUNT; i++) {
		reg = PMIC_LEVEL1_INT_STATUS_ADDR(i);
		level1_int_status = 0;
		if (spmi_read_cmd(spmi_controller->controller, SPMI_CMD_EXT_READL,
						SPMI_SLAVEID, reg, &level1_int_status, 1))
			dev_err(&spmi_controller->controller->dev,
					"failed to read level1_int_status,reg = %x\n", reg);
		while (level1_int_status) {
			first_irq_id = ffs(level1_int_status) - 1;
			level1_int_status &= ~BIT(first_irq_id);
			first_irq_id += i * 8;
			reg = PMIC_LEVEL2_INT_STATUS_ADDR(first_irq_id);
			level2_int_status = 0;
			if (spmi_read_cmd(spmi_controller->controller, SPMI_CMD_EXT_READL,
						SPMI_SLAVEID, reg, &level2_int_status, 1))
				dev_err(&spmi_controller->controller->dev,
					"failed to read level2_int_status, reg = %x\n", reg);
			if (level2_int_status)
				periph_interrupt(spmi_controller, first_irq_id, level2_int_status);
		}
	}

	chained_irq_exit(chip, desc);
}

static void xr_irq_mask(struct irq_data *d)
{
	struct spmi_controller_dev *spmi_controller = irq_data_get_irq_chip_data(d);
	u8  first_irq_id = hwirq_to_first_irq(d->hwirq);
	u8  second_irq_id = hwirq_to_second_irq(d->hwirq);
	u16 reg;
	u8  data;
	unsigned long flags;

	reg = PMIC_LEVEL2_INT_MASK_ADDR(first_irq_id);
	spin_lock_irqsave(&spmi_controller->irq_lock, flags);
	read_pmic_int_reg(d, reg, &data, 1);
	data |= BIT(second_irq_id);
	write_pmic_int_reg(d, reg, &data, 1);
	spin_unlock_irqrestore(&spmi_controller->irq_lock, flags);
}

static void xr_irq_unmask(struct irq_data *d)
{
	struct spmi_controller_dev *spmi_controller = irq_data_get_irq_chip_data(d);
	u8  first_irq_id = hwirq_to_first_irq(d->hwirq);
	u8  second_irq_id = hwirq_to_second_irq(d->hwirq);
	u16 reg;
	u8  data;
	unsigned long flags;

	reg = PMIC_LEVEL2_INT_MASK_ADDR(first_irq_id);
	spin_lock_irqsave(&spmi_controller->irq_lock, flags);
	read_pmic_int_reg(d, reg, &data, 1);
	data &= ~BIT(second_irq_id);
	write_pmic_int_reg(d, reg, &data, 1);
	spin_unlock_irqrestore(&spmi_controller->irq_lock, flags);
}

static struct irq_chip pmic_irqchip = {
	.name		= "xr_pmic_spmi",
	.irq_mask	= xr_irq_mask,
	.irq_unmask	= xr_irq_unmask,
	.irq_disable = xr_irq_mask,
	.irq_enable  = xr_irq_unmask,
	.flags       = IRQCHIP_SKIP_SET_WAKE,
};

static int xr_irq_domain_translate(struct irq_domain *d,
					struct irq_fwspec *fwspec,
					unsigned long *out_hwirq,
					unsigned int *out_type)
{
	struct spmi_controller_dev *spmi_controller = d->host_data;
	u32 *intspec = fwspec->param;

	dev_dbg(&spmi_controller->controller->dev, "intspec[0] 0x%x intspec[1] 0x%x intspec[2] 0x%x\n",
		intspec[0], intspec[1], intspec[2]);

	if (irq_domain_get_of_node(d) != spmi_controller->controller->dev.of_node)
		return -EINVAL;
	if (fwspec->param_count != 4)
		return -EINVAL;
	if (intspec[0] > 0xF || intspec[1] > 0xF || intspec[2] > 0x7)
		return -EINVAL;

	*out_hwirq = spec_to_hwirq(intspec[0], intspec[1], intspec[2]);
	*out_type  = intspec[3] & IRQ_TYPE_SENSE_MASK;

	dev_dbg(&spmi_controller->controller->dev, "out_hwirq = %lu\n", *out_hwirq);

	return 0;
}

static void xr_irq_domain_map(struct spmi_controller_dev *spmi_controller,
				   struct irq_domain *domain, unsigned int virq,
				   irq_hw_number_t hwirq, unsigned int type)
{
	irq_flow_handler_t handler;

	dev_dbg(&spmi_controller->controller->dev, "virq = %u, hwirq = %lu, type = %u\n",
		virq, hwirq, type);

	handler = handle_level_irq;

	irq_domain_set_info(domain, virq, hwirq, &pmic_irqchip, spmi_controller,
			    handler, NULL, NULL);
}

static int xr_irq_domain_alloc(struct irq_domain *domain,
				    unsigned int virq, unsigned int nr_irqs,
				    void *data)
{
	struct spmi_controller_dev *spmi_controller = domain->host_data;
	struct irq_fwspec *fwspec = data;
	irq_hw_number_t hwirq;
	unsigned int type;
	int ret, i;

	ret = xr_irq_domain_translate(domain, fwspec, &hwirq, &type);
	if (ret)
		return ret;

	for (i = 0; i < nr_irqs; i++)
		xr_irq_domain_map(spmi_controller, domain, virq + i, hwirq + i,
				       type);

	return 0;
}

static const struct irq_domain_ops pmic_irq_domain_ops = {
	.alloc = xr_irq_domain_alloc,
	.free = irq_domain_free_irqs_common,
	.translate = xr_irq_domain_translate,
};

static void pmic_int_init(struct spmi_controller_dev *spmi_controller)
{
	u8 data;
	u16 reg;
	int i;

	for (i = 0; i < PMIC_LEVEL1_INT_COUNT; i++) {
		reg = PMIC_LEVEL1_INT_MASK_ADDR(i);
		data = 0;

		if (spmi_write_cmd(spmi_controller->controller, SPMI_CMD_EXT_WRITEL,
						SPMI_SLAVEID, reg, &data, 1))
			dev_err(&spmi_controller->controller->dev,
				"failed to enable level1_int_mask,reg = %x\n", reg);
	}
}

static int xr_spmi_probe(struct platform_device *pdev)
{
	struct spmi_controller_dev *spmi_controller;
	struct spmi_controller *ctrl;
	struct resource *res;
	int ret;

	ctrl = spmi_controller_alloc(&pdev->dev, sizeof(*spmi_controller));
	if (!ctrl) {
		dev_err(&pdev->dev, "can not allocate spmi_controller data\n");
		return -ENOMEM;
	}
	spmi_controller = spmi_controller_get_drvdata(ctrl);
	spmi_controller->controller = ctrl;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "can not get resource!\n");
		ret = -EINVAL;
		goto err_put_ctrl;
	}
	spmi_controller->base = devm_ioremap(&pdev->dev, res->start,
					     resource_size(res));
	if (!spmi_controller->base) {
		dev_err(&pdev->dev, "can not remap base addr!\n");
		ret = -EADDRNOTAVAIL;
		goto err_put_ctrl;
	}

	spmi_controller->irq = platform_get_irq(pdev, 0);
	if (spmi_controller->irq < 0) {
		ret = spmi_controller->irq;
		goto err_put_ctrl;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "spmi-channel", &spmi_controller->channel);
	if (ret) {
		dev_err(&pdev->dev, "can not get channel\n");
		ret = -ENODEV;
		goto err_put_ctrl;
	}

	platform_set_drvdata(pdev, ctrl);
	spin_lock_init(&spmi_controller->lock);
	spin_lock_init(&spmi_controller->irq_lock);
	ctrl->read_cmd = spmi_read_cmd;
	ctrl->write_cmd = spmi_write_cmd;

	spmi_controller->domain = irq_domain_add_tree(pdev->dev.of_node,
					 &pmic_irq_domain_ops, spmi_controller);
	if (!spmi_controller->domain) {
		dev_err(&pdev->dev, "unable to create irq_domain\n");
		ret = -ENOMEM;
		goto err_put_ctrl;
	}

	irq_set_chained_handler_and_data(spmi_controller->irq, pmic_chained_irq,
					spmi_controller);
	ret = spmi_controller_add(ctrl);
	if (ret)
		goto err_domain_remove;

	pmic_int_init(spmi_controller);

	return 0;

err_domain_remove:
	irq_set_chained_handler_and_data(spmi_controller->irq, NULL, NULL);
	irq_domain_remove(spmi_controller->domain);
err_put_ctrl:
	spmi_controller_put(ctrl);
	return ret;
}

static int xr_spmi_remove(struct platform_device *pdev)
{
	struct spmi_controller *ctrl = platform_get_drvdata(pdev);
	struct spmi_controller_dev *spmi_controller = spmi_controller_get_drvdata(ctrl);

	spmi_controller_remove(ctrl);
	irq_set_chained_handler_and_data(spmi_controller->irq, NULL, NULL);
	irq_domain_remove(spmi_controller->domain);
	spmi_controller_put(ctrl);
	return 0;
}

static const struct of_device_id xr_spmi_controller_match_table[] = {
	{ .compatible = "xring,spmi-controller", },
	{},
};
MODULE_DEVICE_TABLE(of, xr_spmi_controller_match_table);

static struct platform_driver xr_spmi_controller_driver = {
	.probe		= xr_spmi_probe,
	.remove		= xr_spmi_remove,
	.driver		= {
		.name	= "xr_spmi_controller",
		.of_match_table = xr_spmi_controller_match_table,
	},
};
module_platform_driver(xr_spmi_controller_driver);

MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:spmi_controller");
