#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/fuel_gauge.h>


LOG_MODULE_REGISTER(battery);


#include "battery.h"

#define BAT_NODE		DT_ALIAS(bat_ind)


#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }


static const struct device *fuel_gauge_dev = DEVICE_DT_GET(DT_NODELABEL(fuelguage));
// static const struct device *fuel_gauge_dev;
static const struct device *const led = DEVICE_DT_GET(BAT_NODE);


static struct led_rgb ind_color;

int runBattery() {

    union fuel_gauge_prop_val val;

    if (!device_is_ready(led)) {
        LOG_ERR("Battery LED not ready");
        return 0;
    }

    if (!device_is_ready(fuel_gauge_dev)) {
        LOG_ERR("Fuel gauge device not ready");
        memcpy(&ind_color, &(struct led_rgb) RGB(0, 0, 255), sizeof(struct led_rgb));
        while (true) {
            led_strip_update_rgb(led, &ind_color, 1);
            k_sleep(K_MSEC(5000));

        }
        return 0;
    }

    while (1) {
        if (fuel_gauge_get_prop(fuel_gauge_dev, FUEL_GAUGE_VOLTAGE, &val) < 0) {
            LOG_ERR("Failed to get voltage");
            continue;
        }
        LOG_INF("Battery Voltage: %d mV", val.voltage);

        memcpy(&ind_color, &(struct led_rgb) RGB(0, 255, 0), sizeof(struct led_rgb));
        led_strip_update_rgb(led, &ind_color, 1);

        k_sleep(K_MSEC(5000));
    }
}