#include <linux/init.h>
#include <linux/module.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/acpi.h>
#include <linux/of.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
// #include <linux/regulator/consumer.h>
// #include <drm/drm_notifier_mi.h>
#include <linux/notifier.h>
#include <linux/interrupt.h>
#include "nano_macro.h"
#include <linux/regmap.h>
#include <linux/../../drivers/base/regmap/internal.h>
#include <linux/regulator/driver.h>
#include <linux/../../drivers/regulator/internal.h>


extern uint32_t get_hw_id_value(void);

#ifdef ENABLE_PMIC_DUMP
void dump_regmap(struct nano_i2c_client *i2c_client, int base)
{
    struct regmap *map = i2c_client->map;
    int buf[257];
    int addr = 0;
    int i = 0;

    if (IS_ERR_OR_NULL(map))
    {
        dbgprint(INFO_LEVEL, "regmap error");
        return;
    }

    i = 0;
    memset(buf, 0, sizeof(buf));

    for (addr = base; addr <= base+0xFF; addr++)
        regmap_read(map, addr, &buf[i++]);

    dbgprint(INFO_LEVEL, "addr_base: %04x", base);
    for (i = 0; i < 32; i++)
    {
        dbgprint(INFO_LEVEL, "%s: %04x:0x%04x %04x:0x%04x %04x:0x%04x %04x:0x%04x %04x:0x%04x %04x:0x%04x %04x:0x%04x %04x:0x%04x", regmap_name(map),
                    base+i*8+0, buf[i*8+0], base+i*8+1, buf[i*8+1], base+i*8+2, buf[i*8+2], base+i*8+3, buf[i*8+3],
                        base+i*8+4, buf[i*8+4], base+i*8+5, buf[i*8+5], base+i*8+6, buf[i*8+6], base+i*8+7, buf[i*8+7]);
    }

}

static int my_device_match(struct device *dev, void *data)
{
    struct nano_i2c_client *i2c_client = data;

    if (IS_ERR_OR_NULL(i2c_client->map))
    {
        i2c_client->map = dev_get_regmap(dev, NULL);
        dbgprint(INFO_LEVEL, "child's name is %s, get reg map %s", dev_name(dev), !IS_ERR_OR_NULL(i2c_client->map) ? "success" : "fail");
        if (!IS_ERR_OR_NULL(i2c_client->map))
        {
            dbgprint(INFO_LEVEL, "reg map name is %s", regmap_name(i2c_client->map));
            if (!sysfs_streq(regmap_name(i2c_client->map), REGMAP_NAME))
            {
                dbgprint(INFO_LEVEL, "reg map name is not %s, clear map", REGMAP_NAME);
                i2c_client->map = NULL;
            }
        }
    }

    if (IS_ERR_OR_NULL(i2c_client->map))
        device_find_child(dev, (void *)i2c_client, my_device_match);

    return !IS_ERR_OR_NULL(i2c_client->map);
}

struct device *find_device_by_regmap(struct device *parent, struct nano_i2c_client *i2c_client)
{
    return device_find_child(parent, (void *)i2c_client, my_device_match);
}
#endif // ENABLE_PMIC_DUMP


int Nanosic_regulator_init(struct nano_i2c_client *I2client, struct device_node *of_node, struct i2c_client *client)
{
    int ret = 0;
#ifdef ENABLE_PMIC_DUMP
    struct device *spmi_device_dev;
    struct device_node *np = client->dev.of_node;
    struct device_node *spmi_device_np;
#endif // ENABLE_PMIC_DUMP

    ret = of_property_read_string(of_node, "nanosic,vdd-name", &(I2client->vdd_name));
    if (!ret)
    {
        dbgprint(ALERT_LEVEL, "get vdd-name %s", I2client->vdd_name);

        I2client->vdd_reg = devm_regulator_get(&client->dev, I2client->vdd_name);
        if (IS_ERR_OR_NULL(I2client->vdd_reg))
        {
            dbgprint(ERROR_LEVEL, "Failed to get %s regulator: %ld\n", I2client->vdd_name, PTR_ERR(I2client->vdd_reg));
            ret = PTR_ERR(I2client->vdd_reg);
            goto _err_vdd_reg;
        }
#ifdef ENABLE_PMIC_DUMP
        I2client->vdd_addr_base = L13B_ADDR_BASE;
#endif // ENABLE_PMIC_DUMP
    }
    else
    {
        dbgprint(ERROR_LEVEL, "Failed to get vdd-name: %d\n", ret);
        goto _err_get_vdd_name;
    }
        /* use L6B as dvdd before P1.1 */
        ret = of_property_read_string(of_node, "nanosic,dvdd-name", &(I2client->dvdd_name));
        if (!ret)
        {
            dbgprint(ALERT_LEVEL, "get dvdd-name %s", I2client->dvdd_name);

            I2client->dvdd_reg = devm_regulator_get(&client->dev, I2client->dvdd_name);
            if (IS_ERR_OR_NULL(I2client->dvdd_reg))
            {
                dbgprint(ERROR_LEVEL, "Failed to get %s regulator: %ld\n", I2client->dvdd_name, PTR_ERR(I2client->dvdd_reg));
                ret = PTR_ERR(I2client->vdd_reg);
                goto _err_dvdd_reg;
            }
#ifdef ENABLE_PMIC_DUMP
            I2client->dvdd_addr_base = L6B_ADDR_BASE;
#endif // ENABLE_PMIC_DUMP
        }
        else
        {
            dbgprint(ERROR_LEVEL, "Failed to get dvdd-name: %d\n", ret);
            goto _err_get_dvdd_name;
        }

    dbgprint(ALERT_LEVEL, "set vdd 3300000-3300000");
    ret = regulator_set_voltage(I2client->vdd_reg, 3300000, 3300000);
    if (ret)
    {
        dbgprint(ERROR_LEVEL, "Failed to set %s voltage: %d\n", I2client->vdd_name, ret);
        goto _err_set_vdd_voltage;
    }

    dbgprint(ALERT_LEVEL, "set dvdd 1800000-1800000");
    ret = regulator_set_voltage(I2client->dvdd_reg, 1800000, 1800000);
    if (ret)
    {
        dbgprint(ERROR_LEVEL, "Failed to set %s voltage: %d\n", I2client->dvdd_name, ret);
        goto _err_set_dvdd_voltage;
    }

#ifdef ENABLE_PMIC_DUMP
    spmi_device_np = of_parse_phandle(np, "spmi-device", 0);
    if (!spmi_device_np)
    {
        dbgprint(ERROR_LEVEL, "Failed to find spmi-device device\n");
    }

    spmi_device_dev = &of_find_device_by_node(spmi_device_np)->dev;
    if (!spmi_device_dev)
    {
        dbgprint(ERROR_LEVEL, "Failed to get spmi_device device\n");
    }

    dbgprint(INFO_LEVEL, "%s try to get regmap\n", dev_name(spmi_device_dev));
    find_device_by_regmap(spmi_device_dev, I2client);

    dbgprint(INFO_LEVEL, "get regmap %s!\n", !IS_ERR_OR_NULL(I2client->map) ? "success" : "fail");
#endif // ENABLE_PMIC_DUMP

    dbgprint(ALERT_LEVEL, "regulator init success!");

    return ret;


_err_set_dvdd_voltage:
_err_set_vdd_voltage:
_err_get_dvdd_name:
_err_dvdd_reg:
_err_get_vdd_name:
_err_vdd_reg:

    return ret;

}

int Nanosic_switch_regulator(struct nano_i2c_client *I2client, bool on)
{
    int ret = 0;

    if (on)
    {
        /* turn on vdd regulator */
        dbgprint(ALERT_LEVEL, "enable vdd start!\n");
        ret = regulator_enable(I2client->vdd_reg);
        if (ret)
        {
            dbgprint(ERROR_LEVEL, "Failed to enable vdd regulator: %d\n", ret);
            return ret;
        }
        // dbgprint(ALERT_LEVEL, "enable vdd success!\n");
        /* spec 0 ~ 500us */

        /* turn on dvdd regulator */
        // dbgprint(ALERT_LEVEL, "enable dvdd start!\n");
        ret = regulator_enable(I2client->dvdd_reg);
        if (ret)
        {
            dbgprint(ERROR_LEVEL, "Failed to enable dvdd regulator: %d\n", ret);
            goto err_enable_vdd;
        }
        // dbgprint(ALERT_LEVEL, "enable dvdd success!\n");

        dbgprint(ALERT_LEVEL, "regulator enable success!\n");
    }
    else
    {
        /* turn off dvdd regulator */
        ret = regulator_disable(I2client->dvdd_reg);
        if (ret)
        {
            dbgprint(ERROR_LEVEL, "Failed to disable dvdd regulator: %d\n", ret);
            return ret;
        }

        /* spec 0 ~ 500us */

        /* turn off vdd regulator */
        ret = regulator_disable(I2client->vdd_reg);
        if (ret)
        {
            dbgprint(ERROR_LEVEL, "Failed to disable vdd regulator: %d\n", ret);
            return ret;
        }
        dbgprint(ALERT_LEVEL, "regulator disable success!\n");
    }

#ifdef ENABLE_PMIC_DUMP
    dump_regmap(I2client, I2client->dvdd_addr_base);
    dump_regmap(I2client, I2client->vdd_addr_base);
#endif // ENABLE_PMIC_DUMP

    return ret;

err_enable_vdd:
    regulator_disable(I2client->vdd_reg);

    return ret;
}

