#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <math.h>
#include "driver/i2c_master.h"
#include "driver/ledc.h" //adding our ledc PWM for motors
#include "driver/gpio.h" // allows us to call gpio names 
#include "esp_timer.h"
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

  //function for our ledc_timer_config  to control motors LEDDCC 
  ledc_timer_config_t motor1_config = {
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_10_BIT,
    .timer_num = LEDC_TIMER_0,
    .freq_hz = 1000,
    .clk_cfg = LEDC_AUTO_CLK,

  };

  //config for our channel to controll motors LEDC
  ledc_channel_config_t motor1_channel_config = {
    .gpio_num = 1,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .timer_sel = LEDC_TIMER_0,
    .duty = 0,// since we dont have motor connect and we dont want motor to instantly start up 0 = 0%, 512 = 50% , 1023 = 100%
    .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,

  };


  //GPIO CONFIGS
  gpio_config_t gpios_config = {
    .pin_bit_mask = (1ULL << 3) | (1ULL << 0) | (1ULL <<4), // tells our esp which gpio pins this config applies too and rihgt now we have it say gpio0 or 3 or 4 
    .mode = GPIO_MODE_OUTPUT,// we want these pins to just output signals 0/1 
    .pull_up_en = GPIO_PULLUP_DISABLE, //we are not sing any internal pull up 
    .pull_down_en = GPIO_PULLDOWN_DISABLE, // we do not need pull down 
    .intr_type = GPIO_INTR_DISABLE, // we are not using any interupts in these pins 
  };

  //calling out GPIO config 
  ESP_ERROR_CHECK(gpio_config(&gpios_config));


  //calling out config 
  ESP_ERROR_CHECK(ledc_timer_config(&motor1_config));

  //calling our channel config 
  ESP_ERROR_CHECK(ledc_channel_config(&motor1_channel_config));


  //going to turn our TB6612 driver off to make sure we are safe 
  ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_4,0));

  //make sure our AIN1 and 2 are 0 

  ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_3,0));
  ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_0,0));

  //make sure our duty is 0 before we start 
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0,0));

  ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0));

  vTaskDelay(pdMS_TO_TICKS(5000)); //wait 5 seconds before starting motor 


  //make motor move forward 
  ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_3,1));
  ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_0,0));
 
  //going to set our duty 
ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0,153)); // params spped_mode, channel, duty values 
ESP_LOGI(TAG,"SET DUTY: 15%");

//then we need to update our duty 
ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0));// params spped_mode and channel
ESP_LOGI(TAG,"UPDATE DUTY");

vTaskDelay(pdMS_TO_TICKS(1000));
  ESP_LOGI(TAG,"WAIT");


//setting our driver to active 
  ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_4,1));


  /*
ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0,1023));
ESP_LOGI(TAG,"SET DUTY: 100%");

  ledc_update_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0);
ESP_LOGI(TAG,"UPDATE");

 vTaskDelay( pdMS_TO_TICKS(1000));
ESP_LOGI(TAG,"WAIT");

  */

  ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0,0));

  ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0));



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

//this is going to check if we can read out IMU data again (every 5ms)
  uint8_t data_ready = 0;
 



//this fucntion is going to try and read data from our accel 
  int16_t accel_x=0 , accel_y=0,accel_z=0;


  //function that is going to read our gyro data 
  int16_t gyro_x = 0, gyro_y =0, gyro_z =0;


  // going to hold our values of the converted to actuall meaningfull data like g or degrees 
  float gyro_conv_x = 0;
  float gyro_conv_y= 0;
  float gyro_conv_z = 0;


  //going to hold values for our accel conversion
  float accel_conv_x = 0;
  float accel_conv_y = 0;
  float accel_conv_z = 0;

  //these are going to hold the corrected conversions with bias so that we dont have gyro drift build up as much 
  float gyro_conv_x_corrected = 0 ;
  float gyro_conv_y_corrected= 0;
  float gyro_conv_z_corrected = 0;

  //defining our prev_time
  int64_t prev_time = 0;
  
  //gyro prediction for current loop 
  float gyro_angle_predicted =0 ;


// complementary filter 
  float filtered_angle = 0;

//gyro old reading 
  float gyro_old =0;

  float gyro_count =0;

  float gyro_sum = 0;

  float bias =0;



// calibration loop that will dtermine our gyro bias  
while(gyro_count < 1000)
  {
//this is going to be our start time for our 5ms gyro 200hz read
  //int64_t start_gyro_read_timer = esp_timer_get_time();

    //this is going to get rid of the code above so we dont use esp timers and actually only move on once we have data ready bit 
  ESP_ERROR_CHECK(icm42670_get_data_ready(&data_ready));

    if(data_ready == 0x01)
    {
  ESP_ERROR_CHECK(icm42670_read_gyro(&gyro_x,&gyro_y,&gyro_z));



  ESP_LOGI(TAG,"gyro_x: %d \n gyro_y: %d \n gyro_z: %d",gyro_x,gyro_y,gyro_z);
 


  //going to create the conversion from GYRO data into physical units example g for accelorometer and /s for gyroscope 
  gyro_conv_x = (float)gyro_x / 65.5;
  gyro_conv_y = (float)gyro_y / 65.5;
  gyro_conv_z = (float)gyro_z / 65.5;


 printf("GYRO_X = %f \n GYRO_Y = %f \n GYRO_Z = %f\n", gyro_conv_x,gyro_conv_y,gyro_conv_z);


     gyro_sum += gyro_conv_x;

/*int64_t total_time = 0;
    int64_t end_gyro_read_timer =0 ;
    while(total_time <= 5000)
    { 
      end_gyro_read_timer = esp_timer_get_time();
      total_time = end_gyro_read_timer - start_gyro_read_timer;
    }
*/
gyro_count++;
  
    ESP_LOGI(TAG,"Waited 5ms");}
  }

bias = gyro_sum / 1000;


  printf("this is the bias: %f\n", bias);


prev_time = esp_timer_get_time();
  //normal loop 
  while(1)
  {
   //checking if our gyro or accel is ready to read again 
  ESP_ERROR_CHECK(icm42670_get_data_ready(&data_ready));
    if(data_ready == 0x01)
    {

  //keeps track of 200hz 5ms timing 
  //int64_t working_loop_timer_start = esp_timer_get_time();
  ESP_ERROR_CHECK(icm42670_read_gyro(&gyro_x,&gyro_y,&gyro_z));
  ESP_ERROR_CHECK(icm42670_read_accel(&accel_x,&accel_y,&accel_z)); //these are going in ass addreses since we defined that in our icm.c file they are pointers so they should be pointing to the addr of where we want to store them 


//converting our raw data form accel into real data 

  accel_conv_x = (float)accel_x / 8192;
  accel_conv_y = (float)accel_y / 8192;
  accel_conv_z = (float)accel_z / 8192;


  //going to create the conversion from GYRO data into physical units example g for accelorometer and /s for gyroscope 
  gyro_conv_x_corrected = ((float)gyro_x / 65.5) - (bias); // only need x axis since this is our pitch angle for robot 


  //creating the dt so the time difference between prv time and new time 
  int64_t now = esp_timer_get_time();
  float elapsed_time = now - prev_time;
    prev_time = now;
  elapsed_time = elapsed_time / 1000000;
  float dt = 0;
  dt = elapsed_time;



//to get our change in angle duirng dt from gyro
  float angle_change = 0;
    angle_change = gyro_conv_x_corrected * dt ;

    float gyro_new = filtered_angle + angle_change; 

    gyro_angle_predicted = gyro_new;


  //going to calculate the angle 
  float pitch_deg =0 ;
  float pitch_rad = 0;
  pitch_rad = atan2f(-accel_conv_x,accel_conv_z);
  pitch_deg = (pitch_rad * 180) / M_PI;




    //complementary filter 
    filtered_angle = ((0.98)*(gyro_angle_predicted)) + ((1-0.98)*(pitch_deg));
  

    //printing the actuall filtered angle 
//    ESP_LOGI(TAG,"Filtered Angle: %f\n", filtered_angle);
  //    ESP_LOGI(TAG,"Aceel pitch: %f\n", pitch_deg);
   //   ESP_LOGI(TAG,"ACCEL x: %f Z: %f\n", accel_conv_x,accel_conv_z);
ESP_LOGI(TAG, "Accel: %.2f | Filtered: %.2f", pitch_deg, filtered_angle);

    }
  
   /* printf("Filtered Angle: %f\n", filtered_angle);

  printf("Accel pitch: %f\n",pitch_deg);


    
    printf("raw/converted gyro: %d\n", gyro_x);
    printf("gyro_conv_x_corrected: %f\n", gyro_conv_x_corrected);

    printf("angle we are moving at: %f\n", gyro_new);
  

    int64_t working_loop_timer_end=0;
    int64_t check_total_time=0;
    while(check_total_time < 5000)
    {
      working_loop_timer_end = esp_timer_get_time();
      check_total_time = working_loop_timer_end -working_loop_timer_start;
    }
    */
  }
}
