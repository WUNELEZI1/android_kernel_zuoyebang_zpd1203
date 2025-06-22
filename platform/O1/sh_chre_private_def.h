// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __SH_CHRE_PRIVATE_DEF_H__
#define __SH_CHRE_PRIVATE_DEF_H__
#if defined(__KERNEL__) && !defined(__NuttX__)
#include <linux/stddef.h>
#else
#include <stddef.h>
#endif

enum class MessageExtension : uint32_t {
  MESSAGE_START = 0,
  HUB_INFO_REQUESET = 2,
  HUB_INFO_RESPONSE = 3,
  NANOAPP_LIST_REQUESET = 4,
  NANOAPP_LIST_RESPONSE = 5,
  DEBUG_DUMP_REQUEST = 12,
  DEUBG_DUMP_RESPONSE = 14,
  SETTING_CHANGE_MESSAGE = 18,
  SELF_TEST_REQUEST = 20,
  SELF_TEST_RESPONSE = 21,
  NANOAPP_ENABLE_REQUEST = 33,
  NANOAPP_DISABLE_REQUEST = 34,
  CHRE_RESTART_REQUEST = 38,
  MESSAGE_END,
};

#endif
