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
dmesg -C
mode="666"
vcmd_support=${1,-"vcmd=0"}
module=${2:-"vsi_vcx"}
device="/tmp/dev/${module}"
offset=${3:-0x0}
polling=${4:-1}
klog_lvl=${5:-5}
arb_urgent=${6:-0}
arb_weight=${7:-0x1d}
arb_timewindow=${8:-0x1d}
arb_bw_overflow=${9:-0}
vsi_dumplvl=${10:-5}
#Used to setup default parameters
DefaultParameter(){
    vcmd=0
    #default value can be added to here
}
echo

if [ ! -e /tmp/dev ]
then
    mkdir /tmp/dev/
fi
echo "Help information:"
echo "Input format should be like as below"
echo "./driver_load.sh vcmd=0(default) or (1)"
if [ $# -eq 0 ]
then
    DefaultParameter
    echo " Default vcmd_supported value = $vcmd"
else
    para_1="$vcmd_support"
    vcmd_input=${para_1##*=}
    vcmd=$vcmd_input

    if [ $vcmd -ne 0 ] && [ $vcmd -ne 1 ]
    then
        echo "Invalid vcmd_supported value, which = $vcmd"
        echo "vcmd_supported should be 0 or 1"
    fi
    echo "vcmd_supported = $vcmd"
fi
#vcmd_supported = 0(default) or 1
#insert module
rm_module=`lsmod | grep -w $module`
if [ ! -z "$rm_module" ]
then
   rmmod $module || exit 1
fi
if [ $vcmd -eq 1 ]
then
    insmod $module.ko enc_dev_n=$module vcmd_supported=$vcmd ddr_offset=$offset \
    vcmd_isr_polling=$polling vsi_kloglvl=$klog_lvl arbiter_weight=$arb_weight \
    arbiter_urgent=$arb_urgent arbiter_timewindow=$arb_timewindow \
    vsi_dumplvl=$vsi_dumplvl arbiter_bw_overflow=$arb_bw_overflow || exit 1
else
    insmod $module.ko enc_dev_n=$module vcmd_supported=$vcmd || exit 1
fi

echo "module $module inserted"

#remove old nod
rm -f $device

#read the major asigned at loading time
major=`cat /proc/devices | grep -w $module | cut -c1-3`

echo "$module major = $major"

#create dev node
mknod $device c $major 0

echo "node $device created"

#give all 'rw' access
chmod $mode $device

echo "set node access to $mode"

#the end
echo
