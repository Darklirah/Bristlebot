#include "Imu.h"
#include <Wire.h>
#include <Preferences.h>

// MPU-6050-Register
static const uint8_t REG_SMPLRT_DIV   = 0x19;
static const uint8_t REG_CONFIG       = 0x1A;
static const uint8_t REG_GYRO_CONFIG  = 0x1B;
static const uint8_t REG_ACCEL_CONFIG = 0x1C;
static const uint8_t REG_ACCEL_XOUT_H = 0x3B;
static const uint8_t REG_PWR_MGMT_1   = 0x6B;
static const uint8_t REG_WHO_AM_I     = 0x75;

// Skalierung passend zu IMU_GYRO_FS / IMU_ACCEL_FS aus Config.h
static const float GYRO_LSB_PER_DPS = 131.0f / (1 << IMU_GYRO_FS);   // FS=1 -> 65,5
static const float ACCEL_LSB_PER_G  = 16384.0f / (1 << IMU_ACCEL_FS); // FS=2 -> 4096

bool Imu::write8(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(IMU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

bool Imu::readBurst(uint8_t reg, uint8_t* buf, uint8_t len) {
  Wire.beginTransmission(IMU_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((int)IMU_ADDR, (int)len) != len) return false;
  for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
  return true;
}

bool Imu::begin() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_FREQ_HZ);

  uint8_t who = 0;
  if (!readBurst(REG_WHO_AM_I, &who, 1)) { _present = false; return false; }
  // Nachbauten melden gern 0x68, 0x70, 0x72 oder 0x98. Nur 0x00/0xFF
  // bedeuten sicher "da ist nichts".
  if (who == 0x00 || who == 0xFF) { _present = false; return false; }

  // Takt vom X-Gyro statt vom internen Oszillator -- deutlich stabiler
  if (!write8(REG_PWR_MGMT_1, 0x01)) { _present = false; return false; }
  delay(10);
  write8(REG_CONFIG,       IMU_DLPF_CFG);
  write8(REG_GYRO_CONFIG,  (uint8_t)(IMU_GYRO_FS  << 3));
  write8(REG_ACCEL_CONFIG, (uint8_t)(IMU_ACCEL_FS << 3));
  // Mit aktivem DLPF liegt die Basisrate bei 1 kHz -> 9 ergibt 100 Hz
  write8(REG_SMPLRT_DIV, 9);
  delay(20);

  _present = true;
  _lastUs  = micros();
  loadBias();
  return true;
}

// =====================================================================
//  Nullpunkt
// =====================================================================
void Imu::startCalibration() {
  if (!_present) return;
  _calSum   = 0.0;
  _calN     = 0;
  _calDone  = false;
  _calUntil = millis() + IMU_CAL_MS;
}

void Imu::loadBias() {
  Preferences p;
  p.begin(NVS_NAMESPACE, true);
  _biasZ   = p.getFloat("gz", 0.0f);
  _calDone = p.getBool("gzok", false);
  p.end();
}

void Imu::saveBias() {
  Preferences p;
  p.begin(NVS_NAMESPACE, false);
  p.putFloat("gz", _biasZ);
  p.putBool("gzok", true);
  p.end();
}

// =====================================================================
//  Abtastung
// =====================================================================
void Imu::update() {
  if (!_present) return;

  const uint32_t now = millis();
  if ((int32_t)(now - _nextSample) < 0) return;
  _nextSample = now + IMU_SAMPLE_MS;

  uint8_t b[14];
  if (!readBurst(REG_ACCEL_XOUT_H, b, 14)) return;

  const int16_t ax = (int16_t)((b[0]  << 8) | b[1]);
  const int16_t ay = (int16_t)((b[2]  << 8) | b[3]);
  const int16_t az = (int16_t)((b[4]  << 8) | b[5]);
  // b[6],b[7] = Temperatur, b[8..11] = Gyro X/Y -- hier nicht gebraucht
  const int16_t gz = (int16_t)((b[12] << 8) | b[13]);

  // echte Schrittweite messen statt IMU_SAMPLE_MS anzunehmen: die
  // Hauptschleife ist nicht taktfest, und der Integrationsfehler
  // waere sonst systematisch
  const uint32_t nowUs = micros();
  float dt = (nowUs - _lastUs) * 1e-6f;
  _lastUs = nowUs;
  if (dt <= 0.0f || dt > 0.25f) dt = IMU_SAMPLE_MS * 0.001f;  // Ausreisser

  // ---- Nullpunktaufnahme ----
  if (_calUntil) {
    _calSum += gz;
    _calN++;
    if ((int32_t)(now - _calUntil) >= 0) {
      _calUntil = 0;
      if (_calN > 50) {
        _biasZ   = (float)(_calSum / (double)_calN);
        _calDone = true;
        _heading = 0.0f;
        saveBias();
      }
    }
    _rate = 0.0f;
    return;
  }

  // ---- Kurs ----
  float dps = (gz - _biasZ) / GYRO_LSB_PER_DPS;
  if (fabsf(dps) < IMU_RATE_DEADBAND_DPS) dps = 0.0f;   // Rauschen nicht aufintegrieren
  _rate     = dps;
  _heading += dps * dt;

  // ---- Lage ----
  // Beide Groessen stark tiefpassgefiltert: die Rohwerte sind auf diesem
  // Geraet durch die Vibration praktisch unlesbar.
  const float azG = az / ACCEL_LSB_PER_G;
  const float mag = sqrtf((float)ax * ax + (float)ay * ay + (float)az * az) / ACCEL_LSB_PER_G;
  const float a   = 0.02f;                       // Zeitkonstante ~0,25 s bei 200 Hz
  _azLp  = _azLp  * (1.0f - a) + azG * a;
  _magLp = _magLp * (1.0f - a) + mag * a;

  // Umgekippt: die Z-Achse zeigt nicht mehr nach oben
  if (_azLp < TILT_AZ_MIN_G) {
    if (_tiltSince == 0) _tiltSince = now;
    if (now - _tiltSince > TILT_CONFIRM_MS) _tipped = true;
  } else {
    _tiltSince = 0;
    _tipped = false;
  }

  // Hochgehoben: der Betrag der Beschleunigung weicht laenger von 1 g ab.
  // Bewusst als Schaetzung gefuehrt -- waehrend der Fahrt ruettelt es so
  // stark, dass hier Fehlalarme moeglich sind. Deshalb grosszuegige
  // Schwellen und eine lange Bestaetigungszeit.
  if (_magLp < LIFT_G_LOW || _magLp > LIFT_G_HIGH) {
    if (_liftSince == 0) _liftSince = now;
    if (now - _liftSince > LIFT_CONFIRM_MS) _lifted = true;
  } else {
    _liftSince = 0;
    _lifted = false;
  }
}
