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

#ifndef _HANTRO_VCMD_DBGFS_H
#define _HANTRO_VCMD_DBGFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/ioctl.h>
#include <linux/types.h>

/* for debugfs */
#define MAX_RESERVED_TIME 100
#define IDLE              0
#define RESERVED          1
#define DECODING          2
#define DONE              3
#define RECORD_N_HW_REGS  4

struct dev_dbgfs_info {
	u32 decode_cycles[MAX_RESERVED_TIME];
	int cycle_index;
	int num_cmdbuf_linked;
	u32 num_cmdbuf_exe[MAX_RESERVED_TIME];
	int exe_cmdbuf_index;
	u32 processed_cmdbuf_num;
	u32 num_cmdbuf_twoidle;
	u32 pre_hw_rdy_cmdbuf_num;
	u64 reserved_time[MAX_RESERVED_TIME];
	u32 reserved_cmdbufid[MAX_RESERVED_TIME];
	int reserved_index;
	u64 link_time[MAX_RESERVED_TIME];
	u32 link_cmdbufid[MAX_RESERVED_TIME];
	int link_index;
	u64 interrupt_time[MAX_RESERVED_TIME];
	int interrupt_index;
	u64 vcmd_workload;
	u64 active_start_time[MAX_RESERVED_TIME];
	u64 active_return_time[MAX_RESERVED_TIME];
	u32 active_hw_index;
	u32 active_state;
	u32 cmdbuf_num_done[MAX_RESERVED_TIME];
	u32 prev_cmdbuf_done[MAX_RESERVED_TIME];
	void *vcmd_mgr;
	void *dev;
	/* char *subsys_name =
	 * {"reg_subsys0", "reg_subsys1",
	 *  "reg_subsys2", "reg_subsys3"};
	 */
};

struct dbgfs_priv {
	struct dev_dbgfs_info *dev_dbgfs_info;
	u64 vcd_cycles;
	int N_reserved;
	/*dbgfs dentry pointer*/
	struct dentry *debugfs_root;
	struct dentry *root_cmdbuf[SLOT_NUM_CMDBUF];
};

int _dbgfs_init(void *vcmd_mgr);
void _dbgfs_init_ctx(void *vcmd_mgr, u32 store_hw_rdy_cmdbuf);
void _dbgfs_cleanup(void *_vcmd_mgr);
void _dbgfs_record_reserved_time(void *_dev_dbgfs, u32 cmdbuf_id);
void _dbgfs_record_link_time(void *_dev_dbgfs, u32 cmdbuf_id,
							u32 sw_cmdbuf_rdy_num, u64 exe_time);
void _dbgfs_record_active_start_time(void *_dev_dbgfs);
void _dbgfs_record_cmdbuf_num(void *_dev_dbgfs);
void _dbgfs_remove_cmdbuf(void *_dev_dbgfs, u32 cmdbuf_id);
void _dbgfs_reset_exe_cmdbuf_num(void *_dev_dbgfs);
void _dbgfs_record_vcd_cycles(void *_dev_dbgfs, u32 cmdbuf_id,
							  u32 module_type, u32 has_apb_arbiter);
void _dbgfs_update_index(void *_dev_dbgfs);

#ifdef __cplusplus
}
#endif

#endif
