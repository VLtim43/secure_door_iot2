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

static int ir_remote_digit(uint32_t code) {
  static const uint32_t digit_codes[] = {
      IR_CODE_0, IR_CODE_1, IR_CODE_2, IR_CODE_3, IR_CODE_4,
      IR_CODE_5, IR_CODE_6, IR_CODE_7, IR_CODE_8, IR_CODE_9,
  };

  for (int digit = 0; digit < 10; digit++) {
    if (code == digit_codes[digit]) {
      return digit;
    }
  }

  return -1;
}

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

  while (true) {
    ir_rx_event_t event;
    uint32_t raw_code = 0;

    ESP_ERROR_CHECK(rmt_receive(rx_channel, s_ir_symbols, sizeof(s_ir_symbols),
                                &receive_config));
    if (xQueueReceive(rx_queue, &event, portMAX_DELAY) != pdTRUE) {
      continue;
    }

    if (!ir_remote_parse_nec(s_ir_symbols, event.num_symbols, &raw_code)) {
      continue;
    }

    int digit = ir_remote_digit(raw_code);
    if (digit >= 0) {
      ESP_LOGI(TAG, "code: 0x%08lX, number: %d", (unsigned long)raw_code,
               digit);
    } else {
      ESP_LOGI(TAG, "code: 0x%08lX, number: unknown",
               (unsigned long)raw_code);
    }
  }
}

void ir_remote_start(void) {
  configASSERT(xTaskCreate(ir_remote_task, "ir_remote", 4096, NULL, 5, NULL) ==
               pdPASS);
}
