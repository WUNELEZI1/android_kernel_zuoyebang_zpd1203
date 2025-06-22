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

#ifndef __XR_TEST_H__
#define __XR_TEST_H__

/* IOCTL commands */

#define AUDIO_IOC_MAGIC	'k'

#define _CRT_SECURE_NO_WARNINGS
#ifdef	_CRT_SECURE_NO_WARNINGS
#define memcpy_s(dest, destsz, src, count) (memcpy((dest), (src), (count)) != NULL ? 0 : 1)
#endif

#define MBOX_DATA_BASE_COUNT    15

enum audio_test_ioc_num {
	AUDIO_CASE_0 = 0,
	AUDIO_CASE_1,
	AUDIO_CASE_2,
	AUDIO_CASE_3,
	AUDIO_CASE_4,
	AUDIO_CASE_5,
	AUDIO_CASE_6,
	IOC_BOUNDARY,
};

#define AUDIO_TEST_CASE_000			_IO(AUDIO_IOC_MAGIC, AUDIO_CASE_0)
#define AUDIO_TEST_CASE_001			_IO(AUDIO_IOC_MAGIC, AUDIO_CASE_1)
#define AUDIO_TEST_CASE_002			_IO(AUDIO_IOC_MAGIC, AUDIO_CASE_2)
#define AUDIO_TEST_CASE_003			_IO(AUDIO_IOC_MAGIC, AUDIO_CASE_3)
#define AUDIO_TEST_CASE_004			_IO(AUDIO_IOC_MAGIC, AUDIO_CASE_4)
#define AUDIO_TEST_CASE_005			_IO(AUDIO_IOC_MAGIC, AUDIO_CASE_5)
#define AUDIO_TEST_CASE_006			_IO(AUDIO_IOC_MAGIC, AUDIO_CASE_6)

#endif /* __XR_TEST_H__ */

