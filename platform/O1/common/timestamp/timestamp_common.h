/*
* Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
*
 * Description: timestamp common head file
*/

#ifndef __TIMESTAMP_COMMON_H__
#define __TIMESTAMP_COMMON_H__

#define TS_ITER                         (2)
#define TS_BIT                          (32)
#define TS_MASK                         (0xFFFFFFFF)

#define TS_CNT_ENABLE                   (1UL << (0))
#define TS_HALT_ENABLE                  (1UL << (1))

#define TS_NSEC_PER_USEC                (1000)
#define TS_MSEC_PER_SEC                 (1000)
#define TS_USEC_PER_SEC                 (1000000)

#define TS_CNT_TO_SEC(cnt, freq)        (cnt / freq)
#define TS_CNT_TO_MSEC(cnt, freq)       (TS_CNT_TO_SEC(cnt, freq) * TS_MSEC_PER_SEC \
                                        + (cnt % freq) * TS_MSEC_PER_SEC / freq)
#define TS_CNT_TO_USEC(cnt, freq)       (TS_CNT_TO_SEC(cnt, freq) * TS_USEC_PER_SEC \
                                        + (cnt % freq) * TS_USEC_PER_SEC / freq)

#endif /* __TIMESTAMP_COMMON_H__ */
