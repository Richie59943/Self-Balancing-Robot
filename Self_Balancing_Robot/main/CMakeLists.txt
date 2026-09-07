idf_component_register(SRCS "self_balancing_robot.c" "icm42670.c"
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS ""
                       REQUIRES esp_driver_i2c esp_timer esp_driver_ledc esp_driver_gpio)
