#include "../include/gpio.h"
#include "../include/i2c.h"
#include <zephyr/kernel.h>
#include <stdlib.h>

#define BITS		8
#define MSB_INDEX	7
#define READBIT(byte_data, count)	(((byte_data) >> ((MSB_INDEX) - ((count) % (BITS)))) & 0x01)
#define WRITEBIT(data, count)		((data) << ((MSB_INDEX) - ((count) % (BITS))))

enum {
	NO_OPERATION,
	WRITE_OPERATION,
	READ_OPERATION, 
	WRITE_READ_OPERATION,
};

static struct i2c_bus_details *buses = NULL;
static unsigned char bus_count = 0;

static char i2c_send(const unsigned char *data, const unsigned short byte_count, 
			      const struct i2c_bus_details *i2c_bus_details, 
			      const unsigned char delay)
{
	unsigned short count = 0;
	
	while (count < byte_count * BITS) {
	
		gpio_pin_output_state(&(i2c_bus_details->scl), GPIO_OUTPUT_LOW);
		gpio_pin_output_state(&(i2c_bus_details->sda), READBIT(*data, count));
		k_usleep(delay);
		gpio_pin_output_state(&(i2c_bus_details->scl), GPIO_OUTPUT_HIGH);
		k_usleep(delay);

		if (!(++count % BITS)) {
			/* checking for ACK on the 9th pulse */
			gpio_pin_output_state(&(i2c_bus_details->scl), GPIO_OUTPUT_LOW);
			gpio_pin_output_state(&(i2c_bus_details->sda), GPIO_OUTPUT_HIGH);
			k_usleep(delay);
			gpio_pin_output_state(&(i2c_bus_details->scl), GPIO_OUTPUT_HIGH);
			k_usleep(delay);

			/* read the sda if 1 then it is NACK */
			if (gpio_pin_read(&(i2c_bus_details->sda))) {
				return -RETURN_FAILURE;
			}
			
			/* if the bit reacehed the end return success */
			if (count == byte_count * BITS) return RETURN_SUCCESS;
			
			/* next byte */
			data++;
			
			/* check for clock stretching */
			gpio_pin_output_state(&(i2c_bus_details->scl), GPIO_OUTPUT_LOW);
			gpio_pin_output_state(&(i2c_bus_details->sda), READBIT(*data, count));
			count++;
			k_usleep(delay);
			gpio_pin_output_state(&(i2c_bus_details->scl), GPIO_OUTPUT_HIGH);
			k_usleep(delay);
			
			/* check for whether SCL is HIGH or not if not HIGH then it is pulled down by the TARGET */
			while(!gpio_pin_read(&(i2c_bus_details->scl)));
		}
	}
	return -RETURN_FAILURE;
}

static char i2c_receive(unsigned char *receive_buff, const unsigned short byte_count, 
				 const struct i2c_bus_details *i2c_bus_details,
				 const unsigned char delay)
{
	unsigned short count = 0;
	
	while (count < byte_count * BITS) {
	
		gpio_pin_output_state(&(i2c_bus_details->scl), GPIO_OUTPUT_LOW);
		k_usleep(delay);
		gpio_pin_output_state(&(i2c_bus_details->scl), GPIO_OUTPUT_HIGH);
		k_usleep(delay);
		/* read the bit from sda and store from MSB to LSB */
		*receive_buff |= WRITEBIT(gpio_pin_read(&(i2c_bus_details->sda)), count);

		if (!(++count % BITS)) {
			/* send ACK on the 9th pulse */
			gpio_pin_output_state(&(i2c_bus_details->scl), GPIO_OUTPUT_LOW);
			gpio_pin_output_state(&(i2c_bus_details->sda), (count == byte_count * BITS));
			k_usleep(delay);
			gpio_pin_output_state(&(i2c_bus_details->scl), GPIO_OUTPUT_HIGH);
			k_usleep(delay);
			
			if (count == byte_count * BITS)
				return RETURN_SUCCESS;

			/* release the sda */
			gpio_pin_output_state(&(i2c_bus_details->scl), GPIO_OUTPUT_LOW);
			k_usleep(delay / 2);
			gpio_pin_output_state(&(i2c_bus_details->sda), GPIO_OUTPUT_HIGH);

			/* next byte */
			receive_buff++;
		}
	}
	return -RETURN_FAILURE;
}

static char start_condition(const struct i2c_bus_details *i2c_bus_details, unsigned char *addr, unsigned char delay) 
{
	char ret;
	gpio_pin_output_state(&(i2c_bus_details->scl), GPIO_OUTPUT_LOW);
	gpio_pin_output_state(&(i2c_bus_details->sda), GPIO_OUTPUT_HIGH);
	k_usleep(delay);
	gpio_pin_output_state(&(i2c_bus_details->scl), GPIO_OUTPUT_HIGH);
	gpio_pin_output_state(&(i2c_bus_details->sda), GPIO_OUTPUT_LOW);
	ret = i2c_send(addr, 1, i2c_bus_details, delay);
	return ret;
}

static void stop_condition(const struct i2c_bus_details *i2c_bus_details, unsigned char delay)
{
	gpio_pin_output_state(&(i2c_bus_details->scl), GPIO_OUTPUT_LOW);
	gpio_pin_output_state(&(i2c_bus_details->sda), GPIO_OUTPUT_LOW);
	k_usleep(delay);
	gpio_pin_output_state(&(i2c_bus_details->scl), GPIO_OUTPUT_HIGH);
	k_usleep(delay / 2);
	gpio_pin_output_state(&(i2c_bus_details->sda), GPIO_OUTPUT_HIGH);
}

static char do_write(const struct i2c_transfer_details *write, struct i2c_bus_details *bus, unsigned char delay)
{
	if (!write->data_buffer || !write->byte_count)
		return -INVARG;
	return i2c_send(write->data_buffer, write->byte_count, bus, delay);
}

static char do_read(const struct i2c_transfer_details *read, struct i2c_bus_details *bus, unsigned char delay)
{
	if (!read->data_buffer || !read->byte_count)
		return -INVARG;
	return i2c_receive(read->data_buffer, read->byte_count, bus, delay);
}

char i2c_bus_configure(const struct i2c_bus_details *i2c_bus_details)
{
	if (!i2c_bus_details)
		return -INVARG;
	unsigned char new_cap = (bus_count + 1) * sizeof(struct i2c_bus_details);
	struct i2c_bus_details *tmp = realloc(buses, new_cap);
	if (!tmp)
		return -RETURN_FAILURE;
	buses = tmp;
	buses[bus_count] = *i2c_bus_details;

	gpio_pin_pupd(&(i2c_bus_details->sda), GPIO_PULL_UP);
	gpio_pin_output_type(&(i2c_bus_details->sda), GPIO_OUTPUT_OPEN_DRAIN);
	gpio_pin_output_state(&(i2c_bus_details->sda), GPIO_OUTPUT_HIGH);
	gpio_pin_output_mode(&(i2c_bus_details->sda), GPIO_OUTPUT_ENABLE);

	gpio_pin_pupd(&(i2c_bus_details->scl), GPIO_PULL_UP);
	gpio_pin_output_type(&(i2c_bus_details->scl), GPIO_OUTPUT_OPEN_DRAIN);
	gpio_pin_output_state(&(i2c_bus_details->scl), GPIO_OUTPUT_HIGH);
	gpio_pin_output_mode(&(i2c_bus_details->scl), GPIO_OUTPUT_ENABLE);

	return bus_count++;
}

static struct i2c_bus_details* find_bus_details(unsigned char bus_number)
{
	if (bus_number >= bus_count)
		return NULL;
	return &buses[bus_number];
}

char i2c_transfer(const struct i2c_target_details *i2c_target_details, 
		  const struct i2c_transfer_details *i2c_write_details,
		  const struct i2c_transfer_details *i2c_read_details)
{
	if (!i2c_target_details				|| 
	    (i2c_target_details->target_address < 0x07) || 
	    (i2c_target_details->target_address > 0x7F) ||
	    (i2c_target_details->bus < 0)) {
		   return -INVARG;
	}
	
	/* delay calculation */
	unsigned char delay = ((1.0 / KHZ(i2c_target_details->mode)) * 1000000) / 2;
	/* shifting target_address and adding the operation to the LSB and sending the target_address */
	unsigned char addr = (i2c_target_details->target_address << 1);
	/* return values capturing var */
	char ret;
	/* to find the bus address based on the bus_number */
	struct i2c_bus_details *bus = find_bus_details(i2c_target_details->bus_number);
	unsigned char flag = 0x0;

	if (!bus)
		return -INVARG;
	flag |= (!!i2c_write_details);
	flag |= (!!i2c_read_details) << 1;
	addr |= ((!(flag & 0x01)) & ((flag >> 1) & 0x01));
	ret = start_condition(bus, &addr, delay);
	
	if (ret) 
		goto stop;
	switch (flag) {
		case NO_OPERATION:
			break;
		case WRITE_OPERATION:
			ret = do_write(i2c_write_details, bus, delay);
			break;
		case READ_OPERATION:
			ret = do_read(i2c_read_details, bus, delay);
			break;
		case WRITE_READ_OPERATION:
			ret = do_write(i2c_write_details, bus, delay);
			if (ret) 
				break;
			addr |= 1;
			ret = start_condition(bus, &addr, delay);
			if (ret) 
				break;
			ret = do_read(i2c_read_details, bus, delay);
			break;
	}
	stop:
	stop_condition(bus, delay);
	return ret;
}
