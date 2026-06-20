#ifndef NTP_H
#define NTP_H

#include <time.h>
#include "../config.h"

void initNTP() {
    configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
}

bool getJam(char* buffer, size_t size, bool blinkState) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        snprintf(buffer, size, "--:--");
        return false;
    }

    if (blinkState)
        strftime(buffer, size, "%H:%M", &timeinfo);
    else
        strftime(buffer, size, "%H %M", &timeinfo);

    return true;
}

#endif
