import { NextRequest, NextResponse } from "next/server";
import { publishThresholds } from "@/lib/mqtt-publisher";

export const dynamic = "force-dynamic";

export async function POST(request: NextRequest) {
  try {
    const { deviceId, warning, critical } = await request.json();
    if (!deviceId || typeof warning !== "number" || typeof critical !== "number") {
      return NextResponse.json({ error: "deviceId, warning, critical required" }, { status: 400 });
    }
    publishThresholds(deviceId, warning, critical);
    return NextResponse.json({ ok: true });
  } catch {
    return NextResponse.json({ error: "Invalid JSON" }, { status: 400 });
  }
}
