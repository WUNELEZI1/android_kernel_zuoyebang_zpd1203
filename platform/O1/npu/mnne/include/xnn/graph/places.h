// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#pragma once
#include <set>
#include <string>

#if defined _WIN32 || defined __CYGWIN__
#define XNN_HELPER_DLL_IMPORT __declspec(dllimport)
#define XNN_HELPER_DLL_EXPORT __declspec(dllexport)
#define XNN_HELPER_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define XNN_HELPER_DLL_IMPORT __attribute__((visibility("default")))
#define XNN_HELPER_DLL_EXPORT __attribute__((visibility("default")))
#else
#define XNN_HELPER_DLL_IMPORT
#define XNN_HELPER_DLL_EXPORT
#endif
#endif

#define LITE_API XNN_HELPER_DLL_EXPORT
#define LITE_API_IMPORT XNN_HELPER_DLL_IMPORT

namespace mnne {
namespace lite {

enum class L3CacheSetMethod {
  kDeviceL3Cache = 0,
  kDeviceL2Cache = 1,
  kAbsolute = 2
};

enum class TargetType : int {
  kUnk = 0,
  kHost = 1,
  kARM = 4,
  kOpenCL = 5,
  kAny = 6,
  kNPU = 8,
  kNPUMapping = 9,
  kF1NPU = 100,
  NUM = 101,
};

enum class PrecisionType : int {
  kUnk = 0,
  kFloat = 1,
  kInt8 = 2,
  kInt32 = 3,
  kAny = 4,
  kFP16 = 5,
  kBool = 6,
  kInt64 = 7,
  kInt16 = 8,
  kUInt8 = 9,
  kFP64 = 10,
  kUInt16 = 11,
  kUInt32 = 12,
  kUInt64 = 13,
  kAuto = 14,
  NUM = 15
};

typedef enum {
  LITE_PRECISION_NORMAL = 0,
  LITE_PRECISION_LOW = 1,
} PrecisionMode;

enum class DataLayoutType : int {
  kUnk = 0,
  kNCHW = 1,
  kNHWC = 3,
  kImageDefault = 4,
  kImageFolder = 5,
  kImageNW = 6,
  kAny = 2,
  kMetalTexture2DArray = 7,
  kMetalTexture2D = 8,
  NUM = 9
};

typedef enum {
  LITE_POWER_LOW = 0,
  LITE_POWER_BALANCE = 1,
  LITE_POWER_HIGH = 2,
  LITE_POWER_FULL = 3,
} PowerMode;

typedef enum {
  CL_TUNE_NONE = 0,
  CL_TUNE_RAPID = 1,
  CL_TUNE_NORMAL = 2,
  CL_TUNE_EXHAUSTIVE = 3
} CLTuneMode;

typedef enum {
  CL_PRECISION_AUTO = 0,
  CL_PRECISION_FP32 = 1,
  CL_PRECISION_FP16 = 2
} CLPrecisionType;

__attribute__((unused)) static size_t PrecisionTypeLength(PrecisionType type) {
  switch (type) {
    case PrecisionType::kFloat:
      return 4;
    case PrecisionType::kFP64:
      return 8;
    case PrecisionType::kUInt8:
      return 1;
    case PrecisionType::kInt8:
      return 1;
    case PrecisionType::kInt32:
      return 4;
    case PrecisionType::kUInt32:
      return 4;
    case PrecisionType::kInt64:
      return 8;
    case PrecisionType::kUInt64:
      return 8;
    case PrecisionType::kFP16:
      return 2;
    case PrecisionType::kInt16:
      return 2;
    case PrecisionType::kUInt16:
      return 2;
    default:
      return 0;
  }
}

enum class QuantType : int {
  QUANT_INT8,
  QUANT_INT16,
};

template <typename T>
struct PrecisionTypeTrait {
  constexpr static PrecisionType Type() { return PrecisionType::kUnk; }
};

#define _ForEachPrecisionTypeHelper(callback, cpp_type, precision_type) \
  callback(cpp_type, ::mnne::lite::PrecisionType::precision_type);

#define _ForEachPrecisionType(callback)                   \
  _ForEachPrecisionTypeHelper(callback, bool, kBool);     \
  _ForEachPrecisionTypeHelper(callback, float, kFloat);   \
  _ForEachPrecisionTypeHelper(callback, double, kFP64);   \
  _ForEachPrecisionTypeHelper(callback, uint8_t, kUInt8); \
  _ForEachPrecisionTypeHelper(callback, int8_t, kInt8);   \
  _ForEachPrecisionTypeHelper(callback, int16_t, kInt16); \
  _ForEachPrecisionTypeHelper(callback, int, kInt32);     \
  _ForEachPrecisionTypeHelper(callback, int64_t, kInt64);

#define DefinePrecisionTypeTrait(cpp_type, precision_type)           \
  template <>                                                        \
  struct PrecisionTypeTrait<cpp_type> {                              \
    constexpr static PrecisionType Type() { return precision_type; } \
  }

_ForEachPrecisionType(DefinePrecisionTypeTrait);

#ifdef ENABLE_ARM_FP16
typedef __fp16 float16_t;
_ForEachPrecisionTypeHelper(DefinePrecisionTypeTrait, float16_t, kFP16);
#endif

#undef _ForEachPrecisionTypeHelper
#undef _ForEachPrecisionType
#undef DefinePrecisionTypeTrait

#define TARGET(item__) mnne::lite::TargetType::item__
#define PRECISION(item__) mnne::lite::PrecisionType::item__
#define DATALAYOUT(item__) mnne::lite::DataLayoutType::item__

const std::string& TargetToStr(TargetType target);

const std::string& PrecisionToStr(PrecisionType precision);

const std::string& DataLayoutToStr(DataLayoutType layout);

const std::string& TargetRepr(TargetType target);

const std::string& PrecisionRepr(PrecisionType precision);

const std::string& DataLayoutRepr(DataLayoutType layout);

const std::string& CLTuneModeToStr(CLTuneMode mode);

const std::string& CLPrecisionTypeToStr(CLPrecisionType type);

std::set<TargetType> ExpandValidTargets(TargetType target = TARGET(kAny));

std::set<PrecisionType> ExpandValidPrecisions(
    PrecisionType precision = PRECISION(kAny));

std::set<DataLayoutType> ExpandValidLayouts(
    DataLayoutType layout = DATALAYOUT(kAny));

struct LITE_API Place {
  TargetType target{TARGET(kUnk)};
  PrecisionType precision{PRECISION(kUnk)};
  DataLayoutType layout{DATALAYOUT(kUnk)};
  int16_t device{0};

  Place() = default;
  Place(TargetType target,
        PrecisionType precision = PRECISION(kFloat),
        DataLayoutType layout = DATALAYOUT(kNCHW),
        int16_t device = 0);

  bool is_valid() const;

  size_t hash() const;

  bool operator==(const Place& other) const;

  bool operator!=(const Place& other) const;

  friend bool operator<(const Place& a, const Place& b);

  std::string DebugString() const;

};

}
}
