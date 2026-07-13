#ifndef GPIO_H
#define GPIO_H

#define true    1 
#define false   0 

#define GPIO_INPUT_ENABLE	0x0
#define GPIO_OUTPUT_ENABLE	0x1

#define GPIO_OUTPUT_PUSH_PULL	0x0 
#define GPIO_OUTPUT_OPEN_DRAIN	0x1

#define GPIO_NO_PULL_UP_DOWN	0x0
#define GPIO_PULL_UP		0x1
#define GPIO_PULL_DOWN		0x2

#define GPIO_OUTPUT_LOW		0x0
#define GPIO_OUTPUT_HIGH	0x1

struct gpio_conf {
	unsigned char	port;
	unsigned char	pin;
};

void gpio_pin_output_mode(const struct gpio_conf *gpio_conf, unsigned char flag);
void gpio_pin_output_type(const struct gpio_conf *gpio_conf, unsigned char flag);
void gpio_pin_output_state(const struct gpio_conf *gpio_conf, unsigned char flag);
void gpio_pin_pupd(const struct gpio_conf *gpio_conf, unsigned char flag);
void gpio_pin_toggle(const struct gpio_conf *gpio_conf);
unsigned char gpio_pin_read(const struct gpio_conf *gpio_conf);

#endif
