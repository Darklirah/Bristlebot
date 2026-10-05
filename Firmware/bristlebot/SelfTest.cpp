#include "SelfTest.h"

// Feste Reihenfolge des Durchlaufs. Explizit als Liste, damit die Pausen
// zwischen den Motoren ohne Sonderfaelle dastehen und die Reihenfolge an
// genau einer Stelle aenderbar ist.
static const uint8_t SEQ[] = {
  SelfTest::LedFL, SelfTest::LedFR, SelfTest::LedRL, SelfTest::LedRR,
  SelfTest::AllBlink,
  SelfTest::Gap, SelfTest::MotorLeft,
  SelfTest::Gap, SelfTest::MotorRight,
  SelfTest::Gap
};
static const uint8_t SEQ_LEN = sizeof(SEQ) / sizeof(SEQ[0]);

uint16_t SelfTest::durationOf(uint8_t step) {
  switch (step) {
    case LedFL: case LedFR: case LedRL: case LedRR: return TEST_LED_STEP_MS;
    case AllBlink:                                  return TEST_BLINK_MS;
    case MotorLeft: case MotorRight:                return TEST_RAMP_MS;
    case Gap:
    default:                                        return TEST_GAP_MS;
  }
}

void SelfTest::start() {
  _running = true;
  enter(0);
}

void SelfTest::stop() {
  _running = false;
  _mL = _mR = 0.0f;
  _percent = 0;
  for (uint8_t i = 0; i < LED_COUNT; i++) _led[i] = LedState::Off;
}

// Abschnitt betreten: Ausgaenge auf den Anfangszustand setzen.
void SelfTest::enter(uint8_t seqIndex) {
  _seq     = (uint8_t)(seqIndex % SEQ_LEN);
  _step    = SEQ[_seq];
  _entered = millis();
  _until   = _entered + durationOf(_step);
  _mL = _mR = 0.0f;
  _percent = 0;

  switch (_step) {
    // LEDs nacheinander dazuschalten -- am Ende leuchten alle vier.
    // Wer eine vermisst, weiss sofort, welche.
    case LedFL:
      for (uint8_t i = 0; i < LED_COUNT; i++) _led[i] = LedState::Off;
      _led[LED_FL] = LedState::On;
      break;
    case LedFR: _led[LED_FR] = LedState::On; break;
    case LedRL: _led[LED_RL] = LedState::On; break;
    case LedRR: _led[LED_RR] = LedState::On; break;

    case AllBlink:
      for (uint8_t i = 0; i < LED_COUNT; i++) _led[i] = LedState::Blink;
      break;

    // Beim Motortest leuchtet nur die LED der getesteten Seite. Damit
    // faellt sofort auf, wenn Motor und Seite vertauscht verdrahtet sind.
    case MotorLeft:
      for (uint8_t i = 0; i < LED_COUNT; i++) _led[i] = LedState::Off;
      _led[LED_FL] = LedState::On;
      break;
    case MotorRight:
      for (uint8_t i = 0; i < LED_COUNT; i++) _led[i] = LedState::Off;
      _led[LED_FR] = LedState::On;
      break;

    case Gap:
    default:
      for (uint8_t i = 0; i < LED_COUNT; i++) _led[i] = LedState::Off;
      break;
  }
}

void SelfTest::update() {
  if (!_running) return;
  const uint32_t now = millis();

  // Motorrampe: linear von gerade-eben-an bis Vollgas. Die untere Grenze
  // macht Motors::shape() selbst -- hier reicht der Anforderungswert.
  if (_step == MotorLeft || _step == MotorRight) {
    float t = (float)(now - _entered) / (float)TEST_RAMP_MS;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    const float level = 0.02f + 0.98f * t;
    _percent = (uint8_t)(level * 100.0f + 0.5f);
    if (_step == MotorLeft) { _mL = level; _mR = 0.0f; }
    else                    { _mR = level; _mL = 0.0f; }
  }

  if ((int32_t)(now - _until) < 0) return;
  enter((uint8_t)(_seq + 1));     // laeuft zyklisch bis zum Stopp
}
