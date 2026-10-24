#include "rgb_led.h"

#include <stdbool.h>

#include "config/pins.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char* TAG = "RGB_LED";

static void rgb_led_set_channel(gpio_num_t gpio, bool enabled) {
  ESP_ERROR_CHECK(gpio_set_level(gpio, enabled ? 1 : 0));
}

void rgb_led_start(void) {
  gpio_config_t config = {
      .pin_bit_mask = (1ULL << RGB_LED_RED_GPIO) |
                      (1ULL << RGB_LED_GREEN_GPIO) | (1ULL << RGB_LED_BLUE_GPIO),
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };

  ESP_ERROR_CHECK(gpio_config(&config));
  rgb_led_set_color(RGB_LED_COLOR_OFF);
  ESP_LOGI(TAG, "ready on R=%d G=%d B=%d", RGB_LED_RED_GPIO,
           RGB_LED_GREEN_GPIO, RGB_LED_BLUE_GPIO);
}

void rgb_led_set_color(rgb_led_color_t color) {
  bool red = false;
  bool green = false;
  bool blue = false;

  switch (color) {
    case RGB_LED_COLOR_RED:
      red = true;
      break;
    case RGB_LED_COLOR_GREEN:
      green = true;
      break;
    case RGB_LED_COLOR_BLUE:
      blue = true;
      break;
    case RGB_LED_COLOR_YELLOW:
      red = true;
      green = true;
      break;
    case RGB_LED_COLOR_MAGENTA:
      red = true;
      blue = true;
      break;
    case RGB_LED_COLOR_CYAN:
      green = true;
      blue = true;
      break;
    case RGB_LED_COLOR_WHITE:
      red = true;
      green = true;
      blue = true;
      break;
    case RGB_LED_COLOR_OFF:
    default:
      break;
  }

  rgb_led_set_channel(RGB_LED_RED_GPIO, red);
  rgb_led_set_channel(RGB_LED_GREEN_GPIO, green);
  rgb_led_set_channel(RGB_LED_BLUE_GPIO, blue);
}
