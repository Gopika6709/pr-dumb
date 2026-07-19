#ifndef EEPROM_H
#define EEPROM_H
#include "i2c.h"

#define BYTE_WRITE 1
#define PAGE_WRITE 16

#define BYTE_ERASE 1
#define PAGE_ERASE 16

#define CURRENT_ADDRESS 0
#define	RANDOM_ADDRESS 1
#define SEQUENTIAL 2

struct eeprom_operation_details {
	unsigned char operation_type; //write , read, erase
	unsigned short total_bytes;
};

#ifndef I2C_H
#define I2C_H
struct i2c_bus_details {
	unsigned char sda_pin;
        unsigned char sda_port;
        unsigned char scl_pin;
        unsigned char scl_port;
};

void *i2c_bus_configure( struct i2c_bus details *i2c_bus_details);
#endif

char eeprom_write(void *bus_id, unsigned char *data, unsigned char data_word_address,
		           struct eeprom_operation_details *eeprom_operation_details);

char eeprom_read(void *bus_id, unsigned char *data, unsigned char data_word_address,
			  struct eeprom_operation_details *eeprom_operation_details);

char eeprom_erase(void *bus_id, unsigned char data_word_address,
			   struct eeprom_operation_details *eeprom_operation_details);

#endif
