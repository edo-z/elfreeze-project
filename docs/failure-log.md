# Failure Log

## 2026-06-21 — Dashboard Blank: HMR WebSocket Blocked by Cloudflare
**What failed:** Dashboard page loaded HTML and CSS but never hydrated with data. Stat cards showed "---" indefinitely. Console flooded with `NS_ERROR_WEBSOCKET_CONNECTION_REFUSED` for `wss://elfreeze.aldozeno.my.id/_next/webpack-hmr`.
**Root cause:** Server ran `npm run dev` (Next.js development mode). Dev mode injects a Turbopack HMR WebSocket client that connects to the server for hot reload. The Cloudflare tunnel at `elfreeze.aldozeno.my.id` does not proxy WebSocket connections — they are refused at the edge. HMR client retries repeatedly, preventing React from completing hydration.
**Fix applied:** Changed `elfreeze.service` to use `npm run start` (production mode). Production server does not inject HMR WebSocket client. Required running `npm run build` first.
**Prevention:** Use `npm run start` (not `dev`) for any service behind Cloudflare tunnel where WebSocket proxying is not explicitly configured. Verify by checking page source for absence of HMR chunk references.
