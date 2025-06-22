// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */
#ifndef XCV_XCV_H
#define XCV_XCV_H

#include "xcv/include/xcv_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

XCV_API int xcvEnvSetup(xcvConfig* config);


XCV_API void xcvCleanUp(void);


XCV_API void* xcvMemAlloc(uint32_t nBytes);


XCV_API void xcvMemFree(void* ptr);


int xcvSetOpPowerMode(xcvOpPowerMode powerMode);

int xcvDumpDataToFile(xcvDataDumpType type, const char* dumpPath);

XCV_API int xcvResize(uint8_t*     srcImg,
                      uint32_t     srcWidth,
                      uint32_t     srcHeight,
                      float32_t    xScale,
                      float32_t    yScale,
                      uint32_t     tileWidth,
                      uint32_t     tileHeight,
                      uint8_t*     dstImg,
                      xcvScaleType scaleType,
                      uint32_t     cycle);

XCV_API int xcvResizeBGR(uint8_t*     srcImg,
                         uint32_t     srcWidth,
                         uint32_t     srcHeight,
                         float32_t    xScale,
                         float32_t    yScale,
                         uint32_t     tileWidth,
                         uint32_t     tileHeight,
                         uint8_t*     dstImg,
                         xcvScaleType scaleType,
                         uint32_t     cycle);

XCV_API int xcvYUV420ToRGB(uint8_t* srcImg,
                           uint32_t srcWidth,
                           uint32_t srcHeight,
                           uint32_t tileWidth,
                           uint32_t tileHeight,
                           uint8_t* dstImg,
                           uint32_t cycle);

XCV_API int xcvRGBToYUV420(uint8_t* srcImg,
                           uint32_t srcWidth,
                           uint32_t srcHeight,
                           uint32_t tileWidth,
                           uint32_t tileHeight,
                           uint8_t* dstImg,
                           uint32_t cycle);

XCV_API int xcvBoxFilter(uint8_t*      srcImg,
                         uint32_t      srcWidth,
                         uint32_t      srcHeight,
                         uint32_t      tileWidth,
                         uint32_t      tileHeight,
                         xcvKernelType kernelType,
                         xcvBorderType borderType,
                         uint8_t*      dstImg,
                         uint32_t      cycle);

XCV_API int xcvGaussianFilter(uint8_t*      srcImg,
                              uint32_t      srcWidth,
                              uint32_t      srcHeight,
                              uint32_t      tileWidth,
                              uint32_t      tileHeight,
                              xcvKernelType kernelType,
                              xcvBorderType borderType,
                              float         sigma,
                              uint8_t*      dstImg,
                              uint32_t      cycle);

XCV_API int xcvSobelFilter(uint8_t*      srcImg,
                           uint32_t      srcWidth,
                           uint32_t      srcHeight,
                           uint32_t      tileWidth,
                           uint32_t      tileHeight,
                           xcvKernelType kernelType,
                           xcvBorderType borderType,
                           uint32_t      normalize,
                           uint8_t*      dstImg,
                           uint32_t      cycle);

XCV_API int xcvLaplacianFilter(uint8_t*      srcImg,
                               uint32_t      srcWidth,
                               uint32_t      srcHeight,
                               uint32_t      tileWidth,
                               uint32_t      tileHeight,
                               xcvBorderType borderType,
                               uint16_t*     dstImg,
                               uint32_t      cycle);

XCV_API int xcvPyrUpS16(int16_t*      srcImg,
                        uint32_t      srcWidth,
                        uint32_t      srcHeight,
                        uint32_t      tileWidth,
                        uint32_t      tileHeight,
                        xcvBorderType borderType,
                        uint32_t      numPryLevels,
                        int16_t*      dstImg,
                        uint32_t      cycle);

XCV_API int xcvPyrDownS16(int16_t*      srcImg,
                          uint32_t      srcWidth,
                          uint32_t      srcHeight,
                          uint32_t      tileWidth,
                          uint32_t      tileHeight,
                          xcvBorderType borderType,
                          uint32_t      numPryLevels,
                          int16_t*      dstImg,
                          uint32_t      cycle);

XCV_API int xcvDilate(uint8_t*      srcImg,
                      uint32_t      srcWidth,
                      uint32_t      srcHeight,
                      uint32_t      tileWidth,
                      uint32_t      tileHeight,
                      uint32_t      maskWidth,
                      uint32_t      maskHeight,
                      xcvBorderType borderType,
                      uint8_t*      dstImg,
                      uint32_t      cycle);

XCV_API int xcvErode(uint8_t*      srcImg,
                      uint32_t      srcWidth,
                      uint32_t      srcHeight,
                      uint32_t      tileWidth,
                      uint32_t      tileHeight,
                      uint32_t      maskWidth,
                      uint32_t      maskHeight,
                      xcvBorderType borderType,
                      uint8_t*      dstImg,
                      uint32_t      cycle);

XCV_API int xcvIntegralImage(uint8_t*      srcImg,
                      uint32_t      srcWidth,
                      uint32_t      srcHeight,
                      uint32_t      tileWidth,
                      uint32_t      tileHeight,
                      uint32_t*     dstImg,
                      uint32_t      cycle);

XCV_API int xcvCalcHist(uint8_t*      srcImg,
                      uint32_t      srcWidth,
                      uint32_t      srcHeight,
                      uint32_t      tileWidth,
                      uint32_t      tileHeight,
                      xcvBorderType borderType,
                      uint8_t*     dstImg,
                      uint32_t      cycle);

XCV_API int xcvFast(uint8_t*      srcImg,
                      uint32_t      srcWidth,
                      uint32_t      srcHeight,
                      uint32_t      tileWidth,
                      uint32_t      tileHeight,
                      xcvBorderType borderType,
                      uint32_t      threshold,
                      uint32_t      bytesPerPel,
                      uint8_t*     dstImg,
                      uint32_t      cycle);

XCV_API int xcvNLM(uint8_t*      srcImg,
                      uint32_t      srcWidth,
                      uint32_t      srcHeight,
                      uint32_t      tileWidth,
                      uint32_t      tileHeight,
                      xcvBorderType borderType,
                      uint32_t      searchWindowSize,
                      uint32_t      templateWindowSize,
                      float32_t     sigma,
                      uint8_t*     dstImg,
                      uint32_t      cycle);

XCV_API int xcvThresh(uint8_t*      srcImg,
                      uint32_t      srcWidth,
                      uint32_t      srcHeight,
                      uint32_t      tileWidth,
                      uint32_t      tileHeight,
                      xcvBorderType borderType,
                      uint32_t      threshold,
                      xcvThreshType  threshType,
                      uint8_t*     dstImg,
                      uint32_t      cycle);

XCV_API int xcvThreshTriangle(uint8_t*      srcImg,
                      uint32_t      srcWidth,
                      uint32_t      srcHeight,
                      uint32_t      tileWidth,
                      uint32_t      tileHeight,
                      xcvBorderType borderType,
                      uint32_t      threshold,
                      xcvThreshType  threshType,
                      uint8_t*     dstImg,
                      uint32_t      cycle);

XCV_API int xcvRotate(uint8_t*      srcImg,
                      uint32_t      srcWidth,
                      uint32_t      srcHeight,
                      uint32_t      tileWidth,
                      uint32_t      tileHeight,
                      uint8_t*     dstImg,
                      uint32_t      cycle);

XCV_API int xcvWarp(uint8_t*      srcImg,
                      uint32_t      srcWidth,
                      uint32_t      srcHeight,
                      uint32_t      tileWidth,
                      uint32_t      tileHeight,
                      xcvBorderType borderType,
                      uint8_t*     dstImg,
                      uint32_t      cycle);

#ifdef __cplusplus
}
#endif

#endif
