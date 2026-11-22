#pragma once
#include <Arduino.h>

// Coin balances, stored in the smallest unit (fils / qirsh / kopek / cents)
// so there are no floating-point rounding errors. Persisted in flash (NVS),
// so the balance survives power loss.

enum Currency : uint8_t { CUR_AED, CUR_JOD, CUR_BYN, CUR_USD, CUR_COUNT };

namespace Bank {
void begin();                                  // load balances from flash
void add(Currency c, int32_t cents);           // add a coin and save
void clearAll();                               // after emptying the box
int32_t balance(Currency c);                   // in cents
int32_t totalAedCents();                       // everything converted to AED
const char *name(Currency c);                  // "AED", "JOD", ...
bool parseCurrency(const char *s, Currency &out);
String format(int32_t cents);                  // 1250 -> "12.50"

// Currency the user chose on the keypad for the coins being inserted (saved).
Currency selected();
void select(Currency c);

// Looks up a coin by its pulse count in the given currency's table.
bool coinFromPulses(Currency c, uint8_t pulses, int32_t &cents);
}  // namespace Bank
