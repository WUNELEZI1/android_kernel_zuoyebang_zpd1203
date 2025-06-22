/* SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0 */
/*
 * Copyright (C) 2015-2019 Intel Corp. All rights reserved
 * Copyright (C) 2021-2022 Linaro Ltd
 * Copyright (C) 2023-2024 X-Ring technologies Inc
 */
#ifndef __RPMB_H__
#define __RPMB_H__

#include <linux/types.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/kref.h>

enum rpmb_auth_method {
	RPMB_HMAC_ALGO_SHA_256 = 0,
};

enum rpmb_storage_type {
	RPMB_UFS = 0,
};

/**
 * struct rpmb_ioc_cap_cmd - rpmb capabilities
 *
 * @region: rpmb region within RPMB partition.
 * @capacity: storage capacity (in units of 1KB)
 * @block_size: storage data block size (in units of bytes)
 * @wr_cnt_max: maximal number of block that can be written in a single request.
 * @rd_cnt_max: maximal number of block that can be read in a single request.
 * @auth_method: authentication method
 * @storage_type: storage type
 * @reserved: reserved to align to 4 bytes.
 */
struct rpmb_ioc_cap_cmd {
	__u16 region;
	__u16 capacity;
	__u16 block_size;
	__u16 wr_cnt_max;
	__u16 rd_cnt_max;
	__u16 auth_method;
	__u16 storage_type;
	__u16 reserved;
};

/**
 * struct rpmb_ioc_reqresp_cmd - general purpose reqresp
 *
 * Most RPMB operations consist of a set of request frames and response frames.
 *
 * It is used for programming the key, reading the counter , and reading or
 * writing blocks to the device. If the frames are malformed they may be
 * rejected by the underlying driver or the device itself.
 *
 * Assuming the transaction succeeds it is still up to user space to
 * validate the response and check MAC values correspond to the
 * programmed keys.
 *
 * @len: length of request frame
 * @rlen: length of response frame
 * @req: ptr to device specific request frame
 * @resp: ptr to device specific response frame
 * @region: RPMB region within the physical device
 */
struct rpmb_ioc_reqresp_cmd {
	__u32 len;
	__u32 rlen;
	__u64 req;
	__u64 resp;
	__u8 region;
};

#define RPMB_IOC_CAP_CMD     _IOWR(0xB8, 81, struct rpmb_ioc_cap_cmd)
#define RPMB_IOC_PKEY_CMD    _IOWR(0xB8, 82, struct rpmb_ioc_reqresp_cmd)
#define RPMB_IOC_COUNTER_CMD _IOWR(0xB8, 84, struct rpmb_ioc_reqresp_cmd)
#define RPMB_IOC_WBLOCKS_CMD _IOWR(0xB8, 85, struct rpmb_ioc_reqresp_cmd)
#define RPMB_IOC_RBLOCKS_CMD _IOWR(0xB8, 86, struct rpmb_ioc_reqresp_cmd)

/**
 * struct rpmb_ops - RPMB ops to be implemented by underlying block device
 *
 * @program_key    : program device key (once only op).
 * @get_capacity   : rpmb size in block size units in for region.
 * @get_write_counter: read write counter from RPMB device
 * @write_blocks   : write blocks to RPMB device
 * @read_blocks    : read blocks from RPMB device
 */
struct rpmb_ops {
	int (*program_key)(struct device *dev, u8 region,
			   int len, u8 *req,
			   int rlen, u8 *resp);
	int (*get_capacity)(struct device *dev, u8 region);
	int (*get_write_counter)(struct device *dev, u8 region,
				 int len, u8 *req,
				 int rlen, u8 *resp);
	int (*write_blocks)(struct device *dev, u8 region,
			    int len, u8 *req,
			    int rlen, u8 *resp);
	int (*read_blocks)(struct device *dev, u8 region,
			    int len, u8 *req,
			    int rlen, u8 *resp);
};

/**
 * struct rpmb_cap - RPMB capabilities
 *
 * @block_size     : block size in bytes
 * @wr_cnt_max     : maximal number of blocks that can be
 *                   written in one access.
 * @rd_cnt_max     : maximal number of blocks that can be
 *                   read in one access.
 * @auth_method    : authentication method
 * @storage_type   : storage type
 * @dev_id         : unique device identifier
 * @dev_id_len     : unique device identifier length
 */
struct rpmb_cap {
	u16 block_size;
	u16 wr_cnt_max;
	u16 rd_cnt_max;
	u16 auth_method;
	u16 storage_type;
	const u8 *dev_id;
	size_t dev_id_len;
};

/**
 * struct rpmb_dev - device which can support RPMB partition
 *
 * @lock       : the device lock
 * @dev        : device
 * @id         : device id
 * @cdev       : character dev
 * @status     : device status
 * @ops        : operation exported by rpmb
 */
struct rpmb_dev {
	struct mutex lock; /* device serialization lock */
	struct device dev;
	int id;
#ifdef CONFIG_XRING_RPMB_INTF_DEV
	struct cdev cdev;
	unsigned long status;
#endif /* CONFIG_XRING_RPMB_INTF_DEV */
	struct rpmb_cap cap;
	const struct rpmb_ops *ops;
};

#define to_rpmb_dev(x) container_of((x), struct rpmb_dev, dev)

#if IS_ENABLED(CONFIG_XRING_RPMB)
struct rpmb_dev *rpmb_dev_get(struct rpmb_dev *rdev);
void rpmb_dev_put(struct rpmb_dev *rdev);
struct rpmb_dev *rpmb_dev_find_by_device(struct device *parent);
struct rpmb_dev *rpmb_dev_find_by_name(const char *name);
struct rpmb_dev *rpmb_dev_register(struct device *dev,
				   const struct rpmb_ops *ops);
void *rpmb_dev_get_drvdata(const struct rpmb_dev *rdev);
void rpmb_dev_set_drvdata(struct rpmb_dev *rdev, void *data);
int rpmb_dev_unregister_by_device(struct device *dev);
int rpmb_get_capacity(struct rpmb_dev *rdev, u8 region);
int rpmb_program_key(struct rpmb_dev *rdev, u8 region,
		     int len, u8 *req, int rlen, u8 *resp);
int rpmb_get_write_counter(struct rpmb_dev *rdev, u8 region,
			   int len, u8 *req, int rlen, u8 *resp);
int rpmb_write_blocks(struct rpmb_dev *rdev, u8 region,
		      int len, u8 *req, int rlen, u8 *resp);
int rpmb_read_blocks(struct rpmb_dev *rdev, u8 region,
		     int len, u8 *req, int rlen, u8 *resp);

#else
static inline struct rpmb_dev *rpmb_dev_get(struct rpmb_dev *rdev)
{
	return NULL;
}

static inline void rpmb_dev_put(struct rpmb_dev *rdev) { }

static inline struct rpmb_dev *rpmb_dev_find_by_device(struct device *parent)
{
	return NULL;
}

static inline struct rpmb_dev *rpmb_dev_find_by_name(const char *name)
{
	return NULL;
}

static inline void *rpmb_dev_get_drvdata(const struct rpmb_dev *rdev)
{
	return NULL;
}

static inline void rpmb_dev_set_drvdata(struct rpmb_dev *rdev, void *data)
{
}

static inline struct rpmb_dev *rpmb_dev_register(struct device *dev,
						 const struct rpmb_ops *ops)
{
	return NULL;
}

static inline int rpmb_dev_unregister_by_device(struct device *dev)
{
	return 0;
}

static inline int rpmb_get_capacity(struct rpmb_dev *rdev, u8 region)
{
	return 0;
}

static inline int rpmb_program_key(struct rpmb_dev *rdev, u8 region,
				   int len, u8 *req, int rlen, u8 *resp)
{
	return 0;
}

static inline int rpmb_get_write_counter(struct rpmb_dev *rdev, u8 region,
					 int len, u8 *req, int rlen, u8 *resp)
{
	return 0;
}

static inline int rpmb_write_blocks(struct rpmb_dev *rdev, u8 region,
				    int len, u8 *req, int rlen, u8 *resp)
{
	return 0;
}

static inline int rpmb_read_blocks(struct rpmb_dev *rdev, u8 region,
				   int len, u8 *req, int rlen, u8 *resp)
{
	return 0;
}

#endif /* CONFIG_XRING_RPMB */

#endif /* __RPMB_H__ */
