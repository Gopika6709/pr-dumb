#ifndef STM32_HEADER_H
#define STM32_HEADER_H

#define GPIO_BASE (0x40020000) 
#define PORT_OFFSET(x) ((volatile struct gpio*)(GPIO_BASE + (0x400 * x)))
#include <zephyr/kernel.h>

struct gpio{
	unsigned int mode_register;
	unsigned int type_register;
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

void output_enable(unsigned char, unsigned char);

void output_high(unsigned char, unsigned char);

void output_low(unsigned char, unsigned char);

unsigned char output_read(unsigned char, unsigned char);

void output_write(unsigned char, unsigned char, unsigned char);

void output_toggle(unsigned char, unsigned char);

void send_byte(unsigned char, unsigned char, unsigned char); //sends 8 bits 
	
void input_enable(unsigned char, unsigned char);

unsigned char input_read(unsigned char, unsigned char);

void input_high(unsigned char, unsigned char);

void pullup_enable(unsigned char, unsigned char);

void pulldown_enable(unsigned char, unsigned char);

void open_drain_enable(unsigned char, unsigned char);

#endif
