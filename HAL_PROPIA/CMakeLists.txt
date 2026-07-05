idf_component_register(SRCS "src/hal_gpio.c"
                       INCLUDE_DIRS "inc"
                       REQUIRES driver esp_driver_gpio)