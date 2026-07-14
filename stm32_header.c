#include"stm32_header.h"

void output_enable(unsigned char pin_num, unsigned char port)
{
	volatile struct gpio* port_addr = PORT_OFFSET(port);
	port_addr->mode_register |= (1 << 2*(pin_num)); //moder offset 0x00
}

void output_high(unsigned char pin_num, unsigned char port)
{
	volatile struct gpio* port_addr = PORT_OFFSET(port);
	port_addr->output_data_register |= (1 << pin_num); //odr offset 0x14
}

void output_low(unsigned char pin_num, unsigned char port)
{	
	volatile struct gpio* port_addr = PORT_OFFSET(port);
	port_addr->output_data_register &= ~(1 << pin_num); //odr offset 0x14
}

void input_enable(unsigned char pin_num, unsigned char port)
{
	volatile struct gpio* port_addr = PORT_OFFSET(port);
	port_addr->mode_register &= ~(3 << 2*(pin_num)); //ip mode
}

void input_high(unsigned char pin_num, unsigned char port)
{
	volatile struct gpio* port_addr = PORT_OFFSET(port);
	port_addr->input_data_register |= (1 << (pin_num)); 
}

unsigned char input_read(unsigned char pin_num, unsigned char port)
{
	volatile struct gpio* port_addr = PORT_OFFSET(port);
 	return (((port_addr->input_data_register) >> pin_num) & 1);
}

unsigned char output_read(unsigned char pin_num, unsigned char port)
{
	volatile struct gpio* port_addr = PORT_OFFSET(port);
 	return (((port_addr->output_data_register) >> pin_num) & 1);
}

void output_write(unsigned char pin_num, unsigned char port, unsigned char data)
{
	volatile struct gpio* port_addr = PORT_OFFSET(port);
 	(port_addr->output_data_register) &= ~(1 << pin_num); //clr odr
 	(port_addr->output_data_register) |= (data & 1) << pin_num; //set odr 
}

void output_toggle(unsigned char pin_num, unsigned char port)
{
	volatile struct gpio* port_addr = PORT_OFFSET(port);
	 
 	(port_addr->output_data_register) ^= (1 << pin_num); 
}

void pullup_enable(unsigned char pin_num, unsigned char port)
{
	volatile struct gpio* port_addr = PORT_OFFSET(port);
	port_addr->pullup_pulldown_register &= ~(3 << 2*(pin_num)); //register clear
	port_addr->pullup_pulldown_register |= (1 << 2*(pin_num)); //pull up mode
}

void pulldown_enable(unsigned char pin_num, unsigned char port)
{
	volatile struct gpio* port_addr = PORT_OFFSET(port);
	port_addr->pullup_pulldown_register &= ~(3 << 2*(pin_num)); //register clear
	port_addr->pullup_pulldown_register |= (2 << 2*(pin_num)); //pull down mode
}

void open_drain_enable(unsigned char pin_num, unsigned char port)
{
	volatile struct gpio* port_addr = PORT_OFFSET(port);
	port_addr->type_register |= (1 << pin_num); //open drain output mode
}

void send_byte(unsigned char data, unsigned char pin_num, unsigned char port)
{ //i dont need to compute the port offset as this function calls another function
	for(unsigned char i = 0; i < 8; i++){
		k_msleep(500);
		((data >> i) & 1) ? output_high(pin_num, port) : output_low(pin_num, port);
	}
}

