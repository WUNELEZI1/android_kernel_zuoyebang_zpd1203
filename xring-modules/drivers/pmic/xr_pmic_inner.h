/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024 XRing Technologies Co., Ltd.
 *
 */
#ifndef __XR_PMIC_INNER_H__
#define __XR_PMIC_INNER_H__

#if IS_ENABLED(CONFIG_XRING_PMIC_RTC)
int xr_pmic_rtc_init(void);
void xr_pmic_rtc_exit(void);
#else
static int xr_pmic_rtc_init(void)
{
	return 0;
}
static void xr_pmic_rtc_exit(void)
{
}
#endif

#if IS_ENABLED(CONFIG_XRING_PMIC_ADC)
int xr_pmic_adc_init(void);
void xr_pmic_adc_exit(void);
#else
static int xr_pmic_adc_init(void)
{
	return 0;
}
static void xr_pmic_adc_exit(void)
{
}
#endif

#if IS_ENABLED(CONFIG_XRING_PMIC_RGB_LED)
int xr_pmic_led_init(void);
void xr_pmic_led_exit(void);
#else
static int xr_pmic_led_init(void)
{
	return 0;
}
static void xr_pmic_led_exit(void)
{
}
#endif

#if IS_ENABLED(CONFIG_XRING_PMIC_KEY)
int xr_pmic_key_init(void);
void xr_pmic_key_exit(void);
#else
static int xr_pmic_key_init(void)
{
	return 0;
}
static void xr_pmic_key_exit(void)
{
}
#endif

#if IS_ENABLED(CONFIG_XRING_PMIC_MNTN)
int xr_pmic_mntn_init(void);
void xr_pmic_mntn_exit(void);
#else
static int xr_pmic_mntn_init(void)
{
	return 0;
}
static void xr_pmic_mntn_exit(void)
{
}
#endif

#endif /* __XR_PMIC_INNER_H__ */
