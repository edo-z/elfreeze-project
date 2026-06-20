# Elfreeze Architecture

## Overview
Temperature monitoring dashboard: ESP32-S3 with DS18B20 → MQTT over WSS → EMQX → HTTP POST → Next.js ring buffer → Charts.

## Stack
- **Frontend:** Next.js 16 (App Router, TypeScript)
- **Styling:** Tailwind CSS v4
- **UI:** shadcn/ui + react-icons
- **Charts:** Recharts (via shadcn/chart)
- **Firmware:** ESP32-S3 Arduino (no PlatformIO)
- **Broker:** EMQX 6.0.1 (native, not Docker)

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

## MQTT Transport Architecture

**Current (stable):** Raw `WiFiClientSecure` + manual WebSocket framing.
**Replaced:** `WebSocketsClient` library (had internal response parsing bugs with Cloudflare).

Key design decisions:
- TLS via `WiFiClientSecure.setInsecure()` (Cloudflare cert trusted at network level)
- WebSocket upgrade headers sent manually (`Host`, `Origin`, `Sec-WebSocket-Protocol`)
- Masking in-place on caller's local buffer, single `write()` call per frame
- Incoming frames parsed byte-by-byte from a linear `rxBuf` with `memmove` compaction
- MQTT binary frames (CONNECT, PUBLISH, SUBSCRIBE, PINGREQ) reused from prototype

## EMQX Integration
- **WebSocket listener:** port 8083 (local, no TLS — Cloudflare handles TLS termination)
- **MQTT TCP listener:** port 1883 (local, for testing)
- **Dashboard:** http://localhost:18083 (credentials: admin / ewaldo12345)
- **Rule:** `rule_elfreeze` — FROM `elfreeze/+/data` → HTTP POST http://localhost:3003/api/sensor/mqtt
  - Body: `${p}` (decoded JSON payload)
  - Content-Type: application/json

## Ring Buffer (Next.js)
- In-memory array, max 288 entries (~24h @ 5min interval, ~38min @ 8s interval)
- POST `/api/sensor/mqtt`: accepts `{"suhu": number}`, generates `time` from Date.now()
- GET `/api/sensor/readings`: returns full buffer as JSON array
- No database — data lives only in memory (lost on server restart)

## Firmware Structure (Arduino/)
```
Arduino/
├── Arduino.ino              # Main loop orchestration
├── config.h                 # All constants (WiFi, MQTT, pins, thresholds)
├── sensor/
│   ├── sensor.h             # DS18B20 read + random fallback
│   ├── display.h            # SH1106 OLED
│   ├── ntp.h                # NTP time sync
│   └── alarm.h              # Buzzer threshold
└── Frimware/
    ├── mqtt.h               # WiFiClientSecure + manual WS framing + MQTT frames
    ├── device.h             # Device identity (MAC-based, NVS)
    ├── firmware.h           # Versioning (NVS)
    └── ota.h                # Pull OTA polling (disabled if ota_base_url empty)
```

## Infrastructure
- **Subdomain:** elfreeze.aldozeno.my.id
- **MQTT endpoint:** mqtt.aldozeno.my.id (Cloudflare proxy → ws://localhost:8083)
- **Port:** 3003
- **Tunnel:** Cloudflare (tunnel ID: cddb35c3)
- **Process Manager:** systemd user service (elfreeze.service)

## Project Structure
```
src/
├── app/          # Next.js App Router pages
├── components/   # UI components (shadcn/ui + custom)
└── lib/          # Utility functions (ring-buffer.ts, temperature-data.ts)
```
