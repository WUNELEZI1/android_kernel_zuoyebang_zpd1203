/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef __MDR_ADAPTER_AP_H__
#define __MDR_ADAPTER_AP_H__

#define MDR_CONSOLE_LOGLEVEL_DEFAULT 7
#define MDR_COMMON_CALLBACK 0x1ull
#define LPM3_ADDR_SHIFT 26
#define LPM3_NMI_SIZE	0x100
#define ACPU_WDT_FIQ	"acpu wdt fiq"

extern void *g_mem_addr;
int mdr_ap_adapter_init(void);
int mdr_ap_panic_notify(struct notifier_block *nb, unsigned long event, void *buf);
char *mdr_get_category_name(u32 e_exce_type, u32 subtype);
int mdr_mem_init(void);
char *get_core_name(unsigned int coreid);
char *get_reboot_reason_name(unsigned int exec);
char *get_reboot_subtype_name(unsigned int exec, unsigned int subtype);
#endif
