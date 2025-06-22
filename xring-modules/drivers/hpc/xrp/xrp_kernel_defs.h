/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
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



#ifndef _XRP_KERNEL_DEFS_H
#define _XRP_KERNEL_DEFS_H

#define XRP_IOCTL_MAGIC 'r'
#define XRP_IOCTL_ALLOC		_IO(XRP_IOCTL_MAGIC, 1)
#define XRP_IOCTL_FREE		_IO(XRP_IOCTL_MAGIC, 2)
#define XRP_IOCTL_QUEUE		_IO(XRP_IOCTL_MAGIC, 3)
#define XRP_IOCTL_QUEUE_NS	_IO(XRP_IOCTL_MAGIC, 4)
#define XRP_IOCTL_VDSP_POWERON	_IO(XRP_IOCTL_MAGIC, 5)
#define XRP_IOCTL_VDSP_RUN	_IO(XRP_IOCTL_MAGIC, 6)
#define XRP_IOCTL_VDSP_POWEROFF	_IO(XRP_IOCTL_MAGIC, 7)
#define XRP_IOCTL_VDSP_MEMMAP	_IOWR(XRP_IOCTL_MAGIC, 8, struct xrp_mem_info)


#define XRP_IOCTL_ENQUEUE _IOWR('X', 1, struct xrp_task_info_l)
#define XRP_IOCTL_CANCEL _IOWR('X', 2, struct xrp_task_cancel)
#define XRP_IOCTL_WAIT _IOWR('X', 3, struct xrp_task_info_l)
#define VDSP_IOCTL_MAP _IOWR('X', 4, struct vdsp_memmap_info)
#define VDSP_IOCTL_UNMAP _IOWR('X', 5, struct vdsp_memmap_info)

#define XRP_DSP_CMD_INLINE_INDATA_SIZE 256
#define XRP_DSP_CMD_INLINE_OUTDATA_SIZE 16
#define XRP_DSP_CMD_INLINE_BUFFER_SIZE 8
#define VDSP_CMD_MAX_NUM 8

#define LOG_LEVEL_NONE    0
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_PERF    2
#define LOG_LEVEL_INFO    3
#define LOG_LEVEL_DEBUG   4
#define LOG_LEVEL_TRACE   5

#define XRP_DRV_TAG "XRP_DRV"

extern int log_level;
#if IS_ENABLED(CONFIG_XRING_DEBUG)
#define xrp_print(level, fmt, ...) \
do { \
	if (level <= log_level) { \
		switch (level) { \
		case LOG_LEVEL_ERROR: \
			pr_err("XRP_DRV [%s]: " fmt, __func__, ##__VA_ARGS__); \
			break; \
		case LOG_LEVEL_PERF: \
			pr_debug("XRP_DRV [%s]: " fmt, __func__, ##__VA_ARGS__); \
			break; \
		case LOG_LEVEL_INFO: \
			pr_info("XRP_DRV [%s]: " fmt, __func__, ##__VA_ARGS__); \
			break; \
		case LOG_LEVEL_DEBUG: \
			pr_debug("XRP_DRV [%s]: " fmt, __func__, ##__VA_ARGS__); \
			break; \
		case LOG_LEVEL_TRACE: \
			pr_debug("XRP_DRV [%s]: " fmt, __func__, ##__VA_ARGS__); \
			break; \
		default: \
			pr_info("XRP_DRV_TAG [%s]: " fmt, __func__, ##__VA_ARGS__); \
			break; \
		} \
	} \
} while (0)
#else
#define xrp_print(level, fmt, ...) \
do { \
	switch (level) { \
	case LOG_LEVEL_ERROR: \
		pr_err("XRP_DRV [%s]: " fmt, __func__, ##__VA_ARGS__); \
		break; \
	case LOG_LEVEL_PERF: \
		pr_debug("XRP_DRV [%s]: " fmt, __func__, ##__VA_ARGS__); \
		break; \
	case LOG_LEVEL_INFO: \
		pr_info("XRP_DRV [%s]: " fmt, __func__, ##__VA_ARGS__); \
		break; \
	case LOG_LEVEL_DEBUG: \
		pr_debug("XRP_DRV [%s]: " fmt, __func__, ##__VA_ARGS__); \
		break; \
	case LOG_LEVEL_TRACE: \
		pr_debug("XRP_DRV [%s]: " fmt, __func__, ##__VA_ARGS__); \
		break; \
	default: \
		pr_info("XRP_DRV_TAG [%s]: " fmt, __func__, ##__VA_ARGS__); \
		break; \
	} \
} while (0)
#endif
enum {
	XRP_FLAG_READ = 0x1,
	XRP_FLAG_WRITE = 0x2,
	XRP_FLAG_READ_WRITE = 0x3,
};

//vdsp status
enum {
	VDSP_DEFAULT = 0x0,
	VDSP_POWER_ON = 0x1,
	VDSP_POWER_OFF = 0x2,
	VDSP_BEGIN_WAITI = 0x3,
	VDSP_END_WAITI = 0x4,
};

struct xrp_mem_map_info {
	bool flag;
	int prot;
	u32 da;
};

struct xrp_mem_info {
	int fd;
	int size;
	int type;//0-sysram;  1-sysrom
	struct xrp_mem_map_info vdsp;
};

struct vdsp_memmap_info {
	int fd;
	int size;
	int type;
	bool flag;
	int prot;
	unsigned int da;
};

struct boot_kernel_mem_map_info {
	struct vdsp_memmap_info  map_info;
	void *dmabuf;
	void *kernel_vaddr;
	dma_addr_t dma_handle;
};

struct xrp_vdsp_buffer {
	uint32_t flag;
	uint32_t size;
	uint32_t addr;
};

struct vdsp_profiling_data {
	uint32_t total_cycle;
};

struct common_param {
	uint32_t op_id;
	uint32_t input_num;
	uint32_t output_num;
	uint32_t io_size[XRP_DSP_CMD_INLINE_BUFFER_SIZE];
	uint32_t cycle;
};

struct xrp_vdsp_cmd {
	uint32_t in_data_size;
	uint32_t out_data_size;
	uint32_t buffer_num;
	uint8_t in_data[XRP_DSP_CMD_INLINE_INDATA_SIZE];
	uint8_t out_data[XRP_DSP_CMD_INLINE_OUTDATA_SIZE];
	struct xrp_vdsp_buffer buffer[XRP_DSP_CMD_INLINE_BUFFER_SIZE];
	uint32_t profile_level;
	struct vdsp_profiling_data profile_info;
	uint32_t status;
	struct common_param com_param;
};

struct xrp_vdsp_cmds {
	uint32_t cmd_type;
	uint32_t cmd_num;
	struct xrp_vdsp_cmd dsp_cmds[VDSP_CMD_MAX_NUM];
	uint32_t cmd_time[8];
};

//kernel vdsp shakehands
struct xrp_shakehand_cmd {
	uint32_t vdsp_status;
};

struct xrp_task_info_l {
	uint32_t    msg_id;
	uint32_t    priority;
	uint32_t    wait_time_ms;
	uint32_t    total_cycle;
	struct xrp_vdsp_cmds   cmds;
};

struct xrp_task_cancel {
	uint32_t msg_id;
	uint32_t priority;
	uint32_t status;
};

struct xrp_ioctl_alloc {
	__u32 size;
	__u32 align;
	__u64 addr;
};

struct xrp_ioctl_buffer {
	__u32 flags;
	__u32 size;
	__u64 addr;
};

enum {
	XRP_QUEUE_FLAG_NSID = 0x4,
	XRP_QUEUE_FLAG_PRIO = 0xff00,
	XRP_QUEUE_FLAG_PRIO_SHIFT = 8,

	XRP_QUEUE_VALID_FLAGS =
		XRP_QUEUE_FLAG_NSID |
		XRP_QUEUE_FLAG_PRIO,
};

struct xrp_ioctl_queue {
	__u32 flags;
	__u32 in_data_size;
	__u32 out_data_size;
	__u32 buffer_size;
	__u64 in_data_addr;
	__u64 out_data_addr;
	__u64 buffer_addr;
	__u64 nsid_addr;
};

void set_hw_ops(void *dev, const void *hw_ops, void *hw_arg);
int vdsp_task_memmap(void *dev, struct boot_kernel_mem_map_info *mem_map_info);
int vdsp_task_memunmap(void *dev, struct vdsp_memmap_info *mem_map_info);

int vdsp_poweron_done_action(bool should_wake_up);
int vdsp_poweroff_done_action(bool should_wake_up);
int task_first_open_action(bool should_wake_up);
int task_last_close_action(bool should_wake_up);
void update_task_count(bool increment, uint32_t operator_count, uint64_t cycles);

#endif
