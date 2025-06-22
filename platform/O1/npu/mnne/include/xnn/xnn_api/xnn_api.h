// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef XNN_API_H_
#define XNN_API_H_
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "xnn/graph/places.h"
#include "xnn/unified_api/xnn_tensor.h"
#include "xnn/xnn_api/config.h"

namespace mnne {
namespace lite {

class LITE_API XnnConfig : public ConfigBase {

  PrecisionMode precision_mode_{LITE_PRECISION_NORMAL};
  std::string lite_model_file_;
  const char* model_buffer_{nullptr};
  mnne_uint32_t model_buffer_size_{0};
  bool model_from_memory_{false};
  TargetType target_{TargetType::kARM};
  XnnProfileConfig profileConfig_;
  XnnShareMode shareMode_{XNN_SHARED_NONE};
  mnne_uint32_t createNetworkType_{XNN_CREATE_NETWORK_FROM_MEMORY};
  XNNLogLevel logLevel{XNN_LOG_CLOSE};
  bool ddrVoteClose{false};

 public:
  void set_model_from_file(const std::string& x){ lite_model_file_ = x;}
  void set_model_from_buffer(const std::string &x);
  void set_model_from_buffer(const char* buf, mnne_uint32_t size);
  const std::string& lite_model_file() const;
  bool is_model_from_memory() const;
  void set_profile_config(XnnProfileConfig& profileConfig) { profileConfig_ = profileConfig; };
  const XnnProfileConfig& get_profile_config() const { return profileConfig_; };
  void set_precision_mode(PrecisionMode mode) { precision_mode_ = mode; }
  const char* get_model_buffer_ptr() const;
  mnne_uint32_t get_model_buffer_size() const;

  PrecisionMode precision_mode() const { return precision_mode_; }

  void set_backend(TargetType target) { target_ = target; }
  TargetType get_backend() const { return target_; }

  void set_shareMode(XnnShareMode shareMode);
  XnnShareMode get_shareMode() const;

  void set_create_network_type(mnne_uint32_t networkType);
  mnne_uint32_t get_create_network_type() const;
  void set_log_level(XNNLogLevel level);
  void set_ddr_vote_close();
  bool get_ddr_vote_close() const;
};

class LITE_API XnnEvent {
 public:
  XnnEvent();
  status_t Synchronize();
  void* getEvent();
  virtual ~XnnEvent();
 private:
  void* mEvent{nullptr};
};

enum class LiteModelType { kProtobuf = 0, kNaiveBuffer, UNK };

class LITE_API XnnPredictor {
 public:
  XnnPredictor() = default;

  virtual XNNTensor* GetInput(int i) = 0;

  virtual XNNTensor* GetOutput(int i) const = 0;

  virtual void Run() = 0;

  virtual status_t AsyncRun(XnnAsyncRunCallback) = 0;

  virtual std::string GetVersion() const = 0;

  virtual std::vector<std::string> GetInputNames() = 0;

  virtual std::vector<std::string> GetOutputNames() = 0;

  virtual std::vector<IOInfo> GetInputInfos() = 0;

  virtual std::vector<IOInfo> GetOutputInfos() = 0;

  virtual void SaveOptimizedModel(
      const std::string& model_dir,
      LiteModelType model_type = LiteModelType::kProtobuf,
      bool record_info = false);

  virtual ~XnnPredictor() = default;

  virtual status_t AsyncRun() = 0;
  virtual status_t Synchronize() = 0;
  virtual status_t AsyncRun(XnnEvent* event) = 0;
};

template <typename ConfigT>
LITE_API std::shared_ptr<XnnPredictor> CreateXnnPredictor(const ConfigT&);

LITE_API std::shared_ptr<XnnPredictor> CreateXnnPredictor(const XnnConfig&);

LITE_API std::shared_ptr<XnnPredictor> CreateSharedXnnPredictor(const XnnConfig& config, XnnPredictor* basePredictor,
                                                                XnnPredictor* weightSharedPredictor = nullptr);

}
}


#endif
