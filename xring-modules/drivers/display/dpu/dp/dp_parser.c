// SPDX-License-Identifier: GPL-2.0
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

#include <linux/slab.h>
#include <linux/of_irq.h>
#include "dp_display.h"
#include "dp_link.h"
#include "dp_parser.h"
#include "dpu_log.h"

#define DP_IOMEM_UNMAP(iomem) \
	do { \
		if (iomem) { \
			iounmap(iomem); \
			iomem = NULL; \
		} \
	} while (0)

static int dp_parser_blk_info_get(struct platform_device *pdev,
		const char *name, struct dp_blk_cap *blk_cap)
{
	void __iomem *iomem_ptr = NULL;
	struct resource *res;
	unsigned long size;

	if (!pdev || !name || !blk_cap) {
		DP_ERROR("invalid parameters: %pK, %s, %pK\n", pdev, name, blk_cap);
		return -EINVAL;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, name);
	if (!res) {
		DP_ERROR("failed to get memory resource: %s\n", name);
		return -EINVAL;
	}

	size = resource_size(res);

	iomem_ptr = ioremap(res->start, size);
	if (!iomem_ptr) {
		DP_ERROR("failed to remap blk: %s\n", name);
		return -ENOMEM;
	}

	DP_DEBUG("%s: 0x%llx, %lu\n", name, res->start, size);

	snprintf(blk_cap->name, BLK_NAME_LEN, "%s", name);
	blk_cap->paddr = res->start;
	blk_cap->len = size;
	blk_cap->vaddr = iomem_ptr;

	return 0;
}

static void dp_parser_blk_info_release(struct dp_parser *parser)
{
	DP_IOMEM_UNMAP(parser->ctrl_cap.vaddr);
	DP_IOMEM_UNMAP(parser->sctrl_cap.vaddr);
	DP_IOMEM_UNMAP(parser->tmg_cap.vaddr);
	DP_IOMEM_UNMAP(parser->msgbus_cap.vaddr);
}

static int dp_parser_blk_info_parse(struct platform_device *pdev,
		struct dp_parser *parser)
{
	int ret;

	ret = dp_parser_blk_info_get(pdev, "ctrl", &parser->ctrl_cap);
	if (ret) {
		DP_ERROR("failed get ctrl info\n");
		goto error;
	}

	ret = dp_parser_blk_info_get(pdev, "sctrl", &parser->sctrl_cap);
	if (ret) {
		DP_ERROR("failed get sctrl info\n");
		goto error;
	}

	ret = dp_parser_blk_info_get(pdev, "tmg", &parser->tmg_cap);
	if (ret) {
		DP_ERROR("failed get tmg info\n");
		goto error;
	}

	ret = dp_parser_blk_info_get(pdev, "msgbus", &parser->msgbus_cap);
	if (ret) {
		DP_ERROR("failed get msgbus info\n");
		goto error;
	}

	return 0;
error:
	dp_parser_blk_info_release(parser);
	return ret;
}

static void dp_parser_signal_table_parse(struct platform_device *pdev)
{
	u32 link_rate, swing_level, preemp_level, pre, main, post;
	const char *signal_table_name = "signal-table";
	int element_count = 0, tuple_count = 0;
	const u32 tuple_size = 6;
	int ret = 0;
	u32 i;

	element_count = of_property_count_u32_elems(pdev->dev.of_node, signal_table_name);
	if (element_count < 0 || element_count % tuple_size != 0) {
		DP_INFO("no valid signal-table in dt, %d\n", element_count);
		return;
	}

	tuple_count = element_count / tuple_size;

	for (i = 0; i < tuple_count; i++) {
		ret = of_property_read_u32_index(pdev->dev.of_node,
				signal_table_name, i * tuple_size, &link_rate);
		if (ret) {
			DP_ERROR("failed to parse link_rate, index %d, ret %d\n", i * tuple_size, ret);
			return;
		}
		if (link_rate > DP_LINK_RATE_MAX) {
			DP_ERROR("invalid link_rate %d\n", link_rate);
			return;
		}

		ret = of_property_read_u32_index(pdev->dev.of_node,
				signal_table_name, i * tuple_size + 1, &swing_level);
		if (ret) {
			DP_ERROR("failed to parse swing_level, index %d, ret %d\n", i * tuple_size + 1, ret);
			return;
		}
		if (swing_level > MAX_VOLTAGE_SWING_LEVEL) {
			DP_ERROR("invalid swing_level %d\n", swing_level);
			return;
		}

		ret = of_property_read_u32_index(pdev->dev.of_node,
				signal_table_name, i * tuple_size + 2, &preemp_level);
		if (ret) {
			DP_ERROR("failed to parse preemp_level, index %d, ret %d\n", i * tuple_size + 2, ret);
			return;
		}
		if (preemp_level > MAX_PRE_EMPHASIS_LEVEL) {
			DP_ERROR("invalid preemp_level %d\n", preemp_level);
			return;
		}
		if (swing_level + preemp_level > MAX_VOLTAGE_SWING_LEVEL) {
			DP_ERROR("invalid swing_level %d and preemp_level %d combination\n",
					swing_level, preemp_level);
			return;
		}

		ret = of_property_read_u32_index(pdev->dev.of_node,
				signal_table_name, i * tuple_size + 3, &pre);
		if (ret) {
			DP_ERROR("failed to parse pre, index %d, ret %d\n", i * tuple_size + 3, ret);
			return;
		}

		ret = of_property_read_u32_index(pdev->dev.of_node,
				signal_table_name, i * tuple_size + 4, &main);
		if (ret) {
			DP_ERROR("failed to parse main, index %d, ret %d\n", i * tuple_size + 4, ret);
			return;
		}

		ret = of_property_read_u32_index(pdev->dev.of_node,
				signal_table_name, i * tuple_size + 5, &post);
		if (ret) {
			DP_ERROR("failed to parse post, index %d, ret %d\n", i * tuple_size + 5, ret);
			return;
		}

		dp_link_set_default_signal_table(link_rate, swing_level, preemp_level, pre, main, post);
	}
}

static void dp_parser_phy_param_parse(struct platform_device *pdev)
{
	u32 init_link_rate, init_swing_level, init_preemp_level;
	int ret = 0;

	ret = of_property_read_u32(pdev->dev.of_node, "max-link-rate", &init_link_rate);
	if (ret == 0)
		dp_link_set_default_param("init_link_rate", &init_link_rate);
	else
		DP_INFO("no max-link-rate in dt, ret %d\n", ret);

	ret = of_property_read_u32(pdev->dev.of_node, "init-swing-level", &init_swing_level);
	if (ret == 0)
		dp_link_set_default_param("init_swing_level", &init_swing_level);
	else
		DP_INFO("no init-swing-level in dt, ret %d\n", ret);

	ret = of_property_read_u32(pdev->dev.of_node, "init-preemp-level", &init_preemp_level);
	if (ret == 0)
		dp_link_set_default_param("init_preemp_level", &init_preemp_level);
	else
		DP_INFO("no init-preemp-level in dt, ret %d\n", ret);

	dp_parser_signal_table_parse(pdev);
}

int dp_parser_init(struct platform_device *pdev, struct dp_parser **parser)
{
	static u8 default_index;
	struct dp_parser *dp_parser;
	int ret;

	dp_parser = kzalloc(sizeof(*dp_parser), GFP_KERNEL);
	if (!dp_parser)
		return -ENOMEM;

	ret = of_property_read_u32(pdev->dev.of_node, "cell-index",
			&dp_parser->index);
	if (ret)
		dp_parser->index = default_index++;

	if (dp_parser->index >= MAX_DP_DISPLAY_NUM) {
		DP_ERROR("wrong dp index %d\n", dp_parser->index);
		return -EINVAL;
	}

	ret = of_irq_get_byname(pdev->dev.of_node, "dptx_irq");
	if (ret <= 0) {
		DP_ERROR("failed to get dptx irq, ret %d\n", ret);
		return ret;
	}

	dp_parser->dptx_irq = ret;

	ret = dp_parser_blk_info_parse(pdev, dp_parser);
	if (ret < 0) {
		DP_ERROR("failed to parse dp blk info, ret %d\n", ret);
		return ret;
	}

	dp_parser_phy_param_parse(pdev);

	*parser = dp_parser;

	return 0;
}

void dp_parser_deinit(struct dp_parser *parser)
{
	dp_parser_blk_info_release(parser);
	kfree(parser);
}
