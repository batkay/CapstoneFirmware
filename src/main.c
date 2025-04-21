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

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include "audio.h"
#include "gesture.h"
#include "led.h"
#include "servo.h"
#include "battery.h"
#include "switch.h"
#include "ble_led.h"

#define DELAY_TIME K_MSEC(50)

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

// BLE characteristics
// define bluetooth characteristic UUID
#define RX_CHARACTERISTIC  0x94, 0xF5, 0x56, 0x67, 0x86, 0x49, 0x1D, 0xA4, \
			                    0x34, 0x41, 0x32, 0x69, 0x00, 0x0D, 0x9F, 0x9D
#define RX_CHARACTERISTIC_UUID  BT_UUID_DECLARE_128(RX_CHARACTERISTIC)

static K_SEM_DEFINE(ble_init_ok, 0, 1);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_LEN (sizeof(DEVICE_NAME)-1)

static uint8_t mfg_data[] = { 'B', 'a', 't', 'k', 'a', 'y' };

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, RX_CHARACTERISTIC),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, 6),
};

static struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

// bluetooth connection callback
static void connected(struct bt_conn *conn, uint8_t err)
{
	#ifdef DEBUG
	if (err) {
		printk("Connection failed, err 0x%02x %s\n", err, bt_hci_err_to_str(err));
	} else {
		printk("Connected\n");
	}
	#endif

}

// bluetooth disconnect callback
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	#ifdef DEBUG
	printk("Disconnected, reason 0x%02x %s\n", reason, bt_hci_err_to_str(reason));
	#endif

	// if (get_modified_name()) {
	// 	get_name(name, MAX_NAME_LENGTH + 1);
	// }
	// sd ->data = name;
	// sd ->data_len = strlen(name);

	// // bt_le_adv_stop();

	// bt_conn_unref(conn);
	bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	// bt_conn_ref(conn);
	// currConn = NULL;
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};
static struct led_rgb bluetoothColor;

// Define bluetooth characteristics
BT_GATT_SERVICE_DEFINE(led_service,
	BT_GATT_PRIMARY_SERVICE(RX_CHARACTERISTIC_UUID),
	BT_GATT_CHARACTERISTIC(LED_SERVICE_UUID,
				BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE | BT_GATT_CHRC_EXT_PROP,
				BT_GATT_PERM_READ | BT_GATT_PERM_WRITE | BT_GATT_PERM_PREPARE_WRITE,
				read_led, on_receive_led, &bluetoothColor),
);

// Initialize bluetooth
static void bt_ready()
{
	int err = 0;

	//Start advertising
	err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
	if (err) 
	{
		#ifdef DEBUG
		printk("Advertising failed to start (err %d)\n", err);
		#endif
		return;
	}
	#ifdef DEBUG
	printk("Advertising successfully started\n");
	#endif

	//Initalize services
	k_sem_give(&ble_init_ok);

}

// test method to try disconnecting all connected devices
static void disconnect_device(struct bt_conn *conn, void *user_data)
{
    int err;
    struct bt_conn_info info;

	/* Check if the connection is still active */
    /* Retrieve connection information */
    err = bt_conn_get_info(conn, &info);
    if (err) {
		#ifdef DEBUG
        printk("Failed to get connection info: %d\n", err);
		#endif
        return;
    }

    /* Check if the connection is in the connected state */
    if (info.state != BT_CONN_STATE_CONNECTED || info.state != BT_CONN_STATE_CONNECTING) {
		#ifdef DEBUG
        printk("Device already disconnected or in invalid state\n");
		#endif
        return;
    }

    /* Disconnect the device */
    err = bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
	#ifdef DEBUG
    if (err) {
        printk("Failed to disconnect device: %d\n", err);
    } else {
        printk("Device disconnected successfully\n");
    }
	#endif
}

int main(void) {
    start();
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

K_THREAD_DEFINE(bat_id, STACKSIZE, runBattery, NULL, NULL, NULL,
	PRIORITY, 0, 0);