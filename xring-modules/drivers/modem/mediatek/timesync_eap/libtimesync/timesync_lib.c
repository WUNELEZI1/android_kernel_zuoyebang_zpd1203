// SPDX-License-Identifier: GPL-2.0 OR MIT
/*
* Copyright (c) 2022 - 2023 MediaTek Inc.
*/

#include <linux/sched/signal.h>
#include <linux/delay.h>

#include "timesync_lib.h"

#if defined(CONFIG_TIMESYNC_SAP)
extern int port_timesync_write_data(const char *buf, size_t len);
extern int port_timesync_read_data(char *buf, size_t len);

#define mtk_port_search(__name__, __data__, __cnt__) (__cnt__)
#define mtk_port_open(__port__, __flag__) (0)
#define mtk_port_close(__port__)

#define mtk_port_write(__port__, __data__, __len__) port_timesync_write_data(__data__, __len__)
#define mtk_port_read(__port__, __data__, __len__) port_timesync_read_data(__data__, __len__)
#else
extern int mtk_port_search(char * name, dev_t *result, unsigned int max_cnt);
extern int mtk_port_open(dev_t devt, int flags);
extern int mtk_port_close(dev_t devt);
extern int mtk_port_write(dev_t devt, void *buf, unsigned int len);
extern int mtk_port_read(dev_t devt, void *buf, unsigned int len);
#endif

#define PORT_ERROR_RETRY        (0)

typedef struct {
    dev_t           port_id;
    unsigned int    seq_num[MSG_CHN_NR];
} PORT_CTRL_BLOCK_T;
static PORT_CTRL_BLOCK_T s_ctrl_block;

/*
 * ret < 0 for port failure
 * ret == 0 for port retry
 * ret > 0 for reservation
 */
static int port_error_handle(
    const int   errno)
{
    int ret = PORT_ERROR_RETRY;

    switch (errno) {
        case -ENODEV:
            if(port_trxn_init(PORT_TIMEOUT_INF) == 0) {
                ret = PORT_ERROR_RETRY;
            }
            else {
                ret = -EINVAL;
            }
            break;
        case -ENOMEM:
            ret = PORT_ERROR_RETRY;
            break;
        case -EINTR:
        default:
            ret = errno;
            break;
    }
    // default handling for all errno to supress error log
    msleep(PORT_POLL_MESC);

    return ret;
}

static int port_msg_write(
    const dev_t     port_id,
    const void      *ptr_data,
    unsigned int    data_len)
{
    int ret = 0;
    int rslt;

    do {
        if (signal_pending(current)) {
            ret = -EINTR;
            break;
        }
        rslt = mtk_port_write(port_id, (void *) ptr_data, data_len);
        if (rslt >= 0) {
            ptr_data += rslt;
            data_len -= rslt;
            ret += rslt;
        } else {
            ret = rslt;
            TIMESYNC_LOG_ERR("port write failed with ret=%d\n", ret);
            ret = port_error_handle(ret);
            if (ret == PORT_ERROR_RETRY) {
                continue;
            } else {
                break;
            }
        }
    } while (data_len > 0);

    return ret;
}

static int port_msg_read(
    const dev_t     port_id,
    void            *ptr_data,
    unsigned int    data_len)
{
    int ret = 0;
    int rslt;

    do {
        if (signal_pending(current)) {
            ret = -EINTR;
            break;
        }
        rslt = mtk_port_read(port_id, (void *) ptr_data, data_len);
        if (rslt >= 0) {
            ptr_data += rslt;
            data_len -= rslt;
            ret += rslt;
        } else {
            ret = rslt;
            TIMESYNC_LOG_ERR("port read failed with ret=%d w/i port_id=0x%x\n", ret, port_id);
            ret = port_error_handle(ret);
            if (ret == PORT_ERROR_RETRY) {
                continue;
            } else {
                break;
            }
        }
    } while (data_len > 0);

    return ret;
}

int port_trxn_cmd(
    const TRXN_CMD_T    trxn_cmd,
    const MSG_CHN_T     msg_chn,
    const MSG_CMD_T     msg_cmd,
    void                *ptr_data,
    unsigned int        data_len)
{
    MSG_HDR_T           msg_hdr;
    int                 ret;

    if (trxn_cmd >= TRXN_CMD_NR) {
        TIMESYNC_LOG_ERR("invalid trxn_cmd=%d\n", trxn_cmd);
        return -EINVAL;
    }
    if (msg_chn >= MSG_CHN_NR) {
        TIMESYNC_LOG_ERR("invalid msg_chn=%d\n", msg_chn);
        return -EINVAL;
    }
    if (msg_cmd >= MSG_CMD_NR) {
        TIMESYNC_LOG_ERR("invalid msg_cmd=%d\n", msg_cmd);
        return -EINVAL;
    }
    if (ptr_data == NULL || data_len == 0) {
        TIMESYNC_LOG_ERR("ptr_data=null or data_len=0\n");
        return -EFAULT;
    }

    switch (trxn_cmd) {
        case TRXN_CMD_WRITE:
            if (msg_cmd == MSG_CMD_RESET) {
                s_ctrl_block.seq_num[msg_chn] = 0;
            }
            msg_hdr.seq_num = s_ctrl_block.seq_num[msg_chn] + 1;
            msg_hdr.chn = msg_chn;
            msg_hdr.cmd = msg_cmd;
            msg_hdr.resp = (msg_cmd == MSG_CMD_ERROR) ? MSG_RESP_ERROR : MSG_RESP_SUCCESS;
            msg_hdr.data_len = data_len;

            TIMESYNC_LOG_INFO("write data with seq_num=0x%x\n", msg_hdr.seq_num);
            ret = port_msg_write(s_ctrl_block.port_id, &msg_hdr, sizeof(msg_hdr));

            if (ret >= 0) {
                TIMESYNC_LOG_INFO("write data with data_len=0x%x\n", msg_hdr.data_len);
                ret = port_msg_write(s_ctrl_block.port_id, ptr_data, data_len);
            }

            if (ret >= 0) {
                s_ctrl_block.seq_num[msg_chn] = msg_hdr.seq_num;
            }
            break;
        case TRXN_CMD_READ:
            ret = port_msg_read(s_ctrl_block.port_id, &msg_hdr, sizeof(msg_hdr));

            if (ret >= 0) {
                TIMESYNC_LOG_INFO("read data with seq_num=0x%x\n", msg_hdr.seq_num);
                if (data_len < msg_hdr.data_len) {
                    TIMESYNC_LOG_ERR("read out of memory with msg_len=%u and buf_len=%u\n", msg_hdr.data_len, data_len);
                    do {
                        ret = port_msg_read(s_ctrl_block.port_id, ptr_data, data_len);
                        msg_hdr.data_len -= ret;
                        data_len = (data_len < msg_hdr.data_len) ? data_len : msg_hdr.data_len;
                    } while ((ret >= 0) && (data_len > 0));
                    ret = -ENOMEM;
                    break;
                }
                ret = port_msg_read(s_ctrl_block.port_id, ptr_data, msg_hdr.data_len);
            }

            if (ret >= 0) {
                TIMESYNC_LOG_INFO("read data with data_len=0x%x\n", msg_hdr.data_len);
                if (msg_hdr.chn != msg_chn) {
                    TIMESYNC_LOG_ERR("invalid msg_chn=%d with chn=%d\n", msg_hdr.chn, msg_chn);
                    ret = -EPROTO;
                } else {
                    if (msg_hdr.cmd == MSG_CMD_RESET) {
                        TIMESYNC_LOG_WARN("receive remote agent reset command\n");
                        ret = -EREMCHG;
                    } else if (msg_hdr.seq_num != (s_ctrl_block.seq_num[msg_chn] + 1)) {
                        TIMESYNC_LOG_ERR("invalid incomming seq=0x%x with expected seq=0x%x in chn=0x%x\n",
                                msg_hdr.seq_num, s_ctrl_block.seq_num[msg_chn] + 1, msg_chn);
                        ret = -EPROTO;
                    } else if ((msg_hdr.cmd == MSG_CMD_ERROR) && (msg_hdr.resp == MSG_RESP_ERROR)) {
                        ret = *((int *)ptr_data);
                        TIMESYNC_LOG_ERR("error response=%d from the agent\n", ret);
                    } else if (msg_hdr.cmd != msg_cmd) {
                        TIMESYNC_LOG_ERR("invalid msg_cmd=%d with cmd=%d\n", msg_hdr.cmd, msg_cmd);
                        ret = -EPROTO;
                    } else {
                        // normal case
                    }
                    s_ctrl_block.seq_num[msg_hdr.chn] = msg_hdr.seq_num;
                }
            }
            break;
        default:
            TIMESYNC_LOG_ERR("invalid trxn_cmd=%d\n", trxn_cmd);
            ret = -ENOTSUPP;
            break;
    }

    return ret;
}

int port_trxn_init(const int timeout_msec)
{
    int ret;
    int sleep_msec = timeout_msec;
    long retry_num = 0;

    // close the port before reinit due to port busy
    port_trxn_exit();

    do {
        ret = mtk_port_search(PORT_NAME, &s_ctrl_block.port_id, PORT_MAX_NUM);

        if (ret == PORT_MAX_NUM) {
            ret = mtk_port_open(s_ctrl_block.port_id, PORT_FLAGS);
            if (ret != 0) {
                TIMESYNC_LOG_ERR("port open failed with ret=%d\n", ret);
                s_ctrl_block.port_id = PORT_NULL;
            }
        } else {
            retry_num++;
            if(( retry_num > 10 ) && ( retry_num % 10 == 0 ) && ( retry_num < 100 )) {
                TIMESYNC_LOG_WARN("port search failed with ret=%d\n", ret);
                ret = -EINVAL;
            }
            else if(retry_num > 100){
                ret = -EINVAL;
                break;
            }else {
                ret = -EINVAL;
            }
        }

        if ((ret != 0) &&
                ((timeout_msec == PORT_TIMEOUT_INF) || (sleep_msec > 0))) {
            msleep(PORT_POLL_MESC);
            sleep_msec -= PORT_POLL_MESC;
        } else {
            break;
        }
    // for rmmod kernel module to terminate the thread
    } while (!signal_pending(current));

    if (ret == 0) {
        TIMESYNC_LOG_NOTICE("port init successfully with port=0x%x\n", s_ctrl_block.port_id);
    } else {
        TIMESYNC_LOG_ERR("port init failed with ret=%d\n", ret);
    }

    return ret;
}

void port_trxn_exit(void)
{
    if (s_ctrl_block.port_id != PORT_NULL) {
        mtk_port_close(s_ctrl_block.port_id);
        s_ctrl_block.port_id = PORT_NULL;
    }
}

int port_trxn_reset(
    const unsigned int clear_queue_enable)
{
    ktime_t rest_time = ktime_get_boottime();
    int ret;

    do {
        if (signal_pending(current)) {
            ret = -EINTR;
            break;
        }
        TIMESYNC_LOG_NOTICE("writing HS TX with time=%llu ns\n", rest_time);
        ret = port_trxn_cmd(
                TRXN_CMD_WRITE,
                MSG_CHN_TIMESYNC,
                MSG_CMD_RESET,
                &rest_time, sizeof(rest_time));
        if (ret >= 0) {
            ret = 0;
        }
        TIMESYNC_LOG_NOTICE("write HS TX with ret=%d\n", ret);

        if ((ret == 0) && (clear_queue_enable == PORT_TRXN_CLEAR_QUEUE_EN)) {
            do {
                if (signal_pending(current)) {
                    ret = -EINTR;
                    break;
                }
                TIMESYNC_LOG_NOTICE("reading HS RX at time=%llu ns\n", ktime_get_boottime());
                ret = port_trxn_cmd(
                        TRXN_CMD_READ,
                        MSG_CHN_TIMESYNC,
                        MSG_CMD_RESET,
                        &rest_time, sizeof(rest_time));
            } while (ret != -EREMCHG);
            if (ret == -EREMCHG) { // receive HS reset message
                TIMESYNC_LOG_NOTICE("read HS RX with time=%llu ns\n", rest_time);
                ret = 0;
            }
        }
    } while (ret != 0);

    return ret;
}
