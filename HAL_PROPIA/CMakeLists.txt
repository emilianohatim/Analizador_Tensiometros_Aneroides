idf_component_register(SRCS "src/hal_gpio.c" "src/hal_i2c.c"
                       INCLUDE_DIRS "inc"
                       REQUIRES driver esp_driver_gpio esp_driver_i2c)