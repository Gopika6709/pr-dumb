#include "eeprom.h"
#define EEPROM_ADDR 0x50
#define STANDARD_MODE 100

#define WRITE 0
#define ERASE 1

static char write_api(void *bus_id, const unsigned char *data, unsigned char data_word_address,
		  struct eeprom_operation_details *eeprom_operation_details, unsigned char flag)
{	
	struct i2c_target_details target; 
	struct i2c_transfer_details tf;
	unsigned char *data_arr;
	char ret_val;
	
	if ((!bus_id) || (!eeprom_operation_details) || (!data_word_address) ||
	   ((eeprom_operation_details->operation_type != BYTE_WRITE) && 
	   (eeprom_operation_details->operation_type != PAGE_WRITE))) 
		return -INV_ARG;
	
	target.target_address = EEPROM_ADDR;
       	target.mode = STANDARD_MODE; 
	target.i2c_bus = bus_id;
	data_arr = malloc(eeprom_operation_details->total_bytes + 1);
	
	if (!data_arr)
	       return -FAILURE;	

	*data_arr = data_word_address;  

	if (!flag){
		for (unsigned short i = 1; i <= eeprom_operation_details->total_bytes; i++){
			data_arr[i] = data[i-1];
		}
	} 
	else {
		for (unsigned short i = 1; i <= eeprom_operation_details->total_bytes; i++){
			data_arr[i] = 0xff;
		}
	}
	
	tf.byte_count = eeprom_operation_details->total_bytes + 1; 
	tf.data_buffer = data_arr;
	ret_val = i2c_transfer(&target, &tf, NULL); 
	free(data_arr);
	return ret_val;
}

char eeprom_write(void *bus_id, const unsigned char *data, unsigned char data_word_address,
		  struct eeprom_operation_details *eeprom_operation_details)
{
	if(!data)
	{
		return -INV_ARG;
	}
	return write_api(bus_id, data, data_word_address, eeprom_operation_details, WRITE);
}

char eeprom_erase(void *bus_id, unsigned char data_word_address,
                  struct eeprom_operation_details *eeprom_operation_details)
{		
	return write_api(bus_id, NULL, data_word_address, eeprom_operation_details, ERASE);
}

char eeprom_read(void *bus_id, unsigned char *data, unsigned char data_word_address,
                 struct eeprom_operation_details *eeprom_operation_details)
{
	struct i2c_target_details target;
	struct i2c_transfer_details tf_w, tf_r;
	char ret_val;
	
	if ((!bus_id) || (!data) || (!eeprom_operation_details) || 
	   (eeprom_operation_details->operation_type > SEQUENTIAL))	
		return -INV_ARG;	
  		
	target.target_address = EEPROM_ADDR;
       	target.mode = STANDARD_MODE; 
	target.i2c_bus = bus_id;	
	
	tf_r.byte_count = eeprom_operation_details->total_bytes; 
	tf_r.data_buffer = data;
	ret_val = -INV_ARG;

	if ((!data_word_address) && ((!eeprom_operation_details->operation_type) || 
	   (eeprom_operation_details->operation_type == 2))){		//curr_addr_read 
		
		ret_val = i2c_transfer(&target, NULL, &tf_r);
	}

	if ((data_word_address) && ((eeprom_operation_details->operation_type == 1) || 
	   (eeprom_operation_details->operation_type == 2))){		//random read
		
		tf_w.byte_count= 1;
	        tf_w.data_buffer = &data_word_address; 
		ret_val = i2c_transfer(&target, &tf_w, &tf_r);
	}
	return ret_val;
}
