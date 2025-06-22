/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/clk.h>

#include <dt-bindings/xring/platform-specific/hss2_crg.h>
#include <dt-bindings/xring/platform-specific/pctrl.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include "pcie-designware.h"

#include <soc/xring/xring_smmu_wrapper.h>

#define to_xring_pcie(x)	dev_get_drvdata((x)->dev)

#define DFT_AUX_CLK_RATE	19670000
#define DFT_FW_CLK_RATE		208896000
#define MHZ_TO_HZ(freq)		((freq) * 1000 * 1000)

#define PHY_WAIT_MAX_RETRIES	1000
#define PHY_WAIT_USLEEP_MIN	900
#define PHY_WAIT_USLEEP_MAX	1000

#define PCIE_PCTRL_BASE		ACPU_PCTRL
#define PCIE_PCTRL_SIZE		0x2000

/* Synopsys-specific PCIe configuration registers */
#define PF0_PORT_LOGIC				0x700
#define LANE_SKEW_OFF				(PF0_PORT_LOGIC + 0x14)
#define  ELASTIC_BUFFER_MODE			BIT(26)
#define COHERENCY_CONTROL_3_OFF			(PF0_PORT_LOGIC + 0x1e8)
#define  CFG_MSTER_AWCACHE_MODE_MASK		GENMASK(14, 11)
#define AUX_CLK_FREQ_OFF			(PF0_PORT_LOGIC + 0x440)
#define  AUX_CLK_FREQ_MASK			GENMASK(9, 0)
#define  AUX_CLK_FREQ				20
#define AMBA_ERROR_RESPONSE_DEFAULT_OFF		(PF0_PORT_LOGIC + 0x1d0)
#define  AMBA_ERROR_RESPONSE_CRS_BIT		3
#define  AMBA_ERROR_RESPONSE_CRS_MASK		GENMASK(4, 3)
#define  AMBA_ERROR_RESPONSE_CRS_OKAY_FFFF0001	2

/* aspm */
#define ASPM_CAP_L1_SS_PCIPM		(PCI_L1SS_CAP_PCIPM_L1_2 | \
					PCI_L1SS_CAP_PCIPM_L1_1 |\
					PCI_L1SS_CAP_L1_PM_SS)
#define ASPM_CAP_L1_SS			(PCI_L1SS_CAP_ASPM_L1_2 | PCI_L1SS_CAP_ASPM_L1_1)

/* Xring PCIe ctrl register */
#define PCI_CTRL_REG_SIZE		(0x1040)
#define  OFFSET_04			(4)
#define  OFFSET_08			(8)
#define  OFFSET_12			(12)
#define  OFFSET_16			(16)

#define SYS_INT_PER_REG			32
#define SYS_INT_SET_NUM			4
#define SYS_INT_BLOCK_SIZE		0x10
#define MSI_ROUTE_BLOCK_SIZE		0x4

/* smmu */
#define SC_SMMU_TCU_QREQN_CG_SET		0x10100
#define SC_SMMU_TCU_QREQN_PD			(1 << 0)
#define SC_SMMU_TCU_QACCEPTN_CG_ST		(1 << 7)
#define SC_SMMU_TCU_QACCEPTN_PD			(1 << 2)

/* CRG */
#define HSS2_CRG_CLKGT0_PCIE		(HSS2_CRG_CLKGT0_RO_GT_CLK_PCIE_PLL_LOGIC_MASK)
#define HSS2_CRG_CLKGT0_PCIE0		(HSS2_CRG_CLKGT0_RO_GT_SW_CLK_PCIE0_FW_MASK)
#define HSS2_CRG_CLKGT0_PCIE1		(HSS2_CRG_CLKGT0_RO_GT_SW_CLK_PCIE1_FW_MASK)
#define HSS2_CRG_RST0_PCIE		(HSS2_CRG_RST0_RO_IP_RST_PLL_LOGIC_N_MASK |\
					HSS2_CRG_RST0_RO_IP_RST_SSMOD_N_MASK |\
					HSS2_CRG_RST0_RO_IP_ARST_HSS2_TCU_N_MASK |\
					HSS2_CRG_RST0_RO_IP_RST_HSS2_BUS_DATA_N_MASK |\
					HSS2_CRG_RST0_RO_IP_RST_TCU_TPC_N_MASK)
#define HSS2_CRG_RST0_PCIE0		(HSS2_CRG_RST0_RO_IP_PRST_PCIE0_DBI_N_MASK |\
					HSS2_CRG_RST0_RO_IP_RST_PCIE0_AUX_N_MASK |\
					HSS2_CRG_RST0_RO_IP_PRST_PCIE0_N_MASK |\
					HSS2_CRG_RST0_RO_IP_ARST_HSS2_TBU0_N_MASK |\
					HSS2_CRG_RST0_RO_IP_RST_PCIE0_TPC_N_MASK)
#define HSS2_CRG_RST0_PCIE1		(HSS2_CRG_RST0_RO_IP_PRST_PCIE1_DBI_N_MASK |\
					HSS2_CRG_RST0_RO_IP_RST_PCIE1_AUX_N_MASK |\
					HSS2_CRG_RST0_RO_IP_PRST_PCIE1_N_MASK |\
					HSS2_CRG_RST0_RO_IP_ARST_HSS2_TBU1_N_MASK |\
					HSS2_CRG_RST0_RO_IP_RST_PCIE1_TPC_N_MASK)

#define RESET_INTERVAL_MS		100

#define XRING_PCIE_MAX_PORT		2

/* Print PCIe register.
 * Print 4 registers in one line,index0,4,8, and c
 */
#define PCI_CTRL_REG_INDEX_0		(0x0)
#define PCI_CTRL_REG_INDEX_4		(0x4)
#define PCI_CTRL_REG_INDEX_8		(0x8)
#define PCI_CTRL_REG_INDEX_C		(0xc)

/* for L2 */
#define L23RDY 0x14
#define L2_IDLE 0x15
#define PCI_PM_CAP_PME_D3 0x4000

#define LLDO1_PWR_REG 0x245b
#define FID_SPMI_BURST_READ 0xc3001300

#define XRING_LINK_WAIT_MAX_RETRIES 100000
#define XRING_LINK_WAIT_USLEEP_MIN 10
#define XRING_LINK_WAIT_USLEEP_MAX 20

enum sys_int_reg_count {
	SYS_INT_COUNTER0,
	SYS_INT_COUNTER1,
	SYS_INT_COUNTER2,
	SYS_INT_COUNTER3,
};

struct pcie_sr_data {
	/* Save the route data register of PCIe suspend and resume   */
	uint32_t		sr_route_data[100];
	uint32_t		sr_restore_data[100];
	uint32_t		sr_route_reg_addr[2];
	uint32_t		sr_route_reg_len;
	/* Save the PCIe dbi data register of PCIe suspend and resume   */
	uint32_t		sr_reg_list[100];
	uint32_t		sr_reg_list_len;
};

struct xring_pcie_clk {
	struct clk	*pclk_hss2_tcu_slv;
	struct clk	*aclk_hss2_tcu;
	struct clk	*clk_tcu_tpc;
	struct clk	*clk_pcie_pll;
	struct clk	*pclk_pcie_apb;
	struct clk	*aclk_pcie_axi_slv;
	struct clk	*aclk_hss2_tbu;
	struct clk	*aclk_pcie_axi_mst;
	struct clk	*clk_pcie_aux;
	struct clk	*clk_pcie_tpc;
	struct clk	*clk_pcie_fw;
	u32		aux_clk_rate;
	u32		fw_clk_rate;
};

struct xring_pcie {
	struct dw_pcie		*pci;
	void __iomem		*pci_ctrl;
	void __iomem		*pcie_phy;
	void __iomem		*pcie_phy_apb0;
	int			perstn_gpio;
	int			ep_pmic_en_gpio;
	int			port_num;
	int			sys_intr;
	u32			sys_int_mask[SYS_INT_SET_NUM];
	u32			rst_intrvl;
	u32			device_vendor_id;
	struct pcie_sr_data	sr_data;
	struct regmap		*regmap_hss2_crg;
	struct regmap           *regmap_hss2_top;
	bool			is_fpga;
	/* soft_off flag. If soft off is called again before soft on, it will crash. */
	bool			soft_off;
	bool			sysint_mask_flag;
	bool			clk_enable;
	const char		*tbu_name;

	struct xring_pcie_clk	*clk;

	u32	saved_l1ss_ctl1;
	u32	saved_l1ss_ctl2;
	u32	saved_msi_mask[MAX_MSI_CTRLS];
	u32	saved_msi_enable[MAX_MSI_CTRLS];
	bool cfg_saved;
	bool insr;
	spinlock_t cfg_lock;
	atomic_t sys_int_atomic;
};

struct pcie_info {
	unsigned int modem:1;
	unsigned int wifi:1;
};

void xring_pcie_config_phy_pg_mode(struct xring_pcie *xring_pcie, bool enable);
void xring_pcie_lp_attach_sysfs(struct device *dev);
void xring_pcie_lp_detach_sysfs(struct device *dev);
void xring_pcie_hss2_doze_unmask(struct xring_pcie *xring_pcie, bool unmask);
static struct platform_device *pdev_list[XRING_PCIE_MAX_PORT];
int xring_pcie_config_l2(struct xring_pcie *xring_pcie);
