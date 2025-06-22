// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright(c) 2023 - 2024 X-Ring technologies Inc.
 */
#include <asm-generic/errno-base.h>
#include <asm/unaligned.h>
#include <linux/async.h>
#include <linux/bsg.h>
#include <linux/version.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_ioctl.h>
#include <scsi/scsi_proto.h>
#include <scsi/scsi_dbg.h>
#include <scsi/sg.h>
#include "ufshcd.h"

#include <soc/xring/rpmb.h>

#include "ufs-xring.h"

struct rpmb_frame {
	__u8 stuff[196];
	__u8 key_mac[32];
	__u8 data[256];
	__u8 nonce[16];
	__be32 write_counter;
	__be16 addr;
	__be16 block_count;
	__be16 result;
	__be16 req_resp;
};

#define RPMB_FRAME_SIZE sizeof(struct rpmb_frame)

#define UFS_RPMB_REGION_MAX 3
#define UFS_SCSI_RPMB_WLUN (UFS_UPIU_RPMB_WLUN &\
			    ~UFS_UPIU_WLUN_ID |\
			    SCSI_W_LUN_BASE)

#define SEC_PROTOCOL_UFS 0xEC
#define SEC_PROTOCOL_CMD_SIZE 12
#define SEC_PROTOCOL_RETRY 3
#define SEC_PROTOCOL_TIMEOUT msecs_to_jiffies(30000)

#define SECOND_BYTE_MASK 0xFF00

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
enum rpmb_unit_desc_param {
	RPMB_UNIT_DESC_PARAM_LEN		= 0x0,
	RPMB_UNIT_DESC_PARAM_TYPE		= 0x1,
	RPMB_UNIT_DESC_PARAM_UNIT_INDEX		= 0x2,
	RPMB_UNIT_DESC_PARAM_LU_ENABLE		= 0x3,
	RPMB_UNIT_DESC_PARAM_BOOT_LUN_ID	= 0x4,
	RPMB_UNIT_DESC_PARAM_LU_WR_PROTECT	= 0x5,
	RPMB_UNIT_DESC_PARAM_LU_Q_DEPTH		= 0x6,
	RPMB_UNIT_DESC_PARAM_PSA_SENSITIVE	= 0x7,
	RPMB_UNIT_DESC_PARAM_MEM_TYPE		= 0x8,
	RPMB_UNIT_DESC_PARAM_REGION_EN		= 0x9,
	RPMB_UNIT_DESC_PARAM_LOGICAL_BLK_SIZE	= 0xA,
	RPMB_UNIT_DESC_PARAM_LOGICAL_BLK_COUNT	= 0xB,
	RPMB_UNIT_DESC_PARAM_REGION0_SIZE	= 0x13,
	RPMB_UNIT_DESC_PARAM_REGION1_SIZE	= 0x14,
	RPMB_UNIT_DESC_PARAM_REGION2_SIZE	= 0x15,
	RPMB_UNIT_DESC_PARAM_REGION3_SIZE	= 0x16,
	RPMB_UNIT_DESC_PARAM_PROVISIONING_TYPE	= 0x17,
	RPMB_UNIT_DESC_PARAM_PHY_MEM_RSRC_CNT	= 0x18,
};
#endif

enum rpmb_req_type {
	RPMB_PROGRAM_KEY	= 0x01,
	RPMB_GET_WRITE_COUNTER	= 0x02,
	RPMB_DATA_WRITE		= 0x03,
	RPMB_DATA_READ		= 0x04,
	RPMB_RESULT_READ	= 0x05,
};

static int do_scsi_security_cmd(struct scsi_device *sdev, u8 region,
				void *buf, unsigned int len, bool out)
{
	struct scsi_sense_hdr sshdr = { 0 };
	int retry = SEC_PROTOCOL_RETRY;
	int ret;
	u8 cmd[SEC_PROTOCOL_CMD_SIZE];

	/*
	 * 0:	OPERATION CODE
	 * 1:	SECURITY PROTOCOL
	 * 2-3:	RPMB Region
	 * 5:	INC_512 for bit 7
	 * 6-9:	ALLOCATION / TRANSFER LENGTH
	 */
	memset(cmd, 0, SEC_PROTOCOL_CMD_SIZE);
	cmd[0] = out ? SECURITY_PROTOCOL_OUT : SECURITY_PROTOCOL_IN;
	cmd[1] = SEC_PROTOCOL_UFS;
	put_unaligned_be16((region << 8) | 1, cmd + 2);
	cmd[4] = 0;
	put_unaligned_be32(len, cmd + 6);

retry:
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	ret = scsi_execute_req(sdev, cmd, out ? DMA_TO_DEVICE : DMA_FROM_DEVICE,
			       buf, len, &sshdr,
			       SEC_PROTOCOL_TIMEOUT, SEC_PROTOCOL_RETRY, NULL);
#else
	ret = scsi_execute_cmd(sdev, cmd, out ? REQ_OP_DRV_OUT : REQ_OP_DRV_IN,
			buf, len, SEC_PROTOCOL_TIMEOUT, SEC_PROTOCOL_RETRY,
			&(struct scsi_exec_args) {
				.sshdr = &sshdr,
			});
#endif

	if (ret) {
		/* device reset may occur several times */
		if (retry-- > 0 && scsi_sense_valid(&sshdr) &&
		    sshdr.sense_key == UNIT_ATTENTION) {
			dev_dbg(&sdev->sdev_gendev, "RPMB scsi cmd retry: %d\n",
				retry);
			goto retry;
		} else {
			dev_err(&sdev->sdev_gendev, "RPMB scsi cmd fail: %d\n",
				ret);
			ret = -EIO;
			if (scsi_sense_valid(&sshdr) && sshdr.sense_key)
				scsi_print_sense_hdr(sdev, "RPMB scsi cmd", &sshdr);
		}
	}

	return ret;
}

static int do_rpmb_cmd(struct device *dev, u8 region,
		       struct rpmb_frame *req, unsigned int len,
		       struct rpmb_frame *resp, unsigned int rlen)
{
	struct scsi_device *sdev = to_scsi_device(dev);
	int ret;
	u16 req_resp;

	ret = scsi_device_get(sdev);
	if (ret)
		return ret;
	scsi_autopm_get_device(sdev);

	/* SECURITY PROTOCOL OUT */
	ret = do_scsi_security_cmd(sdev, region, req, len, true);
	if (ret) {
		dev_err(&sdev->sdev_gendev, "RPMB SO 1 failed: %d\n", ret);
		goto out;
	}

	/* SECURITY PROTOCOL OUT */
	req_resp = be16_to_cpu(req->req_resp & SECOND_BYTE_MASK);
	if ((req_resp == RPMB_PROGRAM_KEY) ||
	    (req_resp == RPMB_DATA_WRITE)) {
		memset(req, 0, RPMB_FRAME_SIZE);
		req_resp = (req_resp & SECOND_BYTE_MASK) | RPMB_RESULT_READ;
		req->req_resp = cpu_to_be16(req_resp);
		ret = do_scsi_security_cmd(sdev, region, req,
					   RPMB_FRAME_SIZE, true);
		if (ret) {
			dev_err(&sdev->sdev_gendev, "RPMB SO 2 failed: %d\n", ret);
			goto out;
		}
	}

	/* SECURITY PROTOCOL IN */
	ret = do_scsi_security_cmd(sdev, region, resp, rlen, false);
	if (ret) {
		dev_err(&sdev->sdev_gendev, "RPMB SI failed: %d\n", ret);
		goto out;
	}

out:
	scsi_autopm_put_device(sdev);
	scsi_device_put(sdev);
	return ret;
}

static int ufs_xring_rpmb_get_capacity(struct device *dev, u8 region)
{
	struct scsi_device *sdev = to_scsi_device(dev);
	struct ufs_hba *hba = shost_priv(sdev->host);
	int ret;
	u8 *desc_buf;

	if (region > UFS_RPMB_REGION_MAX)
		return -EINVAL;

	ret = scsi_device_get(sdev);
	if (ret)
		return ret;

	desc_buf = kmalloc(QUERY_DESC_MAX_SIZE, GFP_KERNEL);
	if (!desc_buf) {
		ret = -ENOMEM;
		goto out_dev;
	}

	ret = ufshcd_read_desc_param(hba, QUERY_DESC_IDN_UNIT, UFS_UPIU_RPMB_WLUN,
				     0, desc_buf, QUERY_DESC_MAX_SIZE);
	if (ret) {
		dev_err(hba->dev, "%s: cann't get rpmb desc: %d\n",
			dev_name(hba->dev), ret);
		goto out;
	}

	/* region 0 is always enabled */
	if (region == 0)
		/* region size is defined in 128KB unit */
		ret = desc_buf[RPMB_UNIT_DESC_PARAM_REGION0_SIZE] * 128;
	else if (desc_buf[RPMB_UNIT_DESC_PARAM_REGION_EN] & (1 << region))
		ret = desc_buf[RPMB_UNIT_DESC_PARAM_REGION0_SIZE + region] * 128;
	else
		ret = 0;

out:
	kfree(desc_buf);
out_dev:
	scsi_device_put(sdev);
	return ret;
}

static bool check_resp(u16 req, struct rpmb_frame *f)
{
	u16 req_resp = be16_to_cpu(f->req_resp);

	/* According to spec, the response corresponds to the request */
	if ((req << 8) != req_resp)
		return false;

	return true;
}

static int ufs_xring_rpmb_program_key(struct device *dev, u8 region,
				      int len, u8 *req, int rlen, u8 *resp)
{
	struct rpmb_frame *in = (struct rpmb_frame *)req;
	struct rpmb_frame *out = (struct rpmb_frame *)resp;
	int ret;

	if (len != RPMB_FRAME_SIZE || rlen != RPMB_FRAME_SIZE)
		return -EINVAL;

	if (be16_to_cpu(in->req_resp) != RPMB_PROGRAM_KEY)
		return -EINVAL;

	ret = do_rpmb_cmd(dev, region, in, len, out, rlen);
	if (!ret && (out->result || !check_resp(RPMB_PROGRAM_KEY, out)))
		dev_warn(dev, "RPMB Program key failed: %0X %0X",
			 be16_to_cpu(out->result),
			 be16_to_cpu(out->req_resp));

	return ret;
}

static int ufs_xring_rpmb_get_write_counter(struct device *dev, u8 region,
					    int len, u8 *req, int rlen, u8 *resp)
{
	struct rpmb_frame *in = (struct rpmb_frame *)req;
	struct rpmb_frame *out = (struct rpmb_frame *)resp;
	int ret;

	if (len != RPMB_FRAME_SIZE || rlen != RPMB_FRAME_SIZE)
		return -EINVAL;

	if (be16_to_cpu(in->req_resp) != RPMB_GET_WRITE_COUNTER)
		return -EINVAL;

	ret = do_rpmb_cmd(dev, region, in, len, out, rlen);
	if (!ret && (out->result || !check_resp(RPMB_GET_WRITE_COUNTER, out)))
		dev_warn(dev, "RPMB get write counter failed: %0X %0X",
			 be16_to_cpu(out->result),
			 be16_to_cpu(out->req_resp));

	return ret;
}

static int ufs_xring_rpmb_write_blocks(struct device *dev, u8 region,
				       int len, u8 *req, int rlen, u8 *resp)
{
	struct rpmb_frame *in = (struct rpmb_frame *)req;
	struct rpmb_frame *out = (struct rpmb_frame *)resp;
	struct rpmb_dev *rdev = rpmb_dev_find_by_device(dev);
	int ret;

	if (len % RPMB_FRAME_SIZE || rlen != RPMB_FRAME_SIZE)
		return -EINVAL;

	if (len > rdev->cap.wr_cnt_max * RPMB_FRAME_SIZE)
		return -EINVAL;

	if (be16_to_cpu(in->req_resp) != RPMB_DATA_WRITE)
		return -EINVAL;

	ret = do_rpmb_cmd(dev, region, in, len, out, rlen);
	if (!ret && (out->result || !check_resp(RPMB_DATA_WRITE, out)))
		dev_warn(dev, "RPMB write failed: %0X %0X",
			 be16_to_cpu(out->result),
			 be16_to_cpu(out->req_resp));

	return ret;
}

static int ufs_xring_rpmb_read_blocks(struct device *dev, u8 region,
				       int len, u8 *req, int rlen, u8 *resp)
{
	struct rpmb_frame *in = (struct rpmb_frame *)req;
	struct rpmb_frame *out = (struct rpmb_frame *)resp;
	struct rpmb_dev *rdev = rpmb_dev_find_by_device(dev);
	int ret;

	if (len != RPMB_FRAME_SIZE || rlen % RPMB_FRAME_SIZE)
		return -EINVAL;

	if (rlen > rdev->cap.rd_cnt_max * RPMB_FRAME_SIZE)
		return -EINVAL;

	if (be16_to_cpu(in->req_resp) != RPMB_DATA_READ)
		return -EINVAL;

	ret = do_rpmb_cmd(dev, region, in, len, out, rlen);
	if (!ret && (out->result || !check_resp(RPMB_DATA_READ, out)))
		dev_warn(dev, "RPMB read failed: %0X %0X",
			 be16_to_cpu(out->result),
			 be16_to_cpu(out->req_resp));

	return ret;
}

static struct rpmb_ops ufs_xring_rpmb_ops = {
	.program_key = ufs_xring_rpmb_program_key,
	.get_capacity = ufs_xring_rpmb_get_capacity,
	.get_write_counter = ufs_xring_rpmb_get_write_counter,
	.write_blocks = ufs_xring_rpmb_write_blocks,
	.read_blocks = ufs_xring_rpmb_read_blocks,
};

void ufs_xring_rpmb_init_work(void *data, async_cookie_t cookie)
{
	struct ufs_hba *hba = (struct ufs_hba *)data;
	int err;
	u8 *desc_buf;
	struct rpmb_cap cap = { 0 };
	struct rpmb_dev *rdev = NULL;
	struct scsi_device *sdev;
	struct Scsi_Host *host = hba->host;

	wait_for_device_probe();

	sdev = scsi_device_lookup(host, 0, 0, UFS_SCSI_RPMB_WLUN);
	if (!sdev) {
		dev_err(hba->dev, "%s: can't find rpmb device\n", __func__);
		err = -ENODEV;
		return;
	}

	desc_buf = kmalloc(QUERY_DESC_MAX_SIZE, GFP_KERNEL);
	if (!desc_buf) {
		err = -ENOMEM;
		goto out_put;
	}

	err = ufshcd_read_desc_param(hba, QUERY_DESC_IDN_GEOMETRY, 0, 0,
				     desc_buf, QUERY_DESC_MAX_SIZE);
	if (err) {
		dev_err(hba->dev, "%s: can't get rpmb rw size: %d\n",
			__func__, err);
		goto out_free;
	}
	cap.wr_cnt_max = desc_buf[GEOMETRY_DESC_PARAM_RPMB_RW_SIZE];
	cap.rd_cnt_max = desc_buf[GEOMETRY_DESC_PARAM_RPMB_RW_SIZE];

	memset(desc_buf, 0, QUERY_DESC_MAX_SIZE);
	err = ufshcd_read_desc_param(hba, QUERY_DESC_IDN_UNIT, UFS_UPIU_RPMB_WLUN,
				     0, desc_buf, QUERY_DESC_MAX_SIZE);
	if (err) {
		dev_err(hba->dev, "%s: can't get rpmb desc: %d\n",
			__func__, err);
		goto out_free;
	}

	/* Bit-4: If set to 1, Advanced RPMB Mode is enabled */
	if (desc_buf[RPMB_UNIT_DESC_PARAM_REGION_EN] & 0x10) {
		dev_err(hba->dev, "%s: can't support Advanced RPMB\n",
			__func__);
		err = -EINVAL;
		goto out_free;
	}

	cap.block_size = 1 << desc_buf[RPMB_UNIT_DESC_PARAM_LOGICAL_BLK_SIZE];
	cap.auth_method = RPMB_HMAC_ALGO_SHA_256;
	cap.storage_type = RPMB_UFS;

	rdev = rpmb_dev_register(&sdev->sdev_gendev, &ufs_xring_rpmb_ops);
	if (IS_ERR(rdev)) {
		dev_err(hba->dev, "%s: can't register to rpmb: %ld\n",
			__func__, PTR_ERR(rdev));
		err = PTR_ERR(rdev);
		goto out_free;
	}
	rdev->cap = cap;

out_free:
	kfree(desc_buf);
out_put:
	scsi_device_put(sdev);

	if (rdev)
		dev_info(hba->dev, "xring ufs rpmb init success\n");
	else
		dev_err(hba->dev, "xring ufs rpmb init fail: %d\n", err);
}

static ASYNC_DOMAIN_EXCLUSIVE(ufs_rpmb_domain);
void ufs_xring_rpmb_probe(struct ufs_hba *hba)
{
	async_schedule_domain(ufs_xring_rpmb_init_work, hba, &ufs_rpmb_domain);
}
EXPORT_SYMBOL_GPL(ufs_xring_rpmb_probe);

void ufs_xring_rpmb_remove(struct ufs_hba *hba)
{
	struct scsi_device *sdev;
	struct Scsi_Host *host = hba->host;
	int err;

	sdev = scsi_device_lookup(host, 0, 0, UFS_SCSI_RPMB_WLUN);
	if (!sdev) {
		dev_err(hba->dev, "%s: can't find rpmb device\n", __func__);
		return;
	}

	err = rpmb_dev_unregister_by_device(&sdev->sdev_gendev);
	if (err)
		dev_err(hba->dev, "%s: remove rpmb dev failed: %d\n", __func__, err);

	scsi_device_put(sdev);
}
EXPORT_SYMBOL_GPL(ufs_xring_rpmb_remove);

MODULE_LICENSE("GPL v2");
