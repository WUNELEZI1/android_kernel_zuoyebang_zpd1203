#ifndef __XR_ID_H__
#define __XR_ID_H__

/********SOC Short Code***********/
#define SOC_O1      0x01

/********Batch Number Code********/
#define ES_TYPE     0x00
#define CS_TYPE     0x01
#define EC_TYPE     0x02

// O1 chipId
#define O1_ES_CHIPID  ((SOC_O1<<3) | ES_TYPE)
#define O1_CS_CHIPID  ((SOC_O1<<3) | CS_TYPE)
#define O1_EC_CHIPID  ((SOC_O1<<3) | EC_TYPE)

#endif
