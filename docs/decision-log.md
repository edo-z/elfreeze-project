# Decision Log

## 2026-06-21 — Switch to Production Mode (npm run start)
**Decision:** Run elfreeze.service with `npm run start` instead of `npm run dev`
**Rationale:** Dev server injects HMR WebSocket client that tries to connect to `wss://elfreeze.aldozeno.my.id/_next/webpack-hmr`. Cloudflare tunnel doesn't proxy WebSocket for this subdomain, so the connection is refused. The HMR client retries in a loop, flooding console and preventing React from hydrating the page.
**Alternatives considered:** Configuring Cloudflare to proxy WebSocket on this subdomain, or removing the HMR client via config — but production mode is more appropriate for a deployed service.
**Consequences:** Requires `npm run build` before restarting the service. No live reload on code changes.

## 2026-06-21 — Default Threshold ReferenceLines on Charts
**Decision:** Add user-adjustable Warning (yellow) and Critical (red) dashed reference lines to both AreaChart and BarChart.
**Rationale:** Give users immediate visual feedback when temperature approaches dangerous levels. Input controls above charts allow real-time adjustment without editing code.
**Alternatives considered:** Hardcoding thresholds in config.h only — but dashboard visual is independent of microcontroller setpoint.
**Consequences:** Thresholds set in dashboard are also pushed to ESP32 via MQTT (see architecture — Config Push Flow). Two independent concerns: visual overlay + device setpoint.

## 2026-06-21 — Downsample Chart Data to 60 Points
**Decision:** `downsampleReadings()` in temperature-data.ts caps chart data at ~60 points by averaging `suhu` values within equal-sized buckets.
**Rationale:** Ring buffer holds 288 entries (~38min @ 8s interval). Rendering all 288 on a ~600px chart produces a solid red blob (2px per point). Downsampling produces a clear, readable trend line.
**Alternatives considered:** Filtering server-side before returning data — chose client-side downsampling to keep raw data accessible via API.
**Consequences:** Chart shows representative trend, not every data point. Tooltip shows averaged values per bucket, not raw readings.

## 2026-06-21 — Single MQTT Callback With Chaining
**Decision:** `setMqttCallback()` in `setupOTA()` overwrites callback set in `setup()` with `otaMessageHandler`. Instead of separate callbacks, `otaMessageHandler` now chains non-OTA topics to `handleConfigMessage`.
**Rationale:** Only one MQTT callback slot available. Both OTA handler and config handler need the same slot. Chaining avoids overwriting.
**Alternatives considered:** Registry of multiple callbacks iterated on each message — over-engineering for two handlers.
**Consequences:** All MQTT PUBLISH packets go through `otaMessageHandler` first. Topic ending in `/ota` handled for OTA; other topics (including `/config`) fall through to `handleConfigMessage`.

## 2026-06-21 — Explicit Save Button (Replace Silent Debounce)
**Decision:** Threshold inputs no longer auto-send via 500ms debounce. User clicks a Save button to push thresholds to ESP32.
**Rationale:** Silent debounce gave no feedback — user couldn't tell if/when the config was sent. Save button shows saving/saved/failed states with "Last applied" label.
**Alternatives considered:** Keep debounce but add toast notification — save button is simpler and more explicit.
**Consequences:** User must click Save after changing values. No risk of stale-closure bugs from debounce timers capturing outdated state.

## 2026-06-21 — Fetch deviceId From Server API Instead of Client Module
**Decision:** Browser gets deviceId via `GET /api/device/last` instead of importing `getLastDeviceId()` from `last-device.ts`.
**Rationale:** In Next.js `"use client"` components, module-level variables are separate instances between server and client bundles. The MQTT route handler updates the server's `lastDeviceId`, but the browser's instance stays `""`. Fetching from an API bridges this gap.
**Alternatives considered:** Embedding deviceId in page SSR, using React context, or polling the readings API for deviceId — dedicated API endpoint is simplest.
**Consequences:** New route `/api/device/last` created. Page fetches it every 10s alongside readings. DeviceId is available ~10s after first ESP32 data flows post-restart.
