#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include "servo.h"
#include "gesture.h"

#define NUM_SERVO 6

static const struct pwm_dt_spec ser0 = PWM_DT_SPEC_GET(DT_NODELABEL(servo0));
static const struct pwm_dt_spec ser1 = PWM_DT_SPEC_GET(DT_NODELABEL(servo1));
static const struct pwm_dt_spec ser2 = PWM_DT_SPEC_GET(DT_NODELABEL(servo2));
static const struct pwm_dt_spec ser3 = PWM_DT_SPEC_GET(DT_NODELABEL(servo3));
static const struct pwm_dt_spec ser4 = PWM_DT_SPEC_GET(DT_NODELABEL(servo4));
static const struct pwm_dt_spec ser5 = PWM_DT_SPEC_GET(DT_NODELABEL(servo5));

static const struct pwm_dt_spec* servos[NUM_SERVO];

static const uint32_t min_pulse = DT_PROP(DT_NODELABEL(servo0), min_pulse);
static const uint32_t max_pulse = DT_PROP(DT_NODELABEL(servo0), max_pulse);

static const int factor = (max_pulse - min_pulse)/255;

#define STEP PWM_USEC(100)

enum direction {
	DOWN,
	UP,
};

int servoCtrl() {
	uint32_t pulse_width = min_pulse;
	int32_t prox;
	int ret;

	printk("Servomotor control\n");
	servos[0] = &ser0;
	servos[1] = &ser1;
	servos[2] = &ser2;
	servos[3] = &ser3;
	servos[4] = &ser4;
	servos[5] = &ser5;

	for (int i = 0; i < NUM_SERVO; ++i) {
		if (!pwm_is_ready_dt(servos[i])) {
			printk("Error: PWM device %s is not ready\n", servos[i]->dev->name);
			return 0;
		}
	}

	while(true) {
		if (proximityReady()) {
			prox = getProximity(); // 0 to 255
			pulse_width = prox * factor + min_pulse;
			for (int i = 0; i < NUM_SERVO; ++i) {
				ret = pwm_set_pulse_dt(servos[i], pulse_width);
			}
		}
		// k_sleep(K_SECONDS(1));
	}
	return 0;
}