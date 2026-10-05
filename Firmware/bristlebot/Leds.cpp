#include "Leds.h"

void Leds::begin() {
  const uint8_t pins[4] = { PIN_LED_FL, PIN_LED_FR, PIN_LED_RL, PIN_LED_RR };
  for (uint8_t p : pins) { pinMode(p, OUTPUT); digitalWrite(p, LOW); }
  _t0 = millis();
}

void Leds::setSteer(float steer) { _steer = constrain(steer, -1.0f, 1.0f); }
void Leds::setPhase(Phase p)     { _phase = p; }

void Leds::apply(bool fl, bool fr, bool rl, bool rr) {
  digitalWrite(PIN_LED_FL, fl ? HIGH : LOW);
  digitalWrite(PIN_LED_FR, fr ? HIGH : LOW);
  digitalWrite(PIN_LED_RL, rl ? HIGH : LOW);
  digitalWrite(PIN_LED_RR, rr ? HIGH : LOW);
}

void Leds::allOff() { apply(false, false, false, false); }

void Leds::update() {
  const uint32_t t    = millis() - _t0;
  const bool fastOn   = ((t / BLINK_FAST_MS) & 1) == 0;
  const bool slowOn   = ((t / BLINK_SLOW_MS) & 1) == 0;

  switch (_phase) {

    case Phase::Disarmed:
      // Ruecklichter pulsen langsam = bereit, aber gesperrt
      apply(false, false, slowOn, slowOn);
      return;

    case Phase::Calibrating:
      // alles schnell blinkend = bitte jetzt ueber die Linie schwenken
      apply(fastOn, fastOn, fastOn, fastOn);
      return;

    case Phase::Lost:
      // Ruecklichter an, vorne im Gegentakt = Linie weg, Motoren aus
      apply(fastOn, !fastOn, true, true);
      return;

    case Phase::LowBattery:
      apply(false, false, fastOn, fastOn);
      return;

    case Phase::Searching:
    case Phase::Running:
    default:
      break;
  }

  // Normalbetrieb: Ruecklichter dauerhaft an, gelb blinkt auf der
  // Seite, in die gelenkt wird.
  const bool left  = (_steer < -STEER_DEADZONE) && fastOn;
  const bool right = (_steer >  STEER_DEADZONE) && fastOn;
  apply(left, right, true, true);
}
