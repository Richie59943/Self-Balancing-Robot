#include "icm42670.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
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

};

//functions is going to write to our IMU 
// the reg and value are two addresses so we have where we wnt to send it (reg) and what we want to send v(value)
static esp_err_t icm42670_wrtie_register(uint8_t reg, uint8_t value)
{
  uint8_t data[2] = { //created a array that will hold both 
            reg,value
                };

  return i2c_master_transmit(icm42670_handle, data, sizeof(data),1000 );
}

//creating our config for our write to register 
esp_err_t icm42670_configure(void) // funciton is not going to take in any argumetns 
{
  //this is pretty much saying write valie of our valu into the register in this case icm42670 
  esp_err_t err = icm42670_wrtie_register(ICM42670_PWR_MGMT0,0x0F); //going to us eour pre defined locations 

if (err != ESP_OK)
{
  ESP_LOGE(TAG,"Failed to configure PWR_MGMT0");
      return err;
}

  vTaskDelay(pdMS_TO_TICKS(1)); // putting a one milisecond delay becuase our data sheet mentions we cannot send another request before 200 us

  return ESP_OK;
};


esp_err_t icm42670_get_power(uint8_t *value)
{
  return icm42670_read_reg(ICM42670_PWR_MGMT0, value);
}

//funcitons is getting device id 
//
esp_err_t icm42670_get_device_id(uint8_t *device_id)
{
  return icm42670_read_reg(ICM42670_WHO_AM_I, device_id);
}

