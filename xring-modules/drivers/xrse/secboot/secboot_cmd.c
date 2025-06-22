// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 */

#include "../xrse_internal.h"
#include "secboot_status_check.h"

static struct module_case secboot_cases[] = {
	{
		.name = "check",
		.store_handle = xr_secboot_status_store
	},
};

struct xrse_module_cmd secboot_module = {
	.module_name = "secboot",
	.cases = secboot_cases,
	.num_cases = ARRAYSIZE(secboot_cases),
};
