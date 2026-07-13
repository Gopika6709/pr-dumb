#include "../include/gpio.h"
#include "../include/common.h"

extern struct hardware_specific_api hardware_api;

void gpio_pin_output_mode(const struct gpio_conf *gpio_conf, unsigned char flag)
{
	if (hardware_api.mode_select)
		hardware_api.mode_select(gpio_conf->port, gpio_conf->pin, flag);
}

void gpio_pin_output_state(const struct gpio_conf *gpio_conf, unsigned char flag)
{
	if (hardware_api.output_state)
		hardware_api.output_state(gpio_conf->port, gpio_conf->pin, flag);
}

void gpio_pin_output_type(const struct gpio_conf *gpio_conf,unsigned char flag)
{
	if (hardware_api.output_type)
		hardware_api.output_type(gpio_conf->port, gpio_conf->pin, flag);
}

void gpio_pin_pupd(const struct gpio_conf *gpio_conf, unsigned char flag)
{
	if (hardware_api.pupd)
		hardware_api.pupd(gpio_conf->port, gpio_conf->pin, flag);
}

void gpio_pin_toggle(const struct gpio_conf *gpio_conf)
{
	if (hardware_api.toggle_pin)
		hardware_api.toggle_pin(gpio_conf->port, gpio_conf->pin);
}

unsigned char gpio_pin_read(const struct gpio_conf *gpio_conf)
{
	return (hardware_api.read_pin(gpio_conf->port, gpio_conf->pin));
}
