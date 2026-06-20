#ifndef CONFIG_H
#define CONFIG_H

// WiFi
const char* ssid     = "Milik Alfarezzz";
const char* password = "alfarezzganteng";

// OLED SH1106
#define OLED_ADDR   0x3C
#define SDA_PIN     21
#define SCL_PIN     22

// DS18B20
#define DS18B20_PIN 4

// Buzzer
#define BUZZER_PIN  14

// Threshold (default, bisa di-override via MQTT config)
float configWarning = 35.0;
float configCritical = 38.0;

// Running text
const char* companyText = "PT ELFREZEE TECH";

// Timing (ms)
#define BLINK_INTERVAL  500
#define LOOP_DELAY      5

// MQTT over WSS
const char* mqtt_server    = "mqtt.aldozeno.my.id";
const int   mqtt_port      = 443;
const char* mqtt_ws_path   = "/mqtt";
const char* mqtt_protocol  = "mqtt";
const char* mqtt_user      = "";
const char* mqtt_pass      = "";

#define MQTT_KEEPALIVE      60
#define MQTT_PING_INTERVAL  30000
#define TOPIC_PREFIX        "elfreeze"

// OTA
const char* ota_base_url    = "";
#define OTA_POLL_INTERVAL   60000

// Loop intervals (ms)
#define INTERVAL_MQTT       8000

// NVS namespaces
#define NVS_DEVICE          "device"
#define NVS_FIRMWARE        "firmware"

#endif
