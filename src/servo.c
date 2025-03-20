#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include "servo.h"

static const struct pwm_dt_spec servo = PWM_DT_SPEC_GET(DT_NODELABEL(servo));
static const uint32_t min_pulse = DT_PROP(DT_NODELABEL(servo), min_pulse);
static const uint32_t max_pulse = DT_PROP(DT_NODELABEL(servo), max_pulse);


#define STEP PWM_USEC(100)

enum direction {
	DOWN,
	UP,
};

int servoCtrl() {
	uint32_t pulse_width = min_pulse;
	enum direction dir = UP;
	int ret;

	printk("Servomotor control\n");

	if (!pwm_is_ready_dt(&servo)) {
		printk("Error: PWM device %s is not ready\n", servo.dev->name);
		return 0;
	}

	while(true) {
		ret = pwm_set_pulse_dt(&servo, pulse_width);
		if (ret < 0) {
			printk("Error %d: failed to set pulse width\n", ret);
			return 0;
		}

		if (dir == DOWN) {
			if (pulse_width <= min_pulse) {
				dir = UP;
				pulse_width = min_pulse;
			} else {
				pulse_width -= STEP;
			}
		} else {
			pulse_width += STEP;

			if (pulse_width >= max_pulse) {
				dir = DOWN;
				pulse_width = max_pulse;
			}
		}
		k_sleep(K_SECONDS(1));
	}
	return 0;
}