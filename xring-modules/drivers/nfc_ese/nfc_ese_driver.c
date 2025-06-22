#include "nfc_ese_driver.h"

static struct nfc_ese_device ese;

int ese_parse_dts(struct nfc_ese_device *ese_dev)
{
	int ret;
	if (ese_dev->driver_device != NULL) {
		ese_dev->pinctrl = pinctrl_get(&ese_dev->driver_device->dev);
		if (IS_ERR(ese_dev->pinctrl)) {
			ret = PTR_ERR(ese_dev->pinctrl);
			pr_info("can't find nfc ese pinctrl\n");
			return ret;
		}

		ese_dev->pins_spiio_spi_mode =
			pinctrl_lookup_state(ese_dev->pinctrl, DTS_PINCTL_SPI_DEFAULT);
		if (IS_ERR(ese_dev->pins_spiio_spi_mode)) {
			ret = PTR_ERR(ese_dev->pins_spiio_spi_mode);
			pr_info("can't find pinctrl default mode\n");
			return ret;
		} else {
			ret = pinctrl_select_state(ese_dev->pinctrl, ese_dev->pins_spiio_spi_mode);
			pr_info("nfc ese default mode select\n");
			return ret;
		}
	} else {
		pr_info("platform device is null\n");
		return -EPERM;
	}
}

static int nfc_ese_probe(struct platform_device *driver_device)
{
	struct nfc_ese_device *ese_dev = &ese;
	int status = -EINVAL;
	ese_dev->driver_device = driver_device;

	/* get pinctrl info from dts or defination */
	status = ese_parse_dts(ese_dev);
	if (status) {
		goto err_dts;
	}
	pr_info( "nfc_ese probe success" );
	return 0;
err_dts:
	ese_dev->driver_device = NULL;
	pr_info( "nfc_ese probe fail\n" );
	return status;
}

static int nfc_ese_remove(struct platform_device *driver_device)
{
	struct nfc_ese_device *ese_dev = &ese;
	ese_dev->driver_device = NULL;
	return 0;
}

static const struct of_device_id nfc_ese_of_match[] = {
	{.compatible = DRIVER_COMPATIBLE,},
	{},
};
MODULE_DEVICE_TABLE(of, nfc_ese_of_match);


static struct platform_driver nfc_ese_platform_driver = {
	.driver = {
		.name = NFC_ESE_DEV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = nfc_ese_of_match,
		},
	.probe = nfc_ese_probe,
	.remove = nfc_ese_remove,
};

/*-------------------------------------------------------------------------*/
static int __init nfc_ese_init(void)
{
	int status = 0;
	status = platform_driver_register(&nfc_ese_platform_driver);
	if (status < 0) {
		pr_debug( "Failed to register nfc ese driver.\n");
		return -EINVAL;
	}
	return status;
}

module_init(nfc_ese_init);

static void __exit nfc_ese_exit(void)
{
	platform_driver_unregister(&nfc_ese_platform_driver);
	pr_info("%s: Unloading NFC ese driver\n", __func__);
}

module_exit(nfc_ese_exit);

MODULE_AUTHOR("xiaomi");
MODULE_DESCRIPTION("Xiaomi nfc ese driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("xiaomi-nfcese");
