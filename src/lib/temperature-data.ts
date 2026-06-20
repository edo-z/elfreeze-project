export interface TempReading {
  time: string;
  suhu: number;
}

export async function fetchReadings(): Promise<TempReading[]> {
  try {
    const res = await fetch("/api/sensor/readings");
    if (!res.ok) return [];
    return await res.json();
  } catch {
    return [];
  }
}

export function downsampleReadings(data: TempReading[], targetCount = 60): TempReading[] {
  if (data.length <= targetCount) return data;

  const bucketSize = data.length / targetCount;
  const result: TempReading[] = [];

  for (let i = 0; i < targetCount; i++) {
    const start = Math.floor(i * bucketSize);
    const end = Math.floor((i + 1) * bucketSize);
    const bucket = data.slice(start, end);
    const mid = bucket[Math.floor(bucket.length / 2)];
    const avg = bucket.reduce((s, r) => s + r.suhu, 0) / bucket.length;
    result.push({ time: mid.time, suhu: Math.round(avg * 10) / 10 });
  }

  return result;
}

function clamp(v: number, min: number, max: number) {
  return Math.min(max, Math.max(min, v));
}

export function generateMockReadings(count: number = 24): TempReading[] {
  const now = new Date();
  now.setMinutes(0, 0, 0);

  return Array.from({ length: count }, (_, i) => {
    const t = new Date(now.getTime() - (count - 1 - i) * 60 * 60 * 1000);
    const hourLabel = t.getHours().toString().padStart(2, "0") + ":00";
    const jitter = (Math.random() - 0.5) * 0.3;

    return {
      time: hourLabel,
      suhu: clamp(34 + jitter, 30, 40),
    };
  });
}
