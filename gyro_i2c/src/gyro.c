#include "../include/i2c.h"
#include "../include/gyro_gy521.h"

#define NULL (void *)0
#define MSB_INDEX	8
#define ACCEL_XOUT	0x3B
#define TEMP_OUT	0x41
#define GYRO_XOUT	0x43
#define PWR_MGMT_1	0x6B
#define GY_521_ADDRESS	0x68

#define ADDRESS(ad0_state) (GY_521_ADDRESS | ad0_state)

#define SINGLE_AXIS_BYTE	2
#define XYZ_AXIS_BYTE		6

static char read_raw_data(short *receive_buffer, unsigned char reg_addr, unsigned char byte_count, unsigned char bus)
{
	char ret;
	unsigned char raw_data_buf[XYZ_AXIS_BYTE] = {0};
	struct i2c_target_details gy_target = {
		GY_521_ADDRESS, STANDARD_MODE, bus
	};
	struct i2c_transfer_details reg_write = {
		1, &reg_addr};
	struct i2c_transfer_details reg_read = {
		byte_count, &raw_data_buf[0]};

	ret = i2c_transfer(&gy_target, &reg_write, &reg_read);
	receive_buffer[0] = ((short)(raw_data_buf[0] << MSB_INDEX | raw_data_buf[1])); 
	if (byte_count == XYZ_AXIS_BYTE) {
		receive_buffer[1] = ((short)(raw_data_buf[2] << MSB_INDEX | raw_data_buf[3])); 
		receive_buffer[2] = ((short)(raw_data_buf[4] << MSB_INDEX | raw_data_buf[5])); 
	}
	return ret;
}

char gy_init(unsigned char ad0_state, unsigned char bus)
{
	char ret = 0;
	unsigned char wakeup_mng_reg[2];
	struct i2c_target_details gy_target = {
		ADDRESS(ad0_state), STANDARD_MODE, bus
	};
	/* power management register and data byte */
	struct i2c_transfer_details wakeup_write_transfer = {
		2, &wakeup_mng_reg[0]
	};

	wakeup_mng_reg[0] = PWR_MGMT_1; /* address of power management register and 0x0 clears the sleep bit */
	wakeup_mng_reg[1] = 0x0;
	ret = i2c_transfer(&gy_target, &wakeup_write_transfer, NULL);
	return ret;
}


char gyroscope_read(enum axis axis, short *receive_buffer, unsigned char bus)
{
	if (!receive_buffer)
		return -INVARG;
	char ret;

	if (axis == XYZ_AXIS)
		ret = read_raw_data(receive_buffer, (GYRO_XOUT + ((axis % XYZ_AXIS) * 2)), XYZ_AXIS_BYTE, bus);
	else 
		ret = read_raw_data(receive_buffer, (GYRO_XOUT + ((axis % XYZ_AXIS) * 2)), SINGLE_AXIS_BYTE, bus);
	return ret;
}

char accelerometer_read(enum axis axis, short *receive_buffer, unsigned char bus)
{
	if (!receive_buffer)
		return -INVARG;
	char ret;

	if (axis == XYZ_AXIS)
		ret = read_raw_data(receive_buffer, (ACCEL_XOUT + ((axis % XYZ_AXIS) * 2)), XYZ_AXIS_BYTE, bus);
	else 
		ret = read_raw_data(receive_buffer, (ACCEL_XOUT + ((axis % XYZ_AXIS) * 2)), SINGLE_AXIS_BYTE, bus);
	return ret;
}

char temperature_read(float *temperature_buffer, unsigned char bus)
{
	if (!temperature_buffer)
		return -INVARG;

	char ret;
	unsigned char temperature_reg		= TEMP_OUT; /* higher byte [15:8] */
	unsigned char temperature_buff[2]	= {0};
	struct i2c_transfer_details temperature_reg_write;
	struct i2c_transfer_details temperature_reg_read;
	struct i2c_target_details gy_target = {
		GY_521_ADDRESS, STANDARD_MODE, bus
	};

	temperature_reg_write.byte_count	= 1;
	temperature_reg_write.data_buffer	= &temperature_reg;

	temperature_reg_read.data_buffer	= &temperature_buff[0];
	temperature_reg_read.byte_count		= 2;
	ret = i2c_transfer(&gy_target, &temperature_reg_write, &temperature_reg_read);
	if (ret) 
		return ret;
	*temperature_buffer =((short)(temperature_buff[0] << MSB_INDEX | temperature_buff[1]) / 340.0) + 36.53;
	return ret;
}
