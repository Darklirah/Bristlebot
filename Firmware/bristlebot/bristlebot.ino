// =====================================================================
//  B R I S T L E B O T
//  Linienfolgender Zahnbuersten-Roboter auf ESP32
//
//  Diese Datei macht nur die Orchestrierung: Zustandsmaschine, Regler,
//  Telemetrie. Alles Hardwarenahe steckt in den Modulen:
//
//    Config.h        Pins, Kennwerte, Feature-Flags  -- hier tunen
//    RobotState.h    Betriebsart, Phase, Tuningstruktur
//    Motors.*        PWM, Kennlinienspreizung, Kickstart
//    LineSensor.*    2x TCRT5000, Median-Filter, Kalibrierung in NVS
//    Leds.*          Blinkmuster fuer Lenkung und Status
//    RemoteControl.* Access Point, Webserver, WebSocket, Protokoll
//    WebUI.h         die Bedienoberflaeche (iPhone + Android, Browser)
//
//  Benoetigte Bibliothek (Library Manager):
//    WebSockets  von Markus Sattler ("arduinoWebSockets") >= 2.4.1
//  Alles andere ist Teil des ESP32-Arduino-Cores.
//
//  Board: "ESP32 Dev Module", Flash 4 MB, Partition "Default 4MB with spiffs"
// =====================================================================
#include "Config.h"
#include "RobotState.h"
#include "Motors.h"
#include "LineSensor.h"
#include "Leds.h"
#include "RemoteControl.h"

// ---------------------------------------------------------------------
//  Module
// ---------------------------------------------------------------------
static Motors        motors;
static LineSensor    line;
static Leds           leds;
static RemoteControl rc;

// ---------------------------------------------------------------------
//  Laufzeitzustand
// ---------------------------------------------------------------------
static Mode   g_mode   = Mode::Manual;
static Phase  g_phase  = Phase::Disarmed;
static bool   g_armed  = false;         // nach dem Einschalten immer gesperrt
static Tuning g_tune;

static float    g_lastError   = 0.0f;
static float    g_lastSteer   = 0.0f;
static int8_t   g_lastErrSign = 0;      // Richtung, in der die Linie zuletzt lag
static uint32_t g_lastCtrlMs  = 0;
static uint32_t g_lostSinceMs = 0;

static uint32_t g_tuneDirtyMs = 0;      // NVS-Schreiben entprellen
static uint32_t g_nextTeleMs  = 0;
static uint32_t g_loopCount   = 0;
static uint16_t g_loopHz      = 0;
static uint32_t g_hzWindowMs  = 0;

#if FEATURE_BATTERY_MONITOR
static float    g_vbat = 0.0f;
#endif
#if FEATURE_MODE_BUTTON
static uint32_t g_btnDebounceMs = 0;
static bool     g_btnLast = true;
#endif

// =====================================================================
//  Tuningwerte in NVS
// =====================================================================
static void loadTuning() {
  Preferences p;
  p.begin(NVS_NAMESPACE, true);
  g_tune.kp       = p.getFloat("kp",   CTRL_KP_DEFAULT);
  g_tune.kd       = p.getFloat("kd",   CTRL_KD_DEFAULT);
  g_tune.base     = p.getFloat("base", CTRL_BASE_DEFAULT);
  g_tune.minLevel = p.getFloat("minl", MOTOR_MIN_LEVEL_DEFAULT);
  p.end();
  motors.setMinLevel(g_tune.minLevel);
}

static void saveTuning() {
  Preferences p;
  p.begin(NVS_NAMESPACE, false);
  p.putFloat("kp",   g_tune.kp);
  p.putFloat("kd",   g_tune.kd);
  p.putFloat("base", g_tune.base);
  p.putFloat("minl", g_tune.minLevel);
  p.end();
}

// =====================================================================
//  Fahrbefehle
// =====================================================================
static inline float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }

// steer > 0 = nach rechts lenken. Rechtskurve heisst: linker Motor
// vibriert kraeftiger -- genau wie beim Differentialantrieb.
static void drive(float base, float steer) {
  g_lastSteer = constrain(steer, -1.0f, 1.0f);
  motors.set(clamp01(base + g_lastSteer), clamp01(base - g_lastSteer));
  leds.setSteer(g_lastSteer);
}

static void halt() {
  motors.stop();
  g_lastSteer = 0.0f;
  leds.setSteer(0.0f);
}

// ---------------------------------------------------------------------
//  Handbetrieb: Joystick mischen
// ---------------------------------------------------------------------
static void runManual() {
  const RemoteInput& in = rc.input();

  // Totmannschalter: ohne frisches Paket wird gestoppt. Verhindert, dass
  // der Roboter weiterlaeuft, wenn das Handy weggeht oder der Browser
  // in den Hintergrund wechselt.
  if (!in.connected || (millis() - in.lastPacketMs) > RC_TIMEOUT_MS) {
    halt();
    g_phase = Phase::Idle;
    return;
  }

  const float speed = in.y > 0.0f ? in.y : 0.0f;   // rueckwaerts gibt es nicht
  const float x     = in.x;

  if (speed < 0.05f) {
    if (fabsf(x) > 0.5f) {
      // Drehen auf der Stelle: nur die aeussere Seite vibriert
      if (x > 0) motors.set(PIVOT_LEVEL, 0.0f);
      else       motors.set(0.0f, PIVOT_LEVEL);
      g_lastSteer = x;
      leds.setSteer(x);
      g_phase = Phase::Running;
    } else {
      halt();
      g_phase = Phase::Idle;
    }
    return;
  }
  g_phase = Phase::Running;

  const float l = speed * (1.0f + x * MANUAL_STEER_AUTHORITY);
  const float r = speed * (1.0f - x * MANUAL_STEER_AUTHORITY);
  motors.set(clamp01(l), clamp01(r));
  g_lastSteer = x;
  leds.setSteer(x);
}

// ---------------------------------------------------------------------
//  Autonomer Linienfolger: PD-Regler auf die Sensordifferenz
// ---------------------------------------------------------------------
static void runAuto() {
  const uint32_t now = millis();

  if (!line.calibrated()) {      // ohne Kalibrierung keine Fahrt
    halt();
    g_phase = Phase::Lost;
    return;
  }

  if (line.lineVisible()) {
    g_lostSinceMs = 0;
    g_phase = Phase::Running;

    const float err = line.error();
    float dt = (now - g_lastCtrlMs) * 0.001f;
    if (dt < 0.001f) dt = 0.001f;         // Division durch Null vermeiden
    const float dErr = (err - g_lastError) / dt;
    g_lastError  = err;
    g_lastCtrlMs = now;
    if (fabsf(err) > 0.05f) g_lastErrSign = (err > 0) ? 1 : -1;

    drive(g_tune.base, g_tune.kp * err + g_tune.kd * dErr);
    return;
  }

  // Linie weg: erst pendelnd suchen, dann aufgeben
  if (g_lostSinceMs == 0) g_lostSinceMs = now;
  const uint32_t lost = now - g_lostSinceMs;

  if (lost > LINE_LOST_GIVEUP_MS) {
    halt();
    g_phase = Phase::Lost;
    return;
  }

  g_phase = Phase::Searching;
  // Zuerst in die Richtung drehen, in der die Linie zuletzt lag,
  // danach im Wechsel mit immer der anderen Seite.
  const bool flip    = ((lost / LINE_LOST_SEARCH_MS) & 1) != 0;
  const bool toRight = (g_lastErrSign >= 0) != flip;
  if (toRight) { motors.set(PIVOT_LEVEL, 0.0f); leds.setSteer(1.0f);  }
  else         { motors.set(0.0f, PIVOT_LEVEL); leds.setSteer(-1.0f); }
}

// =====================================================================
//  Anfragen der Web-App
// =====================================================================
static void applyRequests() {
  RcRequests q = rc.takeRequests();

  if (q.estop) {
    g_armed = false;
    halt();
  }
  if (q.haveArm) {
    g_armed = q.arm;
    if (!g_armed) halt();
  }
  if (q.haveMode && q.mode != g_mode) {
    g_mode = q.mode;
    halt();                         // Betriebsart wechselt nie "fliegend"
    g_lastError = 0.0f;
    g_lostSinceMs = 0;
  }
  if (q.calibrate) {
    halt();
    line.startCalibration();
  }
  if (q.resetCal) {
    line.resetCalibration();
  }
  if (q.haveTuning) {
    g_tune = q.tuning;
    motors.setMinLevel(g_tune.minLevel);
    rc.publishTuning(g_tune);
    g_tuneDirtyMs = millis();       // NVS-Schreiben verzoegert, s. loop()
  }
}

// =====================================================================
//  Telemetrie
// =====================================================================
static void sendTelemetry() {
  char buf[256];
#if FEATURE_BATTERY_MONITOR
  snprintf(buf, sizeof(buf),
    "{\"p\":%u,\"m\":%u,\"a\":%u,\"sL\":%.3f,\"sR\":%.3f,\"e\":%.3f,"
    "\"dL\":%u,\"dR\":%u,\"dMax\":%u,\"calOk\":%u,\"hz\":%u,\"vb\":%.2f}",
    (unsigned)g_phase, (unsigned)g_mode, g_armed ? 1u : 0u,
    line.linenessL(), line.linenessR(), line.error(),
    motors.dutyL(), motors.dutyR(), Motors::dutyMax(),
    line.calibrated() ? 1u : 0u, g_loopHz, g_vbat);
#else
  snprintf(buf, sizeof(buf),
    "{\"p\":%u,\"m\":%u,\"a\":%u,\"sL\":%.3f,\"sR\":%.3f,\"e\":%.3f,"
    "\"dL\":%u,\"dR\":%u,\"dMax\":%u,\"calOk\":%u,\"hz\":%u}",
    (unsigned)g_phase, (unsigned)g_mode, g_armed ? 1u : 0u,
    line.linenessL(), line.linenessR(), line.error(),
    motors.dutyL(), motors.dutyR(), Motors::dutyMax(),
    line.calibrated() ? 1u : 0u, g_loopHz);
#endif
  rc.sendTelemetry(buf);

#if FEATURE_SERIAL_DEBUG
  Serial.println(buf);
#endif
}

// =====================================================================
//  setup / loop
// =====================================================================
void setup() {
#if FEATURE_SERIAL_DEBUG
  Serial.begin(115200);
  delay(150);
  Serial.println();
  Serial.println(F("Bristlebot startet"));
#endif

  // Reihenfolge ist Absicht: erst die Motoren auf Null, dann der Rest.
  motors.begin();
  leds.begin();
  leds.setPhase(Phase::Disarmed);
  line.begin();
  loadTuning();

#if FEATURE_MODE_BUTTON
  pinMode(PIN_MODE_BTN, INPUT_PULLUP);
#endif

  rc.publishTuning(g_tune);
  rc.begin();

#if FEATURE_SERIAL_DEBUG
  Serial.printf("AP   : %s / %s\n", AP_SSID, AP_PASSWORD);
  Serial.printf("URL  : http://%s/\n", WiFi.softAPIP().toString().c_str());
  Serial.printf("Duty : max %u von %u (%.1f V Motor an %.1f V Schiene)\n",
                Motors::dutyMax(), PWM_FULL, MOTOR_RATED_V, MOTOR_SUPPLY_V);
  Serial.printf("Kal. : %s\n", line.calibrated() ? "geladen" : "FEHLT");
#endif

  g_lastCtrlMs = millis();
  g_hzWindowMs = millis();
  g_nextTeleMs = millis() + TELEMETRY_MS;
}

void loop() {
  const uint32_t now = millis();
  g_loopCount++;

  rc.loop();
  line.update();
  applyRequests();

#if FEATURE_MODE_BUTTON
  // Kurzer Druck schaltet die Betriebsart um
  const bool btn = digitalRead(PIN_MODE_BTN);
  if (btn != g_btnLast && (now - g_btnDebounceMs) > 40) {
    g_btnDebounceMs = now;
    g_btnLast = btn;
    if (btn == LOW) {
      g_mode = (g_mode == Mode::Manual) ? Mode::Auto : Mode::Manual;
      halt();
    }
  }
#endif

#if FEATURE_BATTERY_MONITOR
  // Gleitender Mittelwert, der ADC ist durch die Motoren verrauscht
  const float v = analogReadMilliVolts(PIN_VBAT) * 0.001f * VBAT_DIVIDER;
  g_vbat = (g_vbat == 0.0f) ? v : (g_vbat * 0.99f + v * 0.01f);
#endif

  // ---------------- Zustandsmaschine ----------------
  if (line.calibrating()) {
    halt();
    g_phase = Phase::Calibrating;
  }
#if FEATURE_BATTERY_MONITOR
  else if (g_vbat > 1.0f && g_vbat < VBAT_CUTOFF_V) {
    halt();
    g_armed = false;
    g_phase = Phase::LowBattery;
  }
#endif
  else if (!g_armed) {
    halt();
    g_phase = Phase::Disarmed;
  }
  else if (g_mode == Mode::Manual) {
    runManual();            // setzt g_phase selbst (Running / Idle)
  }
  else {
    runAuto();
  }

  leds.setPhase(g_phase);
  motors.update();
  leds.update();

  // Tuningwerte erst 1,5 s nach der letzten Aenderung sichern --
  // ein Schieberegler feuert sonst dutzende NVS-Schreibvorgaenge.
  if (g_tuneDirtyMs && (now - g_tuneDirtyMs) > 1500) {
    g_tuneDirtyMs = 0;
    saveTuning();
  }

  // Schleifenfrequenz messen (steht in der App, guter Gesundheitswert)
  if (now - g_hzWindowMs >= 1000) {
    g_loopHz = (uint16_t)g_loopCount;
    g_loopCount = 0;
    g_hzWindowMs = now;
  }

  if ((int32_t)(now - g_nextTeleMs) >= 0) {
    g_nextTeleMs = now + TELEMETRY_MS;
    sendTelemetry();
  }
}
