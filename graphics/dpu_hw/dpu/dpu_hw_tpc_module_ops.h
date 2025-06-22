#ifndef _DPU_HW_TPC_MODULE_OPS_H_
#define _DPU_HW_TPC_MODULE_OPS_H_

#include "dpu_hw_common.h"
/**
 * dpu_hw_tpc_module_init - dpu tpc module init
 */
void dpu_hw_tpc_module_init(DPU_IOMEM dpu_base);

/**
 * dpu_hw_tpc_module_cfg - dpu tpc config according part id
 */
void dpu_hw_tpc_module_cfg(u32 part_id);

#endif
