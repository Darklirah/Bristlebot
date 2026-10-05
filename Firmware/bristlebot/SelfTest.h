// =====================================================================
//  Bristlebot -- SelfTest.h
//  Eingebauter Funktionstest für Inbetriebnahme und Fehlersuche.
//
//  Läuft alle Ausgänge und Sensoren der Reihe nach durch und wiederholt
//  sich, bis er gestoppt wird:
//
//    1. LED vorne links an
//    2. LED vorne rechts dazu
//    3. LED hinten links dazu
//    4. LED hinten rechts dazu      -- alle vier leuchten jetzt
//    5. alle vier blinken gemeinsam
//    6. Motor LINKS von langsam auf schnell, dann aus
//       (dabei leuchtet die vordere linke LED als Zuordnungshilfe)
//    7. Motor RECHTS genauso
//    8. Lagesensor: Drehung nach rechts, Kursänderung messen
//    9. Lagesensor: Drehung nach links, Kursänderung messen
//   10. Abstandssensor: Messwert anzeigen
//
//  Die Abschnitte 8 und 9 pruefen den Lagesensor NICHT nur auf Leben,
//  sondern auch auf die Drehrichtung: ein vertauschtes Vorzeichen wuerde
//  im Fahrbetrieb jede Winkeldrehung in die falsche Richtung schicken --
//  ein Fehler, der sonst erst beim ersten Fahrprogramm auffaellt und
//  dann schwer zuzuordnen ist.
//
//  Bewusst KEIN Fahrprogramm: einzelne Motoren, Rampen und Sensorurteile
//  lassen sich mit der Programmsprache nicht ausdruecken, und ein
//  Diagnosewerkzeug soll keinen Speicherplatz belegen und nicht
//  versehentlich loeschbar sein.
//
//  Das Modul rechnet nur aus, was es haben will. Motoren und LEDs setzt
//  die Hauptschleife, und sie reicht Kurs und Abstand herein -- so bleibt
//  es frei von Hardware.
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
    GyroRight = 8, GyroLeft = 9, DistCheck = 10,
    STEP_COUNT = 11
  };

  // Urteil je Sensor, wie es die App im Klartext anzeigt
  enum Verdict : uint8_t {
    Untested = 0,    // noch nicht geprueft
    Ok       = 1,
    NoMotion = 2,    // keine Reaktion -- Sensor oder Antrieb
    Reversed = 3,    // Drehrichtung vertauscht
    Missing  = 4,    // Sensor meldet sich nicht
    NoTarget = 5     // Abstandssensor da, aber nichts im Messbereich
  };

  void start();
  void stop();
  // heading in Grad, distMm 0 = kein gueltiger Messwert
  void update(float heading, bool imuOk, uint16_t distMm, bool distOk);
  bool running() const { return _running; }

  // gewünschte Ausgänge
  float    motorL() const { return _mL; }
  float    motorR() const { return _mR; }
  LedState led(uint8_t i) const { return i < LED_COUNT ? _led[i] : LedState::Off; }
  uint8_t  blinkLevel() const { return TEST_BLINK_LEVEL; }

  // Anzeige in der App
  uint8_t step()    const { return _step; }
  uint8_t percent() const { return _percent; }    // Motorrampe, sonst 0
  int16_t info()    const { return _info; }       // Gradaenderung bzw. mm
  uint8_t gyroVerdict() const { return _vGyro; }
  uint8_t distVerdict() const { return _vDist; }

private:
  void enter(uint8_t seqIndex, float heading);
  static uint16_t durationOf(uint8_t step);
  void judgeGyro(bool imuOk);

  bool     _running = false;
  uint8_t  _seq     = 0;        // Position in der festen Reihenfolge
  uint8_t  _step    = LedFL;
  uint32_t _until   = 0;
  uint32_t _entered = 0;

  float    _mL = 0.0f, _mR = 0.0f;
  uint8_t  _percent = 0;
  int16_t  _info    = 0;
  LedState _led[LED_COUNT] = { LedState::Off, LedState::Off,
                               LedState::Off, LedState::Off };

  float    _hdStart   = 0.0f;
  float    _dRight    = 0.0f;   // Kursaenderung im Rechtsabschnitt
  float    _dLeft     = 0.0f;
  bool     _sawTarget = false;
  uint8_t  _vGyro = Untested;
  uint8_t  _vDist = Untested;
};
