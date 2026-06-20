export interface TempReading {
  time: string;
  suhu: number;
}

const MAX_ENTRIES = 288;
const buffer: TempReading[] = [];

export function pushReading(suhu: number): void {
  const now = new Date();
  const time =
    now.getHours().toString().padStart(2, "0") +
    ":" +
    now.getMinutes().toString().padStart(2, "0");

  buffer.push({ time, suhu: Math.round(suhu * 10) / 10 });
  if (buffer.length > MAX_ENTRIES) {
    buffer.shift();
  }
}

export function getReadings(): TempReading[] {
  return buffer;
}
