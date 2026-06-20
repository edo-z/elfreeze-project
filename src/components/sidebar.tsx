"use client";

import { Activity, Thermometer, Gauge, Bell, Settings, ChevronDown } from "lucide-react";
import { cn } from "@/lib/utils";

const NAV_MAIN = [
  { label: "Dashboard", icon: Activity, href: "#", active: true },
  { label: "Temperature", icon: Thermometer, href: "#", active: false },
  { label: "Sensors", icon: Gauge, href: "#", active: false },
  { label: "Alerts", icon: Bell, href: "#", active: false },
  { label: "Settings", icon: Settings, href: "#", active: false },
];

export default function Sidebar() {
  return (
    <aside className="hidden md:flex md:w-[240px] md:flex-col md:shrink-0">
      <div className="flex h-full flex-col border-r border-[#E5E7EB] bg-white">
        <div className="flex items-center gap-2.5 border-b border-[#E5E7EB] px-4 py-3.5">
          <div className="flex size-8 items-center justify-center rounded-lg bg-[#6366F1]">
            <Activity className="size-4 text-white" />
          </div>
          <div>
            <span className="font-heading text-sm font-semibold text-[#111827]">Elfreeze</span>
            <span className="block font-body text-[10px] font-medium text-[#9CA3AF]">Cryo Monitor</span>
          </div>
        </div>

        <nav className="flex-1 space-y-0.5 px-2 py-3">
          <p className="px-2 pb-1 font-body text-[10px] font-semibold uppercase tracking-[0.06em] text-[#9CA3AF]">
            Menu
          </p>
          {NAV_MAIN.map((item) => (
            <a
              key={item.label}
              href={item.href}
              className={cn(
                "flex items-center gap-2.5 rounded-lg px-2 py-2 font-body text-[13px] font-medium transition-colors duration-80",
                item.active
                  ? "bg-[#EEF2FF] text-[#6366F1]"
                  : "text-[#6B7280] hover:bg-[#F3F4F6] hover:text-[#374151]"
              )}
            >
              <item.icon className="size-4 shrink-0" />
              {item.label}
            </a>
          ))}
        </nav>

        <div className="border-t border-[#E5E7EB] p-3">
          <div className="flex items-center gap-2.5 rounded-lg px-2 py-2 text-[#6B7280] hover:bg-[#F3F4F6]">
            <div className="flex size-7 items-center justify-center rounded-full bg-[#6366F1] text-[10px] font-semibold text-white">
              AZ
            </div>
            <div className="flex-1">
              <p className="font-body text-[12px] font-medium text-[#111827]">Aldo Zeno</p>
              <p className="font-body text-[10px] text-[#9CA3AF]">aldo@elfreeze.io</p>
            </div>
            <ChevronDown className="size-3.5 text-[#9CA3AF]" />
          </div>
        </div>
      </div>
    </aside>
  );
}
