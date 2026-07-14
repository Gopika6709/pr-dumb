#ifndef GYRO_H
#define GYRO_H

#define AD0_LOW		0
#define AD0_HIGH	1

#ifndef I2C_H
struct i2c_bus_details {
	struct gpio_conf sda;
	struct gpio_conf scl;
};

#define BUS_CONFIGURE(bus, d_pin, d_port, c_pin, c_port)	\
		struct i2c_bus_details bus = {			\
		.sda.pin = d_pin,				\
		.sda.port = d_port,				\
		.scl.pin = c_pin,				\
		.scl.port = c_port				\
	}							

char i2c_bus_configure(const struct i2c_bus_details *i2c_bus_details);
#endif

enum axis {
	X_AXIS = 0,
	Y_AXIS,
	Z_AXIS,
	XYZ_AXIS
};

char gy_init(unsigned char ad0_state, void *bus);
char gyroscope_read(enum axis axis, short *receive_buffer, void *bus);
char accelerometer_read(enum axis axis, short  *receive_buffer, void *bus);
char temperature_read(float *temperature_buffer, void *bus);

#endif
