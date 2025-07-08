#include <Arduino.h>

#define BOOT_BUTTON_PIN 9

static uint8_t currentBPState = 0;
static uint8_t lastBPState = 0;
static volatile uint32_t bootButtonPressedTime = 0;

bool bootButtonPressed() {
    return digitalRead(BOOT_BUTTON_PIN) == LOW;
}

void initBootButton() {
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP); 
}

void IRAM_ATTR handleBPInterrupt() {
    if (millis() - bootButtonPressedTime < BUTTON_DEBOUNCE_MILIS) return;
    currentBPState = !lastBPState;
    bootButtonPressedTime = millis();
    // Print current mode
    Serial.println("[INFO] Boot button pressed!");
}

inline void clearBottButtonPressedState() {
    lastBPState = currentBPState;
    bootButtonPressedTime = 0;
}   

void initBootButtonAsInterrupt() {
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BOOT_BUTTON_PIN), handleBPInterrupt, FALLING);
}

bool isBootButtonPressed_interrupt() {
    if ((millis() - bootButtonPressedTime) > 3000) {
        clearBottButtonPressedState();
        return false;
    }
    bool ret = currentBPState != lastBPState;
    clearBottButtonPressedState();
    return ret;
}
