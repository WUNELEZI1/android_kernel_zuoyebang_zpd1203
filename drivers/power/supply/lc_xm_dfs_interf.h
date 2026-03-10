#ifndef LC_XM_DFS_INTERF_H
#define LC_XM_DFS_INTERF_H

#define XM_CHG_DFS_FEATURE 1

#if defined(XM_CHG_DFS_FEATURE)

enum xm_chg_dfx_type {
    CHG_DFX_DEFAULT,
    CHG_DFX_PD_AUTH_ERR,
    CHG_DFX_NONE_STANDARD_CHG,
    CHG_DFX_CORROSION_DISCHARGE,
    CHG_DFX_LPD_DISCHARGE,
    CHG_DFX_CP_IBUS_OCP,
    CHG_DFX_CP_IBAT_OCP,
    CHG_DFX_BATT_CYCLE_COUNT,
    CHG_DFX_UISOC_NOT_FULL,
    CHG_DFX_FG_IIC_ERR,
    CHG_DFX_CP_I2C_ERR,
    CHG_DFX_BATT_LINKER_ABSENT,
    CHG_DFX_BATT_AUTH_ERR,
    CHG_DFX_LPD_DISCHARGE_RESET,

    CHG_DFX_MAX_INDEX,
};

struct dfs_data_cp_i2c_err{
    int master_ok;
    int slave_ok;
};

struct dfs_data_battery {
    int vbat;
    int uisoc;
    int rawsoc;
    int cycle;
};

struct dfx_data_struct {
    int adapter_id;
    struct dfs_data_cp_i2c_err data_cp;
    struct dfs_data_battery data_batt;
};
struct dfs_data_info{
    char *dev_name;
    enum xm_chg_dfx_type type;
    struct dfx_data_struct dfx_data;
};
extern int register_xm_charger_dfs_notifier( struct notifier_block *nb );
extern void xm_charger_dfs_notify(enum xm_chg_dfx_type dfx_type, char *module_name);
extern int unregister_xm_charger_dfs_notifier( struct notifier_block *nb );

#endif // XM_CHG_DFS_FEATURE

#endif // LC_XM_DFS_INTERF_H
