// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2022, MediaTek Inc.
 */

#include <linux/bitfield.h>
#include <linux/completion.h>
#include <linux/device.h>
#include <linux/kref.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/pci.h>
#include <linux/sched/signal.h>
#include <linux/skbuff.h>
#include <linux/time64.h>
#include <linux/timekeeping.h>
#include <linux/timer.h>
#include <linux/wait.h>

#include "mtk_common.h"
#include "mtk_debug.h"
#include "mtk_fsm.h"
#include "mtk_port.h"
#include "mtk_port_io.h"
#include "mtk_reg.h"
#ifdef CONFIG_TX00_UT_FSM
#include "ut_fsm_fake.h"
#endif

#define TAG "FSM"
#define EVT_TF_PAUSE (0)
#define EVT_TF_GATECLOSED (1)

#define FSM_HS_START_MASK	(FSM_F_SAP_HS_START | FSM_F_MD_HS_START)
#define FSM_HS2_DONE_MASK	(FSM_F_SAP_HS2_DONE | FSM_F_MD_HS2_DONE)

#define EXT_EVT_D2H_MDEE_MASK	(EXT_EVT_D2H_EXCEPT_INIT | EXT_EVT_D2H_EXCEPT_INIT_DONE |\
				 EXT_EVT_D2H_EXCEPT_CLEARQ_DONE | EXT_EVT_D2H_EXCEPT_ALLQ_RESET)
#define RTFT_DATA_SIZE		(3 * 1024)
#define DEV_STATE_POLLER_INTVAL	(HZ / 20)
#define BOOTUP_DELAY_TIME	(HZ * 60)

#define MDEE_CHK_ID		0x45584350
#define MDEE_REC_OK_CHK_ID	0x45524543

#define REGION_BITMASK		0xF
#define BROM_EVT_SHIFT		4
#define LK_EVT_SHIFT		8
#define DEVICE_CFG_SHIFT	24
#define DEVICE_CFG_REGION_MASK	0x3
#define FLASHLESS_ENABLE_SHIFT	16

#define HOST_EVT_SHIFT		28
#define HOST_REGION_BITMASK	0xF0000000

enum host_event {
	HOST_EVT_INIT = 0,
	HOST_ENTER_DA = 2,
	HOST_READY_FOR_FB_RESET,
};

enum brom_event {
	BROM_EVT_NORMAL = 0,
	BROM_EVT_JUMP_BL,
	BROM_EVT_TIME_OUT,
	BROM_EVT_JUMP_DA,
	BROM_EVT_START_DL,
	BROM_EVT_FW_DL,
	BROM_EVT_JUMP_LK,
};

enum lk_event {
	LK_EVT_NORMAL = 0,
	LK_EVT_CREATE_PD_PORT,
	LK_EVT_CREATE_FB_PORT,
};

enum device_stage {
	DEV_STAGE_INIT = 0,
	DEV_STAGE_BROM1,
	DEV_STAGE_BROM2,
	DEV_STAGE_LK,
	DEV_STAGE_LINUX,
	DEV_STAGE_PL,
	DEV_STAGE_MAX
};

enum device_cfg {
	DEV_CFG_NORMAL = 0,
	DEV_CFG_MD_ONLY,
};

enum runtime_feature_support_type {
	RTFT_TYPE_NOT_EXIST = 0,
	RTFT_TYPE_NOT_SUPPORT = 1,
	RTFT_TYPE_MUST_SUPPORT = 2,
	RTFT_TYPE_OPTIONAL_SUPPORT = 3,
	RTFT_TYPE_SUPPORT_BACKWARD_COMPAT = 4,
};

enum query_runtime_feature_id {
	QUERY_RTFT_ID_MD_PORT_ENUM = 0,
	QUERY_RTFT_ID_SAP_PORT_ENUM = 1,
	QUERY_RTFT_ID_MD_PORT_CFG = 2,
	QUERY_RTFT_ID_MAX
};

enum ctrl_msg_id {
	CTRL_MSG_HS1 = 0,
	CTRL_MSG_HS2 = 1,
	CTRL_MSG_HS3 = 2,
	CTRL_MSG_HS4 = 3,
	CTRL_MSG_MDEE = 4,
	CTRL_MSG_MDEE_REC_OK = 6,
	CTRL_MSG_MDEE_PASS = 8,
	CTRL_MSG_UNIFIED_PORT_CFG = 11,
};

struct timesync_zone {
	__le32 tz_minutewest;
	__le32 tz_dsttime;
};

struct high_res_timeinfo {
	__le64 utc_us;
	__le64 app_ref;
};

struct mtk_timesync_info {
	__le64 utc;
	struct timesync_zone tz;
	struct high_res_timeinfo hrt;
} __packed;

struct ctrl_msg_header {
	__le32 id;
	__le32 ex_msg;
	__le32 data_len;
	u8 reserved[];
} __packed;

struct runtime_feature_entry {
	u8 feature_id;
	struct runtime_feature_info support_info;
	u8 reserved[2];
	__le32 data_len;
	u8 data[];
};

struct feature_query {
	__le32 head_pattern;
	struct runtime_feature_info ft_set[FEATURE_CNT];
	__le32 tail_pattern;
};

static int mtk_fsm_send_hs1_msg(struct fsm_hs_info *hs_info)
{
	struct mtk_md_fsm *fsm = container_of(hs_info, struct mtk_md_fsm, hs_info[hs_info->id]);
	struct ctrl_msg_header *ctrl_msg_h;
	struct feature_query *ft_query;
	struct sk_buff *skb;
	int ret, msg_size;

	msg_size = sizeof(*ctrl_msg_h) + sizeof(*ft_query);
	skb = __dev_alloc_skb(msg_size, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	skb_put(skb, msg_size);
	/* fill control message header */
	ctrl_msg_h = (struct ctrl_msg_header *)skb->data;
	ctrl_msg_h->id = cpu_to_le32(CTRL_MSG_HS1);
	ctrl_msg_h->ex_msg = 0;
	ctrl_msg_h->data_len = cpu_to_le32(sizeof(*ft_query));

	/* fill feature query structure */
	ft_query = (struct feature_query *)(skb->data + sizeof(*ctrl_msg_h));
	ft_query->head_pattern = cpu_to_le32(FEATURE_QUERY_PATTERN);
	memcpy(ft_query->ft_set, hs_info->query_ft_set, sizeof(hs_info->query_ft_set));
	ft_query->tail_pattern = cpu_to_le32(FEATURE_QUERY_PATTERN);

	/* send handshake1 message to device */
	MTK_HEX_DUMP(fsm->mdev, MTK_DBG_RDIT, MTK_MEMLOG_RG_9,
		     "Dumping hs1 msg:", skb->data, skb->len);
	ret = mtk_port_internal_write(hs_info->ctrl_port, skb);
	if (ret <= 0)
		return ret;

	return 0;
}

static int mtk_fsm_feature_set_match(enum runtime_feature_support_type *cur_ft_spt,
				     struct runtime_feature_info rtft_info_st,
				     struct runtime_feature_info rtft_info_cfg)
{
	int ret = 0;

	switch (FIELD_GET(FEATURE_TYPE, rtft_info_st.feature)) {
	case RTFT_TYPE_NOT_EXIST:
		fallthrough;
	case RTFT_TYPE_NOT_SUPPORT:
		*cur_ft_spt = RTFT_TYPE_NOT_EXIST;
		break;
	case RTFT_TYPE_MUST_SUPPORT:
		if (FIELD_GET(FEATURE_TYPE, rtft_info_cfg.feature) == RTFT_TYPE_NOT_EXIST ||
		    FIELD_GET(FEATURE_TYPE, rtft_info_cfg.feature) == RTFT_TYPE_NOT_SUPPORT)
			ret = -EPROTO;
		else
			*cur_ft_spt = RTFT_TYPE_MUST_SUPPORT;
		break;
	case RTFT_TYPE_OPTIONAL_SUPPORT:
		if (FIELD_GET(FEATURE_TYPE, rtft_info_cfg.feature) == RTFT_TYPE_NOT_EXIST ||
		    FIELD_GET(FEATURE_TYPE, rtft_info_cfg.feature) == RTFT_TYPE_NOT_SUPPORT) {
			*cur_ft_spt = RTFT_TYPE_NOT_SUPPORT;
		} else {
			if (FIELD_GET(FEATURE_VER, rtft_info_st.feature) ==
			    FIELD_GET(FEATURE_VER, rtft_info_cfg.feature))
				*cur_ft_spt = RTFT_TYPE_MUST_SUPPORT;
			else
				*cur_ft_spt = RTFT_TYPE_NOT_SUPPORT;
		}
		break;
	case RTFT_TYPE_SUPPORT_BACKWARD_COMPAT:
		if (FIELD_GET(FEATURE_VER, rtft_info_st.feature) >=
		    FIELD_GET(FEATURE_VER, rtft_info_cfg.feature))
			*cur_ft_spt = RTFT_TYPE_MUST_SUPPORT;
		else
			*cur_ft_spt = RTFT_TYPE_NOT_EXIST;
		break;
	default:
		ret = -EPROTO;
	}

	return ret;
}

static int (*query_rtft_action[FEATURE_CNT])(struct mtk_md_dev *mdev, void *rt_data) = {
	[QUERY_RTFT_ID_MD_PORT_ENUM] = mtk_port_status_update,
	[QUERY_RTFT_ID_SAP_PORT_ENUM] = mtk_port_status_update,
};

static int mtk_fsm_parse_hs2_msg(struct fsm_hs_info *hs_info)
{
	struct mtk_md_fsm *fsm = container_of(hs_info, struct mtk_md_fsm, hs_info[hs_info->id]);
	char *rt_data = ((struct sk_buff *)hs_info->rt_data)->data;
	enum runtime_feature_support_type cur_ft_spt;
	struct runtime_feature_entry *rtft_entry;
	int ft_id, ret = 0, offset;

	offset = sizeof(struct feature_query);
	for (ft_id = 0; ft_id < FEATURE_CNT && offset < hs_info->rt_data_len; ft_id++) {
		rtft_entry = (struct runtime_feature_entry *)(rt_data + offset);
		ret = mtk_fsm_feature_set_match(&cur_ft_spt,
						rtft_entry->support_info,
						hs_info->query_ft_set[ft_id]);
		if (ret < 0)
			break;

		if (cur_ft_spt == RTFT_TYPE_MUST_SUPPORT)
			if (query_rtft_action[ft_id])
				ret = query_rtft_action[ft_id](fsm->mdev, rtft_entry->data);
		if (ret < 0)
			break;

		offset += sizeof(rtft_entry) + le32_to_cpu(rtft_entry->data_len);
	}

	if (ft_id != FEATURE_CNT) {
		MTK_ERR(fsm->mdev, "Unable to handle mistake hs2 msg, fd_id=%d\n", ft_id);
		MTK_HEX_DUMP(fsm->mdev, MTK_DBG_RDIT, MTK_MEMLOG_RG_9,
			     "Dumpping hs2 msg:", rt_data, hs_info->rt_data_len);
		ret = -EPROTO;
	}

	return ret;
}

static int mtk_fsm_rtft_sbp_id_action(struct mtk_md_dev *mdev, void *rt_data)
{
	int sbp_id = 0;

	*(int *)rt_data = sbp_id;

	return sizeof(*rt_data);
}

static int mtk_fsm_rtft_timesync_action(struct mtk_md_dev *mdev, void *rt_data)
{
	struct mtk_timesync_info *ts_info = (struct mtk_timesync_info *)rt_data;
	struct timespec64 ts;

	ktime_get_real_ts64(&ts);
	ts_info->utc = cpu_to_le64(ts.tv_sec);

	return sizeof(*ts_info);
}

static int (*support_rtft_action[FEATURE_CNT])(struct mtk_md_dev *mdev, void *rt_data) = {
	[SUPPORT_RTFT_ID_MD_SBP_ID] = mtk_fsm_rtft_sbp_id_action,
	[SUPPORT_RTFT_ID_MD_DSD] = NULL,
	[SUPPORT_RTFT_ID_MD_TIMESYNC] = mtk_fsm_rtft_timesync_action,
};

void mtk_fsm_supported_rtft_register(enum support_runtime_feature_id rtft_id,
				     int (*cb)(struct mtk_md_dev *mdev, void *rt_data))
{
	support_rtft_action[rtft_id] = cb;
	pr_info("%ps register rtft_id(%d) cb success!\n", __builtin_return_address(0), rtft_id);
}

static int mtk_fsm_append_rtft_entries(struct mtk_md_dev *mdev, void *feature_data,
				       unsigned int *len, struct fsm_hs_info *hs_info)
{
	char *rt_data = ((struct sk_buff *)hs_info->rt_data)->data;
	struct runtime_feature_entry *rtft_entry;
	int ft_id, ret = 0, rtdata_len = 0;
	struct feature_query *ft_query;
	u8 version;

	ft_query = (struct feature_query *)rt_data;
	if (le32_to_cpu(ft_query->head_pattern) != FEATURE_QUERY_PATTERN ||
	    le32_to_cpu(ft_query->tail_pattern) != FEATURE_QUERY_PATTERN) {
		MTK_ERR(mdev,
			"Failed to match ft_query pattern: head=0x%x,tail=0x%x\n",
			le32_to_cpu(ft_query->head_pattern), le32_to_cpu(ft_query->tail_pattern));
		ret = -EPROTO;
		goto hs_err;
	}

	/* parse runtime feature query and fill runtime feature entry */
	rtft_entry = feature_data;
	for (ft_id = 0; ft_id < FEATURE_CNT && rtdata_len < RTFT_DATA_SIZE; ft_id++) {
		rtft_entry->feature_id = ft_id;
		rtft_entry->data_len = 0;

		switch (FIELD_GET(FEATURE_TYPE, ft_query->ft_set[ft_id].feature)) {
		case RTFT_TYPE_NOT_EXIST:
			fallthrough;
		case RTFT_TYPE_NOT_SUPPORT:
			fallthrough;
		case RTFT_TYPE_MUST_SUPPORT:
			rtft_entry->support_info = ft_query->ft_set[ft_id];
			break;
		case RTFT_TYPE_OPTIONAL_SUPPORT:
			if (FIELD_GET(FEATURE_VER, ft_query->ft_set[ft_id].feature) ==
			    FIELD_GET(FEATURE_VER, hs_info->supported_ft_set[ft_id].feature) &&
			    FIELD_GET(FEATURE_TYPE, hs_info->supported_ft_set[ft_id].feature) >=
			    RTFT_TYPE_MUST_SUPPORT)
				rtft_entry->support_info.feature =
					FIELD_PREP(FEATURE_TYPE, RTFT_TYPE_MUST_SUPPORT);
			else
				rtft_entry->support_info.feature =
					FIELD_PREP(FEATURE_TYPE, RTFT_TYPE_NOT_SUPPORT);
			version = FIELD_GET(FEATURE_VER, hs_info->supported_ft_set[ft_id].feature);
			rtft_entry->support_info.feature |= FIELD_PREP(FEATURE_VER, version);
			break;
		case RTFT_TYPE_SUPPORT_BACKWARD_COMPAT:
			if (FIELD_GET(FEATURE_VER, ft_query->ft_set[ft_id].feature) >=
			    FIELD_GET(FEATURE_VER, hs_info->supported_ft_set[ft_id].feature))
				rtft_entry->support_info.feature =
					FIELD_PREP(FEATURE_TYPE, RTFT_TYPE_MUST_SUPPORT);
			else
				rtft_entry->support_info.feature =
					FIELD_PREP(FEATURE_TYPE, RTFT_TYPE_NOT_SUPPORT);
			version = FIELD_GET(FEATURE_VER, hs_info->supported_ft_set[ft_id].feature);
			rtft_entry->support_info.feature |= FIELD_PREP(FEATURE_VER, version);
			break;
		}

		if (FIELD_GET(FEATURE_TYPE, rtft_entry->support_info.feature) ==
		    RTFT_TYPE_MUST_SUPPORT) {
			if (support_rtft_action[ft_id]) {
				ret = support_rtft_action[ft_id](mdev, rtft_entry->data);
				if (ret < 0) {
					MTK_ERR(mdev,
						"Failed to exe rtft act, ft_id = %d, ret = %d\n",
						ft_id, ret);
					goto hs_err;
				}

				rtft_entry->data_len = cpu_to_le32(ret);
			} else {
				rtft_entry->support_info.feature =
					FIELD_PREP(FEATURE_TYPE, RTFT_TYPE_NOT_SUPPORT);
			}
		}

		rtdata_len += sizeof(*rtft_entry) + le32_to_cpu(rtft_entry->data_len);
		rtft_entry = (struct runtime_feature_entry *)(feature_data + rtdata_len);
	}
	*len = rtdata_len;
	return 0;

hs_err:
	MTK_HEX_DUMP(mdev, MTK_DBG_RDIT, MTK_MEMLOG_RG_9,
		     "Dumping hs2 msg:", rt_data, hs_info->rt_data_len);
	*len = 0;
	return ret;
}

static int mtk_fsm_send_hs3_msg(struct fsm_hs_info *hs_info)
{
	struct mtk_md_fsm *fsm = container_of(hs_info, struct mtk_md_fsm, hs_info[hs_info->id]);
	unsigned int data_len, msg_size = 0;
	struct ctrl_msg_header *ctrl_msg_h;
	struct sk_buff *skb;
	int ret;

	skb = __dev_alloc_skb(RTFT_DATA_SIZE, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	/* fill control message header */
	msg_size += sizeof(*ctrl_msg_h);
	ctrl_msg_h = (struct ctrl_msg_header *)skb->data;
	ctrl_msg_h->id = cpu_to_le32(CTRL_MSG_HS3);
	ctrl_msg_h->ex_msg = 0;
	ret = mtk_fsm_append_rtft_entries(fsm->mdev,
					  skb->data + sizeof(*ctrl_msg_h),
					  &data_len, hs_info);
	if (ret) {
		dev_kfree_skb_any(skb);
		return ret;
	}

	ctrl_msg_h->data_len = cpu_to_le32(data_len);
	msg_size += data_len;
	skb_put(skb, msg_size);
	/* send handshake3 message to device */
	MTK_HEX_DUMP(fsm->mdev, MTK_DBG_RDIT, MTK_MEMLOG_RG_9,
		     "Dumping hs3 msg:", skb->data, skb->len);
	ret = mtk_port_internal_write(hs_info->ctrl_port, skb);
	if (ret <= 0)
		return ret;

	return 0;
}

static int mtk_fsm_sap_ctrl_msg_handler(void *__fsm, struct sk_buff *skb)
{
	struct ctrl_msg_header *ctrl_msg_h;
	struct mtk_md_fsm *fsm = __fsm;
	struct fsm_hs_info *hs_info;

	ctrl_msg_h = (struct ctrl_msg_header *)skb->data;
	skb_pull(skb, sizeof(*ctrl_msg_h));

	hs_info = &fsm->hs_info[HS_ID_SAP];
	if (le32_to_cpu(ctrl_msg_h->id) != CTRL_MSG_HS2)
		return -EPROTO;

	hs_info->rt_data = skb;
	hs_info->rt_data_len = skb->len;
	mtk_fsm_evt_submit(fsm->mdev, FSM_EVT_STARTUP,
			   hs_info->fsm_flag_hs2, hs_info, sizeof(*hs_info), 0);

	return 0;
}

static int mtk_fsm_md_ctrl_msg_handler(void *__fsm, struct sk_buff *skb)
{
	struct ctrl_msg_header *ctrl_msg_h;
	struct mtk_md_fsm *fsm = __fsm;
	struct fsm_hs_info *hs_info;
	bool need_free_data = true;
	int ret = 0;
	u32 ex_msg;

	ctrl_msg_h = (struct ctrl_msg_header *)skb->data;
	ex_msg = le32_to_cpu(ctrl_msg_h->ex_msg);
	hs_info = &fsm->hs_info[HS_ID_MD];
	switch (le32_to_cpu(ctrl_msg_h->id)) {
	case CTRL_MSG_HS2:
		need_free_data = false;
		skb_pull(skb, sizeof(*ctrl_msg_h));
		hs_info->rt_data = skb;
		hs_info->rt_data_len = skb->len;
		mtk_fsm_evt_submit(fsm->mdev, FSM_EVT_STARTUP,
				   hs_info->fsm_flag_hs2, hs_info, sizeof(*hs_info), 0);
		break;
	case CTRL_MSG_HS4:
		mtk_fsm_evt_submit(fsm->mdev, FSM_EVT_STARTUP,
				   hs_info->fsm_flag_hs4, hs_info, sizeof(*hs_info), 0);
		break;
	case CTRL_MSG_MDEE:
		if (ex_msg != MDEE_CHK_ID)
			MTK_ERR(fsm->mdev, "Unable to match MDEE pkt(0x%x)\n", ex_msg);
		else
			mtk_fsm_evt_submit(fsm->mdev, FSM_EVT_MDEE,
					   FSM_F_MDEE_MSG, hs_info, sizeof(*hs_info), 0);
		break;
	case CTRL_MSG_MDEE_REC_OK:
		if (ex_msg != MDEE_REC_OK_CHK_ID)
			MTK_ERR(fsm->mdev, "Unable to match MDEE REC OK pkt(0x%x)\n", ex_msg);
		else
			mtk_fsm_evt_submit(fsm->mdev, FSM_EVT_MDEE,
					   FSM_F_MDEE_RECV_OK, NULL, 0, 0);
		break;
	case CTRL_MSG_MDEE_PASS:
		mtk_fsm_evt_submit(fsm->mdev, FSM_EVT_MDEE, FSM_F_MDEE_PASS, NULL, 0, 0);
		break;
	case CTRL_MSG_UNIFIED_PORT_CFG:
		mtk_port_tbl_update(fsm->mdev, skb->data + sizeof(*ctrl_msg_h));
		ret = mtk_port_internal_write(hs_info->ctrl_port, skb);
		if (ret <= 0)
			MTK_ERR(fsm->mdev, "Unable to send port config ack msg\n");
		need_free_data = false;
		break;
	default:
		MTK_ERR(fsm->mdev, "Invalid ctrl msg id\n");
	}

	if (need_free_data)
		dev_kfree_skb(skb);

	return ret;
}

static int (*ctrl_msg_handler[HS_ID_MAX])(void *__fsm, struct sk_buff *skb) = {
	[HS_ID_MD] = mtk_fsm_md_ctrl_msg_handler,
	[HS_ID_SAP] = mtk_fsm_sap_ctrl_msg_handler,
};

static ssize_t fsm_state_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct pci_dev *pdev = container_of(dev, struct pci_dev, dev);
	struct mtk_md_fsm *fsm = ((struct mtk_md_dev *)pci_get_drvdata(pdev))->fsm;

	if (fsm)
		return sprintf(buf, "state=%d, fsm_flag=0x%x\n", fsm->state, fsm->fsm_flag);
	else
		return sprintf(buf, "Invalid FSM!\n");
}

static DEVICE_ATTR_RO(fsm_state);

static void mtk_fsm_host_evt_ack(struct mtk_md_dev *mdev, enum host_event id)
{
	u32 dev_state;

	dev_state = mtk_hw_get_dev_state(mdev);
	dev_state &= ~HOST_REGION_BITMASK;
	dev_state |= id << HOST_EVT_SHIFT;
	mtk_hw_ack_dev_state(mdev, dev_state);

	dev_state = mtk_hw_get_dev_state(mdev);
	if (!(dev_state & id << HOST_EVT_SHIFT))
		MTK_ERR(mdev, "Failed to ack device, state=0x%x", dev_state);
}

static void mtk_fsm_brom_evt_handler(struct mtk_md_dev *mdev, u32 dev_state)
{
	u32 brom_evt = dev_state >> BROM_EVT_SHIFT & REGION_BITMASK;

	switch (brom_evt) {
	case BROM_EVT_JUMP_BL:
		mtk_fsm_evt_submit(mdev, FSM_EVT_DOWNLOAD, FSM_F_DL_JUMPBL, NULL, 0, 0);
		break;
	case BROM_EVT_TIME_OUT:
		mtk_fsm_evt_submit(mdev, FSM_EVT_DOWNLOAD, FSM_F_DL_TIMEOUT, NULL, 0, 0);
		break;
	case BROM_EVT_JUMP_DA:
		mtk_fsm_host_evt_ack(mdev, HOST_ENTER_DA);
		mtk_fsm_evt_submit(mdev, FSM_EVT_DOWNLOAD, FSM_F_DL_DA, NULL, 0, 0);
		break;
	case BROM_EVT_START_DL:
		mtk_fsm_evt_submit(mdev, FSM_EVT_DOWNLOAD, FSM_F_DL_PORT_CREATE, NULL, 0, 0);
		break;
	default:
		MTK_ERR(mdev, "Invalid brom event, value = 0x%x\n", dev_state);
	}
}

static void mtk_fsm_lk_evt_handler(struct mtk_md_dev *mdev, u32 dev_state)
{
	u32 lk_evt = dev_state >> LK_EVT_SHIFT & REGION_BITMASK;

	switch (lk_evt) {
	case LK_EVT_CREATE_PD_PORT:
		mtk_fsm_evt_submit(mdev, FSM_EVT_POSTDUMP, FSM_F_DFLT, NULL, 0, 0);
		break;
	case LK_EVT_CREATE_FB_PORT:
		mtk_fsm_evt_submit(mdev, FSM_EVT_DOWNLOAD, FSM_F_DL_FB, NULL, 0, 0);
		break;
	default:
		MTK_ERR(mdev, "Invalid LK event, value = 0x%x\n", dev_state);
	}
}

static unsigned short flashless_hs_flow;
static unsigned short md_hs_only;

static void mtk_fsm_linux_evt_handler(struct mtk_md_dev *mdev,
				      u32 dev_state, struct mtk_md_fsm *fsm)
{
	u32 dev_cfg = dev_state >> DEVICE_CFG_SHIFT & DEVICE_CFG_REGION_MASK;
	u32 flashless = dev_state >> FLASHLESS_ENABLE_SHIFT & 0x1;
	int hs_id;

	if (mdev->hw_ver == 0x4d80) {
		fsm->hs_done_flag = FSM_F_MD_HS_START | FSM_F_MD_HS2_DONE;
	} else {
		if (dev_cfg == DEV_CFG_MD_ONLY || md_hs_only)
			fsm->hs_done_flag = FSM_F_MD_HS_START | FSM_F_MD_HS2_DONE;
		else
			fsm->hs_done_flag = FSM_HS_START_MASK | FSM_HS2_DONE_MASK;
		if (flashless || flashless_hs_flow) {
			fsm->hs_info[HS_ID_MD].fsm_flag_hs4 = FSM_F_MD_HS4_DONE;
			fsm->hs_done_flag |= FSM_F_MD_HS4_DONE;
		}
	}

	for (hs_id = 0; hs_id < HS_ID_MAX; hs_id++)
		mtk_hw_unmask_ext_evt(mdev, fsm->hs_info[hs_id].mhccif_ch);

	mtk_hw_unmask_ext_evt(mdev, EXT_EVT_D2H_MDEE_MASK);
}

static void mtk_fsm_pl_evt_handler(struct mtk_md_dev *mdev, u32 dev_state)
{
	switch (dev_state >> BROM_EVT_SHIFT & REGION_BITMASK) {
	case BROM_EVT_FW_DL:
		mtk_fsm_evt_submit(mdev, FSM_EVT_DOWNLOAD, FSM_F_DL_PL, NULL, 0, 0);
		break;
	case BROM_EVT_JUMP_LK:
		mtk_fsm_evt_submit(mdev, FSM_EVT_DOWNLOAD, FSM_F_DL_JUMPLK, NULL, 0, 0);
		break;
	default:
		MTK_ERR(mdev, "Invalid pl event, value = 0x%x\n", dev_state);
	}
}

static void mtk_fsm_bootup_dump_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct mtk_md_fsm *fsm;

	fsm = container_of(dwork, struct mtk_md_fsm, bootup_dump_work);
	mtk_fsm_evt_submit(fsm->mdev, FSM_EVT_DUMP, FSM_F_DFLT, NULL, 0, 0);
}

static int mtk_fsm_early_bootup_handler(u32 status, void *__fsm)
{
	struct mtk_md_fsm *fsm = __fsm;
	struct mtk_md_dev *mdev;
	u32 dev_state, dev_stage;

	mdev = fsm->mdev;
	if (mdev->hw_ver == 0x0800 || mdev->hw_ver == 0x0300) {
		mtk_hw_mask_ext_evt(mdev, status);
		mtk_hw_clear_ext_evt(mdev, status);
	}

	dev_state = mtk_hw_get_dev_state(mdev);
	dev_stage = dev_state & REGION_BITMASK;
	if (dev_stage >= DEV_STAGE_MAX) {
		MTK_ERR(fsm->mdev, "Invalid dev state 0x%x\n", dev_state);
		if (mtk_hw_mmio_check(mdev))
			mtk_except_report_evt(mdev, EXCEPT_LINK_ERR);
		return -ENXIO;
	}
	MTK_INFO(mdev, "Device stage change 0x%x->0x%x\n", fsm->last_dev_state, dev_state);

	if (dev_state == fsm->last_dev_state)
		goto exit;
	MTK_INFO(mdev, "Device stage change 0x%x->0x%x\n", fsm->last_dev_state, dev_state);
	fsm->last_dev_state = dev_state;

	cancel_delayed_work(&fsm->bootup_dump_work);
	queue_delayed_work(system_wq, &fsm->bootup_dump_work, BOOTUP_DELAY_TIME);

	switch (dev_stage) {
	case DEV_STAGE_BROM1:
		fallthrough;
	case DEV_STAGE_BROM2:
		mtk_fsm_brom_evt_handler(mdev, dev_state);
		break;
	case DEV_STAGE_LK:
		mtk_fsm_lk_evt_handler(mdev, dev_state);
		break;
	case DEV_STAGE_LINUX:
		mtk_fsm_linux_evt_handler(mdev, dev_state, fsm);
		break;
	case DEV_STAGE_PL:
		mtk_fsm_pl_evt_handler(mdev, dev_state);
		break;
	}

exit:
	if ((mdev->hw_ver == 0x0800 || mdev->hw_ver == 0x0300) && dev_stage != DEV_STAGE_LINUX)
		mtk_hw_unmask_ext_evt(mdev, EXT_EVT_D2H_BOOT_FLOW_SYNC);

	return 0;
}

static void mtk_fsm_dev_state_poller_handler(struct timer_list *t)
{
	struct mtk_md_fsm *fsm = from_timer(fsm, t, dev_state_poller);
	u32 dev_state;
	int ret;

	dev_state = mtk_hw_get_dev_state(fsm->mdev);
	ret = mtk_fsm_early_bootup_handler(EXT_EVT_D2H_BOOT_FLOW_SYNC, fsm);
	if (!ret && (dev_state & REGION_BITMASK) != DEV_STAGE_LINUX)
		mod_timer(&fsm->dev_state_poller, jiffies + DEV_STATE_POLLER_INTVAL);
}

static int mtk_fsm_ctrl_ch_start(struct mtk_md_fsm *fsm, struct fsm_hs_info *hs_info)
{
	if (!hs_info->ctrl_port) {
		hs_info->ctrl_port = mtk_port_internal_open(fsm->mdev, hs_info->port_name, 0);
		if (!hs_info->ctrl_port) {
			MTK_ERR(fsm->mdev, "Failed to open ctrl port(%s)\n",
				hs_info->port_name);
			return -ENODEV;
		}
		mtk_port_internal_recv_register(hs_info->ctrl_port,
						ctrl_msg_handler[hs_info->id], fsm);
	}

	return 0;
}

static void mtk_fsm_ctrl_ch_stop(struct mtk_md_fsm *fsm)
{
	struct fsm_hs_info *hs_info;
	int hs_id;

	for (hs_id = 0; hs_id < HS_ID_MAX; hs_id++) {
		hs_info = &fsm->hs_info[hs_id];
		if (hs_info->ctrl_port) {
			mtk_port_internal_close(hs_info->ctrl_port);
			hs_info->ctrl_port = NULL;
		}
	}
}

static void mtk_fsm_switch_state(struct mtk_md_fsm *fsm,
				 enum mtk_fsm_state to_state,
				 struct mtk_fsm_evt *event)
{
	unsigned long start_jiffies, end_jiffies;
	char uevent_info[MTK_UEVENT_INFO_LEN];
	struct mtk_fsm_notifier *nt;
	struct mtk_fsm_param param;

	param.from = fsm->state;
	param.to = to_state;
	param.evt_id = event ? event->id : FSM_EVT_MAX;
	param.fsm_flag = event ? event->fsm_flag : FSM_F_DFLT;

	list_for_each_entry(nt, &fsm->pre_notifiers, entry) {
		start_jiffies = jiffies;
		nt->cb(&param, nt->data);
		end_jiffies = jiffies;
		MTK_DBG(fsm->mdev, MTK_DBG_FSM, MTK_MEMLOG_RG_0,
			"Fsm pre notifier(%d) processed time :%d(ms)\n",
			   nt->id, jiffies_to_msecs(end_jiffies - start_jiffies));
	}

	fsm->state = to_state;
	fsm->fsm_flag |= event ? event->fsm_flag : FSM_F_DFLT;
	MTK_INFO(fsm->mdev, "FSM transited to state=%d, fsm_flag=0x%x\n",
		 to_state, fsm->fsm_flag);

	snprintf(uevent_info, MTK_UEVENT_INFO_LEN,
		 "state=%d, fsm_flag=0x%x", to_state, fsm->fsm_flag);
	mtk_uevent_notify(fsm->mdev->dev, MTK_UEVENT_FSM, uevent_info);

	list_for_each_entry(nt, &fsm->post_notifiers, entry) {
		start_jiffies = jiffies;
		nt->cb(&param, nt->data);
		end_jiffies = jiffies;
		MTK_DBG(fsm->mdev, MTK_DBG_FSM, MTK_MEMLOG_RG_0,
			"Fsm post notifier(%d) processed time :%d(ms)\n",
			   nt->id, jiffies_to_msecs(end_jiffies - start_jiffies));
	}
}

static int mtk_fsm_startup_act(struct mtk_md_fsm *fsm, struct mtk_fsm_evt *event)
{
	enum mtk_fsm_state to_state = FSM_STATE_BOOTUP;
	struct fsm_hs_info *hs_info = event->data;
	struct mtk_md_dev *mdev = fsm->mdev;
	int ret = 0;

	if (fsm->state != FSM_STATE_ON && fsm->state != FSM_STATE_DOWNLOAD &&
	    fsm->state != FSM_STATE_BOOTUP) {
		ret = -EPROTO;
		goto free_rt_data;
	}

	if (event->fsm_flag & FSM_HS_START_MASK) {
		mtk_fsm_switch_state(fsm, to_state, event);

		ret = mtk_fsm_ctrl_ch_start(fsm, hs_info);
		if (!ret)
			ret = mtk_fsm_send_hs1_msg(hs_info);
		if (ret)
			goto hs_err;
	} else if (event->fsm_flag & FSM_HS2_DONE_MASK) {
		ret = mtk_fsm_parse_hs2_msg(hs_info);
		if (!ret) {
			mtk_fsm_switch_state(fsm, to_state, event);
			ret = mtk_fsm_send_hs3_msg(hs_info);
		}
		dev_kfree_skb(hs_info->rt_data);
		hs_info->rt_data = NULL;
		if (ret)
			goto hs_err;
	} else if (event->fsm_flag & FSM_F_MD_HS4_DONE) {
		mtk_fsm_switch_state(fsm, to_state, event);
	}

	if (((fsm->fsm_flag | event->fsm_flag) & fsm->hs_done_flag) == fsm->hs_done_flag) {
		to_state = FSM_STATE_READY;
		mtk_fsm_switch_state(fsm, to_state, NULL);
		cancel_delayed_work(&fsm->bootup_dump_work);
	}

	return 0;

free_rt_data:
	if (hs_info && hs_info->rt_data) {
		dev_kfree_skb(hs_info->rt_data);
		hs_info->rt_data = NULL;
	}
hs_err:
	MTK_ERR(mdev, "Failed to hs with device %d:0x%x, ret=%d", fsm->state, fsm->fsm_flag, ret);
	return ret;
}

static int mtk_fsm_mdee_act(struct mtk_md_fsm *fsm, struct mtk_fsm_evt *event)
{
	struct mtk_md_dev *mdev = fsm->mdev;
	struct ctrl_msg_header *ctrl_msg_h;
	struct fsm_hs_info *hs_info;
	struct sk_buff *ctrl_msg;
	int ret;

	if (fsm->state != FSM_STATE_ON && fsm->state != FSM_STATE_BOOTUP &&
	    fsm->state != FSM_STATE_READY && fsm->state != FSM_STATE_MDEE)
		return -EPROTO;

	mtk_fsm_switch_state(fsm, FSM_STATE_MDEE, event);

	switch (event->fsm_flag) {
	case FSM_F_MDEE_INIT:
		cancel_delayed_work(&fsm->bootup_dump_work);
		queue_delayed_work(system_wq, &fsm->bootup_dump_work, BOOTUP_DELAY_TIME);
		mtk_hw_send_ext_evt(mdev, EXT_EVT_H2D_EXCEPT_ACK);
		mtk_fsm_ctrl_ch_start(fsm, &fsm->hs_info[HS_ID_MD]);
		{//add by xiaomi for speedup crash reboot
			extern bool g_t800_recovery_nodump_flag;
			if (g_t800_recovery_nodump_flag) {
				extern void t800_recovery_start(int value1, int value2);
				t800_recovery_start(3, 0);
			}
		}
		break;
	case FSM_F_MDEE_CLEARQ_DONE:
		mtk_hw_send_ext_evt(mdev, EXT_EVT_H2D_EXCEPT_CLEARQ_ACK);
		break;
	case FSM_F_MDEE_MSG:
		hs_info = event->data;
		ctrl_msg = __dev_alloc_skb(sizeof(*ctrl_msg), GFP_KERNEL);
		if (!ctrl_msg)
			return -ENOMEM;

		skb_put(ctrl_msg, sizeof(*ctrl_msg_h));
		/* fill control message header */
		ctrl_msg_h = (struct ctrl_msg_header *)ctrl_msg->data;
		ctrl_msg_h->id = cpu_to_le32(CTRL_MSG_MDEE);
		ctrl_msg_h->ex_msg = cpu_to_le32(MDEE_CHK_ID);
		ctrl_msg_h->data_len = 0;

		ret = mtk_port_internal_write(hs_info->ctrl_port, ctrl_msg);
		if (ret <= 0) {
			dev_err(mdev->dev, "Unable to send MDEE msg, ret = %d\n", ret);
			return -EPROTO;
		}
		break;
	case FSM_F_MDEE_RECV_OK:
		dev_info(mdev->dev, "MDEE hs1 done\n");
		break;
	case FSM_F_MDEE_PASS:
		cancel_delayed_work(&fsm->bootup_dump_work);
		dev_info(mdev->dev, "MDEE hs2 done\n");
		break;
	}

	return 0;
}

static int mtk_fsm_download_act(struct mtk_md_fsm *fsm, struct mtk_fsm_evt *event)
{
	if (fsm->state != FSM_STATE_ON && fsm->state != FSM_STATE_DOWNLOAD)
		return -EPROTO;

	mtk_fsm_switch_state(fsm, FSM_STATE_DOWNLOAD, event);

	return 0;
}

static int mtk_fsm_postdump_act(struct mtk_md_fsm *fsm, struct mtk_fsm_evt *event)
{
	if (fsm->state != FSM_STATE_ON && fsm->state != FSM_STATE_DOWNLOAD)
		return -EPROTO;

	mtk_fsm_switch_state(fsm, FSM_STATE_POSTDUMP, event);

	return 0;
}

static void mtk_fsm_evt_release(struct kref *kref)
{
	struct mtk_fsm_evt *event = container_of(kref, struct mtk_fsm_evt, kref);

	devm_kfree(event->mdev->dev, event);
}

static void mtk_fsm_evt_put(struct mtk_fsm_evt *event)
{
	kref_put(&event->kref, mtk_fsm_evt_release);
}

static void mtk_fsm_evt_finish(struct mtk_md_fsm *fsm,
			       struct mtk_fsm_evt *event, int retval)
{
	if (event->mode & EVT_MODE_BLOCKING) {
		event->status = retval;
		wake_up(&fsm->evt_waitq);
	}
	mtk_fsm_evt_put(event);
}

static void mtk_fsm_evt_cleanup(struct mtk_md_fsm *fsm, struct list_head *evtq)
{
	struct mtk_fsm_evt *event, *tmp;

	list_for_each_entry_safe(event, tmp, evtq, entry) {
		list_del(&event->entry);
		mtk_fsm_evt_finish(fsm, event, FSM_EVT_RET_FAIL);
	}
}

static void mtk_fsm_hw_unregister_ext_evt(struct mtk_md_fsm *fsm)
{
	struct mtk_md_dev *mdev = fsm->mdev;
	struct fsm_hs_info *hs_info;
	int hs_id;

	mtk_hw_unregister_ext_evt(mdev, EXT_EVT_D2H_BOOT_FLOW_SYNC);
	mtk_hw_unregister_ext_evt(mdev, EXT_EVT_D2H_MDEE_MASK);
	for (hs_id = 0; hs_id < HS_ID_MAX; hs_id++) {
		hs_info = &fsm->hs_info[hs_id];
		mtk_hw_unregister_ext_evt(mdev, hs_info->mhccif_ch);
	}
}

static int mtk_fsm_enter_off_state(struct mtk_md_fsm *fsm, struct mtk_fsm_evt *event)
{
	struct mtk_md_dev *mdev = fsm->mdev;
	int hs_id;

	if (fsm->state == FSM_STATE_OFF || fsm->state == FSM_STATE_INVALID)
		return -EPROTO;

	mtk_hw_mask_ext_evt(mdev, EXT_EVT_D2H_BOOT_FLOW_SYNC);
	for (hs_id = 0; hs_id < HS_ID_MAX; hs_id++)
		mtk_hw_mask_ext_evt(mdev, fsm->hs_info[hs_id].mhccif_ch);
	mtk_hw_mask_ext_evt(mdev, EXT_EVT_D2H_MDEE_MASK);
	mtk_fsm_hw_unregister_ext_evt(fsm);

	mtk_fsm_ctrl_ch_stop(fsm);
	del_timer_sync(&fsm->dev_state_poller);
	cancel_delayed_work_sync(&fsm->bootup_dump_work);
	mtk_fsm_switch_state(fsm, FSM_STATE_OFF, event);

	return 0;
}

static int mtk_fsm_dev_rm_act(struct mtk_md_fsm *fsm, struct mtk_fsm_evt *event)
{
	unsigned long flags;

	spin_lock_irqsave(&fsm->evtq_lock, flags);
	set_bit(EVT_TF_GATECLOSED, &fsm->t_flag);
	mtk_fsm_evt_cleanup(fsm, &fsm->evtq);
	spin_unlock_irqrestore(&fsm->evtq_lock, flags);

	return mtk_fsm_enter_off_state(fsm, event);
}

static int mtk_fsm_hs1_handler(u32 status, void *__hs_info)
{
	struct fsm_hs_info *hs_info = __hs_info;
	struct mtk_md_dev *mdev;
	struct mtk_md_fsm *fsm;

	fsm = container_of(hs_info, struct mtk_md_fsm, hs_info[hs_info->id]);
	mdev = fsm->mdev;
	mtk_fsm_evt_submit(mdev, FSM_EVT_STARTUP,
			   hs_info->fsm_flag_hs1, hs_info, sizeof(*hs_info), 0);
	mtk_hw_mask_ext_evt(mdev, hs_info->mhccif_ch);
	mtk_hw_clear_ext_evt(mdev, hs_info->mhccif_ch);

	return 0;
}

static int mtk_fsm_mdee_handler(u32 status, void *__fsm)
{
	u32 handled_mdee_mhccif_ch = 0;
	struct mtk_md_fsm *fsm = __fsm;
	struct mtk_md_dev *mdev;

	mdev = fsm->mdev;

	if (status & EXT_EVT_D2H_EXCEPT_INIT) {
		mtk_fsm_evt_submit(mdev, FSM_EVT_MDEE,
				   FSM_F_MDEE_INIT, NULL, 0, 0);
		handled_mdee_mhccif_ch |= EXT_EVT_D2H_EXCEPT_INIT;
	}

	if (status & EXT_EVT_D2H_EXCEPT_INIT_DONE) {
		mtk_fsm_evt_submit(mdev, FSM_EVT_MDEE,
				   FSM_F_MDEE_INIT_DONE, NULL, 0, 0);
		handled_mdee_mhccif_ch |= EXT_EVT_D2H_EXCEPT_INIT_DONE;
	}

	if (status & EXT_EVT_D2H_EXCEPT_CLEARQ_DONE) {
		mtk_fsm_evt_submit(mdev, FSM_EVT_MDEE,
				   FSM_F_MDEE_CLEARQ_DONE, NULL, 0, 0);
		handled_mdee_mhccif_ch |= EXT_EVT_D2H_EXCEPT_CLEARQ_DONE;
	}

	if (status & EXT_EVT_D2H_EXCEPT_ALLQ_RESET) {
		mtk_fsm_evt_submit(mdev, FSM_EVT_MDEE,
				   FSM_F_MDEE_ALLQ_RESET, NULL, 0, 0);
		handled_mdee_mhccif_ch |= EXT_EVT_D2H_EXCEPT_ALLQ_RESET;
	}

	mtk_hw_mask_ext_evt(mdev, handled_mdee_mhccif_ch);
	mtk_hw_clear_ext_evt(mdev, handled_mdee_mhccif_ch);

	return 0;
}

static void mtk_fsm_hs_info_init(struct mtk_md_fsm *fsm)
{
	struct fsm_hs_info *hs_info;
	int hs_id;

	for (hs_id = 0; hs_id < HS_ID_MAX; hs_id++) {
		hs_info = &fsm->hs_info[hs_id];
		hs_info->id = hs_id;
		hs_info->ctrl_port = NULL;
		hs_info->rt_data = NULL;
		switch (hs_id) {
		case HS_ID_MD:
			snprintf(hs_info->port_name, PORT_NAME_LEN, "MDCTRL");
			hs_info->mhccif_ch = EXT_EVT_D2H_ASYNC_HS_NOTIFY_MD;
			hs_info->fsm_flag_hs1 = FSM_F_MD_HS_START;
			hs_info->fsm_flag_hs2 = FSM_F_MD_HS2_DONE;
			hs_info->query_ft_set[QUERY_RTFT_ID_MD_PORT_ENUM].feature =
				FIELD_PREP(FEATURE_TYPE, RTFT_TYPE_MUST_SUPPORT);
			hs_info->query_ft_set[QUERY_RTFT_ID_MD_PORT_ENUM].feature |=
				FIELD_PREP(FEATURE_VER, 0);
			hs_info->query_ft_set[QUERY_RTFT_ID_MD_PORT_CFG].feature =
				FIELD_PREP(FEATURE_TYPE, RTFT_TYPE_OPTIONAL_SUPPORT);
			hs_info->query_ft_set[QUERY_RTFT_ID_MD_PORT_CFG].feature |=
				FIELD_PREP(FEATURE_VER, 0);
			hs_info->supported_ft_set[SUPPORT_RTFT_ID_MD_SBP_ID].feature =
				FIELD_PREP(FEATURE_TYPE, RTFT_TYPE_MUST_SUPPORT);
			hs_info->supported_ft_set[SUPPORT_RTFT_ID_MD_SBP_ID].feature |=
				FIELD_PREP(FEATURE_VER, 0);
			hs_info->supported_ft_set[SUPPORT_RTFT_ID_MD_DSD].feature =
				FIELD_PREP(FEATURE_TYPE, RTFT_TYPE_MUST_SUPPORT);
			hs_info->supported_ft_set[SUPPORT_RTFT_ID_MD_DSD].feature |=
				FIELD_PREP(FEATURE_VER, 0);
			hs_info->supported_ft_set[SUPPORT_RTFT_ID_MD_TIMESYNC].feature =
				FIELD_PREP(FEATURE_TYPE, RTFT_TYPE_MUST_SUPPORT);
			hs_info->supported_ft_set[SUPPORT_RTFT_ID_MD_TIMESYNC].feature |=
				FIELD_PREP(FEATURE_VER, 0);
			break;
		case HS_ID_SAP:
			snprintf(hs_info->port_name, PORT_NAME_LEN, "SAPCTRL");
			hs_info->mhccif_ch = EXT_EVT_D2H_ASYNC_HS_NOTIFY_SAP;
			hs_info->fsm_flag_hs1 = FSM_F_SAP_HS_START;
			hs_info->fsm_flag_hs2 = FSM_F_SAP_HS2_DONE;
			hs_info->query_ft_set[QUERY_RTFT_ID_SAP_PORT_ENUM].feature =
				FIELD_PREP(FEATURE_TYPE, RTFT_TYPE_MUST_SUPPORT);
			hs_info->query_ft_set[QUERY_RTFT_ID_SAP_PORT_ENUM].feature |=
				FIELD_PREP(FEATURE_VER, 0);
			break;
		}
	}
}

static void mtk_fsm_hw_register_ext_evt(struct mtk_md_fsm *fsm)
{
	struct mtk_md_dev *mdev = fsm->mdev;
	struct fsm_hs_info *hs_info;
	int hs_id;

	mtk_hw_register_ext_evt(fsm->mdev, EXT_EVT_D2H_BOOT_FLOW_SYNC,
				mtk_fsm_early_bootup_handler, fsm);
	mtk_hw_register_ext_evt(mdev, EXT_EVT_D2H_MDEE_MASK, mtk_fsm_mdee_handler, fsm);
	for (hs_id = 0; hs_id < HS_ID_MAX; hs_id++) {
		hs_info = &fsm->hs_info[hs_id];
		mtk_hw_register_ext_evt(mdev, hs_info->mhccif_ch,
					mtk_fsm_hs1_handler, hs_info);
	}
}

static int mtk_fsm_dev_add_act(struct mtk_md_fsm *fsm, struct mtk_fsm_evt *event)
{
	struct mtk_md_dev *mdev = fsm->mdev;

	if (fsm->state != FSM_STATE_OFF && fsm->state != FSM_STATE_INVALID)
		return -EPROTO;

	mtk_fsm_switch_state(fsm, FSM_STATE_ON, event);
	mtk_fsm_hw_register_ext_evt(fsm);

	if (mdev->hw_ver == 0x4d75)
		mod_timer(&fsm->dev_state_poller, jiffies + DEV_STATE_POLLER_INTVAL);
	else if (mdev->hw_ver == 0x4d80)
		mtk_fsm_linux_evt_handler(mdev, 0, fsm);
	else if (mdev->hw_ver == 0x0800 || mdev->hw_ver == 0x0300)
		mtk_hw_unmask_ext_evt(mdev, EXT_EVT_D2H_BOOT_FLOW_SYNC);

	queue_delayed_work(system_wq, &fsm->bootup_dump_work, BOOTUP_DELAY_TIME);

	return 0;
}

static void mtk_fsm_reset(struct mtk_md_fsm *fsm)
{
	//fsm->t_flag = 0;
	//reinit_completion(&fsm->paused);
	fsm->last_dev_state = 0;
	fsm->fsm_flag = FSM_F_DFLT;

	mtk_fsm_hs_info_init(fsm);
}

static int mtk_fsm_dev_reinit_act(struct mtk_md_fsm *fsm, struct mtk_fsm_evt *event)
{
	struct mtk_md_dev *mdev = fsm->mdev;

	if (fsm->state != FSM_STATE_OFF)
		return -EPROTO;

	if (event->fsm_flag == FSM_F_FULL_REINIT) {
		mtk_hw_reinit(mdev, REINIT_TYPE_EXP);
		event->fsm_flag = 0;
	} else {
		mtk_hw_reinit(mdev, REINIT_TYPE_RESUME);
	}

	mtk_fsm_reset(fsm);
	mtk_fsm_switch_state(fsm, FSM_STATE_ON, event);
	mtk_fsm_hw_register_ext_evt(fsm);

	if (mdev->hw_ver == 0x4d75)
		mod_timer(&fsm->dev_state_poller, jiffies + DEV_STATE_POLLER_INTVAL);
	else if (mdev->hw_ver == 0x4d80)
		mtk_fsm_linux_evt_handler(mdev, 0, fsm);
	else if (mdev->hw_ver == 0x0800 || mdev->hw_ver == 0x0300)
		mtk_hw_unmask_ext_evt(mdev, EXT_EVT_D2H_BOOT_FLOW_SYNC);

	queue_delayed_work(system_wq, &fsm->bootup_dump_work, BOOTUP_DELAY_TIME);

	return 0;
}

static int mtk_fsm_fb_reset_act(struct mtk_md_fsm *fsm, struct mtk_fsm_evt *event)
{
	struct mtk_md_dev *mdev = fsm->mdev;

	if (fsm->state != FSM_STATE_READY)
		return -EPROTO;

	mtk_fsm_host_evt_ack(mdev, HOST_READY_FOR_FB_RESET);
	mtk_hw_send_ext_evt(mdev, EXT_EVT_H2D_DEVICE_RESET);

	return 0;
}

static int mtk_fsm_dump_act(struct mtk_md_fsm *fsm, struct mtk_fsm_evt *event)
{
	if (fsm->state == FSM_STATE_OFF || fsm->state == FSM_STATE_INVALID)
		return -EPROTO;

	mtk_dev_dump(fsm->mdev);
	//extern int mdm_mdr_trigger_exception(void);
	//extern bool mtk_except_link_err_trigger_panic;
	//if (mtk_except_link_err_trigger_panic) {
	//	mdm_mdr_trigger_exception();
	//	mtk_except_link_err_trigger_panic = false;
	//}

	return 0;
}

/***********************************xiaomi notify modem status to user space start ***/
extern void t800_send_monitor_uevent(int id, int value1, int value2, int value3);
static int mtk_fsm_xiaomi_modem_act(struct mtk_md_fsm *fsm, struct mtk_fsm_evt *event)
{
	if (event->len != 16)
		return -1;
	int *p = event->data;
	t800_send_monitor_uevent(p[0], p[1], p[2], p[3]);
	return 0;
}
/***********************************xiaomi notify modem status to user space end ***/

static int (*evts_act_tbl[FSM_EVT_MAX])(struct mtk_md_fsm *__fsm, struct mtk_fsm_evt *event) = {
	[FSM_EVT_DOWNLOAD] = mtk_fsm_download_act,
	[FSM_EVT_POSTDUMP] = mtk_fsm_postdump_act,
	[FSM_EVT_STARTUP] = mtk_fsm_startup_act,
	[FSM_EVT_LINKDOWN] = mtk_fsm_enter_off_state,
	[FSM_EVT_AER] = mtk_fsm_enter_off_state,
	[FSM_EVT_COLD_RESUME] = mtk_fsm_enter_off_state,
	[FSM_EVT_REINIT] = mtk_fsm_dev_reinit_act,
	[FSM_EVT_MDEE] = mtk_fsm_mdee_act,
	[FSM_EVT_DEV_RESET_REQ] = mtk_fsm_enter_off_state,
	[FSM_EVT_DEV_RM] = mtk_fsm_dev_rm_act,
	[FSM_EVT_DEV_ADD] = mtk_fsm_dev_add_act,
	[FSM_EVT_SOFT_OFF] = mtk_fsm_enter_off_state,
	[FSM_EVT_FB_RESET] = mtk_fsm_fb_reset_act,
	[FSM_EVT_PWROFF] = mtk_fsm_enter_off_state,
	[FSM_EVT_DUMP] = mtk_fsm_dump_act,
	[FSM_EVT_XIAOMI_MODEM] = mtk_fsm_xiaomi_modem_act,
};

/**
 * mtk_fsm_start() - start FSM service
 * @mdev: mdev pointer to mtk_md_dev
 *
 * This function start a fsm service to handle fsm event.
 *
 * Return: return value is 0 on success, a negative error code on failure.
 */
int mtk_fsm_start(struct mtk_md_dev *mdev)
{
	struct mtk_md_fsm *fsm = mdev->fsm;

	if (!fsm)
		return -EINVAL;

	MTK_INFO(mdev, "Start fsm by %ps!\n", __builtin_return_address(0));
	clear_bit(EVT_TF_PAUSE, &fsm->t_flag);
	if (!fsm->fsm_handler)
		return -EFAULT;

	wake_up_process(fsm->fsm_handler);
	return 0;
}

/**
 * mtk_fsm_pause() - pause fsm service
 * @mdev: pointer to mtk_md_dev.
 *
 * If the function is called in irq context, it is able to be paused, or
 * it will return as soon. It can only work in process context.
 *
 * Return:
 * * 0: the fsm handler thread is paused.
 * * <0: fail to pause fsm handler thread.
 */
int mtk_fsm_pause(struct mtk_md_dev *mdev)
{
	struct mtk_md_fsm *fsm = mdev->fsm;
	bool reinit_flag = false;

	if (!fsm)
		return -EINVAL;

	MTK_INFO(mdev, "Pause fsm by %ps!\n", __builtin_return_address(0));
	if (!test_and_set_bit(EVT_TF_PAUSE, &fsm->t_flag)) {
		reinit_completion(&fsm->paused);
		wake_up_process(fsm->fsm_handler);
		reinit_flag = true;
		MTK_WARN(mdev,
                         "fsm reinit completion in mtk_fsm_pause \n");
	}
	MTK_WARN(mdev,
                         "start wait for fsm->paused status in mtk_fsm_pause \n");
	wait_for_completion(&fsm->paused);
	return 0;
}

static void mkt_fsm_notifier_cleanup(struct mtk_md_dev *mdev, struct list_head *ntq)
{
	struct mtk_fsm_notifier *nt, *tmp;

	list_for_each_entry_safe(nt, tmp, ntq, entry) {
		list_del(&nt->entry);
		MTK_WARN(mdev,
			 "Having to free notifier(%d) by FSM!\n", nt->id);
		devm_kfree(mdev->dev, nt);
	}
}

static void mtk_fsm_notifier_insert(struct mtk_fsm_notifier *notifier, struct list_head *head)
{
	struct mtk_fsm_notifier *nt;

	list_for_each_entry(nt, head, entry) {
		if (notifier->prio > nt->prio) {
			list_add(&notifier->entry, nt->entry.prev);
			return;
		}
	}
	list_add_tail(&notifier->entry, head);
}

/**
 * mtk_fsm_notifier_register() - register notifier callback
 * @mdev: pointer to mtk_md_dev
 * @id: user id
 * @cb: pointer to notification callback provided by user
 * @data: pointer to user data if any
 * @prio: PRIO_0, PRIO_1
 * @is_pre: 1: pre switch, 0: post switch
 *
 * Return: return value is 0 on success, a negative error code on failure.
 */
int mtk_fsm_notifier_register(struct mtk_md_dev *mdev,
			      enum mtk_user_id id,
			      void (*cb)(struct mtk_fsm_param *, void *data),
			      void *data,
			      enum mtk_fsm_prio prio,
			      bool is_pre)
{
	struct mtk_md_fsm *fsm = mdev->fsm;
	struct mtk_fsm_notifier *notifier;

	if (!fsm)
		return -EINVAL;

	if (id >= MTK_USER_MAX || !cb || prio >= FSM_PRIO_MAX)
		return -EINVAL;

	notifier = devm_kzalloc(mdev->dev, sizeof(*notifier), GFP_KERNEL);
	if (!notifier)
		return -ENOMEM;

	INIT_LIST_HEAD(&notifier->entry);
	notifier->id = id;
	notifier->cb = cb;
	notifier->data = data;
	notifier->prio = prio;

	if (is_pre)
		mtk_fsm_notifier_insert(notifier, &fsm->pre_notifiers);
	else
		mtk_fsm_notifier_insert(notifier, &fsm->post_notifiers);

	return 0;
}

/**
 * mtk_fsm_notifier_unregister() - unregister notifier callback
 * @mdev: pointer to mtk_md_dev
 * @id: user id
 *
 * Return: return value is 0 on success, a negative error code on failure.
 */
int mtk_fsm_notifier_unregister(struct mtk_md_dev *mdev, enum mtk_user_id id)
{
	struct mtk_md_fsm *fsm = mdev->fsm;
	struct mtk_fsm_notifier *nt, *tmp;

	if (!fsm)
		return -EINVAL;

	list_for_each_entry_safe(nt, tmp, &fsm->pre_notifiers, entry) {
		if (nt->id == id) {
			list_del(&nt->entry);
			devm_kfree(mdev->dev, nt);
			break;
		}
	}
	list_for_each_entry_safe(nt, tmp, &fsm->post_notifiers, entry) {
		if (nt->id == id) {
			list_del(&nt->entry);
			devm_kfree(mdev->dev, nt);
			break;
		}
	}
	return 0;
}

static void mtk_fsm_evt_timeout(struct timer_list *t)
{
	struct mtk_md_fsm *fsm = from_timer(fsm, t, evt_timer);

	MTK_WARN(fsm->mdev, "Handling fsm event timeout!\n");
}

#define BLOCKING_EVT_TIMEOUT				(4 * EVT_HANDLER_TIMEOUT)

/**
 * mtk_fsm_evt_submit() - submit event
 * @mdev: pointer to mtk_md_dev
 * @id: event id
 * @flag: state flag
 * @data: user data
 * @len: data length
 * @mode: EVT_MODE_BLOCKING(1<<0) means that submit blocking until
 *        event is handled, EVT_MODE_TOHEAD(1<<1) means the event
 *        will be handled in high priority.
 *
 * Return: 0 will be returned, if the event is appended (non-blocking)
 *         or event is completed(blocking), -1 will be returned if
 *         timeout, 1 will be returned if it is finished.
 */
int mtk_fsm_evt_submit(struct mtk_md_dev *mdev,
		       enum mtk_fsm_evt_id id,
		       enum mtk_fsm_flag flag,
		       void *data, unsigned int len,
		       unsigned char mode)
{
	struct mtk_md_fsm *fsm = mdev->fsm;
	struct mtk_fsm_evt *event;
	unsigned long flags;
	int ret = 0;

	if (!fsm || id >= FSM_EVT_MAX) {
		MTK_ERR(mdev, "Invalid param!\n");
		return FSM_EVT_RET_FAIL;
	}

	if (test_bit(EVT_TF_GATECLOSED, &fsm->t_flag)) {
		MTK_ERR(mdev, "Failed to submit evt, fsm has been removed!\n");
		return FSM_EVT_RET_FAIL;
	}

	event = devm_kzalloc(mdev->dev, sizeof(*event),
			     (in_irq() || in_softirq() || irqs_disabled()) ?
			     GFP_ATOMIC : GFP_KERNEL);
	if (!event) {
		MTK_ERR(mdev, "Failed to alloc event!\n");
		return FSM_EVT_RET_FAIL;
	}

	kref_init(&event->kref);
	event->mdev = mdev;
	event->id = id;
	event->fsm_flag = flag;
	event->status = FSM_EVT_RET_ONGOING;
	event->data = data;
	event->len = len;
	event->mode = mode;
	if (event->id != FSM_EVT_XIAOMI_MODEM) {
		MTK_INFO(mdev, "Event%d(with mode 0x%x) is appended by %ps\n",
		 event->id, event->mode, __builtin_return_address(0));
	}

	spin_lock_irqsave(&fsm->evtq_lock, flags);
	if (test_bit(EVT_TF_GATECLOSED, &fsm->t_flag)) {
		spin_unlock_irqrestore(&fsm->evtq_lock, flags);
		mtk_fsm_evt_put(event);
		MTK_ERR(mdev, "Failed to add event, fsm dev has been removed!\n");
		return FSM_EVT_RET_FAIL;
	}

	kref_get(&event->kref);
	if (mode & EVT_MODE_TOHEAD)
		list_add(&event->entry, &fsm->evtq);
	else
		list_add_tail(&event->entry, &fsm->evtq);
	spin_unlock_irqrestore(&fsm->evtq_lock, flags);

	wake_up_process(fsm->fsm_handler);
	if (mode & EVT_MODE_BLOCKING) {
		ret = wait_event_timeout(fsm->evt_waitq,
					 (event->status != 0), BLOCKING_EVT_TIMEOUT);
		if (!ret && event->status != FSM_EVT_RET_DONE)
			MTK_ERR(mdev, "Handling fsm blocking event timeout!\n");

		ret = event->status;
	}
	mtk_fsm_evt_put(event);

	return ret;
}

static int mtk_fsm_evt_handler(void *__fsm)
{
	struct mtk_md_fsm *fsm = __fsm;
	struct mtk_fsm_evt *event;
	unsigned long flags;
	int ret;

wake_up:
	while (!kthread_should_stop() &&
	       !test_bit(EVT_TF_PAUSE, &fsm->t_flag) && !list_empty(&fsm->evtq)) {
		spin_lock_irqsave(&fsm->evtq_lock, flags);
		event = list_first_entry(&fsm->evtq, struct mtk_fsm_evt, entry);
		list_del(&event->entry);
		spin_unlock_irqrestore(&fsm->evtq_lock, flags);

		if (event->id != FSM_EVT_XIAOMI_MODEM) {
			MTK_INFO(fsm->mdev, "Event%d(0x%x) is under handling\n",
			 event->id, event->fsm_flag);
		}

		mod_timer(&fsm->evt_timer, jiffies + EVT_HANDLER_TIMEOUT);

		if (event->id < FSM_EVT_MAX) {
			ret = evts_act_tbl[event->id](fsm, event);
			if (ret) {
				MTK_ERR(fsm->mdev,
					"Failed to handle evt, fsm state = %d, ret = %d\n",
					fsm->state, ret);
				mtk_fsm_evt_finish(fsm, event, FSM_EVT_RET_FAIL);
			} else {
				mtk_fsm_evt_finish(fsm, event, FSM_EVT_RET_DONE);
			}
		} else {
			mtk_fsm_evt_finish(fsm, event, FSM_EVT_RET_DONE);
		}

		del_timer_sync(&fsm->evt_timer);
	}

	if (kthread_should_stop())
		return 0;

	if (test_bit(EVT_TF_PAUSE, &fsm->t_flag))
		complete_all(&fsm->paused);

	set_current_state(TASK_INTERRUPTIBLE);
	schedule();

	if (fatal_signal_pending(current)) {
		/* event handler thread is killed by fatal signal,
		 * all the waiters will be waken up.
		 */
		complete_all(&fsm->paused);
		mtk_fsm_evt_cleanup(fsm, &fsm->evtq);
		return -ERESTARTSYS;
	}
	goto wake_up;
}

/**
 * mtk_fsm_init() - allocate FSM control block and initialize it
 * @mdev: pointer to mtk_md_dev
 *
 * This function creates a mtk_md_fsm structure dynamically and hook
 * it up to mtk_md_dev. When you are finished with this structure,
 * call mtk_fsm_exit() and the structure will be dynamically freed.
 *
 * Return: return value is 0 on success, a negative error code on failure.
 */
int mtk_fsm_init(struct mtk_md_dev *mdev)
{
	struct mtk_md_fsm *fsm;
	int ret;

	fsm = devm_kzalloc(mdev->dev, sizeof(*fsm), GFP_KERNEL);
	if (!fsm)
		return -ENOMEM;

	fsm->fsm_handler = kthread_create(mtk_fsm_evt_handler, fsm, "fsm_evt_thread%d_%s",
					  mdev->hw_ver, mdev->dev_str);
	if (IS_ERR(fsm->fsm_handler)) {
		ret = PTR_ERR(fsm->fsm_handler);
		goto err_create;
	}

	fsm->mdev = mdev;
	init_completion(&fsm->paused);
	timer_setup(&fsm->evt_timer, mtk_fsm_evt_timeout, 0);
	timer_setup(&fsm->dev_state_poller, mtk_fsm_dev_state_poller_handler, 0);
	INIT_DELAYED_WORK(&fsm->bootup_dump_work, mtk_fsm_bootup_dump_work);

	fsm->state = FSM_STATE_INVALID;
	fsm->fsm_flag = FSM_F_DFLT;

	INIT_LIST_HEAD(&fsm->evtq);
	spin_lock_init(&fsm->evtq_lock);
	init_waitqueue_head(&fsm->evt_waitq);

	INIT_LIST_HEAD(&fsm->pre_notifiers);
	INIT_LIST_HEAD(&fsm->post_notifiers);

	mtk_fsm_hs_info_init(fsm);
	mdev->fsm = fsm;
	if (device_create_file(mdev->dev, &dev_attr_fsm_state))
		MTK_WARN(mdev, "Unable to create fsm_state sysfs entry\n");

	return 0;
err_create:
	devm_kfree(mdev->dev, fsm);
	return ret;
}

/**
 * mtk_fsm_exit() - free FSM control block
 * @mdev: pointer to mtk_md_dev
 *
 * Return: return value is 0 on success, a negative error code on failure.
 */
int mtk_fsm_exit(struct mtk_md_dev *mdev)
{
	struct mtk_md_fsm *fsm = mdev->fsm;
	unsigned long flags;

	if (!fsm)
		return -EINVAL;

	if (fsm->fsm_handler) {
		kthread_stop(fsm->fsm_handler);
		fsm->fsm_handler = NULL;
	}
	complete_all(&fsm->paused);

	spin_lock_irqsave(&fsm->evtq_lock, flags);
	if (WARN_ON(!list_empty(&fsm->evtq)))
		mtk_fsm_evt_cleanup(fsm, &fsm->evtq);
	spin_unlock_irqrestore(&fsm->evtq_lock, flags);

	mkt_fsm_notifier_cleanup(mdev, &fsm->pre_notifiers);
	mkt_fsm_notifier_cleanup(mdev, &fsm->post_notifiers);

	device_remove_file(mdev->dev, &dev_attr_fsm_state);

	devm_kfree(mdev->dev, fsm);
	return 0;
}

module_param(flashless_hs_flow, ushort, 0644);
MODULE_PARM_DESC(flashless_hs_flow, "To enable flashless handshake flow");

module_param(md_hs_only, ushort, 0644);
MODULE_PARM_DESC(md_hs_only, "To decide if only handshake with modem only");
