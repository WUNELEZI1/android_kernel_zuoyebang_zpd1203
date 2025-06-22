#include <linux/version.h>
#include <soc/xring/dfx_switch.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
#include "mdr_public_if.h"
#include "mdr_pub.h"
#else
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_public_if.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h>
#endif

#include "mdm_mdr.h"

struct mdr_exception_info_s modem_exception = {
		.e_modid            = (u32)MODID_MODEM_PANIC,
		.e_modid_end        = (u32)MODID_MODEM_PANIC,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_WAIT,
		.e_notify_core_mask = MDR_CP,
		.e_reset_core_mask  = MDR_CP,
		.e_from_core        = MDR_CP,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = CP_S_MODEM,
		.e_exce_subtype     = 0,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "modem",
		.e_desc             = "modem",
};

int mdm_mdr_exception_register(void)
{
	int ret = 0;

	ret = mdr_register_exception(&modem_exception);
	if (ret == 0) {
		pr_err("exception register fail\n");
		return -EINVAL;
	}

	return 0;
}

int mdm_mdr_exception_unregister(void)
{
	int ret = 0;

	ret = mdr_unregister_exception(modem_exception.e_modid);
	if (ret == 0) {
		pr_err("exception unregister fail\n");
		return -EINVAL;
	}

	return 0;
}

int mdm_mdr_trigger_exception(void)
{
	int ret = 0;
	mdr_system_error(MODID_MODEM_PANIC, 0, 0);
	//panic("MODID_MODEM_PANIC");
	return ret;
}
EXPORT_SYMBOL(mdm_mdr_trigger_exception);
