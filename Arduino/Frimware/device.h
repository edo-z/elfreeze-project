#ifndef DEVICE_H
#define DEVICE_H

#include <Preferences.h>
#include <WiFi.h>
#include "../config.h"

Preferences prefs;

void initDevice() {
    prefs.begin(NVS_DEVICE, false);
}

String getDeviceId() {
    String id = prefs.getString("deviceId", "");
    if (id.length() == 0) {
        id = "elfreeze-" + WiFi.macAddress();
        id.replace(":", "");
        prefs.putString("deviceId", id);
    }
    return id;
}

String getApiKey() {
    return prefs.getString("apiKey", "");
}

void setApiKey(const char* key) {
    prefs.putString("apiKey", key);
}

#endif
