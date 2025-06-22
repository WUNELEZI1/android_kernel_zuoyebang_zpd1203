// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 *
 * Description: noc exception driver
 */

#define pr_fmt(fmt)	"[xr_dfx][noc]:%s %d: " fmt, __func__, __LINE__

#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/delay.h>
#include "dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h"
#include <soc/xring/securelib/securec.h>
#include "noc.h"

enum noc_bus_error {
	NOC_BUS_ERROR_NONE = 0,
	NOC_BUS_ERROR_NON_TMO,
};

enum noc_errcode {
	ERRCODE_SLV = 0,
	ERRCODE_DEC,
	ERRCODE_UNS,
	ERRCODE_DISC,
	ERRCODE_SEC,
	ERRCODE_HIDE,
	ERRCODE_TMO,
	ERRCODE_MAXNUM
};

enum mid_list {
	NA = 0,
	LPIS_MCU,
	SENSOR_MCU,
	SENSOR_DMA,
	DSP,
	NO_5,
	ADSP_DMA,
	LPIS_DMA,
	SPI_DMA,
	CSI5,
	UFS_CTRL,
	USB,
	DP_ESM,
	PCIE0,
	PCIE1,
	GIC,
	XCTRL_CPU,
	NO_17,
	HS2,
	XRSE_M85,
	XRSE_DMA,
	CRYPTO_0,
	CRYPTO_1,
	CRYPTO_2,
	DMA_S,
	DMA_NS,
	PERF,
	AXI_BIST,
	XCTRL_DDR,
	EMMC,
	CORESIGHT,
	NO_31,
	NO_32,
	NO_33,
	UFS_ESI,
	KM,
	KRAKE,
	NO_37,
	NO_38,
	NO_39,
	LAYER_RDMA1,
	LAYER_RDMA2,
	LAYER_RDMA3,
	LAYER_RDMA4,
	LAYER_RDMA5,
	LAYER_RDMA6,
	LAYER_RDMA7,
	LAYER_RDMA8,
	LAYER_RDMA9,
	LAYER_RDMA10,
	WB_WDMA0,
	WB_WDMA1,
	DSC_RDMA0,
	DSC_RDMA1,
	CMDLIST,
	MMU_TLB,
	RDMA,
	WDMA,
	CMDLIST_RDMA,
	NO_59,
	VDEV_VPU,
	M1_TCU,
	VENC_VPU,
	ISP_MCU,
	ISP_DMA,
	M2_TCU,
	VDSP_MCU,
	VDSP_DMA,
	NPU_MCU0,
	NPU_MCU1,
	NPU_MCU2,
	NPU_MCU3,
	NPU_MCU4,
	NPU_MCU5,
	XIO_DMA,
	NPU_M85,
	NPU_DMA,
	CPU_PP,
	CPU_L,
	CPU_M0,
	CPU_M1,
	CPU_B,
	CPU_WB,
	NPU_TCU,
	M2_BIST,
	NO_85,
	DRAM_BIST,
	DRM_XRSE_M85 = 179,
	DRM_XRSE_DMA,
	DRM_XRSE_CRYPTO_0,
	DRM_XRSE_CRYPTO_1,
	DRM_XRSE_CRYPTO_2,
	DRM_KRAKE = 196,
	DRM_LAYER_RDMA1 = 200,
	DRM_LAYER_RDMA2,
	DRM_LAYER_RDMA3,
	DRM_LAYER_RDMA4,
	DRM_LAYER_RDMA5,
	DRM_LAYER_RDMA6,
	DRM_LAYER_RDMA7,
	DRM_LAYER_RDMA8,
	DRM_LAYER_RDMA9,
	DRM_LAYER_RDMA10,
	DRM_WB_WDMA0,
	DRM_WB_WDMA1,
	DRM_RDMA = 216,
	DRM_WDMA,
	DRM_VDEC = 220,
	DRM_VENC = 222,
	MID_NUMMAX
};

struct noc_exception_info {
	struct list_head list;
	u64 coreid;
	u32 modid;
	u32 errcode;
};

struct noc_mid_corid_info {
	enum mid_list mid;
	enum CORE_LIST coreid;
};

static struct noc_mid_corid_info g_noc_mid_corid_map[] = {
	{ CPU_PP, MDR_AP },
	{ CPU_L, MDR_AP },
	{ CPU_M0, MDR_AP },
	{ CPU_M1, MDR_AP },
	{ CPU_B, MDR_AP },
	{ CPU_WB, MDR_AP },
	{ DSP, MDR_AUDIO },
	{ ADSP_DMA, MDR_AUDIO },
	{ LPIS_MCU, MDR_LPM3 },
	{ SENSOR_DMA, MDR_SHUB },
	{ ISP_MCU, MDR_ISP },
	{ ISP_DMA, MDR_ISP },
	{ NPU_MCU0, MDR_NPU },
	{ NPU_MCU1, MDR_NPU },
	{ NPU_MCU2, MDR_NPU },
	{ NPU_MCU3, MDR_NPU },
	{ NPU_MCU4, MDR_NPU },
	{ NPU_MCU5, MDR_NPU },
	{ XIO_DMA, MDR_NPU },
	{ KRAKE, MDR_GPU },
	{ DRM_KRAKE, MDR_GPU },
	{ VDSP_MCU, MDR_VDSP },
	{ VDSP_DMA, MDR_VDSP },
	{ XRSE_M85, MDR_XRSE },
	{ XRSE_DMA, MDR_XRSE },
	{ CRYPTO_0, MDR_XRSE },
	{ CRYPTO_1, MDR_XRSE },
	{ CRYPTO_2, MDR_XRSE },
	{ KM, MDR_XRSE },
	{ DRM_XRSE_M85, MDR_XRSE },
	{ DRM_XRSE_DMA, MDR_XRSE },
	{ DRM_XRSE_CRYPTO_0, MDR_XRSE },
	{ DRM_XRSE_CRYPTO_1, MDR_XRSE },
	{ DRM_XRSE_CRYPTO_2, MDR_XRSE },
	{ LAYER_RDMA1, MDR_DPU },
	{ LAYER_RDMA2, MDR_DPU },
	{ LAYER_RDMA3, MDR_DPU },
	{ LAYER_RDMA4, MDR_DPU },
	{ LAYER_RDMA5, MDR_DPU },
	{ LAYER_RDMA6, MDR_DPU },
	{ LAYER_RDMA7, MDR_DPU },
	{ LAYER_RDMA8, MDR_DPU },
	{ LAYER_RDMA9, MDR_DPU },
	{ LAYER_RDMA10, MDR_DPU },
	{ WB_WDMA0, MDR_DPU },
	{ WB_WDMA1, MDR_DPU },
	{ DSC_RDMA0, MDR_DPU },
	{ DSC_RDMA1, MDR_DPU },
	{ CMDLIST, MDR_DPU },
	{ MMU_TLB, MDR_DPU },
	{ DRM_LAYER_RDMA1, MDR_DPU },
	{ DRM_LAYER_RDMA2, MDR_DPU },
	{ DRM_LAYER_RDMA3, MDR_DPU },
	{ DRM_LAYER_RDMA4, MDR_DPU },
	{ DRM_LAYER_RDMA5, MDR_DPU },
	{ DRM_LAYER_RDMA6, MDR_DPU },
	{ DRM_LAYER_RDMA7, MDR_DPU },
	{ DRM_LAYER_RDMA8, MDR_DPU },
	{ DRM_LAYER_RDMA9, MDR_DPU },
	{ DRM_LAYER_RDMA10, MDR_DPU },
	{ DRM_WB_WDMA0, MDR_DPU },
	{ DRM_WB_WDMA1, MDR_DPU },
	{ VDEV_VPU, MDR_VDEC },
	{ DRM_VDEC, MDR_VDEC },
	{ VENC_VPU, MDR_VENC },
	{ DRM_VENC, MDR_VENC },
};

static LIST_HEAD(g_noc_exception_list);
static DEFINE_SPINLOCK(g_noc_exception_list_lock);
static struct workqueue_struct *g_noc_exception_wq;
static struct work_struct g_noc_exception_w;
static bool g_noc_exception_ap;
static bool g_noc_exception_tmo;

static struct mdr_exception_info_s g_ap_noc_exception = {
	.e_modid = MODID_XR_AP_NOC,
	.e_modid_end = MODID_XR_AP_NOC,
	.e_process_priority = MDR_ERR,
	.e_reboot_priority = MDR_REBOOT_NOW,
	.e_notify_core_mask = MDR_AP,
	.e_reset_core_mask = MDR_AP,
	.e_from_core = MDR_AP,
	.e_reentrant = (u32)MDR_REENTRANT_DISALLOW,
	.e_exce_type = XR_AP_NOC,
	.e_exce_subtype = 0,
	.e_upload_flag = MDR_UPLOAD_YES,
	.e_from_module = "ap",
	.e_desc = "ap",
};

static struct noc_exception_info *get_exception_info(u64 coreid)
{
	struct noc_exception_info *info = NULL;
	struct list_head *pos = NULL;
	struct list_head *tmp = NULL;

	list_for_each_safe(pos, tmp, &g_noc_exception_list) {
		info = list_entry(pos, struct noc_exception_info, list);
		if (info->coreid == coreid)
			return info;
	}

	return NULL;
}

static u64 get_exception_coreid(u32 mid)
{
	u64 ret = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(g_noc_mid_corid_map); i++) {
		if (g_noc_mid_corid_map[i].mid == mid) {
			ret = g_noc_mid_corid_map[i].coreid;
			break;
		}
	}
	pr_info("get coreid[0x%llx]\n", ret);

	return ret;
}

void set_noc_exception_list(u32 mid, u32 err_code)
{
	struct noc_exception_info *info = NULL;
	u64 coreid;

	if (mid >= MID_NUMMAX) {
		pr_err("mid[0x%x] out of range\n", mid);
		return;
	}

	if (err_code >= ERRCODE_MAXNUM) {
		pr_err("err_code[0x%x] out of range\n", err_code);
		return;
	}

	if (g_noc_exception_tmo || g_noc_exception_ap)
		return;

	coreid = get_exception_coreid(mid);
	if (!coreid) {
		pr_err("get coreid failed\n");
		return;
	}

	if (coreid == MDR_AP) {
		g_noc_exception_ap = true;
		pr_err("ap noc excption\n");
		return;
	}

	if (err_code == ERRCODE_TMO) {
		g_noc_exception_tmo = true;
		pr_err("noc tmo excption\n");
		return;
	}

	info = get_exception_info(coreid);
	if (!info) {
		pr_err("exception info not exist\n");
		return;
	}

	info->errcode = NOC_BUS_ERROR_NON_TMO;
}

static int register_noc_modid_list(u64 coreid, u32 modid)
{
	struct noc_exception_info *info = NULL;
	unsigned long flags;

	info = get_exception_info(coreid);
	if (info != NULL) {
		pr_err("coreid 0x%llx is already registered\n", coreid);
		return 0;
	}

	info = kmalloc(sizeof(struct noc_exception_info), GFP_KERNEL);
	if (!info) {
		pr_err("kmalloc noc exception info failed\n");
		return -ENOMEM;
	}
	info->coreid = coreid;
	info->modid = modid;
	info->errcode = NOC_BUS_ERROR_NONE;

	spin_lock_irqsave(&g_noc_exception_list_lock, flags);
	list_add_tail(&info->list, &g_noc_exception_list);
	spin_unlock_irqrestore(&g_noc_exception_list_lock, flags);
	pr_info("coreid 0x%llx registered\n", coreid);
	return 0;
}

static int unregister_noc_modid_list(u64 coreid)
{
	struct noc_exception_info *info = NULL;
	unsigned long flags;

	info = get_exception_info(coreid);
	if (!info) {
		pr_err("coreid 0x%llx get info failed\n", coreid);
		return -EINVAL;
	}

	spin_lock_irqsave(&g_noc_exception_list_lock, flags);
	list_del(&info->list);
	spin_unlock_irqrestore(&g_noc_exception_list_lock, flags);

	kfree(info);
	pr_info("coreid 0x%llx unregistered\n", coreid);

	return 0;
}

int noc_register_exception(struct mdr_exception_info_s *e)
{
	int ret = 0;

	if (!e) {
		pr_err("NULL noc exception info\n");
		return -EINVAL;
	}

	if (!mdr_register_exception(e)) {
		pr_err("register noc exception failed\n");
		return -EPERM;
	}

	ret = register_noc_modid_list(e->e_from_core, e->e_modid);
	if (ret) {
		pr_err("set noc modid failed\n");
		return ret;
	}

	pr_info("register noc exception success\n");

	return ret;
}
EXPORT_SYMBOL(noc_register_exception);

int noc_unregister_exception(struct mdr_exception_info_s *e)
{
	int ret = 0;

	if (!e) {
		pr_err("NULL noc exception info\n");
		return -1;
	}

	if (mdr_unregister_exception(e->e_modid)) {
		pr_err("unregister noc exception failed\n");
		return -1;
	}

	ret = unregister_noc_modid_list(e->e_from_core);
	if (ret) {
		pr_err("clr noc modid failed\n");
		return ret;
	}

	pr_info("unregister noc exception success\n");

	return ret;
}
EXPORT_SYMBOL(noc_unregister_exception);

static void noc_exception_work_func(struct work_struct *work)
{
	struct noc_exception_info *info = NULL;
	struct list_head *pos = NULL;
	struct list_head *tmp = NULL;

	pr_info("noc exception work func\n");

	if (g_noc_exception_tmo) {
		mdr_syserr_process_for_ap(MODID_XR_AP_NOC, 0, 0);
		return;
	}

	list_for_each_safe(pos, tmp, &g_noc_exception_list) {
		info = list_entry(pos, struct noc_exception_info, list);
		if (info->errcode != NOC_BUS_ERROR_NONE) {
			if (info->modid) {
				pr_info("bus error, coreid[0x%llx], modid[0x%x]\n", info->coreid, info->modid);
				mdr_system_error(info->modid, 0, 0);
				info->errcode = NOC_BUS_ERROR_NONE;
			}
		}
	}

	pr_info("noc exception work func done\n");
}

void noc_exception_process(void)
{
	pr_info("noc exception process\n");

	if (g_noc_exception_wq == NULL) {
		pr_err("invalid exception wq\n");
		return;
	}

	if (g_noc_exception_ap) {
		pr_info("bypass noc exception process\n");
		return;
	}

	queue_work(g_noc_exception_wq, &g_noc_exception_w);
}

#if IS_ENABLED(CONFIG_XRING_NOC_DMA_EXCEPTION)
static struct dmd_dev noc_dma_dmd_dev = {
	.name = "noc_dma",
	.module_name = "DMA",
	.buff_size = DMD_MAX_LOG_SIZE,
	.fault_level = "WARNING",
};

enum mid_list dma_mid[] = {
	LPIS_DMA,
	SPI_DMA,
	CSI5,
	UFS_CTRL,
	USB,
	DP_ESM,
	PCIE0,
	PCIE1,
	DMA_S,
	DMA_NS,
	EMMC,
	LAYER_RDMA1,
	LAYER_RDMA2,
	LAYER_RDMA3,
	LAYER_RDMA4,
	LAYER_RDMA5,
	LAYER_RDMA6,
	LAYER_RDMA7,
	LAYER_RDMA8,
	LAYER_RDMA9,
	LAYER_RDMA10,
	WB_WDMA0,
	WB_WDMA1,
	DSC_RDMA0,
	DSC_RDMA1,
	RDMA,
	WDMA,
	CMDLIST_RDMA,
	DRM_LAYER_RDMA1,
	DRM_LAYER_RDMA2,
	DRM_LAYER_RDMA3,
	DRM_LAYER_RDMA4,
	DRM_LAYER_RDMA5,
	DRM_LAYER_RDMA6,
	DRM_LAYER_RDMA7,
	DRM_LAYER_RDMA8,
	DRM_LAYER_RDMA9,
	DRM_LAYER_RDMA10,
	DRM_WB_WDMA0,
	DRM_WB_WDMA1,
	DRM_RDMA,
	DRM_WDMA,
};

static bool is_dma_mid(u32 mid)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(dma_mid); i++) {
		if (mid == dma_mid[i])
			return true;
	}

	return false;
}

void noc_dma_exception(struct noc_dma_exception_s *e, const char *mid_name,
			u32 mid, u64 targetaddr, u32 opc)
{
	struct noc_dma_info info;

	if (!e) {
		pr_err("invalid noc dma exception info\n");
		return;
	}

	if (!e->is_inited) {
		pr_err("noc_dma process not init\n");
		return;
	}

	if (!mid_name) {
		pr_err("invalid mid name\n");
		return;
	}

	if (!is_dma_mid(mid))
		return;

	info.name = mid_name;
	info.mid = mid;
	info.targetaddr = targetaddr;
	info.opc = opc;

	if (!kfifo_in_spinlocked_noirqsave(&e->msg_fifo, &info, 1, &e->lock))
		pr_err("kfifo_in fail\n");
}

void noc_dma_process(struct noc_dma_exception_s *e)
{
	if (!e) {
		pr_err("invalid noc dma exception info\n");
		return;
	}

	if (!e->is_inited) {
		pr_err("noc_dma process not init\n");
		return;
	}

	if (!e->wq) {
		pr_err("invalid exception wq\n");
		return;
	}

	pr_info("dma noc exception process\n");
	queue_work(e->wq, &e->work);
}

static void noc_dma_work_func(struct work_struct *work)
{
	char noc_dma_buf[DMD_MAX_LOG_SIZE] = {0};
	struct noc_dma_info info;
	int ret;
	struct noc_dma_exception_s *e = NULL;

	pr_info("enter\n");

	if (!work) {
		pr_err("invalid work\n");
		return;
	}

	e = container_of(work, struct noc_dma_exception_s, work);

	if (!e->dmd_client) {
		pr_err("invalid dma noc dmd client\n");
		return;
	}

	while (kfifo_out_spinlocked(&e->msg_fifo, &info, 1, &e->lock)) {
		ret = snprintf_s(noc_dma_buf, DMD_MAX_LOG_SIZE, DMD_MAX_LOG_SIZE - 1,
			"NOC_DMA: %s: %d, targetaddr: 0x%llx, r/w: %d",
			info.name, info.mid, info.targetaddr, info.opc);
		if (ret < 0) {
			pr_err("snprintf_s fail\n");
			break;
		}

		dmd_client_record(e->dmd_client, DMD_NOC_DMA_ERROR_NO, "%s", noc_dma_buf);

		ret = memset_s(noc_dma_buf, DMD_MAX_LOG_SIZE, 0, DMD_MAX_LOG_SIZE);
		if (ret != EOK)
			pr_err("memset_s fail\n");

		/* wait dmd work handle one msg done */
		msleep(100);
	}

	pr_info("exit\n");
}

int noc_dma_process_init(struct noc_dma_exception_s *e)
{
	int ret = 0;

	if (!e) {
		pr_err("invalid noc dma exception info\n");
		return -EINVAL;
	}

	e->wq = create_singlethread_workqueue("noc_dma_wq");
	if (!e->wq) {
		pr_err("noc_dma_wq create fail\n");
		return -ENOMEM;
	}

	INIT_WORK(&e->work, noc_dma_work_func);
	INIT_KFIFO(e->msg_fifo);
	spin_lock_init(&e->lock);

	/* register noc_dma dmd */
	e->dmd_client = dmd_register_client(&noc_dma_dmd_dev);
	if (!e->dmd_client) {
		pr_err("noc_dma dmd register failed\n");
		ret = -ENOMEM;
		goto err;
	}

	e->is_inited = true;
	pr_info("noc_dma process init success\n");
	return 0;
err:
	destroy_workqueue(e->wq);
	e->wq = NULL;
	return ret;
}

int noc_dma_process_exit(struct noc_dma_exception_s *e)
{
	if (!e) {
		pr_err("invalid noc dma exception info\n");
		return -EINVAL;
	}

	if (!e->is_inited) {
		pr_err("noc_dma process not init\n");
		return 0;
	}

	e->is_inited = false;

	flush_workqueue(e->wq);
	destroy_workqueue(e->wq);
	e->wq = NULL;

	dmd_unregister_client(e->dmd_client, &noc_dma_dmd_dev);
	e->dmd_client = NULL;

	return 0;
}
#endif

void noc_exception_wq_init(void)
{
	g_noc_exception_wq = create_singlethread_workqueue("noc_exception_wq");
	if (g_noc_exception_wq == NULL) {
		pr_err("noc_exception_wq create fail\n");
		return;
	}

	INIT_WORK(&g_noc_exception_w, noc_exception_work_func);

	if (noc_register_exception(&g_ap_noc_exception))
		pr_err("register ap noc exception fail\n");
}
