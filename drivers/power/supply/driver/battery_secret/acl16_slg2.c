/*
 *  w1_slg.c - Aisinochip SLG driver
 *
 * Copyright (c) Shanghai Aisinochip Electronics Techology Co., Ltd.
 *
 */
//#define pr_fmt(fmt) "[slg2 %s,%d] " fmt "\n", __func__, __LINE__

#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/crc16.h>
#include <linux/of_gpio.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include "../../middleware/battery_secret_manager/inc/battery_secret_class.h"

#undef pr_err
#undef pr_debug
#undef pr_info
#define MODULE_TAG "slg2_drv"
#include "../../middleware/lc_logfs_class/inc/lc_logfs_class.h"

#define CRC16_INIT                  0
#define BATT_INFO_PAGE              0
#define FIRST_USE_PAGE              1
#define CYCLE_SOH_PAGE              2
#define READ_PAGE_MAX               2
#define WRITE_PAGE_MAX              10
#define GENERAL_WAIT_MAX            10
#define DC_INIT_VALUE				0x1FFFF

#define W1_SKIP_ROM		            0xCC
#define SLG2_MEMORY_WRITE           0xDA
#define SLG2_MEMORY_READ            0xCA
#define SLG2_DC_DECREASE            0x24
#define SLG2_DC_GET                 0xC0

#define W1_RESPONSE_SUCCESS         0xAA
#define W1_BUS_ERROR                -201
#define SLG2_ERR_VALUE              -202
#define W1_CMD_ERROR                -203
#define W1_RECV_CRC_ERROR           -204
#define W1_CMD_OK                   0

struct slg2_data {
	int cycle_count_curr;
	int dc_value_curr;
	char *uisoh_curr;
	int rawsoh_curr;
	int is_auth;
	struct platform_device *pdev;
	struct device *dev;
	const char *role;
	const char *chip_name;
	raw_spinlock_t io_lock;
	struct gpio_desc *gpiod;
	struct gpio_desc *enable_gpiod;
	struct secret_device *secret_dev;
	uint8_t batt_id;
} *g_info;

//static int page0_valid;
static int page2_valid;
//static uint8_t page0_buf[32];
static uint8_t page2_buf[32];
static u32 cmd_len = 0;
static u8 cmd_buffer[256];

static void hex2ascii(u8 *output, u8 *input, u32 len)
{
    u32 i;

    for (i = 0; i < len; i++) {
        output[i * 2] = ((input[i] & 0xF0) >> 4) + 0x30;
        output[i * 2 + 1] = (input[i] & 0x0F) + 0x30;
        if (output[i * 2] >= 0x3A)
            output[i * 2] += 7;
        if (output[i * 2 + 1] >= 0x3A)
            output[i * 2 + 1] += 7;
    }
}

static void w1_hex_dump(const char *prefix, u8 *data_buf, int length)
{
    char buffer[128];

    if (!prefix) {
        pr_err("prefix null");
        return;
    }

    if ((strlen(prefix) + length * 2) > sizeof(buffer))
        pr_err("length too large");
    else {
        if (length == 0 || data_buf == NULL)
            pr_info("%s\n", prefix);
        else {
            memset(buffer, 0x00, sizeof(buffer));
            hex2ascii(buffer, data_buf, length);
            pr_info("%s%s", prefix, buffer);
        }
    }
}

static void w1_ndelay(u32 ns)
{
	uint64_t pre, now;

	pre = ktime_get_ns();
	while (1) {
		now = ktime_get_ns();
		if (now - pre >= ns)
			break;
	}
}

static void w1_udelay(u32 us)
{
	w1_ndelay(us * 1000);
}

static void w1_mdelay(int ms)
{
	mdelay(ms);
}

static void w1_write_bit(u8 bit)
{
	gpiod_direction_output(g_info->gpiod, 1);
	if (bit) {
		gpiod_set_value(g_info->gpiod, 0);
		w1_ndelay(1000);
		gpiod_set_value(g_info->gpiod, 1);
		w1_udelay(16);
	} else {
		gpiod_set_value(g_info->gpiod, 0);
		w1_udelay(8);
		gpiod_set_value(g_info->gpiod, 1);
		w1_udelay(8);
	}
}

static u8 w1_read_bit(void)
{
	int result;

	gpiod_direction_output(g_info->gpiod, 1);
	gpiod_set_value(g_info->gpiod, 0);
	w1_ndelay(1000);
	gpiod_direction_input(g_info->gpiod);
	w1_ndelay(200);
	result = gpiod_get_value(g_info->gpiod);
	w1_udelay(16);

	return result;
}

static int w1_reset_bus(void)
{
	int level;
	unsigned long flags = 0;

	gpiod_direction_output(g_info->gpiod, 1);
	raw_spin_lock_irqsave(&g_info->io_lock, flags);
	gpiod_set_value(g_info->gpiod, 0);
	w1_udelay(64);
	gpiod_direction_input(g_info->gpiod);
	w1_udelay(25);
	level = gpiod_get_value(g_info->gpiod);
	raw_spin_unlock_irqrestore(&g_info->io_lock, flags);
	w1_mdelay(1);

	return level;
}

static void w1_bus_recovery(void)
{
	gpiod_direction_output(g_info->gpiod, 0);
	w1_mdelay(20);
	gpiod_direction_output(g_info->gpiod, 1);
	w1_mdelay(50);
}

static void w1_write_byte(u8 byte)
{
	int i;
	unsigned long flags = 0;

	raw_spin_lock_irqsave(&g_info->io_lock, flags);
	for (i = 0; i < 8; ++i)
		w1_write_bit((byte >> i) & 0x1);
	raw_spin_unlock_irqrestore(&g_info->io_lock, flags);
}

static void w1_write_block(const u8 *buf, int len)
{
	int i;

	for (i = 0; i < len; ++i)
		w1_write_byte(buf[i]);
}

static u8 w1_read_byte(void)
{
	int i;
	u8 res = 0;
	unsigned long flags = 0;

	raw_spin_lock_irqsave(&g_info->io_lock, flags);
	for (i = 0; i < 8; ++i)
		res |= w1_read_bit() << i;
	raw_spin_unlock_irqrestore(&g_info->io_lock, flags);

	return res;
}

static void w1_read_block(u8 *buf, int len)
{
	int i;

	for (i = 0; i < len; i++)
		buf[i] = w1_read_byte();
}

static int slg2_bus_send_recv(
	u8 *input, int input_len,
	int delay_ms, u8 *output, int output_len)
{
	int index;
	unsigned short calc_crc;
	unsigned short recv_crc;
	u8 input_buf[64];
	u8 recv_buf[64];

	memcpy(input_buf, input, input_len);
	index = input_len;
	calc_crc = crc16(CRC16_INIT, input_buf, index);
	calc_crc ^= 0xFFFF;
	input_buf[index++] = calc_crc >> 8;
	input_buf[index++] = calc_crc & 0xFF;
	if (w1_reset_bus()) {
		pr_err("reset bus failed");
		return W1_BUS_ERROR;
	}
	// skip ROM
	w1_write_byte(W1_SKIP_ROM);
	w1_hex_dump("slg2 <- CC", NULL, 0);
	w1_mdelay(1);
	w1_write_block(input_buf, index);
	w1_hex_dump("slg2 <- ", input_buf, index);
	w1_mdelay(delay_ms);
	w1_read_block(recv_buf, output_len + 4);
	w1_hex_dump("slg2 -> ", recv_buf, output_len + 4);
	if ((output_len + 1) != recv_buf[0])
		pr_err("invalid length:%u", recv_buf[0]);
	calc_crc = crc16(CRC16_INIT, recv_buf, output_len + 2);
	calc_crc ^= 0xFFFF;
	recv_crc = (recv_buf[output_len + 2] << 8) + recv_buf[output_len + 3];
	if (recv_crc == calc_crc) {
		if (recv_buf[1] != W1_RESPONSE_SUCCESS) {
			pr_err("invalid status:%02X", recv_buf[1]);
			w1_bus_recovery();
			return W1_CMD_ERROR;
		} else {
			memcpy(output, &recv_buf[2], output_len);
			memcpy(cmd_buffer, &recv_buf[0], output_len);
			w1_mdelay(10);
			return W1_CMD_OK;
		}
	} else {
		pr_err("crc error");
		w1_bus_recovery();
		return W1_RECV_CRC_ERROR;
	}
}

// slg <- CC
// slg <- CA010051DE
// slg -> 21AA534C424E355334323138303032333730574D443141303030303030303030303088CE
static int slg2_memory_read(int index, u8 *read_buf)
{
    u8 send[8];

	/*if (index == BATT_INFO_PAGE && page0_valid) {
		memcpy(read_buf, page0_buf, 32);
		pr_err("use last page0 buffer data");
		return 0;
	}*/
	if (index == CYCLE_SOH_PAGE && page2_valid) {
		memcpy(read_buf, page2_buf, 32);
		pr_err("use last page2 buffer data");
		return 0;
	}

	send[0] = SLG2_MEMORY_READ;
	send[1] = 1;
	send[2] = index;
	if (!slg2_bus_send_recv(send, 3, READ_PAGE_MAX, read_buf, 32)) {
		/*if (index == BATT_INFO_PAGE) {
			/age0_valid = 1;
			memcpy(page0_buf, read_buf, 32);
		}*/
		if (index == CYCLE_SOH_PAGE) {
			page2_valid = 1;
			memcpy(page2_buf, read_buf, 32);
		}
		return 0;
	} else {
		pr_err("failed, page index:%d", index);
    	return -EINVAL;
	}
}

// slg <- CC
// slg <- DA2102FFEEDDCCBBAA99887766554433221100FFEEDDCCBBAA99887766554433221100DB78
// slg -> 01AA107E
static int slg2_memory_write(int index, const u8 *write_data)
{
    u8 send[64], recv[8];

	send[0] = SLG2_MEMORY_WRITE;
	send[1] = 0x21;
	send[2] = index;
	memcpy(&send[3], write_data, 32);
	if (!slg2_bus_send_recv(send, 35, WRITE_PAGE_MAX, recv, 0)) {
		if (index == CYCLE_SOH_PAGE) {
			page2_valid = 1;
			memcpy(page2_buf, write_data, 32);
		}
		return 0;
	} else {
		if (index == CYCLE_SOH_PAGE)
			page2_valid = 0;
		pr_err("failed, page index:%d", index);
    	return -EINVAL;
	}

}

// slg <- CC
// slg <- C000FFAF
// slg -> 05AA0001F907E0B5
static int slg2_dc_get(u32 *dc_cnt)
{
	u8 send[8], recv[8];

	if (g_info->dc_value_curr != SLG2_ERR_VALUE) {
		*dc_cnt = g_info->dc_value_curr;
		pr_err("use last dc value:%d", g_info->dc_value_curr);
		return 0;
	}
	send[0] = SLG2_DC_GET;
	send[1] = 0x00;
	if (!slg2_bus_send_recv(send, 2, GENERAL_WAIT_MAX, recv, 4)) {
		g_info->dc_value_curr = (recv[0] << 24) + (recv[1] << 16) + (recv[2] << 8) + (recv[3]);
		*dc_cnt = g_info->dc_value_curr;
		pr_err("new dc value:%d", g_info->dc_value_curr);
		return 0;
	} else {
		pr_err("failed");
		g_info->dc_value_curr = SLG2_ERR_VALUE;
    	return -EINVAL;
	}
}

// slg <- CC
// slg <- 2400FFE4
// slg -> 01AA107E
static int slg2_dc_decrease(void)
{
	u8 send[8], recv[8];

	send[0] = SLG2_DC_DECREASE;
	send[1] = 0x00;
	memset(recv, 0xFF, sizeof(recv));
	if (!slg2_bus_send_recv(send, 2, GENERAL_WAIT_MAX, recv, 0)) {
		g_info->dc_value_curr--;
		return 0;
	} else {
		pr_err("failed");
		g_info->dc_value_curr = SLG2_ERR_VALUE;
    	return -EINVAL;
	}
}

int slg2_get_cycle_count(struct secret_device *secret_dev, int *cycle_count)
{
	u32 dc_val;
	int get_count;
	u8 page_buf[32];
	struct slg2_data *info = dev_get_drvdata(&secret_dev->dev);

	pr_err("enter");
	if (!info){
		pr_err("get slg2_data fail");
		return -EINVAL;
	}
	if (slg2_memory_read(CYCLE_SOH_PAGE, page_buf)) {
		pr_err("read page2 failed");
		return -EINVAL;
	}
	get_count = page_buf[30];
	if (get_count == 0xFF)
		get_count = 0;
	pr_err("page cycle count:%d", get_count);
	if (get_count >= 29) {
		if (slg2_dc_get(&dc_val)) {
			pr_err("failed read dc");
			return -EINVAL;
		}
		*cycle_count = DC_INIT_VALUE - dc_val;
		if (*cycle_count < get_count)
			*cycle_count = get_count;
	} else
		*cycle_count = get_count;

	info->cycle_count_curr = *cycle_count;
	pr_err("get_cycle_count:0x%x\n", *cycle_count);
	pr_err("exit");

	return 0;
}

int slg2_set_cycle_count(struct secret_device *secret_dev, int set_cycle_count)
{
	int cnt;
	u32 dc_val;
	u8 page_buf[32];
	struct slg2_data *info = dev_get_drvdata(&secret_dev->dev);

	if (!info) {
		pr_err("get slg2_data fail");
		return -EINVAL;
	}
	cnt = set_cycle_count - info->cycle_count_curr;
	pr_err("add cycle count:%d", cnt);
/*
	if (cnt <= 0) {
		pr_err("invalid cycle count:%d, current cycle count:%d",
			set_cycle_count, info->cycle_count_curr);
		return 0;
	}
*/
	if (slg2_memory_read(CYCLE_SOH_PAGE, page_buf)) {
		pr_err("read page2 failed");
		return -EINVAL;
	}
	pr_err("set_cycle_count:%d", set_cycle_count);
	if (set_cycle_count < 30) {
		page_buf[30] = set_cycle_count;
		if (slg2_memory_write(CYCLE_SOH_PAGE, page_buf)) {
			pr_err("write cycle count:%d failed", set_cycle_count);
			return -EINVAL;
		}
	} else if (set_cycle_count >= 30 && info->cycle_count_curr <= 29) {
		page_buf[30] = 29;
		if (slg2_memory_write(CYCLE_SOH_PAGE, page_buf)) {
			pr_err("write cycle count:29 failed");
			return -EINVAL;
		}
		if (slg2_dc_get(&dc_val)) {
			pr_err("read dc value failed");
			return -EINVAL;
		}
		cnt = set_cycle_count - (DC_INIT_VALUE - dc_val);
		if (cnt <= 0) {
			pr_err("invalid dc:%d", dc_val);
			return 0;
		}
		while (cnt) {
			if (slg2_dc_get(&dc_val)) {
				pr_err("read last dc value failed");
				return -EINVAL;
			}
			if (DC_INIT_VALUE - dc_val >= set_cycle_count) {
				pr_err("dc decrease to cycle count:%d succeeded", set_cycle_count);
				return 0;
			}
			if (slg2_dc_decrease()) {
				pr_err("dc decrease faile, cnt:%d", cnt);
				return -EINVAL;
			}
			cnt--;
		}
	} else {
		while (cnt) {
			if (slg2_dc_get(&dc_val)) {
				pr_err("read last dc value failed");
				return -EINVAL;
			}
			if (DC_INIT_VALUE - dc_val >= set_cycle_count) {
				pr_err("dc decrease to cycle count:%d succeeded", set_cycle_count);
				return 0;
			}
			if (slg2_dc_decrease()) {
				pr_err("dc decrease faile, cnt:%d", cnt);
				return -EINVAL;
			}
			cnt--;
		}
	}

	info->cycle_count_curr = set_cycle_count;
	return 0;
}

static int slg2_get_ui_soh(struct secret_device *secret_dev, u8 *ui_soh_data, int len)
{
	int i = 0, invalid_count = 0;
	u8 page_buf[32];
	struct slg2_data *info = dev_get_drvdata(&secret_dev->dev);

	if (!info) {
		pr_err("get slg2_data failed");
		return -EINVAL;
	}

	if (slg2_memory_read(CYCLE_SOH_PAGE, page_buf)) {
		pr_err("read ui soh paged failed");
		return -EINVAL;
	}
	memcpy(ui_soh_data, page_buf, len);
	for (i = 0; i < len; i++) {
		if (ui_soh_data[i] == 0xff)
			invalid_count++;
	}
	if (invalid_count >= 5) {
		pr_err("%s invalid value, set 0\n", __func__);
		memset(ui_soh_data, 0, len);
	}

	w1_hex_dump("ui soh:", ui_soh_data, len);

	return 0;
}

static int slg2_set_ui_soh(struct secret_device *secret_dev, u8 *ui_soh_data, int len, int raw_soh)
{
	u8 page_buf[32];
	struct slg2_data *info = dev_get_drvdata(&secret_dev->dev);

	if (!info) {
		pr_err("get slg2_data failed");
		return -EINVAL;
	}
	if (slg2_memory_read(CYCLE_SOH_PAGE, page_buf)) {
		pr_err("read ui soh page failed");
		return -EINVAL;
	}

	memcpy(page_buf, ui_soh_data, len);
	page_buf[15] = raw_soh;
	if (slg2_memory_write(CYCLE_SOH_PAGE, page_buf)) {
		pr_err("update ui soh data failed");
		return -EINVAL;
	} else {
		w1_hex_dump("new ui soh:", ui_soh_data, len);
		return 0;
	}
}

static int slg2_get_raw_soh(struct secret_device *secret_dev, int *val)
{
	u8 page_buf[32];
	struct slg2_data *info = dev_get_drvdata(&secret_dev->dev);

	if (!info) {
		pr_err("get slg2_data failed");
		return -EINVAL;
	}

	if (slg2_memory_read(CYCLE_SOH_PAGE, page_buf)) {
		pr_err("read raw soh page failed");
		return -EINVAL;
	}
	*val = page_buf[15];
	if (*val > 100) {
		*val = 100;
		page_buf[15] = *val;

		if (slg2_memory_write(CYCLE_SOH_PAGE, page_buf)) {
			pr_err("set raw_soh 100 failed");
			return -EINVAL;
		}
	}
	pr_err("raw soh:%d", page_buf[15]);

	return 0;
}

static int slg2_set_raw_soh(struct secret_device *secret_dev, int raw_soh)
{
	u8 page_buf[32];
	struct slg2_data *info = dev_get_drvdata(&secret_dev->dev);

	if (!info) {
		pr_err("get slg2_data fail");
		return -EINVAL;
	}

	if (slg2_memory_read(CYCLE_SOH_PAGE, page_buf)) {
		pr_err("read raw soh failed");
		return -EINVAL;
	}

	page_buf[15] = raw_soh;
	if (slg2_memory_write(CYCLE_SOH_PAGE, page_buf)) {
		pr_err("update raw soh failed");
		return -EINVAL;
	}
	pr_err("new raw soh:%d", page_buf[15]);

	return 0;
}

static int slg2_get_batt_manufacture_date(struct secret_device *secret_dev, char * const buff, size_t size)
{
	const char *batt_sn_cmdline;
	char batt_info[33], batt_date[9];
	struct slg2_data *info = dev_get_drvdata(&secret_dev->dev);

	if (!info) {
		pr_err("get slg2_data failed");
		return -EINVAL;
	}
	batt_sn_cmdline = find_secret_attr_by_name("batt_sn");
	if (batt_sn_cmdline) {
		strcpy(batt_info, batt_sn_cmdline);
		batt_date[0] = '2'; batt_date[1] = '0';
		batt_date[2] = '2'; batt_date[3] = batt_info[6];
		// month
		if (batt_info[7] <= '9') {
			batt_date[4] = '0';
			batt_date[5] = batt_info[7];
		} else {
			batt_date[4] = '1';
			batt_date[5] = '0' + batt_info[7] - 'A';
		}
		// day
		batt_date[6] = batt_info[8];
		batt_date[7] = batt_info[9];
		strcpy(buff, batt_date);
		pr_info("battery manufacture date:%s\n", batt_date);

		return 0;
	} else {
			pr_err("get batt_sn_cmdline failed");
			return -EINVAL;
	}
}

int slg2_get_battery_id(struct secret_device *secret_dev, int *batt_id)
{
	int ret, val;
	const char *batt_id_cmdline;
	struct slg2_data *info = dev_get_drvdata(&secret_dev->dev);

	if (!info) {
		pr_err("get slg2_data fail");
		return -EINVAL;
	}
	batt_id_cmdline = find_secret_attr_by_name("batt_id");
	if (batt_id_cmdline) {
		ret = kstrtoint(batt_id_cmdline, 10, &val);
		if (ret < 0) {
			pr_err("parse cycle count failed");
			return -EINVAL;
		} else {
			info->batt_id = val;
			*batt_id = val;
			return 0;
		}
	}
	return -EINVAL;
}

int slg2_is_battery_auth(struct secret_device *secret_dev, int *is_auth)
{
	int ret, val;
	const char *is_auth_cmdline;
	struct slg2_data *info = dev_get_drvdata(&secret_dev->dev);

	if (!info) {
		pr_err("get slg2_data fail");
		return -EINVAL;
	}
	is_auth_cmdline = find_secret_attr_by_name("is_auth");
	if (is_auth_cmdline) {
		ret = kstrtoint(is_auth_cmdline, 10, &val);
		if (ret < 0) {
			pr_err("parse cycle count failed");
			return -EINVAL;
		} else {
			info->is_auth = val;
			*is_auth = val;
			return 0;
		}
	}
	return -EINVAL;
}

int slg2_get_battery_sn(struct secret_device *secret_dev, char * const buff, size_t size)
{
	const char *batt_sn_cmdline;
	struct slg2_data *info = dev_get_drvdata(&secret_dev->dev);

	if (!info) {
		pr_err("get slg2_data failed");
		return -EINVAL;
	}
	batt_sn_cmdline = find_secret_attr_by_name("batt_sn");
	if (batt_sn_cmdline) {
		strcpy(buff, batt_sn_cmdline);
		pr_info("battery sn:%s\n", buff);

		return 0;
	} else {
			pr_err("get batt_sn_cmdline failed");
			return -EINVAL;
	}
}

int slg2_get_battery_first_use_time(struct secret_device *secret_dev, char * const buff, size_t size)
{
	int i = 0, invalid_count = 0;
	uint8_t page_data[32];
	struct slg2_data *info = dev_get_drvdata(&secret_dev->dev);

	if (!info) {
		pr_err("get slg2_data fail");
		return -EINVAL;
	}

	if (slg2_memory_read(FIRST_USE_PAGE, page_data)) {
		pr_err("read first_use_time failed");
		return -EINVAL;
	}

	memcpy(buff, page_data, 6);
	for (i = 0; i < 6; i++) {
		if (buff[i] == 0xff)
			invalid_count++;
	}

	if (invalid_count >= 5) {
		pr_err("invalid value, set 0\n");
		memset(buff, '0', size);
	}

	w1_hex_dump("get first_use_time:", buff, 6);
	return 0;
}

int slg2_set_battery_first_use_time(struct secret_device *secret_dev, const char *time)
{
	int i = 0;
	uint8_t page_data[32];
	struct slg2_data *info = dev_get_drvdata(&secret_dev->dev);

	if (!info) {
		pr_err("get slg2_data fail");
		return -EINVAL;
	}
	if (slg2_memory_read(FIRST_USE_PAGE, page_data)) {
		pr_err("read first_use_time failed");
		return -EINVAL;
	}

	for (i = 0; i < 6; i++) {
		page_data[i] = time[i];
	}

	if (slg2_memory_write(FIRST_USE_PAGE, page_data)) {
		pr_err("update first_use_time failed");
		return -EINVAL;
	}

	w1_hex_dump("set first_use_time:", page_data, 6);
	return 0;
}

int slg2_clear_cycle_count(struct secret_device *secret_dev)
{
	uint8_t flag_data[32];
	uint8_t cycle_data[32];
	struct slg2_data *info = dev_get_drvdata(&secret_dev->dev);

	if (!info) {
		pr_err("get slg2_data fail");
		return -EINVAL;
	}
	// read page1 data[7]
	if (slg2_memory_read(FIRST_USE_PAGE, flag_data)) {
		pr_err("read clear_flag failed");
		return -EINVAL;
	}

	//compare
	if (flag_data[7] == 0x1) {
		pr_err("cycle_data is already clear");
		return -EINVAL;
	}
	//read cyc
	if (slg2_memory_read(CYCLE_SOH_PAGE, cycle_data)) {
		pr_err("read cycle_data failed");
		return -EINVAL;
	}
	if (cycle_data[30] > 10) {
		pr_err("cycle_data over 10");
		return -EINVAL;
	}
	//write 0xFF -> cyc
	cycle_data[30] = 0xFF;
	if (slg2_memory_write(CYCLE_SOH_PAGE, cycle_data)) {
		pr_err("cycle_data clear failed");
		return -EINVAL;
	}
	//write 0x1 -> page1 data[7]
	flag_data[7] = 0x1;
	if (slg2_memory_write(FIRST_USE_PAGE, flag_data)) {
		pr_err("update flag_data failed");
		return -EINVAL;
	}
	page2_valid = 0;
	pr_info("clear cycle succ");
	return 0;
}

static int onewire_gpio_init( struct slg2_data *info)
{
	struct gpio_desc *gpiod;
	enum gpiod_flags  gflags = GPIOD_OUT_LOW;

	gpiod = devm_gpiod_get(info->dev, "onewire", gflags);
	if (IS_ERR(gpiod)) {
		pr_err("gpio_request (pin) failed");
		return PTR_ERR(gpiod);
	}
	info->gpiod = gpiod;
	gpiod_direction_output(gpiod, 1);
	pr_err("succeeded");

	return 0;
}

static int enable_gpio_init( struct slg2_data *info)
{
	struct gpio_desc *gpiod;
	enum gpiod_flags gflags = GPIOD_OUT_HIGH;

	gpiod = devm_gpiod_get(info->dev, "enable", gflags);
	if (IS_ERR(gpiod)) {
		pr_err("pio_request (pin) failed");
		return 0;
	}
	info->enable_gpiod = gpiod;
	gpiod_direction_output(gpiod, 1);
	pr_err("succeeded");

	return 0;
}

void cmdline_attrs_init(struct slg2_data *info)
{
	int ret, val;
	const char *cycle_count_cmdline;
	const char *dc_value_cmdline;
	const char *raw_soh_cmdline;
	const char *ui_soh_cmdline;

	info->dc_value_curr = SLG2_ERR_VALUE;
	info->cycle_count_curr = SLG2_ERR_VALUE;
	info->rawsoh_curr = SLG2_ERR_VALUE;
	info->uisoh_curr = NULL;

	cycle_count_cmdline = find_secret_attr_by_name("cc");
	dc_value_cmdline = find_secret_attr_by_name("dc");
	raw_soh_cmdline = find_secret_attr_by_name("rsoh");
	ui_soh_cmdline = find_secret_attr_by_name("uisoh");
	info->uisoh_curr = kzalloc((strlen(ui_soh_cmdline) + 1), GFP_KERNEL);
	if (cycle_count_cmdline) {
		ret = kstrtoint(cycle_count_cmdline, 10, &val);
		if (ret < 0) {
			pr_err("parse cycle count failed");
		} else {
			pr_err("parse cycle count succ,cycle_count=%d", val);
			info->cycle_count_curr = val;
		}
	}
	if (dc_value_cmdline) {
		ret = kstrtoint(dc_value_cmdline, 10, &val);
		if (ret < 0) {
			pr_err("parse dc failed");
		} else {
			pr_err("parse dc succ,dc=%d", val);
			info->dc_value_curr = val;
		}
	}
	if (raw_soh_cmdline) {
		ret = kstrtoint(raw_soh_cmdline, 10, &val);
		if (ret < 0) {
			pr_err("parse raw soh failed");
		} else {
			pr_err("parse raw soh succ,raw_soh=%d", val);
			info->rawsoh_curr = val;
		}
	}
	if (!ui_soh_cmdline) {
		pr_err("parse ui soh failed");
	} else {
		pr_err("parse ui soh succ,ui_soh=%s", ui_soh_cmdline);
		strcpy(info->uisoh_curr, ui_soh_cmdline);
	}
}

struct secret_ops slg2_secret_ops = {
	.set_cycle_count = slg2_set_cycle_count,
	.get_cycle_count = slg2_get_cycle_count,
	.get_uisoh = slg2_get_ui_soh,
	.set_uisoh = slg2_set_ui_soh,
	.get_rawsoh = slg2_get_raw_soh,
	.set_rawsoh = slg2_set_raw_soh,
	.get_battery_manufacture_date = slg2_get_batt_manufacture_date,
	.get_battery_first_use_time = slg2_get_battery_first_use_time,
	.set_battery_first_use_time = slg2_set_battery_first_use_time,
	.get_battery_id = slg2_get_battery_id,
	.is_battery_auth = slg2_is_battery_auth,
	.get_battery_sn = slg2_get_battery_sn,
	.clear_cycle_count = slg2_clear_cycle_count,
};

// cat /sys/bus/platform/devices/odm/odm:acl_slg/cmdbuf
static ssize_t slg2_cmdbuf_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int i, len = 0;

	for (i = 0; i < cmd_len; i++)
		len += snprintf(buf + len, PAGE_SIZE, "%02X ", cmd_buffer[i]);
	len += snprintf(buf + len, PAGE_SIZE, "\n");

	return len;
}

// echo "20 32 CA 01 00" > /sys/bus/platform/devices/odm/odm:acl_slg/cmdbuf
// 20(H): read data length
// 32(H): delay(ms) before read from secret ic
// 44(H): command byte
// 00(H): command payload
static ssize_t slg2_cmdbuf_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int     i;
	ssize_t len = 0;
	ulong   tx_hex[64];
	u8      tx_hex_u8[64];
	u32     num = 0, wait_time;
	char    *tx_ptr, *hexstr, txbuf[128];

	cmd_len = 0;
	tx_ptr = txbuf;
	sscanf(buf, "%02X", &cmd_len);
	sscanf(&buf[3], "%02X", &wait_time);
	memset(txbuf, 0x00, sizeof(txbuf));
	strcpy(txbuf, &buf[6]);
	memset(cmd_buffer, 0x00, sizeof(cmd_buffer));
	while ((hexstr = strsep(&tx_ptr, " ")) != NULL) {
		if (kstrtoul(hexstr, 16, &tx_hex[num]) < 0) {
			cmd_len = 0;
			pr_err("parse byte(%u) failed", num);
			return -EINVAL;
		}
		num++;
	}

	memset(txbuf, 0x00, sizeof(txbuf));
	len += snprintf(txbuf, sizeof(txbuf), "tx data:");
	for (i = 0; i < num; i++)
		len += snprintf(txbuf + len, PAGE_SIZE, " %02X", (u32)tx_hex[i]);
	pr_info("%s", txbuf);

	for (i = 0; i < num; i++)
		tx_hex_u8[i] = tx_hex[i];

	if (slg2_bus_send_recv(tx_hex_u8, num, wait_time, cmd_buffer, cmd_len)) {
		cmd_len = 0;
		pr_err("write cmdbuf failed");
		return -EINVAL;
	}
	cmd_len += 4;
	page2_valid = 0;

	return size;
}

static struct device_attribute dev_attr_slg2_cmdbuf =
	__ATTR(slg2_cmdbuf, S_IRUSR | S_IWUSR, slg2_cmdbuf_show, slg2_cmdbuf_store);

static ssize_t slg2_clear_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int clear_date = 0;
	u8 flag_data[32];

	if (slg2_memory_read(FIRST_USE_PAGE, flag_data)) {
		pr_err("read clear_flag failed");
		return -EINVAL;
	}

	clear_date = flag_data[7];
	return sprintf(buf, "%d\n", clear_date);
}

static ssize_t slg2_clear_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int ret, val;
	u8 flag_data[32];

	ret = sscanf(buf, "%x", &val);
	if (slg2_memory_read(FIRST_USE_PAGE, flag_data)) {
		pr_err("read clear_flag failed");
		return -EINVAL;
	}

	flag_data[7] = val;
	if (slg2_memory_write(FIRST_USE_PAGE, flag_data)) {
		pr_err("update flag_data failed");
		return -EINVAL;
	}
	return size;
}
static struct device_attribute dev_attr_slg2_clear =
	__ATTR(slg2_clear, S_IRUSR | S_IWUSR, slg2_clear_show, slg2_clear_store);

static int slg2_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct slg2_data *info;
	const char *chip_name_cmdline;

	pr_err("enter");
	info = devm_kzalloc(&pdev->dev, sizeof(struct slg2_data), GFP_KERNEL);
	if (!info) {
		pr_err("alloc mem fail");
		return -ENOMEM;
	}

	info->pdev = pdev;
	info->dev = &pdev->dev;
	platform_set_drvdata(pdev, info);
	ret = of_property_read_string(pdev->dev.of_node, "role", &info->role);
	if (ret < 0) {
		pr_info("can not find role (%d)", ret);
		info->role = MASTER_SECRET;
	}
	ret = of_property_read_string(pdev->dev.of_node, "chip_name", &info->chip_name);
	if (ret < 0) {
		pr_info("can not find chip name(%d)", ret);
		info->chip_name = "ACL16_SLG2";
	}
	chip_name_cmdline = find_secret_attr_by_name("chip_name");
	#if 1 // 0 for debug
	if (!chip_name_cmdline || strcmp(chip_name_cmdline, info->chip_name) != 0){
		pr_info("chip_name:%s chip_name from cmdline:%s not match!",
			info->chip_name, !chip_name_cmdline? "NULL" : chip_name_cmdline);
		return -EINVAL;
	}
	#endif
	cmdline_attrs_init(info);
	ret = onewire_gpio_init(info);
	if (ret) {
		pr_info("onewire_gpio_init fail");
		return ret;
	}
	ret = enable_gpio_init(info);
	if (ret) {
		pr_info("enable_gpio_init fail");
		return ret;
	}
	g_info = info;
	raw_spin_lock_init(&info->io_lock);
	info->secret_dev = secret_device_register(info->role, info->chip_name, NULL, info, &slg2_secret_ops);
	if (IS_ERR_OR_NULL(info->secret_dev)) {
		pr_err("failed to register secret device");
		return PTR_ERR(info->secret_dev);
	}

	if (device_create_file(info->dev, &dev_attr_slg2_cmdbuf))
		pr_err("create %s file failed\n", dev_attr_slg2_cmdbuf.attr.name);
	if (device_create_file(info->dev, &dev_attr_slg2_clear))
		pr_err("create %s file failed\n", dev_attr_slg2_clear.attr.name);

	pr_err("drv probe succeeded");

	return 0;
}

static int slg2_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id slg2_of_ids[] = {
	{ .compatible = "acl,slg2" },
	{ },
};

static struct platform_driver slg2_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "acl,slg",
		.of_match_table = slg2_of_ids,
	},
	.probe = slg2_probe,
	.remove = slg2_remove,
};

static int __init slg2_init(void)
{
	pr_err("enter");
	return platform_driver_register(&slg2_driver);
}

static void __exit slg2_exit(void)
{
	pr_err("nter");
	platform_driver_unregister(&slg2_driver);
}

module_init(slg2_init);
module_exit(slg2_exit);
MODULE_LICENSE("GPL");
