"use client";

import { useState, useEffect, useRef } from "react";
import { X, Loader2, Check, AlertCircle } from "lucide-react";
import Sidebar from "@/components/sidebar";
import Header from "@/components/header";
import StatCard from "@/components/stat-card";
import ChartOverview from "@/components/chart-overview";
import ChartComparison from "@/components/chart-comparison";
import ReadingsTable from "@/components/readings-table";
import { fetchReadings, generateMockReadings, type TempReading } from "@/lib/temperature-data";

function computeTrend(a: number, b: number): "up" | "down" | "neutral" {
  if (a > b) return "up";
  if (a < b) return "down";
  return "neutral";
}

export default function Home() {
  const [mobileMenuOpen, setMobileMenuOpen] = useState(false);
  const [readings, setReadings] = useState<TempReading[]>([]);
  const [thresholdWarning, setThresholdWarning] = useState(35);
  const [thresholdCritical, setThresholdCritical] = useState(38);
  const [deviceId, setDeviceId] = useState<string | null>(null);
  const [saveStatus, setSaveStatus] = useState<"idle" | "saving" | "saved" | "error">("idle");
  const [lastSaved, setLastSaved] = useState<{ w: number; c: number } | null>(null);

  const warningRef = useRef(thresholdWarning);
  const criticalRef = useRef(thresholdCritical);
  warningRef.current = thresholdWarning;
  criticalRef.current = thresholdCritical;

  async function handleSave() {
    if (!deviceId) {
      setSaveStatus("error");
      setTimeout(() => setSaveStatus("idle"), 2000);
      return;
    }
    setSaveStatus("saving");
    try {
      const res = await fetch("/api/device/config", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ deviceId, warning: warningRef.current, critical: criticalRef.current }),
      });
      if (!res.ok) throw new Error("HTTP " + res.status);
      setSaveStatus("saved");
      setLastSaved({ w: warningRef.current, c: criticalRef.current });
      setTimeout(() => setSaveStatus("idle"), 2000);
    } catch {
      setSaveStatus("error");
      setTimeout(() => setSaveStatus("idle"), 2000);
    }
  }

  useEffect(() => {
    async function load() {
      const [readingsRes, deviceRes] = await Promise.all([
        fetchReadings(),
        fetch("/api/device/last").then(r => r.json()).catch(() => ({ deviceId: null })),
      ]);
      if (readingsRes.length > 0) {
        setReadings(readingsRes);
      } else {
        setReadings(generateMockReadings(24));
      }
      if (deviceRes.deviceId) setDeviceId(deviceRes.deviceId);
    }
    load();
    const interval = setInterval(load, 10000);
    return () => clearInterval(interval);
  }, []);

  const latest = readings[readings.length - 1];
  const prev = readings.length > 1 ? readings[readings.length - 2] : undefined;
  const avg = readings.length > 0
    ? (readings.reduce((s, r) => s + r.suhu, 0) / readings.length).toFixed(1)
    : "---";
  const maxVal = readings.length > 0
    ? Math.max(...readings.map(r => r.suhu)).toFixed(1)
    : "---";
  const minVal = readings.length > 0
    ? Math.min(...readings.map(r => r.suhu)).toFixed(1)
    : "---";

  const tempCards = [
    {
      title: "Current Temp",
      value: latest ? `${latest.suhu.toFixed(1)}°C` : "---",
      subtitle: latest ? `Updated ${latest.time}` : "No data yet",
      trend: latest && prev ? computeTrend(latest.suhu, prev.suhu) : "neutral" as const,
      color: "#EF4444",
      data: readings.slice(-30).map(r => r.suhu),
    },
    {
      title: "Average",
      value: `${avg}°C`,
      subtitle: `From ${readings.length} readings`,
      trend: "neutral" as const,
      color: "#6366F1",
      data: [],
    },
    {
      title: "Maximum",
      value: `${maxVal}°C`,
      subtitle: "Peak temperature",
      trend: "neutral" as const,
      color: "#F59E0B",
      data: [],
    },
    {
      title: "Minimum",
      value: `${minVal}°C`,
      subtitle: "Lowest temperature",
      trend: "neutral" as const,
      color: "#10B981",
      data: [],
    },
  ];

  return (
    <div className="flex min-h-screen bg-[#F3F4F6]">
      <Sidebar />

      {/* Mobile drawer overlay */}
      {mobileMenuOpen && (
        <div className="fixed inset-0 z-40 md:hidden">
          <div className="absolute inset-0 bg-black/30" onClick={() => setMobileMenuOpen(false)} />
          <div className="relative z-50 w-[260px] animate-slide-up">
            <div className="flex h-full min-h-screen">
              <Sidebar />
            </div>
          </div>
        </div>
      )}

      <div className="flex flex-1 flex-col">
        <Header onMenuToggle={() => setMobileMenuOpen(!mobileMenuOpen)} />

        <main className="flex-1 overflow-auto">
          <div className="mx-auto w-full max-w-[1700px] px-4 py-5 md:px-6 md:py-6">
            {/* Overview section */}
            <h2 className="mb-3 font-heading text-[17px] font-semibold leading-[24px] text-[#111827]">
              Overview
            </h2>
            <div className="grid grid-cols-1 gap-3 sm:grid-cols-2 lg:grid-cols-4">
              {tempCards.map((card, i) => (
                <StatCard key={card.title} {...card} />
              ))}
              <div className="rounded-[8px] border border-[#E5E7EB] bg-white p-3 shadow-subtle sm:col-span-2 lg:col-span-4">
                <p className="font-body text-[11px] font-medium text-[#6366F1]">Explore your data</p>
                <p className="mt-0.5 font-body text-[12px] text-[#9CA3AF]">
                  Uncover performance and visitor insights with our data wizardry.
                </p>
                <button className="mt-2 rounded-lg bg-[#6366F1] px-3 py-1.5 font-body text-[12px] font-semibold text-white transition-colors hover:bg-[#4F46E5]">
                  Get insights
                </button>
              </div>
            </div>

            {/* Threshold controls */}
            <div className="mt-4 flex flex-wrap items-center gap-4">
              <div className="flex items-center gap-2">
                <label className="font-body text-[11px] font-medium text-[#F59E0B]">Warning</label>
                <input
                  type="number"
                  value={thresholdWarning}
                  onChange={(e) => setThresholdWarning(Number(e.target.value))}
                  className="w-16 rounded border border-[#E5E7EB] px-2 py-1 text-[12px] font-mono text-[#374151] outline-none focus:border-[#F59E0B]"
                />
                <span className="font-body text-[10px] text-[#9CA3AF]">°C</span>
              </div>
              <div className="flex items-center gap-2">
                <label className="font-body text-[11px] font-medium text-[#EF4444]">Critical</label>
                <input
                  type="number"
                  value={thresholdCritical}
                  onChange={(e) => setThresholdCritical(Number(e.target.value))}
                  className="w-16 rounded border border-[#E5E7EB] px-2 py-1 text-[12px] font-mono text-[#374151] outline-none focus:border-[#EF4444]"
                />
                <span className="font-body text-[10px] text-[#9CA3AF]">°C</span>
              </div>

              <button
                onClick={handleSave}
                disabled={saveStatus === "saving"}
                className={`flex items-center gap-1.5 rounded-lg px-3 py-1.5 font-body text-[12px] font-semibold transition-colors ${
                  saveStatus === "saved"
                    ? "bg-[#10B981] text-white"
                    : saveStatus === "error"
                      ? "bg-[#EF4444] text-white"
                      : "bg-[#6366F1] text-white hover:bg-[#4F46E5]"
                } disabled:opacity-60`}
              >
                {saveStatus === "saving" && <Loader2 className="h-3.5 w-3.5 animate-spin" />}
                {saveStatus === "saved" && <Check className="h-3.5 w-3.5" />}
                {saveStatus === "error" && <AlertCircle className="h-3.5 w-3.5" />}
                {saveStatus === "idle" && "Save"}
                {saveStatus === "saving" && "Saving..."}
                {saveStatus === "saved" && "Applied"}
                {saveStatus === "error" && "Failed"}
              </button>

              {lastSaved && (
                <span className="font-body text-[10px] text-[#9CA3AF]">
                  Last applied: W:{lastSaved.w}°C C:{lastSaved.c}°C
                </span>
              )}
            </div>

            {/* Charts row */}
            <div className="mt-4 grid grid-cols-1 gap-3 lg:grid-cols-2">
              <div className="animate-slide-up" style={{ animationDelay: "100ms" }}>
                <ChartOverview data={readings} thresholdWarning={thresholdWarning} thresholdCritical={thresholdCritical} />
              </div>
              <div className="animate-slide-up" style={{ animationDelay: "200ms" }}>
                <ChartComparison data={readings} thresholdWarning={thresholdWarning} thresholdCritical={thresholdCritical} />
              </div>
            </div>

            {/* Details section */}
            <h2 className="mb-3 mt-6 font-heading text-[17px] font-semibold leading-[24px] text-[#111827]">
              Details
            </h2>
            <div className="grid grid-cols-1 gap-3">
              <div className="animate-slide-up" style={{ animationDelay: "300ms" }}>
                <ReadingsTable readings={readings} />
              </div>
            </div>

            {/* Footer */}
            <p className="mt-8 text-center font-body text-[11px] text-[#9CA3AF]">
              Copyright © Elfreeze 2026.
            </p>
          </div>
        </main>
      </div>
    </div>
  );
}
