#ifndef ICM42670_H //include guard so our compiler does not process header multiple times 
#define ICM42670_H

#include "driver/i2c_master.h"
#include "esp_err.h"

#define ICM42670_I2C_ADDR 0x68 //i2c addr on espressif board 
#define ICM42670_WHO_AM_I 0x75 //register that checks what device being used 
#define ICM42670_DEVICE_ID 0x67 //this is the expected responce 

#define ICM42670_PWR_MGMT0 0x1F //this is the address of our power for imu 

//this is our gyro confgi address where we want to send our config too 
#define ICM42670_GYRO_ADDR 0x20

//this is the config we want to send to the GYRO ADDR 
#define ICM42670_GYRO_CONFIG 0x48

#define ICM42670_ACCEL_ADDR 0x21 //address of our accel
#define ICM42670_ACCEL_CONFIG 0x48 //hex we want to send to our accel addr

#define ICM42670_INT_STATUS_DRDY 0x39 // this is going to allow us to check when data from gyro and accel are ready 


//this is saying that we are going to have a function named "icm42670_init()" and it will take the i2c bus handle and return error codes 
esp_err_t icm42670_init(i2c_master_bus_handle_t bus);


//this is saying we are going to have a function that will get our device ID 
esp_err_t icm42670_get_device_id(uint8_t *device_id);


//this function is going to write to our register
esp_err_t icm42670_configure(void);


//function to get pwr managment back to see if it worked
esp_err_t icm42670_get_power(uint8_t *value);


//function for our gyro config 
esp_err_t icm42670_config_gyro(void);


//funciton to read our gyro address to check 
esp_err_t icm42670_get_gyro_config(uint8_t *value);


//function to configure our accel
esp_err_t icm42670_config_accel(void);

//function to check if we have correct address 
esp_err_t icm42670_get_accel_config(uint8_t *value);


//function that is going to read data from imu accl 
esp_err_t icm42670_read_accel(int16_t *accel_x, int16_t *accel_y, int16_t *accel_z);


//function that is going to read our Gyro data from IMU
esp_err_t icm42670_read_gyro(int16_t *gyro_x, int16_t *gyro_y, int16_t *gyro_z);

//this is our function to just check if we can read a data ready bit 
esp_err_t icm42670_get_data_ready(uint8_t *data_ready);
#endif
