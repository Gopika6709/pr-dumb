#ifndef STM32_HEADER_H
#define STM32_HEADER_H

#define GPIO_BASE (0x40020000) 
#define PORT_OFFSET(x) ((volatile struct gpio_reg*)(GPIO_BASE + (0x400 * (x))))

#define INPUT_MODE 0
#define OUTPUT_MODE 1

#define HIGH 1
#define LOW 0

#define NO_PULLUP_PULLDOWN 0
#define PULL_UP 1
#define PULL_DOWN 2

#define PUSH_PULL 0
#define OPEN_DRAIN 1


struct gpio_reg{
	unsigned int mode_register;
	unsigned int output_type_register;
	unsigned int speed_register;
	unsigned int pullup_pulldown_register;
	unsigned int input_data_register;
	unsigned int output_data_register;
	unsigned int bitset_reset_register;
};

enum {
	PORT_A,
        PORT_B,
        PORT_C,
        PORT_D,
        PORT_E,
        PORT_F,
        PORT_G,
        PORT_H
};


void gpio_pin_mode(unsigned char pin, unsigned char port, unsigned char mode);

void gpio_pin_output_type(unsigned char pin, unsigned char port, unsigned char state);

void gpio_pin_write(unsigned char pin, unsigned char port, unsigned char data); //writes odr

unsigned char gpio_pin_read(unsigned char pin, unsigned char port); //reads idr

void gpio_pin_toggle(unsigned char pin, unsigned char port); //toggle odr

void gpio_pupdr_config(unsigned char pin, unsigned char port, unsigned char up_down);

#endif
