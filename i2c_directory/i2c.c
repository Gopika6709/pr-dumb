#include "i2c.h"
#include "stm32_header.h"

#define I2C_WRITE 0
#define I2C_READ 1

#define BIT_PER_BYTE 8
#define MSB_BIT 7
#define LSB_BIT 0

#define CURRENT_BIT(index) (MSB_BIT - ((index) % BIT_PER_BYTE))
#define SEND_BIT(data_buffer, index) (((*data_buffer) >> CURRENT_BIT(index))& 1)
#define RECEIVE_BIT(data_op, index) ((data_op) << CURRENT_BIT(index))


void* i2c_bus_configure(struct i2c_bus_details* bus)
{
	struct i2c_bus_details* temp_bus_ptr;
	if(!bus)
		return NULL;

	temp_bus_ptr = malloc(sizeof(struct i2c_bus_details));
	if (!temp_bus_ptr)
		return NULL;

	*temp_bus_ptr = *bus;

	gpio_pupdr_config(bus->scl_pin, bus->scl_port, PULL_UP);
	gpio_pupdr_config(bus->sda_pin, bus->sda_port, PULL_UP);

	gpio_pin_output_type(bus->scl_pin, bus->scl_port, OPEN_DRAIN);
	gpio_pin_output_type(bus->sda_pin, bus->sda_port, OPEN_DRAIN);
	
	gpio_pin_write(bus->scl_pin, bus->scl_port, HIGH);
	gpio_pin_write(bus->sda_pin, bus->sda_port, HIGH);
	
	gpio_pin_mode(bus->scl_pin, bus->scl_port, OUTPUT_MODE);
	gpio_pin_mode(bus->sda_pin, bus->sda_port, OUTPUT_MODE);
	
	return temp_bus_ptr;	
}
	
static void start_condition(const struct i2c_bus_details *bus, unsigned char delay)
{
	gpio_pin_write(bus->scl_pin, bus->scl_port, LOW);
	gpio_pin_write(bus->sda_pin, bus->sda_port, HIGH);
	k_usleep(delay); //1st pulse - low 
	
	gpio_pin_write(bus->scl_pin, bus->scl_port, HIGH);
	gpio_pin_write(bus->sda_pin, bus->sda_port, LOW);
	k_usleep(delay); //1st pulse - low 
}

static void stop_condition(const struct i2c_bus_details *bus, unsigned char delay)
{
	gpio_pin_write(bus->scl_pin, bus->scl_port, LOW);
	gpio_pin_write(bus->sda_pin, bus->sda_port, LOW);
	k_usleep(delay); 
	
	gpio_pin_write(bus->scl_pin, bus->scl_port, HIGH);
	gpio_pin_write(bus->sda_pin, bus->sda_port, HIGH);
	k_usleep(delay);  
}

static char i2c_send(unsigned char* data_buffer, unsigned short byte_count, 
		       const struct i2c_bus_details *bus, unsigned char delay)
{
	for (unsigned int bit_index = LSB_BIT; bit_index < (byte_count * BIT_PER_BYTE); bit_index++){
		gpio_pin_write(bus->scl_pin, bus->scl_port, LOW);
		k_usleep(delay); 

		gpio_pin_write(bus->sda_pin, bus->sda_port, SEND_BIT(data_buffer, bit_index));

		gpio_pin_write(bus->scl_pin, bus->scl_port, HIGH);
		k_usleep(delay);
		
		if (!CURRENT_BIT(bit_index)) {
			gpio_pin_write(bus->scl_pin, bus->scl_port, LOW);
			gpio_pin_write(bus->sda_pin, bus->sda_port, HIGH);
			k_usleep(delay); //9th pulse low
			gpio_pin_write(bus->scl_pin, bus->scl_port, HIGH);
			k_usleep(delay); //9th pulse high
	
			if (gpio_pin_read(bus->sda_pin, bus->sda_port)) {
				return -FAILURE;
			}
		
			if (bit_index == (byte_count * BIT_PER_BYTE) - 1)
				return RETURN_SUCCESS;
	
			bit_index++; 
			data_buffer++;
			gpio_pin_write(bus->scl_pin, bus->scl_port, LOW);
			gpio_pin_write(bus->sda_pin, bus->sda_port, SEND_BIT(data_buffer, bit_index));
			k_usleep(delay); //10th pulse low
			gpio_pin_write(bus->scl_pin, bus->scl_port, HIGH);
			k_usleep(delay); //10th pulse high

		while (!gpio_pin_read(bus->scl_pin, bus->scl_port)); //clk stretching
		}
	}
	return -FAILURE;
}
        
static char i2c_receive(unsigned char* data_buffer, unsigned short byte_count, 
		const struct i2c_bus_details *bus, unsigned char delay)	
{
	*data_buffer = 0;		
	for (unsigned int bit_index = LSB_BIT; bit_index < (byte_count * BIT_PER_BYTE); bit_index++){	
		gpio_pin_write(bus->scl_pin, bus->scl_port, LOW);
		k_usleep(delay); 
		gpio_pin_write(bus->scl_pin, bus->scl_port, HIGH);
		k_usleep(delay);

		*data_buffer |= RECEIVE_BIT(gpio_pin_read( bus->sda_pin, bus->sda_port), bit_index);	

		if (!CURRENT_BIT(bit_index))
		{ 	
			gpio_pin_write(bus->scl_pin, bus->scl_port, LOW);
			k_usleep(delay); //9th pulse - low
			
			gpio_pin_write(bus->sda_pin, bus->sda_port, (bit_index == (byte_count * BIT_PER_BYTE) - 1)); // ACK/NACK
			
			gpio_pin_write(bus->scl_pin, bus->scl_port, HIGH);
			k_usleep(delay); //9th pulse - high 
			gpio_pin_write(bus->scl_pin, bus->scl_port, LOW);
			gpio_pin_write(bus->sda_pin, bus->sda_port, HIGH);	// reset sda again
			
			if (bit_index < ((byte_count * BIT_PER_BYTE) - 1)){
				data_buffer++;
				*data_buffer = 0;	
			}
		}
	}
	return RETURN_SUCCESS; 
}

char i2c_transfer(struct i2c_target_details   *i2c_target,
                           struct i2c_transfer_details *i2c_write_details,
                           struct i2c_transfer_details *i2c_read_details)
{
	unsigned char delay;
	unsigned char target_addr;
	char flag;
	struct i2c_bus_details *bus;
	
	if ((!i2c_target) || (i2c_target->target_address) < 7 || (i2c_target->target_address) > 127 || 
	   (!i2c_target->mode) || (!i2c_target->i2c_bus))
		return -INV_ARG;
	
	delay = ((1.0 / (2 * KHZ(i2c_target->mode))) * 1000000);	// Clock time period
	flag = -INV_ARG; 
	bus = i2c_target->i2c_bus;

	start_condition(bus, delay);
	
	if ((!i2c_read_details) && (!i2c_write_details))
	{
	       	target_addr = (i2c_target->target_address << 1) | I2C_WRITE ; 					
		flag = i2c_send( &target_addr, 1, bus, delay);			
		goto exit;
	} 
	
	if (i2c_write_details)
	{ 
		if ((!i2c_write_details->byte_count) || (!i2c_write_details->data_buffer))
			return -INV_ARG;
       
		target_addr = (i2c_target->target_address << 1) | I2C_WRITE ; 						
		flag = i2c_send( &target_addr, 1, bus, delay);		

		if (!flag){
			flag = i2c_send(i2c_write_details->data_buffer, i2c_write_details->byte_count, 
					bus, delay);
		}

		if (flag){
			goto exit;
		}
	}
	
	if (i2c_read_details)
	{
		if ((!i2c_read_details->byte_count) || (!i2c_read_details->data_buffer))
			return -INV_ARG;
		
		if (i2c_write_details)	//repeated start
			start_condition(bus, delay);
			
		target_addr = (i2c_target->target_address << 1) | I2C_READ ; 					
		flag = i2c_send( &target_addr, 1, bus, delay);		
		if (!flag){
			flag = (i2c_receive(i2c_read_details->data_buffer, i2c_read_details->byte_count, 
					    bus, delay));
		}
	}
exit:
	stop_condition(bus , delay);
	return flag;
}
