#ifndef COMMON_H
#define COMMON_H

struct hardware_specific_api {
	char (*check_pin)(unsigned char port, unsigned char pin);
	void (*mode_select)(unsigned char port, unsigned char pin, unsigned char flag);
	void (*output_state)(unsigned char port, unsigned char pin, unsigned char flag);
	void (*output_type)(unsigned char port, unsigned char pin, unsigned char flag);
	void (*pupd)(unsigned char port, unsigned char pin, unsigned char flag);
	void (*toggle_pin)(unsigned char port, unsigned char pin);
	unsigned char (*read_pin)(unsigned char port, unsigned char pin);
};

#endif
