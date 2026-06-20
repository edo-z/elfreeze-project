#ifndef ALARM_H
#define ALARM_H

#include <Arduino.h>
#include "../config.h"

void initAlarm() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
}

void cekAlarm(float suhu) {
    if (suhu >= configCritical) {
        digitalWrite(BUZZER_PIN, (millis() % 200 < 100) ? HIGH : LOW);
    } else if (suhu >= configWarning) {
        digitalWrite(BUZZER_PIN, (millis() % 1000 < 500) ? HIGH : LOW);
    } else {
        digitalWrite(BUZZER_PIN, LOW);
    }
}

#endif
