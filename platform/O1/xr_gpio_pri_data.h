// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XR_GPIO_PRI_DATA_H__
#define __XR_GPIO_PRI_DATA_H__


#define GPIO_LPIS_AREA_START	0
#define GPIO_LPIS_AREA_END	80


#define GPIO_PERI_AREA1_START	81
#define GPIO_PERI_AREA1_END	170
#define GPIO_PERI_AREA2_START	177
#define GPIO_PERI_AREA2_END	234


#define GPIO_BIT(nr)			(1UL << (nr))
#define GPIO_INVALID_VALUE		0xFFFF


#define LPIS_GPIO_IP_NUMS	3
#define PERI_GPIO_IP_NUMS	5
#define GPIO_IP_MAX_GPIO_NUMS	32
#define GPIO_IP_MAP_SIZE	0x80
#define LPIS_LAST_CONTROLLER_SHIFT_32	15
#define XRSE_IO_SIZE	6

#define SWPORTA_GPIO_16_31(x)		((x) >= 16)
#define GPIO_TO_DR1_BIT_INDEX(x)	((x) - 16)
#define GPIO_OUTPUT_HIGH(pin_mask)	((pin_mask) | (((pin_mask) << 16) & 0xFFFF0000))
#define GPIO_OUTPUT_LOW(pin_mask)	(((pin_mask) << 16) & 0xFFFF0000)


#define TO_GPIO_PIN_INDEX(pin)		((pin) % GPIO_IP_MAX_GPIO_NUMS)
#define TO_GPIO_CONTROLLER(pin)		((pin) / GPIO_IP_MAX_GPIO_NUMS)


#define CONVERT_GPIO_AND_IP(gpio_pin, controller_id, gpio) do { \
	if (gpio_pin <= GPIO_LPIS_AREA_END) { \
		gpio = gpio_pin; \
	} else if (gpio_pin >= GPIO_PERI_AREA1_START && gpio_pin <= GPIO_PERI_AREA1_END){ \
		gpio = gpio_pin + LPIS_LAST_CONTROLLER_SHIFT_32; \
	} else if (gpio_pin >= GPIO_PERI_AREA2_START && gpio_pin <= GPIO_PERI_AREA2_END) { \
		gpio = gpio_pin + LPIS_LAST_CONTROLLER_SHIFT_32 - XRSE_IO_SIZE; \
	} else { \
		controller_id = GPIO_INVALID_VALUE; \
		break; \
	} \
	controller_id = TO_GPIO_CONTROLLER(gpio); \
	gpio = TO_GPIO_PIN_INDEX(gpio); \
} while(0)

#endif
