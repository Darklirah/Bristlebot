// =====================================================================
//  Bristlebot -- Leds.h
//  4 Signal-LEDs: 2x gelb vorne (Lenkanzeige), 2x rot hinten (Status).
//  Alle Blinkmuster laufen zeitgesteuert in update(), nie blockierend.
// =====================================================================
#pragma once
#include <Arduino.h>
#include "Config.h"
#include "RobotState.h"

class Leds {
public:
  void begin();
  void update();                      // in jedem loop() aufrufen

  // Lenkanzeige: steer < 0 = nach links, > 0 = nach rechts.
  // Betrag unterhalb der Totzone -> beide gelben LEDs aus.
  void setSteer(float steer);
  void setPhase(Phase p);
  void allOff();

private:
  void apply(bool fl, bool fr, bool rl, bool rr);

  float    _steer = 0.0f;
  Phase    _phase = Phase::Disarmed;
  uint32_t _t0    = 0;

  static constexpr float    STEER_DEADZONE = 0.12f;
  static constexpr uint16_t BLINK_FAST_MS  = 100;   // 5 Hz
  static constexpr uint16_t BLINK_SLOW_MS  = 500;   // 1 Hz
};
