// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/firmware.h>
#include <linux/dma-mapping.h>
#include <linux/dma-heap.h>
#include "soc/xring/xring_smmu_wrapper.h"
#include <soc/xring/xr_dmabuf_helper.h>
#include "hpc_internal.h"
#include <soc/xring/dfx_switch.h>

#define HBM_ARRAY_SIZE    (4)
#define HVON_ARRAY_SIZE   (5)
const char *hbm_array_name[HBM_TYPE_MAX] = {
	"imgmem",
	"rvqueuemem",
	"rvringmem",
	"rsmem",
	"ncalmem",
	"hmdrmem",
};
static struct wakeup_source *g_npu_sys_pm_wksrc;

#if IS_ENABLED(CONFIG_XRING_DEBUG)
int hpc_log_level = HPC_LOG_LEVEL_INFO;
module_param(hpc_log_level, int, 0644);
MODULE_PARM_DESC(hpc_log_level, "Control log level (0=none, 1=info, ...)");
#endif

static bool is_wksrc_active(void)
{
	if ((g_npu_sys_pm_wksrc != NULL) && (g_npu_sys_pm_wksrc->active))
		return true;

	hpcdbg("g_npu_sys_pm_wksrc->active is false\n");
	return false;
}

static void hdev_rpc_rx_vq_work(struct work_struct *work)
{
	struct hpc_device *hdev = container_of(work, struct hpc_device,
					rpc_rx_work);

	hpc_vq_interrupt(hdev->hvdev, 0);
}

static void hdev_rpc_rx_callback(struct mbox_client *cl, void *msg)
{
	struct hpc_device *hdev = dev_get_drvdata(cl->dev);

	if (!msg)
		hpc_vq_interrupt(hdev->hvdev, 1);
	else
		queue_work(hdev->wq, &hdev->rpc_rx_work);
}

static int hdev_mbox_ch_init(struct hpc_device *hdev)
{
	struct device *dev = &hdev->pdev->dev;
	struct mbox_client *cl;

	hpcdbg("entry\n");
	if (!of_get_property(dev->of_node, "mbox-names", NULL))
		return -EFAULT;

	cl = &hdev->cl;
	cl->dev = dev;
	cl->tx_block = true;
	cl->tx_tout = 1200;
	cl->knows_txdone = false;
	cl->rx_callback = hdev_rpc_rx_callback;

	hdev->tx_ch = mbox_request_channel_byname(cl, "tx");
	if (IS_ERR(hdev->tx_ch)) {
		hpcerr("mailbox tx channel request failed\n");
		return PTR_ERR(hdev->tx_ch);
	}

	hdev->rx_ch = mbox_request_channel_byname(cl, "rx");
	if (IS_ERR(hdev->rx_ch)) {
		hpcerr("mailbox rx channel request failed\n");
		mbox_free_channel(hdev->tx_ch);
		return PTR_ERR(hdev->rx_ch);
	}
	hpcdbg("end\n");

	return 0;
}

static void hdev_mbox_ch_deinit(struct hpc_device *hdev)
{
	hpcdbg("entry\n");
	if (hdev->tx_ch)
		mbox_free_channel(hdev->tx_ch);
	if (hdev->rx_ch)
		mbox_free_channel(hdev->rx_ch);
	hdev->tx_ch = NULL;
	hdev->rx_ch = NULL;
	hpcdbg("end\n");
}

static void hdev_free_sec_mem(struct hpc_device *hdev)
{
	struct hpc_mem_info *mem;

	if (!hdev->img_buf)
		return;
	xr_dmabuf_kernel_unaccount(hdev->img_buf, XR_DMABUF_KERNEL_NPU);
	dma_heap_buffer_free(hdev->img_buf);
	hdev->img_buf = NULL;
	mem = &hdev->hbm[HBM_IMG_TYPE];
	mem->fd = 0;
	mem->va = NULL;
	mem->pa = 0;
}

static int hdev_alloc_sec_mem(struct hpc_device *hdev)
{
	struct hpc_mem_info *mem;
	struct dma_buf *dma_buf = NULL;
	u32 sfd;
	int ret = 0;

	mem = &hdev->hbm[HBM_IMG_TYPE];
	dma_buf = dma_heap_buffer_alloc(hdev->system_heap, mem->size, O_RDWR, 0);
	if (IS_ERR(dma_buf)) {
		hpcerr("dma heap alloc buf fail, ret = %ld(%pe)", PTR_ERR(dma_buf), dma_buf);
		return -ENOMEM;
	}

	xr_dmabuf_kernel_account(dma_buf, XR_DMABUF_KERNEL_NPU);
	sfd = xr_dmabuf_helper_get_sfd(dma_buf);
	if (!sfd) {
		hpcerr("get load_buf sfd failed");
		ret = -EINVAL;
		goto free_load_mem;
	}

	mem->fd = (int)sfd;
	hdev->img_buf = dma_buf;

	return ret;

free_load_mem:
	xr_dmabuf_kernel_unaccount(dma_buf, XR_DMABUF_KERNEL_NPU);
	dma_heap_buffer_free(dma_buf);

	return ret;
}

static int hdev_mem_init(struct hpc_device *hdev)
{
	struct hpc_mem_info *mem;
	void *p_log_header_addr = 0;

	hpcdbg("entry\n");

	mem = &hdev->hbm[HBM_RS_TYPE];
	p_log_header_addr = (void *)((u64)mem->va + LOG_HEADER_ADDR_OFFSET);
	if (is_switch_on(SWITCH_HMDR)) {
		hpcdbg("hmdr_nv is enable\n");
		hdev->hmdev->g_hli.hmdr_enable = HMDR_NV_ENABLE;
		memcpy(p_log_header_addr, &hdev->hmdev->g_hli, sizeof(struct hpc_log_info));
		hdev->hmdev->p_hli = (struct hpc_log_info *)p_log_header_addr;
	} else {
		hpcdbg("hmdr_nv is disenable\n");
		((struct hpc_log_info *)p_log_header_addr)->hmdr_enable = HMDR_NV_DISENABLE;
	}

	if (is_switch_on(SWITCH_NPU_WDT)) {
		hpcdbg("npu wdt is enable\n");
		((struct hpc_log_info *)p_log_header_addr)->wdt_enable = HMDR_NV_ENABLE;
	} else {
		hpcdbg("npu wdt is disenable\n");
		((struct hpc_log_info *)p_log_header_addr)->wdt_enable = HMDR_NV_DISENABLE;
	}
	((struct hpc_log_info *)p_log_header_addr)->log_drop = HMDR_NORM_LOG_DROP;//log_drop_space_init

	hpcdbg("end\n");

	return 0;
}

static void hdev_mem_deinit(struct hpc_device *hdev)
{
	hpcdbg("entry\n");
	if (hdev->hmdev) {
		hdev->hmdev->g_hli.writep = hdev->hmdev->p_hli->writep;
		hdev->hmdev->p_hli = NULL;
	}

	hpcdbg("end\n");
}

static void hdev_vdev_init(struct hpc_device *hdev)
{
	struct hpc_mem_info *mem = &hdev->hbm[HBM_RS_TYPE];
	struct hpc_virtio_share_para *hvsp = mem->va;
	struct hpc_vq_share_para *vqsp = &hvsp->hvqsp;
	struct hpc_vring_share_para *vrsp;
	struct hpc_virtio_device *hvdev = hdev->hvdev;
	u32 num;
	int i;

	hpcdbg("entry\n");
	memcpy(hvsp, &hdev->table, sizeof(hdev->table));
	mem = &hdev->hbm[HBM_RVRING_TYPE];
	for (i = 0; i < HPC_VRING_NUM_MAX; i++) {
		vrsp = &hvsp->hvrsp[i];
		vrsp->pa = mem->pa + i * vrsp->size;
		vrsp->va = (u64)mem->va + i * vrsp->size;
	}

	num = vrsp->num;
	if (!num) {
		hpcerr("vring buffer num is 0\n");
		return;
	}

	mem = &hdev->hbm[HBM_RVQUEUE_TYPE];
	vqsp->size = mem->size / (num * HPC_VRING_NUM_MAX);
	vqsp->da = mem->da;
	vqsp->pa = (u32)mem->pa;
	vqsp->va = (u64)mem->va;
	hvdev->hpc_vsp = hvsp;
	hpcdbg("end\n");
}

static int hdev_pwup(struct hpc_device *hdev)
{
	int ret;

	hpcdbg("entry\n");
	ret = regulator_enable(hdev->regulator);
	if (ret != 0)
		return ret;

	ret = xring_smmu_tcu_ctrl(&hdev->hmp_pdev->dev, XRING_SMMU_TCU_POWER_ON);
	if (ret != 0) {
		hpcerr("smmu tcu link up failed: %d\n", ret);
		goto err_tcu_linkup;
	}

	hpcdbg("end\n");
	return ret;

err_tcu_linkup:
	(void)regulator_disable(hdev->regulator);

	return ret;
}

static int hdev_pwdn(struct hpc_device *hdev)
{
	int ret;

	hpcdbg("entry\n");

	ret = xring_smmu_tcu_ctrl(&hdev->hmp_pdev->dev, XRING_SMMU_TCU_POWER_OFF);
	if (ret != 0)
		hpcerr("smmu tcu link down failed: %d\n", ret);

	if (!regulator_is_enabled(hdev->regulator)) {
		hpcdbg("hpc regulator is disable");
		return ret;
	}
	ret = regulator_disable(hdev->regulator);
	if (ret != 0)
		hpcerr("hpc subsys power off failed: %d\n", ret);

	hpcdbg("end\n");
	return ret;
}

static int hdev_rproc_init(struct hpc_device *hdev)
{
	struct hpc_mem_info *info = &hdev->hbm[HBM_NCAL_TYPE];
	int ret;

	ret = hdev_mem_init(hdev);
	if (ret != 0)
		return ret;

	ret = hdev_pwup(hdev);
	if (ret != 0)
		goto err_deinit_mem;

	ret = hdev_mbox_ch_init(hdev);
	if (ret != 0)
		goto err_pwdn_hpc;

	ret = hpc_mem_map_al_iova(info->pa, info->size, info->iova, IOMMU_READ | IOMMU_WRITE);
	if (ret != 0)
		goto err_map_al_iova;

	hdev_vdev_init(hdev);

	return 0;

err_map_al_iova:
	hdev_mbox_ch_deinit(hdev);

err_pwdn_hpc:
	(void)hdev_pwdn(hdev);

err_deinit_mem:
	hdev_mem_deinit(hdev);

	return ret;
}

static int hdev_rproc_deinit(struct hpc_device *hdev)
{
	struct hpc_mem_info *info = &hdev->hbm[HBM_NCAL_TYPE];
	int ret;

	hpcdbg("entry");
	ret = hpc_mem_unmap_al_iova(info->iova, info->size);
	if (ret != 0)
		hpcerr("hpc unmap al mem failed\n");

	hdev_mbox_ch_deinit(hdev);
	ret = hdev_pwdn(hdev);
	if (ret != 0)
		hpcerr("hpc power down failed: %d\n", ret);

	hdev_mem_deinit(hdev);
	hpcdbg("end");

	return ret;
}

int hdev_boot(struct hpc_device *hdev)
{
	int ret = 0;

	hpcdbg("hdev boot start\n");
	mutex_lock(&hdev->lock);

	if (atomic_inc_return(&hdev->power) > 1)
		goto err_unlock_mutex;

	if (!is_wksrc_active()) {
		__pm_stay_awake(g_npu_sys_pm_wksrc);
		hpcdbg("__pm_stay_awake.g_npu_sys_pm_wksrc end\n");
	}
	ret = hdev_rproc_init(hdev);
	if (ret != 0)
		goto err_dec_atomic;

	ret = hree_ca_boot(0);
	if (ret != 0)
		goto err_deinit_rproc;

	ret = hpc_vdev_boot(hdev->hvdev);
	if (ret != 0)
		goto err_pwdn_mcu;
	hpcdbg("wait for rpmsg name service create\n");
	ret = wait_for_completion_timeout(&hdev->boot, msecs_to_jiffies(60000));
	if (ret) {
		hpcdbg("wait for rpmsg completion success %d ms\n", 60000 - jiffies_to_msecs(ret));
	} else {
		hpcdbg("wait for rpmsg completion failed\n");
		ret = -ETIMEDOUT;
		goto err_vdev_shutdown;
	}

	set_npu_excp_false();
	mutex_unlock(&hdev->lock);
	hpcdbg("hdev boot end\n");

	return 0;

err_vdev_shutdown:
	hpc_vdev_shutdown(hdev->hvdev);

err_pwdn_mcu:
	(void)hree_ca_shutdown();

err_deinit_rproc:
	(void)hdev_rproc_deinit(hdev);

err_dec_atomic:
	atomic_dec(&hdev->power);
	__pm_relax(g_npu_sys_pm_wksrc);

err_unlock_mutex:
	mutex_unlock(&hdev->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(hdev_boot);

int hdev_shutdown(struct hpc_device *hdev)
{
	int ret = 0;
	struct hpc_mdr_device *hmdev = hdev->hmdev;

	hpcdbg("entry\n");
	mutex_lock(&hdev->lock);

	if (atomic_read(&hdev->power) == 0)
		goto out;

	if (!atomic_dec_and_test(&hdev->power))
		goto out;

	ret = hree_ca_shutdown();
	if (ret != 0)
		goto out;

	hpcdbg("hree ca shutdown end\n");
	if (cancel_work_sync(&hdev->rpc_rx_work))
		hpcdbg("rpc_rx_work item successfully canceled.\n");
	else
		hpcdbg("rpc_rx_work item was not in the queue.\n");

	hpc_vdev_shutdown(hdev->hvdev);
	ret = hdev_rproc_deinit(hdev);
	if (ret != 0)
		goto out;

	reinit_completion(&hdev->boot);

	if (hmdev)
		kill_fasync(&hmdev->fasync_q, SIGRTMAX, POLL_IN);

	if (is_wksrc_active()) {
		__pm_relax(g_npu_sys_pm_wksrc);
		hpcdbg("__pm_relax.g_npu_sys_pm_wksrc end\n");
	}

out:
	mutex_unlock(&hdev->lock);
	hpcdbg("end\n");

	return ret;
}
EXPORT_SYMBOL_GPL(hdev_shutdown);

int hdev_shutdown_atomic(struct hpc_device *hdev)
{
	int ret = 0;

	hpcinfo("entry\n");
	if (atomic_read(&hdev->power) == 0)
		goto out;

	ret = hpc_smc_shutdown();
out:
	hpcinfo("end\n");
	return ret;
}
EXPORT_SYMBOL_GPL(hdev_shutdown_atomic);

int hdev_resource_release(struct hpc_device *hdev)
{
	int ret = 0;

	hpcinfo("entry\n");
	mutex_lock(&hdev->lock);

	if (atomic_read(&hdev->power) == 0)
		goto out;

	ret = hree_ca_shutdown();
	if (ret != 0)
		goto out;

	if (cancel_work_sync(&hdev->rpc_rx_work))
		hpcdbg("rpc_rx_work item successfully canceled.\n");
	else
		hpcdbg("rpc_rx_work item was not in the queue.\n");

	hpc_vdev_shutdown(hdev->hvdev);
	ret = hdev_rproc_deinit(hdev);
	if (ret != 0)
		goto out;

	reinit_completion(&hdev->boot);

	atomic_set(&hdev->power, 0);
	if (is_wksrc_active()) {
		__pm_relax(g_npu_sys_pm_wksrc);
		hpcdbg("__pm_relax.g_npu_sys_pm_wksrc end\n");
	}

out:
	mutex_unlock(&hdev->lock);
	hpcinfo("end\n");
	return ret;
}
EXPORT_SYMBOL_GPL(hdev_resource_release);

bool hdev_booted(struct hpc_device *hdev)
{
	if (!hdev)
		return false;

	return atomic_read(&hdev->power) != 0;
}
EXPORT_SYMBOL_GPL(hdev_booted);

static bool hdev_rpc_kick(struct hpc_virtio_device *hvdev, int vqid)
{
	struct hpc_device *hdev = hvdev->priv;
	u32 msg[2];
	int ret;

	if (!hdev->tx_ch) {
		hpcerr("ipcm tx channel not initialized\n");
		return false;
	}

	msg[0] = 1;
	msg[1] = vqid;
	ret = mbox_send_message(hdev->tx_ch, (void *)msg);
	if (ret < 0) {
		hpcerr("send msg failed: vqid %d, ret %d\n", vqid, ret);
		return false;
	}

	return true;
}

static struct hpc_virtio_ops hvops = {
	.kick = hdev_rpc_kick,
};

static void hpc_of_unparse_heap(struct hpc_device *hdev)
{
	if (hdev->system_heap)
		dma_heap_put(hdev->system_heap);
	if (hdev->hmp_pdev)
		put_device(&hdev->hmp_pdev->dev);
}

static int hpc_of_parse_heap(struct hpc_device *hdev, struct device *dev)
{
	struct device_node *np = dev->of_node;
	struct device_node *node = NULL;
	const char *name;
	int ret;

	if (of_property_read_string(np, "mcu-heap-name", &name)) {
		hpcerr("requires mcu heap name failed\n");
		return -EINVAL;
	}

	node = of_parse_phandle(np, name, 0);
	if (!node) {
		hpcerr("can't parse \"%s\" property\n", name);
		return -EINVAL;
	}

	hdev->hmp_pdev = of_find_device_by_node(node);
	of_node_put(node);
	if (!hdev->hmp_pdev) {
		hpcerr("find mcu heap pdev %s by node failed\n", name);
		return -EINVAL;
	}

	hdev->system_heap = dma_heap_find("xring_npu_dym");
	if (IS_ERR_OR_NULL(hdev->system_heap)) {
		ret = PTR_ERR(hdev->system_heap);
		hpcerr("system head not find, ret = %d(%pe)", ret, hdev->system_heap);
		return -EINVAL;
	}

	return 0;
}

static void hpc_unregister_sub_pdev(struct hpc_device *hdev)
{
	if (hdev->hadev)
		(void)haon_exit(hdev);
	if (hdev->hmdev)
		(void)hmdr_exit(hdev);
	if (hdev->hcdev_pdev)
		platform_device_unregister(hdev->hcdev_pdev);
}

static int hpc_register_sub_pdev(struct hpc_device *hdev, struct device *dev)
{
	int ret = 0;

	hdev->hcdev_pdev = platform_device_register_data(dev, "hpc-cdev",
			PLATFORM_DEVID_AUTO, NULL, 0);
	if (IS_ERR(hdev->hcdev_pdev)) {
		hpcerr("register hcdev pdev fail\n");
		return PTR_ERR(hdev->hcdev_pdev);
	}

	if (is_switch_on(SWITCH_HMDR)) {
		hpcinfo("hmdr_nv is enable, then init hmdr\n");
		ret = hmdr_init(hdev);
		if (IS_ERR(hdev->hmdev)) {
			hpcerr("register hmdr dev failed\n");
			goto unregister_pdev;
		}
	} else {
		hpcinfo("hmdr_nv is disenable, will not init hmdr\n");
	}

	ret = haon_init(hdev);
	if (IS_ERR(hdev->hadev)) {
		hpcerr("register haon dev failed\n");
		goto unregister_pdev;
	}
	return 0;

unregister_pdev:
	hpc_unregister_sub_pdev(hdev);
	return ret;
}

static int hdev_of_node_mem_init(struct hpc_device *hdev, struct device *dev)
{
	struct device_node *np = dev->of_node;
	struct hpc_mem_info *mem = NULL;
	int ret, i;

	ret = of_property_read_u32(np, "hbmsz", &hdev->hbmsz);
	if (ret < 0) {
		hpcerr("read boot memory size failed: %d\n", ret);
		return ret;
	}
	hpcdbg("boot memory size is 0x%x\n", hdev->hbmsz);

	for (i = 0; i < HBM_TYPE_MAX; i++) {
		ret = of_property_read_u32_array(np, hbm_array_name[i],
			(u32 *)&hdev->hbm[i], HBM_ARRAY_SIZE);
		if (ret < 0) {
			hpcerr("read boot %s failed: %d\n", hbm_array_name[i], ret);
			return ret;
		}
		hpcdbg("%s, fd:%d, size:0x%x, prot:%d, da:%pK\n",
				hbm_array_name[i], hdev->hbm[i].fd, hdev->hbm[i].size,
				hdev->hbm[i].prot, (void *)(long)hdev->hbm[i].da);
	}

	hdev->hbm[HBM_NCAL_TYPE].iova = 0x1000;
	hdev->hbm_info_size = HBM_TYPE_MAX * sizeof(struct hpc_mem_info);
	mem = &hdev->hbm[HBM_IMG_TYPE];
	mem->va = dma_alloc_coherent(dev, mem->size,
			&mem->pa, GFP_KERNEL);
	if (!mem->va) {
		hpcerr("dma alloc coherent for fw mem failed\n");
		return -ENOMEM;
	}

	return 0;
}

static int hdev_of_node_vsp_init(struct hpc_virtio_share_para *table,
		struct device *dev)
{
	struct device_node *np = dev->of_node;
	struct hpc_vdev_share_para *vsp = &table->hvsp;
	struct hpc_vring_share_para *vrsp;
	int ret;

	ret = of_property_read_u32_array(np, "vdevsp", (u32 *)vsp, HVON_ARRAY_SIZE);
	if (ret < 0) {
		hpcerr("read vdevsp info failed: %d\n", ret);
		return ret;
	}

	vsp->num_vrings = HPC_VRING_NUM_MAX;
	hpcdbg("vdevsp info, id %u, notifyid 0x%x, dfeatures 0x%x ",
			vsp->id, vsp->notifyid, vsp->dfeatures);
	hpcdbg("gfeatures 0x%x, config_len 0x%x, num_vrings %u\n",
			vsp->gfeatures, vsp->config_len, vsp->num_vrings);

	vrsp = &table->hvrsp[0];
	ret = of_property_read_u32_array(np, "vring0sp", (u32 *)vrsp, HVON_ARRAY_SIZE);
	if (ret < 0) {
		hpcerr("read vring0sp info failed: %d\n", ret);
		return ret;
	}

	hpcdbg("vring0sp info, align 0x%x, num %u, notifyid 0x%x, size 0x%x, pa %pK, da %pK\n",
			vrsp->align, vrsp->num, vrsp->notifyid,
			vrsp->size, (void *)(long)vrsp->pa, (void *)(long)vrsp->da);

	vrsp = &table->hvrsp[1];
	ret = of_property_read_u32_array(np, "vring1sp", (u32 *)vrsp, HVON_ARRAY_SIZE);
	if (ret < 0) {
		hpcerr("read vring1sp info failed: %d\n", ret);
		return ret;
	}

	hpcdbg("vring1sp info, align 0x%x, num %u, notifyid 0x%x, size 0x%x, pa %pK, da %pK\n",
			vrsp->align, vrsp->num, vrsp->notifyid,
			vrsp->size, (void *)(long)vrsp->pa, (void *)(long)vrsp->da);

	return 0;
}

static int hdev_of_node_init(struct hpc_device *hdev,
		struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret;

	ret = hdev_of_node_mem_init(hdev, dev);
	if (ret < 0)
		return ret;

	ret = hdev_of_node_vsp_init(&hdev->table, dev);
	if (ret < 0)
		return ret;

	hdev->regulator = devm_regulator_get(dev, "hpc");
	if (IS_ERR_OR_NULL(hdev->regulator)) {
		hpcerr("hpc regulator get failed\n");
		return PTR_ERR(hdev->regulator);
	}

	ret = hpc_of_parse_heap(hdev, dev);
	if (ret < 0)
		return ret;

	return 0;
}

static int hpc_probe(struct platform_device *pdev)
{
	struct hpc_device *hdev = NULL;
	struct hpc_mem_info *mem, *tmp;
	int ret, i;

	hpcinfo("hpc probe!\n");
	hdev = (struct hpc_device *)__get_free_page(GFP_KERNEL);
	if (!hdev) {
		hpcerr("hpc dev get free page failed\n");
		return -ENOMEM;
	}

	memset((void *)hdev, 0, PAGE_SIZE);
	hdev->pdev = pdev;
	hdev->hmdev = NULL;
	platform_set_drvdata(pdev, hdev);
	ret = hdev_of_node_init(hdev, pdev);
	if (ret < 0) {
		hpcerr("hdev get dtsi info failed\n");
		goto err_put_hdev;
	}

	ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
	if (ret != 0) {
		hpcerr("Failed to enable 64-bit DMA.\n");
		goto err_put_hdev;
	}

	ret = hpc_register_sub_pdev(hdev, &pdev->dev);
	if (ret != 0) {
		hpcerr("register sub device failed.\n");
		goto err_put_hdev;
	}
	hdev->wq = alloc_workqueue("hpc_rx_work", WQ_HIGHPRI | WQ_UNBOUND | WQ_MEM_RECLAIM, 0);
	if (!hdev->wq) {
		hpcerr("create workqueue failed\n");
		ret = -EFAULT;
		goto err_put_sub_pdev;
	}

	INIT_WORK(&hdev->rpc_rx_work, hdev_rpc_rx_vq_work);

	hdev->hvdev = hpc_vdev_alloc(&pdev->dev, "hpc-boot", &hvops, hdev);
	if (!hdev->hvdev) {
		ret = -ENOMEM;
		goto err_put_wq;
	}

	g_npu_sys_pm_wksrc = wakeup_source_register(NULL, "npu_sys_pm_wksrc");
	if (g_npu_sys_pm_wksrc == NULL) {
		hpcerr("%s:%d x_sys_pm_wksrc create err!\n", __func__, __LINE__);
		ret = -ENOMEM;
		goto err_put_wq;
	}

	ret = hree_ca_init(&hdev->hbm[0], hdev->hbm_info_size);
	if (ret != 0)
		goto err_put_vdev;

	ret = hdev_alloc_sec_mem(hdev);
	if (ret != 0)
		goto err_put_ca_init;

	mem = &hdev->hbm[HBM_RVQUEUE_TYPE];
	mem->va = dma_alloc_coherent(&pdev->dev, hdev->hbmsz,
			&mem->pa, GFP_KERNEL);
	if (!mem->va) {
		hpcerr("dma alloc coherent for boot mem failed\n");
		goto err_alloc_sec_mem;
	}
	hpcdbg("%s memory va %pK, pa %pK\n",
			hbm_array_name[HBM_RVQUEUE_TYPE], mem->va, (void *)mem->pa);

	for (i = HBM_RVRING_TYPE; i < HBM_HMDR_TYPE; i++) {
		tmp = &hdev->hbm[i];
		tmp->va = mem->va + mem->size;
		tmp->pa = mem->pa + mem->size;
		mem = tmp;
		hpcdbg("%s memory va %pK, pa %pK\n", hbm_array_name[i], tmp->va, (void *)tmp->pa);
	}
	tmp = &hdev->hbm[i];
	hpcdbg("%s memory va %pK, pa %pK\n", hbm_array_name[i], tmp->va, (void *)tmp->pa);

	mutex_init(&hdev->lock);
	init_completion(&hdev->boot);
	atomic_set(&hdev->power, 0);
	hpcinfo("end\n");

	return 0;

err_alloc_sec_mem:
	hdev_free_sec_mem(hdev);

err_put_ca_init:
	(void)hree_ca_exit();

err_put_vdev:
	hpc_vdev_free(hdev->hvdev);

err_put_wq:
	destroy_workqueue(hdev->wq);

err_put_sub_pdev:
	hpc_unregister_sub_pdev(hdev);

err_put_hdev:
	free_page((unsigned long)hdev);

	return ret;
}

static int hpc_remove(struct platform_device *pdev)
{
	struct hpc_device *hdev = platform_get_drvdata(pdev);
	struct hpc_mem_info *mem;
	int ret, i;

	hpcinfo("hpc remove!\n");
	atomic_set(&hdev->power, 0);
	mutex_destroy(&hdev->lock);

	hdev_free_sec_mem(hdev);
	mem = &hdev->hbm[HBM_RVQUEUE_TYPE];
	if (mem->va)
		dma_free_coherent(&pdev->dev, hdev->hbmsz,
			mem->va, (dma_addr_t)mem->pa);

	for (i = HBM_RVQUEUE_TYPE; i < HBM_HMDR_TYPE; i++) {
		mem = &hdev->hbm[i];
		mem->va = NULL;
		mem->pa = 0;
	}

	ret = hree_ca_exit();
	if (ret) {
		hpcerr("ca exit failed, %d\n", ret);
		return ret;
	}

	hpc_vdev_free(hdev->hvdev);
	destroy_workqueue(hdev->wq);
	hpc_unregister_sub_pdev(hdev);
	hpc_of_unparse_heap(hdev);
	free_page((unsigned long)hdev);
	hpcinfo("hpc end!\n");

	return 0;
}

static const struct of_device_id hpc_id_table[] = {
	{ .compatible = "xring,hpc" },
	{ /* Sentinel */ },
};
MODULE_DEVICE_TABLE(of, hpc_id_table);

static struct platform_driver hpc_driver = {
	.probe = hpc_probe,
	.remove = hpc_remove,
	.driver = {
		.name = "hpc-boot",
		.of_match_table = of_match_ptr(hpc_id_table),
	},
};
module_platform_driver(hpc_driver);

MODULE_SOFTDEP("pre: hpc_mbox");
MODULE_AUTHOR("High Performance Computing Group");
MODULE_DESCRIPTION("HPC Boot Driver");
MODULE_LICENSE("GPL v2");
