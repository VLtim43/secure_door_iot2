#ifndef RGB_LED_H
#define RGB_LED_H

typedef enum {
  RGB_LED_COLOR_OFF = 0,
  RGB_LED_COLOR_RED,
  RGB_LED_COLOR_GREEN,
  RGB_LED_COLOR_BLUE,
  RGB_LED_COLOR_YELLOW,
  RGB_LED_COLOR_MAGENTA,
  RGB_LED_COLOR_CYAN,
  RGB_LED_COLOR_WHITE,
} rgb_led_color_t;

// prepare discrete RGB LED output pins
void rgb_led_start(void);
// set one of the predefined RGB channel combinations
void rgb_led_set_color(rgb_led_color_t color);

#endif
