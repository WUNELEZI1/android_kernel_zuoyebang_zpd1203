// SPDX-License-Identifier: GPL-2.0
/****************************************************************************
 *
 *    The MIT License (MIT)
 *
 *    Copyright (C) 2020  VeriSilicon Microelectronics Co., Ltd.
 *
 *    Permission is hereby granted, free of charge, to any person obtaining a
 *    copy of this software and associated documentation files (the "Software"),
 *    to deal in the Software without restriction, including without limitation
 *    the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *    and/or sell copies of the Software, and to permit persons to whom the
 *    Software is furnished to do so, subject to the following conditions:
 *
 *    The above copyright notice and this permission notice shall be included in
 *    all copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *    DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 *    The GPL License (GPL)
 *
 *    Copyright (C) 2020  VeriSilicon Microelectronics Co., Ltd.
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software Foundation,
 *    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *****************************************************************************
 *
 *    Note: This software is released under dual MIT and GPL licenses. A
 *    recipient may use this file under the terms of either the MIT license or
 *    GPL License. If you wish to use only one license not the other, you can
 *    indicate your decision by deleting one of the above license notices
 *    in your version of this file.
 *
 *****************************************************************************
 */
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include "linux/string.h"
#include "subsys.h"
#include "vdec_log.h"

/* subsystem configuration */
#include "subsys_cfg.h"
//extern struct vcmd_config vcmd_core_array[MAX_SUBSYS_NUM];
//extern u32 total_vcmd_core_num;
//extern unsigned long multicorebase[];
//extern int irq[];
//extern unsigned int iosize[];
//extern int reg_count[];
#ifdef CONFIG_DEC_PM
extern int always_on;
#endif

int ParseVpuDtsiInfo(struct subsys_config *subsys,
		int *subsys_num, int *vcmd, struct platform_device *pdev) {
	int num = ARRAY_SIZE(subsys_array);
	int i, j;
	int irq = -1;
	struct device_node *vdec_node = NULL;
	struct resource *res = NULL;
#ifdef CONFIG_DEC_PM
	const char *power_always_on = NULL;
#endif

	vcmd_irq_flags = 0;
	memset(subsys, 0, sizeof(subsys[0]) * MAX_SUBSYS_NUM);
	for (i = 0; i < MAX_SUBSYS_NUM; i++) {
		subsys[i].base_addr = (i < num) ? subsys_array[i].base : 0;
		subsys[i].irq = -1;
		for (j = 0; j < HW_CORE_MAX; j++) {
			subsys[i].submodule_offset[j] = 0xffff;
			subsys[i].submodule_iosize[j] = 0;
			subsys[i].submodule_hwregs[j] = NULL;
		}
	}

	/* read outstanding value from dts */
	vdec_node = of_find_compatible_node(NULL, NULL, "xring,vpu-dec");

	if (vdec_node) {
#ifdef CONFIG_DEC_PM
		if (of_property_read_string(vdec_node, "power-always-on",
				&power_always_on)) {
			 vdec_klog(LOGLVL_ERROR, "failed to get string power-always-on\n");
		} else {
			if (!strcmp(power_always_on, "enable"))
				always_on = 1;
			else if (!strcmp(power_always_on, "disable"))
				always_on = 0;
			else {
				 vdec_klog(LOGLVL_ERROR, "power-always-on invalid!\n");
			}
		}
#endif
		if (of_property_read_u32(vdec_node, "vcmd-offset",
				&subsys[0].submodule_offset[HW_VCMD])) {
			 vdec_klog(LOGLVL_ERROR, "failed to get vcmd-offset\n");
			return -EINVAL;
		}
		if (of_property_read_u16(vdec_node, "vcmd-size",
				&subsys[0].submodule_iosize[HW_VCMD])) {
			 vdec_klog(LOGLVL_ERROR, "failed to get vcmd-size\n");
			return -EINVAL;
		}
		if (of_property_read_u32(vdec_node, "axife-offset",
				&subsys[0].submodule_offset[HW_AXIFE])) {
			 vdec_klog(LOGLVL_ERROR, "failed to get axife-offset\n");
			return -EINVAL;
		}
		if (of_property_read_u16(vdec_node, "axife-size",
				&subsys[0].submodule_iosize[HW_AXIFE])) {
			 vdec_klog(LOGLVL_ERROR, "failed to get axife-size\n");
			return -EINVAL;
		}
		if (of_property_read_u32(vdec_node, "main-offset",
				&subsys[0].submodule_offset[HW_VCD])) {
			 vdec_klog(LOGLVL_ERROR, "failed to get main-offset\n");
			return -EINVAL;
		}
		if (of_property_read_u16(vdec_node, "main-size",
				&subsys[0].submodule_iosize[HW_VCD])) {
			 vdec_klog(LOGLVL_ERROR, "failed to get main-size\n");
			return -EINVAL;
		}

		vdec_klog(LOGLVL_INFO, "vcmd(0x%x, 0x%x), axife(0x%x, 0x%x), vcd(0x%x, 0x%x)\n",
				subsys[0].submodule_offset[HW_VCMD],
				subsys[0].submodule_iosize[HW_VCMD],
				subsys[0].submodule_offset[HW_AXIFE],
				subsys[0].submodule_iosize[HW_AXIFE],
				subsys[0].submodule_offset[HW_VCD],
				subsys[0].submodule_iosize[HW_VCD]);

		irq = platform_get_irq_byname(pdev, VCMD_IRQ_NAME);
		if (irq < 0) {
			 vdec_klog(LOGLVL_ERROR, "failed to get vdec_irq");
			return -EINVAL;
		}
		subsys[0].irq = irq;
		vcmd_irq_flags = irqd_get_trigger_type(irq_get_irq_data(irq));

		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "vpu_dec0");
		if (!res) {
			vdec_klog(LOGLVL_INFO, "failed to get memory resource\n");
			return -EINVAL;
		}
		subsys[0].base_addr = res->start;
		vdec_klog(LOGLVL_INFO, "subsys[0].base_addr[0x%lx]\n", subsys[0].base_addr);
	} else {
		 vdec_klog(LOGLVL_ERROR, "find compatible node failed\n");
		return -EINVAL;
	}

	*vcmd = (subsys[0].submodule_iosize[HW_VCMD] > 0) ? 1 : 0;
	subsys[0].subsys_type = 2;
	*subsys_num = num;
	vdec_klog(LOGLVL_INFO, "memory: 0x%llx, hantrodec: vcmd = %d\n", res->start, *vcmd);
	return 0;
}
