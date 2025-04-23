/*
 * Copyright (c) 2017 Linaro Limited
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2024 TOKITA Hiroshi
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#include <zephyr/drivers/led_strip.h>

#include <zephyr/sys/util.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/regulator.h>
#include <zephyr/sys/poweroff.h>

#include "gesture.h"
#include "led.h"
#include "servo.h"
#include "switch.h"

#define DELAY_TIME K_MSEC(50)

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

#define BUTTON0	DT_ALIAS(sw0)
static struct gpio_callback button_cb_data;

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(BUTTON0, gpios, {0});

void buttonPressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
	int err;
	err = gpio_pin_interrupt_configure_dt(&button, 	GPIO_INT_LEVEL_ACTIVE);
	if (err) {
		printf("Could not configure sw0 GPIO interrupt (%d)\n", err);
		return;
	}
	end();
	sys_poweroff();
}

int main(void) {
	int err = 0;
    start();

	if (!gpio_is_ready_dt(&button)) {
		printk("Error: button device %s is not ready\n",
				button.port->name);
		return 0;
	}

	err = gpio_pin_configure_dt(&button, (GPIO_INPUT));
	if (err) {
		printk("Error %d: failed to configure button", err);
		return 0;
	}

	err = gpio_pin_interrupt_configure_dt(&button, 	GPIO_INT_EDGE_TO_INACTIVE);

	if (err) {
		printf("Could not configure sw0 GPIO interrupt (%d)\n", err);

		return 0;
	}
	
	gpio_init_callback(&button_cb_data, buttonPressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);

	printk("start");
	return 0;
}



K_THREAD_DEFINE(servo_id, STACKSIZE, servoCtrl, NULL, NULL, NULL,
	PRIORITY + 1, 0, 0);

K_THREAD_DEFINE(gesture_id, STACKSIZE, gestureSample, NULL, NULL, NULL,
	PRIORITY, 0, 0);

K_THREAD_DEFINE(led_id, STACKSIZE, ledControl, NULL, NULL, NULL,
	PRIORITY + 2, 0, 0);
