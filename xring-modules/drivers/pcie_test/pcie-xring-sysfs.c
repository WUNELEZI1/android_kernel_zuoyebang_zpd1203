// SPDX-License-Identifier: GPL-2.0-only
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/of_device.h>
#include <linux/pci.h>
#include <linux/resource.h>
#include <linux/types.h>
#include "pcie-designware.h"
#include <linux/io.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <linux/sizes.h>
#include <linux/of_gpio.h>
#include <dt-bindings/xring/platform-specific/pcie_phy.h>

#include "../pci/pcie-xring-interface.h"
#include "../pci/pcie-xring.h"
#include <dt-bindings/xring/platform-specific/pcie_ctrl.h>
#include <dt-bindings/xring/platform-specific/hss2_top.h>
#include "../../pci.h"
#include "../pci/dwc_c20pcie4_phy_x2_ns_pcs_raw_ref_100m_ext_rom.h"

#define DEVICE_TYPE_OFFSET			0x414
#define EP_RESET_OFFSET				0x314
#define EP_MISC_CTL_1_OFF			0x8bc
#define TARGET_SPEED_OFFSET			0xa0
#define LTSSM_EN_OFFSET				0x18
#define DEV_VEN_ID				0xaaaa16c3
#define BAR0_OFFSET				0x10
#define BAR1_OFFSET				0x14
#define BAR2_OFFSET				0x18
#define BAR3_OFFSET				0x1c
#define BAR4_OFFSET				0x20
#define BAR5_OFFSET				0x24
#define BAR0_MASK_OFFSET			0x40010
#define BAR1_MASK_OFFSET			0x40014
#define BAR2_MASK_OFFSET			0x40018
#define BAR3_MASK_OFFSET			0x4001C
#define BAR5_MASK_OFFSET			0x40024
#define MSI_CAP_REG				0x50
#define MSI_IRQ_NUM_1				0
#define MSI_IRQ_NUM_2				1
#define MSI_IRQ_NUM_4				2
#define MSI_IRQ_NUM_8				3
#define MSI_IRQ_NUM_16				4
#define MSI_IRQ_NUM_32				5
#define MSI_IRQ_NUM_OFFSET			17
#define MEM_BAR_TYPE				0
#define IO_BAR_TYPE				2
#define INBOUND1_OFFSET				0x200
#define INBOUND2_OFFSET				0x400
#define ELBI_OFFSET				0x20000
#define HDMA_TEST_DATA_5A			0x5a5a5a5a
#define HDMA_TEST_DATA_A5			0xa5a5a5a5

/* MSI */
#define  PCI_MSI_FLAGS_64BIT			0x0080	/* 64-bit addresses allowed */
/* atu */
#define PCIE_ATU_UNR_REGION_CTRL1		0x00
#define PCIE_ATU_INCREASE_REGION_SIZE		BIT(13)
#define PCIE_ATU_UNR_REGION_CTRL2		0x04
#define PCIE_ATU_ENABLE				BIT(31)
#define PCIE_ATU_UNR_LOWER_BASE			0x08
#define PCIE_ATU_UNR_UPPER_BASE			0x0C
#define PCIE_ATU_UNR_LOWER_LIMIT		0x10
#define PCIE_ATU_UNR_LOWER_TARGET		0x14
#define PCIE_ATU_UNR_UPPER_TARGET		0x18
#define PCIE_ATU_UNR_UPPER_LIMIT		0x20
#define LINK_WAIT_MAX_IATU_RETRIES		5
#define LINK_WAIT_IATU				9
/* hdma */
#define XRING_HDMA_MODE_W			(1)
#define XRING_HDMA_MODE_R			(2)
#define XRING_HDMA_MODE_WR			(4)
#define HDMA_EN_OFF_WRCH			0x0
#define  HDMA_ENABLE_WRCH			BIT(0)
#define HDMA_DOORBELL_OFF_WRCH			0x4
#define  HDMA_DB_START_WRCH			BIT(0)
#define  HDMA_DB_STOP_WRCH			BIT(1)
#define HDMA_XFERSIZE_OFF_WRCH			0x1c
#define HDMA_SAR_LOW_OFF_WRCH			0x20
#define HDMA_SAR_HIGH_OFF_WRCH			0x24
#define HDMA_DAR_LOW_OFF_WRCH			0x28
#define HDMA_DAR_HIGH_OFF_WRCH			0x2c
#define HDMA_EN_OFF_RDCH			0x0
#define  HDMA_ENABLE_RDCH			BIT(0)
#define HDMA_DOORBELL_OFF_RDCH			0x4
#define  HDMA_DB_START_RDCH			BIT(0)
#define  HDMA_DB_STOP_RDCH			BIT(1)
#define HDMA_XFERSIZE_OFF_RDCH			0x1c
#define HDMA_SAR_LOW_OFF_RDCH			0x20
#define HDMA_SAR_HIGH_OFF_RDCH			0x24
#define HDMA_DAR_LOW_OFF_RDCH			0x28
#define HDMA_DAR_HIGH_OFF_RDCH			0x2c
#define HDMA_INT_SETUP_OFF			0x88
#define HDMA_INT_STATUS_OFF			0x84
#define HDMA_INT_CLEAR_OFF			0x8c
#define  HDMA_INT_STOP				BIT(0)
#define  HDMA_INT_WATERMASK			BIT(1)
#define  HDMA_INT_ABORT				BIT(2)

#define INBOUND_CTRL1_OFFSET			0x00
#define INBOUND_CTRL2_OFFSET			0x04
#define TARGET_ADDR_OFFSET			0x14
#define INBOUND_OFFSET_BASE			0x100
#define PCIE_ATU_FUNC_NUM_MATCH_OFFSET		19
#define ATU_AND_BAR_MODE_ENABLE			3
#define ATU_AND_BAR_MODE_ENABLE_OFFSET		30
#define ATU_EN_OFFSET				31
#define ATU_EN					1
#define LIMIT_SIZE				0xffff
#define ATU1_OUTBOUND_CTRL2			0x204
#define ATU1_OUTBOUND_BASE			0x208
#define ATU1_OUTBOUND_LIMIT			0x210
#define ATU1_OUTBOUND_TARGET			0x214
#define ADDR_TEST_SIZE				0x10
#define DEV_VEN_ID_OFFSET			0x00
#define MSIX_TABLE_SIZE				(16 * 256)
#define HDMA_CH_OFFSET				0x800
#define HDMA_RD_BASE				0x400
#define LANE_ENABLE				0x710
#define NUM_OF_LANES				0x80c
#define LANE_ENABLE_OFFSET			16
#define LANE_ENABLE_MASK			0x3f
#define LANE_SET_MASk				0x1f
#define LANE_OFFSET				8
#define LANE_NUM_NASK				0x1f00
#define LANE_1_ENABLE				0x10120
#define LANE_1_SET				0x301ff
#define DDR_DAR_INIT_VAL			0x0
#define HDMA_TRANSFER_OK			0x0
#define ATU_BAR_OFFSET				8
#define DECIMALISM				10
#define MSI_SLV_OFFSET				0x10000
#define FUNCTION0				0
#define FUNCTION1				1
#define FUNCTION2				2
#define FUNCTION3				3
#define FUNC_NUM_OFSET				5
#define DBI_ACCESS_OFFSET			0x2d8
#define ASPM_STATES_MASK			0x3
#define LINK_CTRL_STATUS_REG			0x80
#define FUNC_MASK				0x7
#define LINK_UP_MASK				1
#define MAX_TIME				10
#define MS_1000					1000
#define TEST_VALUE				0x5a5a5a5a
#define RESET_VALUE				0
#define LINK_STATUS_OFFSET			0x320
#define GEN_MASK				0xf
#define MSIX_CAP_OFFSET				0xb0
#define MSIX_SIZE_MASK				0x7ff
#define MSIX_SIZE_OFFSET			16
#define MSIX_SIZE_256				256
#define BAR0					0
#define BAR1					1
#define BAR2					2
#define BAR3					3
#define FAIL					-1
#define SUCCESS					0

#define PCIE0_PORT				0

#define HDMA_WAIT_MAX_RETRIES			1000
#define PHY_WAIT_USLEEP_MIN			900
#define PHY_WAIT_USLEEP_MAX			1000

#define PCIE_CAP_OFF				0x70
#define LINK_CONTROL2_LINK_STATUS2_REG		(PCIE_CAP_OFF + 0x30)
#define HDMA_SIZE				SZ_16M

static DEFINE_MUTEX(tcu_mutex);

struct xring_pcie_msix_tbl {
	u64 msg_addr;
	u32 msg_data;
	u32 vector_ctrl;
};

struct mtk_gpio {
	int pmic_en;
	int pmic_rst;
};

struct regmap	*regmap_ep_dbi;
struct regmap	*regmap_ep_ctrl;
struct regmap	*regmap_ep_atu;
struct regmap	*regmap_ep_hdma;
struct regmap	*regmap_ep_slv;
struct regmap   *regmap_pcie_phy;
struct regmap	*regmap_hss2_top;

struct regmap	*regmap_hss2_crg;

struct xring_pcie_clk	*clk;

struct device *dev;
struct mtk_gpio mdm_gpio;
struct mtk_gpio wifi_gpio;

static u32 msi_irq_num;
static u32 msix_irq_num;
static u32 hdma_index;
void __iomem	*virt_hdma_write_sar;
void __iomem	*virt_hdma_write_dar;
void __iomem	*virt_hdma_read_sar;
void __iomem	*virt_hdma_read_dar;
static uint8_t ep_lane_num = 1;
dma_addr_t write_sar_phy, write_dar_phy;
dma_addr_t read_sar_phy, read_dar_phy;

void __iomem	*bar0_virt_hdma;
void __iomem	*bar2_virt_hdma;
void __iomem	*bar3_virt_hdma;
dma_addr_t	bar0_inbound_tar, bar2_inbound_tar, bar3_inbound_tar;

bool is_fpga;
int target_speed;

static int port_num = 1;
bool	pcie_clk_is_enable;
u32 ep_slv_base;

static void pcie_ep_rdonly_wr_en(void)
{
	u32 reg;

	regmap_read(regmap_ep_dbi, EP_MISC_CTL_1_OFF, &reg);
	reg |= 0x01;
	regmap_write(regmap_ep_dbi, EP_MISC_CTL_1_OFF, reg);
}

static void pcie_ep_rdonly_wr_dis(void)
{
	u32 reg;

	regmap_read(regmap_ep_dbi, EP_MISC_CTL_1_OFF, &reg);
	reg &= (~1);
	regmap_write(regmap_ep_dbi, EP_MISC_CTL_1_OFF, reg);
}

static int xring_pcie_mem_shutdown(void)
{
	void __iomem *pctrl;
	u32 val;

	pctrl = ioremap(PCIE_PCTRL_BASE, PCIE_PCTRL_SIZE);
	if (!pctrl) {
		dev_err(dev, "failed to ioremap pctrl\n");
		return -ENOMEM;
	}

	if (port_num == 0) {
		val = (PCTRL_MEM_LGROUP0_SC_PCIE0_MEM_SD_MASK << 16) &
			(~PCTRL_MEM_LGROUP0_SC_PCIE0_MEM_SD_MASK);
		writel(val, pctrl + PCTRL_MEM_LGROUP0);

		if ((readl(pctrl + PCTRL_MEM_LPSTATUS0) &
			PCTRL_MEM_LPSTATUS0_PCIE0_MEM_SD_ACK_MASK) != 0) {
			dev_err(dev, "failed to set pcie0 mem shutdown\n");
			return -EINVAL;
		}

	} else if (port_num == 1) {
		val = (PCTRL_MEM_LGROUP1_SC_PCIE1_MEM_SD_MASK << 16) &
			(~PCTRL_MEM_LGROUP1_SC_PCIE1_MEM_SD_MASK);
		writel(val, pctrl + PCTRL_MEM_LGROUP1);

		if ((readl(pctrl + PCTRL_MEM_LPSTATUS1) &
			PCTRL_MEM_LPSTATUS1_PCIE1_MEM_SD_ACK_MASK) != 0) {
			dev_err(dev, "failed to set pcie1 mem shutdown\n");
			return -EINVAL;
		}

	} else {
		dev_err(dev, "Invalid port num, %d\n", port_num);
		return -EINVAL;
	}

	iounmap(pctrl);

	return 0;
}

static int xring_pcie_ep_phy_firmware_upd(void)
{
	void __iomem *pcie_phy_apb0;
	u32 reg;
	int i;

	dev_info(dev, "enter xring_pcie_phy_firmware_upd procedure\n");

	pcie_phy_apb0 = ioremap(0xcee00000, 0x40000);

	/* Change rom boot mode to sram boot mode  */
	regmap_read(regmap_pcie_phy, PCIE_PHY_PHY_MISC_CFG, &reg);
	reg &= ~PCIE_PHY_PHY_MISC_CFG_PHY0_SRAM_BYPASS_MODE_MASK;
	reg &= ~PCIE_PHY_PHY_MISC_CFG_PHY0_SRAM_BOOTLOAD_BYPASS_MODE_MASK;
	reg |= 0x3 << PCIE_PHY_PHY_MISC_CFG_PHY0_SRAM_BOOTLOAD_BYPASS_MODE_SHIFT;
	reg |= 0x0 << PCIE_PHY_PHY_MISC_CFG_PHY0_SRAM_BYPASS_MODE_SHIFT;
	regmap_write(regmap_pcie_phy, PCIE_PHY_PHY_MISC_CFG, reg);

	for (i =  0; i < 100; i++) {
		regmap_read(regmap_pcie_phy, PCIE_PHY_PHY_MISC_RPT, &reg);
		reg &= PCIE_PHY_PHY_MISC_RPT_PHY0_SRAM_INIT_DONE_MASK;
		reg >>= PCIE_PHY_PHY_MISC_RPT_PHY0_SRAM_INIT_DONE_SHIFT;
		/* 100 times for waiting sram_init_done == 0x1 */
		if (reg == 0x1)
			break;

		usleep_range(100, 200);
	}

	if (i == 100) {
		dev_err(dev,
			"xring_pcie_phy_firmware_upd wait sram_init_done failed\n");
		return -1;
	}

	/* Load Firmware to SRAM, SRAM Base on DWC_PHY CSR 0x8000  */
	for (i = 0; i < sizeof(xring_pcie_phy_firmware) / sizeof(u32); i++)
		writel(xring_pcie_phy_firmware[i], pcie_phy_apb0 + ((0x8000 + i) << 2));

	/* After SRAM loaded, Write 0x1 to sram_ext_ld_done to trigger sram load done */
	regmap_write(regmap_pcie_phy, PCIE_PHY_PHY_MISC_CFG2, 0x1);

	dev_info(dev, "xring_pcie_phy_firmware_upd procedure done\n");

	return 0;
}

static void ep_init(void)
{
	u32 reg;

	xring_pcie_ep_phy_firmware_upd();

	regmap_read(regmap_ep_ctrl, PCIE_CTRL_PERST_N_OVERRIDE, &reg);
	reg &= ~PCIE_CTRL_PERST_N_OVERRIDE_PERST_N_SRC_OVER_MASK;
	regmap_write(regmap_ep_ctrl, PCIE_CTRL_PERST_N_OVERRIDE, reg);/* reset ep*/

	msleep(100);
	reg |= PCIE_CTRL_PERST_N_OVERRIDE_PERST_N_SRC_OVER_MASK;
	regmap_write(regmap_ep_ctrl, PCIE_CTRL_PERST_N_OVERRIDE, reg);/* unreset ep */

	xring_pcie_mem_shutdown();

	pcie_ep_rdonly_wr_en();

	/* change ep device id and vendor id */
	regmap_write(regmap_ep_dbi, DEV_VEN_ID_OFFSET, DEV_VEN_ID);

	/* set aux clk rate */
	regmap_read(regmap_ep_dbi, AUX_CLK_FREQ_OFF, &reg);
	reg &= ~AUX_CLK_FREQ_MASK;
	reg |= AUX_CLK_FREQ;
	regmap_write(regmap_ep_dbi, AUX_CLK_FREQ_OFF, reg);

	pcie_ep_rdonly_wr_dis();

	/* set target speed */
	if ((target_speed >= 1) && (target_speed <= 4)) {
		regmap_read(regmap_ep_dbi, LINK_CONTROL2_LINK_STATUS2_REG, &reg);
		reg &= ~GENMASK(3, 0);
		reg |= target_speed;
		regmap_write(regmap_ep_dbi, LINK_CONTROL2_LINK_STATUS2_REG, reg);
	} else {
		dev_err(dev, "Invalid target speed:%d, Use hardware capability.\n", target_speed);
	}

	dev_info(dev, "ep init done.\nven dev id:%d\ntarget speed:%d",
			DEV_VEN_ID, target_speed);
}

static void setup_ep_bars(void)
{
	pcie_ep_rdonly_wr_en();

	regmap_write(regmap_ep_dbi, BAR0_MASK_OFFSET, 0xffff);/* set BAR0 mask */
	regmap_write(regmap_ep_dbi, BAR0_OFFSET, 0xc);

	regmap_write(regmap_ep_dbi, BAR1_MASK_OFFSET, 0xfffe);/* set BAR1 mask */
	regmap_write(regmap_ep_dbi, BAR1_OFFSET, 0x0);

	regmap_write(regmap_ep_dbi, BAR2_MASK_OFFSET, 0xffff);/* set BAR2 mask */
	regmap_write(regmap_ep_dbi, BAR2_OFFSET, 0x1);

	regmap_write(regmap_ep_dbi, BAR3_MASK_OFFSET, 0xffff);/* set BAR3 mask */
	regmap_write(regmap_ep_dbi, BAR3_OFFSET, 0x0);

	pcie_ep_rdonly_wr_dis();

	dev_info(dev, "bar init done.\nbar0/1: 64bit pre-mem, size 64k\nbar2: 32bit io, size 64k\n bar3: 32bit mem, size 64k\n");
}


static void set_msi_irq_num(void)
{

	u32 value;

	pcie_ep_rdonly_wr_en();

	regmap_read(regmap_ep_dbi, MSI_CAP_REG, &value);
	value = value | (MSI_IRQ_NUM_32 << MSI_IRQ_NUM_OFFSET);
	regmap_write(regmap_ep_dbi, MSI_CAP_REG, value);

	pcie_ep_rdonly_wr_dis();
}

static int xring_pcie_prog_outbound_atu(int index, int type,
					u64 cpu_addr, u64 pci_addr,
					u64 size)
{
	u32 retries, val;
	u32 offset = index << 9;
	u64 limit_addr = cpu_addr + size - 1;

	regmap_write(regmap_ep_atu, offset + PCIE_ATU_UNR_LOWER_BASE, lower_32_bits(cpu_addr));

	regmap_write(regmap_ep_atu, offset + PCIE_ATU_UNR_UPPER_BASE, upper_32_bits(cpu_addr));

	regmap_write(regmap_ep_atu, offset + PCIE_ATU_UNR_LOWER_LIMIT, lower_32_bits(limit_addr));

	regmap_write(regmap_ep_atu, offset + PCIE_ATU_UNR_UPPER_LIMIT, upper_32_bits(limit_addr));

	regmap_write(regmap_ep_atu, offset + PCIE_ATU_UNR_LOWER_TARGET, lower_32_bits(pci_addr));

	regmap_write(regmap_ep_atu, offset + PCIE_ATU_UNR_UPPER_TARGET, upper_32_bits(pci_addr));

	val = type;
	if (upper_32_bits(limit_addr) > upper_32_bits(cpu_addr))
		val |= PCIE_ATU_INCREASE_REGION_SIZE;

	regmap_write(regmap_ep_atu, offset + PCIE_ATU_UNR_REGION_CTRL1, val);

	regmap_write(regmap_ep_atu, PCIE_ATU_UNR_REGION_CTRL2, PCIE_ATU_ENABLE);
	for (retries = 0; retries < LINK_WAIT_MAX_IATU_RETRIES; retries++) {
		regmap_read(regmap_ep_atu, PCIE_ATU_UNR_REGION_CTRL2, &val);
		if (val & PCIE_ATU_ENABLE)
			return SUCCESS;
		mdelay(LINK_WAIT_IATU);
	}
	dev_err(dev, "Outbound iATU is not being enabled\n");

	return -ENOMEM;
}
static void xring_pcie_disable_outbound_atu(int index)
{
	u32 offset = index << 9;

	regmap_write(regmap_ep_atu, PCIE_ATU_UNR_REGION_CTRL2 + offset, 0);
}
static int xring_pcie_raise_msi(u8 interrupt_num)
{
	u32 msg_addr_lower, msg_addr_upper;
	u32 msg_ctrl, msg_data;
	u64 msg_addr;
	bool has_upper;
	int ret;
	int index = 0;

	regmap_read(regmap_ep_dbi, MSI_CAP_REG, &msg_ctrl);
	has_upper = !!(msg_ctrl & (PCI_MSI_FLAGS_64BIT << 16));
	regmap_read(regmap_ep_dbi, MSI_CAP_REG + PCI_MSI_ADDRESS_LO, &msg_addr_lower);
	if (has_upper) {
		regmap_read(regmap_ep_dbi, MSI_CAP_REG + PCI_MSI_ADDRESS_HI, &msg_addr_upper);
		regmap_read(regmap_ep_dbi, MSI_CAP_REG + PCI_MSI_DATA_64, &msg_data);
	} else {
		msg_addr_upper = 0;
		regmap_read(regmap_ep_dbi, MSI_CAP_REG + PCI_MSI_DATA_32, &msg_data);
	}

	msg_data = msg_data & 0xffff;
	msg_addr = ((u64)msg_addr_upper) << 32 | (msg_addr_lower);
	ret = xring_pcie_prog_outbound_atu(index, PCIE_ATU_TYPE_MEM,
					(u64)(ep_slv_base + MSI_SLV_OFFSET), msg_addr, SZ_4K);
	if (ret)
		return ret;
	regmap_write(regmap_ep_slv, MSI_SLV_OFFSET, msg_data | (interrupt_num - 1));
	xring_pcie_disable_outbound_atu(index);

	return SUCCESS;
}
static int xring_pcie_raise_msix(u32 interrupt_num)
{
	regmap_write(regmap_ep_dbi, PCIE_MSIX_DOORBELL, (interrupt_num - 1));
	return SUCCESS;
}
static int xring_pcie_hdma_write_config(struct device *dev, int index)
{
	int offset = index * HDMA_CH_OFFSET;
	int offset_dma = 0;
	u32 sar;

	/* init sar */
	dev_info(dev, "init write sar...\n");
	while (offset_dma < HDMA_SIZE) {
		get_random_bytes(&sar, sizeof(u32));
		writel(sar, (virt_hdma_write_sar + offset_dma));
		offset_dma = offset_dma + 4;
	}
	dev_info(dev, "init [write] sar done.\n");

	regmap_write(regmap_ep_hdma, offset + HDMA_SAR_LOW_OFF_WRCH, lower_32_bits(write_sar_phy));
	regmap_write(regmap_ep_hdma, offset + HDMA_SAR_HIGH_OFF_WRCH, upper_32_bits(write_sar_phy));
	regmap_write(regmap_ep_hdma, offset + HDMA_DAR_LOW_OFF_WRCH, lower_32_bits(write_dar_phy));
	regmap_write(regmap_ep_hdma, offset + HDMA_DAR_HIGH_OFF_WRCH, upper_32_bits(write_dar_phy));
	regmap_write(regmap_ep_hdma, offset + HDMA_XFERSIZE_OFF_WRCH, HDMA_SIZE);
	regmap_write(regmap_ep_hdma, offset + HDMA_INT_SETUP_OFF, 0);
	regmap_write(regmap_ep_hdma, offset + HDMA_EN_OFF_WRCH, HDMA_ENABLE_WRCH);

	return SUCCESS;
}
static int xring_pcie_hdma_write_start(struct device *dev, int index)
{
	int offset = index * HDMA_CH_OFFSET;

	return regmap_write(regmap_ep_hdma, offset + HDMA_DOORBELL_OFF_WRCH, HDMA_DB_START_WRCH);
}

static int xring_pcie_hdma_read_config(struct device *dev, int index)
{
	int offset = HDMA_RD_BASE + (index * HDMA_CH_OFFSET);
	long offset_dma = 0;
	u32 sar;

	/* init sar */
	dev_info(dev, "init read sar...\n");
	while (offset_dma < HDMA_SIZE) {
		get_random_bytes(&sar, sizeof(u32));
		writel(sar, (virt_hdma_write_sar + offset_dma));
		offset_dma = offset_dma + 4;
	}
	dev_info(dev, "init [read] sar done.\n");

	regmap_write(regmap_ep_hdma, offset + HDMA_SAR_LOW_OFF_WRCH, lower_32_bits(read_sar_phy));
	regmap_write(regmap_ep_hdma, offset + HDMA_SAR_HIGH_OFF_RDCH, upper_32_bits(read_sar_phy));
	regmap_write(regmap_ep_hdma, offset + HDMA_DAR_LOW_OFF_RDCH, lower_32_bits(read_dar_phy));
	regmap_write(regmap_ep_hdma, offset + HDMA_DAR_HIGH_OFF_RDCH, upper_32_bits(read_dar_phy));
	regmap_write(regmap_ep_hdma, offset + HDMA_XFERSIZE_OFF_RDCH, HDMA_SIZE);
	regmap_write(regmap_ep_hdma, offset + HDMA_INT_SETUP_OFF, 0);
	regmap_write(regmap_ep_hdma, offset + HDMA_EN_OFF_RDCH, HDMA_ENABLE_RDCH);

	return SUCCESS;
}
static int xring_pcie_hdma_read_start(struct device *dev, int index)
{
	int offset = HDMA_RD_BASE + (index * HDMA_CH_OFFSET);

	return regmap_write(regmap_ep_hdma, offset + HDMA_DOORBELL_OFF_RDCH, HDMA_DB_START_RDCH);
}
static int wait_for_hdma_done(int index)
{
	u32 wr_size;
	u32 rd_size;
	u32 retries;
	int offset = index * HDMA_CH_OFFSET;

	/* wait for hdma done ,max time == 1s */
	for (retries = 0; retries < HDMA_WAIT_MAX_RETRIES; retries++) {
		regmap_read(regmap_ep_hdma, offset + HDMA_XFERSIZE_OFF_WRCH, &wr_size);
		regmap_read(regmap_ep_hdma,
				offset + HDMA_RD_BASE + HDMA_XFERSIZE_OFF_RDCH,
				&rd_size);

		if ((wr_size == 0) && (rd_size == 0)) {
			dev_info(dev, "HDMA trans done.\n");
			return 0;
		}

		usleep_range(PHY_WAIT_USLEEP_MIN, PHY_WAIT_USLEEP_MAX);
	}

	dev_err(dev, "HDMA trans not done in 1s. wr_size:0x%x, rd_size:0x%x\n", wr_size, rd_size);

	return -ETIMEDOUT;
}

static int xring_pcie_hdma_done_check(int index)
{
	u32 hdma_write_sar_reg;
	u32 hdma_write_dar_reg;
	u32 hdma_read_sar_reg;
	u32 hdma_read_dar_reg;
	int offset = 0;

	dev_info(dev, "hdma compare dar and sar data...\n");
	while (offset < HDMA_SIZE) {
		hdma_write_sar_reg = readl(virt_hdma_write_sar + offset);
		hdma_write_dar_reg = readl(virt_hdma_write_dar + offset);
		hdma_read_sar_reg = readl(virt_hdma_read_sar + offset);
		hdma_read_dar_reg = readl(virt_hdma_read_dar + offset);

		if (hdma_write_sar_reg != hdma_write_dar_reg) {
			dev_err(dev, "hdma write check fail!!! offset:0x%x, sar:0x%x, dar:0x%x\n",
					offset, hdma_write_sar_reg, hdma_write_dar_reg);
			return -EINVAL;
		}

		if (hdma_read_sar_reg != hdma_read_dar_reg) {
			dev_err(dev, "hdma read check fail!!! offset:0x%x, sar:0x%x, dar:0x%x\n",
					offset, hdma_read_sar_reg, hdma_read_dar_reg);
			return -EINVAL;
		}

		offset += 4;
	}

	dev_info(dev, "hdma read and write check success!\n");

	return 0;
}

static void xring_pcie_hdma(struct device *dev, int index)
{
	int ret = 0;

	xring_pcie_hdma_write_config(dev, index);
	xring_pcie_hdma_read_config(dev, index);
	xring_pcie_hdma_write_start(dev, index);
	xring_pcie_hdma_read_start(dev, index);

	ret = wait_for_hdma_done(index);
	if (ret)
		BUG_ON(ret);

}

static void xring_pcie_hdma_check(struct device *dev, int index)
{
	int ret = 0;

	xring_pcie_hdma_write_config(dev, index);
	xring_pcie_hdma_read_config(dev, index);
	xring_pcie_hdma_write_start(dev, index);
	xring_pcie_hdma_read_start(dev, index);

	ret = wait_for_hdma_done(index);
	if (ret)
		BUG_ON(ret);

	xring_pcie_hdma_done_check(index);
}

static ssize_t set_ep_inbound(void)
{
	long offset_dma = 0;

	while (offset_dma < SZ_4K) {
		writel(offset_dma, (bar0_virt_hdma + offset_dma));
		writel(offset_dma, (bar2_virt_hdma + offset_dma));
		writel(offset_dma, (bar3_virt_hdma + offset_dma));
		offset_dma = offset_dma + 4;
	}

	/* bar0 -> atu1 */
	/* set ep inbound target low address */
	regmap_write(regmap_ep_atu, TARGET_ADDR_OFFSET + INBOUND_OFFSET_BASE,
				bar0_inbound_tar);
	/* set ep inbound ctrl1  mem=0,io=2 */
	regmap_write(regmap_ep_atu, INBOUND_CTRL1_OFFSET + INBOUND_OFFSET_BASE,
				MEM_BAR_TYPE);
	/* ep inbound ctrl2 bit19,30 and 31 set 1,bar << 8 */
	regmap_write(regmap_ep_atu, INBOUND_CTRL2_OFFSET + INBOUND_OFFSET_BASE,
				(1 << PCIE_ATU_FUNC_NUM_MATCH_OFFSET |
				ATU_AND_BAR_MODE_ENABLE << ATU_AND_BAR_MODE_ENABLE_OFFSET
				| BAR0 << ATU_BAR_OFFSET));

	/* bar2 -> atu2 */
	/* set ep inbound target low address */
	regmap_write(regmap_ep_atu, INBOUND1_OFFSET + TARGET_ADDR_OFFSET +
				INBOUND_OFFSET_BASE, bar2_inbound_tar);
	/* set ep inbound ctrl1  mem=0,io=2 */
	regmap_write(regmap_ep_atu, INBOUND1_OFFSET + INBOUND_CTRL1_OFFSET +
				INBOUND_OFFSET_BASE, IO_BAR_TYPE);
	/* ep inbound ctrl2 bit19,30 and 31 set 1,ba r<< 8 */
	regmap_write(regmap_ep_atu, INBOUND1_OFFSET + INBOUND_CTRL2_OFFSET +
				INBOUND_OFFSET_BASE, (1 << PCIE_ATU_FUNC_NUM_MATCH_OFFSET |
				ATU_AND_BAR_MODE_ENABLE << ATU_AND_BAR_MODE_ENABLE_OFFSET |
				BAR2 << ATU_BAR_OFFSET));

	/* bar3 -> atu3 */
	/* set ep inbound target low address */
	regmap_write(regmap_ep_atu, INBOUND2_OFFSET + TARGET_ADDR_OFFSET +
				INBOUND_OFFSET_BASE, bar3_inbound_tar);
	/* set ep inbound ctrl1  mem=0,io=2 */
	regmap_write(regmap_ep_atu, INBOUND2_OFFSET + INBOUND_CTRL1_OFFSET +
				INBOUND_OFFSET_BASE, MEM_BAR_TYPE);
	/* ep inbound ctrl2 bit19,30 and 31 set 1, bar << 8 */
	regmap_write(regmap_ep_atu, INBOUND2_OFFSET + INBOUND_CTRL2_OFFSET +
				INBOUND_OFFSET_BASE, (1 << PCIE_ATU_FUNC_NUM_MATCH_OFFSET |
				ATU_AND_BAR_MODE_ENABLE << ATU_AND_BAR_MODE_ENABLE_OFFSET |
				BAR3 << ATU_BAR_OFFSET));

	return SUCCESS;
}

static void set_ep_lane_num(uint8_t ep_lane_num)
{
	u32 reg_en;
	u32 reg_set;

	/* enable lane */
	regmap_read(regmap_ep_dbi, LANE_ENABLE, &reg_en);
	reg_en &= ~(LANE_ENABLE_MASK << LANE_ENABLE_OFFSET);
	reg_en |= (ep_lane_num * 2 - 1) << LANE_ENABLE_OFFSET;
	regmap_write(regmap_ep_dbi, LANE_ENABLE, reg_en);

	/* set ep lane num */
	regmap_read(regmap_ep_dbi, NUM_OF_LANES, &reg_set);
	reg_set &= ~(LANE_SET_MASk << LANE_OFFSET);
	reg_set |= ep_lane_num << LANE_OFFSET;
	regmap_write(regmap_ep_dbi, NUM_OF_LANES, reg_set);
}


static int pcie_clk_enable(int port_num)
{
	int ret = 0;

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

	ret = clk_prepare_enable(clk->clk_pcie_pll);
	if (ret) {
		dev_err(dev, "failed to enable clk clk_pcie_pll\n");
		goto err_pll;
	}

	ret = clk_prepare_enable(clk->clk_pcie_fw);
	if (ret) {
		dev_err(dev, "failed to enable clk clk_pcie_fw\n");
		goto err_pcie_fw;
	}

	dev_info(dev, "enable clk done\n");
	pcie_clk_is_enable = true;

	return 0;

err_pcie_fw:
	clk_disable_unprepare(clk->clk_pcie_pll);
err_pll:
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

	return ret;
}

static int xring_pcie_parse_clk(void)
{
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

	return 0;
}

static void xring_pcie_powerup_rst_assert(int port_num)
{
	if (port_num == 0)
		regmap_write(regmap_hss2_crg,
				HSS2_CRG_RST0_W1C,
				HSS2_CRG_RST0_RO_IP_RST_PCIE0_POWER_UP_N_MASK);
	else if (port_num == 1)
		regmap_write(regmap_hss2_crg,
				HSS2_CRG_RST0_W1C,
				HSS2_CRG_RST0_RO_IP_RST_PCIE1_POWER_UP_N_MASK);
	else
		dev_err(dev, "Invalid port num, %d\n", port_num);
	dev_info(dev, "pcie%d pwr up rst.\n", port_num);
}

static int xring_pcie_powerup_rst_deassert(int port_num)
{

	if (port_num == 0) {
		regmap_write(regmap_hss2_crg,
				HSS2_CRG_RST0_W1S,
				HSS2_CRG_RST0_RO_IP_RST_PCIE0_POWER_UP_N_MASK);
	} else if (port_num == 1) {
		regmap_write(regmap_hss2_crg,
				HSS2_CRG_RST0_W1S,
				HSS2_CRG_RST0_RO_IP_RST_PCIE1_POWER_UP_N_MASK);
	} else {
		dev_err(dev, "Invalid port num, %d\n", port_num);
		return -EINVAL;
	}

	dev_info(dev, "pcie%d pwr up de-rst.\n", port_num);

	return 0;
}

static int ep_gate_init(int port_num)
{
	u32 val;

	regmap_read(regmap_hss2_crg, HSS2_CRG_RST0_RO, &val);
	val = (~val) & HSS2_CRG_RST0_PCIE;
	regmap_write(regmap_hss2_crg,
			HSS2_CRG_RST0_W1S, HSS2_CRG_RST0_PCIE);

	regmap_read(regmap_hss2_crg, HSS2_CRG_CLKGT0_RO, &val);
	val = (~val) & HSS2_CRG_CLKGT0_PCIE;
	regmap_write(regmap_hss2_crg,
			HSS2_CRG_CLKGT0_W1S, HSS2_CRG_CLKGT0_PCIE);

	if (port_num == 0) {
		regmap_write(regmap_hss2_crg,
				HSS2_CRG_CLKGT0_W1S, HSS2_CRG_CLKGT0_PCIE0);
		regmap_write(regmap_hss2_crg,
				HSS2_CRG_RST0_W1S, HSS2_CRG_RST0_PCIE0);
	} else if (port_num == 1) {
		regmap_write(regmap_hss2_crg,
				HSS2_CRG_CLKGT0_W1S, HSS2_CRG_CLKGT0_PCIE1);
		regmap_write(regmap_hss2_crg,
				HSS2_CRG_RST0_W1S, HSS2_CRG_RST0_PCIE1);
	} else {
		dev_err(dev, "Invalid port num, %d\n", port_num);
		return -EINVAL;
	}

	return 0;

}

static void xring_pcie_ep_phy_init(void)
{
	u32 reg;

	/* set refclk cmn mode */
	regmap_read(regmap_pcie_phy, PCIE_PHY_PIPE_RX0_CFG, &reg);
	reg |= 1 << PCIE_PHY_PIPE_RX0_CFG_PIPE_RX0_ES0_CMN_REFCLK_MODE_SHIFT;
	regmap_write(regmap_pcie_phy, PCIE_PHY_PIPE_RX0_CFG, reg);

	regmap_read(regmap_pcie_phy, PCIE_PHY_PIPE_RX1_CFG, &reg);
	reg |= 1 << PCIE_PHY_PIPE_RX1_CFG_PIPE_RX1_ES0_CMN_REFCLK_MODE_SHIFT;
	regmap_write(regmap_pcie_phy, PCIE_PHY_PIPE_RX1_CFG, reg);

	/* set diffbuf DE_EMP */
	regmap_read(regmap_pcie_phy, PCIE_PHY_PCIE_SYS_TOP_CFG, &reg);
	reg &= ~PCIE_PHY_PCIE_SYS_TOP_CFG_SW_DE_EMP_MASK;
	regmap_write(regmap_pcie_phy, PCIE_PHY_PCIE_SYS_TOP_CFG, reg);

	dev_info(dev, "phy init done.\n");
}

static int ep_crg_init(int port_num)
{
	int ret = 0;

	ret = xring_pcie_parse_clk();
	if (ret)
		return ret;

	if (!pcie_clk_is_enable) {
		ret = pcie_clk_enable(port_num);
		if (ret)
			return ret;
	}

	ret = ep_gate_init(port_num);
	if (ret)
		return ret;

	xring_pcie_ep_phy_init();

	/* change device_type as ep */
	regmap_write(regmap_ep_ctrl, DEVICE_TYPE_OFFSET, 0);

	ret = xring_pcie_powerup_rst_deassert(port_num);
	if (ret)
		return ret;
	dev_info(dev, "pcie%d CRG register init.\n", port_num);
	return 0;
}

static void ep_ltssm_en(void)
{
	u32 reg;

	regmap_read(regmap_ep_ctrl, LTSSM_EN_OFFSET, &reg);
	reg &= ~(GEN_MASK);
	reg = reg | 1;
	regmap_write(regmap_ep_ctrl, LTSSM_EN_OFFSET, reg);
}

static int xring_pcie_wait_smmu_bit(int offset, u32 val)
{
	int retries;
	u32 reg;

	for (retries = 0; retries < LINK_WAIT_MAX_RETRIES; retries++) {
		regmap_read(regmap_hss2_top, offset, &reg);
		if (reg & val)
			return 0;
		usleep_range(LINK_WAIT_USLEEP_MIN, LINK_WAIT_USLEEP_MAX);
	}

	dev_err(dev, "failed to bypass smmu. offset:0x%x, reg:0x%x\n", offset, reg);

	return -ETIMEDOUT;
}

static void bypass_smmu(void)
{
	u32 reg;
	int ret;

	/* tcu */
	mutex_lock(&tcu_mutex);
	regmap_read(regmap_hss2_top, HSS2_TOP_SMMU_TCU_QCH_PD_STATE, &reg);
	if (!(reg & SC_SMMU_TCU_QACCEPTN_PD)) {
		dev_info(dev, "trigger tcu link.\n");

		regmap_write(regmap_hss2_top, HSS2_TOP_SMMU_TCU_QCH_CG_CTRL, SC_SMMU_TCU_QREQN_CG_SET);
		ret = xring_pcie_wait_smmu_bit(HSS2_TOP_SMMU_TCU_QCH_CG_STATE, SC_SMMU_TCU_QACCEPTN_CG_ST);
		if (ret) {
			mutex_unlock(&tcu_mutex);
			return;
		}

		regmap_write(regmap_hss2_top, HSS2_TOP_SMMU_TCU_QCH_PD_CTRL, SC_SMMU_TCU_QREQN_PD);
		ret = xring_pcie_wait_smmu_bit(HSS2_TOP_SMMU_TCU_QCH_PD_STATE, SC_SMMU_TCU_QACCEPTN_PD);
		if (ret) {
			mutex_unlock(&tcu_mutex);
			return;
		}
	}
	mutex_unlock(&tcu_mutex);

	/* tbu0 */
	regmap_read(regmap_hss2_top, HSS2_TOP_SMMU_TBU0_QCH_PD_STATE, &reg);
	if ((port_num == 0) && !(reg & SC_SMMU_TCU_QACCEPTN_PD)) {
		dev_info(dev, "trigger tbu0 link.\n");

		regmap_write(regmap_hss2_top, HSS2_TOP_SMMU_TBU0_QCH_CG_CTRL, SC_SMMU_TCU_QREQN_CG_SET);
		ret = xring_pcie_wait_smmu_bit(HSS2_TOP_SMMU_TBU0_QCH_CG_STATE, SC_SMMU_TCU_QACCEPTN_CG_ST);
		if (ret)
			return;

		regmap_write(regmap_hss2_top, HSS2_TOP_SMMU_TBU0_QCH_PD_CTRL, SC_SMMU_TCU_QREQN_PD);
		ret = xring_pcie_wait_smmu_bit(HSS2_TOP_SMMU_TBU0_QCH_PD_STATE, SC_SMMU_TCU_QACCEPTN_PD);
		if (ret)
			return;
	}

	/* tbu1 */
	regmap_read(regmap_hss2_top, HSS2_TOP_SMMU_TBU1_QCH_PD_STATE, &reg);
	if ((port_num == 1) && !(reg & SC_SMMU_TCU_QACCEPTN_PD)) {
		dev_info(dev, "trigger tbu1 link.\n");

		regmap_write(regmap_hss2_top, HSS2_TOP_SMMU_TBU1_QCH_CG_CTRL, SC_SMMU_TCU_QREQN_CG_SET);
		ret = xring_pcie_wait_smmu_bit(HSS2_TOP_SMMU_TBU1_QCH_CG_STATE, SC_SMMU_TCU_QACCEPTN_CG_ST);
		if (ret)
			return;

		regmap_write(regmap_hss2_top, HSS2_TOP_SMMU_TBU1_QCH_PD_CTRL, SC_SMMU_TCU_QREQN_PD);
		ret = xring_pcie_wait_smmu_bit(HSS2_TOP_SMMU_TBU1_QCH_PD_STATE, SC_SMMU_TCU_QACCEPTN_PD);
		if (ret)
			return;
	}

	dev_info(dev, "smmu bypassed.\n");
}

static void ep_init_enumeration(void)
{
	ep_crg_init(port_num);
	ep_init();
	setup_ep_bars();
	set_ep_inbound();
	set_msi_irq_num();
	ep_ltssm_en();

	dev_info(dev, "ep init done. start enumeration...\n");

	if (port_num == 0)
		xring_pcie_probe_port_by_port(1);
	else if (port_num == 1)
		xring_pcie_probe_port_by_port(0);
	else
		dev_err(dev, "Invalid port num\n");
}

static ssize_t ep_access_rc(void)
{
	void *virt_addr;
	int offset = 0;
	u32 reg;
	u32 slv_val, ddr_val;
	dma_addr_t ep_virt_target_addr;

	virt_addr = dma_alloc_coherent(dev, SZ_1K, &ep_virt_target_addr, GFP_KERNEL);
	if (!virt_addr)
		return -ENOMEM;
	/* set ep outbound base low address */
	regmap_write(regmap_ep_atu, ATU1_OUTBOUND_BASE, ep_slv_base);
	/* set ep outbound limit address */
	regmap_write(regmap_ep_atu, ATU1_OUTBOUND_LIMIT, ep_slv_base + LIMIT_SIZE);

	/* set ep outbound target addrss */
	regmap_write(regmap_ep_atu, ATU1_OUTBOUND_TARGET, ep_virt_target_addr);

	regmap_read(regmap_ep_atu, ATU1_OUTBOUND_CTRL2, &reg);
	reg |= PCIE_ATU_ENABLE;
	/* enable atu1 */
	regmap_write(regmap_ep_atu, ATU1_OUTBOUND_CTRL2, reg);

	/* init the ddr address */
	while (offset < SZ_1K) {
		writel(HDMA_TEST_DATA_5A, virt_addr + offset);
		offset = offset + 4;
	}
	offset = 0;
	/* compare addr value and slv value */
	while (offset < SZ_1K) {
		regmap_write(regmap_ep_slv, offset, HDMA_TEST_DATA_A5);

		regmap_read(regmap_ep_slv, offset, &slv_val);
		ddr_val = readl(virt_addr + offset);
		if (slv_val != ddr_val) {
			dev_err(dev, "ep access rc fail, offset = %x, slv_val=%x, ddr_val=%x\n",
						offset, slv_val, ddr_val);
			dma_free_coherent(dev, SZ_1K, virt_addr, ep_virt_target_addr);
			return FAIL;
		}
		offset = offset + 4;
	}
	dev_info(dev, "ep access rc success\n");

	/* disable atu */
	regmap_read(regmap_ep_atu, ATU1_OUTBOUND_CTRL2, &reg);
	reg &= ~PCIE_ATU_ENABLE;
	regmap_write(regmap_ep_atu, ATU1_OUTBOUND_CTRL2, reg);

	dma_free_coherent(dev, SZ_1K, virt_addr, ep_virt_target_addr);

	return SUCCESS;
}


static void modem_power_up(void)
{
	gpio_set_value(mdm_gpio.pmic_en, 1);
	gpio_set_value(mdm_gpio.pmic_rst, 1);
	xring_pcie_probe_port_by_port(0);
}

static void modem_power_down(void)
{
	gpio_set_value(mdm_gpio.pmic_rst, 0);
	gpio_set_value(mdm_gpio.pmic_en, 0);
	xring_pcie_remove_port_by_port(0);
}

static void wifi_power_up(void)
{
	gpio_set_value(wifi_gpio.pmic_en, 1);
	gpio_set_value(wifi_gpio.pmic_rst, 1);
	xring_pcie_probe_port_by_port(1);
}

static void wifi_power_down(void)
{
	gpio_set_value(wifi_gpio.pmic_rst, 0);
	gpio_set_value(wifi_gpio.pmic_en, 0);
	xring_pcie_remove_port_by_port(1);
}

static ssize_t msi_irq_num_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	dev_info(dev, "msi_irq_num = %d\n", msi_irq_num);

	return SUCCESS;
}

static ssize_t msix_irq_num_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	dev_info(dev, "msix_irq_num = %d\n", msix_irq_num);

	return SUCCESS;
}
static ssize_t hdma_index_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	dev_info(dev, "hdma_index = %d\n", hdma_index);

	return SUCCESS;
}
static ssize_t msi_irq_num_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int value;
	int ret;

	ret = kstrtoint(buf, DECIMALISM, &value);
	if ((value <= 0) || (value > 32)) {
		dev_err(dev, "msi irq num invalid value!\n");
		return FAIL;
	}

	msi_irq_num = value;

	return count;
}

static ssize_t msix_irq_num_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int value;
	int ret;

	ret = kstrtoint(buf, DECIMALISM, &value);
	ret = kstrtoint(buf, DECIMALISM, &value);
	if ((value <= 0) || (value > 32)) {
		dev_err(dev, "msix irq num invalid value!\n");
		return FAIL;
	}

	msix_irq_num = value;

	return count;
}

static ssize_t hdma_index_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	u32 value;
	int ret;

	ret = kstrtoint(buf, DECIMALISM, &value);
	hdma_index = value;

	return count;
}

static ssize_t set_ep_lane_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	u8 lane_num;
	u32 reg;

	regmap_read(regmap_ep_dbi, NUM_OF_LANES, &reg);
	lane_num = (reg & LANE_NUM_NASK) >> LANE_OFFSET;
	dev_info(dev, "ep_lane num = %d\n", lane_num);

	return SUCCESS;
}

static ssize_t set_ep_lane_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	u32 value;
	int ret;

	ret = kstrtoint(buf, DECIMALISM, &value);
	ep_lane_num = value;

	return count;
}

static ssize_t all_function_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	dev_info(dev, "1. set ep ltssm en\n");
	dev_info(dev, "2. setup ep bars\n");
	dev_info(dev, "3. set msi irq numbers\n");
	dev_info(dev, "4. set_ep_inbound\n");
	dev_info(dev, "5. raise msi irq\n");
	dev_info(dev, "6. raise msix irq\n");
	dev_info(dev, "7. start hdma ops\n");
	dev_info(dev, "8. EP access RC\n");
	dev_info(dev, "9. Set ep lane num\n");
	dev_info(dev, "10. EP init and enumeration\n");
	dev_info(dev, "11. start hdma with check\n");
	dev_info(dev, "12. bypass smmu\n");

	return SUCCESS;
}

static ssize_t all_function_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	u32 value;
	int ret;

	ret = kstrtoint(buf, DECIMALISM, &value);
	switch (value) {
	case 1: {
		ep_init();
		break;
	}
	case 2: {
		setup_ep_bars();
		break;
	}
	case 3: {
		set_msi_irq_num();
		break;
	}
	case 4: {
		set_ep_inbound();
		break;
	}
	case 5: {
		xring_pcie_raise_msi(msi_irq_num);
		break;
	}
	case 6: {
		xring_pcie_raise_msix(msix_irq_num);
		break;
	}
	case 7: {
		xring_pcie_hdma(dev, hdma_index);
		break;
	}
	case 8: {
		ep_access_rc();
		break;
	}
	case 9: {
		set_ep_lane_num(ep_lane_num);
		break;
	}
	case 10: {
		ep_init_enumeration();
		break;
	}
	case 11: {
		xring_pcie_hdma_check(dev, hdma_index);
		break;
	}
	case 12: {
		bypass_smmu();
		break;
	}
	default: {
		dev_err(dev, "Invalid num\n");
		return count;
	}
	}

	return count;
}

static ssize_t hdma_pressure_start_show(struct device *dev,
					struct device_attribute *attr, char *buf)
{
	dev_info(dev, "This is an hdma test node!\n");
	return SUCCESS;
}
static ssize_t hdma_pressure_start_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	int hdma_write_offset = 0;
	int hdma_read_offset = HDMA_RD_BASE;
	u32 check_offset;
	int ret;
	u32 wr_reg;
	u32 rd_reg;
	u32 hdma_write_sar_reg;
	u32 hdma_write_dar_reg;
	u32 hdma_read_sar_reg;
	u32 hdma_read_dar_reg;
	u32 wait_times;

	ret = kstrtoint(buf, DECIMALISM, &check_offset);
	writel(HDMA_TEST_DATA_5A, (virt_hdma_write_sar + check_offset));

	writel(DDR_DAR_INIT_VAL, (virt_hdma_write_dar + check_offset));
	regmap_write(regmap_ep_hdma, hdma_write_offset + HDMA_SAR_LOW_OFF_WRCH,
				lower_32_bits(write_sar_phy));
	regmap_write(regmap_ep_hdma, hdma_write_offset + HDMA_SAR_HIGH_OFF_WRCH,
				upper_32_bits(write_sar_phy));
	regmap_write(regmap_ep_hdma, hdma_write_offset + HDMA_DAR_LOW_OFF_WRCH,
				lower_32_bits(write_dar_phy));
	regmap_write(regmap_ep_hdma, hdma_write_offset + HDMA_DAR_HIGH_OFF_WRCH,
				upper_32_bits(write_dar_phy));
	regmap_write(regmap_ep_hdma, hdma_write_offset + HDMA_XFERSIZE_OFF_WRCH,
				HDMA_SIZE);
	regmap_write(regmap_ep_hdma, hdma_write_offset + HDMA_EN_OFF_WRCH,
				HDMA_ENABLE_WRCH);

	/* hdma read */
	writel(HDMA_TEST_DATA_A5, (virt_hdma_read_sar + check_offset));
	writel(DDR_DAR_INIT_VAL, (virt_hdma_read_dar + check_offset));

	regmap_write(regmap_ep_hdma, hdma_read_offset + HDMA_SAR_LOW_OFF_RDCH,
				lower_32_bits(read_sar_phy));
	regmap_write(regmap_ep_hdma, hdma_read_offset + HDMA_SAR_HIGH_OFF_RDCH,
				upper_32_bits(read_sar_phy));
	regmap_write(regmap_ep_hdma, hdma_read_offset + HDMA_DAR_LOW_OFF_RDCH,
				lower_32_bits(read_dar_phy));
	regmap_write(regmap_ep_hdma, hdma_read_offset + HDMA_DAR_HIGH_OFF_RDCH,
				upper_32_bits(read_dar_phy));
	regmap_write(regmap_ep_hdma, hdma_read_offset + HDMA_XFERSIZE_OFF_RDCH,
				HDMA_SIZE);
	regmap_write(regmap_ep_hdma, hdma_read_offset + HDMA_EN_OFF_RDCH,
				HDMA_ENABLE_RDCH);

	hdma_write_sar_reg = readl(virt_hdma_write_sar + check_offset);
	hdma_write_dar_reg = readl(virt_hdma_write_dar + check_offset);
	hdma_read_sar_reg = readl(virt_hdma_read_sar + check_offset);
	hdma_read_dar_reg = readl(virt_hdma_read_dar + check_offset);

	dev_info(dev, "before start Write, offset=%x, sar = %x, dar = %x\n",
				check_offset, hdma_write_sar_reg, hdma_write_dar_reg);
	dev_info(dev, "before Read , offset=%x, sar = %x, dar = %x\n", check_offset,
				hdma_read_sar_reg, hdma_read_dar_reg);

	/* start write hdma */
	regmap_write(regmap_ep_hdma, hdma_write_offset + HDMA_DOORBELL_OFF_WRCH,
				HDMA_DB_START_WRCH);
	/* start read hdma */
	regmap_write(regmap_ep_hdma, hdma_read_offset + HDMA_DOORBELL_OFF_RDCH,
				HDMA_DB_START_RDCH);

	/* wait for hdma stop ,max time == 5s  */
	for (wait_times = 0; wait_times < MAX_TIME; wait_times++) {
		regmap_read(regmap_ep_hdma, HDMA_XFERSIZE_OFF_WRCH, &wr_reg);
		regmap_read(regmap_ep_hdma, HDMA_XFERSIZE_OFF_RDCH, &rd_reg);

		while ((wr_reg != HDMA_TRANSFER_OK) || (rd_reg != HDMA_TRANSFER_OK)) {
			regmap_read(regmap_ep_hdma, HDMA_XFERSIZE_OFF_WRCH, &wr_reg);
			regmap_read(regmap_ep_hdma, HDMA_XFERSIZE_OFF_RDCH, &rd_reg);
			if (wait_times >= MAX_TIME)
				return count;

			wait_times++;
			mdelay(MS_1000);
			continue;
		}
	}

	hdma_write_sar_reg = readl(virt_hdma_write_sar + check_offset);
	hdma_write_dar_reg = readl(virt_hdma_write_dar + check_offset);
	hdma_read_sar_reg = readl(virt_hdma_read_sar + check_offset);
	hdma_read_dar_reg = readl(virt_hdma_read_dar + check_offset);

	dev_info(dev, "after Write direction, offset=%x, sar = %x, dar = %x\n",
				check_offset, hdma_write_sar_reg, hdma_write_dar_reg);
	if (hdma_write_sar_reg != hdma_write_dar_reg) {
		dev_err(dev, "hdma write check fail!\n");
		BUG_ON(ret);
	}
	dev_info(dev, "after Read direction, offset=%x, sar = %x, dar = %x\n",
				check_offset, hdma_read_sar_reg, hdma_read_dar_reg);
	if (hdma_read_sar_reg != hdma_read_dar_reg) {
		dev_err(dev, "hdma read check fail!");
		BUG_ON(ret);
	}
	dev_info(dev, "hdma read and write check success!\n");

	return count;
}
static ssize_t aspm_function0_show(struct device *dev,
					struct device_attribute *attr, char *buf)
{
	dev_info(dev, "Set aspm function0\n");
	return SUCCESS;
}

static void switch_ep_function(u8 func_num)
{
	u32 reg_func;

	regmap_read(regmap_ep_ctrl, DBI_ACCESS_OFFSET, &reg_func);
	if (func_num == 0)
		reg_func &= ~(FUNC_MASK << 5);
	else
		reg_func |= func_num << FUNC_NUM_OFSET;
	/* switch the function num */
	regmap_write(regmap_ep_ctrl, DBI_ACCESS_OFFSET, reg_func);
}
static void set_ep_aspm_status(u8 aspm_enable)
{
	u32 reg_link_ctrl_sta;

	regmap_read(regmap_ep_dbi, LINK_CTRL_STATUS_REG, &reg_link_ctrl_sta);
	reg_link_ctrl_sta &= ~(ASPM_STATES_MASK);
	reg_link_ctrl_sta |= aspm_enable;
	regmap_write(regmap_ep_dbi, LINK_CTRL_STATUS_REG, reg_link_ctrl_sta);
}


static ssize_t aspm_function0_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	u32 aspm_enable;
	u8 ret;
	u8 func_num;

	func_num = FUNCTION0;

	ret = kstrtoint(buf, DECIMALISM, &aspm_enable);

	if (aspm_enable > 3) {
		dev_err(dev, "set aspm value error, Please enter the correct value\n");
		dev_err(dev, "0-No ASPM\n");
		dev_err(dev, "1-L0s supporedt\n");
		dev_err(dev, "2-L1 supported\n");
		dev_err(dev, "3-L0s and L1 supported\n");
	}

	/* switch the function 0 */
	switch_ep_function(func_num);
	/* set ep aspm status */
	set_ep_aspm_status(aspm_enable);

	return count;
}
static ssize_t aspm_function1_show(struct device *dev,
					struct device_attribute *attr, char *buf)
{
	dev_info(dev, "Set aspm function1\n");
	return SUCCESS;
}
static ssize_t aspm_function1_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	u32 aspm_enable;
	u8 ret;
	u8 func_num;

	func_num = FUNCTION1;

	ret = kstrtoint(buf, DECIMALISM, &aspm_enable);

	if (aspm_enable > 3) {
		dev_err(dev, "set aspm value error, Please enter the correct value\n");
		dev_err(dev, "0-No ASPM\n");
		dev_err(dev, "1-L0s supporedt\n");
		dev_err(dev, "2-L1 supported\n");
		dev_err(dev, "3-L0s and L1 supported\n");
	}

	/* switch the function 1 */
	switch_ep_function(func_num);
	/* set ep aspm status */
	set_ep_aspm_status(aspm_enable);

	/* switch the function 0 */
	func_num = FUNCTION0;
	switch_ep_function(func_num);

	return count;
}
static ssize_t aspm_function2_show(struct device *dev,
					struct device_attribute *attr, char *buf)
{
	dev_info(dev, "Set aspm function2\n");

	return SUCCESS;
}
static ssize_t aspm_function2_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	u32 aspm_enable;
	u8 ret;
	u8 func_num;

	func_num = FUNCTION2;

	ret = kstrtoint(buf, DECIMALISM, &aspm_enable);
	if (aspm_enable > 3) {
		dev_err(dev, "set aspm value error, Please enter the correct value\n");
		dev_err(dev, "0-No ASPM\n");
		dev_err(dev, "1-L0s supporedt\n");
		dev_err(dev, "2-L1 supported\n");
		dev_err(dev, "3-L0s and L1 supported\n");
	}
	/* switch the function 2 */
	switch_ep_function(func_num);
	/* set ep aspm status */
	set_ep_aspm_status(aspm_enable);

	/* switch the function 0 */
	func_num = FUNCTION0;
	switch_ep_function(func_num);

	return count;
}
static ssize_t aspm_function3_show(struct device *dev,
					struct device_attribute *attr, char *buf)
{
	dev_info(dev, "Set aspm function3\n");
	return SUCCESS;
}
static ssize_t aspm_function3_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	u32 aspm_enable;
	u8 ret;
	u8 func_num;

	func_num = FUNCTION3;

	ret = kstrtoint(buf, DECIMALISM, &aspm_enable);

	if (aspm_enable > 3) {
		dev_err(dev, "set aspm value error, Please enter the correct value\n");
		dev_err(dev, "0-No ASPM\n");
		dev_err(dev, "1-L0s supporedt\n");
		dev_err(dev, "2-L1 supported\n");
		dev_err(dev, "3-L0s and L1 supported\n");
	}

	/* switch the function 3 */
	switch_ep_function(func_num);
	/* set ep aspm status */
	set_ep_aspm_status(aspm_enable);

	/* switch the function 0 */
	func_num = FUNCTION0;
	switch_ep_function(func_num);

	return count;
}

static ssize_t reset_unreset_show(struct device *dev,
					struct device_attribute *attr, char *buf)
{
	dev_info(dev, "ep reset and unreset test node\n");
	return SUCCESS;
}
static ssize_t reset_unreset_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	u32 reg;

	regmap_write(regmap_ep_ctrl, EP_RESET_OFFSET, 0);/* reset ep*/
	regmap_write(regmap_ep_ctrl, EP_RESET_OFFSET, 2);/* unreset ep */
	regmap_read(regmap_ep_dbi, MSI_CAP_REG + PCI_MSI_DATA_32, &reg);
	if (reg == RESET_VALUE)
		dev_info(dev, "reset and unreset case success, reg value = %x\n", reg);
	else
		dev_info(dev, "reset and unreset case fail, reg value = %x\n", reg);
	regmap_write(regmap_ep_dbi, MSI_CAP_REG + PCI_MSI_DATA_32, TEST_VALUE);
	regmap_read(regmap_ep_dbi, MSI_CAP_REG + PCI_MSI_DATA_32, &reg);
	dev_info(dev, "write value %x\n", TEST_VALUE);

	return count;
}
static ssize_t link_up_pressure_show(struct device *dev,
					struct device_attribute *attr, char *buf)
{
	dev_info(dev, "link up pressure node\n");

	return SUCCESS;
}
static ssize_t link_up_pressure_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	u32 ret;
	u32 link_status;
	u32 link_status_reg;

	ret = kstrtoint(buf, DECIMALISM, &link_status);
	if (link_status >= 2)
		dev_err(dev, "set link status value error, Please enter the correct value(0,1)\n");

	/* enable ep ltssm */
	regmap_read(regmap_ep_ctrl, LTSSM_EN_OFFSET, &link_status_reg);
	if (link_status == 0)
		link_status_reg &= ~(LINK_UP_MASK);
	else
		link_status_reg |= link_status;

	regmap_write(regmap_ep_ctrl, LTSSM_EN_OFFSET, link_status_reg);

	return count;
}

static ssize_t power_functions_show(struct device *dev,
					struct device_attribute *attr, char *buf)
{
	dev_info(dev, "pcie modem/wifi power functions\n");
	dev_info(dev, "1-modem power up and link up");
	dev_info(dev, "2-modem power down");
	dev_info(dev, "3-wifi power up and link up");
	dev_info(dev, "4-wifi power down");

	return SUCCESS;
}


static ssize_t power_functions_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	u32 command;
	u32 ret;

	ret = kstrtoint(buf, DECIMALISM, &command);
	if (command >= 4) {
		dev_err(dev, "err:\n");
		dev_info(dev, "pcie modem/wifi power functions\n");
		dev_info(dev, "1-modem power up and link up");
		dev_info(dev, "2-modem power down");
		dev_info(dev, "3-wifi power up and link up");
		dev_info(dev, "4-wifi power down");
	}

	switch (command) {
	case 1: {
		modem_power_up();
		break;
	}
	case 2: {
		modem_power_down();
		break;
	}
	case 3: {
		wifi_power_up();
		break;
	}
	case 4: {
		wifi_power_down();
		break;
	}
	}
	return count;
}

static ssize_t set_port_num_show(struct device *dev,
					struct device_attribute *attr, char *buf)
{
	dev_info(dev, "pcie set port num 0/1\n");
	dev_info(dev, "current port_num:%d\n", port_num);

	return SUCCESS;
}

static ssize_t set_port_num_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	u32 command;
	u32 ret;

	ret = kstrtoint(buf, DECIMALISM, &command);
	if (command > 1) {
		dev_err(dev, "err: no such port num\n");
		return count;
	}

	port_num = command;

	return count;
}

static DEVICE_ATTR_RW(all_function);
static DEVICE_ATTR_RW(msi_irq_num);
static DEVICE_ATTR_RW(msix_irq_num);
static DEVICE_ATTR_RW(hdma_index);
static DEVICE_ATTR_RW(set_ep_lane);
static DEVICE_ATTR_RW(hdma_pressure_start);
static DEVICE_ATTR_RW(aspm_function0);
static DEVICE_ATTR_RW(aspm_function1);
static DEVICE_ATTR_RW(aspm_function2);
static DEVICE_ATTR_RW(aspm_function3);
static DEVICE_ATTR_RW(reset_unreset);
static DEVICE_ATTR_RW(link_up_pressure);
static DEVICE_ATTR_RW(power_functions);
static DEVICE_ATTR_RW(set_port_num);

static struct attribute *sys_device_attributes[] = {
	&dev_attr_all_function.attr,
	&dev_attr_msi_irq_num.attr,
	&dev_attr_msix_irq_num.attr,
	&dev_attr_hdma_index.attr,
	&dev_attr_set_ep_lane.attr,
	&dev_attr_hdma_pressure_start.attr,
	&dev_attr_aspm_function0.attr,
	&dev_attr_aspm_function1.attr,
	&dev_attr_aspm_function2.attr,
	&dev_attr_aspm_function3.attr,
	&dev_attr_reset_unreset.attr,
	&dev_attr_link_up_pressure.attr,
	&dev_attr_power_functions.attr,
	&dev_attr_set_port_num.attr,
	NULL
};
static const struct attribute_group sys_device_attr_group = {
	.attrs = sys_device_attributes,
};

static ssize_t remove_by_port_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (port_num == 0)
		xring_pcie_remove_port_by_port(1);
	else if (port_num == 1)
		xring_pcie_remove_port_by_port(0);
	else
		dev_err(dev, "Invalid port num\n");

	return 0;
}

static DEVICE_ATTR_RO(remove_by_port);

static ssize_t remove_by_name_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (port_num == 0)
		xring_pcie_remove_port_by_name("wifi");
	else if (port_num == 1)
		xring_pcie_remove_port_by_name("modem");
	else
		dev_err(dev, "Invalid port num\n");

	return 0;
}

static DEVICE_ATTR_RO(remove_by_name);

static ssize_t probe_by_port_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (port_num == 0)
		xring_pcie_probe_port_by_port(1);
	else if (port_num == 1)
		xring_pcie_probe_port_by_port(0);
	else
		dev_err(dev, "Invalid port num\n");

	return 0;
}

static DEVICE_ATTR_RO(probe_by_port);

static ssize_t probe_by_name_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (port_num == 0)
		xring_pcie_probe_port_by_name("wifi");
	else if (port_num == 1)
		xring_pcie_probe_port_by_name("modem");
	else
		dev_err(dev, "Invalid port num\n");

	return 0;
}

static DEVICE_ATTR_RO(probe_by_name);

static void pcie_test_create_sysfs(struct device *dev)
{
	if (device_create_file(dev, &dev_attr_probe_by_name) < 0)
		dev_err(dev, "Failed to create file probe_by_name\n");

	if (device_create_file(dev, &dev_attr_probe_by_port) < 0)
		dev_err(dev, "Failed to create file probe_by_port\n");

	if (device_create_file(dev, &dev_attr_remove_by_name) < 0)
		dev_err(dev, "Failed to create file remove_by_name\n");

	if (device_create_file(dev, &dev_attr_remove_by_port) < 0)
		dev_err(dev, "Failed to create file remove_by_port\n");
}

static void pcie_test_remove_sysfs(struct device *dev)
{
	device_remove_file(dev, &dev_attr_probe_by_name);
	device_remove_file(dev, &dev_attr_probe_by_port);
	device_remove_file(dev, &dev_attr_remove_by_name);
	device_remove_file(dev, &dev_attr_remove_by_port);
}

static int xring_pcie_gpio_request(struct device *dev, const char *label)
{
	int pin;

	pin = of_get_named_gpio(dev->of_node, label, 0);
	if (pin < 0) {
		dev_err(dev, "The GPIO <%s> is probed fail, %d\n", label, pin);
		return -1;
	}

	dev_info(dev, "Request GPIO <%s:%d> success\n", label, pin);
	return pin;
}

static void pcie_clk_disable(void)
{
	clk_disable_unprepare(clk->clk_pcie_aux);
	clk_disable_unprepare(clk->aclk_pcie_axi_mst);
	clk_disable_unprepare(clk->aclk_hss2_tbu);
	clk_disable_unprepare(clk->aclk_pcie_axi_slv);
	clk_disable_unprepare(clk->pclk_pcie_apb);
	clk_disable_unprepare(clk->clk_tcu_tpc);
	clk_disable_unprepare(clk->aclk_hss2_tcu);
	clk_disable_unprepare(clk->pclk_hss2_tcu_slv);
	pcie_clk_is_enable = false;
}

static int xring_pcie_sysfs_probe(struct platform_device *pdev)
{
	struct device_node *np;
	int ret;

	dev = &pdev->dev;
	np = dev->of_node;

	clk = devm_kzalloc(dev, sizeof(*clk), GFP_KERNEL);
	pcie_clk_is_enable = false;

	is_fpga = of_property_read_bool(dev->of_node, "fpga");
	regmap_ep_dbi = syscon_regmap_lookup_by_phandle(dev->of_node, "hss_syscon_ep_dbi");
	regmap_ep_slv = syscon_regmap_lookup_by_phandle(dev->of_node, "hss_syscon_ep_slv");
	regmap_ep_ctrl = syscon_regmap_lookup_by_phandle(dev->of_node, "hss_syscon_ep_ctrl");
	regmap_ep_atu = syscon_regmap_lookup_by_phandle(dev->of_node, "hss_syscon_ep_atu");
	regmap_ep_hdma = syscon_regmap_lookup_by_phandle(dev->of_node, "hss_syscon_ep_hdma");
	regmap_pcie_phy = syscon_regmap_lookup_by_phandle(dev->of_node, "pcie_ep_phy");

	regmap_hss2_crg = syscon_regmap_lookup_by_phandle(dev->of_node, "hss2-crg-syscon");
	regmap_hss2_top = syscon_regmap_lookup_by_phandle(dev->of_node,
					"hss2-top-syscon");

	mdm_gpio.pmic_en = xring_pcie_gpio_request(dev, "modem-pmic-en");
	mdm_gpio.pmic_rst = xring_pcie_gpio_request(dev, "modem-pmic-rst");

	wifi_gpio.pmic_en = xring_pcie_gpio_request(dev, "wf-pmic-en");
	wifi_gpio.pmic_rst = xring_pcie_gpio_request(dev, "wf-pmic-rst");

	ret = of_property_read_u32(np, "max-link-speed", &target_speed);
	if (ret)
		dev_err(dev, "failed to get max link speed\n");

	ret = of_property_read_u32(np, "slv-addr", &ep_slv_base);
	if (ret)
		dev_err(dev, "failed to get slv base\n");
	else
		dev_info(dev, "ep slv base:0x%x\n", ep_slv_base);

	port_num = of_get_pci_domain_nr(dev->of_node);

	if (IS_ERR(regmap_ep_dbi)) {
		dev_err(dev, "unable to find dbi syscon registers\n");
		return PTR_ERR(regmap_ep_dbi);
	}
	if (IS_ERR(regmap_ep_slv)) {
		dev_err(dev, "unable to find slv syscon registers\n");
		return PTR_ERR(regmap_ep_slv);
	}
	if (IS_ERR(regmap_ep_ctrl)) {
		dev_err(dev, "unable to find ctrl syscon registers\n");
		return PTR_ERR(regmap_ep_ctrl);
	}
	if (IS_ERR(regmap_ep_atu)) {
		dev_err(dev, "unable to find atu syscon registers\n");
		return PTR_ERR(regmap_ep_atu);
	}
	if (IS_ERR(regmap_ep_hdma)) {
		dev_err(dev, "unable to find hdma syscon registers\n");
		return PTR_ERR(regmap_ep_hdma);
	}
	if (sysfs_create_group(&(dev->kobj), &sys_device_attr_group)) {
		dev_err(dev, "Failed to sysfs create group\n");
		return FAIL;
	}

	pcie_test_create_sysfs(dev);

	dev_info(dev, "alloc three SZ_4K addr for bar...\n");

	bar0_virt_hdma = dma_alloc_coherent(dev, SZ_4K, &bar0_inbound_tar, GFP_KERNEL);
	bar2_virt_hdma = dma_alloc_coherent(dev, SZ_4K, &bar2_inbound_tar, GFP_KERNEL);
	bar3_virt_hdma = dma_alloc_coherent(dev, SZ_4K, &bar3_inbound_tar, GFP_KERNEL);
	if ((!bar0_virt_hdma) || (!bar2_virt_hdma) || (!bar3_virt_hdma))
		dev_err(dev, "Failed to alloc addr for bar\n");

	dev_info(dev, "alloc addr for hdma write...\n");
	virt_hdma_write_sar = dma_alloc_coherent(dev, HDMA_SIZE, &write_sar_phy, GFP_KERNEL);
	virt_hdma_write_dar = dma_alloc_coherent(dev, HDMA_SIZE, &write_dar_phy, GFP_KERNEL);
	if (!virt_hdma_write_sar || !virt_hdma_write_dar) {
		dev_err(dev, "failed to alloc 16M sar or dar for hdma write\n");
		goto err_hdma_wr;
	}

	dev_info(dev, "alloc addr for hdma read...\n");
	virt_hdma_read_sar = dma_alloc_coherent(dev, HDMA_SIZE, &read_sar_phy, GFP_KERNEL);
	virt_hdma_read_dar = dma_alloc_coherent(dev, HDMA_SIZE, &read_dar_phy, GFP_KERNEL);
	if (!virt_hdma_read_sar || !virt_hdma_read_dar) {
		dev_err(dev, "failed to alloc 16M sar or dar for hdma read\n");
		goto err_hdma_rd;
	}

	dev_info(dev, "alloc dma done.\n bar0_tar:0x%llx, bar2_tar:0x%llx, bar3_tar:0x%llx\n",
			bar0_inbound_tar, bar2_inbound_tar, bar3_inbound_tar);
	dev_info(dev, "hdma write sar:0x%llx, hdma write dar:0x%llx, hdma read sar:0x%llx, hdma read dar:0x%llx\n",
			write_sar_phy, write_dar_phy, read_sar_phy, read_dar_phy);

	return 0;

err_hdma_rd:
	dma_free_coherent(dev, HDMA_SIZE, virt_hdma_write_sar, write_sar_phy);
	dma_free_coherent(dev, HDMA_SIZE, virt_hdma_write_dar, write_dar_phy);
err_hdma_wr:
	dma_free_coherent(dev, SZ_64K, bar0_virt_hdma, bar0_inbound_tar);
	dma_free_coherent(dev, SZ_64K, bar2_virt_hdma, bar2_inbound_tar);
	dma_free_coherent(dev, SZ_64K, bar3_virt_hdma, bar3_inbound_tar);
	return 0;
}

static int xring_pcie_sysfs_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&(dev->kobj), &sys_device_attr_group);
	pcie_test_remove_sysfs(&pdev->dev);

	dma_free_coherent(dev, SZ_64K, bar0_virt_hdma, bar0_inbound_tar);
	dma_free_coherent(dev, SZ_64K, bar2_virt_hdma, bar2_inbound_tar);
	dma_free_coherent(dev, SZ_64K, bar3_virt_hdma, bar3_inbound_tar);

	dma_free_coherent(dev, HDMA_SIZE, virt_hdma_read_sar, read_sar_phy);
	dma_free_coherent(dev, HDMA_SIZE, virt_hdma_read_dar, read_dar_phy);
	dma_free_coherent(dev, HDMA_SIZE, virt_hdma_write_sar, write_sar_phy);
	dma_free_coherent(dev, HDMA_SIZE, virt_hdma_write_dar, write_dar_phy);

	if (pcie_clk_is_enable)
		pcie_clk_disable();

	xring_pcie_powerup_rst_assert(port_num);

	return SUCCESS;
}

static const struct of_device_id xring_pcie_sysfs_of_match[] = {
	{ .compatible = "xring,pcie-test-pcie0" },
	{ .compatible = "xring,pcie-test-pcie1" },
	{},
};

static struct platform_driver xring_pcie_sysfs_driver = {
	.driver = {
		.name	= "xring-pcie_sysfs_test",
		.of_match_table = xring_pcie_sysfs_of_match,
	},
	.probe = xring_pcie_sysfs_probe,
	.remove = xring_pcie_sysfs_remove,
};
module_platform_driver(xring_pcie_sysfs_driver);
MODULE_LICENSE("GPL v2");
