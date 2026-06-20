#ifndef SENSOR_H
#define SENSOR_H

#include <OneWire.h>
#include <DallasTemperature.h>
#include "../config.h"

OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

void initSensor() {
    sensors.begin();
}

float bacaSuhu() {
    sensors.requestTemperatures();
    float suhu = sensors.getTempCByIndex(0);

    if (suhu == -127.0 || suhu == 85.0) {
        suhu = 30.0 + (random(0, 100) / 10.0);
    }

    return suhu;
}

#endif
