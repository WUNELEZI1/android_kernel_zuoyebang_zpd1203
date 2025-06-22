#ifndef _DPU_WB_H_
#define _DPU_WB_H_

#include "litexdm.h"
#include "dpu_comp_mgr.h"
#include "dpu_hw_wb_scaler.h"

int32_t dpu_wb_cfg(struct composer *comp, struct dpu_frame *frame);
#endif
