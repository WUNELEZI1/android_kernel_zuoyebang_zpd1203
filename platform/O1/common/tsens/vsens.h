/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __VSENS_ID_H__
#define __VSENS_ID_H__

/* virtual sensor algo enum
 * XR_VIRT_SENS_ALOG_MAXIMUM: soc maximum temperature
 * XR_VIRT_SENS_ALOG_MINMUM:  soc minmum temperature
 * XR_VIRT_SENS_ALOG_SS_MAX:  subsys maximum temperature
 * XR_VIRT_SENS_ALOG_SS_MIN:  subsys minmum temperature
 * XR_VIRT_SENS_ALOG_SS_AVG:  subsys avage temperature
 * XR_VIRT_SENS_ALOG_LINEAR_FIT:  linear fitting temperature
 * XR_VIRT_SENS_ALOG_FIXED:   fixed temperature
*/
#define XR_VIRT_SENS_ALOG_MAXIMUM	0
#define XR_VIRT_SENS_ALOG_MINMUM	1
#define XR_VIRT_SENS_ALOG_SS_MAX	2
#define XR_VIRT_SENS_ALOG_SS_MIN	3
#define XR_VIRT_SENS_ALOG_SS_AVG	4
#define XR_VIRT_SENS_ALOG_LINEAR_FIT	5
#define XR_VIRT_SENS_ALOG_FIXED		6

#endif
