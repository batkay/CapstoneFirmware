#include <errno.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#include <zephyr/sys/util.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/regulator.h>

#include "switch.h"

#define LED_ENABLE DT_ALIAS(led_enable)

const struct device *enable_switch = DEVICE_DT_GET(LED_ENABLE);
static volatile bool on = false;

bool enabled() {
    return on;
}

void start() {
    on = false;
	// setup load switch GPIO
	if (!device_is_ready(enable_switch)) {
        printk("Regulator device not ready\n");
        return;
    }

	int ret = regulator_enable(enable_switch);
    if (ret < 0) {
        printk("Failed to enable regulator: %d\n", ret);
    } else {
        printk("Regulator enabled\n");
    }
	printf("enable on\n");
    on = true;
}

void end() {
    int ret = regulator_disable(enable_switch);
    if (ret < 0) {
        printk("Failed to disable regulator: %d\n", ret);
    } else {
        printk("Regulator disabled\n");
    }
}