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


//function to config our GyRO 
esp_err_t icm42670_config_gyro(void)
{
  esp_err_t err = icm42670_wrtie_register(ICM42670_GYRO_ADDR,ICM42670_GYRO_CONFIG);

  if(err != ESP_OK)
  {
    ESP_LOGE(TAG,"Failed to configure our GYRO");
      return err;
  }

  return ESP_OK;
}


//function to pull our address witin gyro add to check we sent the right one 
esp_err_t icm42670_get_gyro_config(uint8_t *value)
{
  return icm42670_read_reg(ICM42670_GYRO_ADDR,value);

}


//function is going to configure our accel 
esp_err_t icm42670_config_accel(void)
{
  esp_err_t err = icm42670_wrtie_register(ICM42670_ACCEL_ADDR,ICM42670_ACCEL_CONFIG);

  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to config accel");
    return err;
  }
  return ESP_OK;
}

//function to check our address
esp_err_t icm42670_get_accel_config(uint8_t *value)
{
  return icm42670_read_reg(ICM42670_ACCEL_ADDR, value);
}



//function that is going to read our accel
esp_err_t icm42670_read_accel(int16_t *accel_x, int16_t *accel_y, int16_t *accel_z)
{
  uint8_t data[6]; //going to create a array since we are receiving 6 x1 x0,y0,y1,z0z1
  uint8_t reg =0x0B;

  esp_err_t err = i2c_master_transmit_receive(icm42670_handle,&reg,1,data,6,1000); //handle,transmit data, transmit size, recieve data, receive size,timeout 

  if(err != ESP_OK)
  {
    ESP_LOGE(TAG,"Error transmiting accel");
    return err;
  }
  
  //making it clean so we know name 
  uint8_t x1 = data[0];
  uint8_t x0 = data[1];
  uint8_t y1 = data[2];
  uint8_t y0 = data[3];
  uint8_t z1 = data[4];
  uint8_t z0 = data[5];

  //now creating our bit mask 
  *accel_x = (x1 << 8) | x0;

  *accel_y = (y1 << 8) | y0;

  *accel_z = (z1 << 8) | z0;

  return ESP_OK;
}


//function that is going to read our gyro data from IMU
esp_err_t icm42670_read_gyro(int16_t *gyro_x, int16_t *gyro_y,int16_t *gyro_z)
{
  uint8_t data[6]; // amount of data that we want to receive or read 
  uint8_t start_reg = 0x11;


  esp_err_t err = i2c_master_transmit_receive(icm42670_handle,&start_reg,1,data,6,1000); // dev handle, data we want to send, size of that data, data we want to receive , size of that data, and the timeout  

  if (err != ESP_OK)
  {
    ESP_LOGE(TAG,"There was a error transmiting and receiving from gyro");
    return err;
  }
//going to make easier to name so we know which we need to shift 
  uint8_t x1 = data[0];
  uint8_t x0 = data[1];
  uint8_t y1 = data[2];
  uint8_t y0 = data[3];
  uint8_t z1 = data[4];
  uint8_t z0 = data[5];

  //creating these as pointer becuase if not we are just manipulting the copy and not the one actually stored in that pointer or at the addr 

  *gyro_x = (x1 << 8) | x0;
  *gyro_y = (y1 << 8) | y0;
  *gyro_z = (z1 << 8) | z0;

  return ESP_OK;

}
