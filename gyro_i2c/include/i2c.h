#ifndef I2C_H
#define I2C_H

#include "../include/gpio.h"
#define I2C_WRITE	0
#define I2C_READ	1

#define false	0
#define true	1

#define RETURN_SUCCESS 0
#define RETURN_FAILURE 1
#define INVARG  2

#define STANDARD_MODE  100

struct i2c_bus_details {
	struct gpio_conf sda;
	struct gpio_conf scl;
};

struct i2c_transfer_details {
	unsigned short	byte_count;
	unsigned char	*data_buffer;
};

struct i2c_target_details {
	unsigned char target_address;
	unsigned char mode; /* standard mode - 100khz */
	char bus_number;
};

/* api for user */
char i2c_bus_configure(const struct i2c_bus_details *i2c_bus_details);
char i2c_transfer(const struct i2c_target_details *i2c_target_details, 
		  const struct i2c_transfer_details *i2c_write_details,
		  const struct i2c_transfer_details *i2c_read_details);

#endif
