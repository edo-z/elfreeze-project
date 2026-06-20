import { NextRequest, NextResponse } from "next/server";
import { pushReading } from "@/lib/ring-buffer";

export async function POST(request: NextRequest) {
  try {
    const body = await request.json();
    const suhu = body.suhu ?? body.p?.suhu;

    if (suhu === undefined || typeof suhu !== "number") {
      return NextResponse.json({ error: "Field 'suhu' required" }, { status: 400 });
    }

    pushReading(suhu);
    return NextResponse.json({ ok: true });
  } catch {
    return NextResponse.json({ error: "Invalid JSON" }, { status: 400 });
  }
}
