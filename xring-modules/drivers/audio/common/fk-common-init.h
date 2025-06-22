/* SPDX-License-Identifier: GPL-2.0-only
 *
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
// #define SIMULATE_COMMON 1
int fk_audio_cma_init(void);
int xr_audif_init(void);
#ifdef SIMULATE_COMMON
int fk_simulate_init(void);
#endif
// int xr_audio_test_init(void);
int fk_audio_ipc_init(void);
int audio_common_init(void);
int fk_audio_pinctrl_init(void);
int fk_audio_tool_init(void);


void fk_audio_cma_exit(void);
void xr_audif_exit(void);
#ifdef SIMULATE_COMMON
void fk_simulate_exit(void);
#endif
void fk_audio_ipc_exit(void);

// void xr_audio_test_exit(void);
void audio_common_exit(void);
void fk_audio_pinctrl_exit(void);
void fk_audio_tool_exit(void);

