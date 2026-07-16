#include "i2c.h"
#include "stm32_header.h"

#define BIT_PER_BYTE 8
#define MSB_BIT 7
#define LSB_BIT 0

#define CURRENT_BIT(index) (MSB_BIT - (index % BIT_PER_BYTE))

struct i2c_bus_details* configured_bus_details = NULL;

char i2c_bus_configure(struct i2c_bus_details* bus)
{
	static unsigned char configured_bus_ct = 0;
	struct i2c_bus_details* temp_bus_ptr = realloc(configured_bus_details, 
					       (configured_bus_ct + 1) * sizeof(struct i2c_bus_details));
	if (!temp_bus_ptr)
		return -FAILURE;

	temp_bus_ptr[configured_bus_ct] = *bus;
	configured_bus_details = temp_bus_ptr;

	gpio_pupdr_config(bus->scl_pin, bus->scl_port, PULL_UP);
	gpio_pupdr_config(bus->sda_pin, bus->sda_port, PULL_UP);

	gpio_pin_output_type(bus->scl_pin, bus->scl_port, OPEN_DRAIN);
	gpio_pin_output_type(bus->sda_pin, bus->sda_port, OPEN_DRAIN);
	
	gpio_pin_write(bus->scl_pin, bus->scl_port, HIGH);
	gpio_pin_write(bus->sda_pin, bus->sda_port, HIGH);
	
	gpio_pin_mode(bus->scl_pin, bus->scl_port, OUTPUT_MODE);
	gpio_pin_mode(bus->sda_pin, bus->sda_port, OUTPUT_MODE);
	
	return (configured_bus_ct++);	
}
	
void start_condition(const struct i2c_bus_details *bus, unsigned char delay)
{
	gpio_pin_write(bus->scl_pin, bus->scl_port, LOW);
	gpio_pin_write(bus->sda_pin, bus->sda_port, HIGH);
	k_usleep(delay); //1st pulse - low 
	
	gpio_pin_write(bus->scl_pin, bus->scl_port, HIGH);
	gpio_pin_write(bus->sda_pin, bus->sda_port, LOW);
	k_usleep(delay); //1st pulse - low 
}

void stop_condition(const struct i2c_bus_details *bus, unsigned char delay)
{
	gpio_pin_write(bus->scl_pin, bus->scl_port, LOW);
	gpio_pin_write(bus->sda_pin, bus->sda_port, LOW);
	k_usleep(delay); 
	
	gpio_pin_write(bus->scl_pin, bus->scl_port, HIGH);
	gpio_pin_write(bus->sda_pin, bus->sda_port, HIGH);
	k_usleep(delay);  
}

char i2c_send(unsigned char* data_buffer, unsigned short byte_count, 
		       const struct i2c_bus_details *bus, unsigned char delay)
{
	unsigned short byte = 0 ;
	for (unsigned short bit_index = LSB_BIT; bit_index < (byte_count * BIT_PER_BYTE); bit_index++){
		gpio_pin_write(bus->scl_pin, bus->scl_port, LOW);
		k_usleep(delay); 

		gpio_pin_write(bus->sda_pin, bus->sda_port, (data_buffer[byte] >> CURRENT_BIT(bit_index)) & 1);

		gpio_pin_write(bus->scl_pin, bus->scl_port, HIGH);
		k_usleep(delay);
		
		if (!CURRENT_BIT(bit_index)) {
			gpio_pin_write(bus->scl_pin, bus->scl_port, LOW);
			gpio_pin_write(bus->sda_pin, bus->sda_port, HIGH);
			k_usleep(delay); //9th pulse low
			gpio_pin_write(bus->scl_pin, bus->scl_port, HIGH);
			k_usleep(delay); //9th pulse high

			if (gpio_pin_read(bus->sda_pin, bus->sda_port)) {
				stop_condition(bus, delay); //if NACK - stop
				return -FAILURE;
			}
		
			if (bit_index == (byte_count * BIT_PER_BYTE) - 1)
				return SUCCESS;
			bit_index++; 
			byte++;

			gpio_pin_write(bus->scl_pin, bus->scl_port, LOW);
			gpio_pin_write(bus->sda_pin, bus->sda_port,
				      (data_buffer[byte] >> CURRENT_BIT(bit_index)) & 1);

			k_usleep(delay); //10th pulse low

			gpio_pin_write(bus->scl_pin, bus->scl_port, HIGH);
			k_usleep(delay); //10th pulse high

		while (!gpio_pin_read(bus->scl_pin, bus->scl_port)); //clk stretching
		}
	}
	return -FAILURE;
}
        
unsigned char i2c_receive(unsigned char* data_buffer, unsigned short byte_count, 
		const struct i2c_bus_details *bus, unsigned char delay)	
{
	unsigned short byte = 0;
	for (unsigned short bit_index = LSB_BIT; bit_index < (byte_count * BIT_PER_BYTE); bit_index++){	
		if (CURRENT_BIT(bit_index) == MSB_BIT) 
			data_buffer[byte] = 0;		//zeroing out before use
		
		gpio_pin_write(bus->scl_pin, bus->scl_port, LOW);
		k_usleep(delay); 
		gpio_pin_write(bus->scl_pin, bus->scl_port, HIGH);
		k_usleep(delay);

		data_buffer[byte] |= (gpio_pin_read( bus->sda_pin, bus->sda_port) << CURRENT_BIT(bit_index));	

		if (!CURRENT_BIT(bit_index))
		{ 	
			gpio_pin_write(bus->scl_pin, bus->scl_port, LOW);
			k_usleep(delay); //9th pulse - low
			
			if (bit_index == (byte_count * BIT_PER_BYTE) - 1)
				gpio_pin_write(bus->sda_pin, bus->sda_port, HIGH);	// NACK
			else
				gpio_pin_write(bus->sda_pin, bus->sda_port, LOW);	// ACK
			
			gpio_pin_write(bus->scl_pin, bus->scl_port, HIGH);
			k_usleep(delay); //9th pulse - high 
			byte++;	
			gpio_pin_write(bus->scl_pin, bus->scl_port, LOW);
			gpio_pin_write(bus->sda_pin, bus->sda_port, HIGH);	// reset sda again
		}
	}
	return RETURN_SUCCESS; 
}

unsigned char i2c_transfer(struct i2c_target_details   *i2c_target,
                           struct i2c_transfer_details *i2c_read_details,
                           struct i2c_transfer_details *i2c_write_details)
{
	unsigned char delay;
	unsigned char flag;
	unsigned char target_addr;

	if ((!i2c_target) || (i2c_target->target_address) < 7 || (i2c_target->target_address) > 127 || 
	   (!i2c_target->mode) || (i2c_target->bus_id < 0))
		return -INV_ARG;
	
	delay = ((1.0 / (2 * KHZ(i2c_target->mode))) * 1000000);	// Clock time period
	flag = -INV_ARG; //later part rewrites this based on condition

	if ((!i2c_read_details) && (!i2c_write_details))
	{
	       	target_addr = (i2c_target->target_address << 1) | I2C_WRITE ; 					
		start_condition(configured_bus_details + i2c_target->bus_id, delay);
		flag = i2c_send( &target_addr, 1, configured_bus_details + i2c_target->bus_id, delay);		
	} 
	if (i2c_write_details)
	{ 
		if ((!i2c_write_details->byte_count) || (!i2c_write_details->data_buffer))
			return -INV_ARG;
       
		target_addr = (i2c_target->target_address << 1) | I2C_WRITE ; 					
		start_condition(configured_bus_details + i2c_target->bus_id, delay);
		
		if (!i2c_send( &target_addr, 1, configured_bus_details + i2c_target->bus_id, delay)){
			flag = i2c_send(i2c_write_details->data_buffer, i2c_write_details->byte_count, 
					configured_bus_details + i2c_target->bus_id, delay);
		}
	}
	if (i2c_read_details)
	{
		if ((!i2c_read_details->byte_count) || (!i2c_read_details->data_buffer))
			return -INV_ARG;
		
		target_addr = (i2c_target->target_address << 1) | I2C_READ ; 					
		start_condition(configured_bus_details + i2c_target->bus_id, delay);		

		if (!i2c_send(&target_addr, 1, configured_bus_details + i2c_target->bus_id, delay)){		
			flag = (i2c_receive(i2c_read_details->data_buffer, i2c_read_details->byte_count, 
					    configured_bus_details + i2c_target->bus_id, delay));
		}
	}
	stop_condition(configured_bus_details + i2c_target->bus_id , delay);
	return flag;
}





