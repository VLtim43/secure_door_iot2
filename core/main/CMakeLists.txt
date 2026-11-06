idf_component_register(SRCS "main.c" "mqtt/mqtt_connect.c" "display/led/rgb_led.c" "display/OLED/OLED_display.c" "rfid/rfid.c" "ir/ir_remote.c"
                        INCLUDE_DIRS "."
                        REQUIRES esp_driver_gpio esp_driver_rmt esp_driver_i2c esp_wifi esp_event esp_netif nvs_flash mqtt rc522 esp_ssd1306)
