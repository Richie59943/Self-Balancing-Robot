#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <math.h>
#include "driver/i2c_master.h"

#include "icm42670.h"

#define ESP_SDA_GPIO 7 
#define ESP_SCL_GPIO 8

static const char *TAG = "BALANCER";

//IMU CONFIG 
//ACCEL: +-4 g @ 200 
//GYRO: +-500 /s @200 
//ACCEL sensativity: 8192 LSB/g 
//GYRO sensativity: 65.5 LSB 


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


//calling our gyro config in main 
  ESP_ERROR_CHECK(icm42670_config_gyro());
  ESP_LOGI(TAG,"GYRO Configured");

  if (device_id == ICM42670_DEVICE_ID) {
    ESP_LOGI(TAG, "ICM-42670-P detected successfully!");
} else {
    ESP_LOGE(
        TAG,
        "Unexpected device ID. Expected 0x%02X",
        ICM42670_DEVICE_ID
    );
}


  uint8_t address_check =0;
//going to read the value within our gyro addrrr 
ESP_ERROR_CHECK(icm42670_get_gyro_config(&address_check));
  ESP_LOGI(TAG,"GYRO = 0x%02X",address_check);

  if (address_check == 0x48)
  {
    ESP_LOGI(TAG,"GYRo config verified");

  }

  else{
    ESP_LOGE(TAG, "error");
  }
  


ESP_ERROR_CHECK(icm42670_config_accel());
  ESP_LOGI(TAG,"accel aconfigured");

uint8_t accel_addr =0;
//going to read the value within our accel addr and see if we have the correct one 
ESP_ERROR_CHECK(icm42670_get_accel_config(&accel_addr));
  ESP_LOGI(TAG, "ACCEL = 0x%02X", accel_addr);

  if (accel_addr == 0x48)
  {
    ESP_LOGI(TAG, " accel config verified");

  }
  else{
    ESP_LOGE(TAG,"Failed");
  }


//this fucntion is going to try and read data from our accel 
  int16_t accel_x=0 , accel_y=0,accel_z=0;
  ESP_ERROR_CHECK(icm42670_read_accel(&accel_x,&accel_y,&accel_z)); //these are going in ass addreses since we defined that in our icm.c file they are pointers so they should be pointing to the addr of where we want to store them 

  ESP_LOGI(TAG, "ACCEL_X val: %d \n ACEEL_Y val: %d \n ACCEL_Z val: %d \n ",accel_x,accel_y,accel_z);
  


  //function that is going to read our gyro data 
  int16_t gyro_x = 0, gyro_y =0, gyro_z =0;
  ESP_ERROR_CHECK(icm42670_read_gyro(&gyro_x,&gyro_y,&gyro_z));
  ESP_LOGI(TAG,"gyro_x: %d \n gyro_y: %d \n gyro_z: %d",gyro_x,gyro_y,gyro_z);
 


  //going to create the conversion from RAW ACCEL and GYRO data into physical units example g for accelorometer and /s for gyroscope 

  float accel_conv_x = (float)accel_x / 8192;
  float accel_conv_y = (float)accel_y / 8192;
  float accel_conv_z = (float)accel_z / 8192;

  float gyro_conv_x = (float)gyro_x / 65.5;
  float gyro_conv_y = (float)gyro_y / 65.5;
  float gyro_conv_z = (float)gyro_z / 65.5;



  printf("ACCEL_X = %f \n ACCEL_Y = %f \n ACCEL_Z = %f\n", accel_conv_x, accel_conv_y, accel_conv_z); 
  printf("GYRO_X = %f \n GYRO_Y = %f \n GYRO_Z = %f\n", gyro_conv_x,gyro_conv_y,gyro_conv_z);


  //going to calculate the angle 
  float pitch_deg =0 ;
  float pitch_rad = 0;
  pitch_rad = atan2f(-accel_x,accel_z);
  pitch_deg = (pitch_rad * 180) / M_PI;


  printf("this is the pitch: %f\n",pitch_deg);



    while(1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

}
