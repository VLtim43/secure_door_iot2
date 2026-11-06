#include "ir_remote.h"

#include <stdbool.h>
#include <stdint.h>

#include "config/pins.h"
#include "driver/rmt_rx.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "ir_codes.h"

static const char* TAG = "IR_REMOTE";

enum {
  IR_COMMAND_COOLDOWN_MS = 250,
  IR_RESOLUTION_HZ = 1000000,
  IR_SYMBOL_BUFFER_SIZE = 128,
  IR_NEC_LEADER_HIGH_US = 9000,
  IR_NEC_LEADER_LOW_US = 4500,
  IR_NEC_BIT_MARK_US = 560,
  IR_NEC_ZERO_SPACE_US = 560,
  IR_NEC_ONE_SPACE_US = 1690,
  IR_NEC_LEADER_TOLERANCE_US = 1200,
  IR_NEC_BIT_TOLERANCE_US = 300,
  IR_NEC_SPACE_TOLERANCE_US = 500,
};

typedef struct {
  size_t num_symbols;
} ir_rx_event_t;

static rmt_symbol_word_t s_ir_symbols[IR_SYMBOL_BUFFER_SIZE];

// NEC timings vary slightly in practice, so decoding uses tolerance windows.
static bool approx_equal(uint32_t value, uint32_t target, uint32_t tolerance) {
  uint32_t min = target > tolerance ? target - tolerance : 0;
  uint32_t max = target + tolerance;
  return value >= min && value <= max;
}

static bool ir_remote_rx_done_callback(
    rmt_channel_handle_t channel, const rmt_rx_done_event_data_t* edata,
    void* user_data) {
  (void)channel;

  BaseType_t high_task_wakeup = pdFALSE;
  QueueHandle_t queue = (QueueHandle_t)user_data;
  ir_rx_event_t event = {
      .num_symbols = edata->num_symbols,
  };
  xQueueSendFromISR(queue, &event, &high_task_wakeup);
  return high_task_wakeup == pdTRUE;
}

static bool ir_remote_parse_nec(const rmt_symbol_word_t* symbols,
                                size_t num_symbols, uint32_t* raw_code) {
  if (num_symbols < 33) {
    return false;
  }

  if (!approx_equal(symbols[0].duration0, IR_NEC_LEADER_HIGH_US,
                    IR_NEC_LEADER_TOLERANCE_US) ||
      !approx_equal(symbols[0].duration1, IR_NEC_LEADER_LOW_US,
                    IR_NEC_LEADER_TOLERANCE_US)) {
    return false;
  }

  // NEC bits arrive least-significant bit first.
  uint32_t code = 0;
  for (size_t i = 0; i < 32; i++) {
    const rmt_symbol_word_t* symbol = &symbols[i + 1];
    if (!approx_equal(symbol->duration0, IR_NEC_BIT_MARK_US,
                      IR_NEC_BIT_TOLERANCE_US)) {
      return false;
    }

    if (approx_equal(symbol->duration1, IR_NEC_ZERO_SPACE_US,
                     IR_NEC_SPACE_TOLERANCE_US)) {
      continue;
    }

    if (approx_equal(symbol->duration1, IR_NEC_ONE_SPACE_US,
                     IR_NEC_SPACE_TOLERANCE_US)) {
      code |= 1UL << i;
      continue;
    }

    return false;
  }

  *raw_code = code;
  return true;
}

static void ir_remote_log_symbols(const rmt_symbol_word_t* symbols,
                                  size_t num_symbols) {
  size_t preview_count = num_symbols < 6 ? num_symbols : 6;

  ESP_LOGI(TAG, "raw frame with %u symbols", (unsigned)num_symbols);
  for (size_t i = 0; i < preview_count; i++) {
    ESP_LOGI(TAG, "sym[%u] l0=%u d0=%u l1=%u d1=%u", (unsigned)i,
             symbols[i].level0, symbols[i].duration0, symbols[i].level1,
             symbols[i].duration1);
  }
}

static const char* ir_remote_command_name(uint32_t raw_code) {
  switch (raw_code) {
    case IR_CODE_1:
      return "1";
    case IR_CODE_2:
      return "2";
    case IR_CODE_3:
      return "3";
    case IR_CODE_4:
      return "4";
    case IR_CODE_5:
      return "5";
    case IR_CODE_6:
      return "6";
    case IR_CODE_7:
      return "7";
    case IR_CODE_8:
      return "8";
    case IR_CODE_9:
      return "9";
    case IR_CODE_0:
      return "0";
    case IR_CODE_STAR:
      return "STAR";
    case IR_CODE_HASH:
      return "HASH";
    case IR_CODE_UP:
      return "UP";
    case IR_CODE_LEFT:
      return "LEFT";
    case IR_CODE_DOWN:
      return "DOWN";
    case IR_CODE_RIGHT:
      return "RIGHT";
    case IR_CODE_OK:
      return "OK";
    default:
      return NULL;
  }
}

static void ir_remote_task(void* arg) {
  (void)arg;

  rmt_channel_handle_t rx_channel = NULL;
  rmt_rx_channel_config_t rx_config = {
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .resolution_hz = IR_RESOLUTION_HZ,
      .mem_block_symbols = 64,
      .gpio_num = IR_RECEIVER_GPIO,
      .flags = {
          .invert_in = false,
          .with_dma = false,
      },
  };
  rmt_rx_event_callbacks_t callbacks = {
      .on_recv_done = ir_remote_rx_done_callback,
  };
  rmt_receive_config_t receive_config = {
      .signal_range_min_ns = 1000,
      .signal_range_max_ns = 12000000,
  };

  QueueHandle_t rx_queue = xQueueCreate(4, sizeof(ir_rx_event_t));
  configASSERT(rx_queue);
  ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_config, &rx_channel));
  ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rx_channel, &callbacks,
                                                  rx_queue));
  ESP_ERROR_CHECK(rmt_enable(rx_channel));

  ESP_LOGI(TAG, "listening on GPIO%d", IR_RECEIVER_GPIO);

  TickType_t last_command_tick = 0;
  bool have_last_command = false;
  while (true) {
    ir_rx_event_t event;
    uint32_t raw_code = 0;

    ESP_ERROR_CHECK(rmt_receive(rx_channel, s_ir_symbols, sizeof(s_ir_symbols),
                                &receive_config));
    if (xQueueReceive(rx_queue, &event, portMAX_DELAY) != pdTRUE) {
      continue;
    }

    if (!ir_remote_parse_nec(s_ir_symbols, event.num_symbols, &raw_code)) {
      ir_remote_log_symbols(s_ir_symbols, event.num_symbols);
      continue;
    }

    const char* command_name = ir_remote_command_name(raw_code);
    uint8_t address = raw_code & 0xFF;
    uint8_t address_inv = (raw_code >> 8) & 0xFF;
    uint8_t command = (raw_code >> 16) & 0xFF;
    uint8_t command_inv = (raw_code >> 24) & 0xFF;

    if (command_name == NULL) {
      ESP_LOGI(TAG,
               "NEC code=0x%08lX addr=0x%02X addr_inv=0x%02X cmd=0x%02X "
               "cmd_inv=0x%02X",
               (unsigned long)raw_code, address, address_inv, command,
               command_inv);
      continue;
    }

    TickType_t now = xTaskGetTickCount();
    // Drop near-duplicate repeats from one button hold.
    if (have_last_command &&
        (now - last_command_tick) < pdMS_TO_TICKS(IR_COMMAND_COOLDOWN_MS)) {
      continue;
    }

    last_command_tick = now;
    have_last_command = true;
    ESP_LOGI(TAG,
             "Button=%s NEC code=0x%08lX addr=0x%02X addr_inv=0x%02X "
             "cmd=0x%02X cmd_inv=0x%02X",
             command_name, (unsigned long)raw_code, address, address_inv,
             command, command_inv);
  }
}

void ir_remote_start(void) {
  configASSERT(xTaskCreate(ir_remote_task, "ir_remote", 4096, NULL, 5, NULL) ==
               pdPASS);
}
