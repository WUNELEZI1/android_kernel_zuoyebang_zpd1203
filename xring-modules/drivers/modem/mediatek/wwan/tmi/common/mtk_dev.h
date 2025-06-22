/* SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Copyright (c) 2022, MediaTek Inc.
 */

#ifndef __MTK_DEV_H__
#define __MTK_DEV_H__

#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#define MTK_DEV_STR_LEN 16

enum mtk_trans_dev_type {
	MTK_TRANS_DEV_CLDMA,
	MTK_TRANS_DEV_DPMAIF,
	MTK_TRANS_DEV_MAX
};

enum mtk_md_hw_type {
	MTK_MD_HW_PCI = 1,
	MTK_MD_HW_MAX
};

enum mtk_irq_src {
	MTK_IRQ_SRC_MIN,
	MTK_IRQ_SRC_MHCCIF,
	MTK_IRQ_SRC_SAP_RGU,
	MTK_IRQ_SRC_DPMAIF,
	MTK_IRQ_SRC_DPMAIF2,
	MTK_IRQ_SRC_CLDMA0,
	MTK_IRQ_SRC_CLDMA1,
	MTK_IRQ_SRC_CLDMA2,
	MTK_IRQ_SRC_CLDMA3,
	MTK_IRQ_SRC_PM_LOCK,
	MTK_IRQ_SRC_DPMAIF3,
	MTK_IRQ_SRC_AUDIO,
	MTK_IRQ_SRC_DPMAIF4,
	MTK_IRQ_SRC_MAX
};

enum mtk_user_id {
	MTK_USER_HW,
	MTK_USER_CTRL,
	MTK_USER_DPMAIF,
	MTK_USER_PM,
	MTK_USER_EXCEPT,
	MTK_USER_DEVLINK,
	MTK_USER_MAX
};

/* Software event to device */
enum mtk_h2d_sw_evt {
	H2D_SW_EVT_PM_LOCK = 0,
};

enum mtk_d2h_sw_evt {
	D2H_SW_EVT_PM_LOCK_ACK = 0,
};

enum mtk_reset_type {
	RESET_FLDR,
	RESET_PLDR,
	RESET_RGU,
	RESET_NONE,
};

enum mtk_reinit_type {
	REINIT_TYPE_RESUME,
	REINIT_TYPE_EXP,
};

enum mtk_l1ss_grp {
	L1SS_PM,
	L1SS_EXT_EVT,
	L1SS_DATA,
};

enum mtk_except_evt {
	EXCEPT_LINK_ERR,
	EXCEPT_RGU,
	EXCEPT_AER_DETECTED,
	EXCEPT_AER_RESET,
	EXCEPT_AER_RESUME,
	EXCEPT_MAX
};

enum mtk_msg_id {
	MTK_PCIMSG_H2C_READY = 1,
	MTK_PCIMSG_H2C_BAR = 2,
	MTK_PCIMSG_H2C_EXCEPT = 3,
	MTK_PCIMSG_H2C_EXCEPT_ACK = 4,
	MTK_PCIMSG_H2C_SMEM = 5,
	MTK_PCIMSG_C2H_REQ_BAR = 0XA1,
	MTK_PCIMSG_C2H_EXCEPT = 0XA2,
	MTK_PCIMSG_C2H_EXCEPT_ACK = 0XA3,
	MTK_PCIMSG_C2H_REQ_SMEM = 0XA4,
};

enum mtk_dev_log_type {
	MTK_DEV_LOG_BROM_SRAM = 0,
	MTK_DEV_LOG_BROM_SRAM_ORIGIN,
	MTK_DEV_LOG_PL_SRAM,
	MTK_DEV_LOG_PL_DRAM,
	MTK_DEV_LOG_ATF_DRAM,
	MTK_DEV_LOG_HOST_RECVED,
};

#define L1SS_BIT_L1(grp)     BIT(((grp) << 2) + 1)
#define L1SS_BIT_L1_1(grp)   BIT(((grp) << 2) + 2)
#define L1SS_BIT_L1_2(grp)   BIT(((grp) << 2) + 3)

#define PCIE_SUSPEND_CNT_MAGIC	(0x53550000)
#define PCIE_RESUME_CNT_MAGIC	(0x52450000)

#define MTK_PCIE_PORT_NUM	(1)

struct mtk_md_dev;

/**
 * struct mtk_hw_ops - The HW layer operations provided to transaction layer.
 * @read32:         Callback to read 32-bit register.
 *                  Read value from MD. For PCIe, it's BAR 2/3 MMIO read.
 * @write32:        Callback to write 32-bit register.
 *                  Write value to MD. For PCIe, it's BAR 2/3 MMIO write.
 * @get_dev_state:  Callback to get the device's state.
 * @ack_dev_state:  Callback to acknowledge device state.
 * @get_ds_status:  Callback to get device deep sleep status.
 * @ds_lock:        Callback to lock the deep sleep of device.
 * @ds_unlock:      Callback to unlock the deep sleep of device.
 * @set_l1ss:       Callback to set the link L1 and L1ss enable/disable.
 * @get_resume_state:Callback to get PM resume information that device writes.
 * @get_dev_cfg:    Callback to get device configuration.
 * @get_irq_id:     Callback to get the irq id specific IP on a chip.
 * @get_virq_id:     Callback to get the system virtual IRQ.
 * @register_irq:   Callback to register callback function to specific hardware IP.
 * @unregister_irq: Callback to unregister callback function to specific hardware IP.
 * @mask_irq:       Callback to mask the interrupt of specific hardware IP.
 * @unmask_irq:     Callback to unmask the interrupt of specific hardware IP.
 * @clear_irq:      Callback to clear the interrupt of specific hardware IP.
 * @send_sw_evt:    Callback to send HW Layer software interrupt.
 * @register_ext_evt:Callback to register HW Layer external event.
 * @unregister_ext_evt:Callback to unregister HW Layer external event.
 * @mask_ext_evt:   Callback to mask HW Layer external event.
 * @unmask_ext_evt: Callback to unmask HW Layer external event.
 * @clear_ext_evt:  Callback to clear HW Layer external event status.
 * @send_ext_evt:   Callback to send HW Layer external event.
 * @get_ext_evt_status:Callback to get HW Layer external event status.
 * @reset:          Callback to reset device.
 * @reinit:         Callback to execute device re-initialization.
 * @link_check:     Callback to execute hardware link check.
 * @get_hp_status:  Callback to get link hotplug status.
 * @dbg_dump:       Callback to execute HW Layer debug dump.
 * @write_pm_cnt:   Callback to write PM counter to notify device.
 * @get_dev_log:    Callback to get device sram/dram log.
 * @get_log_region_size: Callback to get sram/dram log region size.
 * @get_pm_req_status: Callback to get pm suspend and resume req status.
 * @get_pm_resume_user: Callback to get pm resume user.
 */
struct mtk_hw_ops {
	/* Read value from MD. For PCIe, it's BAR 2/3 MMIO read */
	u32 (*read32)(struct mtk_md_dev *mdev, u64 addr);
	/* Write value to MD. For PCIe, it's BAR 2/3 MMIO write */
	void (*write32)(struct mtk_md_dev *mdev, u64 addr, u32 val);
	/* Device operations */
	u32 (*get_dev_state)(struct mtk_md_dev *mdev);
	void (*ack_dev_state)(struct mtk_md_dev *mdev, u32 state);
	u32 (*get_ds_status)(struct mtk_md_dev *mdev);
	void (*ds_lock)(struct mtk_md_dev *mdev);
	void (*ds_unlock)(struct mtk_md_dev *mdev);
	void (*set_l1ss)(struct mtk_md_dev *mdev, u32 type, bool enable, enum mtk_l1ss_grp);
	u32 (*get_resume_state)(struct mtk_md_dev *mdev);
	u32 (*get_dev_cfg)(struct mtk_md_dev *mdev);
	/* IRQ Related operations */
	int (*get_irq_id)(struct mtk_md_dev *mdev, enum mtk_irq_src irq_src);
	int (*get_virq_id)(struct mtk_md_dev *mdev, int irq_id);
	int (*register_irq)(struct mtk_md_dev *mdev, int irq_id,
			    int (*irq_cb)(int irq_id, void *data), void *data);
	int (*unregister_irq)(struct mtk_md_dev *mdev, int irq_id);
	int (*mask_irq)(struct mtk_md_dev *mdev, int irq_id);
	int (*unmask_irq)(struct mtk_md_dev *mdev, int irq_id);
	int (*clear_irq)(struct mtk_md_dev *mdev, int irq_id);
	int (*reset_sys_irq)(struct mtk_md_dev *mdev);
	void (*send_sw_evt)(struct mtk_md_dev *mdev, enum mtk_h2d_sw_evt evt);
	void (*clear_sw_evt)(struct mtk_md_dev *mdev, enum mtk_d2h_sw_evt evt);
	/* External event related */
	int (*register_ext_evt)(struct mtk_md_dev *mdev, u32 chs,
				int (*evt_cb)(u32 status, void *data), void *data);
	int (*unregister_ext_evt)(struct mtk_md_dev *mdev, u32 chs);
	void (*mask_ext_evt)(struct mtk_md_dev *mdev, u32 chs);
	void (*unmask_ext_evt)(struct mtk_md_dev *mdev, u32 chs);
	void (*clear_ext_evt)(struct mtk_md_dev *mdev, u32 chs);
	int (*send_ext_evt)(struct mtk_md_dev *mdev, u32 ch);
	u32 (*get_ext_evt_status)(struct mtk_md_dev *mdev);
	u32 (*get_ext_traffic_cfg)(struct mtk_md_dev *mdev);

	int (*reset)(struct mtk_md_dev *mdev, enum mtk_reset_type type);
	int (*reinit)(struct mtk_md_dev *mdev, enum mtk_reinit_type type);
	bool (*link_check)(struct mtk_md_dev *mdev);
	bool (*mmio_check)(struct mtk_md_dev *mdev);
	int (*get_hp_status)(struct mtk_md_dev *mdev);
	void (*dbg_dump)(struct mtk_md_dev *mdev);
	void (*write_pm_cnt)(struct mtk_md_dev *mdev, u32 val);
	int (*get_dev_log)(struct mtk_md_dev *mdev,
			   void *buf, size_t count, enum mtk_dev_log_type type);
	int (*get_log_region_size)(struct mtk_md_dev *mdev, enum mtk_dev_log_type type);
	u32 (*get_pm_req_status)(struct mtk_md_dev *mdev);
	u32 (*get_pm_resume_user)(struct mtk_md_dev *mdev);
};

struct mtk_md_except {
	atomic_t flag;
	enum mtk_reset_type type;
	int pci_ext_irq_id;
	struct timer_list timer;
	struct delayed_work remove_rescan;
};

enum mtk_pm_flag {
	PM_F_INIT                 = 0,
	PM_F_SLEEP                = 1,
};

enum mtk_pm_resume_state {
	PM_RESUME_STATE_L3 = 0,
	PM_RESUME_STATE_L1,
	PM_RESUME_STATE_INIT,
	PM_RESUME_STATE_L1_EXCEPT,
	PM_RESUME_STATE_L2,
	PM_RESUME_STATE_L2_EXCEPT,
};

struct mtk_pm_cfg {
	u32 ds_lock_wait_timeout_ms;
	u32 suspend_wait_timeout_ms;
	u32 resume_wait_timeout_ms;
	u32 suspend_wait_timeout_sap_ms;
	u32 resume_wait_timeout_sap_ms;
	u32 ds_lock_polling_max_us;
	u32 ds_lock_polling_min_us;
	u32 ds_lock_polling_interval_us;
	u32 ds_lock_check_bitmask;
	u32 ds_lock_check_val;
	unsigned short runtime_idle_delay;
};

enum mtk_md_pm_flag {
	PM_RESUME_FROM_L3	= 0,
	PM_NEED_SUSPEND_SAP	= 1,
};

struct mtk_md_pm {
	struct list_head entities;
	/* entity_mtx is to protect concurrently
	 * read or write of pm entity list.
	 */
	struct mutex entity_mtx;
	int irq_id;
	u32 ext_evt_chs;
	unsigned long state;

	/* ds_spinlock is to protect concurrently
	 * ds lock or unlock procedure.
	 */
	spinlock_t ds_spinlock;
	struct completion ds_lock_complete;
	atomic_t ds_lock_refcnt;
	struct delayed_work ds_unlock_work;
	u64 ds_lock_sent;
	u64 ds_lock_recv;

	struct completion pm_ack;
	struct completion pm_ack_sap;
	struct delayed_work resume_work;
	/* suspend counter for debug purpose. */
	u32 suspend_cnt;
	u32 resume_cnt;

	unsigned long md_pm_flag;
	struct mtk_pm_cfg cfg;
};

enum mtk_pm_entity_flag {
	PM_SUSPEND_DONE = 1,
	PM_SUSPEND_LATE_DONE = 2,
};

struct mtk_pm_entity {
	struct list_head entry;
	enum mtk_user_id user;
	void *param;
	unsigned long flag;

	int (*suspend)(struct mtk_md_dev *mdev, void *param, bool is_runtime);
	int (*suspend_late)(struct mtk_md_dev *mdev, void *param, bool is_runtime);
	int (*resume_early)(struct mtk_md_dev *mdev, void *param, bool is_runtime,
			    bool link_ready);
	int (*resume)(struct mtk_md_dev *mdev, void *param, bool is_runtime, bool link_ready);
};

/* mtk_md_dev defines the structure of MTK modem device */
struct mtk_md_dev {
	struct device *dev;
	const struct mtk_hw_ops *hw_ops; /* The operations provided by hw layer */
	void *hw_priv;
	u32 hw_ver;
	unsigned long long irq_timestamp;
	int msi_nvecs;
	char dev_str[MTK_DEV_STR_LEN];

	struct mtk_md_fsm *fsm;
	struct mtk_md_pm pm;
	void *ctrl_blk;
	void *data_blk;
	void *devlink;
	void *messenger;
	struct dentry *dev_dentry; /* For debug */
	void *memlog;
	void *dev_log_buff;

	struct mtk_bm_ctrl *bm_ctrl;
	struct mtk_md_except except;
};

int mtk_dev_init(struct mtk_md_dev *mdev);
void mtk_dev_exit(struct mtk_md_dev *mdev);
int mtk_dev_start(struct mtk_md_dev *mdev);
void mtk_dev_dump(struct mtk_md_dev *mdev);

int mtk_pm_init(struct mtk_md_dev *mdev);
int mtk_pm_exit(struct mtk_md_dev *mdev);
int mtk_pm_entity_register(struct mtk_md_dev *mdev, struct mtk_pm_entity *md_entity);
int mtk_pm_entity_unregister(struct mtk_md_dev *mdev, struct mtk_pm_entity *md_entity);
void mtk_pm_ds_lock(struct mtk_md_dev *mdev, enum mtk_user_id user);
int mtk_pm_ds_try_lock(struct mtk_md_dev *mdev, enum mtk_user_id user);
void mtk_pm_ds_unlock(struct mtk_md_dev *mdev, enum mtk_user_id user, bool no_wait);
int mtk_pm_ds_wait_complete(struct mtk_md_dev *mdev, enum mtk_user_id user);
int mtk_pm_ds_try_wait_complete(struct mtk_md_dev *mdev, enum mtk_user_id user);
int mtk_pm_exit_early(struct mtk_md_dev *mdev);
bool mtk_pm_check_dev_reset(struct mtk_md_dev *mdev);

int mtk_pm_runtime_idle(struct device *dev);
int mtk_pm_runtime_suspend(struct device *dev);
int mtk_pm_runtime_resume(struct device *dev, bool atr_init);
int mtk_pm_prepare(struct device *dev);
int mtk_pm_suspend(struct device *dev);
int mtk_pm_resume(struct device *dev, bool atr_init);
int mtk_pm_suspend_noirq(struct device *dev);
int mtk_pm_resume_noirq(struct device *dev);
int mtk_pm_freeze(struct device *dev);
int mtk_pm_freeze_noirq(struct device *dev);
int mtk_pm_thaw(struct device *dev, bool atr_init);
int mtk_pm_thaw_noirq(struct device *dev);
int mtk_pm_poweroff(struct device *dev);
int mtk_pm_restore(struct device *dev, bool atr_init);
void mtk_pm_shutdown(struct mtk_md_dev *mdev);

/**
 * mtk_hw_read32() - Read dword from register.
 * @mdev: Device instance.
 * @addr: Register address.
 *
 * Return: Dword register value.
 */
static inline u32 mtk_hw_read32(struct mtk_md_dev *mdev, u64 addr)
{
	return mdev->hw_ops->read32(mdev, addr);
}

/**
 * mtk_hw_write32() - Write dword to register.
 * @mdev: Device instance.
 * @addr: Register address.
 * @val: Dword to be written.
 */
static inline void mtk_hw_write32(struct mtk_md_dev *mdev, u64 addr, u32 val)
{
	mdev->hw_ops->write32(mdev, addr, val);
}

/**
 * mtk_hw_get_dev_state() - Get device's state register.
 * @mdev: Device instance.
 *
 * Return: The value of state register.
 */
static inline u32 mtk_hw_get_dev_state(struct mtk_md_dev *mdev)
{
	return mdev->hw_ops->get_dev_state(mdev);
}

/**
 * mtk_hw_ack_dev_state() - Write state to device's state register.
 * @mdev: Device instance.
 * @state: The state value to be written.
 */
static inline void mtk_hw_ack_dev_state(struct mtk_md_dev *mdev, u32 state)
{
	mdev->hw_ops->ack_dev_state(mdev, state);
}

/* mtk_hw_get_ds_status() -Get device's deep sleep status.
 *
 * @mdev: Device instance.
 *
 * Return: The value of deep sleep register.
 */
static inline u32 mtk_hw_get_ds_status(struct mtk_md_dev *mdev)
{
	return mdev->hw_ops->get_ds_status(mdev);
}

/* mtk_hw_ds_lock() -Prevent the device from entering deep sleep.
 *
 * @mdev: Device instance.
 */
static inline void mtk_hw_ds_lock(struct mtk_md_dev *mdev)
{
	mdev->hw_ops->ds_lock(mdev);
}

/* mtk_hw_ds_unlock() -Allow the device from entering deep sleep.
 *
 * @mdev: Device instance.
 */
static inline void mtk_hw_ds_unlock(struct mtk_md_dev *mdev)
{
	mdev->hw_ops->ds_unlock(mdev);
}

/* mtk_hw_set_l1ss() -Enable or disable l1ss.
 *
 * @mdev: Device instance.
 * @type: Select the sub-function of l1ss by bit,
 *        please see "enum mtk_l1ss_grp" and "L1SS_BIT_L1", "L1SS_BIT_L1_1", "L1SS_BIT_L1_2".
 * @enable: Input true or false.
 * @user: enum mtk_l1ss_grp.
 */
static inline void mtk_hw_set_l1ss(struct mtk_md_dev *mdev, u32 type, bool enable,
				   enum mtk_l1ss_grp user)
{
	mdev->hw_ops->set_l1ss(mdev, type, enable, user);
}

/* mtk_hw_get_resume_state() -Get device resume status.
 *
 * @mdev: Device instance.
 *
 * Return: The resume state of device.
 */
static inline u32 mtk_hw_get_resume_state(struct mtk_md_dev *mdev)
{
	return mdev->hw_ops->get_resume_state(mdev);
}

/* mtk_hw_get_dev_cfg() -Get device configuration.
 *
 * @mdev: Device instance.
 *
 * Return: The configuration of device.
 */
static inline u32 mtk_hw_get_dev_cfg(struct mtk_md_dev *mdev)
{
	return mdev->hw_ops->get_dev_cfg(mdev);
}

/**
 * mtk_hw_get_irq_id() - Get hardware irq_id by virtual irq_src.
 * @mdev: Device instance.
 * @irq_src: Virtual irq source number.
 *
 * Return:
 * * a negative value - indicates failure.
 * * other values - are valid irq_id.
 */
static inline int mtk_hw_get_irq_id(struct mtk_md_dev *mdev, enum mtk_irq_src irq_src)
{
	return mdev->hw_ops->get_irq_id(mdev, irq_src);
}

/**
 * mtk_hw_get_virq_id() - Get system virtual IRQ  by hardware irq_id.
 * @mdev: Device instance.
 * @irq_id: Interrupt source id.
 *
 * Return:  System virtual IRQ.
 */
static inline int mtk_hw_get_virq_id(struct mtk_md_dev *mdev, int irq_id)
{
	return mdev->hw_ops->get_virq_id(mdev, irq_id);
}

/**
 * mtk_hw_register_irq() - Register the interrupt callback to irq_id.
 * @mdev: Device instance.
 * @irq_id: Hardware irq id.
 * @irq_cb: The interrupt callback.
 * @data: Private data for callback.
 *
 * Return:
 * * 0 - indicates success.
 * * other value - indicates failure.
 */
static inline int mtk_hw_register_irq(struct mtk_md_dev *mdev, int irq_id,
				      int (*irq_cb)(int irq_id, void *data), void *data)
{
	return mdev->hw_ops->register_irq(mdev, irq_id, irq_cb, data);
}

/**
 * mtk_hw_unregister_irq() - Unregister the interrupt callback to irq_id.
 * @mdev: Device instance.
 * @irq_id: Hardware irq id.
 *
 * Return:
 * * 0 - indicates success.
 * * other value - indicates failure.
 */
static inline int mtk_hw_unregister_irq(struct mtk_md_dev *mdev, int irq_id)
{
	return mdev->hw_ops->unregister_irq(mdev, irq_id);
}

/**
 * mtk_hw_mask_irq() - Mask interrupt.
 * @mdev: Device instance.
 * @irq_id: Hardware irq id.
 *
 * Return:
 * * 0 - indicates success.
 * * other value - indicates failure.
 */
static inline int mtk_hw_mask_irq(struct mtk_md_dev *mdev, int irq_id)
{
	return mdev->hw_ops->mask_irq(mdev, irq_id);
}

/**
 * mtk_hw_unmask_irq() - Unmask interrupt.
 * @mdev: Device instance.
 * @irq_id: Hardware irq id.
 *
 * Return:
 * * 0 - indicates success.
 * * other value - indicates failure.
 */
static inline int mtk_hw_unmask_irq(struct mtk_md_dev *mdev, int irq_id)
{
	return mdev->hw_ops->unmask_irq(mdev, irq_id);
}

/**
 * mtk_hw_clear_irq() - Clear interrupt.
 * @mdev: Device instance.
 * @irq_id: Hardware irq id.
 *
 * Return:
 * * 0 - indicates success.
 * * other value - indicates failure.
 */
static inline int mtk_hw_clear_irq(struct mtk_md_dev *mdev, int irq_id)
{
	return mdev->hw_ops->clear_irq(mdev, irq_id);
}

/**
 * mtk_hw_reset_sys_irq() - reset system irq, including rgu and mhccif.
 * @mdev: Device instance.
 *
 * Return:
 * * 0 - indicates success.
 * * other value - indicates failure.
 */
static inline int mtk_hw_reset_sys_irq(struct mtk_md_dev *mdev)
{
	return mdev->hw_ops->reset_sys_irq(mdev);
}

/**
 * mtk_hw_send_sw_evt() -Send software event interrupt to device.
 *
 * @mdev: Device instance.
 * @evt: Software event to device.
 */
static inline void mtk_hw_send_sw_evt(struct mtk_md_dev *mdev, enum mtk_h2d_sw_evt evt)
{
	mdev->hw_ops->send_sw_evt(mdev, evt);
}

/* mtk_hw_clear_sw_evt() -Clear software event.
 *
 * @mdev: Device instance.
 * @evt: Software event to clear.
 */
static inline void mtk_hw_clear_sw_evt(struct mtk_md_dev *mdev, enum mtk_d2h_sw_evt evt)
{
	mdev->hw_ops->clear_sw_evt(mdev, evt);
}

/**
 * mtk_hw_register_ext_evt() - Register callback to external events.
 * @mdev: Device instance.
 * @chs: External event channels.
 * @evt_cb: External events callback.
 * @data: Private data for callback.
 *
 * Return:
 * * 0 - indicates success.
 * * other value - indicates failure.
 */
static inline int mtk_hw_register_ext_evt(struct mtk_md_dev *mdev, u32 chs,
					  int (*evt_cb)(u32 status, void *data), void *data)
{
	return mdev->hw_ops->register_ext_evt(mdev, chs, evt_cb, data);
}

/**
 * mtk_hw_unregister_ext_evt() - Unregister callback to external events.
 * @mdev: Device instance.
 * @chs: External event channels.
 *
 * Return:
 * * 0 - indicates success.
 * * other value - indicates failure.
 */
static inline int mtk_hw_unregister_ext_evt(struct mtk_md_dev *mdev, u32 chs)
{
	return mdev->hw_ops->unregister_ext_evt(mdev, chs);
}

/**
 * mtk_hw_mask_ext_evt() - Mask external events.
 * @mdev: Device instance.
 * @chs: External event channels.
 */
static inline void mtk_hw_mask_ext_evt(struct mtk_md_dev *mdev, u32 chs)
{
	mdev->hw_ops->mask_ext_evt(mdev, chs);
}

/**
 * mtk_hw_unmask_ext_evt() - Unmask external events.
 * @mdev: Device instance.
 * @chs: External event channels.
 */
static inline void mtk_hw_unmask_ext_evt(struct mtk_md_dev *mdev, u32 chs)
{
	mdev->hw_ops->unmask_ext_evt(mdev, chs);
}

/**
 * mtk_hw_clear_ext_evt() - Clear external events.
 * @mdev: Device instance.
 * @chs: External event channels.
 */
static inline void mtk_hw_clear_ext_evt(struct mtk_md_dev *mdev, u32 chs)
{
	mdev->hw_ops->clear_ext_evt(mdev, chs);
}

/**
 * mtk_hw_send_ext_evt() - Send external event to device.
 * @mdev: Device instance.
 * @ch: External event channel, only allow one channel at a time.
 *
 * Return:
 * * 0 - indicates success.
 * * other value - indicates failure.
 */
static inline int mtk_hw_send_ext_evt(struct mtk_md_dev *mdev, u32 ch)
{
	return mdev->hw_ops->send_ext_evt(mdev, ch);
}

/**
 * mtk_hw_get_ext_evt_status() - Get external event status of device.
 * @mdev: Device instance.
 *
 * Return: External event status of device.
 */
static inline u32 mtk_hw_get_ext_evt_status(struct mtk_md_dev *mdev)
{
	return mdev->hw_ops->get_ext_evt_status(mdev);
}

/**
 * mtk_hw_get_ext_traffic_cfg() - Get traffic config
 * @mdev: Device instance.
 *
 * Return: tras value.
 */
static inline u32 mtk_hw_get_ext_traffic_cfg(struct mtk_md_dev *mdev)
{
	return mdev->hw_ops->get_ext_traffic_cfg(mdev);
}

/* mtk_hw_reset() -Reset device.
 *
 * @mdev: Device instance.
 * @type: Reset type.
 *
 * Return: 0 indicates success, other value indicates failure.
 */
static inline int mtk_hw_reset(struct mtk_md_dev *mdev, enum mtk_reset_type type)
{
	return mdev->hw_ops->reset(mdev, type);
}

/* mtk_hw_reinit() -Reinitialize device.
 *
 * @mdev: Device instance.
 * @type: Reinit type.
 *
 * Return: 0 indicates success, other value indicates failure.
 */
static inline int mtk_hw_reinit(struct mtk_md_dev *mdev, enum mtk_reinit_type type)
{
	return mdev->hw_ops->reinit(mdev, type);
}

/* mtk_hw_link_check() -Check if the link is down.
 *
 * @mdev: Device instance.
 *
 * Return: 0 indicates link normally, other value indicates link down.
 */
static inline bool mtk_hw_link_check(struct mtk_md_dev *mdev)
{
	return mdev->hw_ops->link_check(mdev);
}

/**
 * mtk_hw_mmio_check() - Check if the PCIe MMIO is ready.
 * @mdev: Device instance.
 *
 * Return:
 * * 0 - indicates PCIe MMIO is ready.
 * * other value - indicates not ready.
 */
static inline bool mtk_hw_mmio_check(struct mtk_md_dev *mdev)
{
	return mdev->hw_ops->mmio_check(mdev);
}

/**
 * mtk_hw_get_hp_status() - Get whether the device can be hot-plugged.
 * @mdev: Device instance.
 *
 * Return:
 * * 0 - indicates can't.
 * * other value - indicates can.
 */
static inline int mtk_hw_get_hp_status(struct mtk_md_dev *mdev)
{
	return mdev->hw_ops->get_hp_status(mdev);
}

/**
 * mtk_hw_pm_dump() -Help power management feature to dump info when meeting error.
 *
 * @mdev: Device instance.
 */
static inline void mtk_hw_dbg_dump(struct mtk_md_dev *mdev)
{
	mdev->hw_ops->dbg_dump(mdev);
}

/**
 * mtk_hw_write_pm_cnt() - Write PM counter for device to debug.
 *
 * @mdev: Device instance.
 * @val:  The value that host driver wants to write.
 * @flag: Type of pm cnt.
 */
static inline void mtk_hw_write_pm_cnt(struct mtk_md_dev *mdev, u32 val, bool flag)
{
	if (flag)
		mdev->hw_ops->write_pm_cnt(mdev, PCIE_SUSPEND_CNT_MAGIC | val);
	else
		mdev->hw_ops->write_pm_cnt(mdev, PCIE_RESUME_CNT_MAGIC | val);
}

/**
 * mtk_hw_get_pm_req_status() - Get PM req callback status to debug.
 *
 * @mdev: Device instance.
 *
 * Return: Dword register value.
 */

static inline u32 mtk_hw_get_pm_req_status(struct mtk_md_dev *mdev)
{
	return mdev->hw_ops->get_pm_req_status(mdev);
}

/**
 * mtk_hw_get_pm_resume_user() - Get dev PM resume user to debug.
 *
 * @mdev: Device instance.
 *
 * Return: Dword register value.
 */

static inline u32 mtk_hw_get_pm_resume_user(struct mtk_md_dev *mdev)
{
	return mdev->hw_ops->get_pm_resume_user(mdev);
}

/**
 * mtk_hw_get_dev_log() -get device sram/dram log.
 *
 * @mdev: Device instance.
 * @buf:  The buffer pointer to store the device log data.
 * @count: Indicate how many bytes to read.
 * @type: Indicate which type log data to get.
 *
 * Return: A negative means failture,
 *         otherwise is the actual length of log data read from device.
 */
static inline int mtk_hw_get_dev_log(struct mtk_md_dev *mdev,
				     void *buf, size_t count, enum mtk_dev_log_type type)
{
	return mdev->hw_ops->get_dev_log(mdev, buf, count, type);
}

/**
 * mtk_hw_get_log_region_size() -get the size of sram/dram log region.
 *
 * @mdev: Device instance.
 * @type: Indicate which type log region to query.
 *
 * Return: A negative means failture,
 *         otherwise is the actual length of log data region.
 */
static inline int mtk_hw_get_log_region_size(struct mtk_md_dev *mdev, enum mtk_dev_log_type type)
{
	return mdev->hw_ops->get_log_region_size(mdev, type);
}

/**
 * mtk_except_report_evt() - Report exception event.
 * @mdev: pointer to mtk_md_dev
 * @evt: exception event
 *
 * Return:
 * *  0 - OK
 * *  -EFAULT - exception feature is not ready
 */
int mtk_except_report_evt(struct mtk_md_dev *mdev, enum mtk_except_evt evt);

/**
 * mtk_except_start() - Start exception service.
 * @mdev: pointer to mtk_md_dev
 *
 * Return: void
 */
void mtk_except_start(struct mtk_md_dev *mdev);

/**
 * mtk_except_stop() - Stop exception service.
 * @mdev: pointer to mtk_md_dev
 *
 * Return: void
 */
void mtk_except_stop(struct mtk_md_dev *mdev);

/**
 * mtk_except_init() - Initialize exception feature.
 * @mdev: pointer to mtk_md_dev
 *
 * Return:
 * *  0 - OK
 */
int mtk_except_init(struct mtk_md_dev *mdev);

/**
 * mtk_except_exit() - De-Initialize exception feature.
 * @mdev: pointer to mtk_md_dev
 *
 * Return:
 * *  0 - OK
 */
int mtk_except_exit(struct mtk_md_dev *mdev);

/**
 * mtk_data_init() - Init the data path.
 *
 * @mdev: Device instance.
 *
 * Return:
 * * 0 - OK
 * * others - Fail to initialize data plane.
 */
int mtk_data_init(struct mtk_md_dev *mdev);
int mtk_data_exit(struct mtk_md_dev *mdev);
int mtk_data_t700_init(struct mtk_md_dev *mdev);
int mtk_data_t700_exit(struct mtk_md_dev *mdev);
int mtk_data_t800_init(struct mtk_md_dev *mdev);
int mtk_data_t800_exit(struct mtk_md_dev *mdev);

int mtk_pcimsg_messenger_init(struct mtk_md_dev *mdev);
int mtk_pcimsg_messenger_exit(struct mtk_md_dev *mdev);
int mtk_pcimsg_send_msg_to_user(struct mtk_md_dev *mdev, int msg_id);
bool mtk_pcimsg_pci_user_is_busy(struct mtk_md_dev *mdev);
#endif /* __MTK_DEV_H__ */
