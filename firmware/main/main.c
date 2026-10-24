#include "display/rgb_led.h"
#include "mqtt/mqtt_connect.h"
#include "rfid/rfid.h"

void app_main(void) {
  rgb_led_start();
  rfid_start();
  mqtt_start();
}
