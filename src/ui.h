#pragma once
#include <Arduino.h>

// Every screen the OLED can show. Each draw* call renders a full frame.
namespace UI {
bool begin();                                       // false if no OLED found

void drawBoot();
void drawIdle(const char *currency);                // locked, waiting for coins / PIN
void drawCoinAdded(const char *cur, int32_t cents); // "+0.50 AED added to balance"
void drawUnknownCoin(uint8_t pulses, const char *currency);
void drawEnterPin(uint8_t digits, uint8_t attemptsLeft);
void drawPinCorrect(float progress);                // 0..1 while the lock is open
void drawPinWrong(uint8_t attemptsLeft);
void drawLockedOut(uint32_t secondsLeft);
void drawBalances();                                // main screen after authentication
void drawConfirmClear();
}  // namespace UI
