import { NextRequest, NextResponse } from "next/server";
import { pushReading } from "@/lib/ring-buffer";
import { setLastDeviceId } from "@/lib/last-device";

export async function POST(request: NextRequest) {
  try {
    const body = await request.json();
    const suhu = body.suhu ?? body.p?.suhu;

    if (suhu === undefined || typeof suhu !== "number") {
      return NextResponse.json({ error: "Field 'suhu' required" }, { status: 400 });
    }

    pushReading(suhu);

    const deviceId = body.deviceId ?? body.topic?.split("/")?.[1];
    if (deviceId) {
      setLastDeviceId(deviceId);
      console.log(`[DEVICE] Captured deviceId: ${deviceId}`);
    }

    return NextResponse.json({ ok: true });
  } catch {
    return NextResponse.json({ error: "Invalid JSON" }, { status: 400 });
  }
}
