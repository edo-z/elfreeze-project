"use client";

import { Bell, CalendarDays, Search, Menu } from "lucide-react";

interface HeaderProps {
  onMenuToggle?: () => void;
}

export default function Header({ onMenuToggle }: HeaderProps) {
  return (
    <header className="flex w-full items-center justify-between border-b border-[#E5E7EB] bg-white px-4 py-3 md:px-6 md:py-3">
      <div className="flex items-center gap-3">
        <button
          onClick={onMenuToggle}
          className="-ml-1 flex size-8 items-center justify-center rounded-lg text-[#6B7280] hover:bg-[#F3F4F6] md:hidden"
        >
          <Menu className="size-4" />
        </button>
        <nav className="flex items-center gap-1.5 font-body text-[13px] text-[#9CA3AF]">
          <a href="#" className="hover:text-[#374151]">Dashboard</a>
          <span className="text-[#E5E7EB]">/</span>
          <span className="font-medium text-[#374151]">Home</span>
        </nav>
      </div>

      <div className="flex items-center gap-2">
        <div className="hidden items-center gap-1.5 rounded-lg border border-[#E5E7EB] px-2.5 py-1.5 sm:flex">
          <CalendarDays className="size-3.5 text-[#9CA3AF]" />
          <span className="font-body text-[12px] font-medium text-[#374151]">Apr 17, 2023</span>
        </div>

        <button className="flex size-8 items-center justify-center rounded-lg text-[#6B7280] hover:bg-[#F3F4F6]">
          <Search className="size-4" />
        </button>

        <button className="relative flex size-8 items-center justify-center rounded-lg text-[#6B7280] hover:bg-[#F3F4F6]">
          <Bell className="size-4" />
          <span className="absolute right-1.5 top-1.5 size-[7px] rounded-full bg-[#EF4444] ring-1 ring-white" />
        </button>
      </div>
    </header>
  );
}
