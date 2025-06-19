#include <Arduino.h>

#define BOOT_BUTTON_PIN 9

uint32_t currentBPState = 0;
uint32_t lastBPState = 0;
uint32_t bootButtonPressedTime = 0;

bool bootButtonPressed() {
    bootButtonPressedTime = millis();
    return digitalRead(BOOT_BUTTON_PIN) == LOW;
}

void initBootButton() {
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP); 
}

void IRAM_ATTR handleBPInterrupt() {
    currentBPState = !lastBPState;
    bootButtonPressedTime = millis();
    // Print current mode
    Serial.println("[INFO] Boot button pressed!");
}

void initBootButtonAsInterrupt() {
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BOOT_BUTTON_PIN), handleBPInterrupt, FALLING);
}

bool isBootButtonPressed_interrupt() {
    bool ret = currentBPState != lastBPState;
    lastBPState = currentBPState;
    return ret;
}

void clearBootButtonPressedState() {
    lastBPState = 0;
    currentBPState = 0;
}
