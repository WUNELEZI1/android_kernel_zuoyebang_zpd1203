/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __MDR_FIELD_H__
#define __MDR_FIELD_H__

#include <linux/types.h>
#include "dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h"

#define DATA_MAXLEN	15
#define DATATIME_MAXLEN	20	/* 14 + 4 + 2, 2: '.'+'\0' */
#define MDR_PRODUCT_VERSION "PRODUCT_VERSION_STR"

#define MDR_TIME_LEN            16

#define FILE_MAGIC              0xdead8d8d

#ifdef CONFIG_SMP
#define MDR_SMP_FLAG            1
#else
#define MDR_SMP_FLAG            0
#endif

#define MDR_PRODUCT            "PRODUCT_NAME"
#define MDR_VERSION             ((MDR_SMP_FLAG << 16) | (0x204 << 0))

#define MDR_BASEINFO_SIZE         0x200
#define MDR_BUILD_DATE_TIME_LEN 16

#define MDR_DUMP_LOG_START     0x20120113
#define MDR_DUMP_LOG_DONE      0x20140607
#define MDR_PROC_EXEC_START    0xff115501
#define MDR_PROC_EXEC_DONE     0xff123059
#define MDR_REBOOT_DONE        0xff1230ff
#define MDR_PRODUCT_VERSION  "PRODUCT_VERSION_STR"


enum MDR_AREA_LIST {
	MDR_AREA_AP = 0x0,
	MDR_AREA_CP = 0x1,
	MDR_AREA_TEEOS = 0x2,
	MDR_AREA_AUDIO = 0x3,
	MDR_AREA_LPM3 = 0x4,
	MDR_AREA_SHUB = 0x5,
	MDR_AREA_ISP = 0x6,
	MDR_AREA_IVP = 0x7,
	MDR_AREA_UFS = 0x8,
	MDR_AREA_MODEMAP = 0x9,
	MDR_AREA_CLK = 0xA,
	MDR_AREA_REGULATOR = 0xB,
	MDR_AREA_MISEE = 0xD,
	MDR_AREA_NPU   = 0xE,
	MDR_AREA_CONN  = 0xF,
	MDR_AREA_EXCEPTION_TRACE = 0x10,
	MDR_AREA_MAXIMUM = 0x11
};

struct mdr_base_info_s {
	u32 modid;
	u32 arg1;
	u32 arg2;
	u32 e_core;
	u32 e_type;
	u32 e_subtype;
	u32 start_flag;
	u32 savefile_flag;
	u32 reboot_flag;
	u8 e_module[MODULE_NAME_LEN];
	u8 e_desc[STR_EXCEPTIONDESC_MAXLEN];

	u8 datetime[DATATIME_MAXLEN];
};

struct mdr_top_head_s {
	u32 magic;
	u32 version;
	u32 area_number;
	u32 reserve;
	u8 build_time[MDR_BUILD_DATE_TIME_LEN];
	u8 product_name[16];
	u8 product_version[16];
};

struct mdr_area_s {
	u64 offset;	/* offset from area, unit is bytes(1 bytes) */
	u32 length;	/* unit is bytes */
};

#pragma pack(4)
struct mdr_struct_s {
	struct mdr_top_head_s top_head;
	struct mdr_base_info_s base_info;
	struct mdr_area_s area_info[MDR_AREA_MAXIMUM];
	u8 padding2[MDR_BASEINFO_SIZE - sizeof(struct mdr_top_head_s)
		    - sizeof(struct mdr_area_s) * MDR_AREA_MAXIMUM
		    - sizeof(struct mdr_base_info_s)];
};
#pragma pack()

enum mdr_fid_type_e {
	MDR_LONG, /* long long */
	MDR_ARR, /* array  */
	MDR_QUE, /* queue  */
	MDR_STR /* string */
};

/* use 2 bits */
#define MDR_FIELD_TYPE(type) (type << 0)
/* use 1 bits */
#define MDR_FIELD_USED       (1 << 2)
#define MDR_FID_MASK 0xffff0000
#define mdr_offset(type, member) ((u64)&(((type *)0)->member))

struct mdr_field_s {
	u32 id;
	u32 offset;
	u32 size;
	u32 reserve;
};

typedef void (*parse_record_t) (u64 *data, u32 len);

struct field_rec_s {
	u32 id;
	u32 mask;
	u32 type;
	u32 len;
	u32 count;
	void *addr;
	parse_record_t f;
};

struct mdr_struct_s *mdr_get_pbb(void);
struct mdr_struct_s *mdr_get_tmppbb(void);
u64 mdr_get_pbb_size(void);
int mdr_field_init(void);
void mdr_set_area_info(int index, u32 size);
void mdr_save_args(u32 modid, u32 arg1, u32 arg2);
void mdr_show_base_info(int flag);
void mdr_fill_edata(struct mdr_exception_info_s *e, const char *date);
int mdr_get_areainfo(enum MDR_AREA_LIST area, struct mdr_register_module_result *retinfo);
void mdr_get_builddatetime(u8 *out);
u64 mdr_reserved_phymem_addr(void);
u64 mdr_reserved_phymem_size(void);
char *mdr_field_get_datetime(void);
void mdr_field_dumplog_done(void);
void mdr_field_reboot_done(void);
void mdr_field_procexec_done(void);
void mdr_field_baseinfo_reinit(void);
u32 mdr_get_reboot_type(void);


#endif
