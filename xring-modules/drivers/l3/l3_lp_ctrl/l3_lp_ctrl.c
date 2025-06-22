// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>

#define CLUSTER_PPU_OFFSET 0X030000
#define CLUSTER_CONTROL_OFFSET 0X000000
#define PWPR_OFFSET (CLUSTER_PPU_OFFSET + 0X000)
#define PWSR_OFFSET (CLUSTER_PPU_OFFSET + 0X008)
#define PWRCTLR_OFFSET (CLUSTER_CONTROL_OFFSET + 0X0010)
#define DNTH0_OFFSET (CLUSTER_CONTROL_OFFSET + 0X0028)
#define DNTH1_OFFSET (CLUSTER_CONTROL_OFFSET + 0X0030)
#define UPTH0_OFFSET (CLUSTER_CONTROL_OFFSET + 0X0038)
#define UPTH1_OFFSET (CLUSTER_CONTROL_OFFSET + 0X0040)
#define UPTH2_OFFSET (CLUSTER_CONTROL_OFFSET + 0X0090)

#define MODE_RO 0440
#define MODE_RW 0640

#define SMC_OK 0

static inline u32 read_reg32(struct device *dev, u32 offset)
{
	struct arm_smccc_res res;

	arm_smccc_smc(FID_BL31_CPU_REG_READ_EXTREG, offset, 32, 0, 0, 0, 0, 0, &res);
	if (res.a0 != SMC_OK) {
		dev_err(dev, "Read reg32 fail. Offset: 0x%x\n", offset);
		return -EINVAL;
	}
	return (u32)res.a1;
}

static inline u64 read_reg64(struct device *dev, u32 offset)
{
	struct arm_smccc_res res;

	arm_smccc_smc(FID_BL31_CPU_REG_READ_EXTREG, offset, 64, 0, 0, 0, 0, 0, &res);
	if (res.a0 != SMC_OK) {
		dev_err(dev, "Read reg64 fail. Offset: 0x%x\n", offset);
		return -EINVAL;
	}
	return (u64)res.a1;
}

static inline int write_reg32(struct device *dev, u32 offset, u32 val)
{
	struct arm_smccc_res res;

	arm_smccc_smc(FID_BL31_CPU_REG_WRITE_EXTREG, offset, 32, val, 0, 0, 0, 0, &res);
	if (res.a0 != SMC_OK) {
		dev_err(dev, "Write reg32 fail. Offset: 0x%x, val: 0x%x\n", offset, val);
		return -EINVAL;
	}
	return 0;
}

static inline int write_reg64(struct device *dev, u32 offset, u64 val)
{
	struct arm_smccc_res res;

	arm_smccc_smc(FID_BL31_CPU_REG_WRITE_EXTREG, offset, 64, val, 0, 0, 0, 0, &res);
	if (res.a0 != SMC_OK) {
		dev_err(dev, "Write reg64 fail. Offset: 0x%x, val: 0x%llx\n", offset, val);
		return -EINVAL;
	}
	return 0;
}

/* Register unions */
union pwpr_union {
	u32 val;
	struct {
		u32 pwr_policy : 4;
		u32: 4;
		u32 pwr_dyn_en : 1;
		u32: 3;
		u32 lock_en : 1;
		u32: 3;
		u32 op_policy : 4;
		u32: 4;
		u32 op_dyn_en : 1;
		u32: 7;
	} field;
};

union pwsr_union {
	u32 val;
	struct {
		u32 pwr_status : 4;
		u32: 4;
		u32 pwr_dyn_status : 1;
		u32: 3;
		u32 lock_status : 1;
		u32: 3;
		u32 op_status : 4;
		u32: 4;
		u32 op_dyn_status : 1;
		u32: 7;
	} field;
};

union pwrctlr_union {
	u64 val;
	struct {
		u32 funcret : 3;
		u32 opres : 1;
		u32 prtnrq : 2;
		u32 slcrq : 2;
		u32 slcprtn : 1;
		u32 fullret : 3;
		u32 autoprtn : 3;
		u32 autoslc : 3;
		u32 qnap : 3;
		u32 slcbw : 2;
		u32 slcsf : 1;
		u32 oslccnt : 4;
		u32 hslccnt : 4;
		u32 oslcmask : 14;
		u32: 2;
		u32 hslcmask : 14;
		u32: 2;
	} field;
};

#define REG32_SHOW_FUNC(reg_name_lower, reg_name_upper) \
static ssize_t reg_name_lower##_show( \
	struct device *dev, struct device_attribute __maybe_unused *attr, char *buf) \
{ \
	return sprintf(buf, "0x%x\n", read_reg32(dev, reg_name_upper##_OFFSET)); \
}

#define REG64_SHOW_FUNC(reg_name_lower, reg_name_upper) \
static ssize_t reg_name_lower##_show( \
	struct device *dev, struct device_attribute __maybe_unused *attr, char *buf) \
{ \
	return sprintf(buf, "0x%llx\n", read_reg64(dev, reg_name_upper##_OFFSET)); \
}

/* field_name are always lower */
#define REG32_FIELD_SHOW_FUNC(reg_name_lower, reg_name_upper, field_name) \
static ssize_t field_name##_show( \
	struct device *dev, struct device_attribute __maybe_unused *attr, char *buf) \
{ \
	union reg_name_lower##_union reg_name_lower; \
	reg_name_lower.val = read_reg32(dev, reg_name_upper##_OFFSET); \
	return sprintf(buf, "0x%x\n", reg_name_lower.field.field_name); \
}

#define REG64_FIELD_SHOW_FUNC(reg_name_lower, reg_name_upper, field_name) \
static ssize_t field_name##_show( \
	struct device *dev, struct device_attribute __maybe_unused *attr, char *buf) \
{ \
	union reg_name_lower##_union reg_name_lower; \
	reg_name_lower.val = read_reg64(dev, reg_name_upper##_OFFSET); \
	return sprintf(buf, "0x%x\n", reg_name_lower.field.field_name); \
}

#define MAKE_REG32_RO(reg_name_lower, reg_name_upper) \
	REG32_SHOW_FUNC(reg_name_lower, reg_name_upper); \
	static DEVICE_ATTR(reg_name_lower, MODE_RO, reg_name_lower##_show, NULL)

#define MAKE_REG32_FIELD_RO(reg_name_lower, reg_name_upper, field_name) \
	REG32_FIELD_SHOW_FUNC(reg_name_lower, reg_name_upper, field_name); \
	static DEVICE_ATTR(field_name, MODE_RO, field_name##_show, NULL)

/* pwpr */
MAKE_REG32_RO(pwpr, PWPR);

/* pwsr and its bit fields */
MAKE_REG32_RO(pwsr, PWSR);
MAKE_REG32_FIELD_RO(pwsr, PWSR, pwr_status);
MAKE_REG32_FIELD_RO(pwsr, PWSR, pwr_dyn_status);
MAKE_REG32_FIELD_RO(pwsr, PWSR, op_status);
MAKE_REG32_FIELD_RO(pwsr, PWSR, op_dyn_status);

/* pwrctlr and its bit fields */
struct field_info {
	char *field_name;
	u32 upth;
	u32 no_eq;
};

const struct field_info funcret_info = {"funcret", 7, 8};
const struct field_info opres_info = {"opres", 1, 2};
const struct field_info prtnrq_info = {"prtnrq", 3, 2};
const struct field_info slcrq_info = {"slcrq", 2, 3};
const struct field_info slcprtn_info = {"slcprtn", 1, 2};
const struct field_info fullret_info = {"fullret", 7, 8};
const struct field_info autoprtn_info = {"autoprtn", 7, 8};
const struct field_info autoslc_info = {"autoslc", 7, 8};
const struct field_info qnap_info = {"qnap", 7, 8};
const struct field_info slcbw_info = {"slcbw", 3, 2};
const struct field_info slcsf_info = {"slcsf", 1, 2};
const struct field_info oslccnt_info = {"oslccnt", 0b1111, 0b1111 + 1};
const struct field_info hslccnt_info = {"hslccnt", 0b1111, 0b1111 + 1};
const struct field_info oslcmask_info = {"oslcmask", (1 << 14) - 1, 1 << 14};
const struct field_info hslcmask_info = {"hslcmask", (1 << 14) - 1, 1 << 14};

static inline void check_field_value(
	int *ret, struct device *dev, struct field_info this_field_info, u32 val)
{
	char *field_name = this_field_info.field_name;
	u32 upth = this_field_info.upth;
	u32 no_eq = this_field_info.no_eq;

	if (val == no_eq || val > upth) {
		dev_err(dev, "Reject! %s must be <= %u and != %u.\n", field_name, upth, no_eq);
		*ret = -EINVAL;
	}
}

static inline int check_pwrctlr_value(struct device *dev, u64 val)
{
	union pwrctlr_union pwrctlr;
	int ret = 0;

	pwrctlr.val = val;
	check_field_value(&ret, dev, funcret_info, pwrctlr.field.funcret);
	check_field_value(&ret, dev, opres_info, pwrctlr.field.opres);
	check_field_value(&ret, dev, prtnrq_info, pwrctlr.field.prtnrq);
	check_field_value(&ret, dev, slcrq_info, pwrctlr.field.slcrq);
	check_field_value(&ret, dev, slcprtn_info, pwrctlr.field.slcprtn);
	check_field_value(&ret, dev, fullret_info, pwrctlr.field.fullret);
	check_field_value(&ret, dev, autoprtn_info, pwrctlr.field.autoprtn);
	check_field_value(&ret, dev, autoslc_info, pwrctlr.field.autoslc);
	check_field_value(&ret, dev, qnap_info, pwrctlr.field.qnap);
	check_field_value(&ret, dev, slcbw_info, pwrctlr.field.slcbw);
	check_field_value(&ret, dev, slcsf_info, pwrctlr.field.slcsf);
	check_field_value(&ret, dev, oslccnt_info, pwrctlr.field.oslccnt);
	check_field_value(&ret, dev, hslccnt_info, pwrctlr.field.hslccnt);
	check_field_value(&ret, dev, oslcmask_info, pwrctlr.field.oslcmask);
	check_field_value(&ret, dev, hslcmask_info, pwrctlr.field.hslcmask);

	return ret;
}

REG64_SHOW_FUNC(pwrctlr, PWRCTLR);
static ssize_t pwrctlr_store(
	struct device *dev,
	struct device_attribute __maybe_unused *attr,
	const char *buf,
	size_t count)
{
	u64 input;
	int ret;

	ret = kstrtoull(buf, 0, &input);
	if (ret != 0) {
		dev_err(dev, "input must be u64.\n");
		return -EINVAL;
	}
	if (check_pwrctlr_value(dev, input) != 0)
		return -EINVAL;

	write_reg64(dev, PWRCTLR_OFFSET, input);
	return count;
}
static DEVICE_ATTR(pwrctlr, MODE_RW, pwrctlr_show, pwrctlr_store);

#define MAKE_PWRCTLR_FIELD_RW(field_name) \
REG64_FIELD_SHOW_FUNC(pwrctlr, PWRCTLR, field_name); \
static ssize_t field_name##_store( \
	struct device *dev, \
	struct device_attribute __maybe_unused *attr, \
	const char *buf, \
	size_t count) \
{ \
	union pwrctlr_union pwrctlr; \
	u32 input; \
	int ret; \
\
	ret = kstrtouint(buf, 0, &input); \
	if (ret != 0) { \
		dev_err(dev, "input must be u32.\n"); \
		return -EINVAL; \
	} \
	check_field_value(&ret, dev, field_name##_info, input); \
	if (ret != 0) \
		return -EINVAL; \
\
	pwrctlr.val = read_reg64(dev, PWRCTLR_OFFSET); \
	pwrctlr.field.field_name = input; \
\
	write_reg64(dev, PWRCTLR_OFFSET, pwrctlr.val); \
	return count; \
} \
static DEVICE_ATTR(field_name, MODE_RW, field_name##_show, field_name##_store)

MAKE_PWRCTLR_FIELD_RW(funcret);
MAKE_PWRCTLR_FIELD_RW(opres);
MAKE_PWRCTLR_FIELD_RW(prtnrq);
MAKE_PWRCTLR_FIELD_RW(slcrq);
MAKE_PWRCTLR_FIELD_RW(slcprtn);
MAKE_PWRCTLR_FIELD_RW(fullret);
MAKE_PWRCTLR_FIELD_RW(autoprtn);
MAKE_PWRCTLR_FIELD_RW(autoslc);
MAKE_PWRCTLR_FIELD_RW(qnap);
MAKE_PWRCTLR_FIELD_RW(slcbw);
MAKE_PWRCTLR_FIELD_RW(slcsf);
MAKE_PWRCTLR_FIELD_RW(oslccnt);
MAKE_PWRCTLR_FIELD_RW(hslccnt);
MAKE_PWRCTLR_FIELD_RW(oslcmask);
MAKE_PWRCTLR_FIELD_RW(hslcmask);

/* dnth and upth regs */
#define MAKE_TH_RW(reg_name_lower, reg_name_upper) \
REG32_SHOW_FUNC(reg_name_lower, reg_name_upper); \
static ssize_t reg_name_lower##_store( \
	struct device *dev, \
	struct device_attribute __maybe_unused *attr, \
	const char *buf, \
	size_t count) \
{ \
	u32 input; \
	int ret; \
\
	ret = kstrtouint(buf, 0, &input); \
	if (ret != 0) { \
		dev_err(dev, "input must be u32.\n"); \
		return -EINVAL; \
	} \
	write_reg32(dev, reg_name_upper##_OFFSET, input); \
\
	return count; \
} \
static DEVICE_ATTR(reg_name_lower, MODE_RW, reg_name_lower##_show, reg_name_lower##_store)

MAKE_TH_RW(dnth0, DNTH0);
MAKE_TH_RW(dnth1, DNTH1);
MAKE_TH_RW(upth0, UPTH0);
MAKE_TH_RW(upth1, UPTH1);
MAKE_TH_RW(upth2, UPTH2);

/* hit and miss */
#define CLUSTERL3HIT_EL1	S3_0_C15_C4_5
#define CLUSTERL3MISS_EL1	S3_0_C15_C4_6

#define STRINGIFY(input)  #input

#define MAKE_SYSREG_RO(file_name, sysreg) \
static ssize_t file_name##_show( \
	struct device *dev, struct device_attribute __maybe_unused *attr, char *buf) \
{ \
	u64 val; \
\
	__asm__ volatile ("mrs %0, " STRINGIFY(sysreg) : "=r" (val) :: "memory"); \
	return sprintf(buf, "0x%llx\n", val); \
} \
static DEVICE_ATTR(file_name, MODE_RO, file_name##_show, NULL)

MAKE_SYSREG_RO(hit, CLUSTERL3HIT_EL1);
MAKE_SYSREG_RO(miss, CLUSTERL3MISS_EL1);
/* END: prepare for sysfs files */

static struct attribute *g_l3_attrs[] = {
	&dev_attr_pwpr.attr,		/* RO */

	&dev_attr_pwsr.attr,		/* RO */
	&dev_attr_pwr_status.attr,	/* RO */
	&dev_attr_pwr_dyn_status.attr,	/* RO */
	&dev_attr_op_status.attr,	/* RO */
	&dev_attr_op_dyn_status.attr,	/* RO */

	&dev_attr_pwrctlr.attr,		/* RW */
	&dev_attr_funcret.attr,		/* RW */
	&dev_attr_opres.attr,		/* RW */
	&dev_attr_prtnrq.attr,		/* RW */
	&dev_attr_slcrq.attr,		/* RW */
	&dev_attr_slcprtn.attr,		/* RW */
	&dev_attr_fullret.attr,		/* RW */
	&dev_attr_autoprtn.attr,	/* RW */
	&dev_attr_autoslc.attr,		/* RW */
	&dev_attr_qnap.attr,		/* RW */
	&dev_attr_slcbw.attr,		/* RW */
	&dev_attr_slcsf.attr,		/* RW */
	&dev_attr_oslccnt.attr,		/* RW */
	&dev_attr_hslccnt.attr,		/* RW */
	&dev_attr_oslcmask.attr,	/* RW */
	&dev_attr_hslcmask.attr,	/* RW */

	&dev_attr_dnth0.attr,		/* RW */
	&dev_attr_dnth1.attr,		/* RW */
	&dev_attr_upth0.attr,		/* RW */
	&dev_attr_upth1.attr,		/* RW */
	&dev_attr_upth2.attr,		/* RW */

	&dev_attr_hit.attr,		/* RO */
	&dev_attr_miss.attr,		/* RO */
	NULL,
};

static struct attribute_group g_l3_attr_group = {
	.attrs = g_l3_attrs,
};

static int l3_lp_ctrl_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret;

	/* create sysfs files */
	ret = sysfs_create_group(&dev->kobj, &g_l3_attr_group);
	if (ret) {
		dev_err(dev, "failed to create sysfs group!\n");
		return ret;
	}

	return 0;
}

static const struct of_device_id g_l3_lp_ctrl_of_match[] = {
	{
		.compatible = "xring,l3_lp_ctrl",
	},
	{},
};
MODULE_DEVICE_TABLE(of, g_l3_lp_ctrl_of_match);

static struct platform_driver g_l3_lp_ctrl_driver = {
	.probe = l3_lp_ctrl_probe,
	.driver = {
		.name = "l3_lp_ctrl",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(g_l3_lp_ctrl_of_match),
	},
};

static int __init l3_lp_ctrl_init(void)
{
	return platform_driver_register(&g_l3_lp_ctrl_driver);
}
module_init(l3_lp_ctrl_init);

MODULE_AUTHOR("Aoyang Yan <yanaoyang@xiaomi.com>");
MODULE_DESCRIPTION("XRing L3 Low Power Control Driver");
MODULE_LICENSE("GPL v2");
