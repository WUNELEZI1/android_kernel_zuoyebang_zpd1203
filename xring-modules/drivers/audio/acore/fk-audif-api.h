/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _FK_AUDIF_API_
#define _FK_AUDIF_API_

#include <linux/init.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/kthread.h>

typedef void(*audif_data_cb)(void *priv);
typedef void(*recv_msg_cb)(void *msg, int len, void *priv);

static audif_data_cb ap_callback;
void *ap_cb_priv;
struct task_struct *data_thrd;
static int data_thrd_exit;

static int audif_data_trigger_thread(void *priv)
{

	AC_LOG_INFO("enter");

	for (;;) {
		if (data_thrd_exit && kthread_should_stop()) {
			AC_LOG_INFO("complete");
			break;
		}

		if (ap_callback != NULL) {
			ap_callback(ap_cb_priv);
			msleep(20);
		} else {
			msleep(100);
		}
	}

	AC_LOG_INFO("exit");
	return 0;
}

int audif_src_daap_set(bool enable, int16_t ap_src_mode)
{
	return 0;
}

int audif_daap_dg_control_st(int16_t ap_dg_l, int16_t ap_dg_r)
{
	return 0;
}

int audif_fifo_empty_watermark_ap_set(int16_t ap_empty_watermark)
{
	return 0;
}

int audif_fifo_clr_ap_enable(void)
{
	return 0;
}

int audif_aempty_ap_mask_enable(bool enable, audif_data_cb cb, void *priv)
{
	int ret = 0;

	if (enable) {
		ap_callback = cb;
		ap_cb_priv = priv;

		data_thrd_exit = 0;

		AC_LOG_INFO("run audif_data_trigger_thread");
		if (data_thrd == NULL) {
			/* create simulate callback thread */
			data_thrd = kthread_run(audif_data_trigger_thread, NULL, "audif_data%d", 1);
		}
	} else {
		if (!IS_ERR(data_thrd)) {
			AC_LOG_INFO("stop audif_data_trigger_thread");

			//data_thrd_exit = 1;

			//ret = kthread_stop(data_thrd);
			//data_thrd = NULL;

			AC_LOG_INFO("stop audif_data_trigger_thread complete. ret=%d", ret);
		}
		ap_callback = NULL;
		ap_cb_priv = NULL;
	}
	return 0;
}

int audif_fifo_full_watermark_ar_set(int16_t ar_full_watermark)
{
	return 0;
}

int audif_ar_control_mux_src_set(int16_t mux_ar, int16_t ar_src_mode)
{
	return 0;
}

int audif_ad_fifo_enable_ar(bool enable)
{
	return 0;
}

int audif_afull_ar_mask_enable(bool enable, audif_data_cb cb, void *priv)
{
	int ret = 0;

	if (enable) {
		ap_callback = cb;
		ap_cb_priv = priv;

		data_thrd_exit = 0;

		AC_LOG_INFO("run audif_data_trigger_thread");
		if (data_thrd == NULL) {
			/* create simulate callback thread */
			data_thrd = kthread_run(audif_data_trigger_thread, NULL, "audif_data%d", 1);
		}
	} else {
		if (!IS_ERR(data_thrd)) {
			AC_LOG_INFO("stop audif_data_trigger_thread");

			//data_thrd_exit = 1;

			//ret = kthread_stop(data_thrd);
			//data_thrd = NULL;

			AC_LOG_INFO("stop audif_data_trigger_thread complete. ret=%d", ret);
		}
		ap_callback = NULL;
		ap_cb_priv = NULL;
	}
	return ret;
}

int audif_ap_fifo_write_data(void *vir_addr, u64 phy_addr, size_t size)
{
	return 0;
}

int audif_ar_fifo_read_data(void *vir_addr, u64 phy_addr, size_t size)
{
	return size;
}

int fk_audio_ipc_send_sync(unsigned int *msg, unsigned int len)
{
	return 0;
}

//int fk_audio_ipc_send_async(unsigned int *msg, unsigned int len,
//	ack_callback_t cb, void *arg) {

//}

int fk_audio_ipc_recv_register(recv_msg_cb cb, void *priv)
{
	return 0;
}

int fk_audio_ipc_recv_unregister(void)
{
	return 0;
}


#endif /* _FK_AUDIF_API_ */
