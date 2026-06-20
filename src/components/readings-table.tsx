"use client";

import { Thermometer } from "lucide-react";
import type { TempReading } from "@/lib/temperature-data";

function statusLabel(suhu: number): { label: string; color: string } {
  if (suhu > 38) return { label: "HOT", color: "bg-[rgba(239,68,68,0.1)] text-[#EF4444]" };
  if (suhu < 31) return { label: "LOW", color: "bg-[rgba(59,130,246,0.1)] text-[#3B82F6]" };
  return { label: "OK", color: "bg-[rgba(16,185,129,0.1)] text-[#10B981]" };
}

export default function ReadingsTable({ readings }: { readings?: TempReading[] }) {
  if (readings && readings.length > 0) {
    const recent = readings.slice(-8).reverse();
    return (
      <div className="rounded-[8px] border border-[#E5E7EB] bg-white p-3 shadow-subtle">
        <p className="mb-2 font-body text-[10px] font-semibold uppercase tracking-[0.06em] text-[#9CA3AF]">
          Recent Readings
        </p>
        <div className="overflow-hidden rounded-[6px] border border-[#E5E7EB]">
          <table className="w-full text-left">
            <thead>
              <tr className="border-b border-[#E5E7EB] bg-[#F9FAFB]">
                <th className="px-3 py-2 font-body text-[10px] font-semibold uppercase tracking-[0.04em] text-[#6B7280]">Sensor</th>
                <th className="px-3 py-2 font-body text-[10px] font-semibold uppercase tracking-[0.04em] text-[#6B7280]">Value</th>
                <th className="px-3 py-2 font-body text-[10px] font-semibold uppercase tracking-[0.04em] text-[#6B7280]">Time</th>
                <th className="px-3 py-2 font-body text-[10px] font-semibold uppercase tracking-[0.04em] text-[#6B7280]">Status</th>
              </tr>
            </thead>
            <tbody>
              {recent.map((r, i) => {
                const status = statusLabel(r.suhu);
                return (
                  <tr
                    key={i}
                    className="border-b border-[#E5E7EB] last:border-0 transition-colors duration-80 hover:bg-[#EEF2FF]"
                  >
                    <td className="flex items-center gap-2 px-3 py-2.5">
                      <Thermometer className="size-3.5 text-[#9CA3AF]" />
                      <span className="font-body text-[12px] font-medium text-[#374151]">Temperature Sensor</span>
                    </td>
                    <td className="px-3 py-2.5 font-mono text-[12px] font-medium text-[#111827]">{r.suhu.toFixed(1)}°C</td>
                    <td className="px-3 py-2.5 font-body text-[12px] text-[#6B7280]">{r.time}</td>
                    <td className="px-3 py-2.5">
                      <span className={`inline-flex items-center gap-1 rounded-full px-1.5 py-0.5 font-body text-[9px] font-semibold uppercase tracking-[0.04em] ${status.color}`}>
                        <span className="inline-block size-[4px] rounded-full bg-current" />
                        {status.label}
                      </span>
                    </td>
                  </tr>
                );
              })}
            </tbody>
          </table>
        </div>
      </div>
    );
  }

  return (
    <div className="rounded-[8px] border border-[#E5E7EB] bg-white p-3 shadow-subtle">
      <p className="mb-2 font-body text-[10px] font-semibold uppercase tracking-[0.06em] text-[#9CA3AF]">
        Recent Readings
      </p>
      <p className="py-6 text-center font-body text-[12px] text-[#9CA3AF]">
        No sensor data yet
      </p>
    </div>
  );
}
