// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * sysfs driver for Xring coulomb meter
 *
 * Copyright (c) 2024, XRing Technologies Co., Ltd.
 */
#include "xr_coul_core.h"
#include "xr_coul_xp2210.h"
#include "xr_coul_nv.h"

#define AB_PARAM_STR_MAX_LEN  32

/* y2 and y1 unit is mV, the A/B correction voltage is uV,
 * or y2 and y1 unit is mA, the A/B correction voltage is uA,
 * to ensure the calculation accuray, need to multipy by 1000
 * so need to multipy 1000000 when calculate the A/B value
 */
#define CALI_AB_MULTI         1000000
#define CALI_VOL_AB_CMD       "vol"
#define CALI_CUR_AB_CMD       "cur"
#define CALI_AB_CMD_LEN       3

struct xr_coul_cali_info {
	int vol1;         /* voltage1: external mesure mv */
	int vol2;         /* voltage2: external mesure mv */
	u32 vol1_regval;  /* voltage1: coul reg value */
	u32 vol2_regval;  /* voltage2: coul reg value */

	int cur1;         /* current1: external mesure ma */
	int cur2;         /* current2: external mesure ma */
	u32 cur1_regval; /* current1: coul reg value */
	u32 cur2_regval; /* current2: coul reg value */

	bool vol1_valid;
	bool vol2_valid;
	bool cur1_valid;
	bool cur2_valid;

	int vol_a;
	int vol_b;
	int cur_a;
	int cur_b;
	bool vol_ab_valid;
	bool cur_ab_valid;
};

static struct xr_coul_cali_info s_coul_cali_info;

static ssize_t vol_regval_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int ret;
	u32 vol_regval;

	ret = coul_get_batt_vol_regval(&vol_regval);
	if (ret) {
		coul_err("get vol regval fail(%d)\n", ret);
		return ret;
	}

	return sysfs_emit(buf, "0x%x\n", vol_regval);
}

static ssize_t cur_regval_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int ret;
	u32 cur_regval;

	ret = coul_get_batt_cur_regval(&cur_regval);
	if (ret) {
		coul_err("get cur regval fail(%d)\n", ret);
		return ret;
	}

	return sysfs_emit(buf, "0x%x\n", cur_regval);
}

static ssize_t chip_id_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int ret;
	u64 chipid = 0;

	ret = coul_get_chipid(&chipid);
	if (ret)
		return ret;

	return sysfs_emit(buf, "0x%llx\n", chipid);
}

static ssize_t vol_a_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	if (!s_coul_cali_info.vol_ab_valid)
		return -EINVAL;

	return sysfs_emit(buf, "%d\n", s_coul_cali_info.vol_a);
}

static ssize_t vol_b_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	if (!s_coul_cali_info.vol_ab_valid)
		return -EINVAL;

	return sysfs_emit(buf, "%d\n", s_coul_cali_info.vol_b);
}

static ssize_t cur_a_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	if (!s_coul_cali_info.cur_ab_valid)
		return -EINVAL;

	return sysfs_emit(buf, "%d\n", s_coul_cali_info.cur_a);
}

static ssize_t cur_b_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	if (!s_coul_cali_info.cur_ab_valid)
		return -EINVAL;

	return sysfs_emit(buf, "%d\n", s_coul_cali_info.cur_b);
}

static int signed_regval_to_val(u32 reg_val)
{
	if (reg_val & COUL_SING_BIT_MASK)
		return (~(reg_val | COUL_ADC_REG_MASK) + 1) & (~COUL_SING_BIT_MASK);

	return reg_val;
}

/* convert the voltage with the A/B param */
static ssize_t calied_v_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int ret;
	u32 reg_val;
	int val;
	int volt;
	int a, b;

	if (!s_coul_cali_info.vol_ab_valid)
		return -EINVAL;
	a = s_coul_cali_info.vol_a;
	b = s_coul_cali_info.vol_b;

	ret = coul_get_batt_vol_regval(&reg_val);
	if (ret) {
		coul_err("get vol regval fail(%d)\n", ret);
		return ret;
	}
	val = signed_regval_to_val(reg_val);

	volt = div_u64((u64)val * a + b, AB_DIV_OFFSET);
	if (reg_val & COUL_SING_BIT_MASK)
		volt = -volt;

	return sysfs_emit(buf, "%d\n", volt);
}

/* convert the current with the A/B param */
static ssize_t calied_c_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int ret;
	u32 reg_val;
	int val;
	int curr;
	int a, b;

	if (!s_coul_cali_info.cur_ab_valid)
		return -EINVAL;
	a = s_coul_cali_info.cur_a;
	b = s_coul_cali_info.cur_b;

	ret = coul_get_batt_cur_regval(&reg_val);
	if (ret) {
		coul_err("get vol regval fail(%d)\n", ret);
		return ret;
	}
	val = signed_regval_to_val(reg_val);

	curr = div_s64((s64)val * a + b, AB_DIV_OFFSET);
	if (reg_val & COUL_SING_BIT_MASK)
		curr = -curr;

	return sysfs_emit(buf, "%d\n", curr);
}

/* convert the voltage without A/B param */
static ssize_t uncalied_v_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int ret;
	u32 reg_val;
	int val;
	int volt;

	ret = coul_get_batt_vol_regval(&reg_val);
	if (ret) {
		coul_err("get vol regval fail(%d)\n", ret);
		return ret;
	}
	val = signed_regval_to_val(reg_val);

	volt = div_u64((u64)val * REG_TO_UV_MULTI, REG_TO_UV_DIV);

	return sysfs_emit(buf, "%d\n", volt);
}

static ssize_t uncalied_c_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int ret;
	u32 reg_val;
	int val;
	int curr;

	ret = coul_get_batt_cur_regval(&reg_val);
	if (ret) {
		coul_err("get vol regval fail(%d)\n", ret);
		return ret;
	}
	val = signed_regval_to_val(reg_val);

	curr = div_s64((s64)val * REG_TO_UA_MULTI, REG_TO_UA_DIV);
	curr = div_s64(curr, coul_curr_mohm);

	if (reg_val & COUL_SING_BIT_MASK)
		curr = -curr;

	return sysfs_emit(buf, "%d\n", curr);
}


/* format: a=xx,b=xx */
static int parse_ab_value(const char *param_buf, size_t size, int *a, int *b)
{
	int ret;
	char buf[AB_PARAM_STR_MAX_LEN];
	size_t buf_size;
	char *str1;
	char *str2;
	bool a_valid = false;
	bool b_valid = false;

	buf_size = min_t(size_t, size, (sizeof(buf) - 1));
	memcpy(buf, param_buf, buf_size);
	buf[buf_size] = '\0';

	/* split the a=xx and b=xx expression */
	str1 = buf;
	str2 = strchr(str1, ',');
	if (!str2)
		return -EINVAL;

	*str2 = '\0';
	str2++;

	coul_info("str1=%s, str2=%s\n", str1, str2);

	switch (*str1) {
	case 'a':
		ret = kstrtos32(str1 + 2, 0, a);
		if (ret) {
			coul_err("convert a value fail, str=%s\n", str1);
			return ret;
		}
		a_valid = true;
		break;
	case 'b':
		ret = kstrtos32(str1 + 2, 0, b);
		if (ret) {
			coul_err("convert b value fail, str=%s\n", str1);
			return ret;
		}
		b_valid = true;
		break;
	default:
		coul_err("invalid param %s!\n", str1);
		return -EINVAL;
	}

	switch (*str2) {
	case 'a':
		if (a_valid) {
			coul_err("a is parsed, invalid param str1=%s, str2=%s\n", str1, str2);
			return -EINVAL;
		}
		ret = kstrtos32(str2 + 2, 0, a);
		if (ret) {
			coul_err("convert a value fail, str=%s\n", str2);
			return ret;
		}
		a_valid = true;
		break;
	case 'b':
		if (b_valid) {
			coul_err("b is parsed, invalid param str1=%s, str2=%s\n", str1, str2);
			return -EINVAL;
		}
		ret = kstrtos32(str2 + 2, 0, b);
		if (ret) {
			coul_err("convert b value fail, str=%s\n", str2);
			return ret;
		}
		b_valid = true;
		break;
	default:
		coul_err("invalid param %s!\n", str2);
		return -EINVAL;
	}

	if (!a_valid || !b_valid) {
		coul_err("invalid param str1=%s, str2=%s\n", str1, str2);
		return -EINVAL;
	}

	return 0;
}

/* format: a=xx,b=xx */
static ssize_t vol_ab_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t size)
{
	int ret;
	int a = 0;
	int b = 0;

	ret = parse_ab_value(buf, size, &a, &b);
	if (ret < 0)
		return ret;

	s_coul_cali_info.vol_a = a;
	s_coul_cali_info.vol_b = b;
	s_coul_cali_info.vol_ab_valid = true;
	coul_info("vol A=%d, B=%d\n", a, b);

	return size;
}

static ssize_t cur_ab_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t size)
{
	int ret;
	int a = 0;
	int b = 0;

	ret = parse_ab_value(buf, size, &a, &b);
	if (ret < 0)
		return ret;

	s_coul_cali_info.cur_a = a;
	s_coul_cali_info.cur_b = b;
	s_coul_cali_info.cur_ab_valid = true;

	coul_info("cur A=%d, B=%d\n", a, b);

	return size;
}

static ssize_t cali_vol1_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t size)
{
	int ret;
	int vol1;
	u32 vol_regval;

	ret = coul_get_batt_vol_regval(&vol_regval);
	if (ret) {
		coul_err("get vol regval fail(%d)\n", ret);
		return ret;
	}

	ret = kstrtos32(buf, 0, &vol1);
	if (ret)
		return ret;

	s_coul_cali_info.vol1 = vol1;
	s_coul_cali_info.vol1_regval = vol_regval;
	s_coul_cali_info.vol1_valid = true;

	return size;
}

static ssize_t cali_vol2_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t size)
{
	int ret;
	int vol2;
	u32 vol_regval;

	ret = coul_get_batt_vol_regval(&vol_regval);
	if (ret) {
		coul_err("get vol regval fail(%d)\n", ret);
		return ret;
	}

	ret = kstrtos32(buf, 0, &vol2);
	if (ret)
		return ret;

	s_coul_cali_info.vol2 = vol2;
	s_coul_cali_info.vol2_regval = vol_regval;
	s_coul_cali_info.vol2_valid = true;

	return size;
}

static ssize_t cali_cur1_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t size)
{
	int ret;
	int cur1;
	u32 cur_regval;

	ret = coul_get_batt_cur_regval(&cur_regval);
	if (ret) {
		coul_err("get cur regval fail(%d)\n", ret);
		return ret;
	}

	ret = kstrtos32(buf, 0, &cur1);
	if (ret)
		return ret;

	s_coul_cali_info.cur1 = cur1;
	s_coul_cali_info.cur1_regval = cur_regval;
	s_coul_cali_info.cur1_valid = true;

	return size;
}

static ssize_t cali_cur2_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t size)
{
	int ret;
	int cur2;
	u32 cur_regval;

	ret = coul_get_batt_cur_regval(&cur_regval);
	if (ret) {
		coul_err("get cur regval fail(%d)\n", ret);
		return ret;
	}

	ret = kstrtos32(buf, 0, &cur2);
	if (ret) {
		coul_err("cali_cur2 invalid param\n");
		return -EINVAL;
	}

	s_coul_cali_info.cur2 = cur2;
	s_coul_cali_info.cur2_regval = cur_regval;
	s_coul_cali_info.cur2_valid = true;

	return size;
}

static int calc_vol_ab(void)
{
	int64_t x1, x2;
	int64_t y1, y2;
	int a, b;

	if (!s_coul_cali_info.vol1_valid || !s_coul_cali_info.vol2_valid) {
		coul_err("vol1(%d) and vol2(%d) is invalid\n",
				s_coul_cali_info.vol1_valid, s_coul_cali_info.vol2_valid);
		return -EINVAL;
	}

	x1 = signed_regval_to_val(s_coul_cali_info.vol1_regval);
	x2 = signed_regval_to_val(s_coul_cali_info.vol2_regval);
	y1 = s_coul_cali_info.vol1;
	y2 = s_coul_cali_info.vol2;

	if (x1 == x2) {
		coul_err("invalid param x1(%d) == x2(%d)\n", (int)x1, (int)x2);
		return -EINVAL;
	}

	a = div_s64((y2 - y1) * CALI_AB_MULTI, (x2 - x1));
	b = y1 * CALI_AB_MULTI - a * x1;
	coul_info("x1=%d, x2=%d, y1=%d, y2=%d, a=%d, b=%d\n",
		(int)x1, (int)x2, (int)y1, (int)y2, a, b);

	s_coul_cali_info.vol1_valid = false;
	s_coul_cali_info.vol2_valid = false;

	s_coul_cali_info.vol_a = a;
	s_coul_cali_info.vol_b = b;
	s_coul_cali_info.vol_ab_valid = true;

	return 0;
}

static int calc_cur_ab(void)
{
	int64_t x1, x2;
	int64_t y1, y2;
	int a, b;

	if (!s_coul_cali_info.cur1_valid || !s_coul_cali_info.cur2_valid) {
		coul_err("cur1(%d) and cur2(%d) is invalid\n",
				s_coul_cali_info.cur1_valid, s_coul_cali_info.cur2_valid);
		return -EINVAL;
	}

	x1 = signed_regval_to_val(s_coul_cali_info.cur1_regval);
	x2 = signed_regval_to_val(s_coul_cali_info.cur2_regval);
	y1 = s_coul_cali_info.cur1;
	y2 = s_coul_cali_info.cur2;

	if (x1 == x2) {
		coul_err("invalid param x1(%d) == x2(%d)\n", (int)x1, (int)x2);
		return -EINVAL;
	}

	a = div_s64((y2 - y1) * CALI_AB_MULTI, (x2 - x1));
	b = (y1 * CALI_AB_MULTI - a * x1);
	coul_info("x1=%d, x2=%d, y1=%d, y2=%d, a=%d, b=%d\n",
		(int)x1, (int)x2, (int)y1, (int)y2, a, b);

	s_coul_cali_info.cur1_valid = false;
	s_coul_cali_info.cur2_valid = false;

	s_coul_cali_info.cur_a = a;
	s_coul_cali_info.cur_b = b;
	s_coul_cali_info.cur_ab_valid = true;

	return 0;
}

static ssize_t calc_start_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t size)
{
	int ret = 0;

	if (strncmp(buf, CALI_VOL_AB_CMD, CALI_AB_CMD_LEN) == 0)
		ret = calc_vol_ab();
	else if (strncmp(buf, CALI_CUR_AB_CMD, CALI_AB_CMD_LEN) == 0)
		ret = calc_cur_ab();
	else {
		coul_err("Invalid param %s\n", buf);
		return -EINVAL;
	}

	if (ret < 0)
		return ret;

	return size;
}

static ssize_t save_ab_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t size)
{
	int ret = 0;
	u32 save_ab = 0;
	u64 chipid = 0;
	struct coul_cali_params cali_param;

	ret = kstrtou32(buf, 0, &save_ab);
	if (ret)
		return ret;

	if (save_ab != 1) {
		coul_err("param %s invalid\n", buf);
		return -EINVAL;
	}

	if (!s_coul_cali_info.vol_ab_valid) {
		coul_err("vol A/B invalid\n");
		return -EINVAL;
	}

	if (!s_coul_cali_info.cur_ab_valid) {
		coul_err("cur A/B invalid\n");
		return -EINVAL;
	}

	ret = coul_get_chipid(&chipid);
	if (ret) {
		coul_err("read chip id fail(%d)\n", ret);
		return ret;
	}

	/* save A/B to NV */
	memset(&cali_param, 0, sizeof(cali_param));
	cali_param.v_offset_a = s_coul_cali_info.vol_a;
	cali_param.v_offset_b = s_coul_cali_info.vol_b;
	cali_param.c_offset_a = s_coul_cali_info.cur_a;
	cali_param.c_offset_b = s_coul_cali_info.cur_b;
	cali_param.chip_id = chipid;
	ret = coul_nv_write_cali(&cali_param);
	if (ret)
		coul_err("save A/B to nv fail(%d)\n", ret);

	s_coul_cali_info.vol_ab_valid = false;
	s_coul_cali_info.cur_ab_valid = false;

	coul_info("Save vol A,B(%d,%d) cur A,B(%d,%d), chipid(0x%llx) to NV success\n",
			cali_param.v_offset_a, cali_param.v_offset_b,
			cali_param.c_offset_a, cali_param.c_offset_b,
			cali_param.chip_id);

	return size;
}

/* read the voltage and current register value */
static DEVICE_ATTR_RO(vol_regval);
static DEVICE_ATTR_RO(cur_regval);

/* read chip id register value */
static DEVICE_ATTR_RO(chip_id);

static DEVICE_ATTR_RO(vol_a);
static DEVICE_ATTR_RO(vol_b);
static DEVICE_ATTR_RO(cur_a);
static DEVICE_ATTR_RO(cur_b);
static DEVICE_ATTR_RO(calied_v);
static DEVICE_ATTR_RO(calied_c);
static DEVICE_ATTR_RO(uncalied_v);
static DEVICE_ATTR_RO(uncalied_c);

/* save the voltage and current A/B value */
static DEVICE_ATTR_WO(vol_ab);
static DEVICE_ATTR_WO(cur_ab);

/* set the messured voltage and current value */
static DEVICE_ATTR_WO(cali_vol1);
static DEVICE_ATTR_WO(cali_vol2);
static DEVICE_ATTR_WO(cali_cur1);
static DEVICE_ATTR_WO(cali_cur2);
/* start calculate the A/B value */
static DEVICE_ATTR_WO(calc_start);

/* save A/B to NV */
static DEVICE_ATTR_WO(save_ab);

static struct attribute *coul_dev_attrs[] = {
	&dev_attr_vol_regval.attr,
	&dev_attr_cur_regval.attr,
	&dev_attr_chip_id.attr,
	&dev_attr_vol_a.attr,
	&dev_attr_vol_b.attr,
	&dev_attr_cur_a.attr,
	&dev_attr_cur_b.attr,
	&dev_attr_calied_v.attr,
	&dev_attr_calied_c.attr,
	&dev_attr_uncalied_v.attr,
	&dev_attr_uncalied_c.attr,

	&dev_attr_vol_ab.attr,
	&dev_attr_cur_ab.attr,

	&dev_attr_cali_vol1.attr,
	&dev_attr_cali_vol2.attr,
	&dev_attr_cali_cur1.attr,
	&dev_attr_cali_cur2.attr,
	&dev_attr_calc_start.attr,

	&dev_attr_save_ab.attr,
	NULL
};

static const struct attribute_group coul_dev_group = {
	.attrs = coul_dev_attrs,
};

int coul_sysfs_init(void)
{
	int ret;
	struct coul_core_device *chip = get_coul_core_dev();
	struct device *dev = chip->cdev;

	ret = device_add_group(dev, &coul_dev_group);
	if (ret) {
		dev_err(dev, "failed to create coul sysfs(%d)\n", ret);
		return ret;
	}

	return 0;
}

void coul_sysfs_exit(void)
{
	struct coul_core_device *chip = get_coul_core_dev();
	struct device *dev = chip->cdev;

	device_remove_group(dev, &coul_dev_group);
}
