#include "icm42670.h"

#include "driver/i2c_master.h"
#include "esp_log.h"

static const char *TAG = "ICM42670";


static i2c_master_dev_handle_t icm42670_handle;

//this is out init function that configs our device 
esp_err_t icm42670_init(i2c_master_bus_handle_t bus)
{
  i2c_device_config_t dev_cfg = {
    .dev_addr_length =I2C_ADDR_BIT_LEN_7,
    .device_address = ICM42670_I2C_ADDR,
    .scl_speed_hz = 400000,
  };

  esp_err_t err = i2c_master_bus_add_device(
    bus,
    &dev_cfg,
    &icm42670_handle
  );

  if(err != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to add ICM-42670-p");
    return err;

  }

  ESP_LOGI(TAG, "ICM-42670-P added to I2C bus");

  return ESP_OK;
}


//funciton to read and write 
//some i2c devices need write configs before being able to read data from it. 
esp_err_t icm42670_read_reg(uint8_t reg, uint8_t *data)
{
  return i2c_master_transmit_receive(icm42670_handle,&reg,1,data,1,1000);

}

//funcitons is getting device id 
//
esp_err_t icm42670_get_device_id(uint8_t *device_id)
{
  return icm42670_read_reg(ICM42670_WHO_AM_I, device_id);
}

