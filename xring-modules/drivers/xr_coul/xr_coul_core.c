// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * xr coul driver for Xring coulomb meter
 *
 * Copyright (c) 2024, XRing Technologies Co., Ltd.
 */

#include <linux/poll.h>
#include <linux/miscdevice.h>
#include "xr_coul_core.h"
#include "xr_coul_xp2210.h"
#include "xr_coul_dts.h"
#include "xr_coul_nv.h"

#define SR_MAX_RESUME_TIME      90         /* 90 s */

#define WAKEUP_AVG_CUR_250MA    250
#define WAKEUP_AVG_CUR_500MA    500
#define TEMP_RETRY_TIMES        3
#define HALF                    2
#define CALCULATE_SOC_MS        (20 * 1000)
#define INVALID_BATT_ID_VOL     (-999)
#define LOW_TEMP_TENTH          50
#define LOW_SOC                 30
#define DEFAULT_SOC             50
#define DEFAULT_SOC_ARR_VAL     100
#define MAX_FIFO_LEN            10

static struct coul_core_device *priv_coul_core_dev;

/* set low voltage value according low_power status, state -- normal or low power state */
void coul_set_low_vol_state(struct coul_core_device *chip, int state)
{
	int vol;

	if (!chip) {
		coul_err("NULL point\n");
		return;
	}

	if (state == LOW_INT_STATE_RUNNING)
		vol = (int)chip->v_poweroff;
	else
		vol = (int)chip->v_poweroff_sleep;

	if (chip->common_data.batt_temp <= LOW_INT_TEMP_THRED)
		vol = (chip->v_poweroff_low_temp < vol)
				? chip->v_poweroff_low_temp
				: vol;

	if (!IS_IC_OPS_NULL(chip, set_low_vol_alert_thr))
		chip->core_ic_ops->set_low_vol_alert_thr(vol);
}

void coul_chg_start(struct coul_core_device *chip)
{
	coul_debug(" +\n");

	if (!chip) {
		coul_err("NULL point\n");
		return;
	}
	coul_debug("pre charging state is %d\n", chip->common_data.charging_state);

	if (chip->common_data.charging_state == COUL_CHG_STATE_START) {
		coul_err("charging_state:%d, return\n", chip->common_data.charging_state);
		return;
	}

	chip->common_data.pre_charging_state = chip->common_data.charging_state;
	chip->common_data.charging_state = COUL_CHG_STATE_START;

	chip->kdata_ready = true;
	chip->common_data.event_type = XR_COUL_K2U_EVENT_CHG_START;
	wake_up_interruptible(&chip->coul_wait_q);

	coul_debug(" -\n");
}

void coul_chg_stop(struct coul_core_device *chip)
{
	coul_debug(" +\n");

	if (!chip) {
		coul_err("NULL point\n");
		return;
	}

	if (chip->common_data.charging_state == COUL_CHG_STATE_UNKNOW) {
		coul_err("charging_state:%d, return\n", chip->common_data.charging_state);
		return;
	}

	chip->common_data.pre_charging_state = chip->common_data.charging_state;
	chip->common_data.charging_state = COUL_CHG_STATE_STOP;

	if (chip->common_data.pre_charging_state == COUL_CHG_STATE_FULL) {
		coul_err("charging_state:%d, return\n", chip->common_data.charging_state);
		return;
	}

	coul_set_low_vol_state(chip, LOW_INT_STATE_RUNNING);

	chip->kdata_ready = true;
	chip->common_data.event_type = XR_COUL_K2U_EVENT_CHG_STOP;
	wake_up_interruptible(&chip->coul_wait_q);
	coul_debug(" -\n");
}

void coul_chg_full(struct coul_core_device *chip)
{
	coul_debug(" +\n");

	if (!chip) {
		coul_err("NULL point\n");
		return;
	}

	if (chip->common_data.charging_state != COUL_CHG_STATE_START) {
		coul_err("charging_done, pre charge state is %d\n",
			chip->common_data.charging_state);
		return;
	}

	chip->common_data.pre_charging_state = chip->common_data.charging_state;
	chip->common_data.charging_state = COUL_CHG_STATE_FULL;

	chip->kdata_ready = true;
	chip->common_data.event_type = XR_COUL_K2U_EVENT_CHG_FULL;
	wake_up_interruptible(&chip->coul_wait_q);

	coul_debug(" -\n");
}

struct coul_core_device *get_coul_core_dev(void)
{
	return priv_coul_core_dev;
}

static int xr_coul_device_open(struct inode *inode, struct file *file)
{
	struct coul_core_device *chip = priv_coul_core_dev;

	file->private_data = chip;
	coul_debug("xr coul cdev device opened!\n");

	return 0;
}

static int xr_coul_device_release(struct inode *inode, struct file *file)
{
	coul_debug("xr cul cdev device closed!\n");

	return 0;
}

static ssize_t xr_coul_device_read(struct file *file, char __user *buf, size_t count,
			  loff_t *ppos)
{
	int ret;

	struct coul_core_device *chip = file->private_data;
	unsigned long data_size;

	if (chip->hal_read_type)
		data_size = sizeof(chip->k2u_data);
	else
		data_size = sizeof(chip->common_data);

	if (count < data_size) {
		coul_err("Invalid datasize %lu, expected lesser then %zu\n",
							data_size, count);
		return -EINVAL;
	}

	/* non-blocking access, return */
	if (!chip->kdata_ready && (file->f_flags & O_NONBLOCK))
		return 0;

	/* blocking access wait on kdata_ready */
	if (!(file->f_flags & O_NONBLOCK)) {
		ret = wait_event_interruptible(chip->coul_wait_q,
					chip->kdata_ready);
		if (ret < 0) {
			coul_err("Failed wait! rc=%d\n", ret);
			return ret;
		}
	}

	mutex_lock(&chip->data_lock);

	if (!chip->kdata_ready) {
		coul_err("No Data, false wakeup\n");
		ret = -EFAULT;
		goto fail_read;
	}

	if (chip->hal_read_type) {
		if (copy_to_user(buf, &chip->k2u_data, data_size)) {
			coul_err("Failed in copy_to_user k2hal data\n");
			ret = -EFAULT;
			goto fail_read;
		}
		chip->hal_read_type = 0;
	} else {
		if (copy_to_user(buf, &chip->common_data, data_size)) {
			coul_err("Failed in copy_to_user public data\n");
			ret = -EFAULT;
			goto fail_read;
		}
	}

	chip->kdata_ready = false;

	ret = data_size;
fail_read:
	mutex_unlock(&chip->data_lock);
	return ret;
}

static ssize_t xr_coul_device_write(struct file *file, const char __user *buf,
			size_t count, loff_t *ppos)
{
	int ret = -EINVAL;
	struct coul_core_device *chip = file->private_data;
	unsigned long data_size = sizeof(chip->common_data);

	mutex_lock(&chip->data_lock);
	if (count == 0) {
		coul_err("No data!\n");
		goto fail;
	}

	if (count < data_size) {
		coul_err("Invalid datasize %zu expected %lu\n", count, data_size);
		goto fail;
	}

	if (copy_from_user(&chip->common_data, buf, data_size)) {
		coul_err("Failed in copy_from_user\n");
		ret = -EFAULT;
		goto fail;
	}

	ret = data_size;

	queue_delayed_work(system_power_efficient_wq, &chip->ui_soc_work, 0);
fail:
	mutex_unlock(&chip->data_lock);
	return ret;
}

static unsigned int xr_coul_device_poll(struct file *file, poll_table *wait)
{
	struct coul_core_device *chip = file->private_data;
	unsigned int mask = 0;

	poll_wait(file, &chip->coul_wait_q, wait);

	if (chip->kdata_ready)
		mask = POLLIN | POLLRDNORM;

	return mask;
}

static long coul_data_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned long ret = 0;
	int data;
	long long lldata;
	int data_size = sizeof(int);
	unsigned int lldata_size = sizeof(long long);
	int fifo[MAX_FIFO_LEN];
	int max_size = 0;

	struct coul_core_device *chip = priv_coul_core_dev;

	int __user *data_user = (int __user *)arg;
	long long __user *lldata_user = (long long __user *)arg;

	coul_debug("cmd:0x%x arg:%lu\n", cmd, arg);

	switch (cmd) {
	case COUL_GET_K2U_DATA_BEGIN:
		ret = copy_from_user(&chip->hal_read_type, data_user, data_size);
		if (ret)
			coul_err("COUL_GET_K2U_DATA_BEGIN fail, hal_read_type:%d ret:%lu\n",
				chip->hal_read_type, ret);

		chip->kdata_ready = true;
		wake_up_interruptible(&chip->coul_wait_q);
		break;
	case COUL_GET_K2U_DATA_END:
		if (chip->core_init_flag) {
			chip->common_data.event_type = XR_COUL_K2U_EVENT_INIT;
			chip->kdata_ready = true;
			wake_up_interruptible(&chip->coul_wait_q);
			chip->common_data.soc_work_interval = CALCULATE_SOC_MS;
			queue_delayed_work(system_power_efficient_wq,
				&chip->calc_soc_work,
				round_jiffies_relative(msecs_to_jiffies(chip->common_data.soc_work_interval)));
			chip->core_init_flag = 0;
		}
		break;
	case COUL_GET_CC:
		if (!IS_IC_OPS_NULL(chip, get_cc)) {
			data = chip->core_ic_ops->get_cc();
			ret = copy_to_user(data_user, &data, data_size);
			if (ret)
				coul_err("COUL_GET_CC fail, ret:%lu\n", ret);
		}
		break;
	case COUL_SET_CC:
		if (!IS_IC_OPS_NULL(chip, set_cc)) {
			ret = copy_from_user(&data, data_user, data_size);
			if (ret)
				coul_err("COUL_SET_CC fail, data:%d ret:%lu\n",
					data, ret);
			else
				chip->core_ic_ops->set_cc(data);
		}
		break;
	case COUL_RVAL2UV:
		if (!IS_IC_OPS_NULL(chip, regval_convert_uv)) {
			ret = copy_from_user(&data, data_user, data_size);
			if (ret)
				coul_err("COUL_RVAL2UV fail, data:%d ret:%lu\n",
					data, ret);
			else {
				data = chip->core_ic_ops->regval_convert_uv(data);
				ret = copy_to_user(data_user, &data, data_size);
				if (ret)
					coul_err("COUL_GET_CC fail, ret:%lu\n", ret);
			}
		}
		break;
	case COUL_SET_OCV:
		if (!IS_IC_OPS_NULL(chip, set_ocv)) {
			ret = copy_from_user(&data, data_user, data_size);
			if (ret)
				coul_err("COUL_SET_OCV fail, data:%d ret:%lu\n",
					data, ret);
			else
				chip->core_ic_ops->set_ocv(data);
		}
		break;
	case COUL_GET_OCV:
		if (!IS_IC_OPS_NULL(chip, get_ocv)) {
			data = chip->core_ic_ops->get_ocv();
			ret = copy_to_user(data_user, &data, data_size);
			if (ret)
				coul_err("COUL_GET_OCV fail, ret:%lu\n", ret);
		}
		break;
	case COUL_CLR_OCV:
		if (!IS_IC_OPS_NULL(chip, clear_ocv))
			chip->core_ic_ops->clear_ocv();
		break;
	case COUL_SET_OCV_TEMP:
		if (!IS_IC_OPS_NULL(chip, set_ocv_temp)) {
			ret = copy_from_user(&data, data_user, data_size);
			if (ret)
				coul_err("COUL_SET_OCV_TEMP fail, data:%d ret:%lu\n",
					data, ret);
			else
				chip->core_ic_ops->set_ocv_temp(data);
		}
		break;
	case COUL_GET_OCV_TEMP:
		if (!IS_IC_OPS_NULL(chip, get_ocv_temp)) {
			data = chip->core_ic_ops->get_ocv_temp();
			ret = copy_to_user(data_user, &data, data_size);
			if (ret)
				coul_err("COUL_GET_OCV_TEMP fail, ret:%lu\n", ret);
		}
		break;
	case COUL_SET_LOW_VOL_TH:
		if (!IS_IC_OPS_NULL(chip, set_low_vol_alert_thr)) {
			ret = copy_from_user(&data, data_user, data_size);
			if (ret)
				coul_err("COUL_SET_LOW_VOL_TH fail, data:%d ret:%lu\n",
					data, ret);
			else
				chip->core_ic_ops->set_low_vol_alert_thr(data);
		}
		break;
	case COUL_GET_TIME:
		if (!IS_IC_OPS_NULL(chip, read_coul_time)) {
			lldata = chip->core_ic_ops->read_coul_time();
			ret = copy_to_user(lldata_user, &lldata, lldata_size);
			if (ret)
				coul_err("COUL_GET_TIME fail, ret:%lu\n", ret);
		}
		break;
	case COUL_CLR_TIME:
		if (!IS_IC_OPS_NULL(chip, rst_coul_time))
			chip->core_ic_ops->rst_coul_time();
		break;
	case COUL_CLR_CC:
		if (!IS_IC_OPS_NULL(chip, rst_cc))
			chip->core_ic_ops->rst_cc();
		break;
	case COUL_GET_VOL:
		if (!IS_IC_OPS_NULL(chip, get_batt_vol)) {
			data = chip->core_ic_ops->get_batt_vol();
			ret = copy_to_user(data_user, &data, data_size);
			if (ret)
				coul_err("COUL_GET_VOL fail, ret:%lu\n", ret);
		}
		break;
	case COUL_GET_CUR:
		if (!IS_IC_OPS_NULL(chip, get_batt_cur)) {
			data = chip->core_ic_ops->get_batt_cur();
			ret = copy_to_user(data_user, &data, data_size);
			if (ret)
				coul_err("COUL_GET_CUR fail, ret:%lu\n", ret);
		}
		break;
	case COUL_GET_VOL_FIFO:
		if (!IS_IC_OPS_NULL(chip, get_fifo_depth)) {
			max_size = chip->core_ic_ops->get_fifo_depth();
			if ((!IS_IC_OPS_NULL(chip, get_batt_vol_fifo)) &&
				max_size < MAX_FIFO_LEN) {
				max_size *= data_size;
				chip->core_ic_ops->get_batt_vol_fifo(fifo);
				ret = copy_to_user(data_user, fifo, max_size);
				if (ret)
					coul_err("COUL_GET_VOL_FIFO fail, ret:%lu\n", ret);
			}
		}
		break;
	case COUL_GET_CUR_FIFO:
		if (!IS_IC_OPS_NULL(chip, get_fifo_depth)) {
			max_size = chip->core_ic_ops->get_fifo_depth();
			if ((!IS_IC_OPS_NULL(chip, get_batt_cur_fifo)) &&
				max_size < MAX_FIFO_LEN) {
				max_size *= data_size;
				chip->core_ic_ops->get_batt_cur_fifo(fifo);
				ret = copy_to_user(data_user, fifo, max_size);
				if (ret)
					coul_err("COUL_GET_CUR_FIFO fail, ret:%lu\n", ret);
			}
		}
		break;
	case COUL_GET_LP_CC:
		if (!IS_IC_OPS_NULL(chip, get_lp_compensate_cc)) {
			data = chip->core_ic_ops->get_lp_compensate_cc();
			ret = copy_to_user(data_user, &data, data_size);
			if (ret)
				coul_err("COUL_GET_LP_CC fail, ret:%lu\n", ret);
		}
		break;
	case COUL_SET_OCV_LVL:
		if (!IS_IC_OPS_NULL(chip, set_ocv_level)) {
			ret = copy_from_user(&data, data_user, data_size);
			if (ret)
				coul_err("COUL_SET_OCV_LVL fail, data:%d ret:%lu\n",
					data, ret);
			else
				chip->core_ic_ops->set_ocv_level(data);
		}
		break;
	case COUL_GET_OCV_LVL:
		if (!IS_IC_OPS_NULL(chip, read_ocv_level)) {
			data = chip->core_ic_ops->read_ocv_level();
			ret = copy_to_user(data_user, &data, data_size);
			if (ret)
				coul_err("COUL_GET_OCV_LVL fail, ret:%lu\n", ret);
		}
		break;
	case COUL_GET_TEMP:
		if (!IS_IC_OPS_NULL(chip, get_adc_tbatt)) {
			data = chip->core_ic_ops->get_adc_tbatt();
			if (data == ERR_TEMP) {
				coul_err("COUL_GET_TEMP get err tbatt: %d, use temp:%d\n",
					data, DEFAULT_TEMP);
				data = DEFAULT_TEMP;
			}
			ret = copy_to_user(data_user, &data, data_size);
			if (ret)
				coul_err("COUL_GET_TEMP fail, ret:%lu\n", ret);
		}
		break;
	case COUL_GET_OCV_TYPE:
		if (!IS_IC_OPS_NULL(chip, get_use_saved_ocv_flag)) {
			data = chip->core_ic_ops->get_use_saved_ocv_flag();
			if (data < 0)
				coul_err("COUL_GET_OCV_TYPE err data: %d\n", data);

			ret = copy_to_user(data_user, &data, data_size);
			if (ret)
				coul_err("COUL_GET_OCV_TYPE fail, ret:%lu\n", ret);
		}
		break;
	case COUL_GET_FCC_UPDATE_F:
		if (!IS_IC_OPS_NULL(chip, get_ocv_update_fcc_flag)) {
			data = chip->core_ic_ops->get_ocv_update_fcc_flag();
			if (data < 0)
				coul_err("COUL_GET_FCC_UPDATE_F err data: %d\n", data);

			ret = copy_to_user(data_user, &data, data_size);
			if (ret)
				coul_err("COUL_GET_FCC_UPDATE_F fail, ret:%lu\n", ret);
		}
		break;
	case COUL_SET_FCC_UPDATE_F:
		if (!IS_IC_OPS_NULL(chip, set_ocv_update_fcc_flag)) {
			ret = copy_from_user(&data, data_user, data_size);
			if (ret)
				coul_err("COUL_SET_FCC_UPDATE_F fail, data:%d ret:%lu\n",
					data, ret);
			else
				chip->core_ic_ops->set_ocv_update_fcc_flag(data);
		}
		break;
	default:
		coul_err("err cmd :0x%x from hal\n", cmd);
	}

	return ret;
}

static const struct file_operations xr_coul_fops = {
	.owner		= THIS_MODULE,
	.open		= xr_coul_device_open,
	.release	= xr_coul_device_release,
	.read		= xr_coul_device_read,
	.write		= xr_coul_device_write,
	.poll		= xr_coul_device_poll,
	.unlocked_ioctl = coul_data_ioctl,
};

static struct miscdevice coul_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "coul_cdev",
	.fops  = &xr_coul_fops,
};

static int coul_cdev_register(void)
{
	int ret;

	ret = misc_register(&coul_miscdev);
	if (ret) {
		coul_err("cannot register miscdev on minor=%d (err=%d)\n",
			MISC_DYNAMIC_MINOR, ret);
		goto err;
	}

	coul_debug("'/dev/coul_cdev' successfully created\n");
err:
	return ret;
}

static int cout_get_tbatt_retry(struct coul_core_device *chip)
{
	int cnt = 0;
	int tbatt;
	/* default is no battery in sft and udp, so here temp is fixed 25 */
	if (chip->batt_board_id != BAT_BOARD_PRODUCT) {
		coul_err("not support ntc, use default temp\n");
		return DEFAULT_TEMP;
	}

	if (!IS_IC_OPS_NULL(chip, get_adc_tbatt)) {
		while (cnt++ < TEMP_RETRY_TIMES) {
			tbatt = chip->core_ic_ops->get_adc_tbatt();
			if (tbatt == ERR_TEMP)
				coul_err("Bat temp read fail, retry_cnt = %d\n", cnt);
			else
				return tbatt;
		}
		coul_err("Bat temp read retry 3 times, error\n");
	}

	coul_err("get_adc_tbatt ops is null, use default temp\n");
	return DEFAULT_TEMP;
}

static void coul_soc_arr_init(struct coul_core_device *chip)
{
	int i;

	for (i = 0; i < MAX_SOC_ARR_NUM; i++)
		chip->common_data.soc_arr[i] = DEFAULT_SOC_ARR_VAL;
}

static void chip_initial(struct coul_core_device *chip)
{
	coul_core_init_dts(chip);
	mutex_init(&chip->soc_mutex);
	coul_set_low_vol_state(chip, LOW_INT_STATE_RUNNING);

	if (!IS_IC_OPS_NULL(chip, read_coul_time))
		chip->common_data.sr_resume_time = chip->core_ic_ops->read_coul_time();

	if (!IS_IC_OPS_NULL(chip, get_cc))
		chip->resume_cc = chip->core_ic_ops->get_cc();

	chip->common_data.batt_temp = cout_get_tbatt_retry(chip);

	if (!IS_IC_OPS_NULL(chip, get_fifo_depth))
		chip->k2u_data.fifo_depth = chip->core_ic_ops->get_fifo_depth();

	coul_soc_arr_init(chip);
}

static void coul_set_work_cycle(struct coul_core_device *chip)
{
	if (!chip)
		return;

	if ((chip->common_data.batt_temp < LOW_TEMP_TENTH) &&
		(chip->common_data.charging_state != COUL_CHG_STATE_FULL)) {
		chip->common_data.soc_work_interval = CALCULATE_SOC_MS / HALF;
	} else {
		if (chip->common_data.batt_soc > LOW_SOC)
			chip->common_data.soc_work_interval = CALCULATE_SOC_MS;
		else
			chip->common_data.soc_work_interval = CALCULATE_SOC_MS / HALF;
	}
}

/* calculate soc every(schedule workqueue) CALCULATE_SOC_MS */
static void calculate_soc_work(struct work_struct *work)
{
	int ret;
	struct coul_core_device *chip = container_of(work,
		struct coul_core_device, calc_soc_work.work);

	coul_debug(" +\n");
	if (chip->common_data.is_nv_need_save) {
		ret = store_nv_data(chip);
		if (!ret)
			chip->common_data.is_nv_need_save = 0;
	}

	coul_set_low_vol_state(chip, LOW_INT_STATE_RUNNING);
	coul_set_work_cycle(chip);

	chip->common_data.event_type = XR_COUL_K2U_EVENT_SOC;

	chip->kdata_ready = true;
	wake_up_interruptible(&chip->coul_wait_q);

	queue_delayed_work(system_power_efficient_wq,
		&chip->calc_soc_work,
		round_jiffies_relative(msecs_to_jiffies(chip->common_data.soc_work_interval)));
	coul_debug(" -\n");
}

static void coul_vth_soc_check(struct coul_core_device *chip, int *soc)
{
	int i;
	int curr_soc = *soc;
	int vbatt;

	if (!chip->vth_soc_en)
		return;

	if (IS_IC_OPS_NULL(chip, get_batt_vol)) {
		coul_err("get_batt_vol NULL\n");
		return;
	}

	vbatt = chip->core_ic_ops->get_batt_vol() / PERMILLAGE;

	if (chip->common_data.charging_state == COUL_CHG_STATE_START ||
		chip->common_data.charging_state == COUL_CHG_STATE_FULL)
		return;

	for (i = 0; i < VTH_SOC_LEVEL; i++) {
		if (curr_soc < chip->vth_soc_data[i].soc &&
			chip->common_data.ui_soc >= chip->vth_soc_data[i].soc &&
			vbatt >= chip->vth_soc_data[i].vol) {
			*soc = chip->vth_soc_data[i].soc;
			coul_info("vbat:%d mv, curr_soc:%d, new_soc:%d\n", vbatt, curr_soc, *soc);
			}
	}
}

static void reset_soc_filter_fifo(struct coul_core_device *chip)
{
	int i;

	chip->soc_filter_sum = 0;
	chip->soc_filter_count = 0;

	for (i = 0; i < WINDOW_LEN; i++) {
		chip->soc_filter_fifo[i] = chip->common_data.ui_soc;
		chip->soc_filter_sum += chip->soc_filter_fifo[i];
	}
}

static void soc_filter(struct coul_core_device *chip, int *new_soc)
{
	int index;

	index = chip->soc_filter_count % WINDOW_LEN;

	chip->soc_filter_sum -= chip->soc_filter_fifo[index];
	chip->soc_filter_fifo[index] = *new_soc;
	chip->soc_filter_sum += chip->soc_filter_fifo[index];

	if (++chip->soc_filter_count >= WINDOW_LEN)
		chip->soc_filter_count = 0;

	/* rounding-off 0.5 method */
	*new_soc = (chip->soc_filter_sum) / WINDOW_LEN;
}

static bool coul_check_soc_change(struct coul_core_device *chip)
{
	int soc, tmp_soc;

	tmp_soc = chip->common_data.ui_soc;
	soc = chip->common_data.batt_soc;

	coul_vth_soc_check(chip, &soc);

	soc_filter(chip, &soc);

	if (chip->common_data.charging_state == CHARGING_STATE_FULL) {
		if (tmp_soc < soc && tmp_soc < MAX_SOC)
			tmp_soc++;
	} else {
		if (tmp_soc < soc && tmp_soc < MAX_SOC)
			tmp_soc++;
		else if (tmp_soc > soc && tmp_soc > MIN_SOC) {
			/* let soc = 1 when vbat > 3v */
			if (!(tmp_soc == SOC_1 &&
				(coul_intf_read_batt_vol_uv() > SHUT_DOWN_VBAT)))
				tmp_soc--;
		}
	}

	if (chip->common_data.ui_soc != tmp_soc) {
		coul_info("old_soc: %d, new_soc: %d, chg_status:%d\n",
			chip->common_data.ui_soc, tmp_soc, chip->common_data.charging_state);
		chip->common_data.ui_soc = tmp_soc;
		return true;
	}

	return false;
}

/* update ui soc every(schedule workqueue) CALCULATE_SOC_MS */
static void ui_soc_work(struct work_struct *work)
{
	struct coul_core_device *chip = container_of(work,
		struct coul_core_device, ui_soc_work.work);

	coul_debug(" +\n");

	if (chip->battery_psy == NULL) {
		chip->battery_psy = power_supply_get_by_name("battery");
		if (chip->battery_psy == NULL)
			return;
	}

	if (coul_check_soc_change(chip))
		power_supply_changed(chip->battery_psy);

	coul_debug(" -\n");
}

static void coul_work_register(struct coul_core_device *chip)
{
	/* Init soc calc work */
	INIT_DELAYED_WORK(&chip->calc_soc_work, calculate_soc_work);

	/* Init soc calc work */
	INIT_DELAYED_WORK(&chip->ui_soc_work, ui_soc_work);

	chip->work_init_flag = 1;
}

static void coul_get_last_soc(struct coul_core_device *chip)
{
	bool flag_soc_valid = false;
	short soc_regval = 0;

	if (!IS_IC_OPS_NULL(chip, read_last_soc_flag))
		chip->core_ic_ops->read_last_soc_flag(&flag_soc_valid);

	if (!IS_IC_OPS_NULL(chip, read_last_soc))
		chip->core_ic_ops->read_last_soc(&soc_regval);

	coul_info("flag=%d,chip->batt_soc=%d,soc_regval=%d\n",
		flag_soc_valid, chip->common_data.batt_soc, soc_regval);

	if (flag_soc_valid) {
		chip->k2u_data.last_powerdown_soc = soc_regval;
		coul_info("last_powerdown_soc=%d,flag=%d\n",
			soc_regval, flag_soc_valid);
	} else {
		chip->k2u_data.last_powerdown_soc = -1;
		coul_info("last_powerdown_soc invalid\n");
	}

	if (!IS_IC_OPS_NULL(chip, rst_last_soc_flag))
		chip->core_ic_ops->rst_last_soc_flag();
}

static void get_initial_soc(struct coul_core_device *chip)
{
	/* get the first soc value */
	chip_lock();
	coul_get_last_soc(chip);

	if ((!chip->shutdown_soc_en) || (chip->k2u_data.last_powerdown_soc < 0)) {
		coul_info("last_powerdown_soc invalid or closed\n");
		chip_unlock();
		return;
	}

	chip->common_data.batt_soc = chip->k2u_data.last_powerdown_soc;
	chip->common_data.ui_soc = chip->common_data.batt_soc;
	coul_info("battery last soc=%d\n", chip->k2u_data.last_powerdown_soc);
	reset_soc_filter_fifo(chip);
	chip_unlock();
}

static int init_batt_profile(struct coul_core_device *chip)
{
	struct coul_battery_data *tmp_batt_data;

	if (!chip)
		return -EINVAL;

	chip->k2u_data.batt_exist = 1;

	tmp_batt_data = get_battery_profile();
	if (!tmp_batt_data) {
		coul_err("get batt profile fail\n");
		return -EINVAL;
	}

	/* set battery data */
	chip->k2u_data.batt_data = *tmp_batt_data;

	coul_debug("get batt profile success\n");

	return 0;
}

static void rst_moved_batt_param(struct coul_core_device *chip)
{
	int batt_sn_changed_flag = 0;
	int new_batt = 0;

	if ((!chip) || (!chip->core_ic_ops))
		return;

	coul_info("battery is completely pulled out and inserted once\n");

	/*TODO read batt_sn_changed_flag from battery */
	if (!chip->core_ic_ops->is_new_battery(&new_batt)) {
		if (new_batt)
			batt_sn_changed_flag = 1;
	}

	if (batt_sn_changed_flag) { /* clear nv info*/
		chip->common_data.batt_changed_flag = 1;
		coul_nv_params_deinitial(chip);

		if (!IS_IC_OPS_NULL(chip, rst_last_soc_flag))
			chip->core_ic_ops->rst_last_soc_flag();

		coul_info("battery changed, reset chargecycles\n");
	} else {
		coul_nv_params_initial(chip);
	}
}

static int coul_probe(struct platform_device *pdev)
{
	struct coul_core_device *chip = NULL;
	int retval;

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);

	if (!chip)
		return -ENOMEM;

	chip->work_init_flag = 0;
	chip->core_init_flag = 0;

	chip->dev = &pdev->dev;

	init_waitqueue_head(&chip->coul_wait_q);

	mutex_init(&chip->data_lock);
	retval = coul_cdev_register();
	if (retval)
		goto coul_failed;
	chip->cdev = coul_miscdev.this_device;

	retval = coul_devops_register(chip);
	if (retval)
		goto coul_cdev_unreg;

	priv_coul_core_dev = chip;

	platform_set_drvdata(pdev, chip);

	chip_initial(chip);

	retval = init_batt_profile(chip);
	if (retval)
		goto coul_unset_pdev;

	coul_work_register(chip);

	rst_moved_batt_param(chip);
	get_initial_soc(chip);

	chip->core_init_flag = 1;

	coul_debug("coul core probe success!!!\n");
	return 0;

coul_unset_pdev:
	platform_set_drvdata(pdev, NULL);
	priv_coul_core_dev = NULL;

coul_cdev_unreg:
	misc_deregister(&coul_miscdev);
coul_failed:
	coul_err("coul core probe failed\n");
	return retval;
}

static int coul_remove(struct platform_device *pdev)
{
	struct coul_core_device *chip = platform_get_drvdata(pdev);

	if (chip->work_init_flag == 1)
		cancel_delayed_work_sync(&chip->calc_soc_work);

	misc_deregister(&coul_miscdev);
	mutex_destroy(&chip->data_lock);
	mutex_destroy(&chip->soc_mutex);

	return 0;
}

static void coul_shutdown(struct platform_device *pdev)
{
	struct coul_core_device *chip = platform_get_drvdata(pdev);
	int last_soc = chip->common_data.ui_soc;

	if (!chip) {
		coul_err("chip is NULL\n");
		return;
	}

	if (last_soc >= 0 && (!IS_IC_OPS_NULL(chip, set_last_soc)))
		chip->core_ic_ops->set_last_soc(last_soc);

	if (chip->work_init_flag == 1)
		cancel_delayed_work_sync(&chip->calc_soc_work);
}

#ifdef CONFIG_PM
/* clear sleep and wakeup global variable */
static void coul_clear_sr_time_array(struct coul_core_device *chip)
{
	memset(chip->common_data.sr_sleep_time, 0, sizeof(chip->common_data.sr_sleep_time));
	memset(chip->common_data.sr_wakeup_time, 0, sizeof(chip->common_data.sr_wakeup_time));
	chip->common_data.sr_sleep_index = 0;
	chip->common_data.sr_wakeup_index = 0;
}

static int calc_wakeup_avg_current(struct coul_core_device *chip,
	int last_cc, int last_cc_time, int curr_cc, int curr_time)
{
	static int first_in = 1;
	int iavg_ma, delta_cc, delta_time;

	if ((!chip) || (!chip->core_ic_ops) ||
		(!IS_IC_OPS_NULL(chip, get_batt_cur))) {
		coul_err("chip is null\n");
		return 0;
	}

	if (first_in) {
		iavg_ma = chip->core_ic_ops->get_batt_cur() / UA_PER_MA;
		first_in = 0;
	} else {
		delta_cc = curr_cc - last_cc;
		delta_time = curr_time - last_cc_time;
		if (delta_time > 0)
			iavg_ma = (div_s64((s64)delta_cc * SEC_PER_HOUR,
				delta_time)) / UA_PER_MA;
		else
			iavg_ma = chip->core_ic_ops->get_batt_cur() /
				UA_PER_MA;

		coul_info("wake_up delta_time=%ds, iavg_ma=%d\n",
			delta_time, iavg_ma);
	}

	if (abs(iavg_ma) >= WAKEUP_AVG_CUR_500MA) {
		chip->common_data.curr2update_ocv[AVG_CURR_500MA].current_ma = abs(iavg_ma);
		chip->common_data.curr2update_ocv[AVG_CURR_500MA].time = curr_time;
	} else if (abs(iavg_ma) >= WAKEUP_AVG_CUR_250MA) {
		chip->common_data.curr2update_ocv[AVG_CURR_250MA].current_ma = abs(iavg_ma);
		chip->common_data.curr2update_ocv[AVG_CURR_250MA].time = curr_time;
	}
	return iavg_ma;
}

void record_wakeup_info(struct coul_core_device *chip, int wakeup_time)
{
	int wakeup_avg_current_ma = 0;

	if (!chip)
		return;

	if (wakeup_time > SR_MAX_RESUME_TIME) {
		coul_clear_sr_time_array(chip);
		coul_info("[SR]: wakeup_time %d > SR_MAX_RESUME_TIME %d\n",
			wakeup_time, SR_MAX_RESUME_TIME);
		wakeup_avg_current_ma = calc_wakeup_avg_current(
			chip, chip->resume_cc, chip->common_data.sr_resume_time,
			chip->common_data.suspend_cc, chip->common_data.sr_suspend_time);
	} else if (wakeup_time >= 0 &&
		(chip->common_data.sr_wakeup_index < SR_ARR_LEN)) {
		chip->common_data.sr_wakeup_time[chip->common_data.sr_wakeup_index] = wakeup_time;
		chip->common_data.sr_wakeup_index++;
		chip->common_data.sr_wakeup_index = chip->common_data.sr_wakeup_index % SR_ARR_LEN;
		wakeup_avg_current_ma = calc_wakeup_avg_current(
			chip, chip->resume_cc, chip->common_data.sr_resume_time,
			chip->common_data.suspend_cc, chip->common_data.sr_suspend_time);
	} else {
		coul_err("[SR]: wakeup_time=%d, sr_suspend_time=%lld, sr_resume_time=%lld\n",
			wakeup_time, chip->common_data.sr_suspend_time,
			chip->common_data.sr_resume_time);
	}
	coul_info("SUSPEND! cc=%d, time=%lld\n",
		chip->common_data.suspend_cc, chip->common_data.sr_suspend_time);
}

/* suspend function, called when coul enter sleep */
int coul_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct coul_core_device *chip = platform_get_drvdata(pdev);

	int current_sec, wakeup_time;

	if (!chip) {
		coul_err("chip is null\n");
		return 0;
	}

	coul_info(" +\n");

	if ((!IS_IC_OPS_NULL(chip, read_coul_time)) &&
		(!IS_IC_OPS_NULL(chip, get_cc))) {
		current_sec = (int)chip->core_ic_ops->read_coul_time();
		chip_lock();
		chip->common_data.suspend_cc = chip->core_ic_ops->get_cc();
		chip->common_data.sr_suspend_time = current_sec;

		wakeup_time = current_sec - chip->common_data.sr_resume_time;
		record_wakeup_info(chip, wakeup_time);
		chip_unlock();
	}

	if (chip->work_init_flag == 1)
		cancel_delayed_work_sync(&chip->calc_soc_work);

	if (!IS_IC_OPS_NULL(chip, cali_adc))
		chip->core_ic_ops->cali_adc();

	coul_set_low_vol_state(chip, LOW_INT_STATE_SLEEP);

	if (!IS_IC_OPS_NULL(chip, lp_enable))
		chip->core_ic_ops->lp_enable();
	coul_info(" -\n");

	return 0;
}

void record_sleep_info(struct coul_core_device *chip, int sr_sleep_time)
{
	if (sr_sleep_time >= 0 && (chip->common_data.sr_sleep_index < SR_ARR_LEN)) {
		chip->common_data.sr_sleep_time[chip->common_data.sr_sleep_index] = sr_sleep_time;
		chip->common_data.sr_sleep_index++;
		chip->common_data.sr_sleep_index = chip->common_data.sr_sleep_index % SR_ARR_LEN;
	} else {
		coul_info("[SR]: sr_sleep_time = %d\n", sr_sleep_time);
	}
}

static void queue_work_after_coul_resume(struct coul_core_device *chip)
{
	if (chip->work_init_flag == 1) {
		queue_delayed_work(system_power_efficient_wq,
			&chip->calc_soc_work,
			round_jiffies_relative(msecs_to_jiffies(
				(unsigned int)chip->common_data.soc_work_interval)));
	}
}

/* called when coul wakeup from deep sleep */
int coul_resume(struct platform_device *pdev)
{
	struct coul_core_device *chip = platform_get_drvdata(pdev);
	int sr_sleep_time;

	if (!chip) {
		coul_err("chip is null\n");
		return 0;
	}
	coul_info(" +\n");
	chip_lock();

	if (!IS_IC_OPS_NULL(chip, read_coul_time)) {
		chip->common_data.sr_resume_time = (int)chip->core_ic_ops->read_coul_time();
		sr_sleep_time =
			chip->common_data.sr_resume_time - chip->common_data.sr_suspend_time;
		coul_set_low_vol_state(chip, LOW_INT_STATE_RUNNING);
		record_sleep_info(chip, sr_sleep_time);
	}

	if (!IS_IC_OPS_NULL(chip, get_cc))
		chip->resume_cc = chip->core_ic_ops->get_cc();

	chip_unlock();

	chip->common_data.event_type = XR_COUL_K2U_EVENT_RESUME;
	chip->kdata_ready = true;
	wake_up_interruptible(&chip->coul_wait_q);
	queue_work_after_coul_resume(chip);
	coul_info(" -\n");

	return 0;
}
#endif

static const struct of_device_id coul_core_match_table[] = {
	{
		.compatible = "xring,coul_core",
	},
	{
		/* end */
	},
};

static struct platform_driver coul_core_driver = {
	.probe = coul_probe,
	.remove = coul_remove,
	.shutdown = coul_shutdown,
#ifdef CONFIG_PM
	.suspend = coul_suspend,
	.resume = coul_resume,
#endif
	.driver = {
		.name = "xr_coul_core",
		.owner = THIS_MODULE,
		.of_match_table = coul_core_match_table,
	},
};

int coul_core_init(void)
{
	return platform_driver_register(&coul_core_driver);
}

void coul_core_exit(void)
{
	platform_driver_unregister(&coul_core_driver);
}

MODULE_AUTHOR("XRing Technologies Co., Ltd");
MODULE_DESCRIPTION("xring coul core driver");
MODULE_LICENSE("GPL");
