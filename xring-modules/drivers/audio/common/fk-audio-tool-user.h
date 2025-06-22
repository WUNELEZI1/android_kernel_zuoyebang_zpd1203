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

#ifndef _FK_AUDIO_TOOL_H_
#define _FK_AUDIO_TOOL_H_

#define TOOL_IOCTL_MAGIC 'a'

#define AUDIO_SET_CALIBRATION		_IOWR(TOOL_IOCTL_MAGIC, \
							223, void *)
#define AUDIO_GET_CALIBRATION		_IOWR(TOOL_IOCTL_MAGIC, \
							224, void *)
#define AUDIO_SET_DSP_LOG_PARAM		_IOWR(TOOL_IOCTL_MAGIC, \
							225, void *)
#define AUDIO_DUMP_DSP_LOG			_IOWR(TOOL_IOCTL_MAGIC, \
							226, void *)
#define AUDIO_SET_DSP_PCM_PARAM		_IOWR(TOOL_IOCTL_MAGIC, \
							227, void *)
#define AUDIO_DUMP_DSP_PCM			_IOWR(TOOL_IOCTL_MAGIC, \
							228, void *)
#define AUDIO_DEBUG_REG_VALUE			_IOWR(TOOL_IOCTL_MAGIC, \
							229, void *)
#define AUDIO_DSP_UNCACHE_LOG		_IOWR(TOOL_IOCTL_MAGIC, \
							230, void *)

struct sceneParam {
	uint32_t stream;		//stream  id
	uint32_t port;
	bool audifFlag;
	int32_t reserved;
} __packed;

enum cal_id {
	CAL_STREAM = 0,
	CAL_ROUTE,
	CAL_DEVICE,
	CAL_ID_MAX,
};

struct _pipeline_context_t {
	uint8_t *pPipelineData[CAL_ID_MAX];
	uint32_t pipelineLen[CAL_ID_MAX];
};

struct SneceContent {
	struct sceneParam param;
	uint8_t crc[12];
	void *piplelineAddr;
	uint32_t pipelineSize;
};

struct dsp_log_dump_param_user {
	/* log level */
	uint8_t log_level;
	/* pord id */
	uint8_t log_port;
	/* pord enable */
	uint8_t log_enable;
} __packed;

struct dsp_pcm_dump_param_user {
	uint32_t point1;
	uint32_t point2;
	uint32_t point3;
} __packed;

enum tool_type {
	TOOL_TYPE_PIPE_PARAM = 0,
	TOOL_TYPE_AUDIF_PARAM,
	TOOL_TYPE_REG,
	TOOL_TYPE_DUMP_LOG,
	TOOL_TYPE_DUMP_PCM,
	TOOL_TYPE_DUMP_UNCACHE,
	TOOL_TYPE_DYN_PARAM,
	TOOL_TYPE_MAX,
};

struct aud_tool_meta {
	/* enum cal_type */
	int32_t tool_type;
	int32_t reserved[2];
};

struct aud_tool_header {
	int32_t size;
	int32_t version;
};

struct aud_tool_data {
	/* shared buffer size */
	int32_t tool_size;
	/* shared buffer handle */
	void *tool_data;
};

struct dsp_reg_read_info {
	uint32_t reg;
	uint32_t len;
};

struct dsp_reg_write_info {
	uint32_t reg;
	uint32_t value;
};

struct aud_tool_context {
	struct aud_tool_header tool_hdr;
	struct aud_tool_meta tool_meta;
	struct aud_tool_data tool_data_in;
	struct aud_tool_data tool_data_out;
};

#endif

