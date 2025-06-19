#ifndef BOOT_BUTTON_PRESSED_H
#define BOOT_BUTTON_PRESSED_H

uint32_t bootButtonPressedTime = 0;
bool bootButtonPressed();
void initBootButton();
void IRAM_ATTR handleBPInterrupt();
bool isBootButtonPressed_interrupt();
void initBootButtonAsInterrupt();
void clearBootButtonPressedState();
bool isBootButtonStateChanged();

#endif
