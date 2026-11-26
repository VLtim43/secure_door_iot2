#include "OLED_display.h"

#include "config/pins.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "qrcodegen.h"
#include "ssd1306.h"

static const char* TAG = "OLED";
static ssd1306_handle_t s_display_handle;

// void oled_display_show_text(const char* text) {
//   if (s_display_handle == NULL || text == NULL) {
//     return;
//   }

//   ESP_ERROR_CHECK(ssd1306_clear_display(s_display_handle, false));
//   for (unsigned page = 0; page < 8 && *text != '\0'; page++) {
//     char line[17];
//     unsigned length = 0;
//     while (length < sizeof(line) - 1 && *text != '\0' && *text != '\n') {
//       line[length++] = *text++;
//     }
//     line[length] = '\0';
//     if (*text == '\n') {
//       text++;
//     }
//     if (length > 0) {
//       ESP_ERROR_CHECK(ssd1306_display_text(s_display_handle, page, line,
//       false));
//     }
//   }
// }

void oled_display_qrcode(const char* text) {
  if (s_display_handle == NULL || text == NULL) {
    return;
  }

  uint8_t temp[qrcodegen_BUFFER_LEN_FOR_VERSION(1)];
  uint8_t qr[qrcodegen_BUFFER_LEN_FOR_VERSION(1)];

  if (!qrcodegen_encodeText(text, temp, qr, qrcodegen_Ecc_MEDIUM, 1, 1,
                            qrcodegen_Mask_AUTO, false)) {
    ESP_LOGE(TAG, "Text does not fit Version 1 QR");
    return;
  }

  const int size = qrcodegen_getSize(qr);  // 21
  const int scale = 3;
  const int border = 0;  // required quiet zone, in modules
  const int x0 = (128 - (size + 2 * border) * scale) / 2;
  const int y0 = (64 - (size + 2 * border) * scale) / 2;
  ESP_ERROR_CHECK(ssd1306_clear_display(s_display_handle, false));

  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      if (!qrcodegen_getModule(qr, x, y)) {
        continue;
      }

      for (int dy = 0; dy < scale; dy++) {
        for (int dx = 0; dx < scale; dx++) {
          ESP_ERROR_CHECK(ssd1306_set_pixel(
              s_display_handle, x0 + (x + border) * scale + dx,
              y0 + (y + border) * scale + dy, false));
        }
      }
    }
  }

  ESP_ERROR_CHECK(ssd1306_display_pages(s_display_handle));
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
  ESP_LOGI(TAG, "ready on SDA=%d SCL=%d", OLED_I2C_SDA_GPIO, OLED_I2C_SCL_GPIO);
  ESP_ERROR_CHECK(ssd1306_clear_display(s_display_handle, false));
  ESP_ERROR_CHECK(
      ssd1306_display_text(s_display_handle, 3, "Initializing...", false));
  vTaskDelay(pdMS_TO_TICKS(1200));
  oled_display_qrcode("JBSWY3DPEHPK3PXP");
}
