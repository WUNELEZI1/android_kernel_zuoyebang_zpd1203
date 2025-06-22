// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/init.h>
#include <linux/acpi.h>
#include <linux/clocksource.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/time.h>

#include <soc/xring/securelib/securec.h>
#include <dt-bindings/xring/platform-specific/syscnt_fastread_reg.h>
#include <dt-bindings/xring/platform-specific/timestamp_adapt.h>
#include <dt-bindings/xring/platform-specific/common/timestamp/timestamp_common.h>

#define XR_TIMESTAMP_MODE		0444

#define XR_TIMESTAMP_RATING		300
#define XR_TIMESTAMP_WIDTH		64

#define XR_TIMESTAMP_MAXSEC		3600

#define TIMESTAMP_IOC_MAGIC 'k'

#define TIMESTAMP_IOC_RD_COUNT       _IOR(TIMESTAMP_IOC_MAGIC, 0, __u64)
#define TIMESTAMP_IOC_RD_TIME        _IOR(TIMESTAMP_IOC_MAGIC, 1, __u64)

struct xring_timestamp_dev {
	void __iomem        *base;
	u32                 freq;
	u32                 width;
	u32                 rating;
	char                name[32];
	struct miscdevice   misc;

	u32                 mult;
	u32                 shift;
	u32                 calc_ov_shift;
	u64                 calc_ov_cnt;
};

static struct xring_timestamp_dev g_ts;

static inline struct xring_timestamp_dev *to_drvdata(struct file *filp)
{
	return container_of(filp->private_data, struct xring_timestamp_dev, misc);
}

static void calc_factors(struct xring_timestamp_dev *ts)
{
	u32 shift;
	u64 ov_cnt;

	clocks_calc_mult_shift(&ts->mult, &ts->shift, ts->freq, NSEC_PER_SEC, XR_TIMESTAMP_MAXSEC);

	ov_cnt = (u64)ts->freq * XR_TIMESTAMP_MAXSEC;
	shift = 1;
	while (ov_cnt >> shift)
		shift++;

	ts->calc_ov_shift = shift - 1;
	ts->calc_ov_cnt = (u64)1 << ts->calc_ov_shift;

	dev_dbg(ts->misc.parent, "mult=%d, shift=%d, maxsec=%d, calc_ov_cnt=%lld, calc_ov_shift=%u\n",
			ts->mult, ts->shift, XR_TIMESTAMP_MAXSEC,
			ts->calc_ov_cnt, ts->calc_ov_shift);
}

static u64 get_count(struct xring_timestamp_dev *ts)
{
	u64 current_count;
	u32 count_l[2];
	u32 count_h[2];

	if (IS_ERR(ts->base)) {
		dev_err(ts->misc.parent, "%s: base is NULL\n", __func__);
		return 0;
	}

	/*
	 * The 64-bit count value needs two 32-bit register read, the following steps are to avoid
	 * errors caused by 32bit carry.
	 */
	count_l[0] = readl(ts->base + TS_CTRL_CNTCVL);
	count_h[0] = readl(ts->base + TS_CTRL_CNTCVH);
	count_l[1] = readl(ts->base + TS_CTRL_CNTCVL);
	count_h[1] = readl(ts->base + TS_CTRL_CNTCVH);

	if (count_h[0] == count_h[1]) /* No carry happen between count_h[0] and count_h[1] */
		current_count = ((u64)count_h[1] << 32) | count_l[1];
	else /* Carry during count_h[0] ~ count_h[1] */
		current_count = ((u64)count_h[0] << 32) | count_l[0];

	return current_count;
}

static inline u64 __count2ns(u64 count, u32 mult, u32 shift)
{
	return (count * mult) >> shift;
}

static inline u64 __count2ns_high(u64 count, u32 mult, u32 ov_shift, u32 shift)
{
	if (ov_shift > shift)
		return (count * mult) << (ov_shift - shift);
	else
		return (count * mult) >> (shift - ov_shift);
}

u64 xr_timestamp_count2ns(u64 count)
{
	u64 ns_low, ns_high;
	struct xring_timestamp_dev *ts = &g_ts;

	if (count > ts->calc_ov_cnt) {
		/*
		 * Avoid overflow during calculation
		 */
		ns_low = count & (((u64)1 << ts->calc_ov_shift) - 1);
		ns_high = count >> ts->calc_ov_shift;
		ns_low = __count2ns(ns_low, ts->mult, ts->shift);
		ns_high = __count2ns_high(ns_high, ts->mult, ts->calc_ov_shift, ts->shift);
		return ns_high + ns_low;
	} else {
		return  __count2ns(count, ts->mult, ts->shift);
	}

}
EXPORT_SYMBOL(xr_timestamp_count2ns);

u64 xr_timestamp_getcount(void)
{
	struct xring_timestamp_dev *ts = &g_ts;

	return get_count(ts);
}
EXPORT_SYMBOL(xr_timestamp_getcount);

ktime_t xr_timestamp_gettime(void)
{
	u64 count;
	struct xring_timestamp_dev *ts = &g_ts;

	count = get_count(ts);
	if (count == 0)
		return 0;

	return xr_timestamp_count2ns(count);
}
EXPORT_SYMBOL(xr_timestamp_gettime);

static const struct of_device_id xr_timestamp_of_match[] = {
	{ .compatible = "xring,xr-timestamp",  },
	{}
};
MODULE_DEVICE_TABLE(of, xr_timestamp_of_match);

static long ts_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret;
	u64 ts_count, ts_time;
	struct xring_timestamp_dev *ts = to_drvdata(filp);

	switch (cmd) {
	case TIMESTAMP_IOC_RD_COUNT:
		ts_count = get_count(ts);
		ret = copy_to_user((u64 *)arg, &ts_count, sizeof(u64));
		if (ret) {
			dev_err(ts->misc.parent, "copy_to_user failed\n");
			return ret;
		}
		dev_dbg(ts->misc.parent, "xr_timestamp RD_COUNT:%lld\n", ts_count);
		break;

	case TIMESTAMP_IOC_RD_TIME:
		ts_time = xr_timestamp_gettime();
		ret = copy_to_user((u64 *)arg, &ts_time, sizeof(u64));
		if (ret) {
			dev_err(ts->misc.parent, "copy_to_user failed\n");
			return ret;
		}
		dev_dbg(ts->misc.parent, "xr_timestamp RD_TIME:%llu.%09llus\n",
				ts_time / NSEC_PER_SEC,
				ts_time - ts_time / NSEC_PER_SEC * NSEC_PER_SEC);
		break;

	default:
		return -EINVAL;
	}

	return ret;
}

static const struct file_operations xring_timestamp_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = ts_ioctl,
};

static ssize_t boot_offset_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ktime_t real_ns = ktime_get_real();
	ktime_t boot_ns = xr_timestamp_gettime();
	ktime_t boot_offset = ktime_sub(real_ns, boot_ns);

	return sprintf(buf, "%llu\n", (unsigned long long)boot_offset);
}
static DEVICE_ATTR_RO(boot_offset);

static struct attribute *timestamp_dev_attrs[] = {
	&dev_attr_boot_offset.attr,
	NULL
};

static const struct attribute_group timestamp_dev_group = {
	.attrs = timestamp_dev_attrs,
};

static int xring_timestamp_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = pdev->dev.of_node;
	struct xring_timestamp_dev *ts = &g_ts;
	int ret;

	dev_info(dev, "xr-timestamp probe entry\n");

	ts->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(ts->base)) {
		dev_err(dev, "%s: get ioremap failed\n", __func__);
		return PTR_ERR(ts->base);
	}

	ret = of_property_read_u32(node, "frequency", &ts->freq);
	if (ret) {
		dev_err(dev, "%s: get frequency failed\n", __func__);
		return ret;
	}

	ret = of_property_read_u32(node, "width", &ts->width);
	if (ret) {
		ts->width = XR_TIMESTAMP_WIDTH;
		dev_warn(dev, "xr-timestamp use default width (%d)\n", ts->width);
	}

	ret = of_property_read_u32(node, "rating", &ts->rating);
	if (ret) {
		ts->rating = XR_TIMESTAMP_RATING;
		dev_warn(dev, "xr-timestamp use default rating (%d)\n", ts->rating);
	}

	ts->misc.parent = dev;
	ts->misc.name = ts->name;
	ts->misc.minor = MISC_DYNAMIC_MINOR;
	ts->misc.fops = &xring_timestamp_fops;
	ts->misc.mode = XR_TIMESTAMP_MODE;
	strcpy_s(ts->name, sizeof(ts->name), node->name);

	ret = misc_register(&ts->misc);
	if (ret) {
		dev_err(dev, "failed to register misc device '%s': %d\n",
				ts->name, ret);
		return ret;
	}

	platform_set_drvdata(pdev, ts);

	calc_factors(ts);

	ret = sysfs_create_group(&ts->misc.this_device->kobj, &timestamp_dev_group);
	if (ret)
		dev_err(dev, "failed to create sysfs ns '%s': %d\n", ts->name, ret);

	dev_dbg(dev, "xr-timestamp:\n");
	dev_dbg(dev, "\tfreq         : %d\n", ts->freq);
	dev_dbg(dev, "\twidth        : %d\n", ts->width);
	dev_dbg(dev, "\trating       : %d\n", ts->rating);
	dev_dbg(dev, "\tname         : %s\n", ts->name);

	dev_info(dev, "xr-timestamp probe success\n");

	return 0;
}

static int xring_timestamp_remove(struct platform_device *pdev)
{
	struct xring_timestamp_dev *ts = platform_get_drvdata(pdev);

	misc_deregister(&ts->misc);
	return 0;
}

static struct platform_driver xring_timestamp_driver = {
	.probe = xring_timestamp_probe,
	.remove = xring_timestamp_remove,
	.driver = {
		.name = "timestamp",
		.of_match_table = xr_timestamp_of_match,
	},
};

static int __init xring_timestamp_init(void)
{
	return platform_driver_register(&xring_timestamp_driver);
}

static void __exit xring_timestamp_exit(void)
{
	platform_driver_unregister(&xring_timestamp_driver);
}

module_init(xring_timestamp_init);
module_exit(xring_timestamp_exit);

MODULE_AUTHOR("Jinfei Weng <wengjinfei@xiaomi.com>");
MODULE_DESCRIPTION("X-Ring Timestamp Driver");
MODULE_LICENSE("GPL v2");
