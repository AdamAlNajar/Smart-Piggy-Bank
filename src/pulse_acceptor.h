#pragma once
#include <Arduino.h>

// Counts pulses from a pulse-output money acceptor (CH-926 coin acceptor,
// TB74-style bill acceptor, ...). One item = a burst of N pulses followed by
// silence. Counting happens in an interrupt so no pulse is missed while the
// display is being drawn.
class PulseAcceptor {
 public:
  PulseAcceptor(int pin, bool activeLow, uint16_t minPulseMs, uint16_t gapMs)
      : pin_(pin), activeLow_(activeLow), minPulseMs_(minPulseMs), gapMs_(gapMs) {}

  void begin();

  // Returns the pulse count of a finished burst (one coin/bill), else 0.
  uint8_t poll();

 private:
  static void IRAM_ATTR isr(void *arg);

  int pin_;
  bool activeLow_;
  uint16_t minPulseMs_;
  uint16_t gapMs_;

  volatile uint8_t count_ = 0;
  volatile uint32_t lastEdgeMs_ = 0;   // time of the last counted pulse
  volatile uint32_t pulseStartMs_ = 0; // when the current pulse went active
};
