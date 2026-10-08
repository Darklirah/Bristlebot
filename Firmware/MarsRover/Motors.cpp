#include "Motors.h"

// LEDC-Kanaele: 0 und 2, damit beide in unterschiedlichen Timern liegen
// (Kanalpaare 0/1, 2/3 ... teilen sich einen Timer).
static const uint8_t CH_L = 0;
static const uint8_t CH_R = 2;

void Motors::begin() {
#if BB_LEDC_V3
  // Arduino-ESP32 Core 3.x: Kanal wird intern vergeben
  ledcAttach(PIN_MOTOR_L, PWM_FREQ_HZ, PWM_BITS);
  ledcAttach(PIN_MOTOR_R, PWM_FREQ_HZ, PWM_BITS);
#else
  // Core 2.x: Kanal explizit konfigurieren und Pin zuweisen
  ledcSetup(CH_L, PWM_FREQ_HZ, PWM_BITS);
  ledcSetup(CH_R, PWM_FREQ_HZ, PWM_BITS);
  ledcAttachPin(PIN_MOTOR_L, CH_L);
  ledcAttachPin(PIN_MOTOR_R, CH_R);
#endif
  stop();
}

void Motors::write(uint8_t pin, uint8_t channel, uint16_t duty) {
#if BB_LEDC_V3
  (void)channel;
  ledcWrite(pin, duty);
#else
  (void)pin;
  ledcWrite(channel, duty);
#endif
}

void Motors::setMinLevel(float v) {
  _minLevel = constrain(v, 0.0f, 0.95f);
}

// Anforderung 0..1 auf den nutzbaren Duty-Bereich abbilden.
// 0 bleibt echte Null (Motor aus), alles darueber startet bei minLevel.
uint16_t Motors::shape(float request) const {
  if (request <= 0.01f) return 0;
  if (request > 1.0f)   request = 1.0f;
  const float level = _minLevel + (1.0f - _minLevel) * request;
  return (uint16_t)(level * MOTOR_DUTY_MAX);
}

void Motors::set(float left, float right) {
  const uint16_t nl = shape(left);
  const uint16_t nr = shape(right);
  const uint32_t now = millis();

  // Kickstart nur beim Uebergang Stillstand -> Lauf
  if (nl > 0 && _targetL == 0) _kickEndL = now + MOTOR_KICK_MS;
  if (nr > 0 && _targetR == 0) _kickEndR = now + MOTOR_KICK_MS;
  if (nl == 0) _kickEndL = 0;
  if (nr == 0) _kickEndR = 0;

  _targetL = nl;
  _targetR = nr;
}

void Motors::stop() {
  _targetL = _targetR = 0;
  _kickEndL = _kickEndR = 0;
  _outL = _outR = 0;
  write(PIN_MOTOR_L, CH_L, 0);
  write(PIN_MOTOR_R, CH_R, 0);
}

void Motors::update() {
  const uint32_t now = millis();

  uint16_t wantL = _targetL;
  uint16_t wantR = _targetR;
  if (_kickEndL && (int32_t)(_kickEndL - now) > 0) wantL = MOTOR_DUTY_MAX;
  if (_kickEndR && (int32_t)(_kickEndR - now) > 0) wantR = MOTOR_DUTY_MAX;

  if (wantL != _outL) { _outL = wantL; write(PIN_MOTOR_L, CH_L, _outL); }
  if (wantR != _outR) { _outR = wantR; write(PIN_MOTOR_R, CH_R, _outR); }
}
