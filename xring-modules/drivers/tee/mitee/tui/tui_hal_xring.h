/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023 XiaoMi, Inc.
 * All Rights Reserved.
 */
#ifndef MITEE_TUI_HAL_XRING_H
#define MITEE_TUI_HAL_XRING_H

#include "mitee_tui.h"

uint32_t hal_tui_alloc(const char *alloc_name, struct tui_alloc_buffer_t *allocbuffer, size_t allocsize);
void hal_tui_free(void);
int tui_get_panel_vendor(void);

uint32_t hal_alloc_font_buffer(uint32_t font_size);
void hal_get_font_buffer(struct tui_alloc_buffer_t **buffer);

#endif /* MITEE_TUI_HAL_XRING_H */
