// =====================================================================
//  Bristlebot -- Distance.h
//  VL53L0X auf dem Mast: Abstand nach vorn, in Millimetern.
//
//  Benoetigt die Bibliothek "VL53L0X" von Pololu (Library Manager).
//  Der Chip wird mit einer mehrere hundert Zeilen langen Registersequenz
//  initialisiert -- die kommt aus der Bibliothek, nicht von Hand.
//
//  BESONDERHEITEN
//   * laeuft im Dauermessbetrieb; gelesen wird nur, wenn der Chip ein
//     neues Ergebnis gemeldet hat. Sonst wuerde der Lesebefehl bis zum
//     Ende des Messfensters blockieren und die Hauptschleife ausbremsen.
//   * Median aus drei Werten gegen die typischen Einzelausreisser
//   * ausserhalb von DIST_MIN_MM..DIST_MAX_MM gilt der Wert als
//     ungueltig: unter 40 mm ist der Sensor blind, oberhalb 1,2 m wird
//     die Messung auf dunklen Oberflaechen unzuverlaessig
//   * der Sensor ist optional. Fehlt er, melden valid() und present()
//     dauerhaft false und alles andere laeuft weiter.
// =====================================================================
#pragma once
#include <Arduino.h>
#include "Config.h"

class Distance {
public:
  bool begin();
  void update();                       // in jedem loop() aufrufen

  bool     present() const { return _present; }
  bool     valid()   const { return _valid; }
  uint16_t mm()      const { return _mm; }     // 0 = kein gueltiger Messwert

  // Hindernis naeher als der Schwellwert? Ohne Sensor immer false --
  // ein fehlender Sensor darf den Roboter nicht blockieren.
  bool closerThan(uint16_t limitMm) const {
    return _present && _valid && _mm <= limitMm;
  }

private:
  bool     _present = false;
  bool     _valid   = false;
  uint16_t _mm      = 0;

  uint16_t _buf[DIST_MEDIAN] = {0};
  uint8_t  _idx  = 0;
  uint8_t  _warm = 0;
  uint32_t _nextSample = 0;
};
