/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __LINUX_PERF_MONITOR_H__
#define __LINUX_PERF_MONITOR_H__

#define DEV_NAME "perf_monitor"
#define MIN_ACCU		0x3
#define MAX_ACCU		0xc8
#define MAX_TIME		43
#define BIT_NUM			32
#define DATA_SIZE		32
#define ACCU_CONVERT		209
#define CHANNEL_OPEN		0
#define MARK_ONE		1
#define MID_CNT			1010101
#define FILTER_MAINBUS		0x10000
#define FILTER_NPU		0x1E00000
#define FILTER_DMC		0x1E000000
#define FILTER_CPU		0x20000000
#define REG_SHIFT_8		8
#define REG_WIN_EN		(0x1 << 0)
#define REG_CFG_STOP		(0x1 << 1)
#define REG_COMMON		0x3
#define MASK_FF			0xFF00000000
#define MSK_L			0xFFFFFFFF
#define MSK_H			0xFFFFFFFF00000000
#define CONVERSION		0xF4240
#define CLEAR_INT		0xF
#define DERESET			1
#define PCLK			5
#define NSTOUS			0x3E8

#define DATA_IRQ		0xF
#define DATA_ERR_IRQ		0x2
#define PD_IRQ			0x4

#define CRG_SHIFT		0x4
#define MID_CLEAR		0

#define CRG_LONGTH		0x10
#define CRG_SHIFT		0x4

#define Media1_CRG_CLK_OFFSET  0x0040
#define Media1_VCRG_CLK_OFFSET 0x0060
#define Media1_CRG_SOFT_RESETS 0x0090
#define Media1_CRG_RESET_VALUE 0x07c0
#define Media1_CRG_CLK_VALUE   0x3e07c000
#define VDEC_TBU_RESET_VALUE   0x0020
#define VDEC_TBU_CLK_VALUE     0x0048
#define VDEC_TCU_CLK_OFFSET    0x0020
#define VDEC_TCU_CLK_VALUE     0x50000
#define VDEC_TCU_RESET_VALUE   0x0800

#define ISP_CRG_CLK_OFFSET     0x0250
#define ISP_CRG_SOFT_RESETS    0x0270
#define ISP_CRG_CLK_VALUE      0x8080
#define ISP_CRG_RESET_VALUE    0x800000
#define VENC_CRG_CLK_OFFSET    0x0010
#define VENC_CRG_SOFT_RESETS   0x0090
#define VENC_CRG_CLK_VALUE     0x0820
#define VENC_CRG_RESET_VALUE   0x0010
#define Media2_CRG_CLK_VALUE   0x2400
#define Media2_CRG_RESET_VALUE 0x0100

#define NPU_CRG_CLK_OFFSETS    0x0820
#define NPU_CRG_SOFT_RESETS    0x0908
#define NPU_CRG_CLK_VALUE      0x7FF80000
#define NPU_CRG_RESET_VALUE    0x03F0

#define GPU_CRG_CLK_OFFSETS    0x0024
#define GPU_CRG_SOFT_RESETS    0x0010
#define GPU_CRG_CLK_VALUE      0x0c00
#define GPU_CRG_RESET_VALUE    0x1000

#define CPU_L_MID_NUM          0x4e
#define CPU_M0_MID_NUM         0x4f
#define CPU_M1_MID_NUM         0x50
#define CPU_B_MID_NUM          0x51
#define CPU_WB_MID             0x52

#define RESERVE_MEM_SIZE       0x20
#define ONE_FRAME_SIZE         0x20
#define LOOP_MODE_BIT          0x4

#define LPIS_ACTRL_PERFMONITOR_REG_PERFMONITOR_DIS_MASK 0x1

#define FLUSH_CACHE_SIZE       (3 * RESERVE_MEM_SIZE)

enum pattern_type {
	LOOP_MODE = 0,
	ONE_SHOT_MODE
};

enum x1_id {
	X1_NPU = 0,
	X1_DDR
};

struct perf_mon {
	void __iomem		*regs;
	void			*mem;
	int			datasize;
	int			pat;
	const char		*fpath;
	u64			base_addr;
	u64			mem_size;
	struct device		*dev;
	struct device_node	*np;
	struct work_struct	work;
	struct perf_save_file	*pfile;
	void __iomem		*perf_clk_reg;
	u32			perf_clk_value;
	u32			time_shift;
};

struct perf_save_file {
	u64			size;
	loff_t			pos;
	u64			loop_cover_offset;
	u64			loop_bottom_size;
};

int perf_start(u32 num_of_chn_l, u64 num_of_chn_h, enum pattern_type pattern, uint32_t accuracy,
		uint32_t time);
int perf_stop(void);

#endif /* __LINUX_PERF_MONITOR_H__ */
