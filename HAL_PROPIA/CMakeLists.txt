idf_component_register(SRCS "src/hal_gpio.c" "src/hal_i2c.c" "src/hal_adc.c" "src/hal_ssd1306.c"
                       INCLUDE_DIRS "inc"
                       REQUIRES driver esp_driver_gpio esp_driver_i2c esp_adc)