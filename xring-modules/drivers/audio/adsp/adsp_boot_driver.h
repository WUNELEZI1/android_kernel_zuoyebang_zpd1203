/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _ADSP_BOOT_DRIVER_H_
#define _ADSP_BOOT_DRIVER_H_

#include <linux/regulator/consumer.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <dt-bindings/xring/platform-specific/lpis_actrl.h>

#define AUDSYS_POWEROFF_FLOW_DOWN	(ACPU_LPIS_ACTRL + LPIS_ACTRL_AUDSYS_POWERONOFF_FLOW_DONE)
#define AUDSYS_POWER_STATE		(ACPU_LPIS_ACTRL + LPIS_ACTRL_AUDSYS_POWER_STATE)
#define ACPU2ADSP_INTERRPUT_LOCK	(ACPU_LPIS_ACTRL + LPIS_ACTRL_ACPU2ADSP_INTERRPUT_LOCK)
#define AUDSYS_POWERON_STATE					0x1
#define AUDSYS_PREPARE_POWERDOWN				0x2
#define AUDSYS_ADSP_VOTE_SHUTDOWN				0x3
#define WAIT_AUDSYS_POWERON_DONE_TIMEOUT		(10 * 1000)
#define WAIT_AUDSYS_POWEROFF_FLOW_DOWN_TIMEOUT	(10 * 1000)
#define ADSP_BOOT_RETRY_TIME					0x1

struct adsp_boot_dev {
	/* adsp vote regulator */
	struct regulator *adsp_boot_reg;

	/* audsys poweroff flow finish flag */
	void __iomem *audsys_poweroff_flow_down;

	/* audsys power status */
	void __iomem *audsys_power_state;

	/* acpu2adsp interrupt lock reg */
	void __iomem *acpu2adsp_interupt_lock_reg;

	/* adsp load addr */
	void __iomem *adsp_load_addr;

	/* adsp run addr */
	void __iomem *adsp_run_addr;

	unsigned int adsp_image_size;

	bool power_status;
	bool vote_status;
};

int get_adsp_power_status(void);
int copy_adsp_img_to_running_memory(void);
int adsp_poweron(void);
int adsp_poweroff(void);

#endif //_ADSP_BOOT_DRIVER_H_

