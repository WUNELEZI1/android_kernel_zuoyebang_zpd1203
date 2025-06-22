// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  w1_slg.c - Aisinochip SLG driver
 *
 * Copyright (c) Shanghai Aisinochip Electronics Techology Co., Ltd.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include <linux/random.h>

#include <linux/scatterlist.h>

#include <crypto/akcipher.h>
#include <crypto/rng.h>
#include <crypto/algapi.h>
#include <crypto/hash.h>


#include "../xr_w1_internal.h"
#include "../xr_w1.h"

#include "w1_slg.h"

#define W1_FAMILY_SLG 0xAC

#define SLG_DEBUG

#ifdef SLG_DEBUG

#define DEBUG printk
#define DEBUG_BYTES printf_hex

#else

#define DEBUG(fmt, ...)
#define DEBUG_BYTES(a, b)

#endif

#define SLG_MEMORY_READ 0xCA
#define SLG_ECDSA_SIGN 0x2A
#define SLG_DC_DECREASE 0x24
#define SLG_DC_GET 0xC0
#define SLG_ROMID_GET 0xB2
#define SLG_MANDI_GET 0x4F
#define SLG_POWEROFF_MODE 0x58
#define SLG_ECDSA_DISABLE 0xFC
#define SLG_CHIP_KILL 0xFE
#define SLG_AUTHENTICATE 0xFF

#define RESPONSE_SUCCESS 0xAA

#define SLG_MATCH_ONE 0x50
#define SLG_READ_ROM 0x30
#define SLG_RESUME 0xA5
#define SLG_RANDOM_NUMBER_GET 0x84
#define SLG_ERR_FUNC_CMD 0xCD
#define SLG_PRESS_MODE 0xFD

#define SLG_AUTH_PRESS_TEST BIT(0)

#define W1_BUS_ERROR -201
#define COMMAND_ERROR -202
#define PARAM_ERROR -203

#define SLG_NOT_EXIST -210
#define SHASH_ALLOC_FAILED -211
#define AKCIPHER_ALLOC_FAILED -212
#define AKCIPHER_REQUEST_FAILED -213

#define ECC_SIGN_WAIT_MAX	80
#define GENERAL_WAIT_MAX	10

#define MAX_BUFF_LEN 1024
#define MAX_SEND_LEN 4
#define MAX_RECV_LEN 16
#define ROMID_LEN 8
#define MANUID_LEN 2
#define MANUID_ROMID_LEN 12

#define XR_FUNC_PARA_ERR_MASK					(1 << 0)
#define XR_FUNC_CRC_ERR_MASK					(1 << 1)
#define XR_FUNC_RX_COMMU_ERR_MASK				(1 << 2)
#define XR_FUNC_TX_COMMU_ERR_MASK				(1 << 3)
#define XR_FUNC_CMD_UNSUPPORT_ERR_MASK			(1 << 4)
#define XR_FUNC_ACCESS_LIMT_ERR_MASK			(1 << 5)
#define XR_FUNC_EEROM_ERR_MASK					(1 << 6)
#define XR_FUNC_ACCESS_CMD_ERR_MASK				(1 << 7)
#define XR_FUNC_ECDSA_FAIL_ERR_MASK				(1 << 8)
#define XR_FUNC_CHIP_UNVALID_ERR_MASK			(1 << 9)
#define XR_FUNC_RX_CRC_ERR_MASK					(1 << 10)
#define XR_FUNC_SUCCESS_MASK					(1 << 11)
#define XR_ROM_SEARCH_DISCREPANCY_MASK			(1 << 12)
#define XR_ROM_SLV_NO_PRESENT_ERR_MASK			(1 << 13)
#define XR_ROM_TIMEOUT_WARNING_ERR_MASK			(1 << 14)
#define XR_ROM_SEARCH_SLV_NO_PRESENT_ERR_MASK	(1 << 15)

#define XR_MATCH_NORMAL 0
#define XR_MATCH_FORCE 1
#define XR_MATCH_RESUME 2
#define XR_MATCH_READ_ROM 4

#define MIN_CMD_LEN 3
#define MAX_CMD_LEN 139
#define LOC_X 1
#define SINGLE_CMD_LEN 6
#define CMD_PARM_LEN 11
#define LOC_CMD 0
#define LOC_PARM 1

struct w1_slave *slg_slave;

struct result_data_struct {
	unsigned char result_buf[128];
	int result_len;
	int status;
};

static struct result_data_struct result_data;

DEFINE_MUTEX(slg_lock);

#ifdef SLG_DEBUG
static void *hex2asc(unsigned char *dest, unsigned char *src, unsigned int len)
{
	unsigned int i;
	unsigned char *p;

	p = dest;
	if (len % 2)
		*dest++ = (*src++ & 0x0F) + 0x30;
	for (i = 0; i < (len / 2); i++) {
		*dest++ = ((*src & 0xF0) >> 4) + 0x30;
		*dest++ = (*src++ & 0x0F) + 0x30;
	}
	while (p != dest) {
		if (*p >= 0x3A)
			*p += 7;	/* if *p > '9', convert to uppercase */
		p++;
	}
	return ((unsigned char *)dest);
}

static void printf_hex(unsigned char *output, int output_len)
{
	char *buffer = NULL;

	if (output_len == 0)
		return;
	else if (output_len > MAX_BUFF_LEN / 2)
		output_len = MAX_BUFF_LEN / 2;

	buffer = kmalloc(MAX_BUFF_LEN * sizeof(char), GFP_KERNEL);
	if (buffer == NULL)
		return;

	memset(buffer, 0, MAX_BUFF_LEN * sizeof(char));

	hex2asc(buffer, output, output_len << 1);

	DEBUG("w1 data: %s\n", buffer);

	kfree(buffer);
}
#endif

/*
 * Send and Receive data by W1 bus
 *
 * @param input: Send data buffer
 * @param input_len: Send data length
 * @return: 0=Success; others=failure, see Error code
 *
 */

#define BUS_SEND_RETRY 5
#define BUS_SEND_INPUT_LEN_MAX 500

static int bus_send_recv(unsigned char *input, int input_len, int force_match)
{
	int ret = 0;
	unsigned short retry = BUS_SEND_RETRY;

	if (input_len > BUS_SEND_INPUT_LEN_MAX || input == NULL)
		return PARAM_ERROR;

	if (slg_slave == NULL)
		return SLG_NOT_EXIST;

	mutex_lock(&slg_slave->master->mutex);

RETRY:
	if (force_match == XR_MATCH_NORMAL)
		ret = w1_reset_select_slave_reg(slg_slave);
	else if (force_match == XR_MATCH_FORCE)
		ret = w1_reset_select_slave_match(slg_slave);
	else if (force_match == XR_MATCH_RESUME)
		w1_resume(slg_slave);
	else if (force_match == XR_MATCH_READ_ROM)
		w1_read_rom(slg_slave);


	if (ret) {
		DEBUG("w1_reset_select_slave_reg failed\n");
		ret = W1_BUS_ERROR;
		goto OUT;
	}

	ret = w1_write_block_reg(slg_slave->master, input, input_len);
	if (ret) {
		DEBUG("w1_write_block_reg failed\n");
		ret = PARAM_ERROR;
		goto OUT;
	}

	ret = w1_start_cmd(slg_slave);
	if (ret <= 0) {
		DEBUG("w1_start_cmd failed\n");
		ret = W1_BUS_ERROR;
		goto END;
	} else {
		if ((ret & XR_ROM_SEARCH_SLV_NO_PRESENT_ERR_MASK) ||
						(ret & XR_ROM_TIMEOUT_WARNING_ERR_MASK) ||
						(ret & XR_ROM_SLV_NO_PRESENT_ERR_MASK) ||
						(!(ret & XR_FUNC_SUCCESS_MASK)) ||
						(ret & XR_FUNC_RX_CRC_ERR_MASK)) {
			DEBUG("cmd error ret:%d\n", ret);
			ret = COMMAND_ERROR;
			goto END;
		} else {
			ret = 0;
			goto OUT;
		}
	}

END:
	if (retry > 0) {
		DEBUG("[slg] command send failed retry %d\n", 5 - retry + 1);
		retry--;
		goto RETRY;
	}

OUT:
	DEBUG("slg send data end okay\n");
	DEBUG_BYTES(input, input_len);
	mutex_unlock(&slg_slave->master->mutex);

	return ret;
}

/*
 * Read a page of memory data from SLG
 *
 * @param index: The memory page index
 * @param output: Memory data buffer
 * @param output_len: Memory data length
 * @return: 0=Success; others=failure, see Error code
 */
static int slg_memory_read(int index, unsigned char *output, int *output_len)
{
	unsigned char send[8];
	int ret;
	int recv_len = 32;

	if (output == NULL || output_len == NULL)
		return PARAM_ERROR;

	*output_len = 0;

	w1_set_resp_delay(slg_slave, GENERAL_WAIT_MAX);

	if (slg_slave == NULL)
		return SLG_NOT_EXIST;

	send[0] = SLG_MEMORY_READ;
	send[1] = 1;	/* parameter length */
	send[2] = index;

	ret = bus_send_recv(send, 3, XR_MATCH_NORMAL);

	if (ret == 0) {
		ret = w1_read_block_reg(slg_slave->master, output, recv_len);
		DEBUG("%s recv data:\n", __func__);
		DEBUG_BYTES(output, recv_len);
		*output_len = recv_len;
	}

	return ret;
}

/*
 * Do ECDSA sign the data by SLG
 *
 * @param input: The additional data to sign
 * @param input_len: The length of the additional data
 * @param output: The signature data, r and s
 * @param output_len: The length of the signature data
 * @return: 0=Success; others=failure, see Error code
 */

#define SEND_LEN_128 128

static int slg_ecdsa_sign(unsigned char *input, int input_len, unsigned char *output,
		   int *output_len)
{
	unsigned char send[SEND_LEN_128];
	int ret;
	int recv_len = 64;
	int index = 0;

	if (output == NULL || output_len == NULL || input == NULL)
		return PARAM_ERROR;

	*output_len = 0;

	w1_set_resp_delay(slg_slave, ECC_SIGN_WAIT_MAX);

	send[index++] = SLG_ECDSA_SIGN;
	send[index++] = input_len;

	if (input_len > (SEND_LEN_128 - index))
		return PARAM_ERROR;

	memcpy(&send[index], input, input_len);
	index += input_len;

	ret = bus_send_recv(send, index, XR_MATCH_NORMAL);

	if (ret == 0) {
		ret = w1_read_block_reg(slg_slave->master, output, recv_len);
		DEBUG("%s recv data:\n", __func__);
		DEBUG_BYTES(output, recv_len);
		*output_len = recv_len;
	}

	return ret;
}

/*
 * Get the Decrement Counter
 *
 * @param counter: The initial value of the DC
 * @return: postive=The DC value; negative=failure, see Error code
 */
static int slg_dc_get(void)
{
	unsigned char send[8];
	unsigned char recv[8];
	int ret;
	int recv_len = 4;

	w1_set_resp_delay(slg_slave, GENERAL_WAIT_MAX);

	send[0] = SLG_DC_GET;
	send[1] = 0;	/* parameter length */

	memset(recv, 0xFF, sizeof(recv));
	ret = bus_send_recv(send, 2, XR_MATCH_NORMAL);

	if (ret == 0) {
		ret = w1_read_block_reg(slg_slave->master, recv, recv_len);
		if (ret == 0) {
			DEBUG("%s recv data:\n", __func__);
			DEBUG_BYTES(recv, recv_len);
			ret = (recv[0] << 24) + (recv[1] << 16) +
			      (recv[2] << 8) + (recv[3]);
		}
	}

	return ret;
}

/*
 * Do decrease the Decrement Counter
 *
 * @return: 0=Success; others=failure, see Error code
 */
static int slg_dc_decrease(void)
{
	unsigned char send[8];
	int ret;

	w1_set_resp_delay(slg_slave, GENERAL_WAIT_MAX);

	send[0] = SLG_DC_DECREASE;
	send[1] = 0x00;	/* parameter length */

	ret = bus_send_recv(send, 2, XR_MATCH_NORMAL);

	return ret;
}

/*
 * Get ROM ID
 *
 * @param output: ROM ID data buffer
 * @param output_len: 8 bytes
 * @return: 0=Success; others=failure, see Error code
 */
static int slg_romid_get(unsigned char *output, int *output_len, int force_match)
{
	unsigned char send[MAX_SEND_LEN];
	unsigned char recv[MAX_RECV_LEN];
	int ret;
	int recv_len = MANUID_ROMID_LEN;
	int romid_len = ROMID_LEN;

	if (output == NULL || output_len == NULL)
		return PARAM_ERROR;

	*output_len = 0;

	w1_set_resp_delay(slg_slave, GENERAL_WAIT_MAX);

	send[0] = SLG_ROMID_GET;
	send[1] = 0x00;	/* parameter length */

	ret = bus_send_recv(send, 2, force_match);

	if (ret == 0) {
		ret = w1_read_block_reg(slg_slave->master, recv, recv_len);
		if (ret == 0) {
			if (recv[1] != RESPONSE_SUCCESS) {
				ret = -recv[1];
			} else {
				memcpy(output, &recv[2], romid_len);
				*output_len = romid_len;
			}
		}

		DEBUG("%s force_match:%d, recv data:\n", __func__, force_match);
		DEBUG_BYTES(output, romid_len);
		*output_len = romid_len;
	}

	return ret;
}

/*
 * Get Manufacture ID
 *
 * @param output: manufacture data buffer
 * @param output_len: 2 bytes
 * @return: 0=Success; others=failure, see Error code
 */
static int slg_manid_get(unsigned char *output, int *output_len)
{
	unsigned char send[MAX_SEND_LEN];
	unsigned char recv[MAX_RECV_LEN];
	int ret;
	int recv_len = MANUID_ROMID_LEN;
	int manuid_len = MANUID_LEN;

	if (output == NULL || output_len == NULL)
		return PARAM_ERROR;

	*output_len = 0;

	w1_set_resp_delay(slg_slave, GENERAL_WAIT_MAX);

	send[0] = SLG_ROMID_GET;
	send[1] = 0x00;	/* parameter length */

	ret = bus_send_recv(send, 2, XR_MATCH_NORMAL);

	if (ret == 0) {
		ret = w1_read_block_reg(slg_slave->master, recv, recv_len);
		if (ret == 0) {
			if (recv[1] != RESPONSE_SUCCESS) {
				ret = -recv[1];
			} else {
				memcpy(output, &recv[MANUID_LEN + ROMID_LEN], manuid_len);
				*output_len = manuid_len;
			}
		}

		DEBUG("%s recv data:\n", __func__);
		DEBUG_BYTES(output, manuid_len);
	}

	return ret;
}

/*
 * Set SLG to Poweroff Mode
 *
 * @return: 0=Success; others=failure, see Error code
 */
static int slg_poweroff_mode(void)
{
	unsigned char send[4];
	int ret;

	w1_set_resp_delay(slg_slave, GENERAL_WAIT_MAX);

	send[0] = 0x58;	/* command code: poweroff mode */
	send[1] = 0x02;	/* parameter length */
	send[2] = 0x5A;	/* parameter value 0 */
	send[3] = 0xA5;	/* parameter value 1 */

	ret = bus_send_recv(send, 4, XR_MATCH_NORMAL);

	return ret;
}

/*
 * Disable SLG ECDSA function
 *
 * @return: postive=Success; negative=failure, see Error code
 */
static int slg_ecdsa_disable(void)
{
	unsigned char ecdsa_disable[] = { 0x44, 0x69, 0x73, 0x5F, 0x45,
					  0x43, 0x44, 0x53, 0x41 };
	unsigned char send[12];
	int ret;
	int index = 0;

	w1_set_resp_delay(slg_slave, GENERAL_WAIT_MAX);

	send[index++] = SLG_ECDSA_DISABLE;
	send[index++] = sizeof(ecdsa_disable);
	memcpy(&send[index], ecdsa_disable, sizeof(ecdsa_disable));
	index += sizeof(ecdsa_disable);

	ret = bus_send_recv(send, index, XR_MATCH_NORMAL);

	return ret;
}

/*
 * Kill SLG chip
 *
 * @return: postive=Success; negative=failure, see Error code
 */
static int slg_chip_kill(void)
{
	unsigned char chip_kill[] = { 0x44, 0x69, 0x73, 0x44, 0x65,
				      0x76, 0x69, 0x63, 0x65 };
	unsigned char send[12];
	int ret;
	int index = 0;

	w1_set_resp_delay(slg_slave, GENERAL_WAIT_MAX);

	send[index++] = SLG_CHIP_KILL;
	send[index++] = sizeof(chip_kill);
	memcpy(&send[index], chip_kill, sizeof(chip_kill));
	index += sizeof(chip_kill);

	ret = bus_send_recv(send, index, XR_MATCH_NORMAL);

	return ret;
}

/*
 * get random number(32 bytes) from SLG
 *
 * @param index: The length of random number
 * @param output: Memory data buffer
 * @param output_len: Memory data length
 * @return: 0=Success; others=failure, see Error code
 */
static int slg_random_number_read(int index, unsigned char *output, int *output_len)
{
	unsigned char send[8];
	int ret;
	int recv_len = 32;

	if (output == NULL || output_len == NULL)
		return PARAM_ERROR;

	*output_len = 0;

	w1_set_resp_delay(slg_slave, GENERAL_WAIT_MAX);

	if (slg_slave == NULL)
		return SLG_NOT_EXIST;

	send[0] = SLG_RANDOM_NUMBER_GET;
	send[1] = 1;	/* parameter length */
	send[2] = index;

	ret = bus_send_recv(send, 3, XR_MATCH_NORMAL);

	if (ret == 0) {
		ret = w1_read_block_reg(slg_slave->master, output, recv_len);
		DEBUG("%s recv data:\n", __func__);
		DEBUG_BYTES(output, recv_len);
		*output_len = recv_len;
	}

	return ret;
}

/*
 * err func cmd 0xcd
 *
 * @return: postive=The DC value; negative=failure, see Error code
 */
static int slg_err_func_cmd(void)
{
	unsigned char send[8];
	unsigned char recv[8];
	int ret;
	int recv_len = 4;

	w1_set_resp_delay(slg_slave, GENERAL_WAIT_MAX);

	send[0] = SLG_ERR_FUNC_CMD;
	send[1] = 0x00;	/* parameter length */

	memset(recv, 0xFF, sizeof(recv));
	ret = bus_send_recv(send, 2, XR_MATCH_NORMAL);

	if (ret == 0) {
		ret = w1_read_block_reg(slg_slave->master, recv, recv_len);
		if (ret == 0) {
			DEBUG("%s recv data:\n", __func__);
			DEBUG_BYTES(recv, recv_len);
			ret = (recv[0] << 24) + (recv[1] << 16) +
			      (recv[2] << 8) + (recv[3]);
		}
	} else {
		DEBUG("%s cmd unsupport\n", __func__);
	}

	return ret;
}

/*
 * get random number(32 bytes) from SLG
 *
 * @param index: The length of random number
 * @param output: Memory data buffer
 * @param output_len: Memory data length
 * @return: 0=Success; others=failure, see Error code
 */
static int slg_press_test(int count)
{
	int ret = 0;
	int i;

	for (i = 0; i < count; i++) {
		ret = authenticate_battery();
		if (ret)
			DEBUG("%s auth fail ret:%d, i = %u\n", __func__, ret, i);
	}

	return ret;
}

/*
 * Compute SHA256
 *
 * @param data: messgee data buffer
 * @param datalen: messgee data length
 * @param digest: the result of SHA256
 * @return: 0=Success; others=failure, see Error code
 */
static int sha256_compute(const unsigned char *data, unsigned int datalen,
			  unsigned char *digest)
{
	struct crypto_shash *alg;
	char *hash_alg_name = "sha256-generic";
	int ret;

	struct sdesc {
		struct shash_desc shash;
		char ctx[];
	};

	struct sdesc *sdesc;
	int size;

	alg = crypto_alloc_shash(hash_alg_name, 0, 0);
	if (IS_ERR(alg)) {
		DEBUG("can't alloc alg %s\n", hash_alg_name);
		return SHASH_ALLOC_FAILED;
	}

	size = sizeof(struct shash_desc) + crypto_shash_descsize(alg);
	sdesc = kmalloc(size, GFP_KERNEL);
	if (!sdesc) {
		crypto_free_shash(alg);
		return -ENOMEM;
	}

	sdesc->shash.tfm = alg;

	ret = crypto_shash_digest(&sdesc->shash, data, datalen, digest);

	kfree(sdesc);

	crypto_free_shash(alg);

	return ret;
}

/*
 * Verify ECDSA signature
 *
 * @param pub_key: public key buffer, 32 bytes x and 32 bytes y
 * @param msg: messgee data buffer
 * @param msg_len: messgee data length
 * @param sig: the message signature, 32 bytes r and 32 bytes s
 * @return: 0=Success; others=failure, see Error code
 */
static int ecdsa_verify(uint8_t *pub_key, uint8_t *msg, int msg_len,
			uint8_t *sig)
{
	uint8_t pub_key_uncompressed[65] = {
		0x04,
	};
	uint8_t sig_asn1[128];
	uint8_t msg_digest[32];
	int ret = -1;
	uint8_t *heap_sign_asn1, *heap_msg_digest;

	int sig_index;
#define SIG_LEN_INDEX 1

	struct crypto_akcipher *tfm;
	const char *driver = "ecdsa-nist-p256-generic";
	struct akcipher_request *req;
	struct crypto_wait wait;
	struct scatterlist scat_tab[2];

	//uncompressed public key
	memcpy(&pub_key_uncompressed[1], pub_key, 64);

	//calc message sha256 digest
	ret = sha256_compute(msg, msg_len, msg_digest);
	if (ret != 0) {
		DEBUG("sha256 message failed: %d \r\n", ret);
		return ret;
	}

	//packaging ASN.1 signature
	sig_index = 0;
	sig_asn1[sig_index++] = 0x30;
	sig_asn1[sig_index++] = 0x00;

	sig_asn1[sig_index++] = 0x02;
	if (sig[0] >= 0x80) {
		sig_asn1[sig_index++] = 0x21;
		sig_asn1[sig_index++] = 0x00;
	} else {
		sig_asn1[sig_index++] = 0x20;
	}
	memcpy(&sig_asn1[sig_index], sig, 32);
	sig_index += 32;

	sig_asn1[sig_index++] = 0x02;
	if (sig[32] >= 0x80) {
		sig_asn1[sig_index++] = 0x21;
		sig_asn1[sig_index++] = 0x00;
	} else {
		sig_asn1[sig_index++] = 0x20;
	}

	memcpy(&sig_asn1[sig_index], &sig[32], 32);
	sig_index += 32;
	sig_asn1[SIG_LEN_INDEX] = sig_index - 2;

	tfm = crypto_alloc_akcipher(driver, 0, 0);
	if (IS_ERR(tfm)) {
		DEBUG("alg: akcipher: Failed to load tfm for %s: %ld\n", driver,
		      PTR_ERR(tfm));
		return AKCIPHER_ALLOC_FAILED;
	}

	req = akcipher_request_alloc(tfm, GFP_KERNEL);
	if (!req) {
		DEBUG("akcipher_request_alloc failed\r\n");
		ret = AKCIPHER_REQUEST_FAILED;
	} else {
		crypto_init_wait(&wait);
		crypto_akcipher_set_pub_key(
			tfm, pub_key_uncompressed,
			sizeof(pub_key_uncompressed));

		sg_init_table(scat_tab, 2);

		do {
			heap_sign_asn1 = kmalloc(sig_index, GFP_KERNEL);
			if (!heap_sign_asn1) {
				DEBUG("heap_sign_asn1 kmalloc failed\n");
				break;
			}
			memcpy(heap_sign_asn1, sig_asn1, sig_index);

			heap_msg_digest =
				kmalloc(sizeof(msg_digest), GFP_KERNEL);
			if (!heap_msg_digest) {
				DEBUG("heap_msg_digest kmalloc failed\n");
				kfree(heap_sign_asn1);
				break;
			}
			memcpy(heap_msg_digest, msg_digest,
				sizeof(msg_digest));

			sg_set_buf(&scat_tab[0], heap_sign_asn1,
					sig_index);
			sg_set_buf(&scat_tab[1], heap_msg_digest,
					sizeof(msg_digest));

			akcipher_request_set_crypt(req, scat_tab, NULL,
							sig_index,
							sizeof(msg_digest));

			akcipher_request_set_callback(
				req, CRYPTO_TFM_REQ_MAY_BACKLOG,
				crypto_req_done, &wait);

			ret = crypto_wait_req(
				crypto_akcipher_verify(req), &wait);

			kfree(heap_msg_digest);
			kfree(heap_sign_asn1);
		} while (0);

		akcipher_request_free(req);
	}
	crypto_free_akcipher(tfm);

	return ret;
}

//To be signed PAGE index
static const int PAGE_INDEX = 61;

//CA public key X's page number
static const int CA_PUBLIC_KEY_X = 57;

//CA public key Y's page number
static const int CA_PUBLIC_KEY_Y = 58;

//device signer R's page number
static const int DEVICE_SIGNER_R = 59;

//device signer S's page number
static const int DEVICE_SIGNER_S = 60;

//device public key X's page number
static const int DEVICE_PUBLIC_KEY_X = 61;

//device public key Y's page number
static const int DEVICE_PUBLIC_KEY_Y = 62;

static const uint8_t MI_CA_PUBLIC_X[] = { 0xc0, 0xe2, 0x5d, 0x5f, 0xa6, 0x9d,
					  0x5a, 0x3f, 0x95, 0x63, 0xf2, 0x70,
					  0x03, 0x33, 0x77, 0x24, 0x3a, 0x8e,
					  0x49, 0xb8, 0x65, 0x07, 0x73, 0x76,
					  0xa2, 0x85, 0x9b, 0xba, 0x5a, 0x10,
					  0x41, 0x12 };

static const uint8_t MI_CA_PUBLIC_Y[] = { 0x2a, 0x4e, 0xac, 0x7c, 0xc9, 0x61,
					  0x86, 0x28, 0x5c, 0xf1, 0xc0, 0x34,
					  0x09, 0x4d, 0x32, 0x9d, 0xa2, 0x0f,
					  0x54, 0xa3, 0x0b, 0x21, 0x5b, 0xd8,
					  0xf1, 0xf6, 0xf3, 0x17, 0x0c, 0x60,
					  0xc7, 0x8a };

/*
 * Authenticate Battery
 *
 * @return: 0=Success; negative=failure, see Error code
 */
int authenticate_battery(void)
{
	uint8_t romid[8];
	uint8_t manid[2];
	uint8_t device_pubkey[64];
	uint8_t signer[64];
	uint8_t page_data[32];
	uint8_t hrng[32];
	uint8_t tbs[128];
	uint8_t buffer[128];
	uint8_t ca_pubkey[64];

	int len;
	int index;
	int ret;

	DEBUG("%s %d battery authenticate begin\r\n", __func__, __LINE__);

	ret = slg_romid_get(romid, &len, XR_MATCH_NORMAL);
	if (ret != 0) {
		DEBUG("%s %d failed: %d\r\n", __func__, __LINE__, ret);
		return ret;
	}

	ret = slg_manid_get(manid, &len);
	if (ret != 0) {
		DEBUG("%s %d failed: %d\r\n", __func__, __LINE__, ret);
		return ret;
	}

	ret = slg_memory_read(DEVICE_PUBLIC_KEY_X, device_pubkey, &len);
	if (ret != 0) {
		DEBUG("%s %d failed: %d\r\n", __func__, __LINE__, ret);
		return ret;
	}

	ret = slg_memory_read(DEVICE_PUBLIC_KEY_Y, &device_pubkey[32], &len);
	if (ret != 0) {
		DEBUG("%s %d failed: %d\r\n", __func__, __LINE__, ret);
		return ret;
	}

	ret = slg_memory_read(DEVICE_SIGNER_R, signer, &len);
	if (ret != 0) {
		DEBUG("%s %d failed: %d\r\n", __func__, __LINE__, ret);
		return ret;
	}

	ret = slg_memory_read(DEVICE_SIGNER_S, &signer[32], &len);
	if (ret != 0) {
		DEBUG("%s %d failed: %d\r\n", __func__, __LINE__, ret);
		return ret;
	}

	memcpy(ca_pubkey, MI_CA_PUBLIC_X, 32);
	memcpy(&ca_pubkey[32], MI_CA_PUBLIC_Y, 32);

	//step 1. verify device cert
	index = 0;
	memcpy(&tbs[index], romid, sizeof(romid));
	index += sizeof(romid);

	memcpy(&tbs[index], manid, sizeof(manid));
	index += sizeof(manid);

	memcpy(&tbs[index], device_pubkey, sizeof(device_pubkey));
	index += sizeof(device_pubkey);

	ret = ecdsa_verify(ca_pubkey, tbs, index, signer);
	if (ret != 0) {
		DEBUG("%s step 1 failed: %d\r\n", __func__, ret);
		return ret;
	}

	//step 2. verify device signer
	ret = slg_memory_read(PAGE_INDEX, page_data, &len);
	if (ret != 0) {
		DEBUG("%s %d failed: %d\r\n", __func__, __LINE__, ret);
		return ret;
	}

	index = 0;
	get_random_bytes(hrng, 32);

	tbs[index++] = PAGE_INDEX;
	memcpy(&tbs[index], hrng, 32);
	index += 32;

	ret = slg_ecdsa_sign(tbs, index, buffer, &len);
	if (ret != 0) {
		DEBUG("%s %d failed: %d\r\n", __func__, __LINE__, ret);
		return ret;
	}

	index = 0;
	memcpy(&tbs[index], romid, sizeof(romid));
	index += sizeof(romid);

	memcpy(&tbs[index], page_data, sizeof(page_data));
	index += sizeof(page_data);

	tbs[index++] = PAGE_INDEX;

	memcpy(&tbs[index], manid, sizeof(manid));
	index += sizeof(manid);

	memcpy(&tbs[index], hrng, 32);

	index += 32;
	ret = ecdsa_verify(device_pubkey, tbs, index, buffer);
	if (ret != 0) {
		DEBUG("%s step 2 failed: %d\r\n", __func__, ret);
		return ret;
	}

	ret = slg_dc_decrease();
	if (ret != 0) {
		DEBUG("%s %d decrease counter decrease failed: %d\r\n",
		      __func__, __LINE__, ret);
		return ret;
	}

	DEBUG("%s %d battery authenticate success\r\n", __func__, __LINE__);

	return ret;
}
EXPORT_SYMBOL_GPL(authenticate_battery);

static ssize_t authenticator_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	ssize_t count;

	count = sprintf(buf, "%d\n", result_data.status);

	return count;
}

static int authenticator_atoi(const char *buf, size_t count, int *val, char *input)
{
	int ret = -EINVAL;

	if (count < MIN_CMD_LEN || count > MAX_CMD_LEN || (buf[LOC_X] != 'x'))
		return -EINVAL;

	if (count < SINGLE_CMD_LEN)
		ret = sscanf(buf, "0x%x", &val[LOC_CMD]);
	else if ((count < CMD_PARM_LEN) && (buf[SINGLE_CMD_LEN] == 'x'))
		ret = sscanf(buf, "0x%x 0x%x", &val[LOC_CMD], &val[LOC_PARM]);
	else if ((count < MAX_CMD_LEN) && (buf[SINGLE_CMD_LEN] == 'x'))
		ret = sscanf(buf, "0x%x 0x%x %s", &val[LOC_CMD], &val[LOC_PARM], input);
	else
		return -EINVAL;

	return ret;
}

static ssize_t authenticator_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)

{
	int cmd;
	int ret = COMMAND_ERROR;
	int val[2];
	char input[128];

	if (dev == NULL || attr == NULL || buf == NULL) {
		DEBUG("%s param error!\n", __func__);
		return COMMAND_ERROR;
	}

	slg_slave = dev_to_w1_slave(dev);

	if (!slg_slave) {
		DEBUG("w1 slg_slave is null\n");
		return PARAM_ERROR;
	}

	memset(&result_data, 0, sizeof(result_data));

	ret = authenticator_atoi(buf, count, val, input);
	if (ret < 0) {
		DEBUG("w1 slg_slave authenticator_atoi err\n");
		return COMMAND_ERROR;
	}

	cmd = val[0];
	switch (cmd) {
	case SLG_MEMORY_READ:
		ret = slg_memory_read(val[1], result_data.result_buf,
				      &result_data.result_len);
		break;

	case SLG_ECDSA_SIGN:
		ret = slg_ecdsa_sign((unsigned char *)&input, val[1],
				     result_data.result_buf,
				     &result_data.result_len);
		break;

	case SLG_DC_DECREASE:
		ret = slg_dc_decrease();
		result_data.result_len = 0;
		break;

	case SLG_DC_GET:
		ret = slg_dc_get();
		result_data.result_len = 0;
		break;

	case SLG_ROMID_GET:
		ret = slg_romid_get(result_data.result_buf,
				    &result_data.result_len, XR_MATCH_NORMAL);
		break;

	case SLG_MANDI_GET:
		ret = slg_manid_get(result_data.result_buf,
				    &result_data.result_len);
		break;

	case SLG_POWEROFF_MODE:
		ret = slg_poweroff_mode();
		result_data.result_len = 0;
		break;

	case SLG_ECDSA_DISABLE:
		ret = slg_ecdsa_disable();
		result_data.result_len = 0;
		break;

	case SLG_CHIP_KILL:
		ret = slg_chip_kill();
		result_data.result_len = 0;
		break;

	case SLG_AUTHENTICATE:
		ret = authenticate_battery();
		break;

	case SLG_MATCH_ONE:
		ret = slg_romid_get(result_data.result_buf,
				    &result_data.result_len, XR_MATCH_FORCE);
		break;

	case SLG_READ_ROM:
		ret = slg_romid_get(result_data.result_buf,
				    &result_data.result_len, XR_MATCH_READ_ROM);
		break;

	case SLG_RESUME:
		ret = slg_romid_get(result_data.result_buf,
				    &result_data.result_len, XR_MATCH_RESUME);
		break;

	case SLG_RANDOM_NUMBER_GET:
		ret = slg_random_number_read(val[1], result_data.result_buf,
				      &result_data.result_len);
		break;

	case SLG_ERR_FUNC_CMD:
		ret = slg_err_func_cmd();
		break;

	case SLG_PRESS_MODE:
		ret = slg_press_test(val[1]);
		break;

	default:
		DEBUG("w1 cmd: %02X error\n", cmd);
		break;
	}

	result_data.status = ret;

	DEBUG("w1 %s ret: %d\n", __func__, ret);

	return count;
}

static DEVICE_ATTR_RW(authenticator);

static struct attribute *authenticator_attrs[] = {
	&dev_attr_authenticator.attr,
	NULL,
};

static const struct attribute_group authenticator_group = {
	.attrs = authenticator_attrs,
};

static const struct attribute_group *authenticator_groups[] = {
	&authenticator_group,
	NULL,
};

static int authenticator_add_slave(struct w1_slave *sl)
{
	slg_slave = sl;
	return 0;
}

static void authenticator_remove_slave(struct w1_slave *sl)
{
	sl->family_data = NULL;
	slg_slave = NULL;
}

static struct w1_family_ops slg_fops = {
	.add_slave = authenticator_add_slave,
	.remove_slave = authenticator_remove_slave,
	.groups = authenticator_groups,
};

static struct w1_family w1_slg = {
	.fid = W1_FAMILY_SLG,
	.fops = &slg_fops,
};

module_w1_family(w1_slg);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("XuXiaobo <xiaobo.xu@aisinochip.com>");
MODULE_DESCRIPTION("SLG Driver");
MODULE_ALIAS("SLG");
MODULE_SOFTDEP("pre: xr1wm");
