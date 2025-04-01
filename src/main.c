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

#include <zephyr/sys/util.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/regulator.h>



#include "audio.h"
#include "gesture.h"
#include "led.h"
#include "servo.h"
#include "battery.h"

#define DELAY_TIME K_MSEC(50)

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

#define LED_ENABLE DT_ALIAS(led_enable)


// #if !DT_NODE_EXISTS(LED_ENABLE)
// #error "Overlay for power output node not properly defined."
// #endif

const struct device *enable_switch = DEVICE_DT_GET(LED_ENABLE);

int main(void) {
	int err;
	// setup load switch GPIO
	if (!device_is_ready(enable_switch)) {
        printk("Regulator device not ready\n");
        return 0;
    }

	int ret = regulator_enable(enable_switch);
    if (ret < 0) {
        printk("Failed to enable regulator: %d\n", ret);
    } else {
        printk("Regulator enabled\n");
    }
	printf("enable on\n");



	return 0;
}



K_THREAD_DEFINE(servo_id, STACKSIZE, servoCtrl, NULL, NULL, NULL,
	PRIORITY, 0, 0);

K_THREAD_DEFINE(gesture_id, STACKSIZE, gestureSample, NULL, NULL, NULL,
	PRIORITY, 0, 0);

K_THREAD_DEFINE(led_id, STACKSIZE, ledControl, NULL, NULL, NULL,
	PRIORITY, 0, 0);

K_THREAD_DEFINE(audio_id, 64 * STACKSIZE, audioSample, NULL, NULL, NULL,
	PRIORITY, 0, 0);

// K_THREAD_DEFINE(bat_id, STACKSIZE, runBattery, NULL, NULL, NULL,
// 	PRIORITY, 0, 0);