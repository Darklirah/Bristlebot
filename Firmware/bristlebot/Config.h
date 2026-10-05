// =====================================================================
//  Bristlebot -- Config.h
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
static const char     AP_SSID[]      = "Bristlebot";
static const char     AP_PASSWORD[]  = "bristlebot";  // min. 8 Zeichen!
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
