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
#include <zephyr/drivers/led_strip.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/sensor.h>

#include "audio.h"
#include "gesture.h"
#include "led.h"
#include "servo.h"

#define DELAY_TIME K_MSEC(50)

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

K_THREAD_DEFINE(servo_id, STACKSIZE, servoCtrl, NULL, NULL, NULL,
	PRIORITY, 0, 0);

K_THREAD_DEFINE(gesture_id, STACKSIZE, gestureSample, NULL, NULL, NULL,
	PRIORITY, 0, 0);

K_THREAD_DEFINE(led_id, STACKSIZE, ledControl, NULL, NULL, NULL,
	PRIORITY, 0, 0);

K_THREAD_DEFINE(audio_id, 64 * STACKSIZE, audioSample, NULL, NULL, NULL,
	PRIORITY, 0, 0);