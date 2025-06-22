#ifndef __SHUB_APP_ROUTER_H__
#define __SHUB_APP_ROUTER_H__
/**
 * @brief send client exit
 *
 * @param index who exit
 * see vendor/xring/contexthub/hal/XRSensor/Inc/XRSensorClient.h
 * @return int errno
 */
int app_router_notify_exit(unsigned int index);
#endif //__SHUB_APP_ROUTER_H__
