#include "OLED_display.h"

#include "config/pins.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "ssd1306.h"

static const char* TAG = "OLED";
static ssd1306_handle_t s_display_handle;

void oled_display_show_text(const char* text) {
  if (s_display_handle == NULL || text == NULL) {
    return;
  }

  ESP_ERROR_CHECK(ssd1306_clear_display(s_display_handle, false));
  for (unsigned page = 0; page < 8 && *text != '\0'; page++) {
    char line[17];
    unsigned length = 0;
    while (length < sizeof(line) - 1 && *text != '\0' && *text != '\n') {
      line[length++] = *text++;
    }
    line[length] = '\0';
    if (*text == '\n') {
      text++;
    }
    if (length > 0) {
      ESP_ERROR_CHECK(ssd1306_display_text(s_display_handle, page, line, false));
    }
  }
}

void oled_display_start(void) {
  i2c_master_bus_config_t bus_config = {
      .i2c_port = I2C_NUM_0,
      .sda_io_num = OLED_I2C_SDA_GPIO,
      .scl_io_num = OLED_I2C_SCL_GPIO,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .flags.enable_internal_pullup = true,
  };

  i2c_master_bus_handle_t bus_handle = NULL;
  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));
  ssd1306_config_t display_config = I2C_SSD1306_128x64_CONFIG_DEFAULT;
  ESP_ERROR_CHECK(ssd1306_init(bus_handle, &display_config, &s_display_handle));
  ESP_LOGI(TAG, "ready on SDA=%d SCL=%d", OLED_I2C_SDA_GPIO,
           OLED_I2C_SCL_GPIO);
  oled_display_show_text("Secure Door");
}
