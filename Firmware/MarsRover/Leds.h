// =====================================================================
//  MarsRover -- Leds.h
//  4 Signal-LEDs: 2x gelb vorne, 2x rot hinten.
//
//  Zwei Betriebsarten:
//   * normal       -- gelb zeigt die Lenkbewegung, rot ist Statusanzeige
//   * Override     -- das Fahrprogramm setzt jede LED einzeln auf
//                     aus / an / blinken, mit einstellbarer Frequenz
//
//  Sicherheitsrelevante Phasen (gesperrt, Kalibrierung, Linie verloren,
//  Akku leer) haben immer Vorrang, auch im Override. Sonst koennte ein
//  Fahrprogramm die Warnanzeige unsichtbar machen.
//
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

  // --- Normalbetrieb ---
  // Lenkanzeige: steer < 0 = nach links, > 0 = nach rechts.
  // Betrag unterhalb der Totzone -> beide gelben LEDs aus.
  void setSteer(float steer);
  void setPhase(Phase p);
  void allOff();

  // --- Programmbetrieb ---
  void setOverride(bool on);
  bool overrideActive() const { return _override; }
  void setLed(uint8_t index, LedState st);      // LED_FL .. LED_RR
  void setTarget(uint8_t target, LedState st);  // LT_FL .. LT_ALL
  void setBlinkLevel(uint8_t level);            // 1 .. 10  ->  0,5 .. 5 Hz
  uint8_t blinkLevel() const { return _blinkLevel; }
  LedState ledState(uint8_t index) const;

private:
  void apply(bool fl, bool fr, bool rl, bool rr);
  bool render(uint8_t index, bool blinkOn) const;

  float    _steer = 0.0f;
  Phase    _phase = Phase::Disarmed;
  uint32_t _t0    = 0;

  bool     _override = false;
  LedState _state[LED_COUNT] = { LedState::Off, LedState::Off,
                                 LedState::Off, LedState::Off };
  uint8_t  _blinkLevel  = BLINK_LEVEL_DEFAULT;
  uint16_t _blinkHalfMs = 1000 / BLINK_LEVEL_DEFAULT;

  static constexpr float    STEER_DEADZONE = 0.12f;
  static constexpr uint16_t BLINK_FAST_MS  = 100;   // 5 Hz, Statusmuster
  static constexpr uint16_t BLINK_SLOW_MS  = 500;   // 1 Hz, Statusmuster
};
