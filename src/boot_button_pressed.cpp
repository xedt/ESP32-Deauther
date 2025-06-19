#include <Arduino.h>

#define BOOT_BUTTON_PIN 9

uint32_t currentBPState = 0;
uint32_t lastBPState = 0;
bool bootButtonPressed() {
    return digitalRead(BOOT_BUTTON_PIN) == LOW;
}

void initBootButton() {
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP); 
}

void IRAM_ATTR handleBPInterrupt() {
    currentBPState = !lastBPState;
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