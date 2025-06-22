/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __MDR_H__
#define __MDR_H__

#include <soc/xring/dfx_switch.h>
#include "dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h"
#include "dt-bindings/xring/platform-specific/common/mdr/include/dfx_kernel_user.h"
#include "mdr_dump.h"
#include "mdr_subsys_log.h"

#define FPGA 1
#define MDRLOG_OFFSET   0x0c20
#define MDR_CATEGORY_TYPE       "NA"
#define MEMDUMP_SIZE    (10*1024*1024)
#define KERNELLOGSIZE   (1024*1024)
#define SUBSYS_RESET    "subsys_reset"
#define MSEC100_PER_5SEC (MSEC_PER_SEC/100*5)

struct mdr_data {
	u32 fpga_flag;
	u32 fpga_reset_reg_addr;
	u32 fpga_sub_reset_reg_addr;
	u32 lpcore_nmi_addr;
	u32 reserved_mdr_phymem_addr;
	u32 reserved_mdr_phymem_size;
};

void record_exce_type(struct mdr_exception_info_s *e_info);
struct mdr_data *get_mdrdata(void);
void *get_mdrmem_addr(void);
int get_mdr_entry(struct mdr_exception_info_s *p, char *buf, u32 *size, char *path);
int mdr_send_nl(void *info, u32 size);
int mdr_netlink_init(void);
u64 get_dfx_reservedmem_addr(void);
void mdr_wait_subsys_logsave(void);
void mdr_wait_logsave_before_reset(struct mdr_exception_info_s *e_info);
u32 get_modid_map_size(void);
const struct modid_word *get_modid_map(void);

#endif
