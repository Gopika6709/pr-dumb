#include "../include/stm32.h"
#include "../include/common.h"

struct hardware_specific_api hardware_api = {
	.mode_select		= stm32_pin_mode,
	.output_state		= stm32_pin_output,
	.output_type		= stm32_pin_output_type,
	.toggle_pin		= stm32_pin_toggle,
	.pupd			= stm32_pin_pupd, 
	.read_pin		= stm32_pin_read,
};

void stm32_pin_mode(unsigned char port, unsigned char pin, unsigned char flag)
{
	volatile struct gpio_port_regs *gpio_port_regs = (struct gpio_port_regs *)PORT_ADDR(port);
	
	gpio_port_regs->mode_reg &= ~BIT_WRITE(0x3, pin * 2);
	gpio_port_regs->mode_reg |= BIT_WRITE(flag, pin * 2);
}

void stm32_pin_output_type(unsigned char port, unsigned char pin, unsigned char flag)
{
	volatile struct gpio_port_regs *gpio_port_regs = (struct gpio_port_regs *)PORT_ADDR(port);
	
	gpio_port_regs->output_type_reg &= ~BIT_WRITE(0x1, pin);
	gpio_port_regs->output_type_reg |= BIT_WRITE(flag, pin);
}

void stm32_pin_output(unsigned char port, unsigned char pin, unsigned char flag)
{
	volatile struct gpio_port_regs *gpio_port_regs = (struct gpio_port_regs *)PORT_ADDR(port);

	gpio_port_regs->output_data_reg &= ~BIT_WRITE(0x1, pin);
	gpio_port_regs->output_data_reg |= BIT_WRITE(flag, pin);
}

void stm32_pin_pupd(unsigned char port, unsigned char pin, unsigned char flag)
{
	volatile struct gpio_port_regs *gpio_port_regs = (struct gpio_port_regs *)PORT_ADDR(port);
	
	gpio_port_regs->pull_up_down_reg &= ~BIT_WRITE(0x3, pin * 2);
	gpio_port_regs->pull_up_down_reg |= BIT_WRITE(flag, pin * 2);
}

void stm32_pin_toggle(unsigned char port, unsigned char pin)
{
	volatile struct gpio_port_regs *gpio_port_regs = (struct gpio_port_regs *)PORT_ADDR(port);
	
	gpio_port_regs->output_data_reg ^= BIT_WRITE(0x1, pin);
}

unsigned char stm32_pin_read(unsigned char port, unsigned char pin)
{
	volatile struct gpio_port_regs *gpio_port_regs = (struct gpio_port_regs *)PORT_ADDR(port);
	
	return ((gpio_port_regs->input_data_reg & BIT_WRITE(0x1, pin)) >> pin);
}

/*
#define GPIO_INPUT_ENABLE	0x00
#define GPIO_OUPUT_ENABLE	0x01

void stm32_all_in_one(unsigned char port, unsigned char pin, unsigned char flag)
{
	volatile struct gpio_port_regs *gpio_port_regs = (struct gpio_port_regs *)PORT_ADDR(port);
	*(unsigned char *)(PORT_ADDR(port) + ((flag & 0xF0) >> 4)) &= ~BIT_WRITE((flag & 0x0F), pin);
	*(unsigned char *)(PORT_ADDR(port) + ((flag & 0xF0) >> 4)) |= BIT_WRITE((flag & 0x0F), pin);
}
*/
