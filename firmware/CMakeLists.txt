idf_component_register(SRCS "main.c" "mqtt/mqtt_connect.c" "display/rgb_led.c" "rfid/rfid.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_driver_gpio esp_wifi esp_event esp_netif nvs_flash mqtt rc522)
