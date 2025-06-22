/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2024, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XR_HW_TEST_LED_H__
#define __XR_HW_TEST_LED_H__

int hw_test_led_set_ready(void);

int hw_test_led_set_testing(void);

int hw_test_led_set_pass(void);

int hw_test_led_set_fail(void);

int hw_test_led_protocol_set(unsigned int led_protocol);

#endif /* __XR_HW_TEST_LED_H__ */
