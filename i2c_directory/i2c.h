#ifndef I2C_H
#define I2C_H

#include "stm32_header.h"
#include <stdlib.h>

#define RETURN_SUCCESS 0
#define FAILURE 1	 
#define INV_ARG 2

#define I2C_BUS_INIT(bus_var, data_pin, data_port, clk_pin, clk_port) \
	struct i2c_bus_details bus_var = {			\
		.sda_pin = data_pin,				\
		.sda_port = data_port,				\
		.scl_pin = clk_pin,				\
		.scl_port = clk_port				\
	}

struct i2c_bus_details{
        unsigned char sda_pin;
        unsigned char sda_port;
        unsigned char scl_pin;
        unsigned char scl_port;
};

struct i2c_transfer_details {
	unsigned short	byte_count;
	unsigned char	*data_buffer;
};

struct i2c_target_details {
	unsigned char target_address;
	unsigned char mode; /* standard mode - 100khz */
	void *i2c_bus;
};

void *i2c_bus_configure(struct i2c_bus_details *i2c_bus_details);

char i2c_transfer(struct i2c_target_details   *i2c_target_details, 
			   struct i2c_transfer_details *i2c_write_details,
			   struct i2c_transfer_details *i2c_read_details);

#endif
