#include"stm32_header.h"

void gpio_pin_mode(unsigned char pin_num, unsigned char port, unsigned char mode )
{
	volatile struct gpio_reg* port_addr = PORT_OFFSET(port);
	port_addr->mode_register &= ~(3 << 2*(pin_num)); 
	port_addr->mode_register |= (mode << 2*(pin_num)); 
}

void gpio_pin_output_type(unsigned char pin_num, unsigned char port, unsigned char state)
{
	volatile struct gpio_reg* port_addr = PORT_OFFSET(port);
 	port_addr->output_type_register &= ~(1 << pin_num); 
	port_addr->output_type_register |= (state << pin_num); 
}

void gpio_pin_write(unsigned char pin_num, unsigned char port, unsigned char data)
{
	volatile struct gpio_reg* port_addr = PORT_OFFSET(port);
 	(port_addr->output_data_register) &= ~(1 << pin_num); 
 	(port_addr->output_data_register) |= (data & 1) << pin_num;  
}

unsigned char gpio_pin_read(unsigned char pin_num, unsigned char port)
{
	volatile struct gpio_reg* port_addr = PORT_OFFSET(port);
 	return (((port_addr->input_data_register) >> pin_num) & 1);
}

void gpio_pin_toggle(unsigned char pin_num, unsigned char port)
{
	volatile struct gpio_reg* port_addr = PORT_OFFSET(port); 
 	(port_addr->output_data_register) ^= (1 << pin_num); 
}

void gpio_pupdr_config(unsigned char pin_num, unsigned char port, unsigned char up_down)
{
	volatile struct gpio_reg* port_addr = PORT_OFFSET(port);
	port_addr->pullup_pulldown_register &= ~(3 << 2*(pin_num)); 
	port_addr->pullup_pulldown_register |= (up_down << 2*(pin_num)); 
}
