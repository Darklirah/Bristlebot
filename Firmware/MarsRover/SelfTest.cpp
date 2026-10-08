#include "SelfTest.h"

// Feste Reihenfolge des Durchlaufs. Explizit als Liste, damit die Pausen
// zwischen den Abschnitten ohne Sonderfaelle dastehen und die Reihenfolge
// an genau einer Stelle aenderbar ist.
static const uint8_t SEQ[] = {
  SelfTest::LedFL, SelfTest::LedFR, SelfTest::LedRL, SelfTest::LedRR,
  SelfTest::AllBlink,
  SelfTest::Gap, SelfTest::MotorLeft,
  SelfTest::Gap, SelfTest::MotorRight,
  SelfTest::Gap, SelfTest::GyroRight,
  SelfTest::Gap, SelfTest::GyroLeft,
  SelfTest::Gap, SelfTest::DistCheck,
  SelfTest::Gap
};
static const uint8_t SEQ_LEN = sizeof(SEQ) / sizeof(SEQ[0]);

// Ab dieser Kursaenderung gilt eine Drehung als erkannt
static const float TEST_GYRO_MIN_DEG = 15.0f;

uint16_t SelfTest::durationOf(uint8_t step) {
  switch (step) {
    case LedFL: case LedFR: case LedRL: case LedRR: return TEST_LED_STEP_MS;
    case AllBlink:                                  return TEST_BLINK_MS;
    case MotorLeft: case MotorRight:                return TEST_RAMP_MS;
    case GyroRight: case GyroLeft:                  return TEST_GYRO_MS;
    case DistCheck:                                 return TEST_DIST_MS;
    case Gap:
    default:                                        return TEST_GAP_MS;
  }
}

void SelfTest::start() {
  _running = true;
  _vGyro   = Untested;
  _vDist   = Untested;
  _dRight  = _dLeft = 0.0f;
  enter(0, 0.0f);
}

void SelfTest::stop() {
  _running = false;
  _mL = _mR = 0.0f;
  _percent = 0;
  _info    = 0;
  for (uint8_t i = 0; i < LED_COUNT; i++) _led[i] = LedState::Off;
}

// Abschnitt betreten: Ausgaenge auf den Anfangszustand setzen.
void SelfTest::enter(uint8_t seqIndex, float heading) {
  _seq     = (uint8_t)(seqIndex % SEQ_LEN);
  _step    = SEQ[_seq];
  _entered = millis();
  _until   = _entered + durationOf(_step);
  _mL = _mR = 0.0f;
  _percent = 0;
  _info    = 0;

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

    // Lagesensor: der Roboter dreht sich selbst, gemessen wird die
    // Kursaenderung. Gleiche LED-Zuordnung wie beim Motortest.
    case GyroRight:
      for (uint8_t i = 0; i < LED_COUNT; i++) _led[i] = LedState::Off;
      _led[LED_FR] = LedState::Blink;
      _hdStart = heading;
      break;
    case GyroLeft:
      for (uint8_t i = 0; i < LED_COUNT; i++) _led[i] = LedState::Off;
      _led[LED_FL] = LedState::Blink;
      _hdStart = heading;
      break;

    case DistCheck:
      for (uint8_t i = 0; i < LED_COUNT; i++) _led[i] = LedState::Off;
      _led[LED_FL] = _led[LED_FR] = LedState::On;   // "ich schaue nach vorn"
      _sawTarget = false;
      break;

    case Gap:
    default:
      for (uint8_t i = 0; i < LED_COUNT; i++) _led[i] = LedState::Off;
      break;
  }
}

// Urteil ueber den Lagesensor, sobald beide Drehabschnitte durch sind.
void SelfTest::judgeGyro(bool imuOk) {
  if (!imuOk) { _vGyro = Missing; return; }

  const bool rightMoved = fabsf(_dRight) >= TEST_GYRO_MIN_DEG;
  const bool leftMoved  = fabsf(_dLeft)  >= TEST_GYRO_MIN_DEG;

  if (!rightMoved && !leftMoved) {
    // Kann auch heissen, dass sich der Roboter mechanisch nicht gedreht
    // hat -- die App formuliert das entsprechend offen.
    _vGyro = NoMotion;
  } else if (_dRight < 0.0f && _dLeft > 0.0f) {
    _vGyro = Reversed;
  } else if (_dRight > 0.0f && _dLeft < 0.0f) {
    _vGyro = Ok;
  } else {
    _vGyro = NoMotion;
  }
}

void SelfTest::update(float heading, bool imuOk, uint16_t distMm, bool distOk) {
  if (!_running) return;
  const uint32_t now = millis();

  switch (_step) {

    // Motorrampe: linear von gerade-eben-an bis Vollgas. Die untere Grenze
    // macht Motors::shape() selbst -- hier reicht der Anforderungswert.
    case MotorLeft:
    case MotorRight: {
      float t = (float)(now - _entered) / (float)TEST_RAMP_MS;
      t = constrain(t, 0.0f, 1.0f);
      const float level = 0.02f + 0.98f * t;
      _percent = (uint8_t)(level * 100.0f + 0.5f);
      if (_step == MotorLeft) { _mL = level; _mR = 0.0f; }
      else                    { _mR = level; _mL = 0.0f; }
      break;
    }

    // Auf der Stelle drehen und zuschauen, was der Kreisel dazu sagt
    case GyroRight:
      _mL = PIVOT_LEVEL; _mR = 0.0f;
      _info = (int16_t)(heading - _hdStart);
      break;
    case GyroLeft:
      _mL = 0.0f; _mR = PIVOT_LEVEL;
      _info = (int16_t)(heading - _hdStart);
      break;

    case DistCheck:
      _mL = _mR = 0.0f;
      _info = (int16_t)distMm;
      if (distOk && distMm > 0) _sawTarget = true;
      break;

    default:
      _mL = _mR = 0.0f;
      break;
  }

  if ((int32_t)(now - _until) < 0) return;

  // Abschnitt verlassen: Ergebnis festhalten
  switch (_step) {
    case GyroRight: _dRight = heading - _hdStart; break;
    case GyroLeft:
      _dLeft = heading - _hdStart;
      judgeGyro(imuOk);
      break;
    case DistCheck:
      _vDist = !distOk ? Missing : (_sawTarget ? Ok : NoTarget);
      break;
    default: break;
  }

  enter((uint8_t)(_seq + 1), heading);   // laeuft zyklisch bis zum Stopp
}
