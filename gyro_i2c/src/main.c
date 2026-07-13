#include <stdio.h>
#include <zephyr/kernel.h>
#include "../include/i2c.h"
#include "../include/stm32_common.h"
#include "../include/gyro_gy521.h"

#define SDA_PIN	6
#define SCL_PIN	7

int main(void)
{
	/* I2C bus configuration details */
	struct i2c_bus_details bus;
	bus.sda.pin = SDA_PIN;
	bus.sda.port = PORT_A;
	bus.scl.pin = SCL_PIN;
	bus.scl.port = PORT_A;

	unsigned char ret;
	/* configure bus for i2c */
	char bus_n = i2c_bus_configure(&bus);
	printf("bus %d \n", bus_n);
	bus_n = i2c_bus_configure(&bus);
	printf("bus %d \n", bus_n);
	ret = gy_init(bus_n, AD0_LOW);
	float degree = 0;
	ret = temperature_read(&degree, bus_n);
	printf("temp : %d \n", (short)degree);

	short gyro_x[3] = {0};
	while (1) {
		accelerometer_read(XYZ_AXIS, &gyro_x[0], bus_n);
		printf("===================\n");
		printf("gyrox: %d \n", gyro_x[0]);
		printf("gyroy: %d \n", gyro_x[1]);
		printf("gyroz: %d \n", gyro_x[2]);
		k_msleep(500);
	}
	return 0;
}
