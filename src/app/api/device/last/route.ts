import { NextResponse } from "next/server";
import { getLastDeviceId } from "@/lib/last-device";

export const dynamic = "force-dynamic";

export async function GET() {
  return NextResponse.json({ deviceId: getLastDeviceId() || null });
}
