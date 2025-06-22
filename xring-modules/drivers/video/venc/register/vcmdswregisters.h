/*
 * Copyright (c) 2015, Verisilicon Inc. - All Rights Reserved
 * Copyright (c) 2011-2014, Google Inc. - All Rights Reserved
 *
 * This software is dual licensed, either GPL-2.0 or BSD-3-Clause, at your
 * option.
 *
 ********************************************************************************
 *
 * GPL-2.0
 *
 ********************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
 * Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 ********************************************************************************
 *
 * Alternatively, This software may be distributed under the terms of
 * BSD-3-Clause, in which case the following provisions apply instead of the ones
 * mentioned above :
 *
 ********************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ********************************************************************************
 */

//Description: Common SWHW interface structure definition
//Based on document version v1.5.3
#ifndef VCMD_SWREGISTERS_H_
#define VCMD_SWREGISTERS_H_

#include "actypes.h"

struct VcmdSwRegisters {
  uai16 sw_HW_ID;
  uai16 sw_HW_Version;
  uai32 sw_HW_BuildDate;
  uai16 sw_ext_abn_intr_src;
  uai16 sw_ext_norm_intr_src;
  uai32 sw_exe_cmdbuf_count;
  uai32 sw_cmd_exe;
  uai32 sw_cmd_exe_msb;
  uai32 sw_axi_TotalARLen;
  uai32 sw_axi_TotalR;
  uai32 sw_axi_TotalAR;
  uai32 sw_axi_TotalRLast;
  uai32 sw_axi_TotalAWLen;
  uai32 sw_axi_TotalW;
  uai32 sw_axi_TotalAW;
  uai32 sw_axi_TotalWLast;
  uai32 sw_axi_TotalB;
  uai1 sw_axi_ARVALID;
  uai1 sw_axi_ARREADY;
  uai1 sw_axi_RVALID;
  uai1 sw_axi_RREADY;
  uai1 sw_axi_AWVALID;
  uai1 sw_axi_AWREADY;
  uai1 sw_axi_WVALID;
  uai1 sw_axi_WREADY;
  uai1 sw_axi_BVALID;
  uai1 sw_axi_BREADY;
  uai1 sw_HW_ApbArbiter;
  uai1 sw_HW_InitMode;
  uai5 sw_core_state;
  uai1 sw_init_mode;
  uai3 sw_work_state;
  uai1 sw_arb_check_en;
  uai1 sw_init_enable;
  uai1 sw_axi_clk_gate_disable;
  uai1 sw_master_out_clk_gate_disable;
  uai1 sw_core_clk_gate_disable;
  uai1 sw_abort_mode;
  uai1 sw_reset_core;
  uai1 sw_reset_all;
  uai1 sw_start_trigger;
  uai1 sw_irq_arbrst;
  uai1 sw_irq_jmp;
  uai1 sw_irq_arberr;
  uai1 sw_irq_abort;
  uai1 sw_irq_cmderr;
  uai1 sw_irq_timeout;
  uai1 sw_irq_buserr;
  uai1 sw_irq_endcmd;
  uai1 sw_irq_arbrst_en;
  uai1 sw_irq_jmp_en;
  uai1 sw_irq_arberr_en;
  uai1 sw_irq_abort_en;
  uai1 sw_irq_cmderr_en;
  uai1 sw_irq_timeout_en;
  uai1 sw_irq_buserr_en;
  uai1 sw_irq_endcmd_en;
  uai1 sw_timeout_enable;
  uai31 sw_timeout_cycles;
  uai32 sw_cmdbuf_exe_addr;
  uai32 sw_cmdbuf_exe_addr_msb;
  uai16 sw_cmdbuf_exe_length;
  uai4 sw_cmd_swap;
  uai4 sw_data_swap;
  uai8 sw_max_burst_len;
  uai8 sw_axi_id_rd;
  uai8 sw_axi_id_wr;
  uai32 sw_rdy_cmdbuf_count;
  uai16 sw_ext_abn_intr_gate;
  uai16 sw_ext_norm_intr_gate;
  uai32 sw_cmdbuf_exe_id;
  uai32 sw_sram_timing_ctrl;
  uai5 sw_arb_weight;
  uai1 sw_arb_bw_overflow;
  uai1 sw_arb_urgent;
  uai1 sw_arb_enable;
  uai5 sw_arb_time_window_exp;
  uai8 sw_arb_winer_id;
  uai8 sw_arb_cur_id;
  uai2 sw_arb_grp_info;
  uai2 sw_arb_state;
  uai1 sw_arb_rst;
  uai1 sw_arb_ack;
  uai1 sw_arb_fe;
  uai1 sw_arb_req;
  uai32 sw_arb_satisfaction;
};

#endif /*VCMD_SWREGISTERS_H_*/
