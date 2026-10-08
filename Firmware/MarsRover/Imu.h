// =====================================================================
//  MarsRover -- Imu.h
//  MPU-6050 am I2C: Kurswinkel, Kippschutz.
//
//  WAS ER LIEFERT
//    heading()   Kurswinkel in Grad, aus dem Z-Gyro integriert.
//                Relativ, nicht absolut -- 0 ist, wo zuletzt genullt wurde.
//    tipped()    liegt nicht mehr flach
//    lifted()    wird gerade hochgehoben (Schaetzung, siehe unten)
//
//  WAS ER NICHT LIEFERT: Strecken. Die zweifache Integration der
//  Beschleunigung ist auf einem vibrationsgetriebenen Roboter binnen
//  ein bis zwei Sekunden unbrauchbar. "Fahre 30 cm" bleibt unmoeglich.
//
//  VIBRATION
//  Der Sensor sitzt auf einem Geraet, dessen Antriebsprinzip Schuetteln
//  ist. Drei Gegenmassnahmen, alle noetig:
//    1. internes Digitalfilter auf 21 Hz (IMU_DLPF_CFG) -- ohne das
//       faltet sich die Motorvibration ins Nutzsignal
//    2. grosse Messbereiche (+-8 g), damit nichts uebersteuert; ein
//       uebersteuernder MEMS-Beschleunigungssensor erzeugt einen
//       Gleichanteil, der wie eine echte Neigung aussieht
//    3. weiche Montage auf Schaumklebeband, nahe der Drehachse und
//       moeglichst weit weg von den Motoren
//
//  DRIFT
//  Ohne Magnetometer driftet der Kurs um grob 3..6 Grad je Minute, auch
//  nach der Nullpunktaufnahme. Fuer Manoever von Sekunden belanglos, fuer
//  lange Programme nicht. Deshalb nullt jeder Drehbefehl neu.
// =====================================================================
#pragma once
#include <Arduino.h>
#include "Config.h"

class Imu {
public:
  bool begin();                        // false, wenn kein Sensor antwortet
  void update();                       // in jedem loop() aufrufen
  bool present() const { return _present; }

  // --- Kurs ---
  float heading() const { return _heading; }      // Grad, + = nach rechts gedreht
  float rate()    const { return _rate; }         // Grad je Sekunde
  void  zeroHeading() { _heading = 0.0f; }

  // --- Nullpunkt ---
  void startCalibration();             // Roboter dabei ruhig stehen lassen
  bool calibrating() const { return _calUntil != 0; }
  bool calibrated()  const { return _calDone; }
  float bias() const { return _biasZ; }

  // --- Lage ---
  bool tipped() const { return _tipped; }
  bool lifted() const { return _lifted; }
  float accelZ() const { return _azLp; }          // in g, stark gefiltert

private:
  bool write8(uint8_t reg, uint8_t val);
  bool readBurst(uint8_t reg, uint8_t* buf, uint8_t len);
  void loadBias();
  void saveBias();

  bool     _present = false;
  bool     _calDone = false;

  float    _heading = 0.0f;
  float    _rate    = 0.0f;
  float    _biasZ   = 0.0f;            // in LSB

  uint32_t _nextSample = 0;
  uint32_t _lastUs     = 0;

  uint32_t _calUntil = 0;
  double   _calSum   = 0.0;
  uint32_t _calN     = 0;

  float    _azLp  = 1.0f;              // Tiefpass auf die Z-Beschleunigung
  float    _magLp = 1.0f;              // Tiefpass auf den Betrag
  uint32_t _tiltSince = 0;
  uint32_t _liftSince = 0;
  bool     _tipped = false;
  bool     _lifted = false;
};
