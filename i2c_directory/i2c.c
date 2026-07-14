#include "i2c.h"
#include "stm32_header.h"

struct i2c_bus_details* configured_bus_details = NULL;

char i2c_bus_configure(struct i2c_bus_details* bus)
{
	static unsigned char configured_bus_ct = 0;
	struct i2c_bus_details* temp_bus_ptr = realloc(configured_bus_details, 
					       (configured_bus_ct + 1) * sizeof(struct i2c_bus_details));
	if(temp_bus_ptr == NULL)
		return -FAILURE;

	temp_bus_ptr[configured_bus_ct] = *bus;
	configured_bus_details = temp_bus_ptr;
	configured_bus_ct++;

	pullup_enable(bus->sda_pin, bus->sda_port);
        open_drain_enable(bus->sda_pin, bus->sda_port);
        output_high(bus->sda_pin, bus->sda_port);
        output_enable(bus->sda_pin, bus->sda_port);
       	
	pullup_enable(bus->scl_pin, bus->scl_port);
        open_drain_enable(bus->scl_pin, bus->scl_port);
        output_high(bus->scl_pin, bus->scl_port);
        output_enable(bus->scl_pin, bus->scl_port);
	
	return (configured_bus_ct -1);	
}
	
void start_condition(const struct i2c_bus_details *i2c_bus_details, unsigned char delay)
{
	output_low(i2c_bus_details->scl_pin, i2c_bus_details->scl_port); 
	output_high(i2c_bus_details->sda_pin, i2c_bus_details->sda_port); 
	k_usleep(delay); //1st pulse - low 

	output_high(i2c_bus_details->scl_pin, i2c_bus_details->scl_port); 
	output_low(i2c_bus_details->sda_pin, i2c_bus_details->sda_port); 
	k_usleep(delay); //1st pulse - low 
}

void stop_condition(const struct i2c_bus_details *i2c_bus_details, unsigned char delay)
{
	output_low(i2c_bus_details->scl_pin, i2c_bus_details->scl_port); 
	output_low(i2c_bus_details->sda_pin, i2c_bus_details->sda_port); 
	k_usleep(delay);  

	output_high(i2c_bus_details->scl_pin, i2c_bus_details->scl_port); 
	output_high(i2c_bus_details->sda_pin, i2c_bus_details->sda_port); 
	k_usleep(delay);  
}

unsigned char i2c_send(unsigned char* data_buffer, unsigned short byte_count, 
		       const struct i2c_bus_details *i2c_bus, unsigned char delay)
{
	unsigned char total_bits = byte_count * 8;

	for (unsigned short bit_index = 0; bit_index < total_bits; bit_index++) {

	    unsigned short byte = bit_index / 8;
	    unsigned char bit = 7 - (bit_index % 8);

	    output_low(i2c_bus->scl_pin, i2c_bus->scl_port);
	    k_usleep(delay); 

	    output_write(i2c_bus->sda_pin, i2c_bus->sda_port,
			 (data_buffer[byte] >> bit) & 1);

	    output_high(i2c_bus->scl_pin, i2c_bus->scl_port);
	    k_usleep(delay);

	    // After 8 bits chk ACK/NACK 
	    if (bit == 0) {

		output_low(i2c_bus->scl_pin, i2c_bus->scl_port);
		output_high(i2c_bus->sda_pin, i2c_bus->sda_port);   // Release SDA
		k_usleep(delay); //9th pulse low

		output_high(i2c_bus->scl_pin, i2c_bus->scl_port);
		k_usleep(delay); //9th pulse high

		while (!input_read(i2c_bus->scl_pin, i2c_bus->scl_port));

		if (input_read(i2c_bus->sda_pin, i2c_bus->sda_port)) {
		    stop_condition(i2c_bus, delay);
		    return -FAILURE;
		}
	    }
	}
	return RETURN_SUCCESS;
}
        
unsigned char i2c_receive(unsigned char* data_buffer, unsigned short byte_count, 
		const struct i2c_bus_details *i2c_bus, unsigned char delay)	
{
	unsigned short total_bits = byte_count * 8;

	for(unsigned short bit_index = 0; bit_index < total_bits; bit_index++){
		unsigned short byte = bit_index / 8;
    		unsigned char bit = 7 - (bit_index % 8);

    		if (bit == 7)
        		data_buffer[byte] = 0;		
		
		output_low(i2c_bus->scl_pin, i2c_bus->scl_port); 
		k_usleep(delay); //low_time_period
		output_high( i2c_bus->scl_pin, i2c_bus->scl_port);
		k_usleep(delay); //high_time_period		
		
		data_buffer[byte] |= (input_read( i2c_bus->sda_pin, i2c_bus->sda_port) << bit);	
		
		// After 8 bits send ACK/NACK
		if(bit == 0)
		{
			output_low( i2c_bus->scl_pin, i2c_bus->scl_port);
			k_usleep(delay); //9th pulse - low
			
			if(byte == (byte_count - 1)) 
				output_high( i2c_bus->sda_pin, i2c_bus->sda_port );	//sda - high NACK
			else
				output_low( i2c_bus->sda_pin, i2c_bus->sda_port );	//sda - low ACK
			
			output_high(  i2c_bus->scl_pin,  i2c_bus->scl_port );
			k_usleep(delay); //9th pulse - high 
			
			output_low( i2c_bus->scl_pin,  i2c_bus->scl_port );
			output_high( i2c_bus->sda_pin, i2c_bus->sda_port );	//reset the sda after ack/nack
		}
	}
	return RETURN_SUCCESS; 
}

unsigned char i2c_transfer(struct i2c_target_details   *i2c_target,
                           struct i2c_transfer_details *i2c_read_details,
                           struct i2c_transfer_details *i2c_write_details)
{
	if((!i2c_target) || (i2c_target->target_address) < 7 || (i2c_target->target_address) >= 127 || 
	   (! i2c_target->mode) || (i2c_target->bus_id < 0))
		return -INV_ARG;
	
	unsigned char delay = ((1.0 / (2 * (i2c_target->mode * 1000))) * 1000000), flag = 0, tgt_address = 0;	// Clock time period

	if((!i2c_read_details) && (!i2c_write_details))
	{
	       	tgt_address = (i2c_target->target_address << 1) | I2C_WRITE ; 					

		start_condition(configured_bus_details + i2c_target->bus_id, delay);
		flag = i2c_send( &tgt_address, 1, configured_bus_details + i2c_target->bus_id, delay);		// SEND ADDR		
		if(!flag)
			return TGT_PRESENT;
		else 
			return -INV_ARG;	
	} 
	if(i2c_write_details)
	{ 
		if((! i2c_write_details->byte_count) || (! i2c_write_details->data_buffer))
			return -INV_ARG;
       
		tgt_address = (i2c_target->target_address << 1) | I2C_WRITE ; 					

		start_condition(configured_bus_details + i2c_target->bus_id, delay);
		flag = i2c_send( &tgt_address, 1, configured_bus_details + i2c_target->bus_id, delay);		// SEND ADDR		
		if(flag == -1)        
			return -INV_ARG;
		
		flag = i2c_send((i2c_write_details)->data_buffer, (i2c_write_details)->byte_count, 
				configured_bus_details + i2c_target->bus_id, delay);
	}
	if(i2c_read_details)
	{
		if((! i2c_read_details->byte_count) || (! i2c_read_details->data_buffer))
			return -INV_ARG;
		tgt_address = (i2c_target->target_address << 1) | I2C_READ ; 					
		
		start_condition(configured_bus_details + i2c_target->bus_id, delay);
		
		flag = i2c_send( &tgt_address, 1, configured_bus_details + i2c_target->bus_id, delay);		//SEND ADDR		
		if(flag == -1)        
			return -INV_ARG;

		flag = (i2c_receive((i2c_read_details)->data_buffer, (i2c_read_details)->byte_count, 
				    configured_bus_details + i2c_target->bus_id, delay));
	}
	stop_condition(configured_bus_details + i2c_target->bus_id , delay);
	return flag;
	
}





