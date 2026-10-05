// =====================================================================
//  Bristlebot -- Motors.h
//  Zwei Vibrationsmotoren an einer NPN-Low-Side-Stufe, per LEDC-PWM.
//
//  Besonderheiten eines ERM-Vibrationsmotors:
//   * unterhalb einer Mindestspannung dreht er wegen Haftreibung nicht
//     -> Kennlinie wird auf [minLevel .. 1.0] gestreckt
//   * aus dem Stand braucht er einen kurzen Volldampf-Impuls (Kickstart)
//   * der Duty wird zusaetzlich auf MOTOR_DUTY_MAX begrenzt, damit ein
//     3V-Motor an der 5V-Schiene nicht ueberfahren wird
// =====================================================================
#pragma once
#include <Arduino.h>
#include "Config.h"

class Motors {
public:
  void begin();
  void update();                        // in jedem loop() aufrufen (Kickstart-Timing)
  void set(float left, float right);    // 0.0 .. 1.0 je Seite
  void stop();

  void  setMinLevel(float v);
  float minLevel() const { return _minLevel; }

  uint16_t dutyL() const { return _outL; }
  uint16_t dutyR() const { return _outR; }
  static uint16_t dutyMax() { return MOTOR_DUTY_MAX; }

private:
  uint16_t shape(float request) const;
  void     write(uint8_t pin, uint8_t channel, uint16_t duty);

  float    _minLevel   = MOTOR_MIN_LEVEL_DEFAULT;
  uint16_t _targetL    = 0, _targetR = 0;   // gewuenschter Duty
  uint16_t _outL       = 0, _outR    = 0;   // tatsaechlich ausgegeben
  uint32_t _kickEndL   = 0, _kickEndR = 0;
};
