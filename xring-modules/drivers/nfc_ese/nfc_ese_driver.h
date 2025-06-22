#ifndef NFC_ESE_DRIVER_H
#define NFC_ESE_DRIVER_H

/**************************debug******************************/
#define DEBUG

#include <linux/types.h>
#include <linux/input.h>
#include <linux/device.h>
#include <linux/gpio.h>

#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/pinctrl/consumer.h>

#define NFC_ESE_DEV_NAME "xiaomi-nfcese"
#define DRIVER_COMPATIBLE "xiaomi,xiaomi-nfcese"
#define DTS_PINCTL_SPI_INIT          "init"
#define DTS_PINCTL_SPI_DEFAULT          "default"
#define DTS_PINCTL_SPI_SLEEP          "sleep"

struct nfc_ese_device {
	struct platform_device *driver_device;
	struct pinctrl *pinctrl;
	struct pinctrl_state *pins_spiio_spi_mode;
};
#endif /* NFC_ESE_DRIVER_H */
