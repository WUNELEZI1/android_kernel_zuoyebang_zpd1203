
#ifndef SUBPMIC_PROTOCOL_H
#define SUBPMIC_PROTOCOL_H

#include <linux/iio/iio.h>
#include <linux/iio/consumer.h>
#include <linux/power_supply.h>
#include <dt-bindings/iio/qti_power_supply_iio.h>

struct xm_subpmic_protocol {
	struct device *dev;
	struct iio_dev	*indio_dev;
	struct iio_chan_spec	*iio_chan;
	struct iio_channel	*int_iio_chans;
};

#endif /*SUBPMIC_PROTOCOL_H*/

