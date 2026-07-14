#ifndef I2C_H
#define I2C_H

#include "stm32_header.h"
#include <stdlib.h>

#define I2C_WRITE	0
#define I2C_READ	1

#define ACK 0
#define NACK 1

#define RETURN_SUCCESS 0
#define FAILURE 1	 
#define INV_ARG 2
#define TGT_PRESENT 3

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
	unsigned char bus_id;
};

/* api for user */
char i2c_bus_configure(struct i2c_bus_details *i2c_bus_details);

unsigned char i2c_transfer(struct i2c_target_details   *i2c_target_details, 
			   struct i2c_transfer_details *i2c_read_details, 
			   struct i2c_transfer_details *i2c_write_details);

/*
 * helper apis for i2c stack:

unsigned char i2c_send(unsigned char* data_buffer, unsigned short byte_count, struct i2c_bus_details *i2c_bus, unsigned char delay);

unsigned char i2c_receive(unsigned char* data_buffer, unsigned short byte_count, struct i2c_bus_details *i2c_bus, unsigned char delay);

void stop_condition(struct i2c_bus_details *i2c_bus_details, unsigned char delay);

*/

#endif
