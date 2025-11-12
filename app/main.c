/*
 * Copyright (c) 2025 XiNGRZ <hi@xingrz.me>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/input/input.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ticklet, CONFIG_TICKLET_LOG_LEVEL);

static struct gpio_dt_spec reset_gpio = {
	.port = DEVICE_DT_GET(DT_NODELABEL(io_expander)),
	.pin = 10,
	.dt_flags = GPIO_ACTIVE_LOW,
};

static struct gpio_dt_spec boot_gpio = {
	.port = DEVICE_DT_GET(DT_NODELABEL(io_expander)),
	.pin = 11,
	.dt_flags = GPIO_ACTIVE_LOW,
};

static void ticklet_key_callback(struct input_event *evt, void *user_data)
{
	ARG_UNUSED(user_data);

	if (evt->type != INPUT_EV_KEY) {
		return;
	}

	LOG_INF("Key event: code=%u, value=%d", evt->code, evt->value);

	switch (evt->code) {
	case INPUT_KEY_1:
		if (evt->value) {
			gpio_pin_set_dt(&boot_gpio, 1);
			LOG_INF("Holding boot pin...");
		} else {
			gpio_pin_set_dt(&reset_gpio, 1);
			k_msleep(200);
			gpio_pin_set_dt(&reset_gpio, 0);
			k_msleep(200);
			gpio_pin_set_dt(&boot_gpio, 0);
			LOG_INF("Issued alternate reset");
		}
		break;
	case INPUT_KEY_2:
		if (evt->value) {
			gpio_pin_set_dt(&reset_gpio, 1);
			LOG_INF("Holding reset pin...");
		} else {
			k_msleep(200);
			gpio_pin_set_dt(&reset_gpio, 0);
			LOG_INF("Issued normal reset");
		}
		break;
	default:
		break;
	}
}

INPUT_CALLBACK_DEFINE(NULL, ticklet_key_callback, NULL);

int main(void)
{
	int ret;

	LOG_INF("Ticklet FW starting");

	ret = gpio_pin_configure_dt(&reset_gpio, GPIO_OUTPUT_INACTIVE);
	if (ret != 0) {
		LOG_ERR("Failed to configure reset pin: %d", ret);
		return ret;
	}

	ret = gpio_pin_configure_dt(&boot_gpio, GPIO_OUTPUT_INACTIVE);
	if (ret != 0) {
		LOG_ERR("Failed to configure boot pin: %d", ret);
		return ret;
	}

	LOG_INF("Ticklet FW started");

	return 0;
}
