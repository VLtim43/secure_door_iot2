#pragma once

#include <stdbool.h>

void mqtt_start(void);
bool mqtt_is_connected(void);
// Returns the MQTT message ID, or -1 if the scan could not be queued.
int mqtt_publish_card_uid(const char* uid);
