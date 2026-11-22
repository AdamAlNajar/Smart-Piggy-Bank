#include "ui.h"
#include "bank.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/Org_01.h>

// Layouts originally designed in Lopaka (AdafruitGFX Mono, SSD1306 128x64).
// Note: with the default font setCursor(x, y) is the TOP of the text;
// with Org_01 it is the BASELINE.

namespace {
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

void startFrame() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);
}

void fontDefault(uint8_t size = 1) {
  display.setFont(nullptr);
  display.setTextSize(size);
}

void fontOrg(uint8_t size = 1) {
  display.setFont(&Org_01);
  display.setTextSize(size);
}

uint16_t textWidth(const String &s) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  return w;
}

void printCentered(const String &s, int16_t y) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(s, 0, y, &x1, &y1, &w, &h);
  display.setCursor((OLED_WIDTH - (int16_t)w) / 2 - x1, y);
  display.print(s);
}

void printRight(const String &s, int16_t xRight, int16_t y) {
  display.setCursor(xRight - textWidth(s), y);
  display.print(s);
}

// 7x9 padlock icon at (x, y); open = shackle lifted to one side
void drawLock(int16_t x, int16_t y, bool open) {
  display.fillRect(x, y + 4, 7, 5, SSD1306_WHITE);          // body
  display.drawPixel(x + 3, y + 6, SSD1306_BLACK);           // keyhole
  int16_t sx = open ? x + 3 : x + 1;                        // shackle
  display.drawFastVLine(sx, y + 1, open ? 2 : 3, SSD1306_WHITE);
  display.drawFastVLine(sx + 4, y + 1, 3, SSD1306_WHITE);
  display.drawFastHLine(sx + 1, y, 3, SSD1306_WHITE);
}

void titleBar(bool unlocked) {
  fontDefault();
  display.setCursor(36, 1);
  display.print("Smart ATM");
  drawLock(118, 0, unlocked);
}
}  // namespace

namespace UI {

bool begin() {
  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setClock(400000);
  // periphBegin=false: Wire is already started on our pins
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C, true, false)) return true;
  return display.begin(SSD1306_SWITCHCAPVCC, 0x3D, true, false);
}

void drawBoot() {
  startFrame();
  fontDefault(2);
  printCentered("Smart ATM", 18);
  fontOrg();
  printCentered("Starting...", 48);
  display.display();
}

void drawIdle(const char *currency) {
  startFrame();
  titleBar(false);
  display.drawFastHLine(0, 11, OLED_WIDTH, SSD1306_WHITE);

  fontOrg();
  printCentered("Insert coins in:", 20);

  // selected currency, white box with black text
  display.fillRoundRect(40, 24, 48, 18, 3, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  fontDefault(2);
  printCentered(currency, 26);
  display.setTextColor(SSD1306_WHITE);

  fontOrg();
  printCentered("A/B/C/D = currency", 52);
  printCentered("# = enter PIN", 61);
  display.display();
}

void drawUnknownCoin(uint8_t pulses, const char *currency) {
  startFrame();
  fontDefault(2);
  printCentered("Unknown", 10);
  printCentered("coin", 28);
  fontOrg();
  printCentered(String(pulses) + " pulses as " + currency, 50);
  printCentered("Not counted - check table", 60);
  display.display();
}

void drawCoinAdded(const char *cur, int32_t cents) {
  startFrame();
  fontDefault(2);
  printCentered("+" + Bank::format(cents) + " " + cur, 16);
  fontOrg();
  printCentered("added to balance", 46);
  display.display();
}

void drawEnterPin(uint8_t digits, uint8_t attemptsLeft) {
  startFrame();
  fontOrg();
  display.setCursor(40, 9);
  display.print("Enter Pin");

  // One box per PIN digit, '*' in the ones already typed
  const int16_t boxW = 14, boxH = 16, gap = 6;
  int16_t x = (OLED_WIDTH - (PIN_LENGTH * boxW + (PIN_LENGTH - 1) * gap)) / 2;
  fontDefault(2);
  for (uint8_t i = 0; i < PIN_LENGTH; i++, x += boxW + gap) {
    display.drawRect(x, 18, boxW, boxH, SSD1306_WHITE);
    if (i < digits) {
      display.setCursor(x + 2, 19);
      display.print('*');
    }
  }

  fontOrg();
  if (attemptsLeft < MAX_PIN_ATTEMPTS)
    printCentered(String(attemptsLeft) + (attemptsLeft == 1 ? " try left" : " tries left"), 46);
  printCentered("# = OK    * = Delete", 61);
  display.display();
}

void drawPinCorrect(float progress) {
  startFrame();
  drawLock(60, 12, true);
  fontOrg();
  display.setCursor(16, 34);
  display.print("Correct Pin.. Opening");
  // progress bar shows how long the lock stays open
  display.drawRect(14, 44, 100, 6, SSD1306_WHITE);
  display.fillRect(14, 44, (int16_t)(100 * constrain(progress, 0.0f, 1.0f)), 6, SSD1306_WHITE);
  display.display();
}

void drawPinWrong(uint8_t attemptsLeft) {
  startFrame();
  fontOrg();
  display.setCursor(38, 34);
  display.print("Wrong Pin...");
  printCentered(String(attemptsLeft) + (attemptsLeft == 1 ? " try left" : " tries left"), 48);
  display.display();
}

void drawLockedOut(uint32_t secondsLeft) {
  startFrame();
  drawLock(60, 6, false);
  fontOrg();
  printCentered("Too many wrong PINs", 30);
  fontDefault(2);
  printCentered(String(secondsLeft) + "s", 40);
  display.display();
}

void drawBalances() {
  startFrame();
  titleBar(true);

  fontDefault();
  for (uint8_t i = 0; i < CUR_COUNT; i++) {
    int16_t y = 14 + i * 9;
    display.setCursor(7, y);
    display.print(Bank::name((Currency)i));
    printRight(Bank::format(Bank::balance((Currency)i)), 121, y);
  }

  display.drawFastHLine(0, 51, OLED_WIDTH, SSD1306_WHITE);
  display.setCursor(4, 55);
  display.print("Total");
  printRight(Bank::format(Bank::totalAedCents()) + " AED", 124, 55);
  display.display();
}

void drawConfirmClear() {
  startFrame();
  fontDefault();
  printCentered("Box emptied?", 10);
  fontOrg();
  printCentered("This resets all balances", 32);
  printCentered("# = Yes    * = No", 52);
  display.display();
}

}  // namespace UI
