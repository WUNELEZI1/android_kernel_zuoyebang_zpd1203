// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2022, MediaTek Inc.
 */

#include <linux/bitfield.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include "mtk_common.h"
#include "mtk_port.h"
#include "mtk_port_io.h"

#ifdef CONFIG_TX00_UT_PORT
#include "ut_port_fake.h"
#endif

#define TAG					"PORT"

#define MTK_PORT_ENUM_VER					(0)
/* this is an empirical value, negotiate with device designer */
#define MTK_PORT_ENUM_HEAD_PATTERN		(0x5a5a5a5a)
#define MTK_PORT_ENUM_TAIL_PATTERN		(0xa5a5a5a5)
#define MTK_DFLT_TRB_TIMEOUT				(5 * HZ)
#define MTK_DFLT_TRB_STATUS				(0x1)
#define PORT_ATTR_DIR_NAME_LEN			(64)
#define MTK_CHECK_RX_SEQ_MASK	(0x7fff)

#define MTK_PORT_SEARCH_FROM_RADIX_TREE(p, s) ({\
	struct mtk_port *_p;			\
	_p = rcu_dereference_raw(*(s));		\
	if (!_p)				\
		continue;			\
	p = _p;					\
})

#define MTK_PORT_INTERNAL_NODE_CHECK(p, s, i) ({\
	if (radix_tree_is_internal_node(p)) {	\
		s = radix_tree_iter_retry(&(i));\
		continue;			\
	}					\
})

static struct mtk_port_mngr_ops port_mngr_ops;
/* global group for stale ports */
static LIST_HEAD(stale_list_grp);
/* mutex lock for stale_list_group */
DEFINE_MUTEX(port_mngr_grp_mtx);

static DEFINE_IDA(ccci_dev_ids);

/* For debug priprietary loopback port */
static bool proprietary_test;
/* For debug wwan loopback port */
static bool wwan_loopback_test;
/* For debug relayfs loopback port */
static bool relayfs_loopback_test;

static const struct mtk_port_cfg port_cfg[] = {
	{FBOOT_INDEX_TX, FBOOT_INDEX_RX, VQ(13), PORT_TYPE_CHAR, "FBootDownload", PORT_F_RAW_DATA},
	{FW_INDEX_TX, FW_INDEX_RX, VQ(15), PORT_TYPE_CHAR, "FWDownload", PORT_F_RAW_DATA},

	{BROM_INDEX_TX, BROM_INDEX_RX, VQ(0), PORT_TYPE_CHAR, "BromDownload", PORT_F_RAW_DATA},
	{DUMP_INDEX_TX, DUMP_INDEX_RX, VQ(13), PORT_TYPE_CHAR, "CoreDump", PORT_F_RAW_DATA},

	{CCCI_UART2_TX, CCCI_UART2_RX, VQ(11), PORT_TYPE_WWAN, "AT", PORT_F_ALLOW_DROP},
	{CCCI_MIPC_TX, CCCI_MIPC_RX, VQ(9), PORT_TYPE_CHAR, "MIPC0", PORT_F_ALLOW_DROP},
	{CCCI_MD_LOG_TX, CCCI_MD_LOG_RX, VQ(12), PORT_TYPE_RELAYFS, "MDLog", PORT_F_DFLT},
	{CCCI_LB_IT_TX, CCCI_LB_IT_RX, VQ(7), PORT_TYPE_CHAR, "MDLB", PORT_F_ALLOW_DROP},
	{CCCI_MBIM_TX, CCCI_MBIM_RX, VQ(9), PORT_TYPE_WWAN, "MBIM", PORT_F_ALLOW_DROP},
	{CCCI_UART1_TX, CCCI_UART1_RX, VQ(8), PORT_TYPE_CHAR, "MDMETA", PORT_F_DFLT},
	{CCCI_MD_RFS_TX, CCCI_MD_RFS_RX, VQ(14), PORT_TYPE_CHAR, "MDRFS",
		PORT_F_RAW_DATA | PORT_F_FORCE_HOLD_DATA | PORT_F_FORCE_NO_SPILT_PACKET},
	{CCCI_MD_LOG_NTFY_TX, CCCI_MD_LOG_NTFY_RX, VQ(11), PORT_TYPE_CHAR, "MDLogNtfy",
		PORT_F_ALLOW_DROP},

	{CCCI_SAP_LBIT_TX, CCCI_SAP_LBIT_RX, VQ(1), PORT_TYPE_CHAR, "SAPLB", PORT_F_ALLOW_DROP},
	{CCCI_SAP_GNSS_TX, CCCI_SAP_GNSS_RX, VQ(1), PORT_TYPE_WWAN, "GNSS", PORT_F_ALLOW_DROP},
	{CCCI_SAP_META_TX, CCCI_SAP_META_RX, VQ(3), PORT_TYPE_CHAR, "SAPMETA", PORT_F_DFLT},
	{CCCI_SAP_LOG_TX, CCCI_SAP_LOG_RX, VQ(4), PORT_TYPE_CHAR, "SAPLog", PORT_F_DFLT},
	{CCCI_SAP_ADB_TX, CCCI_SAP_ADB_RX, VQ(5), PORT_TYPE_CHAR, "ADB", PORT_F_DFLT},
	{CCCI_SAP_MINIDUMP_NOTIFY_TX, CCCI_SAP_MINIDUMP_NOTIFY_RX, VQ(6), PORT_TYPE_INTERNAL,
		"minidump_notify", PORT_F_DFLT},

	{CCCI_DSS0_TX, CCCI_DSS0_RX, VQ(10), PORT_TYPE_CHAR, MTK_DSS_PORT(0), PORT_F_ALLOW_DROP},
	{CCCI_DSS1_TX, CCCI_DSS1_RX, VQ(10), PORT_TYPE_CHAR, MTK_DSS_PORT(1), PORT_F_ALLOW_DROP},
	{CCCI_DSS2_TX, CCCI_DSS2_RX, VQ(10), PORT_TYPE_CHAR, MTK_DSS_PORT(2), PORT_F_ALLOW_DROP},
	{CCCI_DSS3_TX, CCCI_DSS3_RX, VQ(10), PORT_TYPE_CHAR, MTK_DSS_PORT(3), PORT_F_ALLOW_DROP},
	{CCCI_DSS4_TX, CCCI_DSS4_RX, VQ(10), PORT_TYPE_CHAR, MTK_DSS_PORT(4), PORT_F_ALLOW_DROP},
	{CCCI_DSS5_TX, CCCI_DSS5_RX, VQ(10), PORT_TYPE_CHAR, MTK_DSS_PORT(5), PORT_F_ALLOW_DROP},
	{CCCI_DSS6_TX, CCCI_DSS6_RX, VQ(10), PORT_TYPE_CHAR, MTK_DSS_PORT(6), PORT_F_ALLOW_DROP},
	{CCCI_DSS7_TX, CCCI_DSS7_RX, VQ(10), PORT_TYPE_CHAR, MTK_DSS_PORT(7), PORT_F_ALLOW_DROP},

	{CCCI_CONTROL_TX, CCCI_CONTROL_RX, VQ(7), PORT_TYPE_INTERNAL, "MDCTRL", PORT_F_ALLOW_DROP},
	{CCCI_SAP_CONTROL_TX, CCCI_SAP_CONTROL_RX, VQ(1), PORT_TYPE_INTERNAL, "SAPCTRL",
		PORT_F_ALLOW_DROP},
	{CCCI_SAP_GCCI0_TX, CCCI_SAP_GCCI0_RX, VQ(16), PORT_TYPE_CHAR,
		MTK_GCCI_PORT(0), PORT_F_ALLOW_DROP},
	{CCCI_SAP_GCCI1_TX, CCCI_SAP_GCCI1_RX, VQ(16), PORT_TYPE_CHAR,
		MTK_GCCI_PORT(1), PORT_F_ALLOW_DROP},
	{CCCI_SAP_GCCI2_TX, CCCI_SAP_GCCI2_RX, VQ(16), PORT_TYPE_CHAR,
		MTK_GCCI_PORT(2), PORT_F_ALLOW_DROP},
	{CCCI_SAP_TIMESYNC_TX, CCCI_SAP_TIMESYNC_RX, VQ(16), PORT_TYPE_PROPRIETARY,
		"TimeSync", PORT_F_ALLOW_DROP | PORT_F_FORCE_HOLD_DATA},
	{CCCI_SAP_AGPS_TX, CCCI_SAP_AGPS_RX, VQ(16), PORT_TYPE_CHAR, "AGNSS", PORT_F_ALLOW_DROP},
	{CCCI_SAP_RFS_TX, CCCI_SAP_RFS_RX, VQ(16), PORT_TYPE_CHAR, "SAPRFS", PORT_F_ALLOW_DROP},
	{CCCI_SAP_LPM_TX, CCCI_SAP_LPM_RX, VQ(6), PORT_TYPE_CHAR, "LPM", PORT_F_DFLT},
	{CCCI_SAP_AUD_CFG_TX, CCCI_SAP_AUD_CFG_RX, VQ(18), PORT_TYPE_CHAR, "Speech", PORT_F_DFLT},
};

/* For different scenarios, change the type of specific ports */
static void mtk_port_type_change(struct mtk_port_mngr *port_mngr, struct mtk_port *port)
{
	if (port->info.rx_ch == CCCI_LB_IT_RX) {
		if (proprietary_test) {
			port->info.type = PORT_TYPE_PROPRIETARY;
			MTK_INFO(port_mngr->ctrl_blk->mdev,
				 "Port(%s) change to TYPE_PROPRIETARY\n", port->info.name);
		} else if (wwan_loopback_test) {
			port->info.type = PORT_TYPE_WWAN;
			MTK_INFO(port_mngr->ctrl_blk->mdev,
				 "Port(%s) change to TYPE_WWAN\n", port->info.name);
		} else if (relayfs_loopback_test) {
			port->info.type = PORT_TYPE_RELAYFS;
			MTK_INFO(port_mngr->ctrl_blk->mdev,
				 "Port(%s) change to RELAYFS\n", port->info.name);
		}
	}
#ifdef CONFIG_MTK_DEVLINK_COREDUMP_SUPPORT
	if (port->info.rx_ch == DUMP_INDEX_RX) {
		port->info.type = PORT_TYPE_INTERNAL;
		MTK_INFO(port_mngr->ctrl_blk->mdev,
			 "Port(%s) change to TYPE_INTERNAL\n", port->info.name);
	}
#endif

#ifdef CONFIG_MTK_DEVLINK_FLASH_SUPPORT
	if (port->info.rx_ch == FBOOT_INDEX_RX) {
		port->info.type = PORT_TYPE_INTERNAL;
		MTK_INFO(port_mngr->ctrl_blk->mdev,
			 "Port(%s) change to TYPE_INTERNAL\n", port->info.name);
	}
#endif
}

/* This function working always under mutex lock port_mngr_grp_mtx */
void mtk_port_release(struct kref *port_kref)
{
	struct mtk_stale_list *s_list;
	struct mtk_port *port;

	port = container_of(port_kref, struct mtk_port, kref);
	/* The port on stale list also be deleted when release this port */
	if (!test_bit(PORT_S_ON_STALE_LIST, &port->status))
		goto port_exit;

	list_del(&port->stale_entry);
	list_for_each_entry(s_list, &stale_list_grp, entry) {
		/* If this port is the last port of stale list, free the list and dev_id */
		if (!strncmp(s_list->dev_str, port->dev_str, MTK_DEV_STR_LEN) &&
		    list_empty(&s_list->ports) && s_list->dev_id >= 0) {
			pr_info("Free dev id of stale list(%s)\n", s_list->dev_str);
			ida_free(&ccci_dev_ids, s_list->dev_id);
			s_list->dev_id = -1;
			break;
		}
	}

port_exit:
	ports_ops[port->info.type]->exit(port);
	kfree(port);
}

static int mtk_port_tbl_add(struct mtk_port_mngr *port_mngr, struct mtk_port *port)
{
	int ret;

	ret = radix_tree_insert(&port_mngr->port_tbl[MTK_PORT_TBL_TYPE(port->info.rx_ch)],
				port->info.rx_ch & 0xFFF, port);
	if (ret)
		MTK_INFO(port_mngr->ctrl_blk->mdev, "port(%s) add to port_tbl failed, return %d\n",
			 port->info.name, ret);
	else
		port_mngr->port_cnt++;

	return ret;
}

static void mtk_port_tbl_del(struct mtk_port_mngr *port_mngr, struct mtk_port *port)
{
	radix_tree_delete(&port_mngr->port_tbl[MTK_PORT_TBL_TYPE(port->info.rx_ch)],
			  port->info.rx_ch & 0xFFF);
	port_mngr->port_cnt--;
}

static struct mtk_port *mtk_port_get_from_stale_list(struct mtk_port_mngr *port_mngr,
						     struct mtk_stale_list *s_list,
						     int rx_ch)
{
	struct mtk_port *port, *next_port;
	int ret;

	mutex_lock(&port_mngr_grp_mtx);
	list_for_each_entry_safe(port, next_port, &s_list->ports, stale_entry) {
		if (port->info.rx_ch == rx_ch) {
			kref_get(&port->kref);
			list_del(&port->stale_entry);
			ret = mtk_port_tbl_add(port_mngr, port);
			if (ret) {
				list_add_tail(&port->stale_entry, &s_list->ports);
				kref_put(&port->kref, mtk_port_release);
				mutex_unlock(&port_mngr_grp_mtx);
				MTK_INFO(port_mngr->ctrl_blk->mdev,
					 "Failed when adding (%s) to port mngr\n",
					 port->info.name);
				return ERR_PTR(ret);
			}

			port->port_mngr = port_mngr;
			clear_bit(PORT_S_ON_STALE_LIST, &port->status);
			mutex_unlock(&port_mngr_grp_mtx);
			return port;
		}
	}
	mutex_unlock(&port_mngr_grp_mtx);

	return NULL;
}

static struct mtk_port *mtk_port_alloc_or_restore(struct mtk_port_mngr *port_mngr,
						  struct mtk_port_cfg *dflt_info,
						  struct mtk_stale_list *s_list)
{
	struct mtk_port *port;
	int ret;

	port = mtk_port_get_from_stale_list(port_mngr, s_list, dflt_info->rx_ch);
	if (IS_ERR(port)) {
		/* Failed when adding to port mngr */
		return port;
	}

	if (port) {
		ports_ops[port->info.type]->reset(port);
		goto return_port;
	}

	/* This memory will be free in function "mtk_port_release", if
	 * "mtk_port_release" called by mtk_port_stale_list_grp_cleanup,
	 * we can't use "devm_free" due to no dev(struct device) entity.
	 */
	port = kzalloc(sizeof(*port), GFP_KERNEL);
	if (!port) {
		ret = -ENOMEM;
		goto exit_err;
	}

	memcpy(port, dflt_info, sizeof(*dflt_info));
	mtk_port_type_change(port_mngr, port);
	ret = mtk_port_tbl_add(port_mngr, port);
	if (ret < 0) {
		MTK_ERR(port_mngr->ctrl_blk->mdev,
			"Failed to add port(%s) to port tbl\n", dflt_info->name);
		goto free_port;
	}

	port->port_mngr = port_mngr;
	ports_ops[port->info.type]->init(port);

return_port:
	return port;
free_port:
	kfree(port);
exit_err:
	return ERR_PTR(ret);
}

static struct mtk_port *mtk_port_alloc_with_info(struct mtk_port_mngr *port_mngr,
						 struct mtk_port_cfg_ch_info *ch_info)
{
	struct mtk_port *port;
	int ret;
	int vq;

	/* This memory will be free in function "mtk_port_release",
	 * if "mtk_port_release" called by mtk_port_stale_list_grp_cleanup,
	 * we can't use "devm_free" due to no dev(struct device) entity.
	 */
	port = kzalloc(sizeof(*port), GFP_KERNEL);
	if (!port) {
		MTK_ERR(port_mngr->ctrl_blk->mdev,
			"Failed to alloc memory for port(%s)\n", ch_info->port_name);
		ret = -ENOMEM;
		goto exit_err;
	}
	MTK_HEX_DUMP(port_mngr->ctrl_blk->mdev,
		     MTK_DBG_PORT, MTK_MEMLOG_RG_9,
		     "Dumping port_cfg_ch_info:", ch_info, sizeof(*ch_info));
	MTK_INFO(port_mngr->ctrl_blk->mdev,
		 "Alloc port(%s) by unified port configuration\n", ch_info->port_name);
	port->info.rx_ch = le16_to_cpu(ch_info->dl_ch_id);
	port->info.tx_ch = le16_to_cpu(ch_info->ul_ch_id);
	strncpy(port->info.name, ch_info->port_name, ch_info->port_name_len);
	/* Matching VQ id for port */
	vq = mtk_ctrl_vq_search(port_mngr->ctrl_blk, MTK_PEER_ID(port->info.rx_ch),
				ch_info->ul_hw_queue_id, ch_info->dl_hw_queue_id);
	if (vq < 0) {
		MTK_WARN(port_mngr->ctrl_blk->mdev,
			 "Failed to search vq: request txq%d, rxq%d\n",
			 ch_info->ul_hw_queue_id, ch_info->dl_hw_queue_id);
		ret = vq;
		goto free_port;
	}
	port->info.vq_id = vq;
	port->info.type = PORT_TYPE_CHAR;

	return port;

free_port:
	kfree(port);
exit_err:

	return ERR_PTR(ret);
}

static void mtk_port_free_or_backup(struct mtk_port_mngr *port_mngr,
				    struct mtk_port *port, struct mtk_stale_list *s_list)
{
	mutex_lock(&port_mngr_grp_mtx);
	mtk_port_tbl_del(port_mngr, port);
	if (port->info.type != PORT_TYPE_INTERNAL && s_list) {
		if (test_bit(PORT_S_OPEN, &port->status)) {
			/* backup: move using ports to stale list, for no need to
			 * re-open ports after remove and plug-in device again
			 */
			list_add_tail(&port->stale_entry, &s_list->ports);
			set_bit(PORT_S_ON_STALE_LIST, &port->status);
			MTK_INFO(port->port_mngr->ctrl_blk->mdev,
				 "Port(%s) move to stale list\n", port->info.name);
			memcpy(port->dev_str, port_mngr->ctrl_blk->mdev->dev_str, MTK_DEV_STR_LEN);
			port->port_mngr = NULL;
		}
		kref_put(&port->kref, mtk_port_release);
	} else {
		mtk_port_release(&port->kref);
	}
	mutex_unlock(&port_mngr_grp_mtx);
}

static struct mtk_port *mtk_port_search_by_id(struct mtk_port_mngr *port_mngr, int rx_ch)
{
	int tbl_type = MTK_PORT_TBL_TYPE(rx_ch);

	if (tbl_type < PORT_TBL_SAP || tbl_type >= PORT_TBL_MAX)
		return NULL;

	return radix_tree_lookup(&port_mngr->port_tbl[tbl_type], MTK_CH_ID(rx_ch));
}

struct mtk_port *mtk_port_search_by_name(struct mtk_port_mngr *port_mngr, char *name)
{
	int tbl_type = PORT_TBL_SAP;
	struct radix_tree_iter iter;
	struct mtk_port *port;
	void __rcu **slot;

	do {
		radix_tree_for_each_slot(slot, &port_mngr->port_tbl[tbl_type], &iter, 0) {
			MTK_PORT_SEARCH_FROM_RADIX_TREE(port, slot);
			MTK_PORT_INTERNAL_NODE_CHECK(port, slot, iter);
			if (!strncmp(port->info.name, name, strlen(port->info.name)))
				return port;
		}
		tbl_type++;
	} while (tbl_type < PORT_TBL_MAX);
	return NULL;
}

static int mtk_port_tbl_create(struct mtk_port_mngr *port_mngr, struct mtk_port_cfg *cfg,
			       const int port_cnt, struct mtk_stale_list *s_list)
{
	struct mtk_port_cfg *dflt_port;
	struct mtk_port *port, *next_port;
	int i, ret;

	INIT_RADIX_TREE(&port_mngr->port_tbl[PORT_TBL_SAP], GFP_KERNEL);
	INIT_RADIX_TREE(&port_mngr->port_tbl[PORT_TBL_MD], GFP_KERNEL);

	/* copy ports from static port cfg table */
	for (i = 0; i < port_cnt; i++) {
		dflt_port = cfg + i;
		port = mtk_port_alloc_or_restore(port_mngr, dflt_port, s_list);
		if (IS_ERR(port)) {
			ret = PTR_ERR(port);
			goto free_ports;
		}
	}

	mutex_lock(&port_mngr_grp_mtx);
	list_for_each_entry_safe(port, next_port, &s_list->ports, stale_entry) {
		kref_get(&port->kref);
		list_del(&port->stale_entry);
		ret = mtk_port_tbl_add(port_mngr, port);
		if (ret) {
			list_add_tail(&port->stale_entry, &s_list->ports);
			kref_put(&port->kref, mtk_port_release);
			mutex_unlock(&port_mngr_grp_mtx);
			MTK_INFO(port_mngr->ctrl_blk->mdev,
				 "Failed when adding (%s) to port mngr\n",
				 port->info.name);
			goto free_ports;
		}
		port->port_mngr = port_mngr;
		clear_bit(PORT_S_ON_STALE_LIST, &port->status);
		if (port)
			ports_ops[port->info.type]->reset(port);
	}
	mutex_unlock(&port_mngr_grp_mtx);
	return 0;

free_ports:
	/* free all ports in port table after return err */
	return ret;
}

static void mtk_port_tbl_destroy(struct mtk_port_mngr *port_mngr, struct mtk_stale_list *s_list)
{
	struct mtk_port **ports;
	int tbl_type;
	int ret, idx;

	ports = kcalloc(port_mngr->port_cnt, sizeof(struct mtk_port *), GFP_KERNEL);
	/* VQ may be shared by multiple ports, we have to free or move the ports
	 * after all the ports on the VQ are closed.
	 */
	/* 1. All ports disable and send trb to close vq */
	tbl_type = PORT_TBL_SAP;
	do {
		ret = radix_tree_gang_lookup(&port_mngr->port_tbl[tbl_type],
					     (void **)ports, 0, port_mngr->port_cnt);
		for (idx = 0; idx < ret; idx++)
			ports_ops[ports[idx]->info.type]->disable(ports[idx]);
		tbl_type++;
	} while (tbl_type < PORT_TBL_MAX);

	/* 2. After all vq closed, free or backup the ports */
	tbl_type = PORT_TBL_SAP;
	do {
		ret = radix_tree_gang_lookup(&port_mngr->port_tbl[tbl_type],
					     (void **)ports, 0, port_mngr->port_cnt);
		for (idx = 0; idx < ret; idx++)
			mtk_port_free_or_backup(port_mngr, ports[idx], s_list);
		tbl_type++;
	} while (tbl_type < PORT_TBL_MAX);
	kfree(ports);
}

/**
 * mtk_port_tbl_update() - Update port radix tree table.
 * @mdev: pointer to mtk_md_dev.
 * @data: pointer to config data from device.
 *
 * This function called when host driver received a control message from device.
 *
 * Return: 0 on success and failure value on error.
 */
int mtk_port_tbl_update(struct mtk_md_dev *mdev, void *data)
{
	struct mtk_port_cfg_header *cfg_hdr = data;
	struct mtk_port_cfg_hif_info *hif_info;
	struct mtk_port_cfg_ch_info *ch_info;
	struct mtk_port_mngr *port_mngr;
	struct mtk_ctrl_blk *ctrl_blk;
	int parsed_data_len = 0;
	struct mtk_port *port;
	int ret = 0;

	if (unlikely(!mdev || !cfg_hdr)) {
		pr_err("[PORT][tbl_update] Invalid input value\n");
		ret = -EINVAL;
		goto end;
	}

	ctrl_blk = mdev->ctrl_blk;
	port_mngr = ctrl_blk->port_mngr;

	if (cfg_hdr->msg_type != PORT_CFG_MSG_REQUEST) {
		MTK_WARN(mdev, "Invalid msg_type: %d\n", cfg_hdr->msg_type);
		ret = -EPROTO;
		goto end;
	}

	if (cfg_hdr->is_enable != 1) {
		MTK_WARN(mdev, "Invalid is_enable: %d\n", cfg_hdr->is_enable);
		ret = -EPROTO;
		goto end;
	}
	MTK_HEX_DUMP(port_mngr->ctrl_blk->mdev,
		     MTK_DBG_PORT, MTK_MEMLOG_RG_9,
		     "Dumping port_cfg_header:", cfg_hdr,
		     sizeof(*cfg_hdr) + le16_to_cpu(cfg_hdr->port_config_len));

	switch (cfg_hdr->cfg_type) {
	case PORT_CFG_RPC:
	case PORT_CFG_FS:
	case PORT_CFG_SYSMSG:
		MTK_WARN(mdev, "Unsupported cfg_type: %d\n", cfg_hdr->cfg_type);
		cfg_hdr->is_enable = 0;
		break;
	case PORT_CFG_CH_INFO:
		while (parsed_data_len < le16_to_cpu(cfg_hdr->port_config_len)) {
			ch_info = (struct mtk_port_cfg_ch_info *)(cfg_hdr->data + parsed_data_len);
			parsed_data_len += sizeof(*ch_info);

			port = mtk_port_search_by_id(port_mngr, le16_to_cpu(ch_info->dl_ch_id));
			if (port) {
				continue;
			} else {
				port = mtk_port_alloc_with_info(port_mngr, ch_info);
				if (IS_ERR(port)) {
					ret = PTR_ERR(port);
					continue;
				}

				ret = mtk_port_tbl_add(port_mngr, port);
				if (ret < 0) {
					kfree(port);
					continue;
				}
				port->port_mngr = port_mngr;

				/* port->kref will be 1 after port_init */
				ports_ops[port->info.type]->init(port);
			}
		}
		cfg_hdr->msg_type = PORT_CFG_MSG_RESPONSE;
		break;
	case PORT_CFG_HIF_INFO:
		hif_info = (struct mtk_port_cfg_hif_info *)cfg_hdr->data;
		/* Clean up all the mark of the vqs before next paint, because if
		 * clean up at end of case PORT_CFG_CH_INFO, the ch_info may be
		 * NULL when cfg_hdr->port_config_len is 0, that will lead to can
		 * not get peer_id.
		 */
		mtk_ctrl_vq_color_cleanup(port_mngr->ctrl_blk, hif_info->peer_id);

		while (parsed_data_len < le16_to_cpu(cfg_hdr->port_config_len)) {
			hif_info = (struct mtk_port_cfg_hif_info *)
				   (cfg_hdr->data + parsed_data_len);
			parsed_data_len += sizeof(*hif_info);
			/* Color vq means that mark the vq to configure to the port */
			mtk_ctrl_vq_color_paint(port_mngr->ctrl_blk,
						hif_info->peer_id,
						hif_info->ul_hw_queue_id,
						hif_info->dl_hw_queue_id,
						le32_to_cpu(hif_info->ul_hw_queue_mtu),
						le32_to_cpu(hif_info->dl_hw_queue_mtu));
		}
		cfg_hdr->msg_type = PORT_CFG_MSG_RESPONSE;
		break;
	default:
		MTK_WARN(mdev, "Invalid cfg_type: %d\n", cfg_hdr->cfg_type);
		cfg_hdr->is_enable = 0;
		ret = -EPROTO;
		break;
	}

end:
	return ret;
}

static struct mtk_stale_list *mtk_port_stale_list_create(struct mtk_port_mngr *port_mngr)
{
	struct mtk_stale_list *s_list;

	/* can not use devm_kzalloc here, because should pair with the free operation which
	 * may be no dev pointer.
	 */
	s_list = kzalloc(sizeof(*s_list), GFP_KERNEL);
	if (!s_list)
		return NULL;

	memcpy(s_list->dev_str, port_mngr->ctrl_blk->mdev->dev_str, MTK_DEV_STR_LEN);
	s_list->dev_id = -1;
	INIT_LIST_HEAD(&s_list->ports);

	mutex_lock(&port_mngr_grp_mtx);
	list_add_tail(&s_list->entry, &stale_list_grp);
	mutex_unlock(&port_mngr_grp_mtx);

	return s_list;
}

static void mtk_port_stale_list_destroy(struct mtk_stale_list *s_list)
{
	mutex_lock(&port_mngr_grp_mtx);
	list_del(&s_list->entry);
	mutex_unlock(&port_mngr_grp_mtx);
	kfree(s_list);
}

static struct mtk_stale_list *mtk_port_stale_list_search(const char *dev_str)
{
	struct mtk_stale_list *tmp, *s_list = NULL;

	mutex_lock(&port_mngr_grp_mtx);
	list_for_each_entry(tmp, &stale_list_grp, entry) {
		if (!strncmp(tmp->dev_str, dev_str, MTK_DEV_STR_LEN)) {
			s_list = tmp;
			break;
		}
	}
	mutex_unlock(&port_mngr_grp_mtx);

	return s_list;
}

/**
 * mtk_port_stale_list_grp_cleanup() - Free all stale lists and all ports on it.
 *
 * This function will be called when driver will be removed. It will search all the stale lists.
 * For each stale list, it will free the stale ports, unregister the character device id region and
 * unregister tty driver structure.
 */
void mtk_port_stale_list_grp_cleanup(void)
{
	struct mtk_stale_list *s_list, *next_s_list;
	struct mtk_port *port, *next_port;

	mutex_lock(&port_mngr_grp_mtx);
	list_for_each_entry_safe(s_list, next_s_list, &stale_list_grp, entry) {
		pr_err("Clean stale list of dev_str(%s)\n", s_list->dev_str);
		list_del(&s_list->entry);

		list_for_each_entry_safe(port, next_port, &s_list->ports, stale_entry) {
			list_del(&port->stale_entry);
			mtk_port_release(&port->kref);
		}

		/* can't use devm_kfree, because the port is free,
		 * can't use port to get dev pointer
		 */
		kfree(s_list);
	}
	mutex_unlock(&port_mngr_grp_mtx);
}

static struct mtk_stale_list *mtk_port_stale_list_init(struct mtk_port_mngr *port_mngr)
{
	struct mtk_stale_list *s_list;

	s_list = mtk_port_stale_list_search(port_mngr->ctrl_blk->mdev->dev_str);
	if (!s_list) {
		MTK_INFO(port_mngr->ctrl_blk->mdev, "Create stale list\n");
		s_list = mtk_port_stale_list_create(port_mngr);
		if (unlikely(!s_list))
			return NULL;
	} else {
		MTK_INFO(port_mngr->ctrl_blk->mdev, "Reuse old stale list\n");
	}

	mutex_lock(&port_mngr_grp_mtx);
	if (s_list->dev_id < 0) {
		port_mngr->dev_id = ida_alloc_range(&ccci_dev_ids, 0,
						    MTK_DFLT_MAX_DEV_CNT - 1,
						    GFP_KERNEL);
	} else {
		port_mngr->dev_id = s_list->dev_id;
		s_list->dev_id = -1;
	}
	mutex_unlock(&port_mngr_grp_mtx);

	return s_list;
}

static void mtk_port_stale_list_exit(struct mtk_port_mngr *port_mngr, struct mtk_stale_list *s_list)
{
	if (!s_list)
		return;

	mutex_lock(&port_mngr_grp_mtx);
	if (list_empty(&s_list->ports)) {
		ida_free(&ccci_dev_ids, port_mngr->dev_id);
		mutex_unlock(&port_mngr_grp_mtx);
		mtk_port_stale_list_destroy(s_list);
		MTK_INFO(port_mngr->ctrl_blk->mdev, "Destroy stale list\n");
	} else {
		s_list->dev_id = port_mngr->dev_id;
		mutex_unlock(&port_mngr_grp_mtx);
		MTK_INFO(port_mngr->ctrl_blk->mdev, "Reserve stale list\n");
	}
}

static void mtk_port_trb_init(struct mtk_port *port, struct trb *trb, enum mtk_trb_cmd_type cmd,
			      int (*trb_complete)(struct sk_buff *skb))
{
	kref_init(&trb->kref);
	trb->vqno = port->info.vq_id;
	trb->status = MTK_DFLT_TRB_STATUS;
	trb->priv = port;
	trb->cmd = cmd;
	trb->trb_complete = trb_complete;
}

void mtk_port_trb_free(struct kref *trb_kref)
{
	struct trb *trb = container_of(trb_kref, struct trb, kref);
	struct mtk_port *port = trb->priv;
	struct sk_buff *skb;

	skb = container_of((char *)trb, struct sk_buff, cb[0]);
	if (trb->cmd == TRB_CMD_TX)
		mtk_port_free_tx_skb(port, skb);
	else
		mtk_bm_free(port->port_mngr->ctrl_blk->bm_pool, skb);
}

static int mtk_port_open_trb_complete(struct sk_buff *skb)
{
	struct trb_open_priv *trb_open_priv = (struct trb_open_priv *)skb->data;
	struct trb *trb = (struct trb *)skb->cb;
	struct mtk_port *port = trb->priv;
	struct mtk_port_mngr *port_mngr;

	port_mngr = port->port_mngr;

	if (trb->status && trb->status != -EBUSY)
		goto out;

	if (!trb->status) {
		/*The first port which opens the VQ should let port_mngr record the MTU*/
		port_mngr->vq_info[trb->vqno].tx_mtu = trb_open_priv->tx_mtu;
		port_mngr->vq_info[trb->vqno].rx_mtu = trb_open_priv->rx_mtu;
	}

	port->tx_mtu = port_mngr->vq_info[trb->vqno].tx_mtu;
	port->rx_mtu = port_mngr->vq_info[trb->vqno].rx_mtu;

	/* Minus the len of the header */
	if (!(port->info.flags & PORT_F_RAW_DATA)) {
		port->tx_mtu -= MTK_CCCI_H_ELEN;
		port->rx_mtu -= MTK_CCCI_H_ELEN;
	}

out:
	wake_up_interruptible_all(&port->trb_wq);

	MTK_INFO(port->port_mngr->ctrl_blk->mdev,
		 "Open VQ TRB:status:%d, vq:%d, port:%s, tx_mtu:%d, rx_mtu:%d, tx:0x%x, rx:0x%x\n",
		 trb->status, trb->vqno, port->info.name, port->tx_mtu, port->rx_mtu,
		 port->info.tx_ch, port->info.rx_ch);
	kref_put(&trb->kref, mtk_port_trb_free);
	return 0;
}

static int mtk_port_close_trb_complete(struct sk_buff *skb)
{
	struct trb *trb = (struct trb *)skb->cb;
	struct mtk_port *port = trb->priv;

	wake_up_interruptible_all(&port->trb_wq);
	MTK_INFO(port->port_mngr->ctrl_blk->mdev,
		 "Close VQ TRB: trb->status:%d, vq:%d, port:%s\n",
		 trb->status, trb->vqno, port->info.name);
	kref_put(&trb->kref, mtk_port_trb_free);

	return 0;
}

static int mtk_port_tx_complete(struct sk_buff *skb)
{
	struct trb *trb = (struct trb *)skb->cb;
	struct mtk_port *port = trb->priv;
	struct mtk_ccci_header *ccci_h;

	if (trb->status < 0) {
		MTK_WARN(port->port_mngr->ctrl_blk->mdev,
			 "Failed to send data: trb->status:%d, vq:%d, port:%s\n",
			 trb->status, trb->vqno, port->info.name);
		port->tx_err_cnt++;
	}

	wake_up_interruptible_all(&port->trb_wq);
	if (kref_read(&trb->kref) == 0) {
		ccci_h = (struct mtk_ccci_header *)skb->data;
		MTK_ERR(port->port_mngr->ctrl_blk->mdev, "port %s trb ref count is zero TX header:%08x %08x ",
			port->info.name, ccci_h->packet_len, ccci_h->status);
		mtk_ctrl_dump(port->port_mngr->ctrl_blk->mdev);
		MTK_BUG(port->port_mngr->ctrl_blk->mdev);
	}

	kref_put(&trb->kref, mtk_port_trb_free);

	return 0;
}

int mtk_port_status_check(struct mtk_port *port)
{
	/* If port is enable, it must on port_mngr's port_tbl, so the mdev must exist. */
	if (!test_bit(PORT_S_ENABLE, &port->status))
		return -ENODEV;

	if (!test_bit(PORT_S_OPEN, &port->status) || test_bit(PORT_S_FLUSH, &port->status) ||
	    !test_bit(PORT_S_RDWR, &port->status))
		return -EOPNOTSUPP;

	return 0;
}

/**
 * mtk_port_send_data() - send data to device through trans layer.
 * @port: pointer to channel structure for sending data.
 * @data: data to be sent.
 *
 * This function will be called by port io.
 *
 * Return:
 * * actual sent data length if success.
 * * error value if send failed.
 */
int mtk_port_send_data(struct mtk_port *port, void *data)
{
	struct mtk_port_mngr *port_mngr;
	struct mtk_ctrl_trans *trans;
	struct sk_buff *skb = data;
	bool is_blocking;
	struct trb *trb;
	int ret, len;

	port_mngr = port->port_mngr;
	trans = port_mngr->ctrl_blk->trans;

	is_blocking = !!(port->info.flags & PORT_F_BLOCKING);
	trb = (struct trb *)skb->cb;
	mtk_port_trb_init(port, trb, TRB_CMD_TX, mtk_port_tx_complete);
	len = skb->len;
	kref_get(&trb->kref); /* kref count 1->2 */

	mutex_lock(&port->write_lock);
	ret = mtk_port_status_check(port);
	if (!ret)
		ret = mtk_ctrl_trb_submit(port_mngr->ctrl_blk, skb, is_blocking);
	mutex_unlock(&port->write_lock);

	if (ret < 0) {
		kref_put(&trb->kref, mtk_port_trb_free); /* kref count 2->1 */
		mtk_port_free_tx_skb(port, skb);
		goto end;
	}

	if (!is_blocking) {
		kref_put(&trb->kref, mtk_port_trb_free);
		ret = len;
		goto end;
	}

	/*wait trb done, and no timeout in tx blocking mode*/
	ret = wait_event_interruptible_timeout(port->trb_wq,
					       trb->status <= 0 ||
					       test_bit(PORT_S_FLUSH, &port->status),
					       MTK_DFLT_TRB_TIMEOUT);

	if (ret == -ERESTARTSYS)
		ret = -EINTR;
	else if (test_bit(PORT_S_FLUSH, &port->status))
		ret = -EBUSY;
	else if (!ret)
		ret = -ETIMEDOUT;
	else
		ret = (!trb->status) ? len : trb->status;

//err_free:
	kref_put(&trb->kref, mtk_port_trb_free);

end:
	return ret;
}

static int mtk_port_check_rx_seq(struct mtk_port *port, struct mtk_ccci_header *ccci_h)
{
	u16 seq_num, assert_bit;

	seq_num = FIELD_GET(MTK_HDR_FLD_SEQ, le32_to_cpu(ccci_h->status));
	assert_bit = FIELD_GET(MTK_HDR_FLD_AST, le32_to_cpu(ccci_h->status));
	if (assert_bit && port->rx_seq &&
	    ((seq_num - port->rx_seq) & MTK_CHECK_RX_SEQ_MASK) != 1) {
		MTK_WARN(port->port_mngr->ctrl_blk->mdev,
			 "<ch: %ld> seq num out-of-order %d->%d",
			FIELD_GET(MTK_HDR_FLD_CHN, le32_to_cpu(ccci_h->status)),
			 seq_num, port->rx_seq);
		return -EPROTO;
	}

	return 0;
}

static int mtk_port_rx_dispatch(struct sk_buff *skb, int len, void *priv)
{
	struct mtk_port_mngr *port_mngr;
	struct mtk_ccci_header *ccci_h;
	struct mtk_port *port = priv;
	int ret = -EPROTO;
	u16 channel;

	if (!skb || !priv) {
		ret = -EINVAL;
		goto err_done;
	}

	port_mngr = port->port_mngr;

	/* CLDMA will not handle skb structure, so must handle here */
	skb->len = 0;
	skb_reset_tail_pointer(skb);
	skb_put(skb, len);

	/* If ccci header field has been loaded in skb data,
	 * the data should be dispatched by port manager
	 */
	if (!(port->info.flags & PORT_F_RAW_DATA)) {
		ccci_h = mtk_port_strip_header(skb);
		if (unlikely(!ccci_h))
			goto drop_data;

		MTK_DBG(port_mngr->ctrl_blk->mdev, MTK_DBG_CTRL_RX, MTK_MEMLOG_RG_10,
			"RX header:%08x %08x\n", ccci_h->packet_len, ccci_h->status);

		channel = FIELD_GET(MTK_HDR_FLD_CHN, le32_to_cpu(ccci_h->status));
		port = mtk_port_search_by_id(port_mngr, channel);
		if (unlikely(!port)) {
			MTK_WARN(port_mngr->ctrl_blk->mdev,
				 "Failed to find port by channel: %d\n", channel);
			goto drop_data;
		}

		/* The sequence number must be continuous */
		ret = mtk_port_check_rx_seq(port, ccci_h);
		if (unlikely(ret))
			goto drop_data;

		port->rx_seq = FIELD_GET(MTK_HDR_FLD_SEQ, le32_to_cpu(ccci_h->status));
	}

	ret = ports_ops[port->info.type]->recv(port, skb);

	return ret;

drop_data:
	mtk_port_free_rx_skb(port, skb);
err_done:
	return ret;
}

static void mtk_port_reset(struct mtk_port_mngr *port_mngr)
{
	int tbl_type = PORT_TBL_SAP;
	struct mtk_port **ports;
	int ret, idx;

	ports = kcalloc(port_mngr->port_cnt, sizeof(struct mtk_port *), GFP_KERNEL);
	do {
		ret = radix_tree_gang_lookup(&port_mngr->port_tbl[tbl_type],
					     (void **)ports, 0, port_mngr->port_cnt);
		for (idx = 0; idx < ret; idx++) {
			ports[idx]->enable = false;
			ports_ops[ports[idx]->info.type]->reset(ports[idx]);
		}
		tbl_type++;
	} while (tbl_type < PORT_TBL_MAX);
	kfree(ports);
}

static int mtk_port_enable(struct mtk_port_mngr *port_mngr, int tbl_type)
{
	struct mtk_port **ports;
	int ret, idx;

	ports = kcalloc(port_mngr->port_cnt, sizeof(struct mtk_port *), GFP_KERNEL);

	ret = radix_tree_gang_lookup(&port_mngr->port_tbl[tbl_type],
				     (void **)ports, 0, port_mngr->port_cnt);
	for (idx = 0; idx < ret; idx++) {
		if (ports[idx]->enable)
			ports_ops[ports[idx]->info.type]->enable(ports[idx]);
	}

	kfree(ports);
	return 0;
}

static void mtk_port_disable(struct mtk_port_mngr *port_mngr)
{
	int tbl_type = PORT_TBL_SAP;
	struct mtk_port **ports;
	int ret, idx;

	ports = kcalloc(port_mngr->port_cnt, sizeof(struct mtk_port *), GFP_KERNEL);
	do {
		ret = radix_tree_gang_lookup(&port_mngr->port_tbl[tbl_type],
					     (void **)ports, 0, port_mngr->port_cnt);
		for (idx = 0; idx < ret; idx++) {
			ports[idx]->enable = false;
			ports_ops[ports[idx]->info.type]->disable(ports[idx]);
		}
		tbl_type++;
	} while (tbl_type < PORT_TBL_MAX);
	kfree(ports);
}

static int mtk_minidump_port_skb_recv(void *data, struct sk_buff *skb)
{
	struct mtk_port *port = data;

	if (!port || !skb)
		return -EINVAL;

	/* The minidump application want to receive Netlink message
	 * when there is path info from the device.
	 */
	mtk_netlink_send_msg(MTK_NETLINK_GROUP,
			     skb->data, skb->len);

	mtk_port_free_rx_skb(port, skb);

	return 0;
}

static ssize_t mtk_post_dump_port_show(struct kobject *kobj,
				       struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "/dev/%s", "ttyDUMP");
}

static struct kobj_attribute post_dump_port_attr = {
	.attr = {
		.name = __stringify(post_dump_port),
		.mode = 0444,
	},
	.show = mtk_post_dump_port_show,
	.store = NULL,
};

static struct attribute *port_attr[] = {
	&post_dump_port_attr.attr,
	NULL,
};

static struct attribute_group pcie_attr_group = {
	.attrs = port_attr,
};

static ssize_t mtk_brom_flag_store(struct device *dev, struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct mtk_port_mngr *port_mngr;
	struct mtk_ctrl_blk *ctrl_blk;
	struct mtk_port *brom_port;
	struct pci_dev *pdev;
	char id;

	pdev = container_of(dev, struct pci_dev, dev);
	ctrl_blk = ((struct mtk_md_dev *)pci_get_drvdata(pdev))->ctrl_blk;
	port_mngr = ctrl_blk->port_mngr;
	brom_port = mtk_port_search_by_id(port_mngr, BROM_INDEX_RX);
	if (!brom_port) {
		MTK_WARN(ctrl_blk->mdev, "Failed to find brom dl port\n");
		return count;
	}

	id = *buf - '0';
	switch (id) {
	case 1:
		MTK_INFO(ctrl_blk->mdev, "Set brom dl flag\n");
		brom_dl_flag = 1;
		break;
	case 2:
		MTK_INFO(ctrl_blk->mdev, "Clear brom dl flag\n");
		brom_dl_flag = 0;
		break;
	default:
		MTK_WARN(ctrl_blk->mdev, "Invalid buf value: %s\n", buf);
		break;
	}

	return count;
}

static DEVICE_ATTR_WO(mtk_brom_flag);

#define READ_BROM_CMD_SLEEP_TIME 50
static void mtk_brom_port_start_cmd(struct mtk_port *port)
{
	if (port->info.rx_ch != BROM_INDEX_RX)
		return;

	mtk_port_vq_enable(port);
	set_bit(PORT_S_ENABLE, &port->status);
	set_bit(PORT_S_OPEN, &port->status);
	set_bit(PORT_S_RDWR, &port->status);
	/* "Enter start" cmd */
	mtk_port_send_brom_cmd(port, 0xa0);
	if (mtk_port_read_brom_cmd_ack(port, 0x5f, READ_BROM_CMD_SLEEP_TIME))
		goto disable_port;
	mtk_port_send_brom_cmd(port, 0x0a);
	if (mtk_port_read_brom_cmd_ack(port, 0xf5, READ_BROM_CMD_SLEEP_TIME))
		goto disable_port;
	mtk_port_send_brom_cmd(port, 0x50);
	if (mtk_port_read_brom_cmd_ack(port, 0xaf, READ_BROM_CMD_SLEEP_TIME))
		goto disable_port;
	mtk_port_send_brom_cmd(port, 0x05);
	if (mtk_port_read_brom_cmd_ack(port, 0xfa, READ_BROM_CMD_SLEEP_TIME))
		goto disable_port;
	/* "Jump bl" cmd */
	mtk_port_send_brom_cmd(port, 0xd6);
	if (mtk_port_read_brom_cmd_ack(port, 0xd6, READ_BROM_CMD_SLEEP_TIME))
		goto disable_port;
	if (mtk_port_read_brom_cmd_ack(port, 0x00, READ_BROM_CMD_SLEEP_TIME))
		goto disable_port;
	if (mtk_port_read_brom_cmd_ack(port, 0x00, READ_BROM_CMD_SLEEP_TIME))
		goto disable_port;
	if (mtk_port_read_brom_cmd_ack(port, 0x00, READ_BROM_CMD_SLEEP_TIME))
		goto disable_port;
	if (mtk_port_read_brom_cmd_ack(port, 0x00, READ_BROM_CMD_SLEEP_TIME))
		goto disable_port;

disable_port:
	clear_bit(PORT_S_RDWR, &port->status);
	clear_bit(PORT_S_OPEN, &port->status);
	clear_bit(PORT_S_ENABLE, &port->status);
	mtk_port_vq_disable(port);
}

static int mtk_port_attr_init(struct mtk_port_mngr *port_mngr)
{
	struct mtk_md_dev *mdev = port_mngr->ctrl_blk->mdev;
	char name[PORT_ATTR_DIR_NAME_LEN];
	int ret;

	snprintf(name, PORT_ATTR_DIR_NAME_LEN, "mtk_wwan_%x_pcie", mdev->hw_ver);
	port_mngr->port_attr_kobj = kobject_create_and_add(name, kernel_kobj);

	/* Create the files associated with this kobject */
	ret = sysfs_create_group(port_mngr->port_attr_kobj, &pcie_attr_group);
	if (ret && ret != -EINVAL) {
		MTK_ERR(mdev, "Failed to create port attr, ret=%d\n", ret);
		goto err_create_file;
	}

	/* Create attribute file for bromDL */
	ret = device_create_file(mdev->dev, &dev_attr_mtk_brom_flag);
	if (ret) {
		MTK_ERR(mdev, "Unable to create brom_flag sysfs entry\n");
		sysfs_remove_group(port_mngr->port_attr_kobj, &pcie_attr_group);
		goto err_create_file;
	}

	return 0;

err_create_file:
	kobject_put(port_mngr->port_attr_kobj);
	return ret;
}

static int mtk_port_attr_exit(struct mtk_port_mngr *port_mngr)
{
	struct mtk_md_dev *mdev = port_mngr->ctrl_blk->mdev;

	device_remove_file(mdev->dev, &dev_attr_mtk_brom_flag);

	if (port_mngr->port_attr_kobj) {
		sysfs_remove_group(port_mngr->port_attr_kobj, &pcie_attr_group);
		MTK_INFO(mdev, "Remove port attr file\n");
		kobject_put(port_mngr->port_attr_kobj);
		port_mngr->port_attr_kobj = NULL;
	} else {
		MTK_INFO(mdev, "Port attr file is not exist\n");
	}

	return 0;
}

/**
 * mtk_port_add_header() - Add mtk_ccci_header to TX packet.
 * @skb: pointer to socket buffer
 *
 * This function is called by trb sevice. And it will help to
 * add mtk_ccci_header data to the head of skb->data.
 *
 * Return:
 * * 0:		successfully add, already added or no need ccci header
 * * -EINVAL:	input parameter or members in input is illegal
 */
int mtk_port_add_header(struct sk_buff *skb)
{
	struct mtk_ccci_header *ccci_h;
	struct mtk_port *port;
	struct trb *trb;
	int ret = 0;

	trb = (struct trb *)skb->cb;
	if (trb->status == 0xADDED)
		goto end;

	port = trb->priv;
	if (!port) {
		ret = -EINVAL;
		goto end;
	}
	if (port->info.flags & PORT_F_RAW_DATA) {
		MTK_DBG(port->port_mngr->ctrl_blk->mdev, MTK_DBG_CTRL_TX, MTK_MEMLOG_RG_10,
			"Port %s is raw data mode, no need to addheader\n", port->info.name);
		goto end;
	}

	/* Port layer have reserved data length of ccci_head at the skb head */
	ccci_h = skb_push(skb, sizeof(*ccci_h));

	ccci_h->packet_header = cpu_to_le32(0);
	ccci_h->packet_len = cpu_to_le32(skb->len);
	ccci_h->ex_msg = cpu_to_le32(0);
	ccci_h->status = cpu_to_le32(FIELD_PREP(MTK_HDR_FLD_CHN, port->info.tx_ch) |
				     FIELD_PREP(MTK_HDR_FLD_SEQ, port->tx_seq++) |
				     FIELD_PREP(MTK_HDR_FLD_AST, 1));

	MTK_DBG(port->port_mngr->ctrl_blk->mdev, MTK_DBG_CTRL_TX, MTK_MEMLOG_RG_10,
		"TX header:%08x %08x\n", ccci_h->packet_len, ccci_h->status);

	trb->status = 0xADDED;
end:
	return ret;
}

/**
 * mtk_port_strip_header() - remove mtk_ccci_header from RX packet.
 * @skb: pointer to socket buffer.
 *
 * This function will help to remove mtk_ccci_header data from the head of skb->data.
 * But it will not check if the data of skb head is mtk_ccci_header actually.
 *
 * Return:
 * * ccci_h:	pointer to mtk_ccci_header stripped from socket buffer.
 * * NULL:	data length is invalid.
 */
struct mtk_ccci_header *mtk_port_strip_header(struct sk_buff *skb)
{
	struct mtk_ccci_header *ccci_h;

	if (skb->len < sizeof(*ccci_h)) {
		pr_err_ratelimited("[PORT][strip_header] Invalid input value, length:%d\n",
				   skb->len);
		return NULL;
	}

	ccci_h = (struct mtk_ccci_header *)skb->data;
	skb_pull(skb, sizeof(*ccci_h));

	return ccci_h;
}

/**
 * mtk_port_status_update() - Update ports enumeration information.
 * @mdev: pointer to mtk_md_dev.
 * @data: pointer to mtk_port_enum_msg, which brings enumeration information.
 *
 * This function called when host driver is doing handshake.
 * Structure mtk_port_enum_msg brings ports' enumeration information
 * from modem, and this function handles it and set "enable" of mtk_port
 * to "true" or "false".
 *
 * This function can sleep or can be called from interrupt context.
 *
 * Return:
 * * 0:		success to update ports' status
 * * -EINVAL:	input parameter or members in input structure is illegal
 * * -EPROTO:	message content error
 */
int mtk_port_status_update(struct mtk_md_dev *mdev, void *data)
{
	struct mtk_port_enum_msg *msg = data;
	struct mtk_port_info *port_info;
	struct mtk_port_mngr *port_mngr;
	struct mtk_ctrl_blk *ctrl_blk;
	struct mtk_port *port;
	int port_id;
	int ret = 0;
	u16 ch_id;

	if (unlikely(!mdev || !msg)) {
		pr_err("[PORT][status_update] Invalid input value, mdev:%pK, msg:%pK\n",
		       mdev, msg);
		ret = -EINVAL;
		goto end;
	}

	ctrl_blk = mdev->ctrl_blk;
	port_mngr = ctrl_blk->port_mngr;
	if (le16_to_cpu(msg->version) != MTK_PORT_ENUM_VER) {
		ret = -EPROTO;
		goto end;
	}

	if (le32_to_cpu(msg->head_pattern) != MTK_PORT_ENUM_HEAD_PATTERN ||
	    le32_to_cpu(msg->tail_pattern) != MTK_PORT_ENUM_TAIL_PATTERN) {
		ret = -EPROTO;
		goto end;
	}

	for (port_id = 0; port_id < le16_to_cpu(msg->port_cnt); port_id++) {
		port_info = (struct mtk_port_info *)(msg->data +
						   (sizeof(*port_info) * port_id));
		if (!port_info) {
			MTK_ERR(mdev, "Invalid port info, the index %d\n", port_id);
			ret = -EINVAL;
			goto end;
		}
		ch_id = FIELD_GET(MTK_INFO_FLD_CHID, le16_to_cpu(port_info->channel));
		port = mtk_port_search_by_id(port_mngr, ch_id);
		if (!port) {
			MTK_ERR(mdev, "Failed to find the port 0x%X\n", ch_id);
			continue;
		}
		port->enable = FIELD_GET(MTK_INFO_FLD_EN, le16_to_cpu(port_info->channel));
	}
end:
	return ret;
}

/**
 * mtk_port_mngr_vq_status_check() - Checking VQ status before enable or disable VQ.
 * @skb: pointer to socket buffer
 *
 * This function called before enable or disable VQ, check the VQ status by calculate
 * count of ports which have enabled the VQ.
 *
 * Return:
 * * 0:		first user for enable or last user for disable
 * * -EBUSY:	current VQ is occupied by other ports
 * * -EINVAL:	error command
 */
int mtk_port_mngr_vq_status_check(struct sk_buff *skb)
{
	struct trb *trb = (struct trb *)skb->cb;
	struct trb_open_priv *trb_open_priv;
	struct mtk_port *port = trb->priv;
	struct mtk_port_mngr *port_mngr;
	int ret = 0;

	port_mngr = port->port_mngr;
	switch (trb->cmd) {
	case TRB_CMD_ENABLE:
		port_mngr->vq_info[trb->vqno].port_cnt++;
		if (port_mngr->vq_info[trb->vqno].port_cnt == 1) {
			trb_open_priv = (struct trb_open_priv *)skb->data;
			trb_open_priv->rx_done = mtk_port_rx_dispatch;
			break;
		}

		trb->status = -EBUSY;
		trb->trb_complete(skb);
		ret = -EBUSY;
		break;
	case TRB_CMD_DISABLE:
		if (port_mngr->vq_info[trb->vqno].port_cnt > 0) {
			port_mngr->vq_info[trb->vqno].port_cnt--;
			if (!port_mngr->vq_info[trb->vqno].port_cnt)
				break;
		}
		MTK_INFO(port_mngr->ctrl_blk->mdev,
			 "VQ(%d) still has %d port, skip to handle close skb\n",
		trb->vqno, port_mngr->vq_info[trb->vqno].port_cnt);
		trb->status = -EBUSY;
		trb->trb_complete(skb);
		ret = -EBUSY;
		break;
	default:
		MTK_ERR(port_mngr->ctrl_blk->mdev, "Invalid trb command(%d)\n", trb->cmd);
		ret = -EINVAL;
		break;
	}
	return ret;
}

/**
 * mtk_port_vq_enable() - Function for enable virtual queue.
 * @port: pointer to channel structure for sending data.
 *
 * This function will be called when enable/create port.
 *
 * Return:
 * * trb->status if success.
 * * error value if fail.
 */
int mtk_port_vq_enable(struct mtk_port *port)
{
	struct mtk_port_mngr *port_mngr = port->port_mngr;
	struct sk_buff *skb;
	int ret = -ENOMEM;
	struct trb *trb;

	skb = mtk_bm_alloc(port_mngr->ctrl_blk->bm_pool);
	if (!skb) {
		MTK_ERR(port->port_mngr->ctrl_blk->mdev,
			"Failed to alloc skb of port(%s)\n", port->info.name);
		goto end;
	}
	skb_put(skb, sizeof(struct trb_open_priv));
	trb = (struct trb *)skb->cb;
	mtk_port_trb_init(port, trb, TRB_CMD_ENABLE, mtk_port_open_trb_complete);
	kref_get(&trb->kref);

	ret = mtk_ctrl_trb_submit(port_mngr->ctrl_blk, skb, true);
	if (ret) {
		MTK_ERR(port_mngr->ctrl_blk->mdev,
			"Failed to submit trb for port(%s), ret=%d\n", port->info.name, ret);
		kref_put(&trb->kref, mtk_port_trb_free);
		mtk_port_trb_free(&trb->kref);
		goto end;
	}

start_wait:
	/* wait trb done */
	ret = wait_event_interruptible_timeout(port->trb_wq, trb->status <= 0,
					       MTK_DFLT_TRB_TIMEOUT);
	if (ret == -ERESTARTSYS)
		goto start_wait;
	else if (!ret)
		ret = -ETIMEDOUT;
	else
		ret = trb->status;

	kref_put(&trb->kref, mtk_port_trb_free);

end:
	return ret;
}

/**
 * mtk_port_vq_disable() - Function for disable virtual queue.
 * @port: pointer to channel structure for sending data.
 *
 * This function will be called when disable/destroy port.
 *
 * Return:
 * * trb->status if success.
 * * error value if fail.
 */
int mtk_port_vq_disable(struct mtk_port *port)
{
	struct mtk_port_mngr *port_mngr = port->port_mngr;
	struct sk_buff *skb;
	int ret = -ENOMEM;
	struct trb *trb;

	skb = mtk_bm_alloc(port_mngr->ctrl_blk->bm_pool);
	if (!skb) {
		MTK_ERR(port_mngr->ctrl_blk->mdev,
			"Failed to alloc skb of port(%s)\n", port->info.name);
		goto end;
	}
	skb_put(skb, sizeof(struct trb_open_priv));
	trb = (struct trb *)skb->cb;
	mtk_port_trb_init(port, trb, TRB_CMD_DISABLE, mtk_port_close_trb_complete);
	kref_get(&trb->kref);

	mutex_lock(&port->write_lock);
	ret = mtk_ctrl_trb_submit(port_mngr->ctrl_blk, skb, true);
	mutex_unlock(&port->write_lock);
	if (ret) {
		MTK_ERR(port_mngr->ctrl_blk->mdev,
			"Failed to submit trb for port(%s), ret=%d\n", port->info.name, ret);
		kref_put(&trb->kref, mtk_port_trb_free);
		mtk_port_trb_free(&trb->kref);
		goto end;
	}

start_wait:
	/* wait trb done (must wait until close vq done) */
	ret = wait_event_interruptible(port->trb_wq, trb->status <= 0);
	if (ret == -ERESTARTSYS)
		goto start_wait;

	ret = trb->status;
	kref_put(&trb->kref, mtk_port_trb_free);

end:
	return ret;
}

static void mtk_port_mngr_vqs_enable(struct mtk_port_mngr *port_mngr)
{
	int tbl_type = PORT_TBL_SAP;
	struct mtk_port **ports;
	int ret, idx;

	ports = kcalloc(port_mngr->port_cnt, sizeof(struct mtk_port *), GFP_KERNEL);
	do {
		ret = radix_tree_gang_lookup(&port_mngr->port_tbl[tbl_type],
					     (void **)ports, 0, port_mngr->port_cnt);
		for (idx = 0; idx < ret; idx++) {
			ports[idx]->tx_seq = 0;
			/* After MDEE, cldma reset rx_seq start at 1, not 0 */
			ports[idx]->rx_seq = 0;

			if (!ports[idx]->enable || !test_bit(PORT_S_ENABLE, &ports[idx]->status))
				continue;
			if (tbl_type == PORT_TBL_SAP) {
				mtk_port_vq_enable(ports[idx]);
				set_bit(PORT_S_RDWR, &ports[idx]->status);
			}
		}
		tbl_type++;
	} while (tbl_type < PORT_TBL_MAX);
	kfree(ports);
}

static void mtk_port_mngr_vqs_disable_by_type(struct mtk_port_mngr *port_mngr, int tbl_type)
{
	struct mtk_port **ports;
	int ret, idx;

	ports = kcalloc(port_mngr->port_cnt, sizeof(struct mtk_port *), GFP_KERNEL);

	ret = radix_tree_gang_lookup(&port_mngr->port_tbl[tbl_type],
				     (void **)ports, 0, port_mngr->port_cnt);
	for (idx = 0; idx < ret; idx++) {
		if (!ports[idx]->enable || !test_bit(PORT_S_ENABLE, &ports[idx]->status))
			continue;

		/* Disable R/W after VQ close because device
		 * is removed suddenly or start to sleep.
		 */
		mutex_lock(&ports[idx]->write_lock);
		clear_bit(PORT_S_RDWR, &ports[idx]->status);
		mutex_unlock(&ports[idx]->write_lock);
		mtk_port_vq_disable(ports[idx]);
	}
	kfree(ports);
}

static void mtk_port_mngr_vqs_disable(struct mtk_port_mngr *port_mngr)
{
	int tbl_type = PORT_TBL_SAP;

	do {
		mtk_port_mngr_vqs_disable_by_type(port_mngr, tbl_type);
		tbl_type++;
	} while (tbl_type < PORT_TBL_MAX);
}

static void mtk_port_mngr_mdee_port_enable(struct mtk_port_mngr *port_mngr)
{
	struct mtk_port *port;
	int ret;

	port = mtk_port_search_by_id(port_mngr, CCCI_CONTROL_RX);
	if (port) {
		port->enable = true;
		if (test_bit(PORT_S_ENABLE, &port->status)) {
			set_bit(PORT_S_RDWR, &port->status);
			mtk_port_vq_enable(port);
		} else {
			ret = ports_ops[port->info.type]->enable(port);
		}
	} else {
		MTK_ERR(port_mngr->ctrl_blk->mdev, "Failed to find MD ctrl port\n");
	}

	port = mtk_port_search_by_id(port_mngr, CCCI_MD_LOG_RX);
	if (port) {
		port->enable = true;
		if (test_bit(PORT_S_ENABLE, &port->status)) {
			set_bit(PORT_S_RDWR, &port->status);
			mtk_port_vq_enable(port);
		} else {
			ret = ports_ops[port->info.type]->enable(port);
		}
	} else {
		MTK_ERR(port_mngr->ctrl_blk->mdev, "Failed to find MD LOG port\n");
	}

	port = mtk_port_search_by_id(port_mngr, CCCI_MD_LOG_NTFY_RX);
	if (port) {
		port->enable = true;
		if (test_bit(PORT_S_ENABLE, &port->status)) {
			set_bit(PORT_S_RDWR, &port->status);
			mtk_port_vq_enable(port);
		} else {
			ret = ports_ops[port->info.type]->enable(port);
		}
	} else {
		MTK_ERR(port_mngr->ctrl_blk->mdev, "Failed to find MD LOG NTFY port\n");
	}

	port = mtk_port_search_by_id(port_mngr, CCCI_UART1_RX);
	if (port) {
		port->enable = true;
		if (test_bit(PORT_S_ENABLE, &port->status)) {
			set_bit(PORT_S_RDWR, &port->status);
			mtk_port_vq_enable(port);
		} else {
			ret = ports_ops[port->info.type]->enable(port);
		}
	} else {
		MTK_ERR(port_mngr->ctrl_blk->mdev, "Failed to find MDMETA port\n");
	}

	port = mtk_port_search_by_id(port_mngr, CCCI_MD_RFS_RX);
	if (port) {
		port->enable = true;
		if (test_bit(PORT_S_ENABLE, &port->status)) {
			set_bit(PORT_S_RDWR, &port->status);
			mtk_port_vq_enable(port);
		} else {
			ret = ports_ops[port->info.type]->enable(port);
		}
	} else {
		MTK_ERR(port_mngr->ctrl_blk->mdev, "Failed to find MDRFS port\n");
	}
}

/**
 * mtk_port_mngr_fsm_state_handler() - Handle fsm event after state has been changed.
 * @fsm_param: pointer to mtk_fsm_param, which including fsm state and event.
 * @arg: fsm will pass mtk_port_mngr structure back by using this parameter.
 *
 * This function will be registered to fsm by control block. If registered successful,
 * after fsm state has been changed, the fsm will call this function.
 *
 * This function can sleep or can be called from interrupt context.
 *
 * Return: No return value.
 */
void mtk_port_mngr_fsm_state_handler(struct mtk_fsm_param *fsm_param, void *arg)
{
	struct mtk_port_mngr *port_mngr;
	struct mtk_port *port;
	int evt_id;
	int flag;

	if (!fsm_param || !arg) {
		pr_err("[PORT][fsm_handler] Invalid input value, fsm_param:%pK, arg:%pK\n",
		       fsm_param, arg);
		return;
	}

	port_mngr = arg;
	evt_id = fsm_param->evt_id;
	flag = fsm_param->fsm_flag;

	switch (fsm_param->to) {
	case FSM_STATE_ON:
		if (evt_id == FSM_EVT_REINIT)
			mtk_port_reset(port_mngr);
		break;
	case FSM_STATE_DOWNLOAD:
		if (flag & FSM_F_DL_PORT_CREATE) {
			/* If in reinit flow, other ports may be enabled */
			mtk_port_disable(port_mngr);
			port = mtk_port_search_by_id(port_mngr, BROM_INDEX_RX);
			if (!port) {
				MTK_ERR(port_mngr->ctrl_blk->mdev, "Failed to find brom dl port\n");
				goto end;
			}

			if (brom_dl_flag) {
				/* Change to blocking mode to ensure accuracy */
				port->info.flags |= PORT_F_BLOCKING;
				ports_ops[port->info.type]->enable(port);
			} else {
				port->info.flags &= ~PORT_F_BLOCKING;
				mtk_brom_port_start_cmd(port);
			}
		} else if (flag & FSM_F_DL_DA) {
			port = mtk_port_search_by_id(port_mngr, BROM_INDEX_RX);
			if (!port) {
				MTK_ERR(port_mngr->ctrl_blk->mdev, "Failed to find brom dl port\n");
				goto end;
			}
			/* Change to non-blocking mode to ensure speed */
			port->info.flags &= ~PORT_F_BLOCKING;
		} else if (flag & FSM_F_DL_JUMPBL || flag & FSM_F_DL_TIMEOUT) {
			port = mtk_port_search_by_id(port_mngr, BROM_INDEX_RX);
			if (!port) {
				MTK_ERR(port_mngr->ctrl_blk->mdev, "Failed to find brom dl port\n");
				goto end;
			}
			ports_ops[port->info.type]->disable(port);
		} else if (flag & FSM_F_DL_FB) {
			port = mtk_port_search_by_id(port_mngr, FBOOT_INDEX_RX);
			if (!port) {
				MTK_ERR(port_mngr->ctrl_blk->mdev,
					"Failed to find fastboot download port\n");
				goto end;
			}
			ports_ops[port->info.type]->enable(port);
		} else if  (flag & FSM_F_DL_PL) {
			/* BROM DL port may be still enabled due to jump to bl missing.
			 * Disable BROM DL port firstly if it's enabled.
			 */
			port = mtk_port_search_by_id(port_mngr, BROM_INDEX_RX);
			if (!port) {
				MTK_ERR(port_mngr->ctrl_blk->mdev,
					"Failed to find brom dl port\n");
				goto end;
			}
			if (test_bit(PORT_S_ENABLE, &port->status))
				ports_ops[port->info.type]->disable(port);

			port = mtk_port_search_by_id(port_mngr, FW_INDEX_RX);
			if (!port) {
				MTK_ERR(port_mngr->ctrl_blk->mdev,
					"Failed to find firmware download port\n");
				goto end;
			}
			ports_ops[port->info.type]->enable(port);
		} else if (flag & FSM_F_DL_JUMPLK) {
			port = mtk_port_search_by_id(port_mngr, FW_INDEX_RX);
			if (!port) {
				MTK_ERR(port_mngr->ctrl_blk->mdev,
					"Failed to find fireware download port\n");
				goto end;
			}
			ports_ops[port->info.type]->disable(port);
		} else {
			MTK_WARN(port_mngr->ctrl_blk->mdev, "Not support flag %d\n", flag);
		}
		break;
	case FSM_STATE_POSTDUMP:
		mtk_port_disable(port_mngr);
		port = mtk_port_search_by_id(port_mngr, DUMP_INDEX_RX);
		if (!port) {
			MTK_ERR(port_mngr->ctrl_blk->mdev, "Failed to find dump port\n");
			goto end;
		}
		ports_ops[port->info.type]->enable(port);
		break;
	case FSM_STATE_BOOTUP:
		if (flag & FSM_F_MD_HS_START) {
			port = mtk_port_search_by_id(port_mngr, CCCI_CONTROL_RX);
			if (!port) {
				MTK_ERR(port_mngr->ctrl_blk->mdev, "Failed to find MD ctrl port\n");
				goto end;
			}
			ports_ops[port->info.type]->enable(port);
		} else if (flag & FSM_F_SAP_HS_START) {
			port = mtk_port_search_by_id(port_mngr, FW_INDEX_RX);
			if (port)
				ports_ops[port->info.type]->disable(port);

			port = mtk_port_search_by_id(port_mngr, CCCI_SAP_CONTROL_RX);
			if (!port) {
				MTK_ERR(port_mngr->ctrl_blk->mdev,
					"Failed to find sAP ctrl port\n");
				goto end;
			}
			ports_ops[port->info.type]->enable(port);
		} else if (flag & FSM_F_MD_HS2_DONE) {
			port = mtk_port_search_by_id(port_mngr, CCCI_MD_RFS_RX);
			if (!port) {
				MTK_ERR(port_mngr->ctrl_blk->mdev, "Failed to find MD RFS port\n");
				goto end;
			}
			if (port->enable)
				ports_ops[port->info.type]->enable(port);
		} else if (flag & FSM_F_SAP_HS2_DONE) {
			/* Enable timesync port by default */
			port = mtk_port_search_by_id(port_mngr, CCCI_SAP_TIMESYNC_RX);
			if (port)
				port->enable = true;
			/* Enable minidump port by default */
			port = mtk_port_search_by_id(port_mngr, CCCI_SAP_MINIDUMP_NOTIFY_RX);
			if (port)
				port->enable = true;
			mtk_port_enable(port_mngr, PORT_TBL_SAP);
		}
		break;
	case FSM_STATE_READY:
		port = mtk_port_search_by_id(port_mngr, FBOOT_INDEX_RX);
		if (port)
			ports_ops[port->info.type]->disable(port);
		port = mtk_port_search_by_id(port_mngr, BROM_INDEX_RX);
		if (port)
			ports_ops[port->info.type]->disable(port);
		port = mtk_port_search_by_id(port_mngr, DUMP_INDEX_RX);
		if (port)
			ports_ops[port->info.type]->disable(port);

		mtk_port_enable(port_mngr, PORT_TBL_MD);

		/* The internal port "minidump_notify" opened by port mngr as user */
		port = mtk_port_internal_open(port_mngr->ctrl_blk->mdev, "minidump_notify", 0);
		mtk_port_internal_recv_register(port, &mtk_minidump_port_skb_recv, port);
		break;
	case FSM_STATE_OFF:
		mtk_port_disable(port_mngr);
		break;
	case FSM_STATE_MDEE:
		if (flag & FSM_F_MDEE_INIT) {
			mtk_port_mngr_vqs_disable_by_type(port_mngr, PORT_TBL_MD);
			mtk_port_mngr_mdee_port_enable(port_mngr);
		} else if (flag & FSM_F_MDEE_CLEARQ_DONE) {
			/* the time 2000ms recommended by device-end
			 * it's for wait device prepares the data
			 */
			msleep(2000);
			mtk_port_mngr_vqs_disable(port_mngr);
		} else if (flag & FSM_F_MDEE_ALLQ_RESET) {
			mtk_port_mngr_vqs_enable(port_mngr);
			mtk_port_mngr_mdee_port_enable(port_mngr);
		}
		break;
	default:
		break;
	}
end:
	return;
}

static int mtk_port_mngr_suspend(struct mtk_port_mngr *port_mngr)
{
	port_mngr->in_suspend = true;
	return 0;
}

static int mtk_port_mngr_resume(struct mtk_port_mngr *port_mngr)
{
	port_mngr->in_suspend = false;
	return 0;
}

/**
 * mtk_port_mngr_init() - Initialize mtk_port_mngr and mtk_stale_list.
 * @ctrl_blk: pointer to mtk_ctrl_blk.
 *
 * This function called after trans layer complete initialization.
 * Structure mtk_port_mngr is main body responsible for port management;
 * and this function alloc memory for it.
 * If port manager can't find stale list in stale list group by
 * using dev_str, it will also alloc memory for structure mtk_stale_list.
 * And then it will initialize port table and register fsm callback.
 *
 * Return:
 * * 0:			-success to initialize mtk_port_mngr
 * * -ENOMEM:	-alloc memory for structure failed
 */
int mtk_port_mngr_init(struct mtk_ctrl_blk *ctrl_blk)
{
	struct mtk_port_mngr *port_mngr;
	struct mtk_stale_list *s_list;
	int ret = -ENOMEM;

	port_mngr = devm_kzalloc(ctrl_blk->mdev->dev, sizeof(*port_mngr), GFP_KERNEL);
	if (unlikely(!port_mngr)) {
		MTK_ERR(ctrl_blk->mdev, "Failed to alloc memory for port_mngr\n");
		goto end;
	}

	/* 1.Init port manager basic fields */
	port_mngr->ctrl_blk = ctrl_blk;

	/* 2.Init mtk_stale_list or re-use old one */
	s_list = mtk_port_stale_list_init(port_mngr);
	if (!s_list) {
		MTK_ERR(ctrl_blk->mdev, "Failed to init mtk_stale_list\n");
		goto free_port_mngr;
	}

	/* 3.Put default ports and stale ports to port table */
	ret = mtk_port_tbl_create(port_mngr, (struct mtk_port_cfg *)port_cfg,
				  ARRAY_SIZE(port_cfg), s_list);
	if (unlikely(ret)) {
		MTK_ERR(ctrl_blk->mdev, "Failed to create port_tbl\n");
		goto exit_stale_list;
	}

	/* 4.Init port attribute file */
	ret = mtk_port_attr_init(port_mngr);
	if (unlikely(ret)) {
		MTK_ERR(ctrl_blk->mdev, "Failed to init port attribute file\n");
		goto err_destroy_table;
	}

	port_mngr->ops = &port_mngr_ops;
	port_mngr->in_suspend = false;

	ctrl_blk->port_mngr = port_mngr;
	MTK_INFO(ctrl_blk->mdev, "Initialize port_mngr successfully\n");

	return ret;

err_destroy_table:
exit_stale_list:
	mtk_port_tbl_destroy(port_mngr, s_list);
	mtk_port_stale_list_exit(port_mngr, s_list);
free_port_mngr:
	devm_kfree(ctrl_blk->mdev->dev, port_mngr);
end:
	return ret;
}

/**
 * mtk_port_mngr_exit() - Free the structure mtk_port_mngr.
 * @ctrl_blk: pointer to mtk_ctrl_blk.
 *
 * This function called before trans layer start to exit.
 * It will destroy port table and stale list, free port manager entity.
 * If there are ports that are opened, move these ports to stale list
 * and free the rest ports; if there are ports that are all closed,
 * then also free stale list.
 *
 * Return: No return value.
 */
void mtk_port_mngr_exit(struct mtk_ctrl_blk *ctrl_blk)
{
	struct mtk_port_mngr *port_mngr = ctrl_blk->port_mngr;
	struct mtk_stale_list *s_list;

	s_list = mtk_port_stale_list_search(port_mngr->ctrl_blk->mdev->dev_str);

	/* 1.exit port attribute file*/
	mtk_port_attr_exit(port_mngr);
	/* 2.free or backup ports, then destroy port table */
	mtk_port_tbl_destroy(port_mngr, s_list);
	/* 3.destroy stale list or backup register info to it */
	mtk_port_stale_list_exit(port_mngr, s_list);
	/* 4.free port_mngr structure */
	devm_kfree(ctrl_blk->mdev->dev, port_mngr);
	ctrl_blk->port_mngr = NULL;
	MTK_INFO(ctrl_blk->mdev, "Exit port_mngr successfully\n");
}

static struct mtk_port_mngr_ops port_mngr_ops = {
	.suspend = mtk_port_mngr_suspend,
	.resume = mtk_port_mngr_resume,
};

module_param(proprietary_test, bool, 0644);
MODULE_PARM_DESC(proprietary_test, "This value is used to test kernel API\n");
module_param(wwan_loopback_test, bool, 0644);
MODULE_PARM_DESC(wwan_loopback_test, "This value is used to test wwan port loopback\n");
module_param(relayfs_loopback_test, bool, 0644);
MODULE_PARM_DESC(relayfs_loopback_test, "This value is used to test wwan port loopback\n");
