// Smart Piggy Bank - main state machine
//
//   IDLE --#/digit--> PIN_ENTRY --# correct--> PIN_OK (lock opens) --> OPEN (balances)
//                        |  wrong                                      |  * or timeout
//                        v                                             v
//                     PIN_WRONG --too many--> LOCKOUT --wait--> IDLE  IDLE
//   A coin can be inserted at any time -> COIN_ADDED for a moment, then back.
//   On the idle screen A/B/C/D pick the currency (AED/JOD/BYN/USD) that the
//   inserted coins are counted in; the coin acceptor's pulse count is then
//   looked up in that currency's table (config.h).
//
// Until the keypad and coin acceptor are wired, test from the serial monitor
// (115200 baud, send with newline):
//   1234#            keypad keys: 0-9, * (delete/back), # (enter), A-D, C (clear balances)
//   pulse 2          simulate the coin acceptor sending 2 pulses
//   coin AED 0.50    add a coin directly
//   status           print balances

#include <Arduino.h>
#include "config.h"
#include "bank.h"
#include "ui.h"
#include "pulse_acceptor.h"
#include <Keypad.h>

static char keymap[4][4] = KEYPAD_KEYMAP;
static byte rowPins[4] = KEYPAD_ROW_PINS;
static byte colPins[4] = KEYPAD_COL_PINS;
static Keypad keypad(makeKeymap(keymap), rowPins, colPins, 4, 4);

enum State { S_IDLE, S_PIN_ENTRY, S_PIN_OK, S_PIN_WRONG, S_LOCKOUT, S_OPEN, S_CONFIRM_CLEAR, S_COIN_ADDED, S_UNKNOWN_COIN };

static PulseAcceptor coinAcceptor(COIN_PIN, COIN_ACTIVE_LOW, COIN_MIN_PULSE_MS, COIN_GAP_MS);
static uint8_t unknownPulses = 0;

static State state = S_IDLE;
static State stateBeforeCoin = S_IDLE;
static uint32_t stateSince = 0;     // millis() when the current state started
static uint32_t lastActivity = 0;   // last key press, for timeouts

static String pinBuffer;
static uint8_t attemptsLeft = MAX_PIN_ATTEMPTS;

static Currency lastCoinCur = CUR_AED;
static int32_t lastCoinCents = 0;

static bool hasDisplay = false;

// ---------------------------------------------------------------- helpers

static void setState(State s) {
  state = s;
  stateSince = millis();
  lastActivity = millis();
}

static uint32_t inState() { return millis() - stateSince; }

static void setRelay(bool open) {
#if RELAY_PIN >= 0
  digitalWrite(RELAY_PIN, open ? RELAY_ACTIVE_LEVEL : !RELAY_ACTIVE_LEVEL);
#endif
  Serial.printf("[lock] %s\n", open ? "OPEN" : "closed");
}

// ---------------------------------------------------------------- events

static void onCoin(Currency cur, int32_t cents) {
  Bank::add(cur, cents);
  lastCoinCur = cur;
  lastCoinCents = cents;
  Serial.printf("[coin] +%s %s\n", Bank::format(cents).c_str(), Bank::name(cur));
  // Don't interrupt the open/lockout screens; just record the coin.
  if (state == S_PIN_OK || state == S_LOCKOUT) return;
  if (state != S_COIN_ADDED && state != S_UNKNOWN_COIN) stateBeforeCoin = (state == S_OPEN) ? S_OPEN : S_IDLE;
  setState(S_COIN_ADDED);
}

// A finished pulse burst from the coin acceptor.
static void onPulses(uint8_t pulses) {
  Currency cur = Bank::selected();
  int32_t cents;
  if (Bank::coinFromPulses(cur, pulses, cents)) {
    onCoin(cur, cents);
    return;
  }
  Serial.printf("[coin] unknown: %u pulses (currency %s)\n", pulses, Bank::name(cur));
  if (state == S_PIN_OK || state == S_LOCKOUT) return;
  unknownPulses = pulses;
  if (state != S_COIN_ADDED && state != S_UNKNOWN_COIN) stateBeforeCoin = (state == S_OPEN) ? S_OPEN : S_IDLE;
  setState(S_UNKNOWN_COIN);
}

static void onKey(char key) {
  lastActivity = millis();
  switch (state) {
    case S_IDLE:
      if (toupper(key) >= 'A' && toupper(key) <= 'D') {
        Bank::select((Currency)(toupper(key) - 'A'));
        Serial.printf("[coin] counting coins as %s\n", Bank::name(Bank::selected()));
      } else if (key == '#' || isDigit(key)) {
        pinBuffer = "";
        setState(S_PIN_ENTRY);
        if (isDigit(key)) pinBuffer += key;
      }
      break;

    case S_PIN_ENTRY:
      if (isDigit(key) && pinBuffer.length() < PIN_LENGTH) {
        pinBuffer += key;
      } else if (key == '*') {
        if (pinBuffer.length() == 0) setState(S_IDLE);
        else pinBuffer.remove(pinBuffer.length() - 1);
      } else if (key == '#') {
        if (pinBuffer == PIN_CODE) {
          attemptsLeft = MAX_PIN_ATTEMPTS;
          setRelay(true);
          setState(S_PIN_OK);
        } else {
          attemptsLeft--;
          Serial.printf("[pin] wrong, %u tries left\n", attemptsLeft);
          setState(attemptsLeft == 0 ? S_LOCKOUT : S_PIN_WRONG);
        }
        pinBuffer = "";
      }
      break;

    case S_OPEN:
      if (key == '*') setState(S_IDLE);
      else if (key == 'C' || key == 'c') setState(S_CONFIRM_CLEAR);
      break;

    case S_CONFIRM_CLEAR:
      if (key == '#') {
        Bank::clearAll();
        Serial.println("[bank] balances cleared");
        setState(S_OPEN);
      } else if (key == '*') {
        setState(S_OPEN);
      }
      break;

    default:  // S_PIN_OK, S_PIN_WRONG, S_LOCKOUT, S_COIN_ADDED ignore keys
      break;
  }
}

// ---------------------------------------------------------------- timers

static void updateTimers() {
  switch (state) {
    case S_PIN_ENTRY:
      if (millis() - lastActivity > PIN_ENTRY_TIMEOUT) setState(S_IDLE);
      break;
    case S_PIN_OK:
      if (inState() >= UNLOCK_MS) {
        setRelay(false);
        setState(S_OPEN);
      }
      break;
    case S_PIN_WRONG:
      if (inState() >= WRONG_PIN_MS) {
        pinBuffer = "";
        setState(S_PIN_ENTRY);
      }
      break;
    case S_LOCKOUT:
      if (inState() >= LOCKOUT_MS) {
        attemptsLeft = MAX_PIN_ATTEMPTS;
        setState(S_IDLE);
      }
      break;
    case S_OPEN:
    case S_CONFIRM_CLEAR:
      if (millis() - lastActivity > OPEN_TIMEOUT) setState(S_IDLE);
      break;
    case S_COIN_ADDED:
    case S_UNKNOWN_COIN:
      if (inState() >= COIN_SCREEN_MS) setState(stateBeforeCoin);
      break;
    default:
      break;
  }
}

// ---------------------------------------------------------------- display

static void render() {
  if (!hasDisplay) return;
  switch (state) {
    case S_IDLE:          UI::drawIdle(Bank::name(Bank::selected())); break;
    case S_UNKNOWN_COIN:  UI::drawUnknownCoin(unknownPulses, Bank::name(Bank::selected())); break;
    case S_PIN_ENTRY:     UI::drawEnterPin(pinBuffer.length(), attemptsLeft); break;
    case S_PIN_OK:        UI::drawPinCorrect((float)inState() / UNLOCK_MS); break;
    case S_PIN_WRONG:     UI::drawPinWrong(attemptsLeft); break;
    case S_LOCKOUT:       UI::drawLockedOut((LOCKOUT_MS - inState()) / 1000 + 1); break;
    case S_OPEN:          UI::drawBalances(); break;
    case S_CONFIRM_CLEAR: UI::drawConfirmClear(); break;
    case S_COIN_ADDED:    UI::drawCoinAdded(Bank::name(lastCoinCur), lastCoinCents); break;
  }
}

// ---------------------------------------------------------------- serial test input

static void handleSerialLine(String line) {
  line.trim();
  if (line.length() == 0) return;

  if (line.startsWith("coin")) {             // coin AED 0.50
    char cur[8] = {0};
    float amount = 0;
    if (sscanf(line.c_str(), "coin %7s %f", cur, &amount) == 2) {
      Currency c;
      if (Bank::parseCurrency(cur, c) && amount > 0) {
        onCoin(c, (int32_t)(amount * 100 + 0.5f));
        return;
      }
    }
    Serial.println("usage: coin <AED|JOD|BYN|USD> <amount>");
    return;
  }

  if (line.startsWith("pulse")) {            // pulse 2
    int n = line.substring(5).toInt();
    if (n > 0 && n < 256) onPulses((uint8_t)n);
    else Serial.println("usage: pulse <count>");
    return;
  }

  if (line == "status") {
    for (uint8_t i = 0; i < CUR_COUNT; i++)
      Serial.printf("  %s %s\n", Bank::name((Currency)i), Bank::format(Bank::balance((Currency)i)).c_str());
    Serial.printf("  Total %s AED\n", Bank::format(Bank::totalAedCents()).c_str());
    return;
  }

  for (char k : line) onKey(k);              // treat the rest as keypad presses
}

static void pollSerial() {
  static String buf;
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      handleSerialLine(buf);
      buf = "";
    } else if (buf.length() < 32) {
      buf += c;
    }
  }
}

// ---------------------------------------------------------------- setup / loop

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== Smart ATM boot ===");

#if RELAY_PIN >= 0
  pinMode(RELAY_PIN, OUTPUT);
#endif
  setRelay(false);

  Bank::begin();
  coinAcceptor.begin();

  hasDisplay = UI::begin();
  if (hasDisplay) {
    UI::drawBoot();
    delay(1200);
  } else {
    Serial.println("OLED not found - check wiring (SDA 8, SCL 9, VCC 3V3, GND).");
  }

  setState(S_IDLE);
  Serial.println("Ready. Type keys (e.g. 1234#) or 'coin AED 0.50'.");
}

void loop() {
  pollSerial();
  if (char key = keypad.getKey()) {
    Serial.printf("[key] %c\n", key);
    onKey(key);
  }
  if (uint8_t pulses = coinAcceptor.poll()) onPulses(pulses);
  updateTimers();

  static uint32_t lastDraw = 0;
  if (millis() - lastDraw >= 100) {   // ~10 fps is plenty for this UI
    lastDraw = millis();
    render();
  }
}
