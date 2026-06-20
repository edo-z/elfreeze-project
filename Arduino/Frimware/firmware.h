#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <Preferences.h>
#include <Arduino.h>
#include "../config.h"

Preferences fwPrefs;

struct FirmwareVersion {
    int major;
    int minor;
    int patch;
};

void initFirmware() {
    fwPrefs.begin(NVS_FIRMWARE, false);
}

FirmwareVersion getCurrentVersion() {
    FirmwareVersion v;
    v.major = fwPrefs.getInt("major", 1);
    v.minor = fwPrefs.getInt("minor", 0);
    v.patch = fwPrefs.getInt("patch", 0);
    return v;
}

String getVersionString() {
    FirmwareVersion v = getCurrentVersion();
    return String(v.major) + "." + String(v.minor) + "." + String(v.patch);
}

void setVersion(int major, int minor, int patch) {
    fwPrefs.putInt("major", major);
    fwPrefs.putInt("minor", minor);
    fwPrefs.putInt("patch", patch);
}

bool isNewerVersion(const char* newVersion) {
    FirmwareVersion current = getCurrentVersion();
    int newMajor = 0, newMinor = 0, newPatch = 0;
    sscanf(newVersion, "%d.%d.%d", &newMajor, &newMinor, &newPatch);

    if (newMajor > current.major) return true;
    if (newMajor == current.major && newMinor > current.minor) return true;
    if (newMajor == current.major && newMinor == current.minor && newPatch > current.patch) return true;
    return false;
}

void reportDeviceInfo() {
    FirmwareVersion v = getCurrentVersion();
    Serial.printf("[FW] v%d.%d.%d | Uptime: %lu s | Heap: %u\n",
        v.major, v.minor, v.patch, millis() / 1000, ESP.getFreeHeap());
}

#endif
