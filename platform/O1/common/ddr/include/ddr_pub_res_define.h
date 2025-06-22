/*
 * Copyright (C) 2023-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __DDR_PUB_RES_DEFINE_H__
#define __DDR_PUB_RES_DEFINE_H__

#ifdef CONFIG_XRING_DDR_DEVFREQ_MODULE
#include <dt-bindings/xring/platform-specific/lpis_actrl.h>
#else
#include <lpis_actrl.h>
#endif

#define LPIS_ACTRL_DDR_TYPE_FREQ_REG					LPIS_ACTRL_SC_RSV_NS_5
#define LPIS_ACTRL_DDR_TYPE_FREQ_REG_FREQ_SHIFT				0
#define LPIS_ACTRL_DDR_TYPE_FREQ_REG_FREQ_MASK				0x00007FFF

#endif
