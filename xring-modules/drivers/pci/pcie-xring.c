// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/of_device.h>
#include <linux/pci.h>
#include <linux/resource.h>
#include <linux/types.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/gpio/consumer.h>
#include <linux/of_gpio.h>
#include <linux/version.h>
#include <linux/of_reserved_mem.h>

#include <dt-bindings/xring/platform-specific/flowctrl_cfg_define.h>
#include <dt-bindings/xring/platform-specific/pcie_ctrl.h>
#include <dt-bindings/xring/platform-specific/pcie_phy.h>
#include <dt-bindings/xring/platform-specific/hss2_top.h>
#include "../../pci.h"
#include "pcie-xring.h"
#include "pcie-xring-interface.h"
#include "dwc_c20pcie4_phy_x2_ns_pcs_raw_ref_100m_ext_rom.h"
#include <linux/arm-smccc.h>
#include <linux/pm_wakeup.h>
#include <linux/pm_runtime.h>

static int num_lanes;
#if IS_ENABLED(CONFIG_XRING_DEBUG)
module_param(num_lanes, int, 0644);
MODULE_PARM_DESC(num_lanes,
		"number of lanes (default: 2)");
#endif

static DEFINE_MUTEX(tcu_mutex);
struct pcie_info xring_pcie_status;

void xring_pcie_status_update(struct xring_pcie *xring_pcie, bool is_on)
{
	if (xring_pcie->port_num == 0)
		xring_pcie_status.modem = is_on;
	else
		xring_pcie_status.wifi = is_on;
}

static void xring_pcie_diffbuf_shutdown(struct xring_pcie *xring_pcie, bool state)
{
	u32 reg;

	if (state) {
		reg = readl(xring_pcie->pcie_phy + PCIE_PHY_PCIE_SYS_TOP_CFG);
		reg |= 0x1 << PCIE_PHY_PCIE_SYS_TOP_CFG_SW_PULL_DOWN_SHIFT;
		writel(reg, xring_pcie->pcie_phy + PCIE_PHY_PCIE_SYS_TOP_CFG);
		dev_info(xring_pcie->pci->dev, "trigger diffbuf shutdown\n");
	} else {
		reg = readl(xring_pcie->pcie_phy + PCIE_PHY_PCIE_SYS_TOP_CFG);
		reg &= ~PCIE_PHY_PCIE_SYS_TOP_CFG_SW_PULL_DOWN_MASK;
		writel(reg, xring_pcie->pcie_phy + PCIE_PHY_PCIE_SYS_TOP_CFG);
		dev_info(xring_pcie->pci->dev, "trigger diffbuf powerup\n");
	}
}

static u32 xring_pcie_read_apb32_to_dbi(struct dw_pcie *pci, void __iomem *base, u32 reg,
					size_t size)
{
	void __iomem *addr = base + reg;
	void __iomem *aligned_addr = PTR_ALIGN_DOWN(addr, 0x4);
	u32 val = readl(aligned_addr);
	unsigned int offset = (unsigned long)addr & 0x3;

	if (!IS_ALIGNED((uintptr_t)addr, size)) {
		dev_err(pci->dev, "read Address %p and size %zd are not aligned\n", addr, size);
		return 0;
	}

	if (size == 4)
		return val;

	return (val >> (8 * offset)) & ((1 << (size * 8)) - 1);
}

static void xring_pcie_write_apb32_to_dbi(struct dw_pcie *pci, void __iomem *base, u32 reg,
					size_t size, u32 val)
{
	void __iomem *addr = base + reg;
	void __iomem *aligned_addr = PTR_ALIGN_DOWN(addr, 0x4);
	unsigned int offset = (unsigned long)addr & 0x3;
	u32 mask;
	u32 value;

	if (!IS_ALIGNED((uintptr_t)addr, size)) {
		dev_err(pci->dev, "write Address %p and size %zd are not aligned\n", addr, size);
		return;
	}

	if (size == 4) {
		writel(val, addr);
		return;
	}

	mask = ~(((1 << (size * 8)) - 1) << (offset * 8));
	value = readl(aligned_addr) & mask;
	value |= val << (offset * 8);
	writel(value, aligned_addr);
}

static int xring_pcie_rd_own_conf(struct pci_bus *bus, unsigned int devfn,
					int where, int size, u32 *val)
{
	struct dw_pcie *pci = to_dw_pcie_from_pp(bus->sysdata);

	if (PCI_SLOT(devfn))
		return PCIBIOS_DEVICE_NOT_FOUND;

	*val = dw_pcie_read_dbi(pci, where, size);
	return PCIBIOS_SUCCESSFUL;
}

static int xring_pcie_wr_own_conf(struct pci_bus *bus, unsigned int devfn,
					int where, int size, u32 val)
{
	struct dw_pcie *pci = to_dw_pcie_from_pp(bus->sysdata);

	if (PCI_SLOT(devfn))
		return PCIBIOS_DEVICE_NOT_FOUND;

	dw_pcie_write_dbi(pci, where, size, val);
	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops xring_pcie_ops = {
	.map_bus = dw_pcie_own_conf_map_bus,
	.read = xring_pcie_rd_own_conf,
	.write = xring_pcie_wr_own_conf,
};

static void xring_pcie_hot_reset(struct xring_pcie *xring_pcie, int status)
{
	struct device *dev = xring_pcie->pci->dev;
	u32 reg;
	int retry_time;

	dev_info(dev, "xring pcie hot reset status=%d\n", status);
	writel(status, xring_pcie->pci_ctrl + PCIE_CTRL_HOT_RESET);

	if (status == 1) {
		/* wait 200ms for ltssm to change State_machine to hot_reset */
		retry_time = 20;
		do {
			reg = readl(xring_pcie->pci_ctrl + PCIE_CTRL_SMLH_LTSSM_STATE);
			if (reg == 0x1f)
				return;

			usleep_range(9000, 10000);
		} while (retry_time--);
		dev_err(dev, "xring pcie hot reset wait ltssm in hot_reset timeout!\n");
	}
}

static void xring_pcie_rc_rst_assert(struct xring_pcie *xring_pcie)
{
	u32 reg;

	reg = readl(xring_pcie->pci_ctrl + PCIE_CTRL_PERST_N_OVERRIDE);
	reg &= ~PCIE_CTRL_PERST_N_OVERRIDE_PERST_N_SRC_OVER_MASK;
	writel(reg, xring_pcie->pci_ctrl + PCIE_CTRL_PERST_N_OVERRIDE);
}

static void xring_pcie_rc_rst_deassert(struct xring_pcie *xring_pcie)
{
	u32 reg;

	if (!xring_pcie->insr)
		msleep(xring_pcie->rst_intrvl);
	reg = readl(xring_pcie->pci_ctrl + PCIE_CTRL_PERST_N_OVERRIDE);
	reg |= PCIE_CTRL_PERST_N_OVERRIDE_PERST_N_SRC_OVER_MASK;
	writel(reg, xring_pcie->pci_ctrl + PCIE_CTRL_PERST_N_OVERRIDE);
}

static void xring_pcie_dev_rst_assert(struct xring_pcie *xring_pcie)
{
	gpio_set_value(xring_pcie->perstn_gpio, 0);
}

static void xring_pcie_dev_rst_deassert(struct xring_pcie *xring_pcie)
{
	if (!xring_pcie->insr)
		msleep(xring_pcie->rst_intrvl);
	gpio_set_value(xring_pcie->perstn_gpio, 1);
}

#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
static void xring_pcie_dis_l1ss_cap(struct pcie_port *pp)
#else
static void xring_pcie_dis_l1ss_cap(struct dw_pcie_rp *pp)
#endif
{
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct device *dev = pci->dev;
	int offset;
	u32 val;

	offset = dw_pcie_find_ext_capability(pci, PCI_EXT_CAP_ID_L1SS);

	val = dw_pcie_readl_dbi(pci, offset + PCI_L1SS_CTL1);
	val &= ~(PCI_L1SS_CTL1_ASPM_L1_1 | PCI_L1SS_CTL1_ASPM_L1_2);
	dw_pcie_writel_dbi(pci, offset + PCI_L1SS_CTL1, val);

	val = dw_pcie_readl_dbi(pci, offset + PCI_L1SS_CAP);
	val &= ~(ASPM_CAP_L1_SS_PCIPM | ASPM_CAP_L1_SS);
	dw_pcie_writel_dbi(pci, offset + PCI_L1SS_CAP, val);

	dev_info(dev, "aspm l1 substates disable\n");
}

#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
static void xring_pcie_dis_aspm_cap(struct pcie_port *pp)
#else
static void xring_pcie_dis_aspm_cap(struct dw_pcie_rp *pp)
#endif
{
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct device *dev = pci->dev;
	int offset;
	u32 val;

	xring_pcie_dis_l1ss_cap(pp);

	offset = dw_pcie_find_capability(pci, PCI_CAP_ID_EXP);
	val = dw_pcie_readl_dbi(pci, offset + PCI_EXP_LNKCAP);
	val &= ~PCI_EXP_LNKCAP_ASPMS;
	dw_pcie_writel_dbi(pci, offset + PCI_EXP_LNKCAP, val);

	dev_info(dev, "aspm disable\n");
}

static int xring_pcie_set_qos(struct xring_pcie *xring_pcie)
{
	struct sys_qos_map *sys_qos_table_dma = NULL;
	u32 val, mask;
	int i;

	sys_qos_table_dma = xring_flowctrl_get_cfg_table(SYS_QOS_TABLE_ID);
	if (!sys_qos_table_dma) {
		dev_err(xring_pcie->pci->dev, "get flowctrl cfg_table failed\n");
		return -EINVAL;
	}

	for (i = 0; i < SYS_QOS_REG_NUM; i++) {
		if (((sys_qos_table_dma[i].mst_id == QOS_HSS2_PCIE0_ID) &&
		     (xring_pcie->port_num == 0)) ||
		    ((sys_qos_table_dma[i].mst_id == QOS_HSS2_PCIE1_ID) &&
		     (xring_pcie->port_num == 1))) {
			mask = BIT(sys_qos_table_dma[i].end_bit + 1) -
			       BIT(sys_qos_table_dma[i].start_bit);
			val = readl(xring_pcie->pci_ctrl + sys_qos_table_dma[i].offset);
			val &= ~(mask << sys_qos_table_dma[i].start_bit);
			val |= sys_qos_table_dma[i].val << sys_qos_table_dma[i].start_bit;
			writel(val, xring_pcie->pci_ctrl + sys_qos_table_dma[i].offset);
		}
	}

	return 0;
}

/*
 * Trigger sram load done after SRAM loaded.
 * Untrigger sram load done after rst to load SRAM next time.
 */
static void xring_pcie_sram_load_done(struct xring_pcie *xring_pcie, bool state)
{
	u32 val = !!state;

	writel(val, xring_pcie->pcie_phy + PCIE_PHY_PHY_MISC_CFG2);
}

static int xring_pcie_phy_firmware_upd(struct xring_pcie *xring_pcie)
{
	u32 reg;
	int i;

	for (i =  0; i < 100; i++) {
		reg = readl(xring_pcie->pcie_phy + PCIE_PHY_PHY_MISC_RPT);
		reg &= PCIE_PHY_PHY_MISC_RPT_PHY0_SRAM_INIT_DONE_MASK;
		reg >>= PCIE_PHY_PHY_MISC_RPT_PHY0_SRAM_INIT_DONE_SHIFT;
		/* 100 times for waiting sram_init_done == 0x1 */
		if (reg == 0x1)
			break;

		usleep_range(100, 200);
	}

	if (i == 100) {
		dev_err(xring_pcie->pci->dev,
			"%s wait sram_init_done failed\n", __func__);
		return -1;
	}

	/* Load Firmware to SRAM, SRAM Base on DWC_PHY CSR 0x8000  */
	for (i = 0; i < sizeof(xring_pcie_phy_firmware) / sizeof(u32); i++)
		writel_relaxed(xring_pcie_phy_firmware[i], xring_pcie->pcie_phy_apb0 + ((0x8000 + i) << 2));

	/* wmb after writel_relaxed */
	wmb();

	xring_pcie_sram_load_done(xring_pcie, true);

	return 0;
}

static void xring_pcie_clkreq_sw_mux_set(struct xring_pcie *xring_pcie, bool state)
{
	u32 val;

	val = readl(xring_pcie->pcie_phy + PCIE_PHY_PCIE_SYS_TOP_CFG);
	if (state) {
		if ((val & (1 << PCIE_PHY_PCIE_SYS_TOP_CFG_SW_MUX_CLKREQ_SHIFT)) == 0) {
			val |= 1 << PCIE_PHY_PCIE_SYS_TOP_CFG_SW_MUX_CLKREQ_SHIFT;
			dev_info(xring_pcie->pci->dev, "set clk_req_n off by sw.\n");
		} else {
			dev_err(xring_pcie->pci->dev, "already set clk_req_n off by sw.\n");
		}
	} else {
		if ((val & (1 << PCIE_PHY_PCIE_SYS_TOP_CFG_SW_MUX_CLKREQ_SHIFT)) != 0) {
			val &= ~PCIE_PHY_PCIE_SYS_TOP_CFG_SW_MUX_CLKREQ_MASK;
			dev_info(xring_pcie->pci->dev, "set clk_req_n follow hw.\n");
		} else {
			dev_err(xring_pcie->pci->dev, "already set clk_req_n follow hw.\n");
		}
	}

	writel(val, xring_pcie->pcie_phy + PCIE_PHY_PCIE_SYS_TOP_CFG);
}

static void xring_pcie_clear_sys_int(struct xring_pcie *xring_pcie)
{
	struct device *dev = xring_pcie->pci->dev;
	int i;

	for (i = 0; i < SYS_INT_SET_NUM; i++)
		writel(0xffffffff, xring_pcie->pci_ctrl +
			PCIE_CTRL_SYS_INT0_IRQ_INT_RAW + i * SYS_INT_BLOCK_SIZE);

	dev_info(dev, "Clear previous sys int.\n");
}

static void xring_pcie_unmask_sys_int(struct xring_pcie *xring_pcie)
{
	struct device *dev = xring_pcie->pci->dev;
	struct device_node *np = dev->of_node;
	int ret;
	int i;

	ret = of_property_read_variable_u32_array(np,
						"sys-int-mask",
						xring_pcie->sys_int_mask,
						SYS_INT_SET_NUM,
						SYS_INT_SET_NUM);
	if (ret < 0) {
		dev_err(dev, "failed to parse sys-int-mask\n");
		return;
	}

	xring_pcie->sysint_mask_flag = false;
	for (i = 0; i < SYS_INT_SET_NUM; i++)
		writel(xring_pcie->sys_int_mask[i],
			xring_pcie->pci_ctrl +
			PCIE_CTRL_SYS_INT0_IRQ_INT_MASK +
			i * SYS_INT_BLOCK_SIZE);
}

#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
static int xring_pcie_rc_init(struct pcie_port *pp)
#else
static int xring_pcie_rc_init(struct dw_pcie_rp *pp)
#endif
{
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct xring_pcie *xring_pcie = to_xring_pcie(pci);
	struct device *dev = xring_pcie->pci->dev;
	struct device_node *np = dev->of_node;
	u32 reg;
	int pm;

	xring_pcie_clear_sys_int(xring_pcie);

	pp->bridge->ops = &xring_pcie_ops;
	if (!xring_pcie->insr)
		xring_pcie_rc_rst_assert(xring_pcie);

	xring_pcie_rc_rst_deassert(xring_pcie);

	/* set elastic buff half full mode */
	reg = dw_pcie_readl_dbi(pci, LANE_SKEW_OFF);
	reg &= ~ELASTIC_BUFFER_MODE;
	dw_pcie_writel_dbi(pci, LANE_SKEW_OFF, reg);

	dw_pcie_dbi_ro_wr_en(pci);

	/* set did and vid as xiaomi pci-sig */
	if (!of_property_read_u32(np, "device-vendor-id", &xring_pcie->device_vendor_id))
		dw_pcie_writel_dbi(pci, PCI_VENDOR_ID, xring_pcie->device_vendor_id);

	/* set axcache as 0 */
	reg = dw_pcie_readl_dbi(pci, COHERENCY_CONTROL_3_OFF);
	reg |= CFG_MSTER_AWCACHE_MODE_MASK;
	dw_pcie_writel_dbi(pci, COHERENCY_CONTROL_3_OFF, reg);

	/* disable pme */
	pm = dw_pcie_find_capability(pci, PCI_CAP_ID_PM);
	reg = dw_pcie_readw_dbi(pci, pm + PCI_PM_PMC);
	reg &= ~PCI_PM_CAP_PME_MASK;
	dw_pcie_writew_dbi(pci, pm + PCI_PM_PMC, reg);

	/* set aux clk rate */
	reg = dw_pcie_readl_dbi(pci, AUX_CLK_FREQ_OFF);
	reg &= ~AUX_CLK_FREQ_MASK;
	reg |= AUX_CLK_FREQ;
	dw_pcie_writel_dbi(pci, AUX_CLK_FREQ_OFF, reg);

	/* Enable as 0xFFFF0001 response for CRS */
	reg = dw_pcie_readl_dbi(pci, AMBA_ERROR_RESPONSE_DEFAULT_OFF);
	reg &= ~AMBA_ERROR_RESPONSE_CRS_MASK;
	reg |= (AMBA_ERROR_RESPONSE_CRS_OKAY_FFFF0001 <<
		AMBA_ERROR_RESPONSE_CRS_BIT);
	dw_pcie_writel_dbi(pci, AMBA_ERROR_RESPONSE_DEFAULT_OFF, reg);

	if (of_property_read_bool(np, "aspm-disable"))
		xring_pcie_dis_aspm_cap(pp);
	else if (of_property_read_bool(np, "l1ss-disable") || xring_pcie->is_fpga)
		xring_pcie_dis_l1ss_cap(pp);

	dw_pcie_dbi_ro_wr_dis(pci);

	xring_pcie_unmask_sys_int(xring_pcie);

	return 0;
}

#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
#else
static void xring_pcie_rc_deinit(struct dw_pcie_rp *pp)
{
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct xring_pcie *xring_pcie = to_xring_pcie(pci);

	xring_pcie_rc_rst_assert(xring_pcie);
}
#endif

static const struct dw_pcie_host_ops xring_pcie_host_ops = {
	.host_init =		xring_pcie_rc_init,
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
#else
	.host_deinit =		xring_pcie_rc_deinit,
#endif
};

void set_num_lanes(struct dw_pcie *pci)
{
	u32 val = 0;

	/* Set the number of lanes */
	val = dw_pcie_readl_dbi(pci, PCIE_PORT_LINK_CONTROL);
	val &= ~PORT_LINK_MODE_MASK;
	switch (num_lanes) {
	case 1:
		val |= PORT_LINK_MODE_1_LANES;
		break;
	case 2:
		val |= PORT_LINK_MODE_2_LANES;
		break;
	case 4:
		val |= PORT_LINK_MODE_4_LANES;
		break;
	case 8:
		val |= PORT_LINK_MODE_8_LANES;
		break;
	default:
		dev_err(pci->dev,
			"num-lanes %u: invalid value, use default value\n",
			num_lanes);
		return;
	}
	dw_pcie_writel_dbi(pci, PCIE_PORT_LINK_CONTROL, val);

	/* Set link width speed control register */
	val = dw_pcie_readl_dbi(pci, PCIE_LINK_WIDTH_SPEED_CONTROL);
	val &= ~PORT_LOGIC_LINK_WIDTH_MASK;
	switch (num_lanes) {
	case 1:
		val |= PORT_LOGIC_LINK_WIDTH_1_LANES;
		break;
	case 2:
		val |= PORT_LOGIC_LINK_WIDTH_2_LANES;
		break;
	case 4:
		val |= PORT_LOGIC_LINK_WIDTH_4_LANES;
		break;
	case 8:
		val |= PORT_LOGIC_LINK_WIDTH_8_LANES;
		break;
	}
	dw_pcie_writel_dbi(pci, PCIE_LINK_WIDTH_SPEED_CONTROL, val);
}

static int xring_pcie_wait_for_phystatus(struct xring_pcie *xring_pcie)
{
	int retries;
	u32 reg;

	for (retries = 0; retries < PHY_WAIT_MAX_RETRIES; retries++) {
		reg = readl(xring_pcie->pci_ctrl + PCIE_CTRL_PHY_STATUS);
		reg &= BIT(PCIE_CTRL_PHY_STATUS_PHY_MAC_PHYSTATUS_SHIFT);
		if (reg == PCIE_CTRL_PHY_STATUS_PHY_MAC_PHYSTATUS_MASK) {
			dev_info(xring_pcie->pci->dev, "phy status is ready\n");
			return 0;
		}
		usleep_range(PHY_WAIT_USLEEP_MIN, PHY_WAIT_USLEEP_MAX);
	}

	dev_err(xring_pcie->pci->dev, "phy status is not ready\n");

	return -ETIMEDOUT;
}

static int xring_pcie_wait_for_link(struct dw_pcie *pci)
{
	int retries;

	for (retries = 0; retries < LINK_WAIT_MAX_RETRIES; retries++) {
		if (dw_pcie_link_up(pci))
			return 0;
		usleep_range(LINK_WAIT_USLEEP_MIN, LINK_WAIT_USLEEP_MAX);
	}

	dev_info(pci->dev, "Phy link never came up\n");
	return -ETIMEDOUT;
}

void phy_status_not_ready_check(struct xring_pcie *xring_pcie)
{
	u32 reg1, reg2;

	if (!xring_pcie->insr)
		return;

	reg1 = readl(xring_pcie->pcie_phy + 0x1c);
	reg1 |= BIT(1);
	writel(reg1, xring_pcie->pcie_phy + 0x1c);

	mdelay(10);
	reg1 = readl(xring_pcie->pcie_phy + 0x28);
	reg2 = readl(xring_pcie->pcie_phy + 0x18);

	dev_info(xring_pcie->pci->dev, "phy reference clk done:%d", (reg1 & BIT(19)) == BIT(19));
	dev_info(xring_pcie->pci->dev, "sram init done success:%d", (reg2 & BIT(20)) == BIT(20));

	reg1 = readl(xring_pcie->pcie_phy + 0x1c);
	reg1 &= ~BIT(1);
	writel(reg1, xring_pcie->pcie_phy + 0x1c);
}

void dump_link_info(struct device *dev)
{
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);
	int offset = 0;
	int i;

	for (offset = PCIE_CTRL_SMLH_LTSSM_STATE;
		offset <= PCIE_CTRL_SMLH_LTSSM_STATE_32D; offset += 16) {
		dev_info(dev, "smlh_state: 0x%04x:\t0x%08x\t0x%08x\t0x%08x\t0x%08x\n", offset,
			readl(xring_pcie->pci_ctrl + PCI_CTRL_REG_INDEX_0 + offset),
			readl(xring_pcie->pci_ctrl + PCI_CTRL_REG_INDEX_4 + offset),
			readl(xring_pcie->pci_ctrl + PCI_CTRL_REG_INDEX_8 + offset),
			readl(xring_pcie->pci_ctrl + PCI_CTRL_REG_INDEX_C + offset));
	}

	for (i = 0; i < SYS_INT_SET_NUM; i++)
		dev_info(dev, "sys int status: set%d:\t0x%08x\n", i,
			readl(xring_pcie->pci_ctrl +
			PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS + i * SYS_INT_BLOCK_SIZE));
}

static int xring_pcie_start_link(struct dw_pcie *pci)
{
	struct xring_pcie *xring_pcie = to_xring_pcie(pci);
	u32 reg;
	int retries;
	int ret;

	/* Dynamically modifying the number of lanes when insmod */
	if (num_lanes)
		set_num_lanes(pci);

	ret = xring_pcie_wait_for_phystatus(xring_pcie);
	if (ret)
		goto err_phy;

	reg = readl(xring_pcie->pci_ctrl + PCIE_CTRL_SII_GENERAL_CORE_CTRL);
	reg |= PCIE_CTRL_SII_GENERAL_CORE_CTRL_APP_LTSSM_ENABLE_MASK;
	writel(reg, xring_pcie->pci_ctrl + PCIE_CTRL_SII_GENERAL_CORE_CTRL);

	for (retries = 0; retries < XRING_LINK_WAIT_MAX_RETRIES; retries++) {
		if (dw_pcie_link_up(pci))
			return 0;
		usleep_range(XRING_LINK_WAIT_USLEEP_MIN, XRING_LINK_WAIT_USLEEP_MAX);
	}

	dev_info(pci->dev, "Phy link never came up\n");

err_phy:
	dump_link_info(pci->dev);
	phy_status_not_ready_check(xring_pcie);
	pci->ops->stop_link(pci);

	return -ETIMEDOUT;
}

static void xring_pcie_stop_link(struct dw_pcie *pci)
{
	struct xring_pcie *xring_pcie = to_xring_pcie(pci);
	struct device *dev = xring_pcie->pci->dev;
	u32 reg;

	if (!xring_pcie->insr)
		xring_pcie_dev_rst_assert(xring_pcie);

	reg = readl(xring_pcie->pci_ctrl + PCIE_CTRL_SII_GENERAL_CORE_CTRL);
	reg &= ~PCIE_CTRL_SII_GENERAL_CORE_CTRL_APP_LTSSM_ENABLE_MASK;
	writel(reg, xring_pcie->pci_ctrl + PCIE_CTRL_SII_GENERAL_CORE_CTRL);

	dev_info(dev, "RC ltssm disable.\n");
}

static int xring_pcie_link_up(struct dw_pcie *pci)
{
	struct xring_pcie *xring_pcie = to_xring_pcie(pci);
	u32 debug_info_0, link_statu;

	debug_info_0 = readl(xring_pcie->pci_ctrl + PCIE_CTRL_SII_DEBUG_INFO_0);
	link_statu = readl(xring_pcie->pci_ctrl + PCIE_CTRL_SII_LINK_RST_OR_STATU);

	return (debug_info_0 & PCIE_CTRL_SII_DEBUG_INFO_0_RDLH_LINK_UP_MASK) &&
		(link_statu & PCIE_CTRL_SII_LINK_RST_OR_STATU_SMLH_LINK_UP_MASK);
}

static const struct dw_pcie_ops xring_dw_pcie_ops = {
	.read_dbi = xring_pcie_read_apb32_to_dbi,
	.write_dbi = xring_pcie_write_apb32_to_dbi,
	.start_link = xring_pcie_start_link,
	.stop_link = xring_pcie_stop_link,
	.link_up = xring_pcie_link_up,
};

static irqreturn_t xring_pcie_sys_irq_handler(int irq, void *arg)
{
	struct xring_pcie *xring_pcie = arg;
	u32 reg;
	int i;

	for (i = 0; i < SYS_INT_SET_NUM; i++) {
		reg = readl(xring_pcie->pci_ctrl +
				PCIE_CTRL_SYS_INT0_IRQ_INT_STATUS +
				i * SYS_INT_BLOCK_SIZE);
		if (reg) {
			dev_err(xring_pcie->pci->dev,
				"pcie sys int triggered: set = %d, val = 0x%x\n",
				i, reg);
			writel(reg, xring_pcie->pci_ctrl +
				PCIE_CTRL_SYS_INT0_IRQ_INT_RAW +
				i * SYS_INT_BLOCK_SIZE);
		}
	}

	return IRQ_HANDLED;
}

static ssize_t pci_ctrl_register_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);
	int offset = 0;

	for (offset = 0; offset < PCI_CTRL_REG_SIZE; offset += 16) {
		dev_info(dev, "0x%04x:\t0x%08x\t0x%08x\t0x%08x\t0x%08x\n", offset,
			readl(xring_pcie->pci_ctrl + PCI_CTRL_REG_INDEX_0 + offset),
			readl(xring_pcie->pci_ctrl + PCI_CTRL_REG_INDEX_4 + offset),
			readl(xring_pcie->pci_ctrl + PCI_CTRL_REG_INDEX_8 + offset),
			readl(xring_pcie->pci_ctrl + PCI_CTRL_REG_INDEX_C + offset));
	}

	return 0;
}

static DEVICE_ATTR_RO(pci_ctrl_register);

static ssize_t enable_sys_int_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	dev_info(dev, "Use this sysfs to unmask sys interrupt\n");
	dev_info(dev, "Usage: echo [int_num] > msi_alloc\n");

	return 0;
}

static ssize_t enable_sys_int_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);
	int irq_num;
	u32 reg;
	enum sys_int_reg_count reg_count;
	int offs;

	if (kstrtoint(buf, 0, &irq_num) < 0)
		return -EINVAL;

	reg_count = irq_num / SYS_INT_PER_REG;

	switch (reg_count) {
	case SYS_INT_COUNTER0:
		offs = PCIE_CTRL_SYS_INT0_IRQ_INT_MASK;
		break;
	case SYS_INT_COUNTER1:
		offs = PCIE_CTRL_SYS_INT1_IRQ_INT_MASK;
		break;
	case SYS_INT_COUNTER2:
		offs = PCIE_CTRL_SYS_INT2_IRQ_INT_MASK;
		break;
	case SYS_INT_COUNTER3:
		offs = PCIE_CTRL_SYS_INT3_IRQ_INT_MASK;
		break;
	default:
		dev_err(dev, "Invalid interrupt num: %d\n", irq_num);
		return count;
	}

	irq_num = irq_num % SYS_INT_PER_REG;

	reg = readl(xring_pcie->pci_ctrl + offs);
	reg &= ~BIT(irq_num);
	writel(reg, xring_pcie->pci_ctrl + offs);

	return count;
}

static DEVICE_ATTR_RW(enable_sys_int);

static ssize_t aspm_state_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);
	u8 offset = dw_pcie_find_capability(xring_pcie->pci, PCI_CAP_ID_EXP);
	u32 val;

	val = dw_pcie_readl_dbi(xring_pcie->pci, offset + PCI_EXP_LNKCTL);

	dev_info(dev, "%d\n", val & PCI_EXP_LNKCTL_ASPMC);
	dev_info(dev, "0:disabled; 1:L0s; 2:L1; 3:L0s and L1;\n");

	return 0;
}

static ssize_t aspm_state_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);
	u8 offset = dw_pcie_find_capability(xring_pcie->pci, PCI_CAP_ID_EXP);
	unsigned long aspm_state;
	u32 val;

	if (kstrtoul(buf, 0, &aspm_state) < 0)
		return -EINVAL;

	val = dw_pcie_readl_dbi(xring_pcie->pci, offset + PCI_EXP_LNKCTL);

	if (aspm_state <= PCI_EXP_LNKCTL_ASPMC) {
		val &= ~PCI_EXP_LNKCTL_ASPMC;
		val |= aspm_state;
		dw_pcie_writel_dbi(xring_pcie->pci, offset + PCI_EXP_LNKCTL, val);
	} else {
		dev_err(dev, "Invalid argument\n");
	}

	return count;
}

static DEVICE_ATTR_RW(aspm_state);

static ssize_t start_link_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	dev_info(dev, "use this sysfs to set RC ltssm enable if link never came up while insmod\n");
	dev_info(dev, "usage: echo 1 > start_link\n");

	return 0;
}

static ssize_t start_link_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);
	struct dw_pcie *pci = xring_pcie->pci;
	unsigned long start_link;

	if (kstrtoul(buf, 0, &start_link) < 0)
		return -EINVAL;

	if (start_link)
		pci->ops->start_link(pci);

	return count;
}

static DEVICE_ATTR_RW(start_link);

static ssize_t pcie_gpio_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);
	int pcie_gpio;

	pcie_gpio = gpio_get_value(xring_pcie->perstn_gpio);
	dev_info(dev, "Current level of perst_n gpio is : %d\n", pcie_gpio);

	return 0;
}

static ssize_t pcie_gpio_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);
	unsigned long pcie0_gpio = -1;

	if (kstrtoul(buf, 0, &pcie0_gpio) < 0)
		return -EINVAL;

	if (pcie0_gpio == 1)
		xring_pcie_dev_rst_deassert(xring_pcie);
	else if (pcie0_gpio == 0)
		xring_pcie_dev_rst_assert(xring_pcie);
	else
		dev_info(dev, "Invalid parament\n");

	return count;
}

static DEVICE_ATTR_RW(pcie_gpio);

static ssize_t set_speed_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);
	u16 linkstat;
	enum pci_bus_speed speed;
	u8 offset = dw_pcie_find_capability(xring_pcie->pci, PCI_CAP_ID_EXP);

	dev_info(dev, "use this sysfs to set target speed and retrain\n");
	dev_info(dev, "usage: echo <target_speed> > set_speed\n");

	linkstat = dw_pcie_readw_dbi(xring_pcie->pci, offset + PCI_EXP_LNKSTA);
	speed = pcie_link_speed[linkstat & PCI_EXP_LNKSTA_CLS];

	return sysfs_emit(buf, "%s\n", pci_speed_string(speed));
}

static ssize_t set_speed_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);
	struct dw_pcie *pci = xring_pcie->pci;
	int target_speed;
	u16 linkstat, linkctl2, linkctl;
	u8 offset = dw_pcie_find_capability(xring_pcie->pci, PCI_CAP_ID_EXP);
	int ret;

	if (kstrtoint(buf, 0, &target_speed) < 0)
		return -EINVAL;

	if ((target_speed > PCI_EXP_LNKCTL2_TLS_16_0GT) ||
			(target_speed < PCI_EXP_LNKCTL2_TLS_2_5GT)) {
		dev_err(dev, "Invalid target_speed:%d\n", target_speed);
		return -EINVAL;
	}

	linkstat = dw_pcie_readw_dbi(xring_pcie->pci, offset + PCI_EXP_LNKSTA);
	if (target_speed == (linkstat & PCI_EXP_LNKSTA_CLS)) {
		dev_info(dev, "Current link speed is already %d\n", linkstat & PCI_EXP_LNKSTA_CLS);
		return count;
	}

	dev_info(dev, "Current link speed:%d", linkstat & PCI_EXP_LNKSTA_CLS);

	linkctl2 = dw_pcie_readw_dbi(xring_pcie->pci, offset + PCI_EXP_LNKCTL2);
	linkctl2 &= ~PCI_EXP_LNKCTL2_TLS;
	linkctl2 |= target_speed;
	dw_pcie_writew_dbi(xring_pcie->pci, offset + PCI_EXP_LNKCTL2, linkctl2);

	dev_info(dev, "Triggering link retraining...\n");
	linkctl = dw_pcie_readw_dbi(xring_pcie->pci, offset + PCI_EXP_LNKCTL);
	linkctl |= PCI_EXP_LNKCTL_RL;
	dw_pcie_writew_dbi(xring_pcie->pci, offset + PCI_EXP_LNKCTL, linkctl);

	ret = dw_pcie_wait_for_link(pci);
	if (ret) {
		dev_err(dev, "Failed to retrain link.\n");
	} else {
		linkstat = dw_pcie_readw_dbi(xring_pcie->pci, offset + PCI_EXP_LNKSTA);
		dev_info(dev, "Set target speed succeed. Current link speed:%d",
			linkstat & PCI_EXP_LNKSTA_CLS);
	}

	return count;
}

static DEVICE_ATTR_RW(set_speed);

static int xring_pcie_wait_smmu_bit(struct xring_pcie *xring_pcie, int offset, u32 val)
{
	int retries;
	u32 reg;

	for (retries = 0; retries < XRING_LINK_WAIT_MAX_RETRIES; retries++) {
		regmap_read(xring_pcie->regmap_hss2_top, offset, &reg);
		if (reg & val)
			return 0;
		usleep_range(XRING_LINK_WAIT_USLEEP_MIN, XRING_LINK_WAIT_USLEEP_MAX);
	}

	dev_err(xring_pcie->pci->dev, "failed to bypass smmu. offset:0x%x, reg:0x%x\n",
					offset, reg);

	return -ETIMEDOUT;
}

static void bypass_smmu(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);
	u32 reg;
	int ret;

	/* tcu */
	mutex_lock(&tcu_mutex);
	regmap_read(xring_pcie->regmap_hss2_top, HSS2_TOP_SMMU_TCU_QCH_PD_STATE, &reg);
	if ((reg & SC_SMMU_TCU_QACCEPTN_PD) != SC_SMMU_TCU_QACCEPTN_PD) {

		regmap_write(xring_pcie->regmap_hss2_top,
					HSS2_TOP_SMMU_TCU_QCH_CG_CTRL, SC_SMMU_TCU_QREQN_CG_SET);
		ret = xring_pcie_wait_smmu_bit(xring_pcie,
					HSS2_TOP_SMMU_TCU_QCH_CG_STATE, SC_SMMU_TCU_QACCEPTN_CG_ST);
		if (ret) {
			mutex_unlock(&tcu_mutex);
			return;
		}

		regmap_write(xring_pcie->regmap_hss2_top,
					HSS2_TOP_SMMU_TCU_QCH_PD_CTRL, SC_SMMU_TCU_QREQN_PD);
		ret = xring_pcie_wait_smmu_bit(xring_pcie,
					HSS2_TOP_SMMU_TCU_QCH_PD_STATE, SC_SMMU_TCU_QACCEPTN_PD);
		if (ret) {
			mutex_unlock(&tcu_mutex);
			return;
		}
	}
	mutex_unlock(&tcu_mutex);

	/* tbu0 */
	regmap_read(xring_pcie->regmap_hss2_top, HSS2_TOP_SMMU_TBU0_QCH_PD_STATE, &reg);
	if ((xring_pcie->port_num == 0) &&
		((reg & SC_SMMU_TCU_QACCEPTN_PD) != SC_SMMU_TCU_QACCEPTN_PD)) {

		regmap_write(xring_pcie->regmap_hss2_top,
					HSS2_TOP_SMMU_TBU0_QCH_CG_CTRL, SC_SMMU_TCU_QREQN_CG_SET);
		ret = xring_pcie_wait_smmu_bit(xring_pcie,
					HSS2_TOP_SMMU_TBU0_QCH_CG_STATE, SC_SMMU_TCU_QACCEPTN_CG_ST);
		if (ret)
			return;

		regmap_write(xring_pcie->regmap_hss2_top,
					HSS2_TOP_SMMU_TBU0_QCH_PD_CTRL, SC_SMMU_TCU_QREQN_PD);
		ret = xring_pcie_wait_smmu_bit(xring_pcie,
					HSS2_TOP_SMMU_TBU0_QCH_PD_STATE, SC_SMMU_TCU_QACCEPTN_PD);
		if (ret)
			return;
	}

	/* tbu1 */
	regmap_read(xring_pcie->regmap_hss2_top, HSS2_TOP_SMMU_TBU1_QCH_PD_STATE, &reg);
	if ((xring_pcie->port_num == 1) &&
		((reg & SC_SMMU_TCU_QACCEPTN_PD) != SC_SMMU_TCU_QACCEPTN_PD)) {

		regmap_write(xring_pcie->regmap_hss2_top,
					HSS2_TOP_SMMU_TBU1_QCH_CG_CTRL, SC_SMMU_TCU_QREQN_CG_SET);
		ret = xring_pcie_wait_smmu_bit(xring_pcie,
					HSS2_TOP_SMMU_TBU1_QCH_CG_STATE, SC_SMMU_TCU_QACCEPTN_CG_ST);
		if (ret)
			return;

		regmap_write(xring_pcie->regmap_hss2_top,
					HSS2_TOP_SMMU_TBU1_QCH_PD_CTRL, SC_SMMU_TCU_QREQN_PD);
		ret = xring_pcie_wait_smmu_bit(xring_pcie,
					HSS2_TOP_SMMU_TBU1_QCH_PD_STATE, SC_SMMU_TCU_QACCEPTN_PD);
		if (ret)
			return;
	}

	dev_info(dev, "smmu bypassed.\n");
}

static int xring_pcie_crg_init(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);
	u32 val;

	regmap_read(xring_pcie->regmap_hss2_crg, HSS2_CRG_RST0_RO, &val);
	val = (~val) & HSS2_CRG_RST0_PCIE;
	if (val)
		regmap_write(xring_pcie->regmap_hss2_crg, HSS2_CRG_RST0_W1S, val);

	if (xring_pcie->port_num == 0) {
		regmap_read(xring_pcie->regmap_hss2_crg, HSS2_CRG_RST0_RO, &val);
		val = (~val) & HSS2_CRG_RST0_PCIE0;
		if (val)
			regmap_write(xring_pcie->regmap_hss2_crg, HSS2_CRG_RST0_W1S, val);

	} else if (xring_pcie->port_num == 1) {
		regmap_read(xring_pcie->regmap_hss2_crg, HSS2_CRG_RST0_RO, &val);
		val = (~val) & HSS2_CRG_RST0_PCIE1;
		if (val)
			regmap_write(xring_pcie->regmap_hss2_crg, HSS2_CRG_RST0_W1S, val);

	} else {
		dev_err(dev, "Invalid port num, %d\n", xring_pcie->port_num);
		return -EINVAL;
	}

	dev_info(dev, "pcie%d CRG register init.\n", xring_pcie->port_num);

	return 0;
}

static void xring_pcie_powerup_rst_assert(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);

	if (xring_pcie->port_num == 0)
		regmap_write(xring_pcie->regmap_hss2_crg,
				HSS2_CRG_RST0_W1C,
				HSS2_CRG_RST0_RO_IP_RST_PCIE0_POWER_UP_N_MASK);
	else if (xring_pcie->port_num == 1)
		regmap_write(xring_pcie->regmap_hss2_crg,
				HSS2_CRG_RST0_W1C,
				HSS2_CRG_RST0_RO_IP_RST_PCIE1_POWER_UP_N_MASK);
	else
		dev_err(dev, "Invalid port num, %d\n", xring_pcie->port_num);

	dev_info(dev, "pcie%d pwr up rst.\n", xring_pcie->port_num);
}

static int xring_pcie_powerup_rst_deassert(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);

	if (xring_pcie->port_num == 0) {
		regmap_write(xring_pcie->regmap_hss2_crg,
				HSS2_CRG_RST0_W1S,
				HSS2_CRG_RST0_RO_IP_RST_PCIE0_POWER_UP_N_MASK);
	} else if (xring_pcie->port_num == 1) {
		regmap_write(xring_pcie->regmap_hss2_crg,
				HSS2_CRG_RST0_W1S,
				HSS2_CRG_RST0_RO_IP_RST_PCIE1_POWER_UP_N_MASK);
	} else {
		dev_err(dev, "Invalid port num, %d\n", xring_pcie->port_num);
		return -EINVAL;
	}

	dev_info(dev, "pcie%d pwr up de-rst.\n", xring_pcie->port_num);

	return 0;
}

static void xring_pcie_hss2_dbi_rst(struct platform_device *pdev)
{
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);

	if (xring_pcie->port_num == 0)
		regmap_write(xring_pcie->regmap_hss2_crg, HSS2_CRG_RST0_W1C,
			HSS2_CRG_RST0_RO_IP_PRST_PCIE0_DBI_N_MASK);
	else if (xring_pcie->port_num == 1)
		regmap_write(xring_pcie->regmap_hss2_crg, HSS2_CRG_RST0_W1C,
			HSS2_CRG_RST0_RO_IP_PRST_PCIE1_DBI_N_MASK);
}

static int xring_pcie_mem_shutdown(struct platform_device *pdev)
{
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);
	struct device *dev = &pdev->dev;
	void __iomem *pctrl;
	u32 val;

	pctrl = ioremap(PCIE_PCTRL_BASE, PCIE_PCTRL_SIZE);
	if (!pctrl) {
		dev_err(dev, "failed to ioremap pctrl\n");
		return -ENOMEM;
	}

	if (xring_pcie->port_num == 0) {
		val = (PCTRL_MEM_LGROUP0_SC_PCIE0_MEM_SD_MASK << 16) &
			(~PCTRL_MEM_LGROUP0_SC_PCIE0_MEM_SD_MASK);
		writel(val, pctrl + PCTRL_MEM_LGROUP0);

		if ((readl(pctrl + PCTRL_MEM_LPSTATUS0) &
			PCTRL_MEM_LPSTATUS0_PCIE0_MEM_SD_ACK_MASK) != 0) {
			dev_err(dev, "failed to set pcie0 mem shutdown\n");
			iounmap(pctrl);
			return -EINVAL;
		}

	} else if (xring_pcie->port_num == 1) {
		val = (PCTRL_MEM_LGROUP1_SC_PCIE1_MEM_SD_MASK << 16) &
			(~PCTRL_MEM_LGROUP1_SC_PCIE1_MEM_SD_MASK);
		writel(val, pctrl + PCTRL_MEM_LGROUP1);

		if ((readl(pctrl + PCTRL_MEM_LPSTATUS1) &
			PCTRL_MEM_LPSTATUS1_PCIE1_MEM_SD_ACK_MASK) != 0) {
			dev_err(dev, "failed to set pcie1 mem shutdown\n");
			iounmap(pctrl);
			return -EINVAL;
		}

	} else {
		dev_err(dev, "Invalid port num, %d\n", xring_pcie->port_num);
		iounmap(pctrl);
		return -EINVAL;
	}

	iounmap(pctrl);

	return 0;
}

static void xring_pcie_create_sysfs(struct device *dev)
{
	if (device_create_file(dev, &dev_attr_pci_ctrl_register) < 0)
		dev_err(dev, "Failed to create file pci_ctrl_register\n");

	if (device_create_file(dev, &dev_attr_enable_sys_int) < 0)
		dev_err(dev, "Failed to create file enable_sys_int\n");

	if (device_create_file(dev, &dev_attr_aspm_state) < 0)
		dev_err(dev, "Failed to create file aspm_state\n");

	if (device_create_file(dev, &dev_attr_start_link) < 0)
		dev_err(dev, "Failed to create file start_link\n");

	if (device_create_file(dev, &dev_attr_pcie_gpio) < 0)
		dev_err(dev, "Failed to create file pcie gpio\n");

	if (device_create_file(dev, &dev_attr_set_speed) < 0)
		dev_err(dev, "Failed to create file set_speed\n");
}

static int xring_pcie_dev_rst_init(struct xring_pcie *xring_pcie)
{
	struct device *dev = xring_pcie->pci->dev;
	struct device_node *np = dev->of_node;
	int ret;

	xring_pcie->perstn_gpio = of_get_named_gpio(np, "perstn-gpios", 0);
	if (xring_pcie->perstn_gpio < 0) {
		dev_err(dev, "Failed to request GPIO perst_n\n");
		return xring_pcie->perstn_gpio;
	}

	if (of_property_read_bool(np, "dev-perst-always-on")) {
		dev_info(dev, "ep perst needs always on\n");

		ret = gpio_direction_output(xring_pcie->perstn_gpio, 1);
		if (ret < 0) {
			dev_err(dev, "gpio_direction_output failed:%d\n", ret);
			return ret;
		}
		return 0;
	}

	ret = gpio_direction_output(xring_pcie->perstn_gpio, 0);
	if (ret < 0) {
		dev_err(dev, "gpio_direction_output failed:%d\n", ret);
		return ret;
	}

	xring_pcie_dev_rst_assert(xring_pcie);
	xring_pcie_dev_rst_deassert(xring_pcie);

	return 0;
}

static void xring_pcie_mask_sys_int(struct xring_pcie *xring_pcie)
{
	struct device *dev = xring_pcie->pci->dev;
	int i;

	xring_pcie->sysint_mask_flag = true;

	for (i = 0; i < SYS_INT_SET_NUM; i++)
		writel(0xffffffff, xring_pcie->pci_ctrl +
			PCIE_CTRL_SYS_INT0_IRQ_INT_MASK + i * SYS_INT_BLOCK_SIZE);

	dev_info(dev, "mask all sys int\n");
}

static void xring_pcie_save_l1ss_ctrl(struct xring_pcie *xring_pcie)
{
	struct dw_pcie *pci = xring_pcie->pci;
	struct device *dev = pci->dev;
	u16 l1ss_cap;

	l1ss_cap = dw_pcie_find_ext_capability(pci, PCI_EXT_CAP_ID_L1SS);
	if (!l1ss_cap) {
		dev_info(dev, "Can't find L1SS capability\n");
		return;
	}

	xring_pcie->saved_l1ss_ctl2 = dw_pcie_readl_dbi(pci, l1ss_cap + PCI_L1SS_CTL2);
	xring_pcie->saved_l1ss_ctl1 = dw_pcie_readl_dbi(pci, l1ss_cap + PCI_L1SS_CTL1);
	dev_info(dev, "Save L1SS CTL1=%#x, CTL2=%#x\n", xring_pcie->saved_l1ss_ctl1,
			xring_pcie->saved_l1ss_ctl2);
}

static void xring_pcie_restore_l1ss_ctrl(struct xring_pcie *xring_pcie)
{
	struct dw_pcie *pci = xring_pcie->pci;
	struct device *dev = pci->dev;
	u16 l1ss_cap;

	l1ss_cap = dw_pcie_find_ext_capability(pci, PCI_EXT_CAP_ID_L1SS);
	if (!l1ss_cap) {
		dev_info(dev, "Can't find L1SS capability\n");
		return;
	}

	dw_pcie_writel_dbi(pci, l1ss_cap + PCI_L1SS_CTL2, xring_pcie->saved_l1ss_ctl2);
	dw_pcie_writel_dbi(pci, l1ss_cap + PCI_L1SS_CTL1, xring_pcie->saved_l1ss_ctl1);
	dev_info(dev, "Restore L1SS CTL1=%#x, CTL2=%#x\n", xring_pcie->saved_l1ss_ctl1,
			xring_pcie->saved_l1ss_ctl2);
}

static void xring_pcie_save_msi(struct xring_pcie *xring_pcie)
{
	struct dw_pcie *pci = xring_pcie->pci;
	u32 ctrl;

	for (ctrl = 0; ctrl < MAX_MSI_CTRLS; ctrl++) {
		xring_pcie->saved_msi_mask[ctrl] =
				dw_pcie_readl_dbi(pci, PCIE_MSI_INTR0_MASK +
						(ctrl * MSI_REG_CTRL_BLOCK_SIZE));
		xring_pcie->saved_msi_enable[ctrl] =
				dw_pcie_readl_dbi(pci, PCIE_MSI_INTR0_ENABLE +
						(ctrl * MSI_REG_CTRL_BLOCK_SIZE));
	}
}

static void xring_pcie_restore_msi(struct xring_pcie *xring_pcie)
{
	struct dw_pcie *pci = xring_pcie->pci;
	u32 ctrl;

	for (ctrl = 0; ctrl < MAX_MSI_CTRLS; ctrl++) {
		dw_pcie_writel_dbi(pci, PCIE_MSI_INTR0_MASK +
				(ctrl * MSI_REG_CTRL_BLOCK_SIZE),
				xring_pcie->saved_msi_mask[ctrl]);
		dw_pcie_writel_dbi(pci, PCIE_MSI_INTR0_ENABLE +
				(ctrl * MSI_REG_CTRL_BLOCK_SIZE),
				xring_pcie->saved_msi_enable[ctrl]);
	}
}

static int xring_pcie_clk_enable(struct xring_pcie *xring_pcie)
{
	struct dw_pcie *pci = xring_pcie->pci;
	struct device *dev = pci->dev;
	struct xring_pcie_clk *clk = xring_pcie->clk;
	int ret;

	if (xring_pcie->clk_enable) {
		dev_info(dev, "clk is already enabled.\n");
		return 0;
	}

	ret = clk_prepare_enable(clk->pclk_hss2_tcu_slv);
	if (ret) {
		dev_err(dev, "failed to enable clk pclk_hss2_tcu_slv\n");
		return ret;
	}

	ret = clk_prepare_enable(clk->aclk_hss2_tcu);
	if (ret) {
		dev_err(dev, "failed to enable clk aclk_hss2_tcu\n");
		goto err_hss2_tcu;
	}

	ret = clk_prepare_enable(clk->clk_tcu_tpc);
	if (ret) {
		dev_err(dev, "failed to enable clk clk_tcu_tpc\n");
		goto err_tcu_tpc;
	}

	ret = clk_prepare_enable(clk->pclk_pcie_apb);
	if (ret) {
		dev_err(dev, "failed to enable clk pclk_pcie_apb\n");
		goto err_pcie_apb;
	}

	ret = clk_prepare_enable(clk->aclk_pcie_axi_slv);
	if (ret) {
		dev_err(dev, "failed to enable clk aclk_pcie_axi_slv\n");
		goto err_axi_slv;
	}

	ret = clk_prepare_enable(clk->aclk_hss2_tbu);
	if (ret) {
		dev_err(dev, "failed to enable clk aclk_hss2_tbu\n");
		goto err_hss2_tbu;
	}

	ret = clk_prepare_enable(clk->aclk_pcie_axi_mst);
	if (ret) {
		dev_err(dev, "failed to enable clk aclk_pcie_axi_mst\n");
		goto err_axi_mst;
	}

	ret = clk_prepare_enable(clk->clk_pcie_aux);
	if (ret) {
		dev_err(dev, "failed to enable clk clk_pcie_aux\n");
		goto err_pcie_aux;
	}

	ret = clk_prepare_enable(clk->clk_pcie_tpc);
	if (ret) {
		dev_err(dev, "failed to enable clk clk_pcie_tpc\n");
		goto err_pcie_tpc;
	}

	ret = clk_prepare_enable(clk->clk_pcie_fw);
	if (ret) {
		dev_err(dev, "failed to enable clk clk_pcie_fw\n");
		goto err_pcie_fw;
	}

	xring_pcie->clk_enable = true;
	dev_info(dev, "enable clk done\n");

	return 0;

err_pcie_fw:
	clk_disable_unprepare(clk->clk_pcie_tpc);
err_pcie_tpc:
	clk_disable_unprepare(clk->clk_pcie_aux);
err_pcie_aux:
	clk_disable_unprepare(clk->aclk_pcie_axi_mst);
err_axi_mst:
	clk_disable_unprepare(clk->aclk_hss2_tbu);
err_hss2_tbu:
	clk_disable_unprepare(clk->aclk_pcie_axi_slv);
err_axi_slv:
	clk_disable_unprepare(clk->pclk_pcie_apb);
err_pcie_apb:
	clk_disable_unprepare(clk->clk_tcu_tpc);
err_tcu_tpc:
	clk_disable_unprepare(clk->aclk_hss2_tcu);
err_hss2_tcu:
	clk_disable_unprepare(clk->pclk_hss2_tcu_slv);

	xring_pcie->clk_enable = false;

	return ret;
}

static void xring_pcie_clk_disable(struct xring_pcie *xring_pcie)
{
	struct dw_pcie *pci = xring_pcie->pci;
	struct device *dev = pci->dev;
	struct xring_pcie_clk *clk = xring_pcie->clk;

	if (!xring_pcie->clk_enable) {
		dev_info(dev, "clk is already disabled.\n");
		return;
	}

	clk_disable_unprepare(clk->clk_pcie_fw);
	clk_disable_unprepare(clk->clk_pcie_tpc);
	clk_disable_unprepare(clk->clk_pcie_aux);
	clk_disable_unprepare(clk->aclk_pcie_axi_mst);
	clk_disable_unprepare(clk->aclk_hss2_tbu);
	clk_disable_unprepare(clk->aclk_pcie_axi_slv);
	clk_disable_unprepare(clk->pclk_pcie_apb);
	clk_disable_unprepare(clk->clk_tcu_tpc);
	clk_disable_unprepare(clk->aclk_hss2_tcu);
	clk_disable_unprepare(clk->pclk_hss2_tcu_slv);

	xring_pcie->clk_enable = false;
	dev_info(dev, "disable clk done.\n");
}

static int xring_pcie_parse_clk(struct xring_pcie *xring_pcie)
{
	struct dw_pcie *pci = xring_pcie->pci;
	struct device *dev = pci->dev;
	struct xring_pcie_clk *clk = xring_pcie->clk;
	struct device_node *np = dev->of_node;

	clk->pclk_hss2_tcu_slv = devm_clk_get(dev, "pclk_hss2_tcu_slv");
	if (IS_ERR(clk->pclk_hss2_tcu_slv)) {
		dev_err(dev, "failed to parse clk pclk_hss2_tcu_slv\n");
		return PTR_ERR(clk->pclk_hss2_tcu_slv);
	}

	clk->aclk_hss2_tcu = devm_clk_get(dev, "aclk_hss2_tcu");
	if (IS_ERR(clk->aclk_hss2_tcu)) {
		dev_err(dev, "failed to parse clk aclk_hss2_tcu\n");
		return PTR_ERR(clk->aclk_hss2_tcu);
	}

	clk->clk_tcu_tpc = devm_clk_get(dev, "clk_tcu_tpc");
	if (IS_ERR(clk->clk_tcu_tpc)) {
		dev_err(dev, "failed to parse clk clk_tcu_tpc\n");
		return PTR_ERR(clk->clk_tcu_tpc);
	}

	clk->pclk_pcie_apb = devm_clk_get(dev, "pclk_pcie_apb");
	if (IS_ERR(clk->pclk_pcie_apb)) {
		dev_err(dev, "failed to parse clk pclk_pcie_apb\n");
		return PTR_ERR(clk->pclk_pcie_apb);
	}

	clk->aclk_pcie_axi_slv = devm_clk_get(dev, "aclk_pcie_axi_slv");
	if (IS_ERR(clk->aclk_pcie_axi_slv)) {
		dev_err(dev, "failed to parse clk aclk_pcie_axi_slv\n");
		return PTR_ERR(clk->aclk_pcie_axi_slv);
	}

	clk->aclk_hss2_tbu = devm_clk_get(dev, "aclk_hss2_tbu");
	if (IS_ERR(clk->aclk_hss2_tbu)) {
		dev_err(dev, "failed to parse clk aclk_hss2_tbu\n");
		return PTR_ERR(clk->aclk_hss2_tbu);
	}

	clk->aclk_pcie_axi_mst = devm_clk_get(dev, "aclk_pcie_axi_mst");
	if (IS_ERR(clk->aclk_pcie_axi_mst)) {
		dev_err(dev, "failed to parse clk aclk_pcie_axi_mst\n");
		return PTR_ERR(clk->aclk_pcie_axi_mst);
	}

	clk->clk_pcie_aux = devm_clk_get(dev, "clk_pcie_aux");
	if (IS_ERR(clk->clk_pcie_aux)) {
		dev_err(dev, "failed to parse clk clk_pcie_aux\n");
		return PTR_ERR(clk->clk_pcie_aux);
	}

	clk->clk_pcie_tpc = devm_clk_get(dev, "clk_pcie_tpc");
	if (IS_ERR(clk->clk_pcie_tpc)) {
		dev_err(dev, "failed to parse clk clk_pcie_tpc\n");
		return PTR_ERR(clk->clk_pcie_tpc);
	}

	clk->clk_pcie_pll = devm_clk_get(dev, "clk_pcie_pll");
	if (IS_ERR(clk->clk_pcie_pll)) {
		dev_err(dev, "failed to parse clk clk_pcie_pll\n");
		return PTR_ERR(clk->clk_pcie_pll);
	}

	clk->clk_pcie_fw = devm_clk_get(dev, "clk_pcie_fw");
	if (IS_ERR(clk->clk_pcie_fw)) {
		dev_err(dev, "failed to parse clk clk_pcie_fw\n");
		return PTR_ERR(clk->clk_pcie_fw);
	}

	if (of_property_read_u32(np, "aux-clk-rate", &clk->aux_clk_rate))
		clk->aux_clk_rate = DFT_AUX_CLK_RATE;

	if (of_property_read_u32(np, "fw-clk-rate", &clk->fw_clk_rate))
		clk->fw_clk_rate = DFT_FW_CLK_RATE;

	return 0;
}

static void xring_pcie_phy_init(struct xring_pcie *xring_pcie)
{
	struct dw_pcie *pci = xring_pcie->pci;
	struct device *dev = pci->dev;
	struct device_node *np = dev->of_node;
	u32 de_emp;
	u32 reg;

	/* set refclk cmn mode */
	reg = readl(xring_pcie->pcie_phy + PCIE_PHY_PIPE_RX0_CFG);
	reg |= PCIE_PHY_PIPE_RX0_CFG_PIPE_RX0_ES0_CMN_REFCLK_MODE_MASK;
	writel(reg, xring_pcie->pcie_phy + PCIE_PHY_PIPE_RX0_CFG);

	reg = readl(xring_pcie->pcie_phy + PCIE_PHY_PIPE_RX1_CFG);
	reg |= PCIE_PHY_PIPE_RX1_CFG_PIPE_RX1_ES0_CMN_REFCLK_MODE_MASK;
	writel(reg, xring_pcie->pcie_phy + PCIE_PHY_PIPE_RX1_CFG);

	/* set diffbuf DE_EMP */
	reg = readl(xring_pcie->pcie_phy + PCIE_PHY_PCIE_SYS_TOP_CFG);
	reg &= ~PCIE_PHY_PCIE_SYS_TOP_CFG_SW_DE_EMP_MASK;
	if (!of_property_read_u32(np, "de_emp", &de_emp))
		reg |= de_emp << PCIE_PHY_PCIE_SYS_TOP_CFG_SW_DE_EMP_SHIFT;
	writel(reg, xring_pcie->pcie_phy + PCIE_PHY_PCIE_SYS_TOP_CFG);
}

static int xring_add_pcie_port(struct xring_pcie *xring_pcie,
				 struct platform_device *pdev)
{
	struct dw_pcie *pci = xring_pcie->pci;
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	struct pcie_port *pp = &pci->pp;
#else
	struct dw_pcie_rp *pp = &pci->pp;
#endif
	struct device *dev = &pdev->dev;
	int ret;

	xring_pcie->sys_intr = platform_get_irq_byname(pdev, "sys_intr");
	if (xring_pcie->sys_intr < 0)
		return xring_pcie->sys_intr;

	pp->num_vectors = MAX_MSI_IRQS;
	pp->ops = &xring_pcie_host_ops;

	ret = devm_request_irq(dev, xring_pcie->sys_intr, xring_pcie_sys_irq_handler,
			       IRQF_SHARED, "xring-pcie-sys", xring_pcie);
	if (ret) {
		dev_err(dev, "failed to request sys irq\n");
		return ret;
	}

	/* PCIe Phy Firmware update */
	if (!xring_pcie->is_fpga && of_property_read_bool(dev->of_node, "phy_firmware_upd")) {
		ret = xring_pcie_phy_firmware_upd(xring_pcie);
		if (ret)
			return ret;
	}

	ret = dw_pcie_host_init(pp);
	if (ret) {
		dev_err(dev, "Failed to initialize host\n");
		return ret;
	}

	xring_pcie->sr_data.sr_reg_list_len = of_property_count_elems_of_size(dev->of_node,
									"xring,sr_reg_list",
									sizeof(u32));

	ret = of_property_read_u32_array(dev->of_node,
						"xring,sr_reg_list",
						xring_pcie->sr_data.sr_reg_list,
						xring_pcie->sr_data.sr_reg_list_len);
	if (ret) {
		dev_err(dev, "Failed to read %d elements from xring,sr_reg_list: %d\n",
			xring_pcie->sr_data.sr_reg_list_len, ret);
		return ret;
	}
	xring_pcie->sr_data.sr_route_reg_len = of_property_count_elems_of_size(dev->of_node,
									"xring,route_data_address",
									sizeof(u32));
	ret = of_property_read_u32_array(dev->of_node,
						"xring,route_data_address",
						xring_pcie->sr_data.sr_route_reg_addr,
						xring_pcie->sr_data.sr_route_reg_len);
	if (ret) {
		dev_err(dev, "Failed to read %d elements from xring,route_data_address: %d\n",
			xring_pcie->sr_data.sr_route_reg_len, ret);
		return ret;
	}

	return 0;
}

#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
irqreturn_t xring_handle_msi_irq(struct pcie_port *pp)
#else
irqreturn_t xring_handle_msi_irq(struct dw_pcie_rp *pp)
#endif
{
	int i, pos;
	unsigned long val;
	u32 status, num_ctrls;
	u32 msi_mask;
	irqreturn_t ret = IRQ_NONE;
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct xring_pcie *xring_pcie = to_xring_pcie(pci);

	num_ctrls = pp->num_vectors / MAX_MSI_IRQS_PER_CTRL;

	for (i = 0; i < num_ctrls; i++) {
		status = dw_pcie_readl_dbi(pci, PCIE_MSI_INTR0_STATUS +
					   (i * MSI_REG_CTRL_BLOCK_SIZE));

		msi_mask = readl(xring_pcie->pci_ctrl + PCIE_CTRL_ACPU_MSI_ROUTE_0 +
				(i * MSI_ROUTE_BLOCK_SIZE));

		status &= msi_mask;
		if (!status)
			continue;

		ret = IRQ_HANDLED;
		val = status;
		pos = 0;
		while ((pos = find_next_bit(&val, MAX_MSI_IRQS_PER_CTRL,
					    pos)) != MAX_MSI_IRQS_PER_CTRL) {
			generic_handle_domain_irq(pp->irq_domain,
						  (i * MAX_MSI_IRQS_PER_CTRL) +
						  pos);
			pos++;
		}
	}

	return ret;
}

/* Chained MSI interrupt service routine */
static void xring_chained_msi_isr(struct irq_desc *desc)
{
	struct irq_chip *chip = irq_desc_get_chip(desc);
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	struct pcie_port *pp;
#else
	struct dw_pcie_rp *pp;
#endif

	chained_irq_enter(chip, desc);

	pp = irq_desc_get_handler_data(desc);
	xring_handle_msi_irq(pp);

	chained_irq_exit(chip, desc);
}

static int xring_pcie_msi_handler(struct xring_pcie *xring_pcie,
				struct platform_device *pdev)
{
	struct dw_pcie *pci = xring_pcie->pci;

#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	struct pcie_port *pp = &pci->pp;

	if (pp->msi_irq > 0)
		irq_set_chained_handler_and_data(pp->msi_irq,
					xring_chained_msi_isr,
					pp);
#else
	struct dw_pcie_rp *pp = &pci->pp;
	u32 ctrl, num_ctrls;

	num_ctrls = pp->num_vectors / MAX_MSI_IRQS_PER_CTRL;

	for (ctrl = 0; ctrl < num_ctrls; ctrl++) {
		if (pp->msi_irq[0] > 0)
			irq_set_chained_handler_and_data(pp->msi_irq[0],
						xring_chained_msi_isr,
						pp);
	}
#endif
	return 0;
}

static int xring_pcie_clk_set_rate(struct xring_pcie *xring_pcie)
{
	struct dw_pcie *pci = xring_pcie->pci;
	struct device *dev = pci->dev;
	struct xring_pcie_clk *clk = xring_pcie->clk;
	int ret;

	ret = clk_set_rate(clk->clk_pcie_aux,
			clk->aux_clk_rate);
	if (ret) {
		dev_err(dev, "failed to set clk_pcie_aux rate\n");
		return ret;
	}

	ret = clk_set_rate(clk->clk_pcie_fw,
			clk->fw_clk_rate);
	if (ret) {
		dev_err(dev, "failed to set clk_pcie_fw rate\n");
		return ret;
	}

	return 0;
}

static void xring_pcie_sram_boot_mode(struct xring_pcie *xring_pcie)
{
	u32 reg;

	reg = readl(xring_pcie->pcie_phy + PCIE_PHY_PHY_MISC_CFG);
	reg &= ~PCIE_PHY_PHY_MISC_CFG_PHY0_SRAM_BYPASS_MODE_MASK;
	reg &= ~PCIE_PHY_PHY_MISC_CFG_PHY0_SRAM_BOOTLOAD_BYPASS_MODE_MASK;
	reg |= 0x3 << PCIE_PHY_PHY_MISC_CFG_PHY0_SRAM_BOOTLOAD_BYPASS_MODE_SHIFT;
	reg |= 0x0 << PCIE_PHY_PHY_MISC_CFG_PHY0_SRAM_BYPASS_MODE_SHIFT;
	writel(reg, xring_pcie->pcie_phy + PCIE_PHY_PHY_MISC_CFG);

	reg = readl(xring_pcie->pcie_phy + PCIE_PHY_PHY_MISC_CFG);
	dev_info(xring_pcie->pci->dev, "Change rom boot mode to sram boot mode. reg value:0x%x\n", reg);
}

void xring_pcie_hss2_top_doze_unmask(struct xring_pcie *xring_pcie, bool unmask)
{
	u32 reg = 0;
	u32 val = 0;

	if (xring_pcie_status.modem == 0)
		val = HSS2_TOP_HSS2_DOZE_MASK_PCIE0_L1SS_STATE_MASK_MASK;
	if (xring_pcie_status.wifi == 0)
		val |= HSS2_TOP_HSS2_DOZE_MASK_PCIE1_L1SS_STATE_MASK_MASK;
	if (!val)
		return;

	regmap_read(xring_pcie->regmap_hss2_top, HSS2_TOP_HSS2_DOZE_MASK, &reg);
	if (unmask)
		reg &= ~val;
	else
		reg |= val;
	regmap_write(xring_pcie->regmap_hss2_top, HSS2_TOP_HSS2_DOZE_MASK, reg);
}

static void xring_pcie_L12_doze(struct xring_pcie *xring_pcie, bool status)
{
	struct device *dev = xring_pcie->pci->dev;

	dev_info(dev, "xring_pcie L12 doze mask status=%d\n", status);
	writel(status, xring_pcie->pci_ctrl + PCIE_CTRL_MASK_L12_DOZE);
}

static void xring_pcie_enable_rpm(struct xring_pcie *xring_pcie)
{
	struct pci_host_bridge *bridge;

	bridge = xring_pcie->pci->pp.bridge;

	pm_runtime_forbid(&bridge->dev);
	pm_runtime_set_active(&bridge->dev);
	pm_runtime_enable(&bridge->dev);
	pm_runtime_put_noidle(xring_pcie->pci->dev);

	pm_runtime_set_active(xring_pcie->pci->dev);
	pm_runtime_enable(xring_pcie->pci->dev);

	dev_info(xring_pcie->pci->dev, "enable pcie rpm.\n");
}

static void xring_pcie_disable_rpm(struct xring_pcie *xring_pcie)
{
	struct pci_host_bridge *bridge;

	bridge = xring_pcie->pci->pp.bridge;

	pm_runtime_get(xring_pcie->pci->dev);
	pm_runtime_disable(xring_pcie->pci->dev);
	pm_runtime_set_suspended(xring_pcie->pci->dev);

	pm_runtime_disable(&bridge->dev);
	pm_runtime_set_suspended(&bridge->dev);

	dev_info(xring_pcie->pci->dev, "disable pcie rpm.\n");
}

static int xring_pcie_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct xring_pcie *xring_pcie;
	struct dw_pcie *pci;
	struct device_node *np = dev->of_node;
	struct xring_pcie_clk *clk;
	int ret;

	xring_pcie = devm_kzalloc(dev, sizeof(*xring_pcie), GFP_KERNEL);
	if (!xring_pcie)
		return -ENOMEM;

	pci = devm_kzalloc(dev, sizeof(*pci), GFP_KERNEL);
	if (!pci)
		return -ENOMEM;

	clk = devm_kzalloc(dev, sizeof(*clk), GFP_KERNEL);

	pci->dev = dev;
	pci->ops = &xring_dw_pcie_ops;
	xring_pcie->pci = pci;
	xring_pcie->clk = clk;

	platform_set_drvdata(pdev, xring_pcie);

	xring_pcie->is_fpga = of_property_read_bool(np, "fpga");

	spin_lock_init(&xring_pcie->cfg_lock);
	xring_pcie->port_num = of_get_pci_domain_nr(dev->of_node);
	if (xring_pcie->port_num < 0)
		dev_err(dev, "failed to get domain number\n");
	else
		pdev_list[xring_pcie->port_num] = pdev;

	if (of_property_read_bool(np, "fw-upd-workaround")) {
		dev_info(dev, "check whether ep is powered up\n");

		xring_pcie->ep_pmic_en_gpio = of_get_named_gpio(np, "ep-pmic-en-gpio", 0);
		if (xring_pcie->ep_pmic_en_gpio < 0) {
			dev_err(dev, "Failed to request GPIO ep pmic\n");
			return xring_pcie->ep_pmic_en_gpio;
		}

		if (!gpio_get_value(xring_pcie->ep_pmic_en_gpio)) {
			dev_err(dev, "EP not powered up yet, return here\n");
			return -EINVAL;
		}
	}

	xring_pcie->regmap_hss2_crg = syscon_regmap_lookup_by_phandle(np,
					"hss2-crg-syscon");
	if (IS_ERR(xring_pcie->regmap_hss2_crg)) {
		dev_err(dev, "unable to find hss2_crg syscon registers\n");
		return -ENOMEM;
	}

	xring_pcie->pcie_phy = devm_platform_ioremap_resource_byname(pdev, "pcie_phy");
	if (IS_ERR(xring_pcie->pcie_phy))
		return PTR_ERR(xring_pcie->pcie_phy);

	xring_pcie->pci_ctrl = devm_platform_ioremap_resource_byname(pdev, "pci_ctrl");
	if (IS_ERR(xring_pcie->pci_ctrl))
		return PTR_ERR(xring_pcie->pci_ctrl);

	xring_pcie->pcie_phy_apb0 = devm_platform_ioremap_resource_byname(pdev, "pcie_phy_apb0");
	if (IS_ERR(xring_pcie->pcie_phy_apb0))
		return PTR_ERR(xring_pcie->pcie_phy_apb0);

	ret = device_property_read_u32(dev, "reset-assert-ms",
					&xring_pcie->rst_intrvl);
	if (ret)
		xring_pcie->rst_intrvl = RESET_INTERVAL_MS;

	ret = of_reserved_mem_device_init(&pdev->dev);
	if (ret)
		dev_err(dev, "Failed to parse reserved mem. Alloc 32-bit msi-data may fail\n");

	ret = xring_pcie_parse_clk(xring_pcie);
	if (ret)
		return ret;

	ret = xring_pcie_clk_enable(xring_pcie);
	if (ret)
		return ret;

	ret = xring_pcie_clk_set_rate(xring_pcie);
	if (ret)
		goto err_set_rate;

	ret = xring_pcie_crg_init(pdev);
	if (ret)
		goto err_crg_init;

	xring_pcie->regmap_hss2_top = syscon_regmap_lookup_by_phandle(np,
					"hss2-top-syscon");
	if (IS_ERR(xring_pcie->regmap_hss2_top)) {
		dev_err(dev, "unable to find hss2_top syscon registers\n");
		goto err_crg_init;
	}
	xring_pcie_diffbuf_shutdown(xring_pcie, false);
	xring_pcie_status_update(xring_pcie, true);

	/* enable pcie pll */
	ret = clk_prepare_enable(clk->clk_pcie_pll);
	if (ret) {
		dev_err(dev, "failed to enable clk clk_pcie_pll\n");
		goto err_crg_init;
	}

	ret = xring_pcie_dev_rst_init(xring_pcie);
	if (ret)
		goto err_dev_rst;

	if (!xring_pcie->is_fpga)
		xring_pcie_phy_init(xring_pcie);

	if (of_property_read_bool(np, "refclk-always-on"))
		xring_pcie_clkreq_sw_mux_set(xring_pcie, 1);

	/* unmask L1.2 doze */
	if (of_property_read_bool(np, "l12-doze-unmask"))
		xring_pcie_L12_doze(xring_pcie, 0);

	xring_pcie_config_phy_pg_mode(xring_pcie, true);
	xring_pcie_app_l1sub_cfg(xring_pcie->port_num, true);

	if (!xring_pcie->is_fpga && of_property_read_bool(dev->of_node, "phy_firmware_upd"))
		xring_pcie_sram_boot_mode(xring_pcie);

	ret = xring_pcie_mem_shutdown(pdev);
	if (ret)
		goto err_dev_rst;

	/* delay before phy unreset */
	usleep_range(90, 100);
	ret = xring_pcie_powerup_rst_deassert(pdev);
	if (ret)
		goto err_dev_rst;

	ret = xring_add_pcie_port(xring_pcie, pdev);
	if (ret)
		goto err_add_port;

	if (of_property_read_bool(np, "bypass-smmu"))
		bypass_smmu(pdev);

	xring_pcie_set_qos(xring_pcie);

	ret = xring_pcie_msi_handler(xring_pcie, pdev);
	if (ret < 0)
		dev_err(dev, "Failed to re-clain msi handler\n");

	if (of_property_read_bool(dev->of_node, "create-sysfs")) {
		xring_pcie_create_sysfs(dev);
		xring_pcie_lp_attach_sysfs(dev);
	}

	if (of_property_read_bool(dev->of_node, "rpm-supported"))
		xring_pcie_enable_rpm(xring_pcie);

	return 0;

err_add_port:
	xring_pcie_powerup_rst_assert(pdev);

	if (!of_property_read_bool(dev->of_node, "no-diffbuf-shutdown"))
		xring_pcie_diffbuf_shutdown(xring_pcie, true);

	xring_pcie_sram_load_done(xring_pcie, false);
err_dev_rst:
	clk_disable_unprepare(xring_pcie->clk->clk_pcie_pll);
err_crg_init:
	xring_pcie_hss2_dbi_rst(pdev);
err_set_rate:
	xring_pcie_clk_disable(xring_pcie);

	return ret;
}

static void xring_pcie_sys_int_deinit(struct xring_pcie *xring_pcie)
{
	xring_pcie_mask_sys_int(xring_pcie);
	devm_free_irq(xring_pcie->pci->dev, xring_pcie->sys_intr, xring_pcie);
}

static void xring_pcie_remove_sysfs(struct device *dev)
{
	device_remove_file(dev, &dev_attr_pci_ctrl_register);
	device_remove_file(dev, &dev_attr_enable_sys_int);
	device_remove_file(dev, &dev_attr_aspm_state);
	device_remove_file(dev, &dev_attr_start_link);
	device_remove_file(dev, &dev_attr_pcie_gpio);
	device_remove_file(dev, &dev_attr_set_speed);
	xring_pcie_lp_detach_sysfs(dev);
}

static void xring_pcie_irq_save_restore(struct xring_pcie *xring_pcie, bool save)
{
	uint32_t num = 0, route_reg_addr = 0;
	uint32_t reg_index = 0;

	if (save) {
		for (route_reg_addr = xring_pcie->sr_data.sr_route_reg_addr[0];
			route_reg_addr <= xring_pcie->sr_data.sr_route_reg_addr[1];
				++num, route_reg_addr += 4)
			xring_pcie->sr_data.sr_route_data[num] = readl(xring_pcie->pci_ctrl + route_reg_addr);
		for (reg_index = 0; reg_index < xring_pcie->sr_data.sr_reg_list_len; reg_index++)
			xring_pcie->sr_data.sr_restore_data[reg_index] = readl(xring_pcie->pci->dbi_base +
						xring_pcie->sr_data.sr_reg_list[reg_index]);
	} else {
		for (route_reg_addr = xring_pcie->sr_data.sr_route_reg_addr[0];
			route_reg_addr <= xring_pcie->sr_data.sr_route_reg_addr[1]; ++num, route_reg_addr += 4)
			writel(xring_pcie->sr_data.sr_route_data[num], xring_pcie->pci_ctrl + route_reg_addr);
		for (reg_index = 0; reg_index < xring_pcie->sr_data.sr_reg_list_len; reg_index++)
			writel(xring_pcie->sr_data.sr_restore_data[reg_index], (xring_pcie->pci->dbi_base +
			xring_pcie->sr_data.sr_reg_list[reg_index]));
	}
}

static void xring_pcie_linkup_retry(struct platform_device *pdev,
				struct xring_pcie *xring_pcie, struct device *dev)
{
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	struct pcie_port *pp;
#else
	struct dw_pcie_rp *pp;
#endif

	pp = &xring_pcie->pci->pp;
	xring_pcie_dev_rst_assert(xring_pcie);
	mdelay(10);
	xring_pcie_powerup_rst_assert(pdev);

	xring_pcie_rc_rst_assert(xring_pcie);
	xring_pcie_sram_load_done(xring_pcie, false);
	xring_pcie->pci->ops->stop_link(xring_pcie->pci);

	if (!xring_pcie->is_fpga)
		xring_pcie_phy_init(xring_pcie);

	xring_pcie_config_phy_pg_mode(xring_pcie, true);
	xring_pcie_mem_shutdown(pdev);
	xring_pcie_dev_rst_deassert(xring_pcie);
	xring_pcie_powerup_rst_deassert(pdev);

	if (!xring_pcie->is_fpga && of_property_read_bool(dev->of_node, "phy_firmware_upd"))
		xring_pcie_phy_firmware_upd(xring_pcie);

	xring_pcie_rc_init(pp);
	dw_pcie_setup_rc(pp);
	if (xring_pcie_start_link(xring_pcie->pci) == 0)
		dev_info(dev, "link up");
	else
		dev_info(dev, "link up failed");
}

static int xring_pcie_suspend_noirq(struct device *dev)
{
	struct platform_device *pdev;
	struct xring_pcie *xring_pcie;
	struct dw_pcie *pci;

	int ret = 0;

	pdev = container_of(dev, struct platform_device, dev);
	xring_pcie = platform_get_drvdata(pdev);
	pci = xring_pcie->pci;

	dev_info(pci->dev, "%s: xring pcie suspend enter\n", __func__);
	xring_pcie->insr = true;
	/* PCIe route address:
	 * route_reg_addr[0] is the starting address of the route data register
	 * route_reg_addr[1] is the end address of the route data register
	 */
	xring_pcie_save_l1ss_ctrl(xring_pcie);

	ret = xring_pcie_config_l2(xring_pcie);
	if (ret) {
		dev_info(pci->dev, "cannot enter L2 state\n");
		/* need reset pcie and try linkup again*/
		xring_pcie_linkup_retry(pdev, xring_pcie, dev);
		xring_pcie_restore_l1ss_ctrl(xring_pcie);
		xring_pcie->insr = false;
		return ret;
	}

	xring_pcie_dev_rst_assert(xring_pcie);
	xring_pcie_irq_save_restore(xring_pcie, true);

	if (of_property_read_bool(dev->of_node, "l2-diffbuf-shutdown"))
		xring_pcie_diffbuf_shutdown(xring_pcie, true);

	xring_pcie_hss2_dbi_rst(pdev);
	xring_pcie_clk_disable(xring_pcie);
	clk_disable_unprepare(xring_pcie->clk->clk_pcie_pll);

	dev_info(pci->dev, "%s: xring pcie suspend success\n", __func__);
	return 0;
}

static int xring_pcie_resume_noirq(struct device *dev)
{
	struct platform_device *pdev;
	struct xring_pcie *xring_pcie;
	struct dw_pcie *pci;
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	struct pcie_port *pp;
#else
	struct dw_pcie_rp *pp;
#endif

	struct device_node *np = dev->of_node;
	int ret = 0;

	pdev = container_of(dev, struct platform_device, dev);
	xring_pcie = platform_get_drvdata(pdev);
	dev = &pdev->dev;
	pci = xring_pcie->pci;
	pp = &pci->pp;

	dev_info(pci->dev, "%s: xring pcie resume enter\n", __func__);
	xring_pcie_powerup_rst_assert(pdev);
	ret = xring_pcie_clk_enable(xring_pcie);
	if (ret)
		goto err_resume;

	ret = xring_pcie_crg_init(pdev);
	if (ret)
		goto err_resume;

	if (of_property_read_bool(dev->of_node, "l2-diffbuf-shutdown"))
		xring_pcie_diffbuf_shutdown(xring_pcie, false);

	ret = clk_prepare_enable(xring_pcie->clk->clk_pcie_pll);
	if (ret) {
		dev_err(dev, "failed to enable clk clk_pcie_pll\n");
		goto err_resume;
	}

	//hurry up pull up ep's perst because we need stable clk req.
	xring_pcie_dev_rst_deassert(xring_pcie);

	/* rc exit from L2 if in ap sr */
	xring_pcie_rc_rst_assert(xring_pcie);
	xring_pcie_sram_load_done(xring_pcie, false);
	pci->ops->stop_link(pci);

	if (!xring_pcie->is_fpga)
		xring_pcie_phy_init(xring_pcie);

	xring_pcie_clkreq_sw_mux_set(xring_pcie, 1);

	xring_pcie_config_phy_pg_mode(xring_pcie, true);

	if (!xring_pcie->is_fpga && of_property_read_bool(dev->of_node, "phy_firmware_upd"))
		xring_pcie_sram_boot_mode(xring_pcie);

	ret = xring_pcie_mem_shutdown(pdev);
	if (ret)
		goto err_resume;

	/* delay before phy unreset */
	usleep_range(90, 100);
	ret = xring_pcie_powerup_rst_deassert(pdev);
	if (ret)
		goto err_resume;

	/* PCIe Phy Firmware update */
	if (!xring_pcie->is_fpga && of_property_read_bool(dev->of_node, "phy_firmware_upd")) {
		ret = xring_pcie_phy_firmware_upd(xring_pcie);
		if (ret)
			goto err_resume;
	}

	xring_pcie_rc_init(pp);
	dw_pcie_setup_rc(pp);

	if (xring_pcie_start_link(pci) == 0)
		dev_info(dev, "link up");

	/* PCI_CTRL register,  restore route data
	 * PCIe route address:
	 * route_reg_addr[0] is the starting address of the route data register
	 * route_reg_addr[1] is the end address of the route data register
	 */
	xring_pcie_irq_save_restore(xring_pcie, false);

	if (of_property_read_bool(np, "bypass-smmu"))
		bypass_smmu(pdev);

	xring_pcie_restore_l1ss_ctrl(xring_pcie);

	if (of_property_read_bool(np, "l12-doze-unmask"))
		xring_pcie_L12_doze(xring_pcie, 0);
	xring_pcie_hss2_top_doze_unmask(xring_pcie, true);

err_resume:
	xring_pcie->insr = false;
	dev_info(pci->dev, "%s: xring pcie resume success\n", __func__);

	return ret;
}

static int xring_pcie_resume_early(struct device *dev)
{
	struct platform_device *pdev;
	struct xring_pcie *xring_pcie;

	pdev = container_of(dev, struct platform_device, dev);
	xring_pcie = platform_get_drvdata(pdev);

	dev_info(dev, "%s: xring pcie resume early enter\n", __func__);
	xring_pcie_clkreq_sw_mux_set(xring_pcie, 0);
	return 0;
}

static int xring_pcie_runtime_suspend(struct device *dev)
{
	struct platform_device *pdev;
	struct xring_pcie *xring_pcie;

	pdev = container_of(dev, struct platform_device, dev);
	xring_pcie = platform_get_drvdata(pdev);

	dev_info(dev, "runtime suspend enter.\n");

	xring_pcie_sys_int_deinit(xring_pcie);

	return xring_pcie_suspend_noirq(dev);
}

static int xring_pcie_runtime_resume(struct device *dev)
{
	struct platform_device *pdev;
	struct xring_pcie *xring_pcie;
	int ret;

	dev_info(dev, "runtime resume enter.\n");

	pdev = container_of(dev, struct platform_device, dev);
	xring_pcie = platform_get_drvdata(pdev);

	xring_pcie_resume_noirq(dev);

	ret = devm_request_irq(dev, xring_pcie->sys_intr, xring_pcie_sys_irq_handler,
			       IRQF_SHARED, "xring-pcie-sys", xring_pcie);
	if (ret) {
		dev_err(dev, "failed to request sys irq\n");
		return ret;
	}
	xring_pcie_resume_early(dev);

	return 0;
}

static const struct dev_pm_ops xring_pcie_pm_ops = {
	SET_NOIRQ_SYSTEM_SLEEP_PM_OPS(xring_pcie_suspend_noirq,
				      xring_pcie_resume_noirq)
	SET_LATE_SYSTEM_SLEEP_PM_OPS(NULL, xring_pcie_resume_early)
	SET_RUNTIME_PM_OPS(xring_pcie_runtime_suspend, xring_pcie_runtime_resume, NULL)
};

static int xring_pcie_remove(struct platform_device *pdev)
{
	struct xring_pcie *xring_pcie = platform_get_drvdata(pdev);
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	struct pcie_port *pp = &xring_pcie->pci->pp;
#else
	struct dw_pcie_rp *pp = &xring_pcie->pci->pp;
#endif

	struct device *dev = &pdev->dev;

	if (of_property_read_bool(dev->of_node, "rpm-supported"))
		xring_pcie_disable_rpm(xring_pcie);

	xring_pcie_remove_sysfs(dev);
	xring_pcie_sys_int_deinit(xring_pcie);
	dw_pcie_host_deinit(pp);

	if (!of_property_read_bool(dev->of_node, "no-diffbuf-shutdown"))
		xring_pcie_diffbuf_shutdown(xring_pcie, true);

	xring_pcie_powerup_rst_assert(pdev);
	xring_pcie_status_update(xring_pcie, false);
	xring_pcie_hss2_top_doze_unmask(xring_pcie, true);
	xring_pcie_sram_load_done(xring_pcie, false);
	xring_pcie_hss2_dbi_rst(pdev);
	xring_pcie_clk_disable(xring_pcie);
	clk_disable_unprepare(xring_pcie->clk->clk_pcie_pll);

	return 0;
}

static struct platform_device *xring_pcie_find_pdev_by_port(int port)
{
	struct platform_device *pdev = NULL;

	if (pdev_list[port] && (port < XRING_PCIE_MAX_PORT))
		pdev = pdev_list[port];

	return pdev;
}

static struct platform_device *xring_pcie_find_pdev_by_name(char *name)
{
	struct device_node *np;
	const char *ep_dev_type;

	np = of_find_compatible_node(NULL, NULL, "xring,dw-pcie0");
	if (!np) {
		pr_err("xring-pcie: cannot found xring,dw-pcie0 node!\n");
		return NULL;
	}
	if (!of_property_read_string(np, "ep-device-type", &ep_dev_type)) {
		pr_err("xring-pcie: ep-device-type0 failed");
		return NULL;
	}

	if (!strcmp(ep_dev_type, name))
		return of_find_device_by_node(np);

	np = of_find_compatible_node(NULL, NULL, "xring,dw-pcie1");
	if (!np) {
		pr_err("xring-pcie: cannot found xring,dw-pcie1 node!\n");
		return NULL;
	}
	if (!of_property_read_string(np, "ep-device-type", &ep_dev_type)) {
		pr_err("xring-pcie: cannot found ep-device-type1 node!\n");
		return NULL;
	}

	if (!strcmp(ep_dev_type, name))
		return of_find_device_by_node(np);

	pr_err("xring-pcie: pcie platform device not found!\n");

	return NULL;
}

static int pcie_wait_for_link_retrain(struct pci_dev *pdev)
{
	u16 lnksta, lnkctl2;
	unsigned long end_jiffies;

	end_jiffies = jiffies + msecs_to_jiffies(PCIE_LINK_RETRAIN_TIMEOUT_MS);
	do {
		pcie_capability_read_word(pdev, PCI_EXP_LNKCTL2, &lnkctl2);
		pcie_capability_read_word(pdev, PCI_EXP_LNKSTA, &lnksta);
		if ((lnkctl2 & PCI_EXP_LNKCTL2_TLS) == (lnksta & PCI_EXP_LNKSTA_CLS))
			return 0;
		usleep_range(900, 1000);
	} while (time_before(jiffies, end_jiffies));

	return -ETIMEDOUT;
}

int xring_pcie_retrain_link(struct pci_dev *pdev, int speed)
{
	int ret;
	u16 linkctl2;

	/* Make sure that there is no ongoing link retrain */
	ret = pcie_wait_for_link_retrain(pdev);
	if (ret)
		return ret;

	pcie_capability_read_word(pdev, PCI_EXP_LNKCTL2, &linkctl2);
	linkctl2 &= ~PCI_EXP_LNKCTL2_TLS;
	linkctl2 |= speed;
	pcie_capability_write_word(pdev, PCI_EXP_LNKCTL2, linkctl2);

	pcie_capability_set_word(pdev, PCI_EXP_LNKCTL, PCI_EXP_LNKCTL_RL);

	return pcie_wait_for_link_retrain(pdev);
}

int xring_pcie_set_speed(struct pci_dev *dev, int speed)
{
	struct pci_dev *parent;
	u16 linksta, linkctl2;
	u32 plinkcap, linkcap;
	struct dw_pcie_rp *pp;
	struct dw_pcie *pci;
	struct xring_pcie *xring_pcie;

	pp = dev->bus->sysdata;
	pci = to_dw_pcie_from_pp(pp);
	xring_pcie = to_xring_pcie(pci);
	if (xring_pcie->insr) {
		dev_err(pci->dev, "pcie in suspend, can't set speed\n");
		return -ENODEV;
	}

	parent = dev->bus->self;

	pcie_capability_read_dword(parent, PCI_EXP_LNKCAP, &plinkcap);
	pcie_capability_read_dword(dev, PCI_EXP_LNKCAP, &linkcap);

	if ((speed < PCI_EXP_LNKCTL2_TLS_2_5GT) ||
		(speed > (plinkcap & PCI_EXP_LNKCAP_SLS)) ||
		(speed > (linkcap & PCI_EXP_LNKCAP_SLS))) {
		dev_err(&parent->dev, "Invalid target speed:%d; parent SLS:%d; dev SLS:%d\n",
			speed, (plinkcap & PCI_EXP_LNKCAP_SLS), (linkcap & PCI_EXP_LNKCAP_SLS));
		return -EINVAL;
	}

	pcie_capability_read_word(parent, PCI_EXP_LNKSTA, &linksta);
	if (speed == (linksta & PCI_EXP_LNKSTA_CLS)) {
		dev_info(&parent->dev, "Current link speed is already %d\n", (linksta & PCI_EXP_LNKSTA_CLS));
		return 0;
	}
	dev_info(&parent->dev, "Current link speed before gen switch:%d", (linksta & PCI_EXP_LNKSTA_CLS));

	if (xring_pcie_retrain_link(parent, speed)) {
		dev_err(&parent->dev, "failed to retrain link\n");
		pcie_capability_read_word(parent, PCI_EXP_LNKCTL2, &linkctl2);
		linkctl2 &= ~PCI_EXP_LNKCTL2_TLS;
		linkctl2 |= (linksta & PCI_EXP_LNKSTA_CLS);
		pcie_capability_write_word(parent, PCI_EXP_LNKCTL2, linkctl2);
		return -1;
	}

	dev_info(&parent->dev, "Succeed to set speed as:%d", speed);

	return 0;
}
EXPORT_SYMBOL(xring_pcie_set_speed);

int xring_pcie_probe_port_by_port(int port)
{
	struct platform_device *pdev;

	pdev = xring_pcie_find_pdev_by_port(port);
	if (!pdev) {
		pr_err("xring-pcie: pcie platform %d device not found!\n", port);
		return -ENODEV;
	}

	dev_info(&pdev->dev, "%s:enter pcie port %d probe port\n", __func__, port);

	if (device_attach(&pdev->dev) < 0) {
		device_release_driver(&pdev->dev);
		dev_err(&pdev->dev, "%s: pcie probe fail!\n", __func__);
		return -ENODEV;
	}

	return 0;
}
EXPORT_SYMBOL(xring_pcie_probe_port_by_port);

int xring_pcie_probe_port_by_name(char *name)
{
	struct platform_device *pdev;

	pdev = xring_pcie_find_pdev_by_name(name);
	if (!pdev) {
		pr_err("xring-pcie: pcie platform %s device not found!\n", name);
		return -ENODEV;
	}

	dev_info(&pdev->dev, "%s:enter pcie port %s probe port\n", __func__, name);

	if (device_attach(&pdev->dev) < 0) {
		device_release_driver(&pdev->dev);
		dev_err(&pdev->dev, "%s: pcie probe fail!\n", __func__);
		return -ENODEV;
	}

	return 0;
}
EXPORT_SYMBOL(xring_pcie_probe_port_by_name);

int xring_pcie_remove_port_by_name(char *name)
{
	struct platform_device *pdev;

	pdev = xring_pcie_find_pdev_by_name(name);
	if (!pdev) {
		pr_err("xring-pcie: pcie platform %s device not found!\n", name);
		return -ENODEV;
	}

	dev_info(&pdev->dev, "%s:enter pcie port %s remove port\n", __func__, name);

	device_release_driver(&pdev->dev);

	return 0;
}
EXPORT_SYMBOL(xring_pcie_remove_port_by_name);

int xring_pcie_remove_port_by_port(int port)
{
	struct platform_device *pdev;

	pdev = xring_pcie_find_pdev_by_port(port);
	if (!pdev) {
		pr_err("xring-pcie: pcie platform %d device not found!\n", port);
		return -ENODEV;
	}

	dev_info(&pdev->dev, "%s:enter pcie port %d remove port\n", __func__, port);

	device_release_driver(&pdev->dev);

	return 0;
}
EXPORT_SYMBOL(xring_pcie_remove_port_by_port);

void xring_pcie_pm_runtime_allow(int port, bool en)
{
	struct platform_device *pdev;
	struct xring_pcie *xring_pcie;
	struct pci_host_bridge *bridge;

	pdev = xring_pcie_find_pdev_by_port(port);
	if (!pdev) {
		pr_err("xring-pcie: %s:pcie platform %d device not found!\n", __func__, port);
		return;
	}

	xring_pcie = platform_get_drvdata(pdev);
	if (!xring_pcie) {
		dev_err(&pdev->dev, "PCIe port not found!\n");
		return;
	}
	dev_info(&pdev->dev, "PCIe port %d set rpm state as %d\n", port, en);

	bridge = xring_pcie->pci->pp.bridge;

	if (en)
		pm_runtime_allow(&bridge->dev);
	else
		pm_runtime_forbid(&bridge->dev);
}
EXPORT_SYMBOL(xring_pcie_pm_runtime_allow);

int xring_pcie_soft_on(struct pci_bus *bus)
{
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	struct pcie_port *pp;
#else
	struct dw_pcie_rp *pp;
#endif
	struct dw_pcie *pci;
	struct xring_pcie *xring_pcie;
	struct pci_dev *rc_dev;

	if (!bus) {
		pr_info("xring-pcie: There is no bus, please check the host driver\n");
		return -ENODEV;
	}

	pp = bus->sysdata;
	pci = to_dw_pcie_from_pp(pp);
	xring_pcie = to_xring_pcie(pci);

	dev_info(pci->dev, "enter xring pcie soft on...\n");

	if (!xring_pcie->soft_off) {
		dev_info(pci->dev, "The soft_off is false, can't soft on\n");
		return -EPERM;
	}

/*
 *	soft_off using hot_reset to perform linkdown process,
 *	comment these for furthur reference.
 *
	if (!xring_pcie->is_fpga)
		xring_pcie_phy_init(xring_pcie);

	xring_pcie_rc_init(pp);
	dw_pcie_setup_rc(pp);
*/

	xring_pcie_hot_reset(xring_pcie, 0);
	if (xring_pcie_wait_for_link(pci) == 0)
		dev_info(pci->dev, "link up");
	else
		return -ETIMEDOUT;

	xring_pcie->soft_off = false;
	list_for_each_entry(rc_dev, &bus->devices, bus_list) {
		pci_restore_state(rc_dev);
	}

	xring_pcie_restore_l1ss_ctrl(xring_pcie);
	xring_pcie_restore_msi(xring_pcie);

/*
 *	if (!xring_pcie->is_fpga)
 *		//Trigger link exit from L2 state
 */

	dev_info(pci->dev, "xring pcie soft on done\n");

	return 0;
}
EXPORT_SYMBOL(xring_pcie_soft_on);

int xring_pcie_soft_off(struct pci_bus *bus)
{
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	struct pcie_port *pp;
#else
	struct dw_pcie_rp *pp;
#endif
	struct dw_pcie *pci;
	struct xring_pcie *xring_pcie;
	struct pci_dev *rc_dev;

	if (!bus) {
		pr_info("xring-pcie: There is no bus, please check the host driver\n");
		return -ENODEV;
	}

	pp = bus->sysdata;
	pci = to_dw_pcie_from_pp(pp);
	xring_pcie = to_xring_pcie(pci);

	dev_info(pci->dev, "enter xring pcie soft off...\n");

	if (xring_pcie->soft_off) {
		dev_info(pci->dev, "The soft_off is true, can't soft off\n");
		return -EPERM;
	}

	if (!xring_pcie_link_up(xring_pcie->pci)) {
		dev_info(pci->dev, "Link is down, can't soft off.\n");
		return -EPERM;
	}

	list_for_each_entry(rc_dev, &bus->devices, bus_list) {
		pci_save_state(rc_dev);
	}

	xring_pcie_save_l1ss_ctrl(xring_pcie);
	xring_pcie_save_msi(xring_pcie);

/*
 *	if (!xring_pcie->is_fpga)
 *		//Trigger link to L2 state
 */

	xring_pcie->soft_off = true;
	xring_pcie_hot_reset(xring_pcie, 1);

	dev_info(pci->dev, "xring pcie soft off done\n");

	return 0;
}
EXPORT_SYMBOL(xring_pcie_soft_off);

int xring_pcie_app_l1sub_cfg(int port, bool enable)
{
	struct platform_device *pdev;
	struct xring_pcie *xring_pcie;
	u32 val;

	pdev = xring_pcie_find_pdev_by_port(port);
	if (!pdev) {
		pr_err("xring-pcie: %s:pcie platform %d device not found!\n", __func__, port);
		return -ENODEV;
	}

	xring_pcie = platform_get_drvdata(pdev);
	if (!xring_pcie) {
		dev_err(&pdev->dev, "PCIe port not found!\n");
		return -ENODEV;
	}

	val = readl(xring_pcie->pci_ctrl + PCIE_CTRL_SII_POWER_MANAGEMENT_0);
	if (enable)
		val &= ~(1 << PCIE_CTRL_SII_POWER_MANAGEMENT_0_APP_XFER_PENDING_SHIFT);
	else
		val |= 1 << PCIE_CTRL_SII_POWER_MANAGEMENT_0_APP_XFER_PENDING_SHIFT;

	writel(val, xring_pcie->pci_ctrl + PCIE_CTRL_SII_POWER_MANAGEMENT_0);

	return 0;
}
EXPORT_SYMBOL(xring_pcie_app_l1sub_cfg);

/**
 * xring_pcie_disable_data_trans - Block pcie
 * and do not accept any data packet transmission.
 * @port: The port number which EP use
 */
int xring_pcie_disable_data_trans(int port)
{
	struct platform_device *pdev;
	struct xring_pcie *xring_pcie;
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	struct pcie_port *pp;
#else
	struct dw_pcie_rp *pp;
#endif
	struct pci_dev *rc_dev;
	struct pci_bus *bus;

	pdev = xring_pcie_find_pdev_by_port(port);
	if (!pdev) {
		pr_err("xring-pcie: %s:pcie platform %d device not found!\n", __func__, port);
		return -ENODEV;
	}

	dev_info(&pdev->dev, "port %d enter %s...\n", port, __func__);

	xring_pcie = platform_get_drvdata(pdev);
	if (!xring_pcie) {
		dev_err(&pdev->dev, "PCIe port not found!\n");
		return -ENODEV;
	}

	if (!xring_pcie_link_up(xring_pcie->pci)) {
		dev_info(xring_pcie->pci->dev,
			"Already disable data trans, link is down.\n");
		return 0;
	}

	pp = &xring_pcie->pci->pp;
	bus = pp->bridge->bus;

	list_for_each_entry(rc_dev, &bus->devices, bus_list) {
		pci_save_state(rc_dev);
	}

	xring_pcie_save_l1ss_ctrl(xring_pcie);
	xring_pcie_save_msi(xring_pcie);

	/* To avoid soft_off after disable_date_trans */
	xring_pcie->soft_off = true;
	xring_pcie_hot_reset(xring_pcie, 1);

	dev_info(xring_pcie->pci->dev,
			"PCIe port %d reset.\n", port);

	return 0;
}
EXPORT_SYMBOL(xring_pcie_disable_data_trans);

int xring_pcie_config_l2(struct xring_pcie *xring_pcie)
{
	u16 pmc;
	u32 reg;
	u32 sii_power_mange2;
	u32 smlh_ltssm_state;
	int pm;
	int timeout = 1000;
	int ret = 0;

	if (xring_pcie->is_fpga)
		return -1;

	pm = dw_pcie_find_capability(xring_pcie->pci, PCI_CAP_ID_PM);
	pmc = dw_pcie_readw_dbi(xring_pcie->pci, pm + PCI_PM_CTRL);
	pmc |= PCI_PM_CTRL_PME_ENABLE | PCI_D3hot;
	dw_pcie_writew_dbi(xring_pcie->pci, pm + PCI_PM_CTRL, pmc);

	reg = readl(xring_pcie->pci_ctrl + PCIE_CTRL_L2_ENTRY_EN);
	reg |= BIT(PCIE_CTRL_L2_ENTRY_EN_APPS_PM_XMT_TURNOFF_SHIFT);
	writel(reg, xring_pcie->pci_ctrl + PCIE_CTRL_L2_ENTRY_EN);
	while (timeout--) {
		sii_power_mange2 = readl(xring_pcie->pci_ctrl + PCIE_CTRL_SII_POWER_MANAGEMENT_2);
		smlh_ltssm_state = readl(xring_pcie->pci_ctrl + PCIE_CTRL_SMLH_LTSSM_STATE);
		if ((sii_power_mange2 & PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_LINKST_IN_L2_MASK)
			== PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_LINKST_IN_L2_MASK) {
			dev_info(xring_pcie->pci->dev, "pm_linkst_in_l2");
			goto enter_L2;
		}
		if (((sii_power_mange2 >> PCIE_CTRL_SII_POWER_MANAGEMENT_2_PM_MASTER_STATE_SHIFT) &
			L23RDY) == L23RDY) {
			dev_info(xring_pcie->pci->dev, "L23RDY");
			goto enter_L2;
		}
		if ((smlh_ltssm_state & L2_IDLE) == L2_IDLE) {
			dev_info(xring_pcie->pci->dev, "L2_IDLE");
			goto enter_L2;
		}
		udelay(10);
	}
	dev_err(xring_pcie->pci->dev, "failed to enter L2 state");
	reg = readl(xring_pcie->pci_ctrl + PCIE_CTRL_L2_ENTRY_EN);
	reg &= ~BIT(PCIE_CTRL_L2_ENTRY_EN_APPS_PM_XMT_TURNOFF_SHIFT);
	writel(reg, xring_pcie->pci_ctrl + PCIE_CTRL_L2_ENTRY_EN);

	pm = dw_pcie_find_capability(xring_pcie->pci, PCI_CAP_ID_PM);
	pmc = dw_pcie_readw_dbi(xring_pcie->pci, pm + PCI_PM_CTRL);
	pmc &= ~(PCI_PM_CTRL_PME_ENABLE | PCI_D3hot);
	dw_pcie_writew_dbi(xring_pcie->pci, pm + PCI_PM_CTRL, pmc);

	return -1;
enter_L2:
	dev_info(xring_pcie->pci->dev, "Enter L2 state successfully");
	return ret;
}


static const struct of_device_id xring_pcie_of_match[] = {
	{ .compatible = "xring,dw-pcie0" },
	{ .compatible = "xring,dw-pcie1" },
	{},
};

static struct platform_driver xring_pcie_driver = {
	.driver = {
		.name	= "xring-pcie",
		.of_match_table = xring_pcie_of_match,
		.pm	= &xring_pcie_pm_ops,
	},
	.probe = xring_pcie_probe,
	.remove = xring_pcie_remove,
};
module_platform_driver(xring_pcie_driver);
MODULE_LICENSE("GPL v2");
MODULE_DEVICE_TABLE(of, xring_pcie_of_match);
