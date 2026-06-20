#ifndef DISPLAY_H
#define DISPLAY_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "../config.h"

Adafruit_SH1106G display(128, 64, &Wire, -1);

int scrollX = 128;

bool oledReady = false;

void initDisplay() {
    Wire.begin(SDA_PIN, SCL_PIN);
    oledReady = display.begin(OLED_ADDR, true);
    if (!oledReady) {
        Serial.println("OLED tidak ditemukan!");
    }
}

void splashScreen() {
    if (!oledReady) return;
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(2);
    display.setCursor(10, 15);
    display.println("MONITOR");
    display.setCursor(25, 40);
    display.println("SUHU");
    display.display();
    delay(2000);
}

void drawHeader(const char* jam) {
    display.drawLine(0, 10, 127, 10, SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(2, 1);
    display.print("TEMP MONITOR");
    display.setCursor(92, 1);
    display.print(jam);
}

void drawSuhu(float suhu) {
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%.1fC", suhu);

    int16_t x1, y1;
    uint16_t w, h;
    display.setTextSize(3);
    display.getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((128 - w) / 2, 14);
    display.print(buffer);
}

void drawStatus(float suhu, bool blinkState) {
    display.setTextSize(1);

    if (suhu >= batasSuhu) {
        if (blinkState) {
            display.fillCircle(118, 35, 4, SH110X_WHITE);
        }
        display.setCursor(38, 42);
        display.print("WARNING");
    } else {
        display.setCursor(42, 42);
        display.print("NORMAL");
    }
}

void drawRunningText() {
    display.drawLine(0, 48, 127, 48, SH110X_WHITE);
    display.setTextSize(2);
    display.setCursor(scrollX, 50);
    display.print(companyText);

    scrollX -= 5;
    int textWidth = strlen(companyText) * 12;
    if (scrollX < -textWidth) {
        scrollX = 128;
    }
}

void renderDisplay(float suhu, bool blinkState, const char* jam) {
    if (!oledReady) return;
    display.clearDisplay();
    drawHeader(jam);
    drawSuhu(suhu);
    drawStatus(suhu, blinkState);
    drawRunningText();
    display.display();
}

#endif
