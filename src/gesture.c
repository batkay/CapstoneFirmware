#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/devicetree.h>

#include <zephyr/sys/util.h>

#include "gesture.h"
#include "switch.h"

const struct device *const gesture = DEVICE_DT_GET(DT_NODELABEL(apds9960));
struct sensor_value intensity, pdata;
static bool ready;

uint32_t getProximity() {
	return pdata.val1;
}

bool proximityReady() {
	bool ret = ready;
	ready = false;
	return ret;
}

int gestureSample() {
	int ret = 0;

	while(!enabled()) {
		printk("Waiting");
	}
	k_sleep(K_MSEC(100));


	printk("APDS9960 thread\n");
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
		if (sensor_sample_fetch(gesture)) {
			printk("sensor_sample fetch failed\n");
			continue;
		}
		sensor_channel_get(gesture, SENSOR_CHAN_PROX, &pdata);
		ready = true;

		printk("ambient light intensity %d, proximity %d\n",
		       intensity.val1, pdata.val1);
		
		// k_sleep(K_MSEC(50));
	}
}