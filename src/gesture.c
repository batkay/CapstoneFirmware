#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/devicetree.h>

#include "gesture.h"

const struct device *const gesture = DEVICE_DT_GET(DT_NODELABEL(apds9960));

int gestureSample() {
	struct sensor_value intensity, pdata;

	printk("APDS9960 sample application\n");
	device_init(gesture);
	if (!device_is_ready(gesture)) {
		printk("sensor: device not ready.\n");
		return 0;
	}

	while (true) {
		if (sensor_sample_fetch(gesture)) {
			printk("sensor_sample fetch failed\n");
		}

		sensor_channel_get(gesture, SENSOR_CHAN_LIGHT, &intensity);
		sensor_channel_get(gesture, SENSOR_CHAN_PROX, &pdata);

		printk("ambient light intensity %d, proximity %d\n",
		       intensity.val1, pdata.val1);
		
		k_sleep(K_MSEC(500));
	}
}