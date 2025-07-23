/*
 * Copyright (c) 2017 Linaro Limited
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2024 TOKITA Hiroshi
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>

#define LOG_LEVEL 4
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/gpio.h>

#define STRIP_NODE DT_ALIAS(led_strip)

#if DT_NODE_HAS_PROP(DT_ALIAS(led_strip), chain_length)
#define STRIP_NUM_PIXELS DT_PROP(DT_ALIAS(led_strip), chain_length)
#else
#error Unable to determine length of LED strip
#endif

#define LED0 DT_NODELABEL(led0)
#define POWER_WS2812_NODE DT_NODELABEL(ws2812_power0)

#define DELAY_TIME K_MSEC(CONFIG_SAMPLE_LED_UPDATE_DELAY)

#define RGB(_r, _g, _b) {.r = (_r), .g = (_g), .b = (_b)}

static const struct led_rgb colors[] = {
	RGB(CONFIG_SAMPLE_LED_BRIGHTNESS, 0x00, 0x00), /* red */
	RGB(0x00, CONFIG_SAMPLE_LED_BRIGHTNESS, 0x00), /* green */
	RGB(0x00, 0x00, CONFIG_SAMPLE_LED_BRIGHTNESS), /* blue */
};

static struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0, gpios);
// static struct gpio_dt_spec power_ws2812 = GPIO_DT_SPEC_GET(POWER_WS2812_NODE, gpios);

static struct led_rgb pixels[STRIP_NUM_PIXELS];

static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);

int main(void)
{
	size_t color = 0;
	int rc;

	// if (!gpio_is_ready_dt(&power_ws2812))
	// {
	// 	LOG_ERR("Power WS2812 device %s is not ready", power_ws2812.port->name);
	// 	return 0;
	// }

	if (!gpio_is_ready_dt(&led0))
	{
		LOG_ERR("LED0 device %s is not ready", led0.port->name);
		return 0;
	}

	rc = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (rc < 0)
	{
		LOG_ERR("Failed to configure LED0 device %s", led0.port->name);
		return 0;
	}

	gpio_pin_toggle_dt(&led0);
	k_msleep(6000);
	gpio_pin_toggle_dt(&led0);

	// rc = gpio_pin_configure_dt(&power_ws2812, GPIO_OUTPUT_ACTIVE);
	// if (rc < 0)
	// {
	// 	LOG_ERR("Failed to configure power WS2812 device %s", power_ws2812.port->name);
	// 	return 0;
	// }

	if (device_is_ready(strip))
	{
		LOG_INF("Found LED strip device %s", strip->name);
	}
	else
	{
		LOG_ERR("LED strip device %s is not ready", strip->name);
		return 0;
	}

	LOG_INF("Displaying pattern on strip");
	while (1)
	{
		for (size_t cursor = 0; cursor < ARRAY_SIZE(pixels); cursor++)
		{
			memset(&pixels, 0x00, sizeof(pixels));
			memcpy(&pixels[cursor], &colors[color], sizeof(struct led_rgb));

			rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
			if (rc)
			{
				LOG_ERR("couldn't update strip: %d", rc);
			}

			k_sleep(DELAY_TIME);
		}

		color = (color + 1) % ARRAY_SIZE(colors);
	}

	return 0;
}
