#include "Distance.h"
#include <Wire.h>
#include <VL53L0X.h>

static VL53L0X s_sensor;

// Registeradresse aus dem Datenblatt: steht ein neues Messergebnis bereit?
static const uint8_t REG_RESULT_INTERRUPT_STATUS = 0x13;

bool Distance::begin() {
  // Wire.begin() hat bereits der Lagesensor erledigt. Fehlt dieser,
  // holen wir es hier nach -- der Bus wird sonst nie initialisiert.
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_FREQ_HZ);

  s_sensor.setTimeout(250);
  if (!s_sensor.init()) { _present = false; return false; }

  s_sensor.setMeasurementTimingBudget(DIST_TIMING_BUDGET_US);
  s_sensor.startContinuous();

  _present = true;
  _nextSample = millis();
  return true;
}

void Distance::update() {
  if (!_present) return;

  const uint32_t now = millis();
  if ((int32_t)(now - _nextSample) < 0) return;
  _nextSample = now + DIST_SAMPLE_MS;

  // Erst fragen, ob ueberhaupt etwas fertig ist. Ohne diese Pruefung
  // wartet readRangeContinuousMillimeters() bis zum Ende des
  // Messfensters -- das waeren bis zu 33 ms Stillstand in loop().
  if ((s_sensor.readReg(REG_RESULT_INTERRUPT_STATUS) & 0x07) == 0) return;

  const uint16_t raw = s_sensor.readRangeContinuousMillimeters();
  if (s_sensor.timeoutOccurred()) { _valid = false; return; }

  _buf[_idx] = raw;
  _idx = (uint8_t)((_idx + 1) % DIST_MEDIAN);
  if (_warm < DIST_MEDIAN) { _warm++; return; }

  // Median aus drei Werten
  uint16_t s[DIST_MEDIAN];
  memcpy(s, _buf, sizeof(s));
  for (uint8_t i = 1; i < DIST_MEDIAN; i++) {
    const uint16_t v = s[i];
    int8_t j = (int8_t)i - 1;
    while (j >= 0 && s[j] > v) { s[j + 1] = s[j]; j--; }
    s[j + 1] = v;
  }
  const uint16_t med = s[DIST_MEDIAN / 2];

  // Ausserhalb des brauchbaren Bereichs gilt der Wert als "nichts da".
  // Der Sensor meldet bei freier Sicht Werte um 8190.
  if (med < DIST_MIN_MM || med > DIST_MAX_MM) {
    _valid = false;
    _mm    = 0;
  } else {
    _valid = true;
    _mm    = med;
  }
}
