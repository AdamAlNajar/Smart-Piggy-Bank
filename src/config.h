#pragma once
// All user-tunable settings in one place.
#include <Arduino.h>

// ---------- OLED (I2C) ----------
#define OLED_SDA      8
#define OLED_SCL      9
#define OLED_WIDTH    128
#define OLED_HEIGHT   64

// ---------- 4x4 keypad ----------
// Keypad pins 1-8 (left to right, keys facing you): rows 1-4, then columns 1-4.
// If presses come out as the wrong key, swap the order in these arrays.
#define KEYPAD_ROW_PINS  { 4, 5, 6, 7 }
#define KEYPAD_COL_PINS  { 15, 16, 17, 18 }
#define KEYPAD_KEYMAP    { {'1','2','3','A'}, \
                           {'4','5','6','B'}, \
                           {'7','8','9','C'}, \
                           {'*','0','#','D'} }

// ---------- Solenoid lock relay ----------
// Set to the GPIO driving the relay module once it's wired; -1 = not connected yet.
#define RELAY_PIN          -1
#define RELAY_ACTIVE_LEVEL HIGH     // most 5V relay modules: HIGH or LOW, check yours

// ---------- PIN / security ----------
#define PIN_CODE           "1234"   // change me
#define PIN_LENGTH         4
#define MAX_PIN_ATTEMPTS   3
#define LOCKOUT_MS         30000    // wait after too many wrong PINs

// ---------- Timings ----------
#define UNLOCK_MS          5000     // how long the solenoid stays open
#define COIN_SCREEN_MS     2000     // "+X added" message
#define WRONG_PIN_MS       1500
#define PIN_ENTRY_TIMEOUT  15000    // back to idle if nobody types
#define OPEN_TIMEOUT       30000    // auto-lock the balance screen

// ---------- Coin acceptor ("616" / CH-926 style, pulse output) ----------
// 4-pin socket: DC12V | COIN | GND | COUNTER
//   COIN -> GPIO10 with a 10k pull-up to 3.3V (NOT 12V)
//   GND  -> ESP32 GND (shared with the 12V supply's GND)
// Acceptor switches: NO/NC switch on "NO", signal-width switch on "medium" (50ms).
#define COIN_PIN             10
#define COIN_ACTIVE_LOW      true   // NO mode: line pulled LOW once per pulse
#define COIN_MIN_PULSE_MS    10     // ignore glitches shorter than this
#define COIN_GAP_MS          300    // silence after the last pulse = coin finished
                                    // (pulse width 20/50/70ms; 300ms covers all three)

// Pulses -> value for each currency. The user picks the currency on the keypad
// (A=AED B=JOD C=BYN D=USD), so the same pulse count can mean different coins
// in different currencies. On the acceptor, set each coin's "P" value to the
// pulse count listed here. It learns up to 6 coin types per A/B switch position.
struct CoinDef { uint8_t pulses; int32_t cents; };

static const CoinDef COINS_AED[] = { {1, 25}, {2, 50}, {4, 100} };            // 0.25, 0.50, 1
static const CoinDef COINS_JOD[] = { {1, 10}, {2, 25}, {3, 50}, {4, 100} };   // 10 qirsh, 1/4, 1/2, 1 JD
static const CoinDef COINS_BYN[] = { {1, 10}, {2, 20}, {3, 50}, {4, 100}, {5, 200} };
static const CoinDef COINS_USD[] = { {1, 5}, {2, 10}, {3, 25}, {4, 100} };

// ---------- Bill acceptor (later) ----------
// Most bill acceptors (e.g. TB74 in pulse mode) work the same way; give it its
// own pin + table and it reuses the pulse decoder.
#define BILL_PIN             -1     // -1 = not connected yet

// ---------- Exchange rates to AED (approximate - update as needed) ----------
#define RATE_AED  1.0000f
#define RATE_JOD  5.1800f
#define RATE_BYN  1.1200f
#define RATE_USD  3.6725f
