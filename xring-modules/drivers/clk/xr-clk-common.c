// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "xr-clk-common.h"
#include "clk.h"
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <soc/xring/vote_mng.h>
#include <dt-bindings/xring/platform-specific/ip_regulator_define.h>
#include <dt-bindings/xring/platform-specific/common/clk/include/clk_resource.h>

#define DTS_REG_ATTRIBUTE_NUM	4
#define PLAT_FPGA	1
#define PCIE_SPEED_GEN4	4

DEFINE_SPINLOCK(g_xr_clk_lock);

static int g_plat_fpga;
static bool g_peri_dvfs_lpdc_ft;
static bool g_media_dvfs_lpdc_ft;
static bool g_peri_avs_lpdc_ft;
static bool g_media_avs_lpdc_ft;

static int g_pciepll_vco_rate = VCO_RATE_NO_CARE;

struct clk_votemng {
	unsigned int crg_id;
	unsigned int vote_id;
	struct device *dev;
	struct vote_mng *votemng;
};

struct clk_base {
	unsigned int id;
	void __iomem *base;
};

static struct clk_base base_desc[XR_CLK_MAX_BASECRG] = {
	[XR_CLK_CRGCTRL]      = { XR_CLK_CRGCTRL, 0 },
	[XR_CLK_MEDIA1CRG]    = { XR_CLK_MEDIA1CRG, 0 },
	[XR_CLK_MEDIA2CRG]    = { XR_CLK_MEDIA2CRG, 0 },
	[XR_CLK_HSIF1CRG]     = { XR_CLK_HSIF1CRG, 0 },
	[XR_CLK_HSIF2CRG]     = { XR_CLK_HSIF2CRG, 0 },
	[XR_CLK_LMSCRG]       = { XR_CLK_LMSCRG, 0 },
	[XR_CLK_LPISCRG]      = { XR_CLK_LPISCRG, 0 },
	[XR_CLK_LPISACTRLCRG] = { XR_CLK_LPISACTRLCRG, 0 },
	[XR_CLK_DVSPERI]      = { XR_CLK_DVSPERI, 0 },
	[XR_CLK_DVSMEDIA]     = { XR_CLK_DVSMEDIA, 0 },
};

static struct clk_votemng g_crg_votemng[] = {
	[XR_CLK_MEDIA1CRG] = {XR_CLK_MEDIA1CRG, MEDIA1_SUBSYS_ID, NULL, NULL},
	[XR_CLK_MEDIA2CRG] = {XR_CLK_MEDIA2CRG, MEDIA2_SUBSYS_ID, NULL, NULL},
};

void __iomem *get_xr_clk_base(unsigned int id)
{
	if (id >= XR_CLK_MAX_BASECRG) {
		clkerr("invalid base id %u\n", id);
		return NULL;
	}
	return base_desc[id].base;
}
EXPORT_SYMBOL_GPL(get_xr_clk_base);

int is_fpga(void)
{
	return g_plat_fpga;
}
EXPORT_SYMBOL_GPL(is_fpga);

bool is_peri_dvfs_ft_on(void)
{
	return (g_peri_dvfs_lpdc_ft && IS_ENABLED(CONFIG_XRING_CLK_PERIDVFS));
}
EXPORT_SYMBOL_GPL(is_peri_dvfs_ft_on);

bool is_media_dvfs_ft_on(void)
{
	return (g_media_dvfs_lpdc_ft && IS_ENABLED(CONFIG_XRING_CLK_MEDIADVFS));
}
EXPORT_SYMBOL_GPL(is_media_dvfs_ft_on);

bool is_peri_avs_ft_on(void)
{
	return (g_peri_avs_lpdc_ft && is_peri_dvfs_ft_on());
}
EXPORT_SYMBOL_GPL(is_peri_avs_ft_on);

bool is_media_avs_ft_on(void)
{
	return (g_media_avs_lpdc_ft && is_media_dvfs_ft_on());
}
EXPORT_SYMBOL_GPL(is_media_avs_ft_on);

spinlock_t *get_xr_clk_lock(void)
{
	return &g_xr_clk_lock;
}
EXPORT_SYMBOL_GPL(get_xr_clk_lock);

static struct clk_hw_onecell_data *g_xr_clk_hw_data;

struct clk_hw_onecell_data **xr_clk_hw_data_get(void)
{
	return &g_xr_clk_hw_data;
}
EXPORT_SYMBOL_GPL(xr_clk_hw_data_get);

int crg_base_address_init(struct platform_device *pdev, unsigned long type)
{
	unsigned int reg[DTS_REG_ATTRIBUTE_NUM];
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;

	if (of_property_read_u32_array(np, "reg", reg, DTS_REG_ATTRIBUTE_NUM)) {
		clkerr("get dts reg attribute info failed!\n");
		return -ENOENT;
	}

	base_desc[type].base = devm_ioremap(dev, reg[1], reg[3]);
	if (IS_ERR(base_desc[type].base)) {
		clkerr("clk %lu ioremap error!\n", type);
		return PTR_ERR(base_desc[type].base);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(crg_base_address_init);

int xr_check_clk_hws(struct clk_hw *clks[], unsigned int count)
{
	unsigned int i;

	for (i = 0; i < count; i++)
		if (IS_ERR(clks[i])) {
			clkerr("i.XRing clk %u: register failed with %ld\n",
			       i, PTR_ERR(clks[i]));
			return -EINVAL;
		}

	return 0;
}
EXPORT_SYMBOL_GPL(xr_check_clk_hws);

void xr_unregister_hw_clocks(struct clk_hw *hws[], unsigned int count)
{
	unsigned int i;

	for (i = 0; i < count; i++) {
		if (!IS_ERR_OR_NULL(hws[i]))
			clk_hw_unregister(hws[i]);
	}
}
EXPORT_SYMBOL_GPL(xr_unregister_hw_clocks);

struct clk_hw *xr_clk_hw_get(unsigned int clk_id)
{
	struct clk_hw_onecell_data *hw_data = g_xr_clk_hw_data;

	if (!hw_data) {
		clkerr("g_xr_clk_hw_data do not init!\n");
		return NULL;
	}

	if (clk_id >= hw_data->num) {
		clkerr("clk id err %u\n", clk_id);
		return NULL;
	}

	return hw_data->hws[clk_id];
}
EXPORT_SYMBOL_GPL(xr_clk_hw_get);

void clk_feature_state_init(void)
{
	struct device_node *plat_np = NULL;

	/* if fpga runs, sw_ack and pll lock check feature will be shut down */
	plat_np = of_find_node_by_name(NULL, "fpga-dumyclk");
	if (plat_np)
		g_plat_fpga = PLAT_FPGA;

	plat_np = of_find_node_by_name(NULL, "peri_dvfs_ctrl");
	if (plat_np && of_device_is_available(plat_np))
		g_peri_dvfs_lpdc_ft = true;

	plat_np = of_find_node_by_name(NULL, "media_dvfs_ctrl");
	if (plat_np && of_device_is_available(plat_np))
		g_media_dvfs_lpdc_ft = true;

	plat_np = of_find_node_by_name(NULL, "peri_avs_ctrl");
	if (plat_np && of_device_is_available(plat_np))
		g_peri_avs_lpdc_ft = true;

	plat_np = of_find_node_by_name(NULL, "media_avs_ctrl");
	if (plat_np && of_device_is_available(plat_np))
		g_media_avs_lpdc_ft = true;

}
EXPORT_SYMBOL_GPL(clk_feature_state_init);

int crg_vote_media_power(int crg_idx, bool on_off)
{
	struct device *dev = NULL;
	struct vote_mng *votemng = NULL;
	uint32_t value[4] = {0xff, 0xff, 0xff, 0xff};
	int req_power_stat;
	int req_vote_id;
	int sync = 0;
	int ret;

	if (on_off) {
		req_power_stat = MEDIA_POWER_ON;
		sync = VOTE_MNG_MSG_SYNC;
	} else {
		req_power_stat = MEDIA_POWER_OFF;
		sync = VOTE_MNG_MSG_ASYNC;
	}

	req_vote_id = g_crg_votemng[crg_idx].vote_id;
	dev = g_crg_votemng[crg_idx].dev;
	votemng = g_crg_votemng[crg_idx].votemng;
	if (IS_ERR_OR_NULL(dev) || IS_ERR_OR_NULL(votemng)) {
		clkerr("media votemng not init!\n");
		ret = -ENOMEM;
		goto out;
	}

	/* req_vote_id is defined by regulator, the format of value[x] ought to
	 * match media power ctrl handle.
	 */
	value[0] = req_vote_id;
	value[1] = req_power_stat;
	ret = vote_mng_msg_send(votemng, value, ARRAY_SIZE(value), sync);
	if (ret) {
		clkerr("clk vote media%d subsys %s failed!\n",
			((req_vote_id == XR_CLK_MEDIA1CRG) ? 1 : 2),
			(on_off ? "on" : "off"));
		ret = -EAGAIN;
	}

	if ((sync == VOTE_MNG_MSG_SYNC) && value[2]) {
		clkerr("clk vote media%d subsys %s wait ack fail, value[2]=%u\n",
			((req_vote_id == XR_CLK_MEDIA1CRG) ? 1 : 2),
			(on_off ? "on" : "off"), value[2]);
		ret = -EAGAIN;
	}

out:
	return ret;
}
EXPORT_SYMBOL_GPL(crg_vote_media_power);


/* return：0-MEDIA_POWER_OFF, 1-MEDIA_POWER_ON,
 * other value means a error happened.
 */
int get_media_power_status(int crg_idx)
{
	struct device *dev = NULL;
	struct vote_mng *votemng = NULL;
	uint32_t value[4] = {0xff, 0xff, 0xff, 0xff};
	int ret = 0;

	dev = g_crg_votemng[crg_idx].dev;
	votemng = g_crg_votemng[crg_idx].votemng;
	if (IS_ERR_OR_NULL(dev) || IS_ERR_OR_NULL(votemng)) {
		clkerr("media votemng not init!\n");
		ret = -ENOMEM;
		goto out1;
	}

	value[0] = g_crg_votemng[crg_idx].vote_id;
	value[1] = MEDIA_POWER_GET_STATE;
	ret = vote_mng_msg_send(votemng, value, ARRAY_SIZE(value), VOTE_MNG_MSG_SYNC);
	if (ret) {
		clkerr("clk get media%d subsys power state failed!\n",
			((crg_idx == XR_CLK_MEDIA1CRG) ? 1 : 2));
		ret = -EAGAIN;
		goto out1;
	}

	ret = value[2];

out1:
	return ret;

}
EXPORT_SYMBOL_GPL(get_media_power_status);

void media_power_votemng_init(struct device *dev, int crg_idx)
{
	struct device_node *np = dev->of_node;
	struct vote_mng *votemng = NULL;
	const char *votemng_name = NULL;
	int ret = 0;

	if ((crg_idx != XR_CLK_MEDIA1CRG) && (crg_idx != XR_CLK_MEDIA2CRG))
		return;

	ret = of_property_read_string(np, "vote-mng-names", &votemng_name);
	if (ret) {
		clkerr("parse media%d dts votemng name failed!\n",
			((crg_idx == XR_CLK_MEDIA1CRG) ? 1 : 2));
		return;
	}

	votemng = vote_mng_get(dev, votemng_name);
	if (IS_ERR_OR_NULL(votemng))
		clkerr("get media%d vote_mng failed!\n",
				((crg_idx == XR_CLK_MEDIA1CRG) ? 1 : 2));

	g_crg_votemng[crg_idx].dev = dev;
	g_crg_votemng[crg_idx].votemng = votemng;
}
EXPORT_SYMBOL_GPL(media_power_votemng_init);

void media_power_votemng_exit(void)
{
	int crg_idx;
	int num = ARRAY_SIZE(g_crg_votemng);

	for (crg_idx = 0; crg_idx < num; crg_idx++) {
		if (!IS_ERR_OR_NULL(g_crg_votemng[crg_idx].votemng)) {
			vote_mng_put(g_crg_votemng[crg_idx].dev, g_crg_votemng[crg_idx].votemng);
			g_crg_votemng[crg_idx].votemng = NULL;
		}
	}
}
EXPORT_SYMBOL_GPL(media_power_votemng_exit);

void parse_pciepll_vco_rate(const char * const *compt_string, int num)
{
	struct device_node *np;
	unsigned int max_speed;
	char *pcie_dev_status;
	int i;
	int ret = 0;

	for (i = 0; i < num; i++) {
		np = of_find_compatible_node(NULL, NULL, compt_string[i]);
		if (IS_ERR_OR_NULL(np))
			continue;
		/* if pcie_device not enabled, ignore its speed */
		pcie_dev_status = (char *)of_get_property(np, "status", 0);
		if (strcmp(pcie_dev_status, "okay") != 0)
			continue;
		/* get pcie device max speed requirement */
		ret = of_property_read_u32(np, "max-link-speed", &max_speed);
		if (ret)
			continue;
		if (max_speed == PCIE_SPEED_GEN4) {
			g_pciepll_vco_rate = VCO_RATE_4D8G;
			return;
		}
	}
	g_pciepll_vco_rate = VCO_RATE_2D4G;
}
EXPORT_SYMBOL_GPL(parse_pciepll_vco_rate);

int get_pciepll_vco_rate(void)
{
	return g_pciepll_vco_rate;
}
EXPORT_SYMBOL_GPL(get_pciepll_vco_rate);
