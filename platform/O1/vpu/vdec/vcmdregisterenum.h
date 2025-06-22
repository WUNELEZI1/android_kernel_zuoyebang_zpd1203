/* SPDX-License-Identifier: GPL-2.0 */
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

/* Register interface based on the document version v1.5.3 */
    HWIF_VCMD_HW_ID,
    HWIF_VCMD_HW_VERSION,
    HWIF_VCMD_HW_BUILDDATE,
    HWIF_VCMD_EXT_ABN_INTR_SRC,
    HWIF_VCMD_EXT_NORM_INTR_SRC,
    HWIF_VCMD_EXE_CMDBUF_COUNT,
    HWIF_VCMD_CMD_EXE,
    HWIF_VCMD_CMD_EXE_MSB,
    HWIF_VCMD_AXI_TOTALARLEN,
    HWIF_VCMD_AXI_TOTALR,
    HWIF_VCMD_AXI_TOTALAR,
    HWIF_VCMD_AXI_TOTALRLAST,
    HWIF_VCMD_AXI_TOTALAWLEN,
    HWIF_VCMD_AXI_TOTALW,
    HWIF_VCMD_AXI_TOTALAW,
    HWIF_VCMD_AXI_TOTALWLAST,
    HWIF_VCMD_AXI_TOTALB,
    HWIF_VCMD_AXI_ARVALID,
    HWIF_VCMD_AXI_ARREADY,
    HWIF_VCMD_AXI_RVALID,
    HWIF_VCMD_AXI_RREADY,
    HWIF_VCMD_AXI_AWVALID,
    HWIF_VCMD_AXI_AWREADY,
    HWIF_VCMD_AXI_WVALID,
    HWIF_VCMD_AXI_WREADY,
    HWIF_VCMD_AXI_BVALID,
    HWIF_VCMD_AXI_BREADY,
    HWIF_VCMD_HW_APBARBITER,
    HWIF_VCMD_HW_INITMODE,
    HWIF_VCMD_CORE_STATE,
    HWIF_VCMD_INIT_MODE,
    HWIF_VCMD_WORK_STATE,
    HWIF_VCMD_ARB_CHECK_EN,
    HWIF_VCMD_INIT_ENABLE,
    HWIF_VCMD_AXI_CLK_GATE_DISABLE,
    HWIF_VCMD_MASTER_OUT_CLK_GATE_DISABLE,
    HWIF_VCMD_CORE_CLK_GATE_DISABLE,
    HWIF_VCMD_ABORT_MODE,
    HWIF_VCMD_RESET_CORE,
    HWIF_VCMD_RESET_ALL,
    HWIF_VCMD_START_TRIGGER,
    HWIF_VCMD_IRQ_ARBRST,
    HWIF_VCMD_IRQ_JMP,
    HWIF_VCMD_IRQ_ARBERR,
    HWIF_VCMD_IRQ_ABORT,
    HWIF_VCMD_IRQ_CMDERR,
    HWIF_VCMD_IRQ_TIMEOUT,
    HWIF_VCMD_IRQ_BUSERR,
    HWIF_VCMD_IRQ_ENDCMD,
    HWIF_VCMD_IRQ_ARBRST_EN,
    HWIF_VCMD_IRQ_JMP_EN,
    HWIF_VCMD_IRQ_ARBERR_EN,
    HWIF_VCMD_IRQ_ABORT_EN,
    HWIF_VCMD_IRQ_CMDERR_EN,
    HWIF_VCMD_IRQ_TIMEOUT_EN,
    HWIF_VCMD_IRQ_BUSERR_EN,
    HWIF_VCMD_IRQ_ENDCMD_EN,
    HWIF_VCMD_TIMEOUT_ENABLE,
    HWIF_VCMD_TIMEOUT_CYCLES,
    HWIF_VCMD_CMDBUF_EXE_ADDR,
    HWIF_VCMD_CMDBUF_EXE_ADDR_MSB,
    HWIF_VCMD_CMDBUF_EXE_LENGTH,
    HWIF_VCMD_CMD_SWAP,
    HWIF_VCMD_DATA_SWAP,
    HWIF_VCMD_MAX_BURST_LEN,
    HWIF_VCMD_AXI_ID_RD,
    HWIF_VCMD_AXI_ID_WR,
    HWIF_VCMD_RDY_CMDBUF_COUNT,
    HWIF_VCMD_EXT_ABN_INTR_GATE,
    HWIF_VCMD_EXT_NORM_INTR_GATE,
    HWIF_VCMD_CMDBUF_EXE_ID,
    HWIF_VCMD_SRAM_TIMING_CTRL,
    HWIF_VCMD_ARB_WEIGHT,
    HWIF_VCMD_ARB_BW_OVERFLOW,
    HWIF_VCMD_ARB_URGENT,
    HWIF_VCMD_ARB_ENABLE,
    HWIF_VCMD_ARB_TIME_WINDOW_EXP,
    HWIF_VCMD_ARB_WINER_ID,
    HWIF_VCMD_ARB_CUR_ID,
    HWIF_VCMD_ARB_GRP_INFO,
    HWIF_VCMD_ARB_STATE,
    HWIF_VCMD_ARB_RST,
    HWIF_VCMD_ARB_ACK,
    HWIF_VCMD_ARB_FE,
    HWIF_VCMD_ARB_REQ,
    HWIF_VCMD_ARB_SATISFACTION,