#include "mqtt_connect.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "mqtt_client.h"
#include "nvs_flash.h"
#include "secrets.h"

static const char* TAG = "mqtt_connect";
static const char* TIMESTAMP_TOPIC = "/test";
static const gpio_num_t STATUS_LED_GPIO = GPIO_NUM_2;

static EventGroupHandle_t wifi_events;
static EventGroupHandle_t mqtt_events;
static const EventBits_t WIFI_CONNECTED = BIT0;
static const EventBits_t MQTT_CONNECTED = BIT0;

static void wifi_event_handler(void* arg, esp_event_base_t base,
                               int32_t event_id, void* data) {
  if (base == WIFI_EVENT && (event_id == WIFI_EVENT_STA_START ||
                             event_id == WIFI_EVENT_STA_DISCONNECTED)) {
    xEventGroupClearBits(wifi_events, WIFI_CONNECTED);
    esp_wifi_connect();
    return;
  }

  if (base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    xEventGroupSetBits(wifi_events, WIFI_CONNECTED);
  }
}

static void mqtt_event_handler(void* arg, esp_event_base_t base,
                               int32_t event_id, void* data) {
  if (event_id == MQTT_EVENT_CONNECTED) {
    xEventGroupSetBits(mqtt_events, MQTT_CONNECTED);
    gpio_set_level(STATUS_LED_GPIO, 1);
    ESP_LOGI(TAG, "Connected to AWS IoT");
    return;
  }

  if (event_id == MQTT_EVENT_DISCONNECTED) {
    xEventGroupClearBits(mqtt_events, MQTT_CONNECTED);
    gpio_set_level(STATUS_LED_GPIO, 0);
    ESP_LOGW(TAG, "MQTT disconnected; reconnecting automatically");
  }
}

static void init_nvs(void) {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
}

static void connect_wifi(void) {
  init_nvs();

  wifi_events = xEventGroupCreate();
  configASSERT(wifi_events);

  wifi_init_config_t wifi_init = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_init));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                             wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             wifi_event_handler, NULL));

  wifi_config_t wifi_config = {0};
  strlcpy((char*)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
  strlcpy((char*)wifi_config.sta.password, WIFI_PASSWORD,
          sizeof(wifi_config.sta.password));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  xEventGroupWaitBits(wifi_events, WIFI_CONNECTED, pdFALSE, pdTRUE,
                      portMAX_DELAY);
}

static void sync_time(void) {
  esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
  ESP_ERROR_CHECK(esp_netif_sntp_init(&config));

  while (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(30000)) != ESP_OK) {
    ESP_LOGW(TAG, "Waiting for time sync");
  }
}

static void publish_timestamp(esp_mqtt_client_handle_t client) {
  char payload[32];
  snprintf(payload, sizeof(payload), "%lld", (long long)time(NULL));
  if (esp_mqtt_client_publish(client, TIMESTAMP_TOPIC, payload, 0, 0, 0) < 0) {
    ESP_LOGW(TAG, "Timestamp publish failed");
    return;
  }

  ESP_LOGI(TAG, "Published to %s: %s", TIMESTAMP_TOPIC, payload);
}

static void publish_task(void* arg) {
  esp_mqtt_client_handle_t client = arg;

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    if (!mqtt_is_connected()) {
      continue;
    }

    publish_timestamp(client);
  }
}

void mqtt_start(void) {
  ESP_ERROR_CHECK(gpio_reset_pin(STATUS_LED_GPIO));
  ESP_ERROR_CHECK(gpio_set_direction(STATUS_LED_GPIO, GPIO_MODE_OUTPUT));
  ESP_ERROR_CHECK(gpio_set_level(STATUS_LED_GPIO, 0));

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();
  connect_wifi();
  sync_time();

  static char broker_uri[256];
  int length = snprintf(broker_uri, sizeof(broker_uri), "mqtts://%s:8883",
                        IOT_ENDPOINT);
  configASSERT(length > 0 && (size_t)length < sizeof(broker_uri));

  esp_mqtt_client_config_t config = {
      .broker.address.uri = broker_uri,
      .broker.verification.certificate = ROOT_CA,
      .credentials.client_id = THING_NAME,
      .credentials.authentication.certificate = DEVICE_CERT,
      .credentials.authentication.key = PRIVATE_KEY,
  };
  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&config);
  configASSERT(client);

  mqtt_events = xEventGroupCreate();
  configASSERT(mqtt_events);
  ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, MQTT_EVENT_ANY,
                                                 mqtt_event_handler, NULL));
  ESP_ERROR_CHECK(esp_mqtt_client_start(client));
  ESP_ERROR_CHECK(xTaskCreate(publish_task, "mqtt_publish", 4096, client, 5,
                             NULL) == pdPASS
                      ? ESP_OK
                      : ESP_ERR_NO_MEM);
}

bool mqtt_is_connected(void) {
  return mqtt_events && (xEventGroupGetBits(mqtt_events) & MQTT_CONNECTED);
}
