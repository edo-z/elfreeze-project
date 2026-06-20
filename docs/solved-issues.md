# Solved Issues — ESP32 MQTT + OTA

> Diadaptasi dari Prototype minimal-system-ota (30 solved issues).
> Hanya issues yang relevan dengan Elfreeze dicatat di sini.

## MQTT over WSS

### 1. beginSSL Parameter Order (Critical)
```cpp
// Signature: beginSSL(host, port, url, fingerprint, protocol)
// Parameter ke-5 WAJIB "mqtt" (bukan default "arduino")
webSocket.beginSSL("mqtt.domain", 443, "/mqtt", "", "mqtt");
```
Jika protocol tidak diset `"mqtt"`, EMQX akan reject koneksi.

### 2. TLS Buffer Flush — ESP32 Core 3.x (Critical)
`WiFiClientSecure::write()` buffer data tanpa kirim ke wire.
HTTP Upgrade request (~400 bytes) tidak cukup trigger flush.
**Patch di** `WebSocketsClient.cpp:760`:
```cpp
write(client, (uint8_t *)handshake.c_str(), handshake.length());
if (client->tcp) {
    client->tcp->flush();
}
```

### 3. Stack Allocation — jangan `new` (Critical)
`WiFiClientSecure` harus stack allocation, bukan heap:
```cpp
// ✅ BENAR — stack
WiFiClientSecure streamClient;

// ❌ SALAH — heap, risk fragmentation + crash
WiFiClientSecure *streamClient = new WiFiClientSecure();
```

### 4. MQTT Reconnect Crash — Manual Frames
Library `MQTTClient`/`PubSubClient` memicu crash saat reconnect (heap corruption dari lwmqtt).
**Solusi:** Manual MQTT binary frames (CONNECT/PUBLISH/SUBSCRIBE/PINGREQ) via WebSocket sendBIN.

### 5. CONNACK Delay
Setelah WebSocket terhubung, jangan kirim CONNECT langsung — tunggu 1-2 detik.
Diimplementasikan via `lastMqttAuth` delay 15 detik di `reconnectMQTT()`.

## OTA

### 6. Content-Length Wajib (Critical)
Server WAJIB mengirim header `Content-Length` di response download firmware.
Jika tidak ada, `HTTPClient::getSize()` return 0 dan OTA dibatalkan.
```javascript
// Express
res.setHeader("Content-Length", stat.size);
```

### 7. Pull Interval Minimum 60 detik
Interval polling jangan kurang dari 60 detik — Cloudflare rate limit.
```cpp
#define OTA_POLL_INTERVAL 60000
```

### 8. Push OTA Unreliable
MQTT push callback (`otaMessageHandler`) tidak selalu terpanggil.
**Pull adalah mekanisme utama**, push hanya akselerasi.

### 9. OTA Data Partition Warning
Setelah serial flash, ESP32 kadang warning "OTA data partition invalid".
**Aman — harmless**, auto-fix setelah OTA pertama via mekanisme pull/push.

### 10. MQTT SUBSCRIBE Wajib Packet Identifier (Critical)
**Akar:** `sendMqttSubscribe()` membangun frame SUBSCRIBE tanpa 2-byte Packet Identifier (`packetId`). EMQX menutup koneksi (WebSocket close code 1000) ~7 detik setelah CONNACK.

**Fix:** Tambah `packetId` di variable header SUBSCRIBE sebelum topic filter:
```cpp
var[pos++] = (pid >> 8) & 0xFF;  // Packet Identifier MSB
var[pos++] = pid & 0xFF;         // Packet Identifier LSB
var[pos++] = (tLen >> 8) & 0xFF; // Topic length MSB
var[pos++] = tLen & 0xFF;        // Topic length LSB
```
**Gejala:** Koneksi MQTT berhasil (CONNACK diterima), tapi server disconnect setelah ~7 detik tanpa pesan error.

**Prevention:** Setiap frame SUBSCRIBE/UNSUBSCRIBE harus memiliki 2-byte Packet Identifier sesuai spesifikasi MQTT 3.1.1.

### 11. EMQX HTTP Action 308 Redirect (Critical)
**Akar:** EMQX connector URL `http://localhost:3003/api/sensor/mqtt` dan action path `http://localhost:3003/api/sensor/mqtt` terduplikasi — Next.js merespon 308 redirect (trailing slash) yang tidak dihandle EMQX.

**Fix:** Pisahkan connector URL (base) dan action path (relatif):
```
Connector URL: http://localhost:3003
Action path:   /api/sensor/mqtt
```
**Gejala:** `actions.failed.unknown = 16`, `actions.success = 0`, log EMQX menampilkan `status_code => 308`.

**Prevention:** Di EMQX 6.x, gunakan connector untuk base URL dan action path untuk path saja. Jangan masukkan full URL di keduanya.
