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
    HWIF_VCMD_HW_ID,
    HWIF_VCMD_HW_VERSION,
    HWIF_VCMD_HW_BUILD_DATE,
    HWIF_VCMD_EXT_ABN_INT_SRC_VCD_MMU,
    HWIF_VCMD_EXT_ABN_INT_SRC_VCD_L2CACHE,
    HWIF_VCMD_EXT_ABN_INT_SRC_VCD_DEC400,
    HWIF_VCMD_EXT_ABN_INT_SRC_VCD,
    HWIF_VCMD_EXT_ABN_INT_SRC_CUTREE_MMU,
    HWIF_VCMD_EXT_ABN_INT_SRC_VCE_MMU,
    HWIF_VCMD_EXT_ABN_INT_SRC_VCE_L2CACHE,
    HWIF_VCMD_EXT_ABN_INT_SRC_VCE_DEC400,
    HWIF_VCMD_EXT_ABN_INT_SRC_CUTREE,
    HWIF_VCMD_EXT_ABN_INT_SRC_VCE,
    HWIF_VCMD_EXT_NORM_INT_SRC_VCD_MMU,
    HWIF_VCMD_EXT_NORM_INT_SRC_VCD_L2CACHE,
    HWIF_VCMD_EXT_NORM_INT_SRC_VCD_DEC400,
    HWIF_VCMD_EXT_NORM_INT_SRC_VCD,
    HWIF_VCMD_EXT_NORM_INT_SRC_CUTREE_MMU,
    HWIF_VCMD_EXT_NORM_INT_SRC_VCE_MMU,
    HWIF_VCMD_EXT_NORM_INT_SRC_VCE_L2CACHE,
    HWIF_VCMD_EXT_NORM_INT_SRC_VCE_DEC400,
    HWIF_VCMD_EXT_NORM_INT_SRC_CUTREE,
    HWIF_VCMD_EXT_NORM_INT_SRC_VCE,
    HWIF_VCMD_EXE_CMDBUF_COUNT,
    HWIF_VCMD_EXECUTING_CMD,
    HWIF_VCMD_EXECUTING_CMD_MSB,
    HWIF_VCMD_AXI_TOTAL_AR_LEN,
    HWIF_VCMD_AXI_TOTAL_R,
    HWIF_VCMD_AXI_TOTAL_AR,
    HWIF_VCMD_AXI_TOTAL_R_LAST,
    HWIF_VCMD_AXI_TOTAL_AW_LEN,
    HWIF_VCMD_AXI_TOTAL_W,
    HWIF_VCMD_AXI_TOTAL_AW,
    HWIF_VCMD_AXI_TOTAL_W_LAST,
    HWIF_VCMD_AXI_TOTAL_B,
    HWIF_VCMD_AXI_AR_VALID,
    HWIF_VCMD_AXI_AR_READY,
    HWIF_VCMD_AXI_R_VALID,
    HWIF_VCMD_AXI_R_READY,
    HWIF_VCMD_AXI_AW_VALID,
    HWIF_VCMD_AXI_AW_READY,
    HWIF_VCMD_AXI_W_VALID,
    HWIF_VCMD_AXI_W_READY,
    HWIF_VCMD_AXI_B_VALID,
    HWIF_VCMD_AXI_B_READY,
    HWIF_VCMD_WORK_STATE,
    HWIF_VCMD_AXI_CLK_GATE_DISABLE,
    HWIF_VCMD_MASTER_OUT_CLK_GATE_DISABLE,
    HWIF_VCMD_CORE_CLK_GATE_DISABLE,
    HWIF_VCMD_ABORT_MODE,
    HWIF_VCMD_RESET_CORE,
    HWIF_VCMD_RESET_ALL,
    HWIF_VCMD_START_TRIGGER,
    HWIF_VCMD_IRQ_INTCMD,
    HWIF_VCMD_IRQ_JMPP,
    HWIF_VCMD_IRQ_JMPD,
    HWIF_VCMD_IRQ_RESET,
    HWIF_VCMD_IRQ_ABORT,
    HWIF_VCMD_IRQ_CMDERR,
    HWIF_VCMD_IRQ_TIMEOUT,
    HWIF_VCMD_IRQ_BUSERR,
    HWIF_VCMD_IRQ_ENDCMD,
    HWIF_VCMD_IRQ_INTCMD_EN,
    HWIF_VCMD_IRQ_JMPP_EN,
    HWIF_VCMD_IRQ_JMPD_EN,
    HWIF_VCMD_IRQ_RESET_EN,
    HWIF_VCMD_IRQ_ABORT_EN,
    HWIF_VCMD_IRQ_CMDERR_EN,
    HWIF_VCMD_IRQ_TIMEOUT_EN,
    HWIF_VCMD_IRQ_BUSERR_EN,
    HWIF_VCMD_IRQ_ENDCMD_EN,
    HWIF_VCMD_TIMEOUT_EN,
    HWIF_VCMD_TIMEOUT_CYCLES,
    HWIF_VCMD_EXECUTING_CMD_ADDR,
    HWIF_VCMD_EXECUTING_CMD_ADDR_MSB,
    HWIF_VCMD_EXE_CMDBUF_LENGTH,
    HWIF_VCMD_CMD_SWAP,
    HWIF_VCMD_MAX_BURST_LEN,
    HWIF_VCMD_AXI_ID_RD,
    HWIF_VCMD_AXI_ID_WR,
    HWIF_VCMD_RDY_CMDBUF_COUNT,
    HWIF_VCMD_EXT_ABN_INT_SRC_VCD_MMU_GATE,
    HWIF_VCMD_EXT_ABN_INT_SRC_VCD_L2CACHE_GATE,
    HWIF_VCMD_EXT_ABN_INT_SRC_VCD_DEC400_GATE,
    HWIF_VCMD_EXT_ABN_INT_SRC_VCD_GATE,
    HWIF_VCMD_EXT_ABN_INT_SRC_CUTREE_MMU_GATE,
    HWIF_VCMD_EXT_ABN_INT_SRC_VCE_MMU_GATE,
    HWIF_VCMD_EXT_ABN_INT_SRC_VCE_L2CACHE_GATE,
    HWIF_VCMD_EXT_ABN_INT_SRC_VCE_DEC400_GATE,
    HWIF_VCMD_EXT_ABN_INT_SRC_CUTREE_GATE,
    HWIF_VCMD_EXT_ABN_INT_SRC_VCE_GATE,
    HWIF_VCMD_EXT_NORM_INT_SRC_VCD_MMU_GATE,
    HWIF_VCMD_EXT_NORM_INT_SRC_VCD_L2CACHE_GATE,
    HWIF_VCMD_EXT_NORM_INT_SRC_VCD_DEC400_GATE,
    HWIF_VCMD_EXT_NORM_INT_SRC_VCD_GATE,
    HWIF_VCMD_EXT_NORM_INT_SRC_CUTREE_MMU_GATE,
    HWIF_VCMD_EXT_NORM_INT_SRC_VCE_MMU_GATE,
    HWIF_VCMD_EXT_NORM_INT_SRC_VCE_L2CACHE_GATE,
    HWIF_VCMD_EXT_NORM_INT_SRC_VCE_DEC400_GATE,
    HWIF_VCMD_EXT_NORM_INT_SRC_CUTREE_GATE,
    HWIF_VCMD_EXT_NORM_INT_SRC_VCE_GATE,
    HWIF_VCMD_CMDBUF_EXECUTING_ID,
