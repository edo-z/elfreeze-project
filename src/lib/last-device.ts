let lastDeviceId = "";

export function setLastDeviceId(id: string) {
  lastDeviceId = id;
}

export function getLastDeviceId() {
  return lastDeviceId;
}
