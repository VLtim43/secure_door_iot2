#include "display/OLED/OLED_display.h"
#include "display/led/rgb_led.h"
#include "ir/ir_remote.h"
#include "mqtt/mqtt_connect.h"
#include "rfid/rfid.h"

void app_main(void) {
  // rgb_led_start();
  // ir_remote_start();
  // mqtt_start();
  // rfid_start();
  oled_display_start();
}
