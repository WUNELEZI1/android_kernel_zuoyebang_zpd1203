// SPDX-License-Identifier: GPL-2.0-only

#ifndef XR_PMIC_RTC_HEADER_H
#define XR_PMIC_RTC_HEADER_H

typedef void (*pmic_rtc_callback_t)(void);
extern void pmic_register_alarm_callback(pmic_rtc_callback_t fn);
extern int pmic_rtc_getalarm(struct rtc_wkalrm *alarm);
extern int pmic_rtc_poweroffalarm_enable(bool enabled);
extern int pmic_rtc_setalarm(struct rtc_wkalrm *alarm);
extern int pmic_rtc_getwkalarm(struct rtc_wkalrm *alarm);
extern int pmic_rtc_setwkalarm(struct rtc_wkalrm *alarm);
extern int pmic_rtc_settime(struct rtc_time *tm);
extern int pmic_rtc_gettime(struct rtc_time *tm);

#endif