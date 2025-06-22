/*
 * Copyright (C) 2023-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __XCTRL_DDR_MSG_DEFINE_H__
#define __XCTRL_DDR_MSG_DEFINE_H__

#define MSG_MAX_SIZE			16
#define PT_MSG_RESERVED_SIZE		14
#define DVFS_OK_MSG_RESERVED_SIZE	2

/* kernel commu msg - start */
enum KERNEL_COMMU_CMD {
	KC_CMD_PT = 1,
	KC_CMD_DVFS_OK_INFO = 2,
	KC_CMD_END,
};

struct kc_pt_msg {
	unsigned char cmd_id;
	unsigned char pt_cmd;
	unsigned char reserved[PT_MSG_RESERVED_SIZE];
};

#pragma pack(1)
struct kc_dvfs_ok_msg {
	unsigned char cmd_id;
	unsigned char reserved[DVFS_OK_MSG_RESERVED_SIZE];
	unsigned char freq_id;
	unsigned int isp_dvfs_ok_tout_cnt : 15;
	unsigned int last_isp_dvfs_ok : 1;
	unsigned int dpu_dvfs_ok_tout_cnt : 15;
	unsigned int last_dpu_dvfs_ok : 1;
	unsigned long long last_tout_timestamp;
};
#pragma pack()

union kc_msg {
	unsigned char msg[MSG_MAX_SIZE];
	struct kc_pt_msg pt_msg;
	struct kc_dvfs_ok_msg dvfs_ok_msg;
};
/* kernel commu msg - end */

#endif
