#ifndef OTA_H
#define OTA_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include "../config.h"
#include "mqtt.h"
#include "firmware.h"

// -------------------------------------------------------------
// GLOBAL STATE
// -------------------------------------------------------------
String otaDeviceId = "";
bool otaPending = false;
String pendingUrl = "";
String pendingMd5 = "";
String pendingVersion = "";

unsigned long lastOtaPoll = 0;

// -------------------------------------------------------------
// UTILITY
// -------------------------------------------------------------
static String extractJsonValue(const char* json, const char* key) {
    String search = "\"" + String(key) + "\":\"";
    int start = String(json).indexOf(search);
    if (start < 0) return "";
    start += search.length();
    int end = String(json).indexOf("\"", start);
    if (end < 0) return "";
    return String(json).substring(start, end);
}

// -------------------------------------------------------------
// PUSH OTA — MQTT handler
// -------------------------------------------------------------
void otaMessageHandler(String &topic, String &payload) {
    if (!topic.endsWith("/ota")) {
        handleConfigMessage(topic, payload);
        return;
    }

    pendingUrl = extractJsonValue(payload.c_str(), "url");
    pendingMd5 = extractJsonValue(payload.c_str(), "md5");
    pendingVersion = extractJsonValue(payload.c_str(), "version");

    if (pendingUrl.length() > 0) {
        otaPending = true;
        Serial.println("[OTA] Push command received");
    }
}

// -------------------------------------------------------------
// DOWNLOAD + FLASH
// -------------------------------------------------------------
void performOtaUpdate() {
    if (!otaPending || pendingUrl.length() == 0) return;

    Serial.println("[OTA] Downloading...");
    HTTPClient http;
    WiFiClientSecure streamClient;
    streamClient.setInsecure();

    http.begin(streamClient, pendingUrl);
    http.addHeader("Cache-Control", "no-cache");
    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[OTA] HTTP error: %d\n", httpCode);
        otaPending = false;
        http.end();
        return;
    }

    int contentLength = http.getSize();
    if (contentLength <= 0) {
        Serial.println("[OTA] No Content-Length");
        otaPending = false;
        http.end();
        return;
    }

    if (pendingMd5.length() > 0) {
        Update.setMD5(pendingMd5.c_str());
    }

    if (!Update.begin(contentLength)) {
        otaPending = false;
        http.end();
        return;
    }

    WiFiClient* stream = http.getStreamPtr();
    size_t written = 0;
    uint8_t buffer[1024];

    while (http.connected() && written < contentLength) {
        size_t available = stream->available();
        if (available) {
            int bytesRead = stream->readBytes(buffer, min(available, sizeof(buffer)));
            written += Update.write(buffer, bytesRead);
        }
        delay(1);
    }

    if (!Update.end(true)) {
        Serial.printf("[OTA] Failed: %s\n", Update.errorString());
        otaPending = false;
        http.end();
        return;
    }

    Serial.println("[OTA] Success, rebooting...");
    delay(500);
    ESP.restart();
}

void handleOTA() {
    if (strlen(ota_base_url) == 0) return;
    if (otaPending && otaDeviceId.length() > 0) {
        performOtaUpdate();
    }
}

// -------------------------------------------------------------
// PULL OTA — HTTP polling
// -------------------------------------------------------------
void checkOtaUpdate() {
    if (strlen(ota_base_url) == 0) return;
    if ((unsigned long)(millis() - lastOtaPoll) < OTA_POLL_INTERVAL) return;
    lastOtaPoll = millis();

    if (WiFi.status() != WL_CONNECTED) return;

    HTTPClient http;
    WiFiClientSecure clientSecure;
    clientSecure.setInsecure();

    String url = String(ota_base_url) + "/api/firmware/latest";
    http.begin(clientSecure, url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();
        String version = extractJsonValue(response.c_str(), "version");
        String fwUrl = extractJsonValue(response.c_str(), "url");
        String md5 = extractJsonValue(response.c_str(), "md5");

        if (version.length() > 0 && fwUrl.length() > 0 && isNewerVersion(version.c_str())) {
            pendingUrl = fwUrl;
            pendingMd5 = md5;
            pendingVersion = version;
            otaPending = true;
            Serial.printf("[OTA] New version: %s\n", version.c_str());
        }
    }
    http.end();
}

// -------------------------------------------------------------
// SETUP
// -------------------------------------------------------------
void setupOTA() {
    otaDeviceId = getDeviceId();
    setMqttCallback(otaMessageHandler);
}

#endif
