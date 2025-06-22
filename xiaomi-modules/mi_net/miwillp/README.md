The MiWiLL Engine in the Kernel Area
====

This directory contains 
* the `MiWiLL` Engine, 
 
* the `MiWiLL` modes, and 
* the Device driver for the virtual `MiWiLL` networking device (annotated as "miw0"-like device);


# Protocol Design #

The packet layout defined in `include/miwill_proto.h`;

    >  * MiWiLL Header definition V0 : total length = 1 + 7 + 4 + 2 + 2 = 16
    >  * +--------+--------+--------------+----------+--------+-------------+-------------
    >  * | version|  MODE  |     OPTION   | SRC-ID   | LENGTH | ENCAP-PROTO | PAYLOAD
    >  * +--------+--------+--------------+----------+--------+-------------+-------------
    >  * |  0.5   |   0.5  |      7       |     4    |    2   |       2     |


# GKI Kernel Adoption #

Following the latest GKI regulation from google, the kernel symbols required by specific module (e.g. miwill) from vendor should be appended to the file called `abi_gki_aarch64_<vendor>` located in `kernel_platform/common/android/` directory.

The development of new kernel modules should utilize the original gki symbols as much as possible. Upstreaming new abi symbols of the `abi_gki_aarch64_xiaomi` file to Google requires one longtime process, and we should avoid it.

## The Rule ##
For developement convenience, we could put the symbols to this file, but we should drop or replace them before ship to the production.

## The symbol list ##

The current symbol list includes:
```plaintext
  pernet_ops_rwsem
  dev_mc_flush
  dev_uc_flush
  dev_disable_lro
  netdev_has_upper_dev
  dev_set_mac_address
  dev_set_allmulti
  dev_set_promiscuity
  dev_remove_pack
  dev_add_pack
```

## Howto ##

step 1: Adding the symbols to the the `abi_gki_aarch64_xiaomi` file;
step 2: Re-compile the kernel, by running `RECOMPILE_KERNEL=1 ./kernel_platform/build/android/prepare_vendor.sh`;
step 3: Enter the "miwillp/engine" directory, which hosts the miwill kernel-side source code;
step 4: Re-run the `mm .` or `mmm` command

