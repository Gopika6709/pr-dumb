#ifndef STM32_H
#define STM32_H

#define BIT_WRITE(val, pos)	((val) << (pos))
#define SET(x)			(1 << (x))
#define PORT_ADDR(port)		(PORT_BASE + (port * 0x400))

/* PORT port offset from the base address 0x40020000 */
#define PORT_BASE	0x40020000

#define	PORT_A		0
#define	PORT_B		1
#define	PORT_C		2
#define	PORT_D		3
#define	PORT_E		4
#define	PORT_H		5

/* stm32f411re gpio configuration registers */
struct gpio_port_regs {
	unsigned int mode_reg;
	unsigned int output_type_reg;
	unsigned int output_speed_reg;
	unsigned int pull_up_down_reg;
	unsigned int input_data_reg;
	unsigned int output_data_reg;
};

char stm32_pin_check(unsigned char port, unsigned char pin);
void stm32_pin_mode(unsigned char port, unsigned char pin, unsigned char flag);
void stm32_pin_output_type(unsigned char port, unsigned char pin, unsigned char flag);
void stm32_pin_output(unsigned char port, unsigned char pin, unsigned char flag);
void stm32_pin_pupd(unsigned char port, unsigned char pin, unsigned char flag);
void stm32_pin_toggle(unsigned char port, unsigned char pin);
unsigned char stm32_pin_read(unsigned char port, unsigned char pin);

#endif
