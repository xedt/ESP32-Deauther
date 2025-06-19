#ifndef BOOT_BUTTON_PRESSED_H
#define BOOT_BUTTON_PRESSED_H

bool bootButtonPressed();
void initBootButton();
void IRAM_ATTR handleBPInterrupt();
bool isBootButtonPressed_interrupt();
void initBootButtonAsInterrupt();


#endif