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
	BUS_CONFIGURE(bus0, SDA_PIN, PORT_A, SCL_PIN, PORT_A);
	unsigned char ret;
	float degree = 0;
	short gyro_x[3] = {0};
	void * bus_n = i2c_bus_configure(&bus0);
	
	ret = gy_init(AD0_LOW, bus_n);
	ret = temperature_read(&degree, bus_n);
	printf("temp : %d \n", (short)degree);

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
