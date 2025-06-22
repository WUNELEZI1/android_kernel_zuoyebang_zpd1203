#-------------------------------------------------------------------------------
#-                                                                            --
#-       This software is confidential and proprietary and may be used        --
#-        only as expressly authorized by a licensing agreement from          --
#-                                                                            --
#-                            VeriSilicon Inc.                                --
#-                                                                            --
#-                   (C) COPYRIGHT 2015 VeriSilicon Inc                       --
#-                            ALL RIGHTS RESERVED                             --
#-                                                                            --
#-                 The entire notice above must be reproduced                 --
#-                  on all copies and should not be removed.                  --
#-                                                                            --
#-------------------------------------------------------------------------------

#!/bin/bash

usage(){
    echo "Brief: This shell will create vcx_normal_cfg.h and vcx_vcmd_cfg.h accroding to customer:project"
    echo "Usage: $0 [customer:project]"
    echo "example: $0 VSI:TIG01"
}

PY_PATH="../../../../common/hwcfg/"
# Check parameter
if [ $# -ne 0 ]; then
    echo "$1" | grep -q ":"
    if [ $? -eq 0 ]; then # for customer:prject, like VSI:DEV
        if [ -e "vcx_normal_cfg.h" ]; then
            mv -f vcx_normal_cfg.h vcx_normal_cfg_backup.h
        fi
        if [ -e "vcx_vcmd_cfg.h" ]; then
            mv -f vcx_vcmd_cfg.h vcx_vcmd_cfg_backup.h
        fi
        echo "get accurate customer:project setting, try to output -fpga..."
        python3 ${PY_PATH}gen_cfg.py ${PY_PATH}features.json ${PY_PATH}builds.json get $1 -fpga
    else
        usage
    fi
else
    usage
fi

