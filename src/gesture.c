#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/devicetree.h>

#include "gesture.h"
#include "switch.h"

const struct device *const gesture = DEVICE_DT_GET(DT_NODELABEL(apds9960));

int gestureSample() {
	struct sensor_value intensity, pdata;
	int ret = 0;

	while(!enabled()) {}
	k_sleep(K_MSEC(500));


	printk("APDS9960 sample application\n");
	ret = device_init(gesture);
	if (ret) {
		printk("Sensor failed init %i \n", ret);
		return 0;
	}
	if (!device_is_ready(gesture)) {
		printk("sensor: device not ready.\n");
		return 0;
	}

	while (true) {
		// if (!device_is_ready(gesture)) {
		// 	printk("sensor: device not ready.\n");
		// 	return 0;
		// }
		k_sleep(K_MSEC(500));

		// if (!device_init(gesture)) {
		// 	printk("Sensor failed init\n");
		// 	return 0;
		// }

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