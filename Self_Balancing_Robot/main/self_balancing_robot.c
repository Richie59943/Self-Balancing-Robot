#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "driver/i2c_master.h"

#include "icm42670.h"

#define ESP_SDA_GPIO 7 
#define ESP_SCL_GPIO 8

static const char *TAG = "BALANCER";




void app_main(void)
{
  i2c_master_bus_config_t i2c_mst_config = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = I2C_NUM_0,
    .scl_io_num = ESP_SCL_GPIO,
    .sda_io_num = ESP_SDA_GPIO,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true,
  };

  i2c_master_bus_handle_t bus_handle;
  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config,&bus_handle));

  ESP_LOGD(TAG, "I2c bus initialized");

  ESP_ERROR_CHECK(icm42670_init(bus_handle));

  //calling out fucntion that will read register 
  uint8_t device_id =0; //unsigned integer 

  ESP_ERROR_CHECK(icm42670_get_device_id(&device_id)); //we give the memory addres of device_id so it can be altered 

  //checking error on configure function 
  ESP_ERROR_CHECK(icm42670_configure());
ESP_LOGI(TAG,"ICM-42670-P configured");


  //going to check if we actually stored our pwr mannagment
  uint8_t power_manage = 0;

  ESP_ERROR_CHECK(icm42670_get_power(&power_manage));

  if(power_manage == 0x0F)
  {
    ESP_LOGI(TAG,"Congrats ACELL and GYRO CONFIGD ");
  }
  else
{
    ESP_LOGI(TAG,"ENEXPECTED value");
  }
  

  if (device_id == ICM42670_DEVICE_ID) {
    ESP_LOGI(TAG, "ICM-42670-P detected successfully!");
} else {
    ESP_LOGE(
        TAG,
        "Unexpected device ID. Expected 0x%02X",
        ICM42670_DEVICE_ID
    );
}

  while(1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

}
