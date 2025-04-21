#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(led);

#include "led.h"
#include "hsv2rgb.h"


#define STRIP_NODE		DT_ALIAS(led_strip)

#if DT_NODE_HAS_PROP(DT_ALIAS(led_strip), chain_length)
#define STRIP_NUM_PIXELS	DT_PROP(DT_ALIAS(led_strip), chain_length)
#else
#error Unable to determine length of LED strip
#endif

#define MAX_BRIGHTNESS 50

static struct led_rgb pixels[STRIP_NUM_PIXELS];

static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);


#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }

static struct led_rgb color = RGB(0x00, 0x00, MAX_BRIGHTNESS);

int ledControl() {
	int rc;

	if (device_is_ready(strip)) {
		LOG_INF("Found LED strip device %s", strip->name);
	} else {
		LOG_ERR("LED strip device %s is not ready", strip->name);
		return 0;
	}

	LOG_INF("Displaying pattern on strip");

	int idx = 0;
	while (true) {
		memset(&pixels, 0x00, sizeof(pixels));
		for (size_t cursor = 0; cursor < STRIP_NUM_PIXELS; cursor++) {
			color = hsv2rgb((cursor + idx) % 360, 100, (MAX_BRIGHTNESS)/255.0 * 100);
			memcpy(&pixels[cursor], &color, sizeof(struct led_rgb));
		}
		// idx = (idx + 1) % (3 * STRIP_NUM_PIXELS);
		idx = (idx + 8) % 360;


		rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
		if (rc) {
			LOG_ERR("couldn't update strip: %d", rc);
		}

		k_sleep(K_MSEC(500));
		

	}
}