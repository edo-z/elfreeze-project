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
