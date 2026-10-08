#include "Leds.h"

void Leds::begin() {
  const uint8_t pins[LED_COUNT] = { PIN_LED_FL, PIN_LED_FR, PIN_LED_RL, PIN_LED_RR };
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
  _t0 = millis();
}

void Leds::setSteer(float steer) { _steer = constrain(steer, -1.0f, 1.0f); }
void Leds::setPhase(Phase p)     { _phase = p; }

void Leds::setOverride(bool on) {
  if (_override == on) return;
  _override = on;
  if (!on) return;
  // Beim Einschalten des Overrides alles auf aus -- das Programm setzt,
  // was es haben will, und erbt keinen Zufallszustand.
  for (uint8_t i = 0; i < LED_COUNT; i++) _state[i] = LedState::Off;
}

void Leds::setLed(uint8_t index, LedState st) {
  if (index < LED_COUNT) _state[index] = st;
}

LedState Leds::ledState(uint8_t index) const {
  return index < LED_COUNT ? _state[index] : LedState::Off;
}

// Zielgruppe auf die betroffenen LEDs auflösen
void Leds::setTarget(uint8_t target, LedState st) {
  switch (target) {
    case LT_FL:    _state[LED_FL] = st; break;
    case LT_FR:    _state[LED_FR] = st; break;
    case LT_RL:    _state[LED_RL] = st; break;
    case LT_RR:    _state[LED_RR] = st; break;
    case LT_FRONT: _state[LED_FL] = _state[LED_FR] = st; break;
    case LT_REAR:  _state[LED_RL] = _state[LED_RR] = st; break;
    case LT_ALL:
      for (uint8_t i = 0; i < LED_COUNT; i++) _state[i] = st;
      break;
    default: break;
  }
}

void Leds::setBlinkLevel(uint8_t level) {
  _blinkLevel  = constrain(level, (uint8_t)1, (uint8_t)10);
  // Stufe 1 -> 0,5 Hz (1000 ms halbe Periode), Stufe 10 -> 5 Hz (100 ms)
  _blinkHalfMs = (uint16_t)(1000 / _blinkLevel);
}

void Leds::apply(bool fl, bool fr, bool rl, bool rr) {
  digitalWrite(PIN_LED_FL, fl ? HIGH : LOW);
  digitalWrite(PIN_LED_FR, fr ? HIGH : LOW);
  digitalWrite(PIN_LED_RL, rl ? HIGH : LOW);
  digitalWrite(PIN_LED_RR, rr ? HIGH : LOW);
}

void Leds::allOff() { apply(false, false, false, false); }

bool Leds::render(uint8_t index, bool blinkOn) const {
  switch (_state[index]) {
    case LedState::On:    return true;
    case LedState::Blink: return blinkOn;
    case LedState::Off:
    default:              return false;
  }
}

void Leds::update() {
  const uint32_t t  = millis() - _t0;
  const bool fastOn = ((t / BLINK_FAST_MS) & 1) == 0;
  const bool slowOn = ((t / BLINK_SLOW_MS) & 1) == 0;

  // --- Sicherheitsanzeigen zuerst, sie überstimmen auch das Programm ---
  switch (_phase) {

    case Phase::Disarmed:
      // Rücklichter pulsen langsam = bereit, aber gesperrt
      apply(false, false, slowOn, slowOn);
      return;

    case Phase::Calibrating:
      // alles schnell blinkend = bitte jetzt über die Linie schwenken
      apply(fastOn, fastOn, fastOn, fastOn);
      return;

    case Phase::Lost:
      // Rücklichter an, vorne im Gegentakt = Linie weg, Motoren aus
      apply(fastOn, !fastOn, true, true);
      return;

    case Phase::LowBattery:
      apply(false, false, fastOn, fastOn);
      return;

    default:
      break;
  }

  // --- Programmbetrieb: jede LED einzeln, eigene Blinkfrequenz ---
  if (_override) {
    const bool blinkOn = ((millis() / _blinkHalfMs) & 1) == 0;
    apply(render(LED_FL, blinkOn), render(LED_FR, blinkOn),
          render(LED_RL, blinkOn), render(LED_RR, blinkOn));
    return;
  }

  // --- Normalbetrieb: Rücklichter an, gelb blinkt auf der Lenkseite ---
  const bool left  = (_steer < -STEER_DEADZONE) && fastOn;
  const bool right = (_steer >  STEER_DEADZONE) && fastOn;
  apply(left, right, true, true);
}
