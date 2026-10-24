#pragma once

// Copy this file to secrets.h and fill in your own values.
static const char WIFI_SSID[] = "YOUR_WIFI_SSID";
static const char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";
static const char IOT_ENDPOINT[] = "YOUR_ENDPOINT-ats.iot.us-east-1.amazonaws.com";
static const char THING_NAME[] = "secure-door-01";

// In secrets.h, encode PEM lines as adjacent C strings, ending each line with \n.
// Include the complete BEGIN/END lines too.
static const char ROOT_CA[] = "PASTE_FULL_AMAZON_ROOT_CA_1_PEM_HERE";
static const char DEVICE_CERT[] = "PASTE_FULL_DEVICE_CERTIFICATE_PEM_HERE";
static const char PRIVATE_KEY[] = "PASTE_FULL_DEVICE_PRIVATE_KEY_PEM_HERE";
