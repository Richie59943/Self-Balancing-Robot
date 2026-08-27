#ifndef ICM42670_H //include guard so our compiler does not process header multiple times 
#define ICM42670_H

#include "driver/i2c_master.h"
#include "esp_err.h"

#define ICM42670_I2C_ADDR 0x68 //i2c addr on espressif board 
#define ICM42670_WHO_AM_I 0x75 //register that checks what device being used 
#define ICM42670_DEVICE_ID 0x67 //this is the expected responce 

//this is saying that we are going to have a function named "icm42670_init()" and it will take the i2c bus handle and return error codes 
esp_err_t icm42670_init(i2c_master_bus_handle_t bus);
esp_err_t icm42670_get_device_id(uint8_t *device_id);

#endif
