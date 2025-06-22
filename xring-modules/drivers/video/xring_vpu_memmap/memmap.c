// SPDX-License-Identifier: GPL-2.0
/**
 * xring dec support smmu
 *
 */
#include <linux/miscdevice.h>
#include <linux/module.h>
#include "memmap.h"

#define MEM_DEVICE_NAME "xring_vpu_memmap"

/* Flag that indicate device is set for dec/enc */
#define XRING_VPU_ENC				(1 << 0)
#define XRING_VPU_DEC				(1 << 1)

static struct device *venc_dev;
static struct device *vdec_dev;
static DEFINE_MUTEX(mlist_lock);
static struct list_head *total_head;

MODULE_IMPORT_NS(DMA_BUF);

int vsi_kloglvl = LOGLVL_INFO;
module_param(vsi_kloglvl, int, 0644);

struct vpu_iova_dma_buf {  // each map iova will malloc the strcut
	struct list_head list;
	struct dma_buf *dma_buf;
	struct dma_buf_attachment *attachment;
	struct sg_table *sgt;
	unsigned long iova;
	int len;
	int fd;
	atomic_t ref_count;
};

struct vpu_instance_node {  // instance data
	struct list_head list;
	struct mutex dbuf_lock;
	struct list_head dbuf_list;
};

int memmap_set_device(struct device *dev, int flag)
{

	if (flag & XRING_VPU_DEC)
		vdec_dev = dev;
	else if (flag & XRING_VPU_ENC)
		venc_dev = dev;
	else {
		memmap_klog(LOGLVL_ERROR, "Invalid flag\n");
		return -EINVAL;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(memmap_set_device);

static struct vpu_iova_dma_buf *vpu_dma_buf_find(int fd, struct vpu_instance_node *priv_data)
{
	struct vpu_iova_dma_buf *itr = NULL;

	list_for_each_entry(itr, &priv_data->dbuf_list, list) {
		if (itr->fd == fd)
			return itr;
	}
	return NULL;
}

static int vpu_smmu_map_iova(struct device *dev, struct vpu_iova_dma_buf *priv, int fd)
{
	priv->dma_buf = dma_buf_get(fd);
	if (IS_ERR(priv->dma_buf)) {
		memmap_klog(LOGLVL_ERROR, "dma_buf_get fail\n");
		return -EINVAL;
	}

	priv->attachment = dma_buf_attach(priv->dma_buf, dev);
	if (IS_ERR(priv->attachment)) {
		memmap_klog(LOGLVL_ERROR, "dma_buf_attach fail\n");
		goto fail_put;
	}

	priv->sgt = dma_buf_map_attachment(priv->attachment, DMA_BIDIRECTIONAL);
	if (IS_ERR(priv->sgt)) {
		memmap_klog(LOGLVL_ERROR, "dma_buf_map_attachment fail\n");
		goto fail_detach;
	}

	priv->fd = fd;
	priv->iova = sg_dma_address(priv->sgt->sgl);
	priv->len = sg_dma_len(priv->sgt->sgl);
	memmap_klog(LOGLVL_DEBUG, "fd = %d, iova = 0x%lx, len = 0x%x, dma_bf=0x%lx, ref=%ld\n",
			priv->fd, priv->iova, priv->len, (unsigned long)priv->dma_buf, atomic_long_read(&priv->dma_buf->file->f_count));
	return 0;

fail_detach:
	dma_buf_detach(priv->dma_buf, priv->attachment);
fail_put:
	dma_buf_put(priv->dma_buf);

	return -EINVAL;
}

static void vpu_smmu_unmap_iova(struct vpu_iova_dma_buf *priv)
{
	memmap_klog(LOGLVL_DEBUG, "fd = %d, iova = 0x%lx, len = 0x%x, dma_bf=0x%lx, ref=%ld\n",
			priv->fd, priv->iova, priv->len, (unsigned long)priv->dma_buf, atomic_long_read(&priv->dma_buf->file->f_count));
	dma_buf_unmap_attachment(priv->attachment, priv->sgt, DMA_BIDIRECTIONAL);
	dma_buf_detach(priv->dma_buf, priv->attachment);
	dma_buf_put(priv->dma_buf);
}

static int vpu_map_iova(struct vpu_dma_buf *dma_buf, struct device *dev, struct vpu_instance_node *priv_data)
{
	struct vpu_iova_dma_buf *priv;

	if (dma_buf->fd < 0) {
		memmap_klog(LOGLVL_ERROR, "userapace fd is illeagal\n\n");
		return -EINVAL;
	}

	mutex_lock(&priv_data->dbuf_lock);
	priv = vpu_dma_buf_find(dma_buf->fd, priv_data);
	if (priv) {
		atomic_inc(&priv->ref_count);
		dma_buf->iova = priv->iova;
		mutex_unlock(&priv_data->dbuf_lock);
		memmap_klog(LOGLVL_WARNING, "same fd to mmap, directly return\n");
		return 0;
	}

	priv = kzalloc(sizeof(struct vpu_iova_dma_buf), GFP_KERNEL);
	if (!priv) {
		memmap_klog(LOGLVL_ERROR, "kzalloc for struct vpu_iova_dma_buf fail\n");
		mutex_unlock(&priv_data->dbuf_lock);
		return -ENOMEM;
	}

	if (vpu_smmu_map_iova(dev, priv, dma_buf->fd)) {
		memmap_klog(LOGLVL_ERROR, "smmu_map_iova fail\n");
		kfree(priv);
		mutex_unlock(&priv_data->dbuf_lock);
		return -EINVAL;
	}

	dma_buf->iova = priv->iova; // pass iova to user
	list_add_tail(&priv->list, &priv_data->dbuf_list);
	atomic_inc(&priv->ref_count);
	mutex_unlock(&priv_data->dbuf_lock);
	memmap_klog(LOGLVL_DEBUG, "smmu_map_iova success\n");
	return 0;
}

static int vpu_unmap_iova(int fd, struct vpu_instance_node *priv_data)
{
	struct vpu_iova_dma_buf *priv = NULL;

	if (fd < 0) {
		memmap_klog(LOGLVL_ERROR, "userapace fd is illeagal\n");
		return -EINVAL;
	}

	mutex_lock(&priv_data->dbuf_lock);
	priv = vpu_dma_buf_find(fd, priv_data);
	if (!priv) {
		memmap_klog(LOGLVL_ERROR, "vpu_dma_buf_find can't find fd : %d\n", fd);
		mutex_unlock(&priv_data->dbuf_lock);
		return -EINVAL;
	}
	mutex_unlock(&priv_data->dbuf_lock);

	if (atomic_dec_and_test(&priv->ref_count)) {
		vpu_smmu_unmap_iova(priv);
		mutex_lock(&priv_data->dbuf_lock);
		list_del(&priv->list);
		mutex_unlock(&priv_data->dbuf_lock);
		kfree(priv);
	}

	return 0;
}

static int vpu_memmap_open(struct inode *inode, struct file *filp)
{
	struct vpu_instance_node *priv_data;

	priv_data = kzalloc(sizeof(*priv_data), GFP_KERNEL);
	if (!priv_data) {
		memmap_klog(LOGLVL_ERROR, "kzalloc for struct vpu_instance_node fail");
		return -ENOMEM;
	}

	mutex_init(&priv_data->dbuf_lock);
	INIT_LIST_HEAD(&priv_data->dbuf_list);
	filp->private_data = priv_data;
	mutex_lock(&mlist_lock);
	list_add_tail(&priv_data->list, total_head);
	mutex_unlock(&mlist_lock);
	memmap_klog(LOGLVL_DEBUG, "module opened\n");
	return 0;
}

static void memmap_clear(struct vpu_instance_node *priv_data)
{
	int i = 0;
	struct vpu_iova_dma_buf *itr = NULL, *temp = NULL;

	memmap_klog(LOGLVL_DEBUG, "Start process clear...\n");
	mutex_lock(&priv_data->dbuf_lock);
	list_for_each_entry_safe(itr, temp, &priv_data->dbuf_list, list) {
		if (itr->fd > 0) {
			list_del(&itr->list);
			memmap_klog(LOGLVL_DEBUG, "i = %d, fd = %d, len = %d, iova = 0x%zx\n", ++i,
					itr->fd, itr->len, itr->iova);
			vpu_smmu_unmap_iova(itr);
			kfree(itr);
			memmap_klog(LOGLVL_DEBUG, "success umap and free\n");
		} else
			memmap_klog(LOGLVL_ERROR, "should not hanppen\n");
	}
	mutex_unlock(&priv_data->dbuf_lock);

	memmap_klog(LOGLVL_DEBUG, "End process clear\n");
}

static int vpu_memmap_release(struct inode *inode, struct file *filp)
{
	struct vpu_instance_node *priv_data;

	priv_data = filp->private_data;
	if (!priv_data)
		return -EINVAL;

	memmap_clear(priv_data);
	mutex_lock(&mlist_lock);
	list_del(&priv_data->list);
	mutex_unlock(&mlist_lock);
	kfree(priv_data);
	filp->private_data = NULL;
	memmap_klog(LOGLVL_DEBUG, "module released\n");
	return 0;
}

static long vpu_memmap_ioctl(struct file *filp, unsigned int cmd,
			   unsigned long arg)
{
	int ret = 0;
	struct vpu_dma_buf dma_buf;
	struct device *dev;
	struct vpu_instance_node *priv_data;

	priv_data = filp->private_data;
	if (unlikely(!priv_data))
		return -EFAULT;

	memmap_klog(LOGLVL_DEBUG, "ioctl cmd 0x%08x\n", cmd);
	if (_IOC_TYPE(cmd) != MEMMAP_IOC_MAGIC)
		return -ENOTTY;
	if (_IOC_NR(cmd) > MEMMAP_IOC_MAXNR)
		return -ENOTTY;

	switch (cmd) {
	case MEMMAP_IOVA:
		if (copy_from_user(&dma_buf, (struct vpu_dma_buf __user *)arg,
						    sizeof(struct vpu_dma_buf))) {
			memmap_klog(LOGLVL_ERROR, "copy_from_user fail\n");
			return -EFAULT;
		}
		if ((dma_buf.flag & XRING_VPU_DEC) && vdec_dev != NULL) {
			dev = vdec_dev;
		} else if ((dma_buf.flag & XRING_VPU_ENC) && venc_dev != NULL) {
			dev = venc_dev;
		} else {
			memmap_klog(LOGLVL_ERROR, "Invalid flag\n");
			return -EFAULT;
		}
		ret = vpu_map_iova(&dma_buf, dev, priv_data);
		if (ret) {
			memmap_klog(LOGLVL_ERROR, "vpu_map_iova fail\n");
			return ret;
		}
		if (copy_to_user((struct vpu_dma_buf __user *)arg,
				&dma_buf, sizeof(struct vpu_dma_buf))) {
			memmap_klog(LOGLVL_ERROR, "copy_to_user fail\n");
			return -EFAULT;
		}
		break;
	case MEMUNMAP_IOVA:
		if (copy_from_user(&dma_buf, (struct vpu_dma_buf __user *)arg,
						    sizeof(struct vpu_dma_buf))) {
			memmap_klog(LOGLVL_ERROR, "copy_from_user fail\n");
			return -EFAULT;
		}
		ret = vpu_unmap_iova(dma_buf.fd, priv_data);
		if (ret) {
			memmap_klog(LOGLVL_ERROR, "vpu_unmap_iova fail\n");
			return ret;
		}
		break;
	case MEMMAP_ABNORMAL:
		memmap_clear(priv_data);
		break;
	default:
		memmap_klog(LOGLVL_WARNING, "unexpected cmd %d\n", cmd);
		return -EINVAL;
	}
	return ret;
}

const struct file_operations vpu_memmap_fops = {
	.owner = THIS_MODULE,
	.open = vpu_memmap_open,
	.release = vpu_memmap_release,
	.unlocked_ioctl = vpu_memmap_ioctl,
};

static struct miscdevice vpu_memmap_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = MEM_DEVICE_NAME,
	.fops = &vpu_memmap_fops,
};

static int __init vpu_memmap_init(void)
{
	int ret;

	ret = misc_register(&vpu_memmap_misc);
	if (ret < 0) {
		memmap_klog(LOGLVL_ERROR, "misc_register fail\n");
		return ret;
	}
	total_head = kzalloc(sizeof(struct list_head), GFP_KERNEL);
	if (!total_head) {
		memmap_klog(LOGLVL_ERROR, "kzalloc for total_head fail\n");
		return -ENOMEM;
	}

	mutex_init(&mlist_lock);
	INIT_LIST_HEAD(total_head);
	memmap_klog(LOGLVL_INFO, "module init success\n");
	return ret;
}

static void __exit vpu_memmap_exit(void)
{
	mutex_destroy(&mlist_lock);
	kfree(total_head);
	misc_deregister(&vpu_memmap_misc);
	memmap_klog(LOGLVL_INFO, "module exit success\n");
}

int get_dma_buf_by_iova(unsigned long iova, unsigned long *offset, struct dma_buf **dma_buf)
{
	struct vpu_instance_node *itr_m = NULL;
	struct vpu_iova_dma_buf *itr = NULL;

	if (!total_head)
		return -EINVAL;

	mutex_lock(&mlist_lock);
	list_for_each_entry(itr_m, total_head, list) {
		mutex_lock(&itr_m->dbuf_lock);
		list_for_each_entry(itr, &itr_m->dbuf_list, list) {
			if (itr->iova <= iova && itr->iova + itr->len > iova) {
				*offset = iova - itr->iova;
				*dma_buf = itr->dma_buf;
				mutex_unlock(&itr_m->dbuf_lock);
				mutex_unlock(&mlist_lock);
				return 0;
			}
		}
		mutex_unlock(&itr_m->dbuf_lock);
	}
	mutex_unlock(&mlist_lock);
	return -EINVAL;
}
EXPORT_SYMBOL(get_dma_buf_by_iova);

module_init(vpu_memmap_init);
module_exit(vpu_memmap_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xring developer");
MODULE_DESCRIPTION("driver module for XRING vpu map iova");
