<!-- BEGIN:nextjs-agent-rules -->
# This is NOT the Next.js you know

This version has breaking changes — APIs, conventions, and file structure may all differ from your training data. Read the relevant guide in `node_modules/next/dist/docs/` before writing any code. Heed deprecation notices.
<!-- END:nextjs-agent-rules -->

# Elfreeze — Temperature Dashboard

## Stack
- **Framework:** Next.js 16 (TypeScript)
- **CSS:** Tailwind CSS v4
- **UI Library:** shadcn/ui + react-icons
- **Dev Server Port:** 3003
- **Subdomain:** elfreeze.aldozeno.my.id
- **Tunnel:** Cloudflare (cddb35c3)

## Commands
- `npm run dev` — start dev server on port 3003
- `npm run build` — production build
- `npm run start` — start production server
- `npm run lint` — lint check

## Project Notes
- Dashboard presents temperature data via MCP protocol
- MCP architecture TBD — may connect to an MCP server serving temperature data
- Uses `@modelcontextprotocol/inspector` for MCP server development/testing

## Current State (2026-06-21)
- Dashboard renders with live temperature data, stat cards, charts, readings table
- Threshold inputs with Save button push config to ESP32 via MQTT with retain:true
- Config flow verified working: Save → POST /api/device/config → mqtt-publisher → EMQX → ESP32
- ESP32 firmware has: MQTT via raw WiFiClientSecure + WebSocket, OTA pull, dynamic thresholds
- **Known issue:** DS18B20 reads -127°C (sensor not detected on GPIO4), random fallback active
- **Known issue:** OTA upload via IP fails ("could not open port"), always falls back to serial
- **Known issue:** DeviceId cleared on server restart, page needs ~10s to recapture
- System runs in production mode (npm run start) behind Cloudflare tunnel
