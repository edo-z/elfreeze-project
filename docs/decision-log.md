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
