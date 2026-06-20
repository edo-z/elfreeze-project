"use client";

import { LineChart, Line, ResponsiveContainer } from "recharts";
import { TrendingUp, TrendingDown, Minus } from "lucide-react";

export interface StatCardProps {
  title: string;
  value: string;
  subtitle: string;
  trend: "up" | "down" | "neutral";
  data: number[];
  color?: string;
}

const TREND_ICON = {
  up: TrendingUp,
  down: TrendingDown,
  neutral: Minus,
};

const TREND_COLOR = {
  up: "text-[#10B981]",
  down: "text-[#EF4444]",
  neutral: "text-[#9CA3AF]",
};

const TREND_BG = {
  up: "bg-[rgba(16,185,129,0.1)] text-[#10B981]",
  down: "bg-[rgba(239,68,68,0.1)] text-[#EF4444]",
  neutral: "bg-[rgba(0,0,0,0.05)] text-[#9CA3AF]",
};

function SparklineChart({ data, color }: { data: number[]; color?: string }) {
  const chartData = data.map((v) => ({ v }));
  return (
    <div className="h-[50px] w-full">
      <ResponsiveContainer width="100%" height="100%">
        <LineChart data={chartData}>
          <Line
            type="monotone"
            dataKey="v"
            stroke={color || "#6366F1"}
            strokeWidth={2}
            dot={false}
            isAnimationActive={false}
          />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
}

export default function StatCard({ title, value, subtitle, trend, data, color }: StatCardProps) {
  const TrendIcon = TREND_ICON[trend];
  const trendLabel = trend === "up" ? "+25%" : trend === "down" ? "-25%" : "+5%";

  return (
    <div className="rounded-[8px] border border-[#E5E7EB] bg-white p-3 shadow-subtle">
      <p className="font-body text-[13px] font-medium text-[#374151]">{title}</p>
      <div className="mt-1 flex items-center justify-between">
        <div>
          <p className="font-heading text-[28px] font-bold leading-[34px] tracking-tight text-[#111827]">
            {value}
          </p>
          <p className="font-body text-[11px] font-medium text-[#9CA3AF]">{subtitle}</p>
        </div>
        <span className={`inline-flex items-center gap-0.5 rounded-full px-1.5 py-0.5 font-body text-[11px] font-semibold ${TREND_BG[trend]}`}>
          <TrendIcon className="size-3" />
          {trendLabel}
        </span>
      </div>
      <div className="mt-2">
        <SparklineChart data={data} color={color} />
      </div>
    </div>
  );
}
