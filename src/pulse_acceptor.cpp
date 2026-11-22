#include "pulse_acceptor.h"

void PulseAcceptor::begin() {
  if (pin_ < 0) return;
  pinMode(pin_, activeLow_ ? INPUT_PULLUP : INPUT_PULLDOWN);
  attachInterruptArg(digitalPinToInterrupt(pin_), isr, this, CHANGE);
}

// A pulse is counted when it ENDS, and only if it lasted >= minPulseMs_,
// so short electrical noise on the wire is ignored.
void IRAM_ATTR PulseAcceptor::isr(void *arg) {
  auto *self = static_cast<PulseAcceptor *>(arg);
  bool active = digitalRead(self->pin_) == (self->activeLow_ ? LOW : HIGH);
  uint32_t now = millis();
  if (active) {
    self->pulseStartMs_ = now;
  } else if (self->pulseStartMs_ && now - self->pulseStartMs_ >= self->minPulseMs_) {
    self->count_++;
    self->lastEdgeMs_ = now;
    self->pulseStartMs_ = 0;
  }
}

uint8_t PulseAcceptor::poll() {
  if (pin_ < 0 || count_ == 0) return 0;
  uint8_t n = 0;
  noInterrupts();
  if (millis() - lastEdgeMs_ >= gapMs_) {  // burst finished
    n = count_;
    count_ = 0;
  }
  interrupts();
  return n;
}
