// =====================================================================
//  Bristlebot -- SelfTest.h
//  Eingebauter Funktionstest für Inbetriebnahme und Fehlersuche.
//
//  Läuft alle Ausgänge der Reihe nach durch und wiederholt sich, bis er
//  gestoppt wird:
//
//    1. LED vorne links an
//    2. LED vorne rechts dazu
//    3. LED hinten links dazu
//    4. LED hinten rechts dazu      -- alle vier leuchten jetzt
//    5. alle vier blinken gemeinsam
//    6. Motor LINKS von langsam auf schnell, dann aus
//       (dabei leuchtet die vordere linke LED als Zuordnungshilfe)
//    7. Motor RECHTS genauso
//    8. kurze Pause, dann von vorn
//
//  Bewusst KEIN Fahrprogramm: einzelne Motoren und eine steigende Rampe
//  lassen sich mit geradeaus/links/rechts nicht ausdrücken, und ein
//  Diagnosewerkzeug soll keinen Speicherplatz belegen und nicht
//  versehentlich löschbar sein.
//
//  Das Modul rechnet nur aus, was es haben will. Motoren und LEDs setzt
//  die Hauptschleife -- so bleibt es frei von Hardware.
// =====================================================================
#pragma once
#include <Arduino.h>
#include "Config.h"
#include "RobotState.h"

class SelfTest {
public:
  enum Step : uint8_t {
    LedFL = 0, LedFR = 1, LedRL = 2, LedRR = 3,
    AllBlink = 4, MotorLeft = 5, MotorRight = 6, Gap = 7,
    STEP_COUNT = 8
  };

  void start();
  void stop();
  void update();
  bool running() const { return _running; }

  // gewünschte Ausgänge
  float    motorL() const { return _mL; }
  float    motorR() const { return _mR; }
  LedState led(uint8_t i) const { return i < LED_COUNT ? _led[i] : LedState::Off; }
  uint8_t  blinkLevel() const { return TEST_BLINK_LEVEL; }

  // Anzeige in der App
  uint8_t step()    const { return _step; }
  uint8_t percent() const { return _percent; }   // Motorrampe, sonst 0

private:
  void enter(uint8_t seqIndex);
  static uint16_t durationOf(uint8_t step);

  bool     _running = false;
  uint8_t  _seq     = 0;        // Position in der festen Reihenfolge
  uint8_t  _step    = LedFL;
  uint32_t _until   = 0;
  uint32_t _entered = 0;

  float    _mL = 0.0f, _mR = 0.0f;
  uint8_t  _percent = 0;
  LedState _led[LED_COUNT] = { LedState::Off, LedState::Off,
                               LedState::Off, LedState::Off };
};
