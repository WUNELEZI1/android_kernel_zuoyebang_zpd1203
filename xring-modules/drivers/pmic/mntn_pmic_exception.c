// SPDX-License-Identifier: GPL-2.0-only
/*
 * Maintenance MPMIC exceptions
 * Record and process the exception info.
 *
 * Copyright (c) 2024 XRing Technologies Co., Ltd.
 *
 */
#define pr_fmt(fmt)     "PMIC_MNTN: " fmt
#define dev_fmt(fmt)    pr_fmt(fmt)

#include <linux/of_platform.h>
#include <linux/regmap.h>
#include <linux/interrupt.h>
#include <linux/power_supply.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include "soc/xring/xr-pmic-spmi.h"
#include "soc/xring/vote_mng.h"
#include "soc/xring/dmd.h"
#include "mntn_pmic_mdr.h"
#include "dt-bindings/xring/platform-specific/pmic/top_reg.h"
#include "inc/mca/platform/platform_buckchg_class.h"

static int report_dmd = 1;
static int report_mdr = 1;

#define VBD_VOLTAGE_RECOVERIED (mntn_dev->vbat_normal_cnt >= mntn_dev->vbat_detect_cnt_max)

#define VBAT_DET_HISTORY_NUM 32

#define OCP_IRQ_NUM_MAX 32
#define OCP_IRQ_NAME_SIZE 32

#define CORE_FREQ_VOTE_MNG_NUM_MAX  8
#define VBD_VOLTAGE_DETECT_INTERVAL 1000 /* 1s */
#define VBD_CONTROL_INTERVAL        10000 /* 10s */

#define UV_PER_MV                   1000
#define IS_VBAT_VALID(mv)           (((mv) > 2000) && ((mv) < 4500))

#define VBD_DMD_LOG_BUF_SIZE  DMD_MAX_LOG_SIZE
/* dma log prefix size is 93 bytes */
#define VBD_DMD_LOG_DATA_SIZE (VBD_DMD_LOG_BUF_SIZE - 96)

enum vbd_ctrl_level {
	VBD_CTRL_DISABLE = 0,
	VBD_CTRL_LEVEL1  = 1,
	VBD_CTRL_LEVEL2  = 2,
	VBD_CTRL_INVALID,
};

struct mntn_mpmic_exception_dev;
struct ocp_irq_info {
	int  irq;
	char name[OCP_IRQ_NAME_SIZE];
	struct mntn_mpmic_exception_dev *mntn_dev;
	u32 mdr_exception_modid;
};

struct vote_mng_info {
	const char *name;
	struct vote_mng *handle;
	int ch;
};

struct mntn_mpmic_exception_dev {
	struct device *dev;
	struct regmap *reg;

	const char *power_supply_name;
	struct power_supply *vbat_psy;
	int otmp_irq; /* over temp interrupt id */
	int vbd_irq;  /* vbat drop interrupt id */
	struct ocp_irq_info ocp_irqs[OCP_IRQ_NUM_MAX];
	int                 ocp_irq_num;

	u32                vbat_mv_normal;
	u32                vbat_normal_cnt;
	u32                vbat_detect_cnt_max;

	struct vote_mng_info core_freq_vote_mngs[CORE_FREQ_VOTE_MNG_NUM_MAX];
	int    core_freq_vote_mng_num;
	struct dmd_client *vbd_dmd_client;
	struct dmd_client *ocp_dmd_client;
	struct dmd_client *otmp_dmd_client;

	struct task_struct *vbd_thread;
	struct delayed_work vbd_ctrl_delay_work;
	spinlock_t          vbd_lock;
	bool                vbd_en;
	bool                vbd_sw_freq_switch_en; /* switch for sw freq reduce */
	bool                voted_to_low_freq;     /* present the vbd vote freq status */
	bool                vbd_ctrl_en;           /* vbd auto switch control en */
	bool                vbd_intr_generated;
	bool                vbd_detect_vbat_cnt;
	bool                vbd_ctrl_level1_force_disable;
	int                 vbd_ctrl_trigged;
	struct mutex        vbd_level_ctrl_lock;   /* protect the level_ctrl param */
	int                 vbd_ctrl_level1_temp;
	int                 vbd_ctrl_level1_capacity;
	int                 vbd_ctrl_level2_temp;
	int                 vbd_ctrl_level2_capacity;
};

static struct dmd_dev g_vbd_dmd_dev = {
	.name = "VBD",
	.module_name = "PMIC",
	.buff_size = VBD_DMD_LOG_BUF_SIZE,
	.fault_level = "WARNING",
};

static struct dmd_dev g_ocp_dmd_dev = {
	.name = "OCP",
	.module_name = "PMIC",
	.buff_size = VBD_DMD_LOG_BUF_SIZE,
	.fault_level = "CRITICAL",
};

static struct dmd_dev g_otmp_dmd_dev = {
	.name = "OTMP",
	.module_name = "PMIC",
	.buff_size = VBD_DMD_LOG_BUF_SIZE,
	.fault_level = "WARNING",
};

static int of_vote_mng_properties(struct platform_device *pdev,
		struct mntn_mpmic_exception_dev *mntn_dev)
{
	struct device_node *np = pdev->dev.of_node;
	int idx;
	int ret;
	int vote_mng_num;

	/* get vote_mng_num */
	ret = of_property_count_strings(np, "vote-mng-names");
	if (ret < 0) {
		dev_err(&pdev->dev, "Fail to get vote-mng-names elem count\n");
		return ret;
	}
	vote_mng_num = ret;
	if (vote_mng_num > CORE_FREQ_VOTE_MNG_NUM_MAX) {
		dev_err(&pdev->dev, "Invalid vote_mng_num %d, max is %d", vote_mng_num,
				CORE_FREQ_VOTE_MNG_NUM_MAX);
		return -EINVAL;
	}

	ret = of_property_count_u32_elems(np, "vote-mng-ch");
	if (ret < 0) {
		dev_err(&pdev->dev, "Fail to get vote-mng-ch elem count\n");
		return ret;
	}
	if (ret != vote_mng_num) {
		dev_err(&pdev->dev, "vote-mng-ch count %d != vote-mng-names count %d\n",
				ret, vote_mng_num);
		return -EINVAL;
	}

	for (idx = 0; idx < vote_mng_num; idx++) {
		const char *vote_name;
		u32 vote_ch;
		struct vote_mng *handle;

		ret = of_property_read_string_index(np, "vote-mng-names",
				idx, &vote_name);
		if (ret < 0) {
			dev_err(&pdev->dev, "Fail to get vote-mng-name[%d]\n", idx);
			return ret;
		}

		ret = of_property_read_u32_index(np, "vote-mng-ch", idx, &vote_ch);
		if (ret < 0) {
			dev_err(&pdev->dev, "Fail to get vote-mng-ch[%d]\n", idx);
			return ret;
		}

		handle = vote_mng_get(&pdev->dev, vote_name);
		if (IS_ERR_OR_NULL(handle)) {
			dev_err(&pdev->dev, "Fail to get vote mng %s handle\n", vote_name);
			return -EPERM;
		}

		mntn_dev->core_freq_vote_mngs[idx].name = vote_name;
		mntn_dev->core_freq_vote_mngs[idx].handle = handle;
		mntn_dev->core_freq_vote_mngs[idx].ch = vote_ch;
	}

	mntn_dev->core_freq_vote_mng_num = vote_mng_num;

	return 0;
}

static int of_parse_dts(struct platform_device *pdev,
		struct mntn_mpmic_exception_dev *mntn_dev)
{
	struct device_node *np = pdev->dev.of_node;
	struct property *prop;
	const char *irq_name;
	int ocp_irq_num = 0;
	int idx;
	int ret;

	ret = platform_get_irq_byname(pdev, "over_temp");
	if (ret < 0)
		return ret;
	mntn_dev->otmp_irq = ret;

	ret = platform_get_irq_byname(pdev, "vbat_drop");
	if (ret < 0)
		return ret;
	mntn_dev->vbd_irq = ret;

	of_property_for_each_string(np, "interrupt-names", prop, irq_name) {
		if (!strncmp(irq_name, "ocp-", 4)) {
			struct ocp_irq_info *irq_info;

			ret = platform_get_irq_byname(pdev, irq_name);
			if (ret < 0)
				return ret;

			irq_info = &mntn_dev->ocp_irqs[ocp_irq_num];
			irq_info->mntn_dev = mntn_dev;
			irq_info->irq = ret;
			snprintf(irq_info->name, OCP_IRQ_NAME_SIZE, "mpmic_%s", irq_name);
			irq_info->mdr_exception_modid = mpmic_mdr_get_e_modid_by_ocp_irq_name(irq_name);
			if (irq_info->mdr_exception_modid == 0) {
				dev_err(&pdev->dev, "Fail to get %s MDR MODID\n", irq_name);
				return -EINVAL;
			}

			ocp_irq_num++;
			if (ocp_irq_num >= OCP_IRQ_NUM_MAX) {
				dev_err(&pdev->dev, "ocp irq num >= %d\n", OCP_IRQ_NUM_MAX);
				return -EINVAL;
			}
		}
	}
	mntn_dev->ocp_irq_num = ocp_irq_num;

	ret = of_property_read_u32(np, "vbat_mv_normal",
			&mntn_dev->vbat_mv_normal);
	if (ret < 0) {
		dev_err(&pdev->dev, "Fail to parse vbat_mv_normal, ret=%d\n", ret);
		return ret;
	}

	ret = of_property_read_u32(np, "vbat_detect_cnt_max",
			&mntn_dev->vbat_detect_cnt_max);
	if (ret < 0) {
		dev_err(&pdev->dev, "Fail to parse vbat_detect_cnt_max, ret=%d\n", ret);
		return ret;
	}

	ret = of_property_read_s32(np, "vbd_ctrl_level1_temp",
			&mntn_dev->vbd_ctrl_level1_temp);
	if (ret < 0) {
		dev_err(&pdev->dev, "Fail to parse vbd_ctrl_level1_temp, ret=%d\n", ret);
		return ret;
	}
	ret = of_property_read_s32(np, "vbd_ctrl_level1_cap",
			&mntn_dev->vbd_ctrl_level1_capacity);
	if (ret < 0) {
		dev_err(&pdev->dev, "Fail to parse vbd_ctrl_level1_cap, ret=%d\n", ret);
		return ret;
	}
	ret = of_property_read_s32(np, "vbd_ctrl_level2_temp",
			&mntn_dev->vbd_ctrl_level2_temp);
	if (ret < 0) {
		dev_err(&pdev->dev, "Fail to parse vbd_ctrl_level2_temp, ret=%d\n", ret);
		return ret;
	}
	ret = of_property_read_s32(np, "vbd_ctrl_level2_cap",
			&mntn_dev->vbd_ctrl_level2_capacity);
	if (ret < 0) {
		dev_err(&pdev->dev, "Fail to parse vbd_ctrl_level2_cap, ret=%d\n", ret);
		return ret;
	}

	ret = of_property_read_string(np, "power_supply", &mntn_dev->power_supply_name);
	if (ret < 0) {
		dev_err(&pdev->dev, "Fail to parse power_supply, ret=%d\n", ret);
		return ret;
	}

	ret = of_vote_mng_properties(pdev, mntn_dev);
	if (ret < 0)
		return ret;

	dev_dbg(&pdev->dev, "ocp_irq_num=%d, (vbat:mv_normal=%d,det_cnt_max=%d)\n",
			ocp_irq_num, mntn_dev->vbat_mv_normal,
			mntn_dev->vbat_detect_cnt_max);

	dev_dbg(&pdev->dev, "core_freq_vote_mng_num = %d\n",
			mntn_dev->core_freq_vote_mng_num);
	for (idx = 0; idx < mntn_dev->core_freq_vote_mng_num; idx++) {
		dev_dbg(&pdev->dev, "core_freq_vote_mngs[%d]: name=%s, ch=%d\n", idx,
				mntn_dev->core_freq_vote_mngs[idx].name,
				mntn_dev->core_freq_vote_mngs[idx].ch);
	}

	return 0;
}

irqreturn_t otmp_irq_handler(int irq, void *data)
{
	struct mntn_mpmic_exception_dev *mntn_dev = data;

	pr_err("over temp exception\n");

	dmd_client_record(mntn_dev->otmp_dmd_client,
			DMD_MPMIC_OTMP_ERROR_NO, "pmic over temp");

	return IRQ_HANDLED;
}

/* reduce the core frequency to the lowest level
 * vote the max freq as 0
 */
static void vote_core_freq_reduce_to_low(
		struct mntn_mpmic_exception_dev *mntn_dev)
{
	struct vote_mng_info *vote_mng;
	int idx;

	pr_debug("%s vote_mng_num=%d\n", __func__, mntn_dev->core_freq_vote_mng_num);

	for (idx = 0; idx < mntn_dev->core_freq_vote_mng_num; idx++) {
		vote_mng = &mntn_dev->core_freq_vote_mngs[idx];

		pr_debug("%s vote_mng_vote_max to 0\n", vote_mng->name);
		vote_mng_vote_max(vote_mng->handle, vote_mng->ch, 0);
	}
}

/* recovery the core frequency to the normal level
 * vote the max freq as U32_MAX
 */
static void vote_core_freq_recovery_to_normal(
		struct mntn_mpmic_exception_dev *mntn_dev)
{
	struct vote_mng_info *vote_mng;
	int idx;

	pr_debug("%s vote_mng_num=%d\n", __func__, mntn_dev->core_freq_vote_mng_num);

	for (idx = 0; idx < mntn_dev->core_freq_vote_mng_num; idx++) {
		vote_mng = &mntn_dev->core_freq_vote_mngs[idx];

		pr_debug("%s vote_mng_vote_max to 0x%x\n", vote_mng->name, U32_MAX);
		vote_mng_vote_max(vote_mng->handle, vote_mng->ch, U32_MAX);
	}
}

/* clear the VBD interrupt
 * when the interrupt is cleared, the hardware 1/2 Frequency division
 * will be cancelled
 */
static void clear_vbd_intr(struct mntn_mpmic_exception_dev *mntn_dev)
{
	unsigned int reg_val;

	reg_val = 1 << TOP_REG_PMIC_LVL2INT_O10_IRQ_INT_FORCE_PCTRL_UVP_VBD_INT_RAW_SHIFT;
	regmap_write(mntn_dev->reg, TOP_REG_PMIC_LVL2INT_O10_IRQ_INT_RAW, reg_val);
	/* after read any register, the interrupt is really cleared */
	regmap_read(mntn_dev->reg, TOP_REG_PMIC_LVL2INT_O10_IRQ_INT_STATUS, &reg_val);
}

static int vbd_set_vbd_en(struct mntn_mpmic_exception_dev *mntn_dev, bool vbd_en)
{
	unsigned int reg_val_rd = 0;
	unsigned int reg_val_wr;
	int ret;

	ret = xr_pmic_reg_read(TOP_REG_PCTRL_CONFIG18, &reg_val_rd);
	if (ret) {
		pr_err("%s read reg failed(%d)\n", __func__, ret);
		return -EIO;
	}

	if (vbd_en)
		reg_val_wr = reg_val_rd & (~TOP_REG_PCTRL_CONFIG18_SW_SYS_VBD_MASK_MASK);
	else
		reg_val_wr = reg_val_rd | TOP_REG_PCTRL_CONFIG18_SW_SYS_VBD_MASK_MASK;

	ret = xr_pmic_reg_write(TOP_REG_PCTRL_CONFIG18, reg_val_wr);
	if (ret) {
		pr_err("%s write reg failed(%d)\n", __func__, ret);
		return -EIO;
	}

	mntn_dev->vbd_en = vbd_en;

	return 0;
}

static irqreturn_t vbd_hw_irq_handler(int irq, void *data)
{
	struct mntn_mpmic_exception_dev *mntn_dev = data;
	unsigned long flags;
	bool voted_to_low_freq = false;

	spin_lock_irqsave(&mntn_dev->vbd_lock, flags);
	if (!mntn_dev->vbd_en) {
		clear_vbd_intr(mntn_dev);
		spin_unlock_irqrestore(&mntn_dev->vbd_lock, flags);
		pr_info("VBD is disabled, exit\n");
		return IRQ_HANDLED;
	}

	if (mntn_dev->vbd_sw_freq_switch_en) {
		vote_core_freq_reduce_to_low(mntn_dev);
		mntn_dev->voted_to_low_freq = true;
		voted_to_low_freq = true;
	}
	spin_unlock_irqrestore(&mntn_dev->vbd_lock, flags);

	disable_irq_nosync(irq);

	mntn_dev->vbd_intr_generated = true;
	mntn_dev->vbd_detect_vbat_cnt = 0;
	mntn_dev->vbat_normal_cnt = 0;
	pr_info("VBD intr, voted_to_low_freq=%d\n", voted_to_low_freq);

	wake_up_process(mntn_dev->vbd_thread);

	return IRQ_HANDLED;
}

static void vbd_ctrl_delay_work_fn(struct work_struct *work)
{
	struct mntn_mpmic_exception_dev *mntn_dev;

	mntn_dev = container_of(work, struct mntn_mpmic_exception_dev, vbd_ctrl_delay_work.work);

	pr_debug("wakeup vbd thread\n");
	mntn_dev->vbd_ctrl_trigged = true;
	wake_up_process(mntn_dev->vbd_thread);
}

static int vbd_get_bat_temp(struct mntn_mpmic_exception_dev *mntn_dev, int *temp)
{
	union power_supply_propval pval = {0,};
	int ret;

	/* get vbat capacity and temperature */
	if (mntn_dev->vbat_psy == NULL) {
		pr_debug("%s psy is null\n", __func__);
		return -EINVAL;
	}

	ret = power_supply_get_property(mntn_dev->vbat_psy,
			POWER_SUPPLY_PROP_TEMP, &pval);
	if (ret < 0) {
		pr_debug("get battery temp failed(%d)\n", ret);
		return -EIO;
	}
	*temp = pval.intval;

	return 0;
}

static int vbd_get_bat_capacity(struct mntn_mpmic_exception_dev *mntn_dev,
		int *capacity)
{
	union power_supply_propval pval = {0,};
	int ret;

	/* get vbat capacity and temperature */
	if (mntn_dev->vbat_psy == NULL) {
		pr_debug("%s psy is null\n", __func__);
		return -EINVAL;
	}

	ret = power_supply_get_property(mntn_dev->vbat_psy,
			POWER_SUPPLY_PROP_CAPACITY, &pval);
	if (ret < 0) {
		pr_debug("get battery capacity failed(%d)\n", ret);
		return -EIO;
	}
	*capacity = pval.intval;

	return 0;
}


static int vbd_ctrl_get_contrl_level(struct mntn_mpmic_exception_dev *mntn_dev,
		enum vbd_ctrl_level *level)
{
	int ret;
	int bat_temp  = 0;
	int bat_capacity = 0;

	ret = vbd_get_bat_temp(mntn_dev, &bat_temp);
	if (ret < 0)
		return ret;
	ret = vbd_get_bat_capacity(mntn_dev, &bat_capacity);
	if (ret < 0)
		return ret;
	pr_info("battery temp:%d, capacity:%d\n", bat_temp, bat_capacity);

	if (bat_temp < mntn_dev->vbd_ctrl_level2_temp &&
			bat_capacity < mntn_dev->vbd_ctrl_level2_capacity)
		*level = VBD_CTRL_LEVEL2;
	else if (bat_temp < mntn_dev->vbd_ctrl_level1_temp &&
			bat_capacity < mntn_dev->vbd_ctrl_level1_capacity)
		*level = VBD_CTRL_LEVEL1;
	else
		*level = VBD_CTRL_DISABLE;

	return 0;
}

static void _do_vbd_ctrl(struct mntn_mpmic_exception_dev *mntn_dev,
		enum vbd_ctrl_level ctrl_level)
{
	int vbd_en = false;
	int vbd_sw_freq_switch_en = false;
	unsigned long flags;
	bool update_vbd_en = false;
	bool update_sw_switch = false;
	bool voted_to_low_freq_1;
	bool voted_to_low_freq_2;
	int ret;

	switch (ctrl_level) {
	case VBD_CTRL_LEVEL1:
		if (mntn_dev->vbd_ctrl_level1_force_disable)
			vbd_en = false;
		else
			vbd_en = true;
		vbd_sw_freq_switch_en = false;
		break;
	case VBD_CTRL_LEVEL2:
		vbd_en = true;
		vbd_sw_freq_switch_en = true;
		break;
	case VBD_CTRL_DISABLE:
		vbd_en = false;
		vbd_sw_freq_switch_en = false;
		break;
	default:
		pr_err("Invalid vbd_ctrl level %d\n", ctrl_level);
		return;
	}

	spin_lock_irqsave(&mntn_dev->vbd_lock, flags);
	voted_to_low_freq_1 = mntn_dev->voted_to_low_freq;

	if (vbd_en != mntn_dev->vbd_en) {
		update_vbd_en = true;
		if (vbd_en) {
			clear_vbd_intr(mntn_dev);
			ret = vbd_set_vbd_en(mntn_dev, true);
			if (ret < 0)
				pr_warn("vbd_ctrl enable vbd failed!");
		} else {
			ret = vbd_set_vbd_en(mntn_dev, false);
			if (ret < 0) {
				spin_unlock_irqrestore(&mntn_dev->vbd_lock, flags);
				pr_warn("vbd_ctrl disable vbd failed!");
				return;
			}
			clear_vbd_intr(mntn_dev);
			mntn_dev->vbd_intr_generated = false;
			if (mntn_dev->voted_to_low_freq) {
				vote_core_freq_recovery_to_normal(mntn_dev);
				mntn_dev->voted_to_low_freq = false;
			}
		}
	}

	if (vbd_sw_freq_switch_en != mntn_dev->vbd_sw_freq_switch_en) {
		update_sw_switch = true;
		if (vbd_sw_freq_switch_en) {
			if (mntn_dev->vbd_intr_generated) {
				if (!mntn_dev->voted_to_low_freq) {
					vote_core_freq_reduce_to_low(mntn_dev);
					mntn_dev->voted_to_low_freq = true;
				}
			}
		} else {
			if (mntn_dev->voted_to_low_freq) {
				vote_core_freq_recovery_to_normal(mntn_dev);
				mntn_dev->voted_to_low_freq = false;
			}
		}
		mntn_dev->vbd_sw_freq_switch_en = vbd_sw_freq_switch_en;
	}

	voted_to_low_freq_2 = mntn_dev->voted_to_low_freq;
	spin_unlock_irqrestore(&mntn_dev->vbd_lock, flags);

	if (update_vbd_en)
		pr_info("vbd_ctrl level=%d, set vbd_en:%d\n", ctrl_level, vbd_en);
	if (update_sw_switch)
		pr_info("vbd_ctrl level=%d, set sw_freq_switch:%d, voted_to_low_freq: %d->%d\n",
				ctrl_level, vbd_sw_freq_switch_en,
				voted_to_low_freq_1, voted_to_low_freq_2);
}

static void vbd_dynamic_switch_control(struct mntn_mpmic_exception_dev *mntn_dev)
{
	int ret;
	enum vbd_ctrl_level ctrl_level;

	ret = vbd_ctrl_get_contrl_level(mntn_dev, &ctrl_level);
	if (ret < 0)
		return;

	_do_vbd_ctrl(mntn_dev, ctrl_level);
}

static int vbd_get_vbat_by_pmic_stat(struct mntn_mpmic_exception_dev *mntn_dev, int *mv)
{
	int ret;
	unsigned int reg_val;

	/* check the vbd warn stat */
	ret = regmap_read(mntn_dev->reg, TOP_REG_INTF_DEBUG2_REG, &reg_val);
	if (ret < 0) {
		pr_err("get vbd warn stat failed(%d)\n", ret);
		return ret;
	}

	if (reg_val & TOP_REG_INTF_DEBUG2_REG_INTF_SYS_UVP_WARN_MASK) {
		pr_info("VBD WARN stat is exist\n");
		*mv = mntn_dev->vbat_mv_normal - 100;
	} else {
		pr_info("VBD WARN stat is cleared\n");
		*mv = mntn_dev->vbat_mv_normal + 100;
	}

	return 0;
}

static int vbd_get_vbat(struct mntn_mpmic_exception_dev *mntn_dev, int *mv)
{
	union power_supply_propval pval = { 0 };
	int ret = 0;

	if (mntn_dev->vbat_psy) {
		ret = power_supply_get_property(mntn_dev->vbat_psy,
				POWER_SUPPLY_PROP_VOLTAGE_NOW, &pval);
		*mv = pval.intval / UV_PER_MV;

		if ((ret < 0) || !IS_VBAT_VALID(*mv)) {
			dev_err(mntn_dev->dev, "power_supply get volt fail (ret=%d, vbat=%dmv)\n",
					ret, *mv);
			ret = vbd_get_vbat_by_pmic_stat(mntn_dev, mv);
		}
	} else
		ret = vbd_get_vbat_by_pmic_stat(mntn_dev, mv);

	return ret;
}

static void vbd_dmd_record_vbat_history(struct mntn_mpmic_exception_dev *mntn_dev,
		int *vbat_history, int vbat_history_num)
{
	char dmd_vbd_log[VBD_DMD_LOG_DATA_SIZE] = {0};
	size_t dmd_log_len = 0;
	int vbat_mv;
	int i;

	dmd_log_len = snprintf(dmd_vbd_log, sizeof(dmd_vbd_log), "vbat_det end, vbat history: ");
	for (i = 0; i < vbat_history_num; i++) {
		vbat_mv = vbat_history[i];

		if (dmd_log_len < sizeof(dmd_vbd_log))
			dmd_log_len += snprintf(dmd_vbd_log + dmd_log_len, sizeof(dmd_vbd_log) - dmd_log_len, "%d ", vbat_mv);
	}

	dmd_client_record(mntn_dev->vbd_dmd_client, DMD_MPMIC_VBD_ERROR_NO, "%s", dmd_vbd_log);
}

static int vbd_thread_fn(void *data)
{
	struct mntn_mpmic_exception_dev *mntn_dev;
	int vbat_history[VBAT_DET_HISTORY_NUM] = {0};
	int vbat_history_num = 0;
	int ret;

	mntn_dev = (struct mntn_mpmic_exception_dev *)data;

	set_freezable();

repeat:
	set_current_state(TASK_INTERRUPTIBLE);	/* mb paired w/ kthread_stop */

	if (kthread_should_stop()) {
		pr_info("vbd_thread stopping\n");
		__set_current_state(TASK_RUNNING);
		return 0;
	}

	pr_debug("vbd_thread is runing\n");
	if (mntn_dev->vbat_psy == NULL)
		mntn_dev->vbat_psy = power_supply_get_by_name(mntn_dev->power_supply_name);

	if (mntn_dev->vbd_ctrl_en && mntn_dev->vbd_ctrl_trigged) {
		vbd_dynamic_switch_control(mntn_dev);
		mntn_dev->vbd_ctrl_trigged = false;
		schedule_delayed_work(&mntn_dev->vbd_ctrl_delay_work, msecs_to_jiffies(VBD_CONTROL_INTERVAL));
	}

	/* detect vbat voltage and recovery freq */
	if (mntn_dev->vbd_intr_generated) {
		int vbat_mv = 0;

		if (mntn_dev->vbd_detect_vbat_cnt == 0) {
			dmd_client_record(mntn_dev->vbd_dmd_client, DMD_MPMIC_VBD_ERROR_NO, "VBD detected");
			vbat_history_num = 0;
			mntn_dev->vbd_detect_vbat_cnt++;
		}

		if (!VBD_VOLTAGE_RECOVERIED) {
			ret = vbd_get_vbat(mntn_dev, &vbat_mv);
			if (ret < 0)
				dev_err(mntn_dev->dev, "Fail to get vbat voltage, ret=%d\n", ret);
			else {
				if (vbat_mv > (mntn_dev->vbat_mv_normal))
					mntn_dev->vbat_normal_cnt++;
				else
					mntn_dev->vbat_normal_cnt = 0;
				if (vbat_history_num < VBAT_DET_HISTORY_NUM)
					vbat_history[vbat_history_num++] = vbat_mv;
				pr_info("vbat %d mv!\n", vbat_mv);
			}
		}

		/* voltage recoveried, recovery freq */
		if (VBD_VOLTAGE_RECOVERIED) {
			clear_vbd_intr(mntn_dev);
			if (mntn_dev->voted_to_low_freq) {
				vote_core_freq_recovery_to_normal(mntn_dev);
				mntn_dev->voted_to_low_freq = false;
				pr_info("freq_recovery_to_normal!\n");
			}
			if (report_dmd)
				vbd_dmd_record_vbat_history(mntn_dev, vbat_history, vbat_history_num);

			mntn_dev->vbd_intr_generated = false;
			pr_info("VBD exit!\n");
			enable_irq(mntn_dev->vbd_irq);
			goto schedule_out;
		}

		/* sleep, and detect again */
		msleep_interruptible(VBD_VOLTAGE_DETECT_INTERVAL);
		goto next_loop;
	}

schedule_out:
	if (!freezing(current))
		schedule();

next_loop:
	try_to_freeze();
	cond_resched();

	goto repeat;
}

static irqreturn_t ocp_irq_handler(int irq, void *data)
{
	struct ocp_irq_info *irq_info = data;
	struct mntn_mpmic_exception_dev *mntn_dev = irq_info->mntn_dev;

	pr_err("%s exception\n", irq_info->name);

	if (ocp_reset_system && report_mdr)
		mdr_syserr_process_for_ap(irq_info->mdr_exception_modid, 0, 0);
	else if (report_dmd)
		dmd_client_record(mntn_dev->ocp_dmd_client, DMD_MPMIC_OCP_ERROR_NO, "%s", irq_info->name);

	return IRQ_HANDLED;
}

static int mntn_mpmic_register_exception_irq(
		struct mntn_mpmic_exception_dev *mntn_dev)
{
	struct device *dev = mntn_dev->dev;
	int idx;
	int ret;

	ret = devm_request_threaded_irq(dev, mntn_dev->otmp_irq, NULL,
			otmp_irq_handler, IRQF_ONESHOT, "mpmic_otmp", mntn_dev);
	if (ret < 0) {
		dev_err(dev, "Fail to request mpmic_otmp irq %d\n", mntn_dev->otmp_irq);
		return ret;
	}

	ret = devm_request_threaded_irq(dev, mntn_dev->vbd_irq, vbd_hw_irq_handler,
			NULL, IRQF_ONESHOT, "mpmic_vbd", mntn_dev);
	if (ret < 0) {
		dev_err(dev, "Fail to request mpmic_vbd irq %d\n", mntn_dev->vbd_irq);
		return ret;
	}

	for (idx = 0; idx < mntn_dev->ocp_irq_num; idx++) {
		struct ocp_irq_info *irq_info = &mntn_dev->ocp_irqs[idx];

		ret = devm_request_threaded_irq(dev, irq_info->irq, NULL,
				ocp_irq_handler, IRQF_ONESHOT, irq_info->name, irq_info);
		if (ret < 0) {
			dev_err(dev, "Fail to request %s irq %d\n",
					irq_info->name, irq_info->irq);
			return ret;
		}
	}

	return 0;
}

static void mntn_mpmic_dmd_deinit(struct mntn_mpmic_exception_dev *mntn_dev)
{
	if (mntn_dev->vbd_dmd_client) {
		dmd_unregister_client(mntn_dev->vbd_dmd_client, &g_vbd_dmd_dev);
		mntn_dev->vbd_dmd_client = NULL;
	}

	if (mntn_dev->ocp_dmd_client) {
		dmd_unregister_client(mntn_dev->ocp_dmd_client, &g_vbd_dmd_dev);
		mntn_dev->ocp_dmd_client = NULL;
	}

	if (mntn_dev->otmp_dmd_client) {
		dmd_unregister_client(mntn_dev->otmp_dmd_client, &g_vbd_dmd_dev);
		mntn_dev->otmp_dmd_client = NULL;
	}
}

static int mntn_mpmic_dmd_init(struct mntn_mpmic_exception_dev *mntn_dev)
{
	struct dmd_client *client;

	client = dmd_register_client(&g_vbd_dmd_dev);
	if (!client) {
		dmd_log_err("vbd dmd client register fail\n");
		return -EIO;
	}
	mntn_dev->vbd_dmd_client = client;

	client = dmd_register_client(&g_ocp_dmd_dev);
	if (!client) {
		dmd_log_err("ocp dmd client register fail\n");
		mntn_mpmic_dmd_deinit(mntn_dev);
		return -EIO;
	}
	mntn_dev->ocp_dmd_client = client;

	client = dmd_register_client(&g_otmp_dmd_dev);
	if (!client) {
		dmd_log_err("otmp dmd client register fail\n");
		mntn_mpmic_dmd_deinit(mntn_dev);
		return -EIO;
	}
	mntn_dev->otmp_dmd_client = client;

	return 0;
}

static int mntn_mpmic_vbd_init(struct mntn_mpmic_exception_dev *mntn_dev)
{
	unsigned int reg_val_rd = 0;
	int ret;

	ret = xr_pmic_reg_read(TOP_REG_PCTRL_CONFIG18, &reg_val_rd);
	if (ret) {
		pr_err("vbd_init: read reg failed(%d)\n", ret);
		return -EIO;
	}
	if (reg_val_rd & TOP_REG_PCTRL_CONFIG18_SW_SYS_VBD_MASK_MASK)
		mntn_dev->vbd_en = false;
	else
		mntn_dev->vbd_en = true;

	mntn_dev->vbd_sw_freq_switch_en = true;
	mntn_dev->vbd_ctrl_en = true;
	mntn_dev->vbd_ctrl_trigged = true;
	INIT_DELAYED_WORK(&mntn_dev->vbd_ctrl_delay_work, vbd_ctrl_delay_work_fn);

	spin_lock_init(&mntn_dev->vbd_lock);
	mutex_init(&mntn_dev->vbd_level_ctrl_lock);

	mntn_dev->vbd_thread = kthread_create(vbd_thread_fn, mntn_dev, "vbd-thread");
	if (IS_ERR(mntn_dev->vbd_thread)) {
		pr_err("create vbd thread failed\n");
		return PTR_ERR(mntn_dev->vbd_thread);
	}

	pr_info("vbd_en=%d, sw_freq_switch_en=%d, ctrl_en=%d\n",
			mntn_dev->vbd_en, mntn_dev->vbd_sw_freq_switch_en, mntn_dev->vbd_ctrl_en);
	pr_info("vbd level1(temp:%d, cap:%d), level2(temp:%d, cap:%d)\n",
			mntn_dev->vbd_ctrl_level1_temp, mntn_dev->vbd_ctrl_level1_capacity,
			mntn_dev->vbd_ctrl_level2_temp, mntn_dev->vbd_ctrl_level2_capacity);

	wake_up_process(mntn_dev->vbd_thread);

	return 0;
}

static void mntn_mpmic_vbd_deinit(struct mntn_mpmic_exception_dev *mntn_dev)
{
	cancel_delayed_work_sync(&mntn_dev->vbd_ctrl_delay_work);

	if (mntn_dev->vbd_thread) {
		kthread_stop(mntn_dev->vbd_thread);
		mntn_dev->vbd_thread = NULL;
	}
}

static ssize_t vbd_en_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct mntn_mpmic_exception_dev *mntn_dev = dev_get_drvdata(dev);
	bool vbd_level1_en;

	if (kstrtobool(buf, &vbd_level1_en) < 0)
		return -EINVAL;

	mntn_dev->vbd_ctrl_level1_force_disable = !vbd_level1_en;

	pr_info("set level1 force hw div %s\n", vbd_level1_en ? "enable" : "disable");

	return size;
}

static ssize_t vbd_en_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mntn_mpmic_exception_dev *mntn_dev = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%d\n", !mntn_dev->vbd_ctrl_level1_force_disable);
}
static DEVICE_ATTR_RW(vbd_en);

static ssize_t vbd_ctrl_en_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct mntn_mpmic_exception_dev *mntn_dev = dev_get_drvdata(dev);
	bool ctrl_en;
	unsigned long flags;
	int ret;

	if (kstrtobool(buf, &ctrl_en) < 0)
		return -EINVAL;
	if (ctrl_en) {
		pr_info("do not support enable vbd_ctrl_en");
		return -EPERM;
	}

	spin_lock_irqsave(&mntn_dev->vbd_lock, flags);
	if (ctrl_en  == mntn_dev->vbd_ctrl_en) {
		spin_unlock_irqrestore(&mntn_dev->vbd_lock, flags);
		return size;
	}

	/* disable vbd_ctrl_en and enable vbd */
	ret = vbd_set_vbd_en(mntn_dev, true);
	if (ret < 0) {
		spin_unlock_irqrestore(&mntn_dev->vbd_lock, flags);
		pr_warn("vbd_ctrl enable vbd failed!");
		return ret;
	}
	mntn_dev->vbd_ctrl_en = false;
	mntn_dev->vbd_sw_freq_switch_en = true;
	spin_unlock_irqrestore(&mntn_dev->vbd_lock, flags);

	pr_info("set vbd_ctrl_en=%s\n", ctrl_en ? "enable" : "disable");

	return size;
}

static ssize_t vbd_ctrl_en_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mntn_mpmic_exception_dev *mntn_dev = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%d\n", mntn_dev->vbd_ctrl_en);
}
static DEVICE_ATTR_RW(vbd_ctrl_en);

static struct attribute *mntn_mpmic_attrs[] =  {
	&dev_attr_vbd_en.attr,
	&dev_attr_vbd_ctrl_en.attr,
	NULL,
};

static const struct attribute_group mntn_mpmic_attr_group = {
	.attrs = mntn_mpmic_attrs,
};


static int mntn_mpmic_exception_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mntn_mpmic_exception_dev *mntn_dev;
	struct regmap *regmap;
	int ret;

	mntn_dev = devm_kzalloc(dev, sizeof(*mntn_dev), GFP_KERNEL);
	if (!mntn_dev)
		return -ENOMEM;

	regmap = dev_get_regmap(dev->parent, NULL);
	if (!regmap) {
		dev_err(dev, "dev_get_regmap failed\n");
		return -ENODEV;
	}
	mntn_dev->reg = regmap;

	ret = of_parse_dts(pdev, mntn_dev);
	if (ret < 0)
		return ret;

	mntn_dev->dev = dev;

	ret = mpmic_mdr_exception_register();
	if (ret < 0) {
		dev_err(dev, "mpmic_mdr_exception_register failed, ret=%d\n", ret);
		return ret;
	}

	ret = mntn_mpmic_dmd_init(mntn_dev);
	if (ret < 0) {
		dev_err(dev, "mntn_mpmic_dmd_init failed, ret=%d\n", ret);
		mpmic_mdr_exception_unregister();
		return ret;
	}

	ret = mntn_mpmic_vbd_init(mntn_dev);
	if (ret < 0) {
		mntn_mpmic_dmd_deinit(mntn_dev);
		mpmic_mdr_exception_unregister();
		return ret;
	}

	ret = mntn_mpmic_register_exception_irq(mntn_dev);
	if (ret < 0) {
		dev_err(dev, "mntn_mpmic_register_exception_irq failed, ret=%d\n", ret);
		mntn_mpmic_vbd_deinit(mntn_dev);
		mntn_mpmic_dmd_deinit(mntn_dev);
		mpmic_mdr_exception_unregister();
		return ret;
	}

	ret = sysfs_create_group(&mntn_dev->dev->kobj, &mntn_mpmic_attr_group);
	if (ret)
		pr_err("sysfs_create_group fail\n");
	ret = sysfs_create_link(&THIS_MODULE->mkobj.kobj, &mntn_dev->dev->kobj, "mntn");
	if (ret)
		pr_err("sysfs_create_link fail\n");

	platform_set_drvdata(pdev, mntn_dev);

	dev_dbg(dev, "probe success!\n");

	return 0;
}

static int mntn_mpmic_exception_remove(struct platform_device *pdev)
{
	struct mntn_mpmic_exception_dev *mntn_dev;
	int idx;

	mntn_dev = platform_get_drvdata(pdev);

	sysfs_remove_link(&THIS_MODULE->mkobj.kobj, "mntn");
	mntn_mpmic_vbd_deinit(mntn_dev);

	for (idx = 0; idx < mntn_dev->core_freq_vote_mng_num; idx++)
		vote_mng_put(mntn_dev->dev, mntn_dev->core_freq_vote_mngs[idx].handle);

	mntn_mpmic_dmd_deinit(mntn_dev);
	mpmic_mdr_exception_unregister();

	if (mntn_dev->vbat_psy) {
		power_supply_put(mntn_dev->vbat_psy);
		mntn_dev->vbat_psy = NULL;
	}

	dev_dbg(&pdev->dev, "removed!\n");

	return 0;
}

static int mntn_mpmic_exception_suspend(struct device *dev)
{
	dev_info(dev, "%s ++\n", __func__);
	dev_info(dev, "%s --\n", __func__);

	return 0;
}

static int mntn_mpmic_exception_resume(struct device *dev)
{
	dev_info(dev, "%s ++\n", __func__);
	dev_info(dev, "%s --\n", __func__);

	return 0;
}

static const struct dev_pm_ops mntn_mpmic_exception_pm_ops = {
	LATE_SYSTEM_SLEEP_PM_OPS(mntn_mpmic_exception_suspend, mntn_mpmic_exception_resume)
};

static const struct of_device_id mntn_mpmic_exception_match_table[] = {
	{ .compatible = "xring,mntn_mpmic-exception", },
	{}
};
MODULE_DEVICE_TABLE(of, mntn_mpmic_exception_match_table);

static struct platform_driver mntn_mpmic_exception_driver = {
	.driver = {
		.name = "mntn_mpmic_exception",
		.of_match_table = mntn_mpmic_exception_match_table,
		.pm = &mntn_mpmic_exception_pm_ops,
	},
	.probe = mntn_mpmic_exception_probe,
	.remove = mntn_mpmic_exception_remove,
};

int xr_pmic_mntn_init(void)
{
	return platform_driver_register(&mntn_mpmic_exception_driver);
}

void xr_pmic_mntn_exit(void)
{
	platform_driver_unregister(&mntn_mpmic_exception_driver);
}
