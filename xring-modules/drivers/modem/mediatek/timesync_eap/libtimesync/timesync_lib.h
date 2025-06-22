// SPDX-License-Identifier: GPL-2.0 OR MIT
/*
* Copyright (c) 2022 - 2023 MediaTek Inc.
*/

#ifndef __TIMESYNC_LIB_H__
#define __TIMESYNC_LIB_H__
#include <linux/ktime.h>
#include <linux/types.h>

#define TIMEOUT_MSEC        (5000)

#define PORT_NAME           ("TimeSync")
#define PORT_FLAGS          (0)
#define PORT_MAX_NUM        (1)
#define PORT_NULL           (0)
#define PORT_POLL_MESC      (1000)
#define PORT_TIMEOUT_INF    (-1)
#define PORT_TIMEOUT_IMMED  (0)
#define PORT_TRXN_CLEAR_QUEUE_EN    (1)
#define PORT_TRXN_CLEAR_QUEUE_DIS   (0)

#define TIMESYNC_LOG_INFO(fmt, args...) pr_info("[%s:%s:%d][INFO]: " fmt, KBUILD_MODNAME, __func__, __LINE__, ##args)
#define TIMESYNC_LOG_NOTICE(fmt, args...) pr_notice("[%s:%s:%d][NOTICE]: " fmt, KBUILD_MODNAME, __func__, __LINE__, ##args)
#define TIMESYNC_LOG_WARN(fmt, args...) pr_notice("[%s:%s:%d][WARN]: " fmt, KBUILD_MODNAME, __func__, __LINE__, ##args)
#define TIMESYNC_LOG_ERR(fmt, args...) pr_notice("[%s:%s:%d][ERR]: " fmt, KBUILD_MODNAME, __func__, __LINE__, ##args)

typedef struct {
    ktime_t req_sap_send_boottime;
    ktime_t req_eap_recv_boottime;
    ktime_t intr_sap_send_boottime;
    ktime_t intr_eap_recv_boottime;
    ktime_t resp_eap_send_boottime;
    ktime_t resp_sap_recv_boottime;
    unsigned int session_id;
} TIMESYNC_TIMESTAMPS_T;

typedef enum {
    TRXN_CMD_WRITE = 0,
    TRXN_CMD_READ,
    TRXN_CMD_NR
} TRXN_CMD_T;

typedef enum {
    MSG_CHN_TIMESYNC = 0,
    MSG_CHN_NR
} MSG_CHN_T;

typedef enum {
    MSG_CMD_REQ = 0,
    MSG_CMD_RESP,
    MSG_CMD_ERROR,
    MSG_CMD_RESET,
    MSG_CMD_NR
} MSG_CMD_T;

typedef enum {
    MSG_RESP_SUCCESS = 0,
    MSG_RESP_ERROR,
    MSG_RESP_NR
} MSG_RESP_T;

typedef struct {
    unsigned int    seq_num;
    unsigned int    data_len;
    MSG_CHN_T       chn;
    MSG_CMD_T       cmd;
    MSG_RESP_T      resp;
} MSG_HDR_T;

/*
 * timeout_msec < 0 for waiting infinitely
 * timeout_msec == 0 for no wait
 * timeout_msec > 0 for waiting with timeout in msec
 */
int port_trxn_init(const int timeout_msec);
void port_trxn_exit(void);

int port_trxn_cmd(
    const TRXN_CMD_T    trxn_cmd,
    const MSG_CHN_T     msg_chn,
    const MSG_CMD_T     msg_cmd,
    void                *ptr_data,
    unsigned int        data_len);

int port_trxn_reset(
    const unsigned int clear_queue_enable);
#endif
