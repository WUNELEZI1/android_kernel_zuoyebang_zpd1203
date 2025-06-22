/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRING_RPROC_API_H__
#define __XRING_RPROC_API_H__
#include <linux/kernel.h>
#include <linux/remoteproc.h>

typedef void (*callback_func_t)(void *priv, void *msg);

enum xrisp_rproc_msg_type {
	XRISP_RPROC_MSG_INVALED = 0,
	XRISP_RPROC_MSG_BOOT_DONE,
	XRISP_RPROC_MSG_BOOT_FAIL,
	XRISP_RPROC_MSG_KICK,
	XRISP_RPROC_SET_MUP_REGION,
	XRISP_RPROC_SET_MUP_REGION_DONE,
	XRISP_RPROC_SET_WFI,
	XRISP_RPROC_TIME_SYNC,
	XRISP_RPROC_MSG_MAX
};

int xrisp_rproc_boot(uint32_t secne);
int xrisp_rproc_shutdown(void);

int xrisp_rproc_state_get(void);
bool xrisp_rproc_is_running(void);
bool xrisp_rproc_is_sec_running(void);

int xrisp_rproc_mbox_send_sync(unsigned int *tx_buffer, unsigned int len);
void xrisp_rproc_msg_register(enum xrisp_rproc_msg_type msg_type, callback_func_t callback,
			      void *priv);
void xrisp_rproc_msg_unregister(enum xrisp_rproc_msg_type msg_type);

void xrisp_rproc_rpmsg_complete(void);
int xrisp_rpmsg_wait_complete(uint32_t times);

bool xrisp_rproc_mpu_avail(void);
void xrisp_rproc_mpu_lock(void);
void xrisp_rproc_mpu_unlock(void);
int xrisp_register_rproc_mpu_ops(void *priv, int (*start)(void *priv),
			      void (*stop)(void *priv, bool crashed));
int xrisp_unregister_rproc_mpu_ops(void);

bool xrisp_mcu_smmu_is_poweron(void);
int xrisp_mcu_smmu_power_on(void);
int xrisp_mcu_smmu_power_off(void);
int xrisp_mcu_set_rate(int rate);

int xrisp_provider_buf_ssmmu_map_all(atomic_t *stop);
void xrisp_ssmmu_unmap_all(void);

unsigned long xrisp_rproc_ocm_pa_to_da(unsigned long pa, size_t len);

/* rproc async api */
void xrisp_rproc_add_powerdown_wait_cond(uint32_t camid);
void xrisp_rproc_wake_powerdown_wait_cond(uint32_t camid);

int xrisp_rproc_update_powerdown_state(void);
bool xrisp_rproc_get_boot_recovery(void);

int xrisp_rproc_async_shutdown(bool direct_powerdown);
int xrisp_rproc_async_boot(uint32_t scene);

int xrisp_rproc_async_ctl_init(void);
void xrisp_rproc_async_ctl_exit(void);

void xrisp_rproc_async_state_release(void);

#endif /* __XRING_RPROC_API_H__ */
