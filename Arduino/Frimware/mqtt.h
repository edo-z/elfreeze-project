#ifndef MY_MQTT_H
#define MY_MQTT_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "../config.h"
#include "device.h"

// -------------------------------------------------------------
// GLOBAL STATE
// -------------------------------------------------------------
WiFiClientSecure mqttWsClient;
bool wsConnected = false;
bool mqttConnected = false;
unsigned long lastMqttPing = 0;
unsigned long lastMqttPublish = 0;
unsigned long lastConnectAttempt = 0;

// Receive buffer
uint8_t rxBuf[2048];
uint16_t rxPos = 0;

void (*mqttCallback)(String &topic, String &payload) = nullptr;

void setMqttCallback(void (*callback)(String &topic, String &payload)) {
    mqttCallback = callback;
}

// -------------------------------------------------------------
// BASE64 (minimal, for WebSocket key)
// -------------------------------------------------------------
static const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64Encode(const uint8_t* input, size_t inLen, char* output, size_t outLen) {
    size_t i = 0, o = 0;
    while (i < inLen && o + 4 <= outLen) {
        uint32_t b = (i < inLen ? input[i++] : 0) << 16;
        b |= (i < inLen ? input[i++] : 0) << 8;
        b |= (i < inLen ? input[i++] : 0);
        output[o++] = b64chars[(b >> 18) & 0x3F];
        output[o++] = b64chars[(b >> 12) & 0x3F];
        output[o++] = (i - 2 == inLen) ? '=' : b64chars[(b >> 6) & 0x3F];
        output[o++] = (i - 1 == inLen) ? '=' : b64chars[b & 0x3F];
    }
    output[o] = '\0';
}

// -------------------------------------------------------------
// WEBSOCKET FRAMING
// -------------------------------------------------------------

void sendWsFrame(uint8_t opcode, uint8_t* data, size_t len) {
    if (!mqttWsClient.connected()) return;

    uint8_t maskKey[4];
    for (int i = 0; i < 4; i++) maskKey[i] = random(0xFF);

    uint8_t header[10];
    size_t hLen = 0;
    header[hLen++] = 0x80 | opcode;

    if (len < 126) {
        header[hLen++] = 0x80 | len;
    } else if (len < 65536) {
        header[hLen++] = 0x80 | 126;
        header[hLen++] = (len >> 8) & 0xFF;
        header[hLen++] = len & 0xFF;
    } else {
        return;
    }

    memcpy(&header[hLen], maskKey, 4);
    hLen += 4;

    // Mask payload in-place (caller buffers are local, safe to modify)
    for (size_t i = 0; i < len; i++) {
        data[i] ^= maskKey[i % 4];
    }

    // Single write: header + masked payload
    mqttWsClient.write(header, hLen);
    if (len > 0) {
        mqttWsClient.write(data, len);
    }
}

// -------------------------------------------------------------
// MANUAL MQTT BINARY FRAMES (send via WebSocket)
// -------------------------------------------------------------

static uint8_t mqttEncodeLength(uint32_t len, uint8_t *buf) {
    uint8_t i = 0;
    do {
        uint8_t b = len % 128;
        len /= 128;
        if (len > 0) b |= 0x80;
        buf[i++] = b;
    } while (len > 0);
    return i;
}

void sendMqttConnect() {
    String clientId = "ELFREEZE-" + String(random(0xffff), HEX);
    uint16_t idLen = clientId.length();
    uint16_t uLen = strlen(mqtt_user);
    uint16_t pLen = strlen(mqtt_pass);

    uint8_t flags = 0x02;
    if (uLen > 0) flags |= 0x80;
    if (pLen > 0) flags |= 0x40;

    uint16_t varLen = 10 + 2 + idLen;
    if (uLen > 0) varLen += 2 + uLen;
    if (pLen > 0) varLen += 2 + pLen;

    uint8_t var[512];
    uint16_t pos = 0;
    var[pos++] = 0x00; var[pos++] = 0x04;
    var[pos++] = 'M'; var[pos++] = 'Q';
    var[pos++] = 'T'; var[pos++] = 'T';
    var[pos++] = 0x04;
    var[pos++] = flags;
    var[pos++] = 0x00; var[pos++] = MQTT_KEEPALIVE;
    var[pos++] = (idLen >> 8) & 0xFF; var[pos++] = idLen & 0xFF;
    memcpy(&var[pos], clientId.c_str(), idLen); pos += idLen;
    if (uLen > 0) {
        var[pos++] = (uLen >> 8) & 0xFF; var[pos++] = uLen & 0xFF;
        memcpy(&var[pos], mqtt_user, uLen); pos += uLen;
    }
    if (pLen > 0) {
        var[pos++] = (pLen >> 8) & 0xFF; var[pos++] = pLen & 0xFF;
        memcpy(&var[pos], mqtt_pass, pLen); pos += pLen;
    }

    uint8_t lenBuf[4];
    uint8_t lenSize = mqttEncodeLength(pos, lenBuf);

    uint8_t pkt[4 + 256];
    uint16_t pktPos = 0;
    pkt[pktPos++] = 0x10;
    memcpy(&pkt[pktPos], lenBuf, lenSize); pktPos += lenSize;
    memcpy(&pkt[pktPos], var, pos); pktPos += pos;

    sendWsFrame(0x02, pkt, pktPos);
}

void sendMqttPublish(const char* topic, const char* payload, bool retain) {
    uint16_t tLen = strlen(topic);
    uint16_t pLen = strlen(payload);
    uint16_t varLen = 2 + tLen + pLen;

    uint8_t var[512];
    uint16_t pos = 0;
    var[pos++] = (tLen >> 8) & 0xFF;
    var[pos++] = tLen & 0xFF;
    memcpy(&var[pos], topic, tLen); pos += tLen;
    memcpy(&var[pos], payload, pLen); pos += pLen;

    uint8_t lenBuf[4];
    uint8_t lenSize = mqttEncodeLength(pos, lenBuf);

    uint8_t fixed = 0x30;
    if (retain) fixed |= 0x01;

    uint8_t pkt[4 + 512];
    uint16_t pktPos = 0;
    pkt[pktPos++] = fixed;
    memcpy(&pkt[pktPos], lenBuf, lenSize); pktPos += lenSize;
    memcpy(&pkt[pktPos], var, pos); pktPos += pos;

    sendWsFrame(0x02, pkt, pktPos);
}

void sendMqttSubscribe(const char* topic) {
    static uint16_t packetId = 1;
    uint16_t tLen = strlen(topic);
    uint16_t varLen = 2 + tLen + 1;

    uint8_t var[512];
    uint16_t pos = 0;
    uint16_t pid = packetId++;
    var[pos++] = (pid >> 8) & 0xFF;
    var[pos++] = pid & 0xFF;
    var[pos++] = (tLen >> 8) & 0xFF;
    var[pos++] = tLen & 0xFF;
    memcpy(&var[pos], topic, tLen); pos += tLen;
    var[pos++] = 0;

    uint8_t lenBuf[4];
    uint8_t lenSize = mqttEncodeLength(pos, lenBuf);

    uint8_t pkt[4 + 256];
    uint16_t pktPos = 0;
    pkt[pktPos++] = 0x82;
    memcpy(&pkt[pktPos], lenBuf, lenSize); pktPos += lenSize;
    memcpy(&pkt[pktPos], var, pos); pktPos += pos;

    sendWsFrame(0x02, pkt, pktPos);
}

void sendMqttPing() {
    uint8_t pkt[2] = {0xC0, 0x00};
    sendWsFrame(0x02, pkt, 2);
}

// -------------------------------------------------------------
// MQTT PACKET HANDLER
// -------------------------------------------------------------

void handleMqttPacket(uint8_t* data, size_t len) {
    if (len < 2) return;
    uint8_t type = data[0] & 0xF0;
    Serial.print("[MQTT] RX type=0x");
    Serial.print(type, HEX);
    Serial.print(" len=");
    Serial.println(len);

    switch (type) {
        case 0x20: {
            if (len < 4) break;
            uint8_t rc = data[3];
            if (rc == 0) {
                mqttConnected = true;
                lastMqttPing = millis();
                Serial.println("[MQTT] Connected");
            } else {
                Serial.printf("[MQTT] CONNACK rejected: rc=%d\n", rc);
            }
            break;
        }
        case 0x30: {
            size_t pos = 1;
            uint32_t remLen = 0;
            uint8_t mul = 1;
            do {
                remLen += (data[pos] & 0x7F) * mul;
                mul *= 128;
            } while ((data[pos++] & 0x80) && pos < len);

            if (pos + 2 > len) break;
            uint16_t tLen = (data[pos] << 8) | data[pos + 1];
            pos += 2;
            if (pos + tLen > len) break;

            String topic = String((char*)&data[pos], tLen);
            pos += tLen;
            if ((data[0] & 0x06) >> 1 >= 1) pos += 2;
            String payload = String((char*)&data[pos], len - pos);

            if (mqttCallback) {
                mqttCallback(topic, payload);
            }
            break;
        }
        case 0x90: break;
        case 0xD0: break;
    }
}

// -------------------------------------------------------------
// WEBSOCKET DATA READER
// -------------------------------------------------------------

void processWsData() {
    if (!mqttWsClient.connected()) return;

    while (mqttWsClient.available()) {
        if (rxPos >= sizeof(rxBuf)) {
            Serial.println("[WS] BUFFER OVERFLOW!");
            rxPos = 0;
        }
        rxBuf[rxPos++] = mqttWsClient.read();
    }

    while (rxPos >= 2) {
        uint8_t opcode = rxBuf[0] & 0x0F;
        bool fin = rxBuf[0] & 0x80;
        bool masked = rxBuf[1] & 0x80;
        uint64_t payloadLen = rxBuf[1] & 0x7F;
        size_t headerLen = 2;

        if (payloadLen == 126) {
            if (rxPos < 4) return;
            payloadLen = (rxBuf[2] << 8) | rxBuf[3];
            headerLen += 2;
        } else if (payloadLen == 127) {
            return; // too large
        }

        uint8_t maskKey[4] = {0};
        if (masked) {
            if (rxPos < headerLen + 4) return;
            memcpy(maskKey, &rxBuf[headerLen], 4);
            headerLen += 4;
        }

        if (rxPos < headerLen + payloadLen) return;

        uint8_t* payload = &rxBuf[headerLen];

        if (masked) {
            for (uint64_t i = 0; i < payloadLen; i++) {
                payload[i] ^= maskKey[i % 4];
            }
        }

        if (opcode == 0x02 || (opcode == 0x00 && fin)) {
            handleMqttPacket(payload, payloadLen);
        } else if (opcode == 0x08) {
            Serial.print("[WS] Close frame received");
            if (payloadLen >= 2) {
                uint16_t code = (payload[0] << 8) | payload[1];
                Serial.printf(" (code=%d)", code);
            }
            Serial.println();
            mqttWsClient.stop();
            wsConnected = false;
            mqttConnected = false;
            rxPos = 0;
            return;
        } else if (opcode == 0x09) {
            sendWsFrame(0x0A, nullptr, 0);
        }

        size_t frameSize = headerLen + payloadLen;
        if (rxPos > frameSize) {
            memmove(rxBuf, &rxBuf[frameSize], rxPos - frameSize);
        }
        rxPos -= frameSize;
    }
}

// -------------------------------------------------------------
// WEBSOCKET UPGRADE
// -------------------------------------------------------------

void sendWsUpgrade() {
    uint8_t randKey[16];
    for (int i = 0; i < 16; i++) randKey[i] = random(0xFF);
    char b64Key[25];
    base64Encode(randKey, 16, b64Key, sizeof(b64Key));

    mqttWsClient.printf("GET %s HTTP/1.1\r\n", mqtt_ws_path);
    mqttWsClient.printf("Host: %s\r\n", mqtt_server);
    mqttWsClient.print("Connection: Upgrade\r\n");
    mqttWsClient.print("Upgrade: websocket\r\n");
    mqttWsClient.print("Sec-WebSocket-Version: 13\r\n");
    mqttWsClient.printf("Sec-WebSocket-Key: %s\r\n", b64Key);
    mqttWsClient.print("Sec-WebSocket-Protocol: mqtt\r\n");
    mqttWsClient.print("Origin: https://mqtt.aldozeno.my.id\r\n");
    mqttWsClient.print("User-Agent: Mozilla/5.0\r\n");
    mqttWsClient.print("\r\n");
}

bool checkWsUpgrade() {
    if (!mqttWsClient.available()) return false;

    String response;
    unsigned long timeout = millis() + 3000;
    while (millis() < timeout) {
        while (mqttWsClient.available()) {
            char c = mqttWsClient.read();
            response += c;
            if (response.endsWith("\r\n\r\n")) {
                goto response_done;
            }
        }
        delay(10);
    }
    response_done:

    if (response.indexOf("101") >= 0) {
        Serial.println("[WS] Upgrade OK (101)");
        wsConnected = true;
        rxPos = 0;
        sendMqttConnect();
        return true;
    }

    Serial.printf("[WS] Upgrade GAGAL. Response:\n%s\n", response.c_str());
    mqttWsClient.stop();
    return false;
}

// -------------------------------------------------------------
// HIGH-LEVEL API
// -------------------------------------------------------------

void setupMQTT() {
    Serial.printf("[MQTT] Free heap: %d\n", ESP.getFreeHeap());
    mqttWsClient.setInsecure();
    mqttWsClient.setTimeout(10);
}

void wsConnect() {
    if (mqttWsClient.connect(mqtt_server, mqtt_port)) {
        Serial.println("[WS] TCP + TLS OK");
        sendWsUpgrade();
    } else {
        Serial.println("[WS] TCP + TLS GAGAL");
    }
}

void reconnectMQTT() {
    unsigned long now = millis();

    if (mqttWsClient.connected() && wsConnected && mqttConnected) {
        return;
    }

    if (!mqttWsClient.connected()) {
        wsConnected = false;
        mqttConnected = false;
        if (now - lastConnectAttempt >= 10000) {
            lastConnectAttempt = now;
            wsConnect();
        }
        return;
    }

    if (mqttWsClient.connected() && !wsConnected) {
        if (checkWsUpgrade()) {
            Serial.println("[WS] WebSocket established");
        }
        return;
    }
}

void mqttLoop() {
    if (mqttWsClient.connected()) {
        processWsData();
    }

    if (mqttConnected && millis() - lastMqttPing >= MQTT_PING_INTERVAL) {
        lastMqttPing = millis();
        sendMqttPing();
    }
}

void subscribeMQTT(const char* topic, uint8_t qos) {
    if (!mqttConnected) return;
    sendMqttSubscribe(topic);
}

void publishMQTT(const char* topic, const char* payload, bool retain = false) {
    if (!mqttConnected) return;
    sendMqttPublish(topic, payload, retain);
}

void sendDataMQTT(String payload) {
    String deviceId = getDeviceId();
    String topic = String(TOPIC_PREFIX) + "/" + deviceId + "/data";
    publishMQTT(topic.c_str(), payload.c_str());
}

void subscribeOTATopic() {
    String deviceId = getDeviceId();
    String topic = String(TOPIC_PREFIX) + "/" + deviceId + "/ota";
    subscribeMQTT(topic.c_str(), 0);
}

void subscribeConfigTopic() {
    String deviceId = getDeviceId();
    String topic = String(TOPIC_PREFIX) + "/" + deviceId + "/config";
    Serial.print("[SUBSCRIBE] ");
    Serial.println(topic);
    subscribeMQTT(topic.c_str(), 0);
}

void handleConfigMessage(String &topic, String &payload) {
    if (!topic.endsWith("/config")) return;

    int wIdx = payload.indexOf("\"warning\"");
    int cIdx = payload.indexOf("\"critical\"");
    if (wIdx >= 0) {
        float val = payload.substring(payload.indexOf(':', wIdx) + 1).toFloat();
        configWarning = val;
    }
    if (cIdx >= 0) {
        float val = payload.substring(payload.indexOf(':', cIdx) + 1).toFloat();
        configCritical = val;
    }
    Serial.print("[CONFIG] warning=");
    Serial.print(configWarning);
    Serial.print(" critical=");
    Serial.println(configCritical);
}

#endif
