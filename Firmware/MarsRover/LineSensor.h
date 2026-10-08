// =====================================================================
//  MarsRover -- LineSensor.h
//  Zwei TCRT5000-Reflexkoppler links/rechts der Linie, analog an ADC1.
//
//  Ablauf:
//   * Median-5-Filter je Kanal  -> toetet die Spikes, die die Vibrations-
//     motoren mechanisch und elektrisch in den ADC einstreuen
//   * Kalibrierung erfasst Min/Max je Kanal, Werte liegen in NVS
//   * daraus "lineness" 0..1 je Seite und daraus der Regelfehler
//
//  Vorzeichen: error > 0 bedeutet "Linie liegt rechts" -> rechts lenken.
// =====================================================================
#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "Config.h"

class LineSensor {
public:
  void begin();
  void update();                       // in jedem loop() aufrufen

  void startCalibration(uint32_t ms = LINE_CAL_MS);
  bool calibrating() const { return _calUntil != 0; }
  bool calibrated()  const { return _calValid; }
  void resetCalibration();

  float linenessL() const { return _lineL; }
  float linenessR() const { return _lineR; }
  float error()     const { return _error; }
  bool  lineVisible() const;

  uint16_t rawL() const { return _rawL; }
  uint16_t rawR() const { return _rawR; }

private:
  static const uint8_t MED = 5;

  // Signed rechnen: hi < lo ist der unkalibrierte Startzustand und wuerde
  // in uint16-Arithmetik ueberlaufen und faelschlich "kalibriert" melden.
  static bool spanOk(uint16_t lo, uint16_t hi);

  uint16_t sampleMedian(uint8_t pin, uint16_t* buf, uint8_t& idx);
  float    toLineness(uint16_t raw, uint16_t lo, uint16_t hi) const;
  void     loadCal();
  void     saveCal();

  Preferences _nvs;

  uint16_t _bufL[MED] = {0}, _bufR[MED] = {0};
  uint8_t  _idxL = 0, _idxR = 0;
  uint8_t  _warm = 0;
  bool     _primed = false;

  uint16_t _rawL = 0, _rawR = 0;
  float    _lineL = 0.0f, _lineR = 0.0f;
  float    _error = 0.0f;

  uint32_t _nextSample = 0;
  uint32_t _calUntil   = 0;
  bool     _calValid   = false;
  uint16_t _loL = 4095, _hiL = 0, _loR = 4095, _hiR = 0;
};
