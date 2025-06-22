#!/bin/bash

set -e

DTC_VERSION="1.6"
LOCAL_PATH=$(pwd)
build_type=""

# DTS Warning intercept
DTC_FLAGS_ERR="-Ereg_format \
               -Eranges_format \
               -Epci_bridge \
               -Espi_bus_bridge \
               -Eunit_address_format \
               -Eavoid_default_addr_size \
               -Echosen_node_stdout_path \
               -Eduplicate_label \
               -Egpios_property \
               -Einterrupt_provider \
               -Egraph_nodes \
               -Egraph_child_address \
               -Egraph_port \
               -Egraph_endpoint \
               -Ei2c_bus_bridge \
               -Edma_ranges_format \
               -Eclocks_property \
               -Ecooling_device_property \
               -Edmas_property \
               -Ehwlocks_property \
               -Einterrupts_extended_property \
               -Eio_channels_property \
               -Eiommus_property \
               -Emboxes_property \
               -Emsi_parent_property \
               -Emux_controls_property \
               -Ephys_property \
               -Epower_domains_property \
               -Epwms_property \
               -Eresets_property \
               -Esound_dai_property \
               -Ethermal_sensors_property \
               -Ealias_paths"

# DTS Warning not care
export DTC_FLAGS="${DTC_FLAGS_ERR} -Wno-pci_device_bus_num \
                  -Wno-pci_device_reg \
                  -Wno-i2c_bus_reg \
                  -Wno-spi_bus_reg \
                  -Wno-simple_bus_reg \
                  -Wno-unit_address_vs_reg \
                  -Wno-deprecated_gpio_property \
                  -Wno-interrupts_property \
                  -Wno-avoid_unnecessary_addr_size \
                  -Wno-unique_unit_address"

# help function, it display the usage of this script.
help() {
cat << EOF
    usage:
        `basename $0` <option>

        options:
           -h/--help                     display this help info
           -j[<num>]                     specify the number of parallel jobs when build the target, the number after -j should be greater than 0
           modules list file path         kernel modules list file represented by absolute path


    an example to build the whole system with maximum parallel jobs as below:
        `basename $0` -j


EOF

exit;
}

apply_gki_patchs()
{
    if [ "$BUILD_VARIANT" == "factory" ]; then
        cp -rf ${GKI_PATCHS_PATH}/*.patch ${GKI_COMMON_PATH}/
        gki_patchs=$(ls ${GKI_COMMON_PATH}/*.patch)

        # cd to gki common
        cd ${GKI_COMMON_PATH}
        for patch in ${gki_patchs[@]}
        do
            # Clean dirty files
            git clean -d -f
            # Clean dirty commit
            git reset --hard $(git rev-parse --abbrev-ref --symbolic-full-name @{u})
            git am --whitespace=fix ${patch}
        done
        # cd back from gki common
        cd -
    fi
}

# Offline fdt overlay apply check
fdt_overlay_apply_check()
{
    # Check the dtc version
    cur_dtc_version=$($DTC -v)
    cur_dtc_version=$(echo "$cur_dtc_version" | awk -F'[ :-]' '{print $4}')
    echo "cur_dtc_version=$cur_dtc_version"
    if [[ $cur_dtc_version < $DTC_VERSION ]]; then
        echo -e "\033[31mError: the dtc version must >= 1.6.0!!!\033[0m"
        exit 1
    fi

    # Kernel fdt overlay apply check
    dtbo_files=$(ls ${KERNEL_DIST_PATH}/*.dtbo)

    for file in ${dtbo_files[@]}
    do
        echo "ufdt_apply_overlay ${KERNEL_DIST_PATH}/*basic.dtb ${file} ${KERNEL_DIST_PATH}/${file##*/}.tmp"
        ufdt_apply_overlay ${KERNEL_DIST_PATH}/*basic.dtb ${file} ${KERNEL_DIST_PATH}/${file##*/}.tmp || exit 1
        ${DTC} ${DTC_FLAGS} -s -O dts -o ${KERNEL_DIST_PATH}/${file##*/}.dts ${KERNEL_DIST_PATH}/${file##*/}.tmp || exit 1
    done
}

# handle special args, now it is used to handle the option for make parallel jobs option(-j).
# the number after "-j" is the jobs in parallel, if no number after -j, use the max jobs in parallel.
# kernel now can't be controlled from this script, so by default use the max jobs in parallel to compile.
handle_special_arg()
{
    # options other than -j are all illegal
    local jobs;
    if [ ${1:0:2} = "-j" ]; then
        jobs=${1:2};
        if [ -z ${jobs} ]; then                                                # just -j option provided
            if [ "$KERNEL_VERSION" == "5" ];then
                parallel_option="-j";
            else
                parallel_option="--jobs=1"
            fi
        else
            if [[ ${jobs} =~ ^[0-9]+$ ]] && [ ${jobs} -gt 0 ]; then           # integer bigger than 0 after -j
                if [ "$KERNEL_VERSION" == "5" ];then
                    parallel_option="-j${jobs}";
                else
                    parallel_option="--jobs=${jobs}"
                fi

            else
                echo invalid -j parameter;
                exit;
            fi
        fi
    else
        if [ -f ${arg} ];then \
            list=`cat ${arg}`
        else
            echo Unknown option: ${1};
            help;
        fi
    fi
}

# function_name : clear_cache
# function description:
#      clear copy header file and build_utils.sh
clear_cache() {
    rm -r ${XRING_MODULES_PATH}/include/dt-bindings/xring/platform-specific
    rm -r ${XRING_MODULES_PATH}/drivers/display/dpu/dpu_hw

    if [ "$KERNEL_VERSION" == "6" ];then
        rm ${BUILD_KERNEL_PATH}/build_utils.sh
        mv ${BUILD_KERNEL_PATH}/build_utils.sh.bak ${BUILD_KERNEL_PATH}/build_utils.sh
    fi
}

# function_name : gki_common_repo_clear
# function description:
#      check kernel image version
check_kernel_image_version() {
    local version=$(strings ${KERNEL_DIST_PATH}/vmlinux | grep -m 1 "Linux version")
    echo "${version}"

    local is_dirty=$(echo "${version}" | grep -o "dirty")
    if [ "${is_dirty}" == "dirty" ]; then
        echo -e "\033[31m*********************************************\033[0m"
        echo -e "\033[31m******The kernel build version is dirty******\033[0m"
        echo -e "\033[31m*********************************************\033[0m"
    fi
}

# function_name : link_header_file
# function description:
#      copy platform/O1 to xring-modules/include
link_header_file() {
    if [ -d "${XRING_MODULES_PATH}/include/dt-bindings/xring/platform-specific" ]; then
        rm -r ${XRING_MODULES_PATH}/include/dt-bindings/xring/platform-specific
    fi

    if [ -d "${XRING_MODULES_PATH}/include/dt-bindings/xring/common" ]; then
        rm -r ${XRING_MODULES_PATH}/include/dt-bindings/xring/common
    fi

    if [ -d "${XRING_MODULES_PATH}/drivers/display/dpu/dpu_hw" ]; then
        rm -r ${XRING_MODULES_PATH}/drivers/display/dpu/dpu_hw
    fi

    ln -s -f ${LOCAL_PATH}/../platform/${TARGET_PLATFORM} ${XRING_MODULES_PATH}/include/dt-bindings/xring/platform-specific || exit 1
    # ln -s -f ${LOCAL_PATH}/../platform/common ${XRING_MODULES_PATH}/include/dt-bindings/xring  || exit 1
    ln -s -f ${LOCAL_PATH}/../graphics/dpu_hw ${XRING_MODULES_PATH}/drivers/display/dpu || exit 1
}

# function_name : build_kernel_5_15
build_kernel_5_15() {
    # user
    if [[ "$BUILD_VARIANT" == "user" ]] || [[ "$build_type" == "dtbs" ]] || [[ "$LTO_TYPE" == "full" ]]; then
        if [ "$KERNEL_DEV_BUILD" = "yes" ]; then
            BUILD_CONFIG=dev_common/build.config.$build_config_type LTO=${LTO_TYPE} SKIP_MRPROPER=${INCREMENTAL_COMPILE} EXT_MODULES="$list" build/build.sh ${parallel_option}
        else
            if [ "$build_type" = "dtbs" ]; then
                BUILD_CONFIG=common/build.config.$build_config_type LTO=${LTO_TYPE} SKIP_MRPROPER=${INCREMENTAL_COMPILE} EXT_MODULES="$list"  build/build.sh ${parallel_option}
            else
                BUILD_CONFIG=common/build.config.$build_config_type LTO=${LTO_TYPE} SKIP_MRPROPER=${INCREMENTAL_COMPILE} EXT_MODULES="$list" KMI_SYMBOL_LIST_STRICT_MODE=1 TRIM_NONLISTED_KMI=1 build/build.sh ${parallel_option}
            fi
        fi
    # san
    elif [[ "$BUILD_VARIANT"  =~ ^(san|kasan)$ ]] || [[ "$IMG_TYPE"  =~ ^(san|kasan)$ ]]; then
        if [ "$KERNEL_DEV_BUILD" = "yes" ]; then
            BUILD_CONFIG=dev_common/build.config.${build_config_type}_kasan LTO=none SKIP_MRPROPER=${INCREMENTAL_COMPILE} EXT_MODULES="$list" build/build.sh ${parallel_option}
        else
            BUILD_CONFIG=common/build.config.${build_config_type}_kasan LTO=none SKIP_MRPROPER=${INCREMENTAL_COMPILE} EXT_MODULES="$list" build/build.sh ${parallel_option}
        fi
    # eng
    else
        if [ "$KERNEL_DEV_BUILD" = "yes" ]; then
            BUILD_CONFIG=dev_common/build.config.${build_config_type}_debug LTO=none SKIP_MRPROPER=${INCREMENTAL_COMPILE} EXT_MODULES="$list" build/build.sh ${parallel_option}
        else
            BUILD_CONFIG=common/build.config.${build_config_type}_debug LTO=none SKIP_MRPROPER=${INCREMENTAL_COMPILE} EXT_MODULES="$list" build/build.sh ${parallel_option}
        fi
    fi
}

# function_name : check_endlf
# function description:
#      Checks whether the file ends with a newline character
check_endlf(){
    recovery_files=$(find "$XRING_CONFIG_PATH" -name "recovery_modules.*")
    vendor_boot_files=$(find "$XRING_CONFIG_PATH" -name "vendor_boot_modules.*" -o -name "vendor_boot_modules_eng.*")
    vendor_dlkm_files=$(find "$XRING_CONFIG_PATH" -name "vendor_dlkm_modules.*" -o -name "vendor_dlkm_modules_eng.*")
    files="$recovery_files $vendor_boot_files $vendor_dlkm_files"
    for file in $files; do
        if [ $(tail -n1 $file | wc -l) -eq 1 ] || [ ! -s "$file" ]; then
            continue
        else
            echo "Error: File '$file' does not end with a newline character."
            exit 1
        fi
    done

    echo "All files end with a newline character."
}

# function_name : sign_dtb_img
sign_dtb_img() {
    in_dtb=$1
    out_dtb=$2
    product_type=$(echo $TARGET_PRODUCT | awk -F"_" '{print $NF}')
    echo "product_type=${product_type}"
    cd ${SIGN_TOOLS_PATH}
    ./cert_request.sh image=${in_dtb} sub_image0=${in_dtb} sub_image0_path=${KERNEL_DIST_PATH}/${in_dtb} \
        product_type=${product_type} platform=${TARGET_PLATFORM} build_variant=${BUILD_VARIANT} \
        image_type=${IMG_TYPE} output_path=${KERNEL_DIST_PATH}
    mv ${KERNEL_DIST_PATH}/sec_sdtb.img ${KERNEL_DIST_PATH}/${out_dtb}
    if [ $? -ne 0 ]; then
        echo "Error: Renaming sec_sdtb.img to sec_dtb.img failed"
        cd ${LOCAL_PATH}
        exit 1
    fi
    cd ${LOCAL_PATH}
}


# function_name : create_pack_dtb_img
# function description:
#      build security_dtb.dtb, pack dtbo.img & security_dtb.img
create_pack_dtb_img(){
    echo "Building  dtbo ..."
    # ${TARGET_PLATFORM} = O1
    cp ${XRING_CONFIG_PATH}/${TARGET_PLATFORM}/dtboimg.cfg ${KERNEL_DIST_PATH}/dtboimg.cfg
    sed -i "s/\${BRANCH}/$BRANCH/g" ${KERNEL_DIST_PATH}/dtboimg.cfg
    ${MKDTIMG} cfg_create ${KERNEL_DIST_PATH}/dtbo.img ${KERNEL_DIST_PATH}/dtboimg.cfg

    echo "Building Security dtb ..."
    cp -f ${PREBUILTS_BINARY_PATH}/sec_dtb/sec_dtb.img ${KERNEL_DIST_PATH}/sec_dtb.img
    cp -f ${PREBUILTS_BINARY_PATH}/sec_dtb/sdtb.img ${KERNEL_DIST_PATH}/sdtb.img
}

# function_name : build_kernel_6_6
build_kernel_6_6() {
    # Checks whether the file ends with a newline character
    check_endlf

    # step 1: copy platform/O1 to xring-modules/include
    link_header_file

    # apply gki patchs for factory
    apply_gki_patchs

    if [ "$LTO_TYPE" = "full" -o "$LTO_TYPE" = "thin" ]; then
        LTO_FLAGS="--lto=${LTO_TYPE}"
    else
        LTO_FLAGS="--lto=none --nokmi_symbol_list_strict_mode --notrim --nokmi_symbol_list_violations_check"
    fi
    echo "LTO_FLAGS=${LTO_FLAGS}"
    # copy fstab and determining if it's a product
    if [[ $TARGET_PRODUCT == *"emu" || $TARGET_PRODUCT == *"fpga" || $TARGET_PRODUCT == *"asic" ]]; then
        is_product=""
    else
        is_product=1
        FSTAB_SOURCE_PATH=${LOCAL_PATH}/../../../device/xiaomi/mivendor/config/fstab.O1_asic
        FSTAB_COPY_PATH=${XRING_CONFIG_PATH}/xiaomi/$(echo $TARGET_PRODUCT | awk -F"_" '{print $NF}')
        cp -f $FSTAB_SOURCE_PATH $FSTAB_COPY_PATH
    fi
    # set build's target
    if [ "$TARGET_PLATFORM" = "O1" ]; then
        if [ -z ${is_product} ]; then
            PRODUCT_FLAGS=""
            build_target="${TARGET_PLATFORM}_fullboot"
        else
            ODM_FLAGS="DEVICE_NAME=${DEVICE_NAME},ANDROID_VERSION=${ANDROID_VERSION},BUILD_NUMBER=${BUILD_NUMBER},BUILD_VARIANT=${BUILD_VARIANT}"
            PRODUCT_FLAGS="--action_env=ODM_FLAGS="$ODM_FLAGS""
            build_target="${TARGET_PRODUCT}"
        fi

        if [ "$build_type" = "dtbs" ]; then
            dtb_target="${TARGET_PLATFORM}_dtb"
        fi
    else
        # O3_{asic/emu/fpga}
        if [ -z ${is_product} ]; then
            if [ "$build_type" = "dtbs" ]; then
                dtb_target=${TARGET_PLATFORM}_dtb
            fi
            if [[ $TARGET_PRODUCT == *emu ]]; then
                build_target=${TARGET_PLATFORM}_miniboot
            else
                build_target=${TARGET_PLATFORM}_fullboot
            fi
        else
            build_target=${TARGET_PRODUCT}
        fi
    fi
    echo "------------build_target=${build_target}------------------"

    # step 3: begin compile
    mv ${BUILD_KERNEL_PATH}/build_utils.sh ${BUILD_KERNEL_PATH}/build_utils.sh.bak
    cp -f ${XRING_UTILS_PATH}/scripts/build_utils.sh ${BUILD_KERNEL_PATH}/build_utils.sh

    if [ "${INCREMENTAL_COMPILE}" == "1" ]; then
        echo "alreday set config local: --config=local"
        set_config_local="--config=local"
    fi
    set_config_local="${set_config_local} --config=stamp"
    DEFCONFIG_FRAGMENT=""
    if [ "$build_type" = "dtbs" ]; then
        # dtb
        if [ "$BUILD_VARIANT" = "eng" ];then
            sed -i 's/\/\/#define CONFIG_XRING_DEBUG/#define CONFIG_XRING_DEBUG/g' ${LOCAL_PATH}/xring-modules/include/dt-bindings/xring/xring_dt_debug.h
        fi
        tools/bazel run ${set_config_local} --lto=none $parallel_option //xring-dts:xring_${dtb_target}_dist --action_env=DTC_FLAGS="$DTC_FLAGS"
        if [ "$BUILD_VARIANT" = "eng" ];then
            sed -i 's/#define CONFIG_XRING_DEBUG/\/\/#define CONFIG_XRING_DEBUG/g' ${LOCAL_PATH}/xring-modules/include/dt-bindings/xring/xring_dt_debug.h
        fi
    elif [ "$build_type" = "kernel" ]; then
        if [ "$BUILD_VARIANT" = "menuconfig" ]; then
            # menuconfig
            echo "begin bazel menuconfig"
            tools/bazel run $parallel_option //xring-modules:xring_${build_target}_user_config -- menuconfig
        elif [ "$BUILD_VARIANT" = "update" ]; then
            # update symbol list
            echo "begin bazel update symbol list"
            tools/bazel run --lto=full $parallel_option //xring-modules:xring_${build_target}_user_abi_update_symbol_list --action_env=TARGET_PRODUCT=$TARGET_PRODUCT --action_env=DTC_FLAGS="$DTC_FLAGS" ${PRODUCT_FLAGS}
        elif [ "$BUILD_VARIANT" = "user" ]; then
            # user
            # tools/bazel run --lto=full $parallel_option //xring-modules:xring_${build_target}_user_abi_update_symbol_list --action_env=TARGET_PRODUCT=$TARGET_PRODUCT --action_env=DTC_FLAGS="$DTC_FLAGS" ${PRODUCT_FLAGS}
            tools/bazel run ${set_config_local} ${LTO_FLAGS} $parallel_option //xring-modules:xring_${build_target}_user_dist --action_env=TARGET_PRODUCT=$TARGET_PRODUCT --action_env=DTC_FLAGS="$DTC_FLAGS" ${PRODUCT_FLAGS}
        elif [[ "$BUILD_VARIANT" =~ ^(san|kasan)$ ]]; then
            # kasan
            if [ "$IMG_TYPE" = "fuzz" ]; then
                echo "------------build fuzz------------------"
                DEFCONFIG_FRAGMENT="--defconfig_fragment=xring-configs:xring_fuzz_defconfig"
            fi
            sed -i 's/\/\/#define CONFIG_XRING_DEBUG/#define CONFIG_XRING_DEBUG/g' ${LOCAL_PATH}/xring-modules/include/dt-bindings/xring/xring_dt_debug.h
            tools/bazel run ${set_config_local} $parallel_option --lto=none --nokmi_symbol_list_strict_mode --notrim --nokmi_symbol_list_violations_check ${DEFCONFIG_FRAGMENT} //xring-modules:xring_${build_target}_kasan_dist --action_env=TARGET_PRODUCT=$TARGET_PRODUCT --action_env=DTC_FLAGS="$DTC_FLAGS" ${PRODUCT_FLAGS}
            sed -i 's/#define CONFIG_XRING_DEBUG/\/\/#define CONFIG_XRING_DEBUG/g' ${LOCAL_PATH}/xring-modules/include/dt-bindings/xring/xring_dt_debug.h
        else
            # eng/ate/slt
            version="eng"
            if [ "$BUILD_VARIANT" = "ate" ] || [ "$IMG_TYPE" = "ate" ];then
                echo "------------build ate------------------"
                DEFCONFIG_FRAGMENT="--defconfig_fragment=xring-configs:xring_ate_defconfig"
            elif [ "$BUILD_VARIANT" = "slt" ] || [ "$IMG_TYPE" = "slt" ]; then
                echo "------------build slt------------------"
                version="slt"
            elif [ "$BUILD_VARIANT" = "factory" ]; then
                echo "------------build factory------------------"
                version="factory"
            else
                echo "------------build eng------------------"
            fi
            sed -i 's/\/\/#define CONFIG_XRING_DEBUG/#define CONFIG_XRING_DEBUG/g' ${LOCAL_PATH}/xring-modules/include/dt-bindings/xring/xring_dt_debug.h
            tools/bazel run ${set_config_local} $parallel_option --lto=none --nokmi_symbol_list_strict_mode --notrim --nokmi_symbol_list_violations_check ${DEFCONFIG_FRAGMENT} //xring-modules:xring_${build_target}_${version}_dist --action_env=TARGET_PRODUCT=$TARGET_PRODUCT --action_env=DTC_FLAGS="$DTC_FLAGS" ${PRODUCT_FLAGS}
            sed -i 's/#define CONFIG_XRING_DEBUG/\/\/#define CONFIG_XRING_DEBUG/g' ${LOCAL_PATH}/xring-modules/include/dt-bindings/xring/xring_dt_debug.h
        fi
    fi

    if [ "$BUILD_VARIANT" != "menuconfig" ] && [ "$BUILD_VARIANT" != "update" ] || [ "$build_type" = "dtbs" ]; then
        # step4: pack dtbo.img and sec_dtb.img
        create_pack_dtb_img
        if [ "$build_type" != "dtbs" ];then
            # pack vendor_boot-debug.img
            cp -f ${KERNEL_DIST_PATH}/vendor_boot.img ${KERNEL_DIST_PATH}/vendor_boot-debug.img
            repack_bootimg --local --dst_bootimg ${KERNEL_DIST_PATH}/vendor_boot-debug.img \
              --ramdisk_add ${XRING_CONFIG_PATH}/xiaomi/force_debuggable:force_debuggable\
              --ramdisk_add ${XRING_CONFIG_PATH}/xiaomi/adb_debug.prop:adb_debug.prop \
              --ramdisk_add ${XRING_CONFIG_PATH}/xiaomi/userdebug_plat_sepolicy.cil:userdebug_plat_sepolicy.cil
            if [ "${IMG_TYPE}" = "userroot" ]; then
                cp -f ${KERNEL_DIST_PATH}/vendor_boot-debug.img ${KERNEL_DIST_PATH}/vendor_boot.img
            fi
        fi
    fi

    # check build kernel verrsion
    check_kernel_image_version

    # step 5: clear copy file
    clear_cache
}

args=( "$@" )
for arg in ${args[*]} ; do
    case ${arg} in
        -h) help;;
        --help) help;;
        dtbs) build_type=dtbs;;
        kernel) build_type=kernel;;
        *) handle_special_arg ${arg};;
    esac
done

build_config_type=${TARGET_PLATFORM}
if [ "$build_type" = "dtbs" ]; then
    build_config_type=${TARGET_PLATFORM}_dtb
fi

echo "ext_module_list: $list"

if [ "$IMG_TYPE" = "fuzz" ]; then
    BUILD_VARIANT="kasan"
fi

echo "*****Build Kernel with $BUILD_VARIANT*****"

# Default is OGKI
if [ "$TARGET_GKI_TYPE" == "gki" ]; then
    target_gki_type=${TARGET_GKI_TYPE}
    set -x
    echo "current build gki_type: ${TARGET_GKI_TYPE}"
    if [ -d ${LOCAL_PATH}/common-gki ]; then
        rm -rf ${LOCAL_PATH}/common
        ln -fs ${LOCAL_PATH}/common-gki ${LOCAL_PATH}/common
    else
        echo "Error: not supported gki build, Please check for it!"
        exit 1
    fi
    set +x
else
    set -x
    echo "current build gki_type: ${TARGET_GKI_TYPE}"
    if [ -d ${LOCAL_PATH}/common-ogki ]; then
        rm -rf ${LOCAL_PATH}/common
        ln -fs ${LOCAL_PATH}/common-ogki ${LOCAL_PATH}/common
    else
        echo "Error: not supported ogki build, Please check for it!"
        exit 1
    fi
    set +x
fi

KERNEL_VERSION=$(grep '^VERSION' common/Makefile | awk '{print $3}')
echo "KERNEL_VERSION=$KERNEL_VERSION"
KERNEL_DIR=${LOCAL_PATH}/common
XRING_MODULES_PATH=${LOCAL_PATH}/xring-modules
SIGN_TOOLS_PATH=${LOCAL_PATH}/../tools/hsm_cert_generator
GKI_COMMON_PATH=${LOCAL_PATH}/common
GKI_PATCHS_PATH=${LOCAL_PATH}/xring-utils/gki_patchs
if [ "$KERNEL_VERSION" == "5" ];then
    KERNEL_DIST_PATH=${LOCAL_PATH}/out/android13-5.15/dist
    KERNEL_DEFCONFIG=${LOCAL_PATH}/out/android13-5.15/common/.config
    DTC=${LOCAL_PATH}/out/android13-5.15/common/scripts/dtc/dtc
elif [ "$KERNEL_VERSION" == "6" ];then
    BRANCH=android15-6.6
    KERNEL_DIST_PATH=${LOCAL_PATH}/out/${BRANCH}/dist
    DTC=${LOCAL_PATH}/prebuilts/kernel-build-tools/linux-x86/bin/dtc
    MKDTIMG=prebuilts/kernel-build-tools/linux-x86/bin/mkdtimg
    XRING_CONFIG_PATH=${LOCAL_PATH}/xring-configs
    XRING_UTILS_PATH=${LOCAL_PATH}/xring-utils
    COMMON_TOOLS_PATH=${LOCAL_PATH}/xring-utils/common_tools/bin
    BUILD_KERNEL_PATH=${LOCAL_PATH}/build/kernel
    PREBUILTS_BINARY_PATH=${LOCAL_PATH}/../prebuilts_binary
fi
export PATH="${COMMON_TOOLS_PATH}:${PATH}"

if [ "$KERNEL_VERSION" == "5" ];then
    echo "go to kernel5.15 build entry"
    build_kernel_5_15
elif [ "$KERNEL_VERSION" == "6" ];then
    echo "go to kernel6.6 build entry"
    build_kernel_6_6
fi

if [ "$?" != "0" ]; then
    if [ "$KERNEL_VERSION" == "6" ];then
        clear_cache
    fi
    exit 1
fi

# DTB Overlay check
if [ "$BUILD_VARIANT" != "update" ] && [ "$BUILD_VARIANT" != "menuconfig" ]; then
    fdt_overlay_apply_check
fi
