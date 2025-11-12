/*
 * Copyright (c) 2025 XiNGRZ <hi@xingrz.me>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led.h>
#include <zephyr/input/input.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ticklet, CONFIG_TICKLET_LOG_LEVEL);

#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

#define LED_ON  (LED_BRIGHTNESS_MAX)
#define LED_OFF (0)

static const uint16_t input_event_codes[] = {
	DT_FOREACH_PROP_ELEM_SEP
	(ZEPHYR_USER_NODE, ticklet_input_event_codes, DT_PROP_BY_IDX, (, ))};

static const struct gpio_dt_spec output_gpios[] = {DT_FOREACH_PROP_ELEM_SEP
	(ZEPHYR_USER_NODE, ticklet_output_gpios, GPIO_DT_SPEC_GET_BY_IDX, (, ))};

static const struct device *leds = DEVICE_DT_GET(DT_PHANDLE(ZEPHYR_USER_NODE, ticklet_leds));

struct ticklet_handler {
	const char *label;
	size_t input_idx;
	int (*handler)(struct input_event *evt, void *user_data);
};

struct ticklet_output {
	const char *label;
	size_t output_idx;
	gpio_dt_flags_t gpio_flags;
};

static int ticklet_handle_key_1(struct input_event *evt, void *user_data);
static int ticklet_handle_key_2(struct input_event *evt, void *user_data);

static struct ticklet_handler ticklet_handlers[] = {
	{.label = "K1", .input_idx = 1, .handler = ticklet_handle_key_1},
	{.label = "K2", .input_idx = 2, .handler = ticklet_handle_key_2},
};

static struct ticklet_output ticklet_outputs[] = {
	{.label = "RESET", .output_idx = 0, .gpio_flags = GPIO_ACTIVE_LOW},
	{.label = "BOOT", .output_idx = 1, .gpio_flags = GPIO_ACTIVE_LOW},
};

enum {
	TICKLET_OUTPUT_RESET = 0,
	TICKLET_OUTPUT_BOOT,
};

static int ticklet_handle_key_1(struct input_event *evt, void *user_data)
{
	ARG_UNUSED(user_data);
	const struct gpio_dt_spec *reset_gpio =
		&output_gpios[ticklet_outputs[TICKLET_OUTPUT_RESET].output_idx];
	const struct gpio_dt_spec *boot_gpio =
		&output_gpios[ticklet_outputs[TICKLET_OUTPUT_BOOT].output_idx];

	if (evt->value) {
		gpio_pin_set_dt(boot_gpio, 1);
		led_set_brightness(leds, 1, LED_ON);
		LOG_INF("Holding boot pin...");
	} else {
		gpio_pin_set_dt(reset_gpio, 1);
		led_set_brightness(leds, 0, LED_ON);
		k_msleep(200);
		gpio_pin_set_dt(reset_gpio, 0);
		led_set_brightness(leds, 0, LED_OFF);
		k_msleep(200);
		gpio_pin_set_dt(boot_gpio, 0);
		led_set_brightness(leds, 1, LED_OFF);
		LOG_INF("Issued alternate reset");
	}

	return 0;
}

static int ticklet_handle_key_2(struct input_event *evt, void *user_data)
{
	ARG_UNUSED(user_data);
	const struct gpio_dt_spec *reset_gpio =
		&output_gpios[ticklet_outputs[TICKLET_OUTPUT_RESET].output_idx];

	if (evt->value) {
		gpio_pin_set_dt(reset_gpio, 1);
		led_set_brightness(leds, 0, LED_ON);
		LOG_INF("Holding reset pin...");
	} else {
		k_msleep(200);
		gpio_pin_set_dt(reset_gpio, 0);
		led_set_brightness(leds, 0, LED_OFF);
		LOG_INF("Issued normal reset");
	}

	return 0;
}

static void ticklet_key_callback(struct input_event *evt, void *user_data)
{
	struct ticklet_handler *handler;

	if (evt->type != INPUT_EV_KEY) {
		return;
	}

	LOG_INF("Key event: code=%u, value=%d", evt->code, evt->value);

	for (size_t i = 0; i < ARRAY_SIZE(ticklet_handlers); i++) {
		handler = &ticklet_handlers[i];

		if (handler->input_idx < ARRAY_SIZE(input_event_codes) &&
		    evt->code == input_event_codes[handler->input_idx]) {
			handler->handler(evt, user_data);
			break;
		}
	}
}

INPUT_CALLBACK_DEFINE(NULL, ticklet_key_callback, NULL);

int main(void)
{
	const struct gpio_dt_spec *output_gpio;
	int ret;

	LOG_INF("Ticklet FW starting");

	for (size_t i = 0; i < ARRAY_SIZE(ticklet_outputs); i++) {
		output_gpio = &output_gpios[ticklet_outputs[i].output_idx];

		ret = gpio_pin_configure_dt(output_gpio,
					    ticklet_outputs[i].gpio_flags | GPIO_OUTPUT_INACTIVE);
		if (ret != 0) {
			LOG_ERR("Failed to configure %s pin: %d", ticklet_outputs[i].label, ret);
			return ret;
		}
	}

	LOG_INF("Ticklet FW started");

	return 0;
}
