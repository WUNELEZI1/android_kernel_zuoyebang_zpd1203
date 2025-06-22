// SPDX-License-Identifier: GPL-2.0 only.
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/syscore_ops.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include "mtk_pwrctl_common.h"
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#ifdef CONFIG_WWAN_GPIO_PWRCTL_UT
#include "ut_pwrctl_common_fake.h"
#endif

#define PWRCTL_DRIVER_NAME		"wwan_gpio_pwrctl"
#define PWRCTL_DEV_NODE_NAME		"wwan_pwrctl"
#define PWRCTL_UEVENT_BUF_MAXLEN	(128)

#ifndef FPGA_MODEM_TEST
#define FPGA_MODEM_TEST
#endif
#ifdef FPGA_MODEM_TEST
extern void xiaomi_modem_power(void);
#endif

struct pwrctl_mdev *mdev = NULL;
/* For cover L2/L3 test */
static bool pwrctl_l3_support = 0;
/* For manufactory test */
static __attribute__ ((weakref("mtk_pwrctl_slt_gpio_init"))) \
int mtk_pwrctl_slt_init(struct pwrctl_mdev *mdev);
static __attribute__ ((weakref("mtk_pwrctl_slt_gpio_uninit"))) \
void mtk_pwrctl_slt_uninit(struct pwrctl_mdev *mdev);

static bool is_t800_hw = false;
static bool is_registed_irq = false;

void mtk_pwrctl_set_power_state(struct pwrctl_mdev *mdev,enum pwrctl_state to_state)
{
	mdev->pm.state = to_state;
	dev_info(&mdev->pdev->dev, "The power state switch to %d\n", to_state);
}
int mtk_pwrctl_get_power_state(struct pwrctl_mdev *mdev)
{
	return mdev->pm.state;
}
int mtk_pwrctl_uevent_notify_user(const char * euvent_info)
{
	struct device *dev = &mdev->pdev->dev;
	char buf[PWRCTL_UEVENT_BUF_MAXLEN];
	char *envp_ext[2];

	memset(buf, 0, sizeof(buf));
	snprintf(buf, PWRCTL_UEVENT_BUF_MAXLEN, "info=%s", euvent_info);
	envp_ext[0] = buf;
	envp_ext[1] = NULL;
	kobject_uevent_env(&dev->kobj, KOBJ_CHANGE, envp_ext);
	dev_info(dev, "Uevent notify: %s\n", envp_ext[0]);
	return 0;
}

int mtk_pwrctl_cb_notify_user(enum pwrctl_evt evt)
{
	struct pwrctl_evt_cb *evt_cb;

	list_for_each_entry(evt_cb, &mdev->notifier.cb_list, entry) {
		if (evt_cb->cb)
			evt_cb->cb(evt,evt_cb->priv);
	}
	dev_info(&mdev->pdev->dev, "cb notify done\n");
	return 0;
}

static int mtk_pwrctl_fops_open(struct inode *inode, struct file *filep)
{
	if (atomic_read(&mdev->port.usage_cnt) > 0) {
		dev_err(&mdev->pdev->dev, "Port [%s] is busy\n", PWRCTL_DEV_NODE_NAME);
		return -EBUSY;
	}

	atomic_inc(&mdev->port.usage_cnt);
	dev_info(&mdev->pdev->dev, "Open port [%s] success\n", PWRCTL_DEV_NODE_NAME);
	return 0;
}

static int mtk_pwrctl_fops_close(struct inode *inode, struct file *filep)
{
	atomic_set(&mdev->port.usage_cnt, 0);
	dev_info(&mdev->pdev->dev, "Close port [%s]\n", PWRCTL_DEV_NODE_NAME);
	return 0;
}

static long mtk_pwrctl_fops_ioctl(struct file *filep, unsigned int ioctl_cmd, unsigned long arg)
{
	return mtk_pwrctl_cmd_process(mdev, ioctl_cmd);
}

static const struct file_operations mtk_pwrctl_fops = {
	.owner = THIS_MODULE,
	.open = mtk_pwrctl_fops_open,
	.release = mtk_pwrctl_fops_close,
	.unlocked_ioctl = mtk_pwrctl_fops_ioctl,
};

static struct miscdevice mtk_pwrctl_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = PWRCTL_DEV_NODE_NAME,
	.fops = &mtk_pwrctl_fops,
};

/***********************************xiaomi modem driver optimize API start***/
typedef ssize_t (*XiaomiModemParaFuncPtr)(struct file *, const char __user *, size_t, loff_t *);
XiaomiModemParaFuncPtr g_XiaomiModemParaFuncPtr;
typedef ssize_t (*XiaomiModemParaReadFuncPtr)(struct file *file, char __user *buf, size_t size, loff_t *ppos);
XiaomiModemParaReadFuncPtr g_XiaomiModemParaReadFuncPtr;
#define MODEM_OPTIMIZE_PARA_COUNT_MAX 100
int mtk_pwrctl_set_para_cb(XiaomiModemParaFuncPtr func, XiaomiModemParaReadFuncPtr readFunc)
{
	g_XiaomiModemParaFuncPtr = func;
	g_XiaomiModemParaReadFuncPtr = readFunc;
	return 0;
}
EXPORT_SYMBOL(mtk_pwrctl_set_para_cb);
int mtk_pwrctl_remove_para_cb(XiaomiModemParaFuncPtr func, XiaomiModemParaReadFuncPtr readFunc)
{
	g_XiaomiModemParaFuncPtr = NULL;
	g_XiaomiModemParaReadFuncPtr = NULL;
	return 0;
}
EXPORT_SYMBOL(mtk_pwrctl_remove_para_cb);

/***********************************************************************
* t800_send_uevent() - send  uevent to thinmd
 * @id: mtk_uevent_id
    @sub_id:
    value1
***********************************************************************/
void t800_send_platform_uevent(int id, int sub_id, int value1, int value2, int value3)
{
	struct device *dev = &mdev->pdev->dev;
	char *envp_ext[2];
	char uevent_info[PWRCTL_UEVENT_BUF_MAXLEN];
	memset(uevent_info, 0, sizeof(uevent_info));
	snprintf(uevent_info, PWRCTL_UEVENT_BUF_MAXLEN,
		"0000:01:00.0:event_id=%d, info=sub_id=0x%x, value1=0x%x, value2=0x%x, value3=0x%x", id, sub_id, value1, value2, value3);

	envp_ext[0] = uevent_info;
	envp_ext[1] = NULL;
	kobject_uevent_env(&dev->kobj, KOBJ_CHANGE, envp_ext);
	dev_info(dev, "Uevent notify: %s\n", envp_ext[0]);
}
EXPORT_SYMBOL(t800_send_platform_uevent);
/* modem_drv_para_set() - set optimization parameters
 * @buffer : parameter ID(one byte) + parameter value (one byte)
 * Return: return value is 0 on success, a negative error code on failure.
 */
 //int static test_count = 0;
ssize_t xiaomi_modem_drv_para_set(struct file *file, const char __user *buffer, size_t count, loff_t *ops)
{
	char para[MODEM_OPTIMIZE_PARA_COUNT_MAX];
	//pr_err("%s   input para count: %lu,%d", __func__,count,test_count);
	//test_count++;
	if (count > MODEM_OPTIMIZE_PARA_COUNT_MAX) {
		pr_err("%s  failed.  unsupport count: %lu, max:%d", __func__, count, MODEM_OPTIMIZE_PARA_COUNT_MAX);
		return -1;
	}
	if (copy_from_user((void *)para, (const void __user *)buffer, count))
		return -EFAULT;
	pr_info("%s  id= %x, para=%x\n", __func__, para[0], para[1]);
	if (para[0] == (char)7/*MODEM_PARA_ID_DUMP*/) {
		t800_send_platform_uevent(8/*MTK_UEVENT_MODEM_DUMP_LEVEL*/, 1, (int)para[1], 0, 0);
		//return count; //do not return as pcie need process dump level
	} else if (para[0] == (char)9) {
		t800_send_platform_uevent(6/*MTK_UEVENT_THINMD_CMD*/, (int)para[1],/*1: power off; 2: power on*/ 0, 0, 0);
		return count;
	} else if (para[0] == (char)10/*MODEM_PARA_ID_NVRECOVERY_REQ*/) {
		t800_send_platform_uevent(6/*MTK_UEVENT_THINMD_CMD*/, 3/*nv recovery*/, 0, 0, 0);
		return count;
	} else if (para[0] == (char)11/*MODEM_PARA_ID_ME_STATE_REQ*/) {
		t800_send_platform_uevent(6/*MTK_UEVENT_THINMD_CMD*/, 4/*phone state*/, (int)para[1], 0, 0);
		return count;
	} else if (para[0] == (char)14/*MODEM_PARA_ID_TMI_LOG*/) {
		t800_send_platform_uevent(6/*MTK_UEVENT_THINMD_CMD*/, 5/*tmi log*/, 0, 0, 0);
		return count;
	}  else if (para[0] == (char)17/*MODEM_PARA_ID_PANIC*/) {
		char *str = NULL;
		pr_info("%s recovery fail=%x\n", __func__, para[1]);
		*str = 1; 
		return count;
	}
	if (g_XiaomiModemParaFuncPtr) {//req is proccessed in pcie host
		g_XiaomiModemParaFuncPtr(file, buffer, count, ops);
	}
	return count;
}

static ssize_t xiaomi_modem_drv_para_read(struct file *file, char __user *buf,
				    size_t size, loff_t *ppos)
{
	if (g_XiaomiModemParaReadFuncPtr) {//req is proccessed in pcie host
		return g_XiaomiModemParaReadFuncPtr(file, buf, size, ppos);
	}
	return 0;
}

static int xiaomi_modem_drv_para_get(struct seq_file *m, void *v)
{
#if 0
	int i = 0, gpio = 0;
	char para[4];
	struct mi_t1_gpio_data *mi_t1_data = (struct mi_t1_gpio_data *) m->private;
	seq_printf(m, "GPIO cout=%d : \n", MI_T1_GPIO_COUNT);
	for (i = 0; i < MI_T1_GPIO_COUNT; i++)
	{
		gpio = gpio_t1_get_state(mi_t1_data->dev, i, config);
		seq_printf(m, "GPIO-%d state=%d \n", gpio, config[0]);
	}
#endif
	return 0;
}
static int xiaomi_modem_drv_open(struct inode *inode, struct file *file)
{
    return single_open(file, xiaomi_modem_drv_para_get, file_inode(file)->i_private);
}
static struct proc_dir_entry *last_xiaomi_modem_sfr_entry;
static struct proc_ops   xiaomi_modem_optimize_para_fops = {
    .proc_open	= xiaomi_modem_drv_open,
    .proc_read	= xiaomi_modem_drv_para_read,
    .proc_lseek	= seq_lseek,
    .proc_release	= single_release,
    .proc_write	= xiaomi_modem_drv_para_set,
};
/***********************************xiaomi modem driver optimize API end***/

static int mtk_pwrctl_create_node(void)
{	
	int ret;
	atomic_set(&mdev->port.usage_cnt, 0);
	ret = misc_register(&mtk_pwrctl_miscdev);
	if ( ret < 0)
	{
		dev_err(&mdev->pdev->dev, "misc register fail: %d\n",ret);
		return ret;
	}
	mdev->port.dev = mtk_pwrctl_miscdev.this_device;
	dev_info(&mdev->pdev->dev, "Create misc device node[%s] done\n",
		 mtk_pwrctl_miscdev.name);
/***********************************xiaomi modem driver optimize API start***/
	if (last_xiaomi_modem_sfr_entry == NULL) {
		last_xiaomi_modem_sfr_entry = proc_create("modem_para", 0666, NULL,
			&xiaomi_modem_optimize_para_fops);
	}
/***********************************xiaomi modem driver optimize API end***/
	return 0;
}
static void mtk_pwrctl_delete_node(void)
{
	if(!mdev->port.dev)
		return;
	misc_deregister(&mtk_pwrctl_miscdev);
	mdev->port.dev = NULL;
	pr_info("[%s][%d]: Delete device node[%s] done\n", 
		PWRCTL_DEV_NODE_NAME, __LINE__, mtk_pwrctl_miscdev.name);
/***********************************xiaomi modem driver optimize API start*****************************************************/
	if (last_xiaomi_modem_sfr_entry) {
		remove_proc_entry("modem_para", NULL);
		last_xiaomi_modem_sfr_entry = NULL;
	}
/***********************************xiaomi modem driver optimize API end*****************************************************/
}
int mtk_pwrctl_event_register_callback( void (*cb)(enum pwrctl_evt , void *),
					void *data)
{
	struct pwrctl_notify *notifier;
	struct pwrctl_evt_cb *evt_cb;

	if (!mdev) {
		pr_err("[%s][%d]: the pwrctl driver is not ready\n", 
		       PWRCTL_DEV_NODE_NAME, __LINE__);
		return -EPERM;
	}

	notifier = &mdev->notifier;
	mutex_lock(&notifier->mlock);
	list_for_each_entry(evt_cb, &notifier->cb_list, entry) {
		if(evt_cb->cb == cb) {
			pr_info("[%s][%d]: this callback was registered\n",
			       PWRCTL_DEV_NODE_NAME, __LINE__);
			return 0;
		}
	}

	evt_cb = kzalloc(sizeof(*evt_cb), GFP_KERNEL);
	if (!evt_cb) {
		pr_err("[%s][%d]: Allocate event callback memory failed for %ps\n",
		       PWRCTL_DEV_NODE_NAME, __LINE__, __builtin_return_address(0));
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&evt_cb->entry);
	evt_cb->cb = cb;
	evt_cb->priv = data;
	
	list_add_tail(&evt_cb->entry, &notifier->cb_list);
	mutex_unlock(&notifier->mlock);

	pr_info("[%s][%d]: Register event callback for %ps\n",
		PWRCTL_DEV_NODE_NAME, __LINE__, __builtin_return_address(0));
	return 0;
}
EXPORT_SYMBOL(mtk_pwrctl_event_register_callback);

int mtk_pwrctl_event_unregister_callback(void (*cb)(enum pwrctl_evt , void *))
{
	struct pwrctl_notify *notifier = &mdev->notifier;
	struct pwrctl_evt_cb *evt_cb, *evt_cb_back;
	if (!mdev) {
		pr_err("[%s][%d]: unregister_callback: the gpio driver is not ready\n",
		       PWRCTL_DEV_NODE_NAME, __LINE__);
		return -EPERM;
	}

	mutex_lock(&notifier->mlock);
	list_for_each_entry_safe(evt_cb, evt_cb_back, &notifier->cb_list, entry) {
		if(evt_cb->cb == cb){
			pr_info("[%s][%d]: Delete event callback for %ps\n",
				PWRCTL_DEV_NODE_NAME, __LINE__, __builtin_return_address(0));
			list_del(&evt_cb->entry);
			kfree(evt_cb);
			break;
		}
		
	}

	mutex_unlock(&notifier->mlock);
	pr_info("[%s][%d]: Unregister event callback for %ps\n",
		PWRCTL_DEV_NODE_NAME, __LINE__, __builtin_return_address(0));
	return 0;
}
EXPORT_SYMBOL(mtk_pwrctl_event_unregister_callback);

int mtk_pwrctl_set_suspend_skip(bool flag)
{
	if (!mdev) {
		pr_err("[%s][%d]: unregister_callback: the gpio driver is not ready\n",
		       PWRCTL_DEV_NODE_NAME, __LINE__);
		return -EPERM;
	}

	if(flag)
		set_bit(PWRCTL_SKIP_SUSPEND_OFF, &mdev->pm.flags);
	else
		clear_bit(PWRCTL_SKIP_SUSPEND_OFF, &mdev->pm.flags);

	pr_info("Set suspend skip flag = %d by %ps\n", flag, __builtin_return_address(0));
	return 0;
}
EXPORT_SYMBOL(mtk_pwrctl_set_suspend_skip);
#ifdef FPGA_MODEM_TEST
struct workqueue_struct *poweron_linkup_wq = NULL;
struct work_struct poweron_linkup_work;
static void poweron_linkup_func(struct work_struct *work)
{
	pr_info("pwrctl: %s start!\n", __func__);
	mtk_pwrctl_power_on(mdev, true);
}
static int  poweron_linkup_work_init(void)
{
	//poweron_linkup_wq = create_workqueue("poweron_linkup_work");
	poweron_linkup_wq = alloc_workqueue("poweron_linkup_work", WQ_UNBOUND | WQ_HIGHPRI, 0);
	//set_workqueue_priority(poweron_linkup_wq, MAX_RT_PRIO-1);
	if (!poweron_linkup_wq)
	{
		pr_info("%s Failed to create workqueue\n", __func__);
		return -1;
	}
	INIT_WORK(&poweron_linkup_work, poweron_linkup_func);
	return 0;
}

void xiaomi_modem_power(void)
{
	if (poweron_linkup_wq == NULL)
	{
		poweron_linkup_work_init();
	}
	pr_info("pwrctl: %s before work!\n", __func__);
	queue_work(poweron_linkup_wq, &poweron_linkup_work);
	pr_info("pwrctl: %s after work!\n", __func__);
}

static void poweron_linkup_work_deinit(void)
{
	flush_workqueue(poweron_linkup_wq);
	destroy_workqueue(poweron_linkup_wq);
}

void xiaomi_modem_power_irq_register(void)
{
	pr_info("%s in\n", __func__);
	is_t800_hw = true;
	if (!mdev) {
		pr_err("The PWRCTL driver is not initialized\n");
		return;
	}
	if (!is_registed_irq) {
		mtk_power_irq_register(&mdev->dev_mngr);
		is_registed_irq = true;
	}
	pr_info("%s done\n", __func__);
}
EXPORT_SYMBOL(xiaomi_modem_power_irq_register);

void xiaomi_modem_power_irq_unregister(void)
{
	pr_info("%s in\n", __func__);
	if (!mdev) {
		pr_err("The PWRCTL driver is not initialized\n");
		return;
	}
	if (is_t800_hw) {
		mtk_power_irq_unregister(&mdev->dev_mngr);
		is_registed_irq = false;
	}
	pr_info("%s done\n", __func__);

}
EXPORT_SYMBOL(xiaomi_modem_power_irq_unregister);
#endif



int mtk_pwrctl_gpio_request(struct device *dev, const char *label)
{
	int pin;
	//int ret;

	pin = of_get_named_gpio(dev->of_node, label, 0);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,6,0)
	if (pin < 0) {
                dev_err(dev, "The GPIO <%s> is probed fail, %d\n", label, pin);
                return -1;
        }
#else
	if (pin < 0 || pin >= ARCH_NR_GPIOS) {
		dev_err(dev, "The GPIO <%s> is probed fail, %d\n", label, pin);
		return -1;
	}
#endif
	//ret = devm_gpio_request(dev, pin, label);
	//if (ret) {
	//	dev_err(dev, "Request GPIO <%s> fail, %d\n", label, ret);
	//	return -1;
	//}

	dev_info(dev, "Request GPIO <%s:%d> success\n", label, pin);
	return pin;
}

int mtk_pwrctl_irq_request(struct device *dev, int pin, irq_handler_t handler,
				  unsigned long irqflags, bool irq_wake_flag)
{
	int irq;
	int ret;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,6,0)
	if (pin < 0) {
                dev_err(dev,"Invalid GPIO<%d>\n", pin);
                return IRQ_NOTCONNECTED;;
        }
#else
	if (pin < 0 || pin >= ARCH_NR_GPIOS) {
		dev_err(dev,"Invalid GPIO<%d>\n", pin);
		return IRQ_NOTCONNECTED;;
	}
#endif
	irq = gpio_to_irq(pin);
	if (irq < 0) {
		dev_err(dev,"Failed to get irq of GPIO<%d>\n", pin);
		return IRQ_NOTCONNECTED;
	}

	ret = devm_request_irq(dev,irq, handler,irqflags, "pwrctl", NULL);
	if (ret < 0) {
		dev_err(dev,"Register handler of GPIO<%d> fail, %d\n", pin, ret);
		return IRQ_NOTCONNECTED;
	}

	dev_info(dev,"Request irq<%d> and register handler of GPIO<%d> success\n", irq, pin);
	if (irq_wake_flag) {
		ret = enable_irq_wake(irq);
		dev_info(&mdev->pdev->dev, "Enable IRQ wake of GPIO<%d> %s\n", pin, ret? "fail" : "success");
	}

	return irq;
}

int mtk_pwrctl_irq_free(struct device *dev, int irq)
{
	if (irq == IRQ_NOTCONNECTED)
		return -EINVAL;

	disable_irq(irq);
	devm_free_irq(dev, irq, NULL);
	dev_info(dev, "free irq %d done\n", irq);
	return 0;
}

static int mtk_pwrctl_syspm_suspend(void)
{
	if(!test_bit(PWRCTL_SKIP_SUSPEND_OFF, &mdev->pm.flags) && pwrctl_l3_support) {
		mtk_pwrctl_disable_irqs();
		mtk_pwrctl_power_off(mdev, false);
	}

	pr_info("pwrctl: Syscore suspend success!\n");
	return 0;
}

static void mtk_pwrctl_syspm_resume(void)
{
	if(!test_bit(PWRCTL_SKIP_SUSPEND_OFF, &mdev->pm.flags) && pwrctl_l3_support) {
		mtk_pwrctl_power_on(mdev, false);
		mtk_pwrctl_enable_irqs();
	}

	pr_info("pwrctl: Syscore resume success!\n");
}
static struct syscore_ops mtk_pwrctl_pm_sysops = {
	.suspend = mtk_pwrctl_syspm_suspend,
	.resume = mtk_pwrctl_syspm_resume,
};
static int mtk_pwrctl_probe(struct platform_device *pdev)
{
	if (!mdev){
		dev_err(&pdev->dev, "The PWRCTL driver is not initialized\n");
		return -EPERM;
	}

	mdev->pdev = pdev;
	dev_info(&mdev->pdev->dev, "probe in\n");
	mutex_init(&mdev->notifier.mlock);

	mtk_pwrctl_dev_init(mdev);
	if (mtk_pwrctl_slt_init)
		mtk_pwrctl_slt_init(mdev);

	register_syscore_ops(&mtk_pwrctl_pm_sysops);
	INIT_LIST_HEAD(&mdev->notifier.cb_list);
#if 0 // remove to excute from ioctl
#ifdef FPGA_MODEM_TEST
	xiaomi_modem_power();
#else
	mtk_pwrctl_power_on(mdev, true);
#endif
#endif
	mtk_pwrctl_create_node();
	dev_info(&pdev->dev, "probe done\n");
	return 0;
}

static int mtk_pwrctl_remove(struct platform_device *pdev)
{
	dev_info(&mdev->pdev->dev, "pwrctl removing\n");
	mtk_pwrctl_delete_node();
	unregister_syscore_ops(&mtk_pwrctl_pm_sysops);
	mtk_pwrctl_dev_uninit(mdev);
	if (mtk_pwrctl_slt_uninit)
		mtk_pwrctl_slt_uninit(mdev);

	dev_info(&pdev->dev, "mtk_pwrctl_remove done\n");
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id mtk_pwrctl_of_ids[] = {
	{.compatible = "mediatek,wwan_pwrctl",},
	{},
};
MODULE_DEVICE_TABLE(of, mtk_pwrctl_of_ids);
#endif

static struct platform_driver wwan_gpio_pwrctl = {
	.driver = {
		.name = "wwan_pwrctl",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(mtk_pwrctl_of_ids),
	},
	.probe = mtk_pwrctl_probe,
	.remove = mtk_pwrctl_remove,
};

static int __init mtk_pwrctl_init(void)
{
	int ret;
	
	mdev = kzalloc(sizeof(*mdev), GFP_KERNEL);
	if (!mdev) {
		pr_err("[%s][%d]: Alloc mdev memory fail", PWRCTL_DEV_NODE_NAME, __LINE__);
		return -ENOMEM;
	}
	ret = platform_driver_register(&wwan_gpio_pwrctl);
	if (ret < 0) {
		pr_err("[%s][%d]: Register platform driver fail", PWRCTL_DEV_NODE_NAME, __LINE__);
		kfree(mdev);
		mdev = NULL;
		return -1;
	}
	pr_info("[%s][%d]: Register platform driver done\n", PWRCTL_DEV_NODE_NAME, __LINE__);
	return 0;
}

static void __exit mtk_pwrctl_exit(void)
{
	pr_info("[%s][%d]: exit platform driver\n", PWRCTL_DEV_NODE_NAME, __LINE__);
	platform_driver_unregister(&wwan_gpio_pwrctl);
	kfree(mdev);
	mdev = NULL;
#ifdef FPGA_MODEM_TEST
	poweron_linkup_work_deinit();
#endif
}

module_init(mtk_pwrctl_init);
module_exit(mtk_pwrctl_exit);

module_param(pwrctl_l3_support, bool, 0644);
MODULE_PARM_DESC(pwrctl_l3_support, "This parameter is used to cover L3 power state\n");

MODULE_LICENSE("GPL");
