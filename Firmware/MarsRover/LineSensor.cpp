#include "LineSensor.h"

bool LineSensor::spanOk(uint16_t lo, uint16_t hi) {
  return ((int32_t)hi - (int32_t)lo) >= (int32_t)LINE_CAL_MIN_SPAN;
}

void LineSensor::begin() {
  analogReadResolution(12);                       // 0 .. 4095
  // 11 dB Daempfung = voller Eingangsbereich bis ~3,1 V
  analogSetPinAttenuation(PIN_LINE_L, ADC_11db);
  analogSetPinAttenuation(PIN_LINE_R, ADC_11db);
  // GPIO34/35 haben keine internen Pull-Widerstaende -- der externe
  // 10k-Pulldown am Emitter des Fototransistors ist deshalb Pflicht.
  pinMode(PIN_LINE_L, INPUT);
  pinMode(PIN_LINE_R, INPUT);

  loadCal();
  _nextSample = millis();
}

// Median aus 5 Werten: kleines Array, Insertion Sort auf einer Kopie
uint16_t LineSensor::sampleMedian(uint8_t pin, uint16_t* buf, uint8_t& idx) {
  buf[idx] = (uint16_t)analogRead(pin);
  idx = (uint8_t)((idx + 1) % MED);

  uint16_t s[MED];
  memcpy(s, buf, sizeof(s));
  for (uint8_t i = 1; i < MED; i++) {
    uint16_t v = s[i];
    int8_t j = (int8_t)i - 1;
    while (j >= 0 && s[j] > v) { s[j + 1] = s[j]; j--; }
    s[j + 1] = v;
  }
  return s[MED / 2];
}

float LineSensor::toLineness(uint16_t raw, uint16_t lo, uint16_t hi) const {
  if (hi <= lo) return 0.0f;
  float n = (float)((int32_t)raw - (int32_t)lo) / (float)((int32_t)hi - (int32_t)lo);
  n = constrain(n, 0.0f, 1.0f);                   // 0 = dunkel, 1 = hell
  return LINE_IS_DARK ? (1.0f - n) : n;           // -> "wie viel Linie sehe ich"
}

void LineSensor::update() {
  const uint32_t now = millis();
  if ((int32_t)(now - _nextSample) < 0) return;
  _nextSample = now + LINE_SAMPLE_MS;

  _rawL = sampleMedian(PIN_LINE_L, _bufL, _idxL);
  _rawR = sampleMedian(PIN_LINE_R, _bufR, _idxR);

  // Die ersten MED Durchlaeufe ist der Ringpuffer noch nicht gefuellt
  if (!_primed) {
    if (++_warm < MED) return;
    _primed = true;
  }

  if (_calUntil) {
    if (_rawL < _loL) _loL = _rawL;
    if (_rawL > _hiL) _hiL = _rawL;
    if (_rawR < _loR) _loR = _rawR;
    if (_rawR > _hiR) _hiR = _rawR;

    if ((int32_t)(now - _calUntil) >= 0) {
      _calUntil = 0;
      const bool ok = spanOk(_loL, _hiL) && spanOk(_loR, _hiR);
      _calValid = ok;
      if (ok) saveCal();
    }
    return;   // waehrend der Kalibrierung keine Regelwerte liefern
  }

  if (!_calValid) { _lineL = _lineR = 0.0f; _error = 0.0f; return; }

  _lineL = toLineness(_rawL, _loL, _hiL);
  _lineR = toLineness(_rawR, _loR, _hiR);
  _error = _lineR - _lineL;     // > 0  ->  Linie liegt rechts
}

bool LineSensor::lineVisible() const {
  return max(_lineL, _lineR) > LINE_PRESENT_THRESHOLD;
}

void LineSensor::startCalibration(uint32_t ms) {
  _loL = 4095; _hiL = 0;
  _loR = 4095; _hiR = 0;
  _calValid = false;
  _calUntil = millis() + ms;
}

void LineSensor::resetCalibration() {
  _calValid = false;
  _loL = 4095; _hiL = 0; _loR = 4095; _hiR = 0;
  _nvs.begin(NVS_NAMESPACE, false);
  _nvs.remove("loL"); _nvs.remove("hiL");
  _nvs.remove("loR"); _nvs.remove("hiR");
  _nvs.end();
}

void LineSensor::loadCal() {
  _nvs.begin(NVS_NAMESPACE, true);          // read only
  _loL = _nvs.getUShort("loL", 4095);
  _hiL = _nvs.getUShort("hiL", 0);
  _loR = _nvs.getUShort("loR", 4095);
  _hiR = _nvs.getUShort("hiR", 0);
  _nvs.end();
  _calValid = spanOk(_loL, _hiL) && spanOk(_loR, _hiR);
}

void LineSensor::saveCal() {
  _nvs.begin(NVS_NAMESPACE, false);
  _nvs.putUShort("loL", _loL); _nvs.putUShort("hiL", _hiL);
  _nvs.putUShort("loR", _loR); _nvs.putUShort("hiR", _hiR);
  _nvs.end();
}
