// =====================================================================
//  MarsRover -- Config.h
//  Zentrale Stelle fuer Pins, Kennwerte und Feature-Flags.
//  Nur hier anfassen, wenn Hardware oder Tuning sich aendert.
// =====================================================================
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------
//  Arduino-Core-Version: die LEDC-API hat sich mit Core 3.x geaendert.
//  Dieses Flag haelt Motors.cpp fuer beide Versionen kompilierbar.
// ---------------------------------------------------------------------
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  #define BB_LEDC_V3 1
#else
  #define BB_LEDC_V3 0
#endif

// ---------------------------------------------------------------------
//  Feature-Flags  (0 = aus; entspricht exakt dem Schaltplan ohne Extras)
// ---------------------------------------------------------------------
#define FEATURE_BATTERY_MONITOR 0   // braucht 2x 100k Spannungsteiler an GPIO39
#define FEATURE_MODE_BUTTON     0   // braucht Taster GPIO4 -> GND
#define FEATURE_SERIAL_DEBUG    1   // Telemetrie auch auf die USB-Konsole

// ---------------------------------------------------------------------
//  P I N B E L E G U N G
//
//  Abweichungen vom Originalentwurf (begruendet in Doku/01_Schaltplan-Review.md):
//    Motoren GPIO 12/13  ->  GPIO 32/33
//      GPIO12 ist Strapping-Pin (MTDI) und legt beim Reset die Flash-
//      Spannung fest. Eine Transistorbasis mit 1k daran kann das Modul
//      am Booten hindern. 32/33 haben keine Strapping-Funktion.
//    Liniensensoren auf GPIO 34/35 (ADC1). ADC2 ist bei aktivem WiFi
//      komplett gesperrt -- mit AP-Betrieb also tabu.
//    LEDs bleiben wie von dir vorgegeben (14/27/26/25).
// ---------------------------------------------------------------------
static const uint8_t PIN_MOTOR_L = 32;   // PWM -> 1k -> Basis T1 -> Motor links
static const uint8_t PIN_MOTOR_R = 33;   // PWM -> 1k -> Basis T2 -> Motor rechts

static const uint8_t PIN_LINE_L  = 34;   // ADC1_CH6, nur Eingang
static const uint8_t PIN_LINE_R  = 35;   // ADC1_CH7, nur Eingang

static const uint8_t PIN_LED_FL  = 14;   // gelb  vorne links
static const uint8_t PIN_LED_FR  = 27;   // gelb  vorne rechts
static const uint8_t PIN_LED_RL  = 26;   // rot   hinten links
static const uint8_t PIN_LED_RR  = 25;   // rot   hinten rechts

#if FEATURE_BATTERY_MONITOR
static const uint8_t PIN_VBAT      = 39;    // ADC1_CH3 ueber Teiler
static const float   VBAT_DIVIDER  = 2.0f;  // 100k / 100k
static const float   VBAT_WARN_V   = 3.40f; // ab hier rote LEDs blinken
static const float   VBAT_CUTOFF_V = 3.20f; // ab hier Motoren aus
#endif
#if FEATURE_MODE_BUTTON
static const uint8_t PIN_MODE_BTN = 4;      // gegen GND, interner Pullup
#endif

// ---------------------------------------------------------------------
//  M O T O R E N   /   P W M
// ---------------------------------------------------------------------
static const uint32_t PWM_FREQ_HZ = 20000;  // >20 kHz: kein hoerbares Pfeifen
static const uint8_t  PWM_BITS    = 10;     // 0 .. 1023
static const uint16_t PWM_FULL    = (1u << PWM_BITS) - 1;

// Spannung der Motorschiene und Nennspannung der Vibrationsmotoren.
// Daraus wird der maximal zulaessige Duty berechnet -- so kannst du
// 3V-Motoren gefahrlos an der 5V-Schiene betreiben.
static const float MOTOR_SUPPLY_V = 5.0f;
static const float MOTOR_RATED_V  = 3.0f;
static const uint16_t MOTOR_DUTY_MAX =
    (uint16_t)(PWM_FULL * (MOTOR_RATED_V < MOTOR_SUPPLY_V
                           ? MOTOR_RATED_V / MOTOR_SUPPLY_V : 1.0f));

// Unterhalb dieses Anteils dreht ein ERM-Motor wegen Haftreibung gar nicht.
// Beim Einschalten gibt es deshalb einen kurzen Volldampf-Impuls.
// Live nachstellbar in der Web-App, wird in NVS gesichert.
static const float    MOTOR_MIN_LEVEL_DEFAULT = 0.35f;
static const uint16_t MOTOR_KICK_MS           = 40;

// ---------------------------------------------------------------------
//  L I N I E N S E N S O R   (2x TCRT5000, analog)
// ---------------------------------------------------------------------
static const bool     LINE_IS_DARK   = true;  // false = helle Linie auf dunkel
static const uint8_t  LINE_SAMPLE_MS = 2;     // Abtastintervall
static const uint16_t LINE_CAL_MS    = 5000;  // Dauer der Kalibrierfahrt
static const float    LINE_PRESENT_THRESHOLD = 0.35f; // ab hier "Linie gesehen"
static const uint16_t LINE_CAL_MIN_SPAN      = 150;   // ADC-Hub, sonst Kal. ungueltig

// ---------------------------------------------------------------------
//  R E G L E R   (Startwerte, live nachstellbar, in NVS gesichert)
// ---------------------------------------------------------------------
static const float CTRL_KP_DEFAULT        = 0.55f;
static const float CTRL_KD_DEFAULT        = 0.08f;
static const float CTRL_BASE_DEFAULT      = 0.55f;  // Grundvibration autonom
static const float MANUAL_STEER_AUTHORITY = 0.60f;  // Lenkanteil im Handbetrieb
static const float PIVOT_LEVEL            = 0.70f;  // Drehen auf der Stelle

// Linie verloren -> Suchmuster, dann Nothalt
static const uint16_t LINE_LOST_SEARCH_MS = 220;   // Halbperiode des Pendelns
static const uint16_t LINE_LOST_GIVEUP_MS = 4000;  // danach Stop

// ---------------------------------------------------------------------
//  F A H R P R O G R A M M
//
//  Ein Programm ist eine Folge einfacher Befehle. Fahrbefehle setzen nur
//  den Zustand, Zeit verbraucht ausschliesslich "warte" -- so lassen sich
//  Fahrt und LEDs frei kombinieren.
// ---------------------------------------------------------------------
static const uint8_t  PROG_SLOTS      = 4;    // Speicherplaetze im Flash
static const uint8_t  PROG_MAX_STEPS  = 48;   // Schritte je Programm
static const uint16_t PROG_MAX_CHARS  = 512;  // serialisierte Laenge
static const uint8_t  PROG_NAME_MAX   = 20;   // Zeichen im Slotnamen

static const uint16_t PROG_WAIT_MIN_MS = 50;
static const uint16_t PROG_WAIT_MAX_MS = 60000;

// Wie viele Sofortbefehle hoechstens in einem loop()-Durchlauf abgearbeitet
// werden. Verhindert, dass ein Programm die Schleife blockiert.
static const uint8_t  PROG_INSTANT_BUDGET = 32;

// Kurvenradius 1..10 -> Lenkanteil. Stufe 1 ist die engste Kurve
// (dreht fast auf der Stelle), Stufe 10 eine weite, sanfte Kurve.
static const uint8_t  PROG_RADIUS_LEVELS = 10;

// Blinkfrequenz Stufe 1..10 -> 0,5 .. 5,0 Hz.
// Halbe Periode in ms = 1000 / Stufe.
static const uint8_t  BLINK_LEVEL_DEFAULT = 4;   // = 2 Hz

// ---------------------------------------------------------------------
//  F E R N S T E U E R U N G   (eigener Access Point + Web-App)
// ---------------------------------------------------------------------
// Der Netzname wird zur Laufzeit zusammengesetzt: AP_PREFIX + "_" + Kennung.
// Die Kennung ist entweder ein selbst vergebener Name aus dem NVS
// ("MarsRover_Petra") oder, solange keiner gesetzt ist, die letzten vier
// Stellen der MAC-Adresse ("MarsRover_A3F2"). Damit sind mehrere Geraete
// ab Werk unterscheidbar, ohne dass jemand etwas einstellen muss.
static const char     AP_PREFIX[]    = "MarsRover";
static const char     AP_PASSWORD[]  = "marsrover";  // min. 8 Zeichen!
static const uint8_t  AP_NAME_MAX    = 16;           // Zeichen im eigenen Namen
static const uint8_t  AP_CHANNEL     = 6;
static const uint16_t WS_PORT        = 81;
static const uint16_t TELEMETRY_MS   = 150;   // ~6,7 Hz an die App
static const uint16_t RC_TIMEOUT_MS  = 500;   // Totmannschalter im Handbetrieb

// Groesste Textzeile, die die App schicken darf. Muss ein komplettes
// Fahrprogramm samt Slotname aufnehmen koennen.
static const uint16_t RC_LINE_MAX    = PROG_MAX_CHARS + PROG_NAME_MAX + 16;

// NVS-Namespace fuer Kalibrierung und Tuningwerte
static const char NVS_NAMESPACE[] = "bbot";

// ---------------------------------------------------------------------
//  F U N K T I O N S T E S T
//
//  Eingebauter Selbsttest fuer Inbetriebnahme und Fehlersuche. Laeuft
//  alle Ausgaenge der Reihe nach durch und wiederholt sich, bis er
//  gestoppt wird. Kein Fahrprogramm -- belegt keinen Speicherplatz und
//  ist nicht editierbar.
// ---------------------------------------------------------------------
static const uint16_t TEST_LED_STEP_MS = 700;   // je LED beim Durchschalten
static const uint16_t TEST_BLINK_MS    = 3000;  // alle gemeinsam blinkend
static const uint16_t TEST_RAMP_MS     = 3000;  // Motorrampe min -> max
static const uint16_t TEST_GAP_MS      = 600;   // Pause zwischen den Abschnitten
static const uint8_t  TEST_BLINK_LEVEL = 4;     // 2 Hz
static const uint16_t TEST_GYRO_MS     = 1800;  // Drehabschnitt je Richtung
static const uint16_t TEST_DIST_MS     = 4000;  // Abstandsmesswert anzeigen

// ---------------------------------------------------------------------
//  L A G E S E N S O R   (MPU-6050 am I2C)
//
//  Liefert ueber das Z-Gyro einen Kurswinkel. Damit wird aus "dreh 1,2 s
//  nach rechts" ein geregeltes "dreh um 90 Grad", und "geradeaus" kann
//  seinen Kurs halten statt wegzudriften.
//
//  Was er NICHT kann: Strecken messen. Zweifache Integration der
//  Beschleunigung ist auf einem vibrationsgetriebenen Roboter binnen
//  ein bis zwei Sekunden unbrauchbar.
//
//  Der Sensor ist optional. Fehlt er, laeuft alles weiter; Drehbefehle
//  fallen dann auf eine Zeitschaetzung zurueck (TURN_RATE_FALLBACK_DPS).
// ---------------------------------------------------------------------
#define FEATURE_IMU 1

static const uint8_t  PIN_I2C_SDA  = 21;
static const uint8_t  PIN_I2C_SCL  = 22;
static const uint8_t  IMU_ADDR     = 0x68;   // AD0 auf GND
static const uint32_t I2C_FREQ_HZ  = 400000;

// Digitalfilter im Sensor. 4 = 21 Hz. Die Motorvibration liegt bei
// 100..200 Hz und wuerde sich ohne dieses Filter ins Nutzsignal falten.
// Bleibt trotzdem Unruhe im Kurs: auf 5 (10 Hz) heruntergehen.
static const uint8_t  IMU_DLPF_CFG = 4;
static const uint8_t  IMU_GYRO_FS  = 1;      // 1 = +-500 deg/s, 65,5 LSB je deg/s
static const uint8_t  IMU_ACCEL_FS = 2;      // 2 = +-8 g, 4096 LSB je g
                                             // bewusst grob: enge Bereiche
                                             // uebersteuern bei Vibration und
                                             // erzeugen dadurch Scheinwerte
static const uint16_t IMU_SAMPLE_MS = 5;     // 200 Hz Abtastung
static const uint16_t IMU_CAL_MS    = 1200;  // Nullpunktaufnahme im Stillstand
static const float    IMU_RATE_DEADBAND_DPS = 0.3f;  // gegen Driften im Stand

// Kurs halten bei "geradeaus"
static const float    HOLD_KP  = 0.020f;     // Lenkanteil je Grad Abweichung
static const float    HOLD_MAX = 0.40f;

// Drehen nach Winkel
static const float    TURN_TOLERANCE_DEG     = 3.0f;
static const uint16_t TURN_TIMEOUT_MS        = 12000;
static const float    TURN_RATE_FALLBACK_DPS = 90.0f;  // nur ohne Lagesensor
static const uint16_t TURN_DEG_MAX           = 360;

// Kipp- und Aufheb-Erkennung
static const float    TILT_AZ_MIN_G    = 0.45f;  // darunter liegt er nicht mehr flach
static const uint16_t TILT_CONFIRM_MS  = 400;
static const float    LIFT_G_LOW       = 0.55f;  // Betrag der Beschleunigung
static const float    LIFT_G_HIGH      = 1.45f;
static const uint16_t LIFT_CONFIRM_MS  = 500;

// ---------------------------------------------------------------------
//  A B S T A N D S S E N S O R   (VL53L0X auf dem Mast)
//
//  Laufzeitmessung mit Infrarotlaser, am selben I2C-Bus wie der
//  Lagesensor (Adresse 0x29 gegen 0x68, kein Konflikt).
//
//  Warum auf einem Mast: der Sensor soll waagerecht nach vorn schauen,
//  nicht auf den Boden. Und anders als ein Magnetometer stoert ihn die
//  Montage dort oben nicht -- eine optische Laufzeitmessung mittelt ueber
//  ihr Messfenster, ein peitschender Mast verwischt hoechstens den
//  Zielpunkt, nicht den Messwert.
//
//  Treiber: Bibliothek "VL53L0X" von Pololu. Die Initialisierung des
//  Chips ist eine mehrere hundert Zeilen lange Registersequenz -- die
//  schreibt man nicht selbst nach.
// ---------------------------------------------------------------------
#define FEATURE_DISTANCE 1

static const uint16_t DIST_TIMING_BUDGET_US = 33000;  // ~30 Messungen je Sekunde
static const uint16_t DIST_SAMPLE_MS = 35;
static const uint16_t DIST_MIN_MM    = 40;    // darunter ist der Sensor blind
static const uint16_t DIST_MAX_MM    = 1200;  // darueber unzuverlaessig
static const uint8_t  DIST_MEDIAN    = 3;     // Median gegen Ausreisser

// Hindernis-Stopp: faehrt der Roboter auf etwas zu, halten die Motoren an.
// In der Oberflaeche abschaltbar.
static const uint16_t OBSTACLE_STOP_MM_DEFAULT = 90;
static const uint16_t OBSTACLE_TIMEOUT_MS      = 15000;  // fuer "fahre bis Hindernis"
