/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 * Mdr public header for Kernel
*/

#ifndef __MDR_PUB_H__
#define __MDR_PUB_H__

#include <linux/module.h>
#include "mdr_public_if.h"

#define STR_EXCEPTIONDESC_MAXLEN	    48
#define MDR_REBOOTREASONWORD_MAXLEN     32
#define MDR_MODID_MAXLEN	50

enum PROCESS_PRI {
    MDR_ERR      = 0x01,
    MDR_WARN,
    MDR_OTHER,
    MDR_PPRI_MAX
};

enum REBOOT_PRI {
    MDR_REBOOT_NOW      = 0x01,
    MDR_REBOOT_WAIT,
    MDR_REBOOT_NO,
    MDR_REBOOT_MAX
};

enum REENTRANT {
    MDR_REENTRANT_ALLOW = 0xff00da00,
    MDR_REENTRANT_DISALLOW
};

enum UPLOAD_FLAG {
    MDR_UPLOAD_YES = 0xff00fa00,
    MDR_UPLOAD_NO
};

enum MDR_RETURN {
    MDR_SUCCESSED                   = 0x9f000000,
    MDR_FAILD                       = 0x9f000001,
    MDR_NULLPOINTER                 = 0x9f0000ff
};

typedef void (*mdr_e_callback)(u32, void*);

struct mdr_exception_info_s {
	struct list_head e_list;
	u32	e_modid;
	u32	e_modid_end;
	u32	e_process_priority;
	u32	e_reboot_priority;
	u64	e_notify_core_mask;
	u64	e_reset_core_mask;
	u64	e_from_core;
	u32	e_reentrant;
	u32	e_exce_type;
	u32	e_exce_subtype;
	u32	e_upload_flag;
	u8	e_from_module[MODULE_NAME_LEN];
	u8	e_desc[STR_EXCEPTIONDESC_MAXLEN];
	u32	e_reserve_u32;
	void	*e_reserve_p;
	mdr_e_callback	e_callback;
};

typedef void (*pfn_cb_dump_done)(u32 modid, u64 coreid);

typedef void (*pf_dump)(u32 modid, u32 etype, u64 coreid, char *logpath, pfn_cb_dump_done fndone, void *data);

typedef void (*pf_reset)(u32 modid, u32 e_type, u64 coreid, void *data);

typedef int (*pfn_exception_init_ops)(u8 *phy_addr, u8 *virt_addr, u32 log_len);

typedef int (*pfn_exception_analysis_ops)(u64 etime, u8 *addr, u32 len, struct mdr_exception_info_s *exception);

struct mdr_module_ops {
    pf_dump    ops_dump;
    pf_reset   ops_reset;
    void    *ops_data;
};

struct mdr_register_module_result {
    u64   log_addr;
    u32   log_len;
    u64   nve;
};

struct reboot_reason_word {
    unsigned char name[MDR_REBOOTREASONWORD_MAXLEN];
    unsigned int num;
    unsigned char category_name[MDR_REBOOTREASONWORD_MAXLEN];
    unsigned int category_num;
};

struct modid_word {
    u8 modid_name[MDR_MODID_MAXLEN];
    u32 modid_num;
};

u32 mdr_register_exception(struct mdr_exception_info_s *e);

int mdr_unregister_exception(u32 modid);

void *mdr_map(phys_addr_t paddr, size_t size);

void mdr_unmap(const void *vaddr);

int mdr_register_module_ops(
	u64 coreid,
	struct mdr_module_ops *ops,
	struct mdr_register_module_result *retinfo);

int mdr_unregister_module_ops(u64 coreid);

void mdr_system_error(u32 modid, u32 arg1, u32 arg2);
void mdr_syserr_process_for_ap(u32 modid, u64 arg1, u64 arg2);

void *mdr_vmap(phys_addr_t paddr, size_t size);
struct reboot_reason_word *get_reboot_reason_map(void);
u32 get_reboot_reason_map_size(void);
void set_reboot_reason(unsigned int reboot_reason);
void set_subtype_exception(unsigned int subtype, bool save_value);
unsigned int get_reboot_reason(void);
void ras_mdr_notify(struct atf_mdr_info *info);
u32 mdr_get_tick(void);
char *mdr_get_timestamp(void);
int mdr_send_nl(void *info, u32 size);
int mdr_filesys_write_log(u64 core_id, char *node_name, char *log_name, u32 log_size);
bool is_mdr_nlpid_valid(void);

int noc_register_exception(struct mdr_exception_info_s *e);

int noc_unregister_exception(struct mdr_exception_info_s *e);
#endif
