# Elfreeze Architecture

## Overview
Temperature monitoring dashboard: ESP32-S3 with DS18B20 → MQTT over WSS → EMQX → HTTP POST → Next.js ring buffer → Charts.

## Stack
- **Frontend:** Next.js 16 (App Router, TypeScript)
- **Styling:** Tailwind CSS v4
- **UI:** shadcn/ui + react-icons
- **Charts:** Recharts
- **Firmware:** ESP32-S3 Arduino (no PlatformIO)
- **Broker:** EMQX 6.0.1 (native, not Docker)
- **Process Manager:** systemd user service (elfreeze.service)

## Data Flow
```
[ESP32-S3] ──WSS──> [Cloudflare] ──tunnel──> [EMQX :8083 WS]
                                                 │
                                      rule: elfreeze/+/data
                                                 │
                                            HTTP POST
                                                 │
                                            [Next.js :3003]
                                           POST /api/sensor/mqtt
                                           GET  /api/sensor/readings
                                                 │
                                            Ring Buffer (288 max)
                                                 │
                                            Dashboard Charts
```

## Config Push Flow (Dashboard → ESP32)
```
[Dashboard Input] ─debounce 500ms─> POST /api/device/config
                                            │
                                    mqtt-publisher.ts (TCP :1883)
                                            │
                                    EMQX (forward to subscriber)
                                            │
                                    elf/{deviceId}/config
                                            │
                                    ESP32 handleConfigMessage()
                                            │
                            configWarning / configCritical updated
                                            │
                              alarm.h (buzzer pattern changes)
```

Thresholds set in dashboard are pushed to ESP32 via MQTT on explicit **Save** button click (not auto-debounce). The MQTT publisher connects from Next.js to EMQX on `localhost:1883` (TCP, no Cloudflare) and publishes with `{ qos: 0, retain: true }`. Using `retain: true` ensures the ESP32 always receives the latest setpoint on every reconnect + subscribe — even if the message was published while the ESP32 was offline (clean session, no persistent subscriptions).

ESP32 subscribes to `elfreeze/{deviceId}/config` and updates `configWarning` / `configCritical` on receipt. Callback is handled by `otaMessageHandler` which chains non-OTA topics to `handleConfigMessage()` — preventing the OTA setup from overwriting the config handler.

Buzzer behavior changes dynamically without reboot.

## MQTT Transport Architecture

**Sensor data (ESP32 → Cloud):** Raw `WiFiClientSecure` + manual WebSocket framing.
**Replaced:** `WebSocketsClient` library (had internal response parsing bugs with Cloudflare).

**Config data (Cloud → ESP32):** `mqtt` npm package via TCP to `localhost:1883`.

Key design decisions:
- TLS via `WiFiClientSecure.setInsecure()` (Cloudflare cert trusted at network level)
- WebSocket upgrade headers sent manually (`Host`, `Origin`, `Sec-WebSocket-Protocol`)
- Masking in-place on caller's local buffer, single `write()` call per frame
- Incoming frames parsed byte-by-byte from a linear `rxBuf` with `memmove` compaction
- MQTT binary frames (CONNECT, PUBLISH, SUBSCRIBE, PINGREQ) reused from prototype

## EMQX Integration
- **WebSocket listener:** port 8083 (local, no TLS — Cloudflare handles TLS termination)
- **MQTT TCP listener:** port 1883 (local, for MQTT publisher from Next.js)
- **Dashboard:** http://localhost:18083 (credentials: admin / ewaldo12345)
- **Rule:** `rule_elfreeze` — FROM `elfreeze/+/data` → HTTP POST http://localhost:3003/api/sensor/mqtt
  - Body: `${p}` (decoded JSON payload)
  - Content-Type: application/json

## Ring Buffer (Next.js)
- In-memory array, max 288 entries (~24h @ 5min interval, ~38min @ 8s interval)
- POST `/api/sensor/mqtt`: accepts `{"suhu": number}`, generates `time` from Date.now()
- GET `/api/sensor/readings`: returns full buffer as JSON array
- No database — data lives only in memory (lost on server restart)
- Also extracts `deviceId` from incoming data and stores in `last-device.ts` for config routing
- Browser fetches deviceId via `GET /api/device/last` (not directly from `last-device.ts` module, because `"use client"` components load a separate module instance)

## Chart Rendering
- **Downsampling:** `downsampleReadings()` caps chart data to ~60 points by averaging suhu within equal-sized buckets. Prevents chart bloat (288 → solid red blob at 2px/pt).
- **Threshold lines:** `ReferenceLine` from Recharts draws dashed Warning (yellow, configurable) and Critical (red, configurable) lines on both AreaChart and BarChart.
- **Stat cards:** Computed live from readings array (Current Temp, Average, Max, Min). No hardcoded demo data.
- **Readings table:** Last 8 readings from API with status indicator (HOT/OK/LOW).

## Firmware Structure (Arduino/)
```
Arduino/
├── Arduino.ino              # Main loop orchestration
├── config.h                 # All constants (WiFi, MQTT, pins, thresholds)
├── sensor/
│   ├── sensor.h             # DS18B20 read + random fallback
│   ├── display.h            # SH1106 OLED
│   ├── ntp.h                # NTP time sync
│   └── alarm.h              # Buzzer (dynamic configWarning / configCritical)
└── Frimware/
    ├── mqtt.h               # WiFiClientSecure + manual WS framing + MQTT frames
    ├── device.h             # Device identity (MAC-based, NVS)
    ├── firmware.h           # Versioning (NVS)
    └── ota.h                # Pull OTA polling (disabled if ota_base_url empty)
```

Thresholds are now dynamic, stored as mutable globals `configWarning` (default 35) and `configCritical` (default 38) in config.h. Updated on receipt of MQTT config message. Buzzer beeps fast (200ms cycle) when >= critical, slow (1s cycle) when >= warning, off otherwise.

## Infrastructure
- **Subdomain:** elfreeze.aldozeno.my.id
- **MQTT endpoint:** mqtt.aldozeno.my.id (Cloudflare proxy → ws://localhost:8083)
- **Port:** 3003
- **Tunnel:** Cloudflare (tunnel ID: cddb35c3)
- **Process Manager:** systemd user service (elfreeze.service)
- **Mode:** Production (`npm run start` — dev mode breaks via Cloudflare due to HMR WebSocket)

## Project Structure
```
src/
├── app/          # Next.js App Router pages
│   └── api/
│       ├── sensor/mqtt/    # POST — receive data from EMQX
│       ├── sensor/readings/# GET — ring buffer
│       ├── device/config/  # POST — push thresholds to ESP32
│       └── device/last/    # GET — last seen deviceId (bridge server→client)
├── components/   # UI components
├── lib/
│   ├── ring-buffer.ts      # In-memory data store
│   ├── temperature-data.ts # fetch + downsample + mock
│   ├── last-device.ts      # Last seen deviceId storage
│   └── mqtt-publisher.ts   # MQTT client (TCP :1883) for config push
```
