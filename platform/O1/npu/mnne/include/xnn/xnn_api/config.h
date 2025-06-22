// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef CONFIG_H_
#define CONFIG_H_
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "xnn/graph/places.h"
#include "xnn/unified_api/xnn_tensor.h"

namespace mnne {
namespace lite {

class LITE_API ConfigBase {
  std::string model_dir_;
  int threads_{1};
  PowerMode mode_{LITE_POWER_BALANCE};
  CLTuneMode opencl_tune_mode_{CL_TUNE_NONE};
  std::string bin_path_{""};
  std::string bin_name_{""};
  PrecisionType precision_{PrecisionType::kAuto};

 public:
  explicit ConfigBase(PowerMode mode = LITE_POWER_BALANCE, int threads = 1);
  void set_model_dir(const std::string& x) { model_dir_ = x; }
  const std::string& model_dir() const { return model_dir_; }
  void set_threads(int threads);
  int threads() const { return threads_; }
  void set_power_mode(PowerMode mode);
  PowerMode power_mode() const { return mode_; }

  void set_binary_path_name(const std::string& path,
                            const std::string& name);

  void set_opencl_tune(CLTuneMode tune_mode = CL_TUNE_NONE,
                       const std::string& path = "",
                       const std::string& name = "",
                       size_t lws_repeats = 4);

  void set_precision(PrecisionType p = PrecisionType::kAuto);
};

}
}


#endif
