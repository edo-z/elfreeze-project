# Failure Log

## 2026-06-21 — Dashboard Blank: HMR WebSocket Blocked by Cloudflare
**What failed:** Dashboard page loaded HTML and CSS but never hydrated with data. Stat cards showed "---" indefinitely. Console flooded with `NS_ERROR_WEBSOCKET_CONNECTION_REFUSED` for `wss://elfreeze.aldozeno.my.id/_next/webpack-hmr`.
**Root cause:** Server ran `npm run dev` (Next.js development mode). Dev mode injects a Turbopack HMR WebSocket client that connects to the server for hot reload. The Cloudflare tunnel at `elfreeze.aldozeno.my.id` does not proxy WebSocket connections — they are refused at the edge. HMR client retries repeatedly, preventing React from completing hydration.
**Fix applied:** Changed `elfreeze.service` to use `npm run start` (production mode). Production server does not inject HMR WebSocket client. Required running `npm run build` first.
**Prevention:** Use `npm run start` (not `dev`) for any service behind Cloudflare tunnel where WebSocket proxying is not explicitly configured. Verify by checking page source for absence of HMR chunk references.

## 2026-06-21 — ESP32 Config MQTT Messages Silently Ignored
**What failed:** Save button showed "Applied" but ESP32 buzzer/display never changed. Serial monitor showed no `[CONFIG]` line. Config PUBLISH packets arrived at ESP32 (`[MQTT] RX type=0x30`) but were not processed.
**Root cause:** `setupOTA()` in `ota.h` calls `setMqttCallback(otaMessageHandler)` AFTER `setup()` already called `setMqttCallback(mqttMessageHandler)`. The OTA handler only processes topics ending in `/ota` and returns immediately for all others. Config messages on `elfreeze/{deviceId}/config` were silently dropped.
**Fix applied:** Modified `otaMessageHandler` to fall through to `handleConfigMessage()` when the topic does not end with `/ota`.
**Prevention:** Before introducing a second MQTT callback consumer (OTA handler), check that it chains existing handlers instead of overwriting. Add a comment in `setMqttCallback` about the single-callback limitation. When adding new handlers, always route unknown topics to existing handlers.
