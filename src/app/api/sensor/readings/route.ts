import { NextResponse } from "next/server";
import { getReadings } from "@/lib/ring-buffer";

export const dynamic = "force-dynamic";

export async function GET() {
  return NextResponse.json(getReadings());
}
