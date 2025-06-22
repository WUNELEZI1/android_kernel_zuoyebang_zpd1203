// SPDX-License-Identifier: GPL-2.0
/*
 * Dummy Smmu Driver
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/iommu.h>
#include <linux/of_address.h>
#include <linux/amba/bus.h>
#include <linux/pci.h>

struct dummy_smmu_device {
	struct iommu_device iommu;
	struct device *dev;
#define ARM_SMMU_FEAT_2_LVL_STRTAB      (1 << 0)
#define ARM_SMMU_FEAT_2_LVL_CDTAB       (1 << 1)
#define ARM_SMMU_FEAT_TT_LE             (1 << 2)
#define ARM_SMMU_FEAT_TT_BE             (1 << 3)
#define ARM_SMMU_FEAT_PRI               (1 << 4)
#define ARM_SMMU_FEAT_ATS               (1 << 5)
#define ARM_SMMU_FEAT_SEV               (1 << 6)
#define ARM_SMMU_FEAT_MSI               (1 << 7)
#define ARM_SMMU_FEAT_COHERENCY         (1 << 8)
#define ARM_SMMU_FEAT_TRANS_S1          (1 << 9)
#define ARM_SMMU_FEAT_TRANS_S2          (1 << 10)
#define ARM_SMMU_FEAT_STALLS            (1 << 11)
#define ARM_SMMU_FEAT_HYP               (1 << 12)
#define ARM_SMMU_FEAT_STALL_FORCE       (1 << 13)
#define ARM_SMMU_FEAT_VAX               (1 << 14)
#define ARM_SMMU_FEAT_RANGE_INV         (1 << 15)
#define ARM_SMMU_FEAT_BTM               (1 << 16)
#define ARM_SMMU_FEAT_SVA               (1 << 17)
#define ARM_SMMU_FEAT_E2H               (1 << 18)
	u32 features;
};

struct dummy_smmu_domain {
	struct dummy_smmu_device *smmu;
	struct iommu_domain domain;
	struct mutex init_mutex;/* mutex when change context of domain */

	struct list_head devices;/* list of dummy_smmu_master */
	spinlock_t devices_lock;

	struct rb_root map_info_root;
	struct mutex map_info_lock;
};

struct dummy_smmu_master {
	struct device *dev;
	struct dummy_smmu_device *smmu;
	struct dummy_smmu_domain *domain;
	struct list_head domain_head;
};

struct xring_map_info {
	struct rb_node node;

	unsigned long iova;
	phys_addr_t paddr;
	size_t pgsize;
};

struct lookup_iommu_group_data {
	struct device_node *np;
	struct iommu_group *group;
};

static struct iommu_ops dummy_smmu_ops;

static inline struct dummy_smmu_domain *to_smmu_domain(struct iommu_domain *dom)
{
	return container_of(dom, struct dummy_smmu_domain, domain);
}

static bool dummy_smmu_capable(enum iommu_cap cap)
{
	switch (cap) {
	case IOMMU_CAP_CACHE_COHERENCY:
		return true;
	case IOMMU_CAP_NOEXEC:
		return true;
	default:
		return false;
	}
}

static struct iommu_domain *dummy_smmu_domain_alloc(unsigned int type)
{
	struct dummy_smmu_domain *smmu_domain;

	if (type != IOMMU_DOMAIN_UNMANAGED &&
		type != IOMMU_DOMAIN_DMA &&
		type != IOMMU_DOMAIN_DMA_FQ &&
		type != IOMMU_DOMAIN_IDENTITY)
		return NULL;

	smmu_domain = kzalloc(sizeof(struct dummy_smmu_domain), GFP_KERNEL);
	if (!smmu_domain)
		return NULL;

	mutex_init(&smmu_domain->init_mutex);
	INIT_LIST_HEAD(&smmu_domain->devices);
	spin_lock_init(&smmu_domain->devices_lock);
	mutex_init(&smmu_domain->map_info_lock);

	return &smmu_domain->domain;
}

static void dummy_smmu_domain_free(struct iommu_domain *domain)
{
	struct dummy_smmu_domain *smmu_domain = to_smmu_domain(domain);

	kfree(smmu_domain);
}

static int dummy_smmu_domain_finalise(struct iommu_domain *domain,
					struct dummy_smmu_master *master)
{
	return 0;
}

static void dummy_smmu_detach_dev(struct dummy_smmu_master *master)
{
	unsigned long flags;
	struct dummy_smmu_domain *smmu_domain = master->domain;

	if (!smmu_domain)
		return;

	spin_lock_irqsave(&smmu_domain->devices_lock, flags);
	list_del(&master->domain_head);
	spin_unlock_irqrestore(&smmu_domain->devices_lock, flags);

	master->domain = NULL;
}

static int dummy_smmu_attach_dev(struct iommu_domain *domain, struct device *dev)
{
	struct dummy_smmu_device *smmu;
	struct dummy_smmu_master *master;
	struct dummy_smmu_domain *smmu_domain = to_smmu_domain(domain);
	unsigned long flags;
	int ret;

	master = dev_iommu_priv_get(dev);
	smmu = master->smmu;
	dummy_smmu_detach_dev(master);

	mutex_lock(&smmu_domain->init_mutex);
	if (!smmu_domain->smmu) {
		smmu_domain->smmu = smmu;
		ret = dummy_smmu_domain_finalise(domain, master);
		if (ret) {
			smmu_domain->smmu = NULL;
			goto out_unlock;
		}
	} else if (smmu_domain->smmu != smmu) {
		dev_err(dev,
			"cannot attach to SMMU %s (upstream of %s)\n",
			dev_name(smmu_domain->smmu->dev),
			dev_name(smmu->dev));
		ret = -ENXIO;
		goto out_unlock;
	}

	master->domain = smmu_domain;

	spin_lock_irqsave(&smmu_domain->devices_lock, flags);
	list_add(&master->domain_head, &smmu_domain->devices);
	spin_unlock_irqrestore(&smmu_domain->devices_lock, flags);

out_unlock:
	mutex_unlock(&smmu_domain->init_mutex);
	return 0;
}

static struct xring_map_info *dummy_smmu_map_info_find(
			struct iommu_domain *domain, unsigned long iova)
{
	struct dummy_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct dummy_smmu_device *smmu = smmu_domain->smmu;
	struct rb_root *root = &smmu_domain->map_info_root;
	struct rb_node **p = &root->rb_node;
	struct rb_node *parent = NULL;
	struct xring_map_info *entry;

	mutex_lock(&smmu_domain->map_info_lock);

	while (*p) {
		parent = *p;
		entry = rb_entry(parent, struct xring_map_info, node);

		if (iova < entry->iova) {
			p = &(*p)->rb_left;
		} else if (iova > entry->iova) {
			if (iova < (entry->iova + entry->pgsize))
				goto find;
			else
				p = &(*p)->rb_right;
		} else {
			goto find;
		}
	}

	mutex_unlock(&smmu_domain->map_info_lock);
	return NULL;

find:
	mutex_unlock(&smmu_domain->map_info_lock);
	dev_info(smmu->dev, "%s iova=0x%llx, paddr=0x%llx, pgsize=0x%x\n",
			__func__, entry->iova, entry->paddr, entry->pgsize);
	return entry;
}

static void dummy_smmu_map_info_add(struct iommu_domain *domain,
			unsigned long iova, phys_addr_t paddr, size_t pgsize)
{
	struct dummy_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct dummy_smmu_device *smmu = smmu_domain->smmu;
	struct rb_root *root = &smmu_domain->map_info_root;
	struct rb_node **p = &root->rb_node;
	struct rb_node *parent = NULL;
	struct xring_map_info *entry, *new;

	mutex_lock(&smmu_domain->map_info_lock);

	while (*p) {
		parent = *p;
		entry = rb_entry(parent, struct xring_map_info, node);

		if (iova < entry->iova) {
			p = &(*p)->rb_left;
		} else if (iova > entry->iova) {
			p = &(*p)->rb_right;
		} else {
			dev_err(smmu->dev, "%s iova already exist\n", __func__);
			return;
		}
	}

	new = kzalloc(sizeof(struct xring_map_info), GFP_KERNEL);
	if (!new)
		return;

	new->iova = iova;
	new->paddr = paddr;
	new->pgsize = pgsize;
	rb_link_node(&new->node, parent, p);
	rb_insert_color(&new->node, root);

	mutex_unlock(&smmu_domain->map_info_lock);

	dev_info(smmu->dev, "%s iova=0x%llx, paddr=0x%llx, pgsize=0x%x\n",
				__func__, iova, paddr, pgsize);
}

static void dummy_smmu_map_info_del(struct iommu_domain *domain,
				struct xring_map_info *entry)
{
	struct dummy_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct dummy_smmu_device *smmu = smmu_domain->smmu;
	struct rb_root *root = &smmu_domain->map_info_root;

	dev_info(smmu->dev, "%s iova=0x%llx, paddr=0x%llx\n",
				__func__, entry->iova, entry->paddr);

	mutex_lock(&smmu_domain->map_info_lock);
	rb_erase(&entry->node, root);
	kfree(entry);
	mutex_unlock(&smmu_domain->map_info_lock);
}

static int dummy_smmu_map_pages(struct iommu_domain *domain, unsigned long iova,
				phys_addr_t paddr, size_t pgsize, size_t pgcount,
				int prot, gfp_t gfp, size_t *mapped)
{
	*mapped += pgsize;
	dummy_smmu_map_info_add(domain, iova, paddr, pgsize);
	return 0;
}

static size_t dummy_smmu_unmap_pages(struct iommu_domain *domain, unsigned long iova,
				size_t pgsize, size_t pgcount,
				struct iommu_iotlb_gather *gather)
{
	struct xring_map_info *entry;

	entry = dummy_smmu_map_info_find(domain, iova);
	if (!entry)
		return 0;

	dummy_smmu_map_info_del(domain, entry);
	return pgsize;
}

static phys_addr_t dummy_smmu_iova_to_phys(struct iommu_domain *domain,
					dma_addr_t iova)
{
	struct xring_map_info *entry;
	phys_addr_t paddr;

	entry = dummy_smmu_map_info_find(domain, iova);
	if (!entry)
		return 0;

	if (iova == entry->iova)
		paddr = entry->paddr;//iova full match
	else
		paddr = entry->paddr + iova - entry->iova;//iova locate at this entry

	return paddr;
}

static int dummy_smmu_insert_master(struct dummy_smmu_device *smmu,
				struct dummy_smmu_master *master)
{
	return 0;
}

static int dummy_smmu_remove_master(struct dummy_smmu_master *master)
{

	return 0;
}

static struct platform_driver dummy_smmu_driver;
static struct dummy_smmu_device *dummy_smmu_get_by_fwnode(struct fwnode_handle *fwnode)
{
	struct device *dev = driver_find_device_by_fwnode(
					&dummy_smmu_driver.driver, fwnode);

	put_device(dev);
	return dev ? dev_get_drvdata(dev) : NULL;
}

static struct iommu_device *dummy_smmu_probe_device(struct device *dev)
{
	struct iommu_fwspec *fwspec = dev_iommu_fwspec_get(dev);
	struct dummy_smmu_device *smmu;
	struct dummy_smmu_master *master;

	if (!fwspec)
		return ERR_PTR(-ENODEV);

	if (fwspec->ops != &dummy_smmu_ops) {
		pr_err("%s fwspec not correct\n", __func__);
		return ERR_PTR(-ENODEV);
	}

	if (WARN_ON_ONCE(dev_iommu_priv_get(dev))) {
		pr_err("%s dev_iommu_priv_get fail\n", __func__);
		return ERR_PTR(-EBUSY);
	}

	smmu = dummy_smmu_get_by_fwnode(fwspec->iommu_fwnode);
	if (!smmu) {
		dev_err(smmu->dev, "%s dev_iommu_priv_get fail\n", __func__);
		return ERR_PTR(-ENODEV);
	}

	master = kzalloc(sizeof(struct dummy_smmu_master), GFP_KERNEL);
	if (!master)
		return ERR_PTR(-ENOMEM);

	master->dev = dev;
	master->smmu = smmu;
	dev_iommu_priv_set(dev, master);
	dummy_smmu_insert_master(smmu, master);

	return &smmu->iommu;
}

static void dummy_smmu_release_device(struct device *dev)
{
	struct dummy_smmu_master *master;

	master = dev_iommu_priv_get(dev);
	if (!master) {
		dev_err(dev, "%s dev_iommu_priv_get fail\n", __func__);
		return;
	}
	dummy_smmu_detach_dev(master);
	dummy_smmu_remove_master(master);
	kfree(master);
	iommu_fwspec_free(dev);
}

static int __bus_lookup_iommu_group(struct device *dev, void *priv)
{
	struct lookup_iommu_group_data *data = priv;
	struct device_node *np;
	struct iommu_group *group;

	group = iommu_group_get(dev);
	if (!group)
		return 0;

	np = of_parse_phandle(dev->of_node, "xring,iommu-group", 0);
	if (np != data->np) {
		iommu_group_put(group);
		return 0;
	}

	pr_info("smmu: found dev: %s has the same group\n", dev_name(dev));

	data->group = group;
	return 1;
}

static struct iommu_group *of_get_device_group(struct device *dev)
{
	struct lookup_iommu_group_data data;
	int ret;

	data.np = of_parse_phandle(dev->of_node, "xring,iommu-group", 0);
	if (!data.np)
		return NULL;

	ret = bus_for_each_dev(&platform_bus_type, NULL, &data,
				__bus_lookup_iommu_group);
	if (ret > 0)
		return data.group;

	ret = bus_for_each_dev(&amba_bustype, NULL, &data,
				__bus_lookup_iommu_group);
	if (ret > 0)
		return data.group;

	return NULL;
}

static struct iommu_group *dummy_smmu_device_group(struct device *dev)
{
	struct iommu_group *group;

	group = of_get_device_group(dev);
	if (group) {
		pr_info("smmu: share group %s\n", dev_name(dev));
		return group;
	}

	pr_info("smmu: create new group %s\n", dev_name(dev));
	if (dev_is_pci(dev))
		group = pci_device_group(dev);
	else
		group = generic_device_group(dev);

	return group;
}

static int dummy_smmu_of_xlate(struct device *dev, struct of_phandle_args *args)
{
	return iommu_fwspec_add_ids(dev, args->args, 1);
}

static struct iommu_ops dummy_smmu_ops = {
	.capable		= dummy_smmu_capable,
	.domain_alloc		= dummy_smmu_domain_alloc,
	.domain_free		= dummy_smmu_domain_free,
	.attach_dev		= dummy_smmu_attach_dev,
	.map_pages		= dummy_smmu_map_pages,
	.unmap_pages		= dummy_smmu_unmap_pages,
	.iova_to_phys		= dummy_smmu_iova_to_phys,
	.probe_device		= dummy_smmu_probe_device,
	.release_device		= dummy_smmu_release_device,
	.device_group		= dummy_smmu_device_group,
	.of_xlate		= dummy_smmu_of_xlate,
	.pgsize_bitmap		= -1UL,
	.owner			= THIS_MODULE,
};

static int dummy_smmu_device_dt_probe(struct platform_device *pdev,
					struct dummy_smmu_device *smmu)
{
	struct device *dev = &pdev->dev;
	u32 cells;
	int ret = -EINVAL;

	if (of_property_read_u32(dev->of_node, "#iommu-cells", &cells))
		dev_err(dev, "missing #iommu-cells property\n");
	else if (cells != 1)
		dev_err(dev, "invalid #iommu-cells value (%d)\n", cells);
	else
		ret = 0;

	return ret;
}

static void dummy_smmu_device_init(struct dummy_smmu_device *smmu)
{

}

static int dummy_smmu_set_bus_ops(struct iommu_ops *ops)
{
	int ret;

#ifdef CONFIG_ARM_AMBA
	if (amba_bustype.iommu_ops != ops) {
		ret = bus_set_iommu(&amba_bustype, ops);
		if (ret) {
			pr_err("%s amba bus_set_iommu fail\n", __func__);
			return -1;
		}
	}
#endif

	if (platform_bus_type.iommu_ops != ops) {
		ret = bus_set_iommu(&platform_bus_type, ops);
		if (ret) {
			pr_err("%s platform bus_set_iommu fail\n", __func__);
			goto fail_reset_amba_ops;
		}
	}

	return 0;

fail_reset_amba_ops:
#ifdef CONFIG_ARM_AMBA
	bus_set_iommu(&amba_bustype, NULL);
#endif
	return -1;
}

static int dummy_smmu_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct dummy_smmu_device *smmu;
	int ret;

	smmu = devm_kzalloc(dev, sizeof(struct dummy_smmu_device), GFP_KERNEL);
	if (!smmu)
		return -1;

	smmu->dev = dev;
	if (dev->of_node)
		dummy_smmu_device_dt_probe(pdev, smmu);

	dummy_smmu_device_init(smmu);

	platform_set_drvdata(pdev, smmu);

	ret = iommu_device_sysfs_add(&smmu->iommu, dev, NULL,
						"dummy_smmu.00000000");
	if (ret) {
		dev_err(dev, "%s iommu_device_sysfs_add fail\n", __func__);
		goto fail_free_smmu;
	}

	ret = iommu_device_register(&smmu->iommu, &dummy_smmu_ops, dev);
	if (ret) {
		dev_err(dev, "%s iommu_device_register fail\n", __func__);
		goto fail_remove_sysfs;
	}

	ret = dummy_smmu_set_bus_ops(&dummy_smmu_ops);
	if (ret < 0) {
		dev_err(dev, "%s dummy_smmu_set_bus_ops fail\n", __func__);
		goto fail_unregister_device;
	}

	return 0;

fail_unregister_device:
	iommu_device_unregister(&smmu->iommu);
fail_remove_sysfs:
	iommu_device_sysfs_remove(&smmu->iommu);
fail_free_smmu:
	kfree(smmu);

	return -1;
}

static int dummy_smmu_remove(struct platform_device *pdev)
{
	struct dummy_smmu_device *smmu;

	smmu = platform_get_drvdata(pdev);
	if (!smmu) {
		dev_err(&pdev->dev, "fail to get dummy_smmu_device\n");
		return -1;
	}

	iommu_device_unregister(&smmu->iommu);
	iommu_device_sysfs_remove(&smmu->iommu);
	kfree(smmu);
	return 0;
}

static const struct of_device_id dummy_smmu_of_match_table[] = {
	{ .compatible = "xring,dummy_smmu" },
	{}
};
MODULE_DEVICE_TABLE(of, dummy_smmu_of_match_table);

static struct platform_driver dummy_smmu_driver = {
	.driver = {
		.name = "xring_dummy_smmu",
		.of_match_table = dummy_smmu_of_match_table,
	},
	.probe = dummy_smmu_probe,
	.remove = dummy_smmu_remove,
};

static int __init dummy_smmu_init(void)
{
	return platform_driver_register(&dummy_smmu_driver);
}

static void __exit dummy_smmu_exit(void)
{
	platform_driver_unregister(&dummy_smmu_driver);
}
module_init(dummy_smmu_init);
module_exit(dummy_smmu_exit);

MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_DESCRIPTION("X-Ring Dummy SMMU Driver");
MODULE_LICENSE("GPL v2");
