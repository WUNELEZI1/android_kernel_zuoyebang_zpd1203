/* SPDX-License-Identifier: GPL-2.0-only
 *
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

#ifndef _AUDIO_TOOL_CORE_H_
#define _AUDIO_TOOL_CORE_H_

#include <linux/miscdevice.h>
#include <linux/atomic.h>

#include "fk-audio-tool-user.h"
#include "fk-audio-log.h"
#include "fk-audio-ipc.h"
#include "../acore/fk-acore-define.h"
#include "../acore/fk-acore-api.h"
#include "../acore/fk-acore.h"
#include "../adsp/adsp_boot_driver.h"

//#define AUDIO_TOOL_DEBUG

#define DSP_LOG_DUMP_MEM_SIZE	0x20000
#define DSP_PCM_DUMP_MEM_SIZE	0x100000

#define DSP_LOG_DUMP_BLK_SIZE	0xA000
#define DSP_LOG_DUMP_BLK_NUM	3
#define DSP_PCM_DUMP_BLK_SIZE	0x4B000
#define DSP_PCM_DUMP_BLK_NUM	3
#define DSP_REG_DUMP_PACKED_LEN	24
#define DSP_UNCACHE_LOG_ENABLE  1
#define DSP_UNCACHE_LOG_PONINT_LEN  8
#define DSP_LOG_LEVEL_MAX  6
#define DSP_LOG_PORT_MAX   7

#define CAL_WAIT_TIMEOUT		(50)

struct dsp_log_dump_param_info {
	struct dsp_log_dump_param_user param;
	uint16_t blk_num;
	uint16_t blk_size;
	dma_addr_t phy_addr;
} __packed;

struct dsp_pcm_dump_param_info {
	struct dsp_pcm_dump_param_user param;
	uint16_t blk_num;
	uint32_t blk_size;
	dma_addr_t phy_addr;
} __packed;

enum log_type {
	LOG_DISABLE = 0,
	LOG_UART = 1,
	LOG_USB_P = 2,
	LOG_USB_C = 4,
	LOG_YYPE_MAX,
};

struct uncache_log_param {
	uint32_t log_type;
	/* point uncache log phy addr */
	uint32_t point_phy_addr;
	/* point log size */
	uint32_t point_len;
	/* char uncache log phy addr */
	uint32_t char_phy_addr;
	/* char log size */
	uint32_t char_len;
} __packed;

struct uncache_log_info {
	uint16_t value;
	uint16_t index;
	uint32_t id;
};

struct log_node {
	struct uncache_log_info info;
	struct log_node *next;
};

struct packetheader {
	uint16_t flag;
	uint16_t packet_type;
	uint32_t packet_len;
	uint32_t packet_wr_num;
	uint32_t packet_send_num;
	uint8_t reserved1;
	uint8_t reserved2;
};

struct dump_info {
	/* payload save phy addr */
	uint32_t phy_addr;
	/* payload size */
	uint32_t len;
	uint32_t reserve;
} __packed;

struct dump_blk {
	/* data buffer virtual address */
	void *vir_addr;
	/* data buffer physic address */
	dma_addr_t phy_addr;
	/* valid data size */
	uint32_t size;
	/* data transfer result
	 *  0: success
	 *  other: failure
	 */
	uint8_t count;
};

struct dsp_log_dump_addr_info {
	/* physical address of log share memory */
	uint32_t p_addr;
	/* size one block buffer, unit byte */
	uint32_t blk_buf_size;
	/* block buffer number */
	uint8_t blk_buf_num;
} __packed;

enum dsp_reg_value_debug_type {
	DSP_REG_VALUE_READ = 0x0,
	DSP_REG_VALUE_WRITE,
};

struct dsp_reg_value_info {
	uint8_t type;
	uint32_t reg_addr;
	uint16_t num;
	uint32_t value;
	dma_addr_t phy_addr;
} __packed;

/* dump data node structure */
struct dump_data_list_node {
	struct list_head data_list;
	struct dump_blk p_buf;
};

struct tool_pdata {
	struct list_head dump_data_list;
};

struct cal_param_info {
	struct sceneParam param;
	uint16_t size;
	dma_addr_t phy_addr;
	uint64_t vir_addr;
} __packed;

/* cal data node structure */
struct cal_data_list_node {
	struct list_head data_list;
	struct cal_param_info cal_data[CAL_ID_MAX];
	uint32_t stream;		//stream  id
	uint32_t port;
	int tool_type;
	int send_flag;
};

struct cal_pdata {
	struct list_head cal_data_list;
};

struct cal_memory_list_node {
	struct list_head data_list;
	uint16_t size;
	dma_addr_t phy_addr;
	uint64_t vir_addr;
};

struct audio_tool_pdata {
	struct list_head dump_data_list;
	struct mutex dump_list_lock;

	struct list_head cal_data_list;
	struct mutex cal_list_lock;

	struct list_head cal_memory_list;
	struct mutex memory_list_lock;

	struct dsp_log_dump_param_info log_param;
	struct dsp_pcm_dump_param_info pcm_param;
	struct cal_param_info cal_param;

	struct dump_blk log_blk_info;
	struct dump_blk pcm_blk_info;
	struct dump_blk reg_blk_info;
	struct mutex mem_lock;

	int pcm_blk_malloc_flag;
	char __user *user_buf;

	struct uncache_log_param uncache_log_p;
	uint8_t uncache_log_en_flag;
	int uncache_log_dumped_flag;

	wait_queue_head_t cal_wait_que;
	atomic_t cal_wait;
};

struct tool_drvdata {
	struct device *dev;
	struct miscdevice miscdev;
	struct fasync_struct *async_queue;
};

struct audio_tool_pdata *fk_audio_tool_pdata_get(void);
void fk_audio_tool_smem_free(struct acore_stream *ac_strm,
	struct xring_aud_msg *sc_msg);
int fk_audio_tool_set_cal(int32_t tool_type,
	size_t data_size, void *data);
int fk_audio_tool_stream_set_cal(struct acore_stream *ac_strm);
int fk_audio_tool_set_log_param(int32_t tool_type,
	size_t data_size, void *data);
int fk_audio_tool_set_pcm_param(int32_t tool_type,
	size_t data_size, void *data);
int fk_audio_tool_get_cal_type(struct aud_tool_data *data);
int fk_audio_tool_dump_read(char __user *buf, size_t len);
void fk_audio_tool_dump_copy_put(struct dump_blk p_buf);
void fk_audio_tool_msg_handle(void *msg, int len, void *priv);
void fk_audio_init_tool_dump_data(void);
int fk_audio_init_tool_pdata(void);
int fk_audio_tool_close_msg_send(void);
int fk_audio_tool_reg_value_read(int32_t tool_type,
	size_t data_size, void *data);
int fk_audio_tool_reg_value_write(int32_t tool_type,
	size_t data_size, void *data);
int fk_audio_tool_uncache_log_enable(uint8_t en);
void fk_audio_adsp_uncache_log_dump(void);
int fk_audio_tool_dsp_uncache_log(int32_t tool_type,
	size_t data_size, void *data);
struct tool_drvdata *fk_audio_tool_drvdata_get(void);
extern struct miscdevice audio_tool_misc;

#endif
