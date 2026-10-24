#include "rfid.h"

#include "config/pins.h"
#include "display/rgb_led.h"
#include "driver/rc522_spi.h"
#include "esp_log.h"
#include "rc522.h"
#include "rc522_picc.h"

static const char* TAG = "RFID";
static rc522_driver_handle_t driver;
static rc522_handle_t scanner;
static rc522_spi_config_t driver_config = {
    .host_id = SPI3_HOST,
    .bus_config =
        &(spi_bus_config_t){
            .miso_io_num = RFID_MISO_GPIO,
            .mosi_io_num = RFID_MOSI_GPIO,
            .sclk_io_num = RFID_SCLK_GPIO,
        },
    .dev_config =
        {
            .spics_io_num = RFID_SDA_GPIO,
        },
    .rst_io_num = RFID_RST_GPIO,
};

static void on_picc_state_changed(void* arg, esp_event_base_t base,
                                  int32_t event_id, void* data) {
  rc522_picc_state_changed_event_t* event = data;
  if (event->picc->state == RC522_PICC_STATE_IDLE &&
      event->old_state >= RC522_PICC_STATE_ACTIVE) {
    rgb_led_set_color(RGB_LED_COLOR_OFF);
    ESP_LOGI(TAG, "Card removed");
    return;
  }

  if (event->picc->state != RC522_PICC_STATE_ACTIVE) {
    return;
  }

  char uid[RC522_PICC_UID_STR_BUFFER_SIZE_MAX];
  if (rc522_picc_uid_to_str(&event->picc->uid, uid, sizeof(uid)) == ESP_OK) {
    ESP_LOGI(TAG, "Card UID: %s", uid);
    rgb_led_set_color(RGB_LED_COLOR_GREEN);
  }
}

void rfid_start(void) {
  ESP_ERROR_CHECK(rc522_spi_create(&driver_config, &driver));
  ESP_ERROR_CHECK(rc522_driver_install(driver));

  rc522_config_t scanner_config = {.driver = driver};
  ESP_ERROR_CHECK(rc522_create(&scanner_config, &scanner));

  ESP_ERROR_CHECK(rc522_register_events(scanner, RC522_EVENT_PICC_STATE_CHANGED,
                                        on_picc_state_changed, NULL));
  ESP_ERROR_CHECK(rc522_start(scanner));
}
