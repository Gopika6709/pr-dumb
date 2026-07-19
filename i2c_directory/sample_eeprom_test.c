#include "eeprom.h"
#include <stdio.h>

void return_check(signed char var)
{
	if(!var)
	{
		printf("operation success\n");
	}else if(var == -1)
	{
		printf("operation failure\n");
	}else if(var == -2)
	{
		printf("invalid argument");
	} 
}

int main(){
	I2C_BUS_INIT(bus_1, 6, PORT_A, 7, PORT_A);
	unsigned char byte = 0;
	unsigned char arr[16] = {0};
	signed char var = 0;
	unsigned char data_word_addr_1 = 0x10;
	unsigned char num[]={0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36};
	void *id = i2c_bus_configure(&bus_1);	

/* page_write */
	struct eeprom_operation_details tf_1 ={.operation_type = PAGE_WRITE, .total_bytes = sizeof(num) };
	var = eeprom_write(id, num, data_word_addr_1, &tf_1);
	printf("\n page write operation \n");
	return_check(var);

/*byte_read */
	struct eeprom_operation_details tf_2 ={.operation_type = RANDOM_ADDRESS, .total_bytes = 1};
	var = eeprom_read(id, &byte, data_word_addr_1, &tf_2);
	printf("\n byte read operation \n");
	return_check(var);

/* random_read before erasing */	
	tf_2.total_bytes = sizeof(arr);		
	var = eeprom_read(id, arr, data_word_addr_1, &tf_2);
	printf("\n random read operation before erasing\n");
	return_check(var);
	for(int i=0; i< 16; i++)
	{
		printf("rx value - %x\n", arr[i]); 
	}

/*erase testing */
	struct eeprom_operation_details tf_3 ={.operation_type = PAGE_ERASE, .total_bytes = sizeof(arr) };
	eeprom_erase(id, data_word_addr_1, &tf_3);
	printf("\n erase operation \n");

/* random_read */	
	var = eeprom_read(id, arr, data_word_addr_1, &tf_2);
	printf("\n random read operation \n");
	return_check(var);
	for(int i=0; i< 16; i++)
	{
		printf("rx value - %x\n", arr[i]); 
	} 
}
