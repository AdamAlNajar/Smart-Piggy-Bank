#include "bank.h"
#include "config.h"
#include <Preferences.h>

namespace {
Preferences prefs;
int32_t balances[CUR_COUNT] = {0};
const char *const NAMES[CUR_COUNT] = {"AED", "JOD", "BYN", "USD"};
const float RATES[CUR_COUNT] = {RATE_AED, RATE_JOD, RATE_BYN, RATE_USD};

Currency selectedCur = CUR_AED;

struct CoinTable { const CoinDef *coins; size_t count; };
#define TABLE(t) { t, sizeof(t) / sizeof(t[0]) }
const CoinTable TABLES[CUR_COUNT] = { TABLE(COINS_AED), TABLE(COINS_JOD), TABLE(COINS_BYN), TABLE(COINS_USD) };

void save(Currency c) { prefs.putInt(NAMES[c], balances[c]); }
}  // namespace

namespace Bank {

void begin() {
  prefs.begin("bank", false);
  for (int i = 0; i < CUR_COUNT; i++) balances[i] = prefs.getInt(NAMES[i], 0);
  uint8_t sel = prefs.getUChar("sel", CUR_AED);
  selectedCur = sel < CUR_COUNT ? (Currency)sel : CUR_AED;
}

Currency selected() { return selectedCur; }

void select(Currency c) {
  if (c == selectedCur) return;
  selectedCur = c;
  prefs.putUChar("sel", c);
}

bool coinFromPulses(Currency c, uint8_t pulses, int32_t &cents) {
  const CoinTable &t = TABLES[c];
  for (size_t i = 0; i < t.count; i++) {
    if (t.coins[i].pulses == pulses) {
      cents = t.coins[i].cents;
      return true;
    }
  }
  return false;
}

void add(Currency c, int32_t cents) {
  balances[c] += cents;
  save(c);
}

void clearAll() {
  for (int i = 0; i < CUR_COUNT; i++) {
    balances[i] = 0;
    save((Currency)i);
  }
}

int32_t balance(Currency c) { return balances[c]; }

int32_t totalAedCents() {
  float total = 0;
  for (int i = 0; i < CUR_COUNT; i++) total += balances[i] * RATES[i];
  return (int32_t)(total + 0.5f);
}

const char *name(Currency c) { return NAMES[c]; }

bool parseCurrency(const char *s, Currency &out) {
  for (int i = 0; i < CUR_COUNT; i++) {
    if (strcasecmp(s, NAMES[i]) == 0) {
      out = (Currency)i;
      return true;
    }
  }
  return false;
}

String format(int32_t cents) {
  char buf[16];
  snprintf(buf, sizeof(buf), "%s%ld.%02ld", cents < 0 ? "-" : "",
           labs(cents) / 100, labs(cents) % 100);
  return String(buf);
}

}  // namespace Bank
