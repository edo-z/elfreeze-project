#ifndef ALARM_H
#define ALARM_H

#include <Arduino.h>
#include "../config.h"

void initAlarm() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
}

void cekAlarm(float suhu) {
    digitalWrite(BUZZER_PIN, suhu >= batasSuhu ? HIGH : LOW);
}

#endif
