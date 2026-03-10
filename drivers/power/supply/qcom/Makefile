# SPDX-License-Identifier: GPL-2.0-only

obj-$(CONFIG_QPNP_SMB5) += qpnp-smb5-main.o
qpnp-smb5-main-y += step-chg-jeita.o battery.o qpnp-smb5.o smb5-lib.o pmic-voter.o storm-watch.o schgm-flash.o battery-profile-loader.o smb5-iio.o smbx-get-chan.o
obj-$(CONFIG_THIRD_GAUGE) += fg_bq28z610.o
fg_bq28z610-objs := bq28z610.o lc_notify.o lc_adaptive_poweroff_voltage.o
obj-$(CONFIG_THIRD_GAUGE)	+= gauge-iio.o
obj-$(CONFIG_CHARGE_PARTITION)	+= charger_partition.o
obj-$(CONFIG_SMB1398_CHARGER)           += qcom-smb1398-charger.o
qcom-smb1398-charger-y += smb1398-charger.o
obj-$(CONFIG_SMB1355_SLAVE_CHARGER)	+= qcom-smb1355-charger.o
qcom-smb1355-charger-y += smb1355-charger.o
obj-$(CONFIG_SMB358_CHARGER)    += smb358-charger.o
obj-$(CONFIG_BQ25960_CHARGER) += bq25960_charger.o
obj-$(CONFIG_BQ25960_CHARGER) += bq25960h_charger.o
obj-$(CONFIG_SC853X_CHARGER)  += sc853x_charger.o
obj-$(CONFIG_PD_POLICY_MANAGER) += pd_policy_manager.o
obj-$(CONFIG_LC_CHARGER_SYSFS)  += lc_charger_sysfs-main.o
lc_charger_sysfs-main-y += lc_charger_sysfs.o lc_charger_sysfs_iio.o
obj-$(CONFIG_CP_QC30) += cp_qc30.o
obj-$(CONFIG_XIAOMI_SMART_CHG) += xm_smart_chg.o

