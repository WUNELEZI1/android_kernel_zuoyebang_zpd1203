#  Copyright 2013 Google Inc. All Rights Reserved.
mode="666"
module=${1:-"hantrodec"}
device="/tmp/dev/$module"
offset=${2:-0x0}
#set dma_used as 1 when using dma and set the host base
dma_used=${3:-0}
host_base=${4:-0x40005000}
vcmd_isr_polling=${5:-1}
vsi_kloglvl=${6:-5}
arb_urgent=${7:-0}
arb_weight=${8:-0x1d}
arb_timewindow=${9:-0x1d}
arb_bw_overflow=${10:-0}
vsi_dumplvl=${11:-5}
echo "$module"

if [ ! -e /tmp/dev ]
then
    mkdir -p /tmp/dev/
fi

#insert module
rm_module=`lsmod |grep $module`
if [ ! -z "$rm_module" ]
then
   sudo rmmod $module || exit 1
fi
sudo insmod $module.ko dec_dev_n="$module" ddr_offset=$offset dma_used=$dma_used  \
                  host_base=$host_base vcmd_isr_polling=$vcmd_isr_polling  \
                  vsi_kloglvl=$vsi_kloglvl arbiter_weight=$arb_weight \
                  arbiter_urgent=$arb_urgent arbiter_timewindow=$arb_timewindow \
                  vsi_dumplvl=$vsi_dumplvl arbiter_bw_overflow=$arb_bw_overflow|| exit 1

echo "module $module inserted"

#remove old nod
rm -f $device

#read the major asigned at loading time
major=`cat /proc/devices | grep -w $module | cut -c1-3`

echo "$module major = $major"

#create dev node
sudo mknod $device c $major 0

echo "node $device created"

#give all 'rw' access
sudo chmod $mode $device

echo "set node access to $mode"

#the end
echo
