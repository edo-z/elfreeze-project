"use client";

import { Activity } from "lucide-react";
import {
  AreaChart,
  Area,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer,
} from "recharts";
import { downsampleReadings, type TempReading } from "@/lib/temperature-data";

function ChartTooltip({
  active,
  payload,
  label,
}: {
  active?: boolean;
  payload?: { color?: string; name?: string; value?: number }[];
  label?: string;
}) {
  if (!active || !payload) return null;
  return (
    <div className="rounded-lg border border-[#E5E7EB] bg-white px-3 py-2 shadow-medium">
      <p className="font-body mb-1 text-[10px] font-semibold uppercase tracking-[0.06em] text-[#9CA3AF]">
        {label}
      </p>
      {payload.map((entry) => (
        <div key={entry.name} className="flex items-center gap-2 py-0.5">
          <span className="inline-block size-[6px] rounded-full" style={{ backgroundColor: entry.color }} />
          <span className="font-body text-[11px] text-[#374151]">{entry.name}</span>
          <span className="ml-auto font-mono text-[11px] font-medium text-[#374151]">
            {entry.value?.toFixed(1)}°C
          </span>
        </div>
      ))}
    </div>
  );
}

export default function ChartOverview({ data }: { data: TempReading[] }) {
  const chartData = downsampleReadings(data);
  return (
    <div className="rounded-[8px] border border-[#E5E7EB] bg-white p-3 shadow-subtle">
      <div className="flex items-center justify-between">
        <p className="font-body text-[10px] font-semibold uppercase tracking-[0.06em] text-[#9CA3AF]">
          Temperature Trend
        </p>
        <Activity className="size-3 text-[#9CA3AF]" />
      </div>
      <div className="mt-3 h-[250px]">
        <ResponsiveContainer width="100%" height="100%">
          <AreaChart data={chartData} margin={{ top: 4, right: 0, left: -18, bottom: 0 }}>
            <defs>
              <linearGradient id="ov-grad-suhu" x1="0" y1="0" x2="0" y2="1">
                <stop offset="0%" stopColor="#EF4444" stopOpacity={0.15} />
                <stop offset="100%" stopColor="#EF4444" stopOpacity={0} />
              </linearGradient>
            </defs>
            <CartesianGrid stroke="#E5E7EB" strokeDasharray="2 2" />
            <XAxis
              dataKey="time"
              tick={{ fill: "#9CA3AF", fontSize: 9, fontFamily: "DM Sans" }}
              tickLine={false}
              axisLine={false}
              interval={3}
            />
            <YAxis
              tick={{ fill: "#9CA3AF", fontSize: 9, fontFamily: "DM Sans" }}
              tickLine={false}
              axisLine={false}
              tickFormatter={(v: number) => `${v}°`}
              domain={[30, 40]}
            />
            <Tooltip content={<ChartTooltip />} />
            <Area
              type="monotone"
              dataKey="suhu"
              name="Temperature"
              stroke="#EF4444"
              strokeWidth={1.5}
              fill="url(#ov-grad-suhu)"
              dot={false}
              activeDot={{ r: 3, strokeWidth: 0, fill: "#EF4444" }}
            />
          </AreaChart>
        </ResponsiveContainer>
      </div>
    </div>
  );
}
