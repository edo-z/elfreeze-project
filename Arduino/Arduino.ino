#include <WiFi.h>
#include "config.h"
#include "sensor/sensor.h"
#include "sensor/display.h"
#include "sensor/ntp.h"
#include "sensor/alarm.h"
#include "Frimware/device.h"
#include "Frimware/firmware.h"
#include "Frimware/mqtt.h"
#include "Frimware/ota.h"

unsigned long lastBlink = 0;
bool blinkState = false;

bool prevMqttConnected = false;

void setup() {
    Serial.begin(115200);
    randomSeed(millis());

    initDevice();
    initFirmware();

    initDisplay();
    splashScreen();

    initSensor();
    initAlarm();
    initNTP();

    if (oledReady) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(10, 25);
        display.println("Connecting WiFi...");
        display.display();
    }

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected");

    String deviceId = getDeviceId();
    Serial.print("[DEVICE] ID: ");
    Serial.println(deviceId);

    setupMQTT();
    setupOTA();

    delay(1000);
}

void loop() {
    if (millis() - lastBlink > BLINK_INTERVAL) {
        blinkState = !blinkState;
        lastBlink = millis();
    }

    // MQTT + OTA lifecycle
    reconnectMQTT();
    mqttLoop();
    handleOTA();
    checkOtaUpdate();

    if (!prevMqttConnected && mqttConnected) {
        subscribeOTATopic();
        reportDeviceInfo();
    }
    prevMqttConnected = mqttConnected;

    // Sensor
    float suhu = bacaSuhu();

    char jam[10];
    getJam(jam, sizeof(jam), blinkState);

    // Serial
    Serial.print("Jam : ");
    Serial.print(jam);
    Serial.print(" | Suhu : ");
    Serial.print(suhu, 1);
    Serial.println(suhu >= batasSuhu ? " C | WARNING" : " C | NORMAL");

    // Alarm
    cekAlarm(suhu);

    // Display
    renderDisplay(suhu, blinkState, jam);

    // MQTT publish tiap INTERVAL_MQTT
    unsigned long now = millis();
    if (mqttConnected && now - lastMqttPublish >= INTERVAL_MQTT) {
        lastMqttPublish = now;

        String payload = "{";
        payload += "\"suhu\":" + String(suhu, 1) + ",";
        payload += "\"deviceId\":\"" + getDeviceId() + "\"";
        payload += "}";

        sendDataMQTT(payload);
    }

    delay(LOOP_DELAY);
}
