// =====================================================================
//  MarsRover -- RobotState.h
//  Gemeinsame Typen: Betriebsart, Unterzustand, LED-Zustand, Tuning.
// =====================================================================
#pragma once
#include <Arduino.h>
#include "Config.h"

enum class Mode : uint8_t {
  Manual  = 0,   // Joystick aus der Web-App
  Auto    = 1,   // autonomer Linienfolger
  Program = 2    // gespeichertes Fahrprogramm abspielen
};

enum class Phase : uint8_t {
  Disarmed    = 0, // Motoren gesperrt -- Zustand nach dem Einschalten
  Running     = 1,
  Searching   = 2, // Linie verloren, Suchpendeln
  Lost        = 3, // Suche aufgegeben
  Calibrating = 4,
  LowBattery  = 5,
  Idle        = 6, // freigegeben, aber kein Fahrbefehl
  ProgramDone = 7, // Fahrprogramm durchgelaufen
  SelfTest    = 8, // eingebauter Funktionstest laeuft
  Tilted      = 9, // umgekippt oder hochgehoben -- Motoren gesperrt
  Obstacle    = 10 // Hindernis voraus -- Motoren angehalten
};

// Zustand einer einzelnen Signal-LED. Im Fahrprogramm direkt setzbar.
enum class LedState : uint8_t { Off = 0, On = 1, Blink = 2 };

// Reihenfolge der LEDs in allen Arrays -- einmal festgelegt, überall gleich.
enum LedIndex : uint8_t {
  LED_FL = 0,   // vorne links, gelb
  LED_FR = 1,   // vorne rechts, gelb
  LED_RL = 2,   // hinten links, rot
  LED_RR = 3,   // hinten rechts, rot
  LED_COUNT = 4
};

// Zielgruppen, die ein LED-Befehl im Fahrprogramm ansprechen kann.
enum LedTarget : uint8_t {
  LT_FL = 0, LT_FR = 1, LT_RL = 2, LT_RR = 3,
  LT_FRONT = 4, LT_REAR = 5, LT_ALL = 6,
  LT_COUNT = 7
};

// Live nachstellbare Werte. Werden in NVS gesichert und beim Start geladen.
struct Tuning {
  float kp       = CTRL_KP_DEFAULT;
  float kd       = CTRL_KD_DEFAULT;
  float base     = CTRL_BASE_DEFAULT;
  float minLevel = MOTOR_MIN_LEVEL_DEFAULT;
};

// Was die Web-App gerade vom Roboter will.
struct RemoteInput {
  float    x = 0.0f;        // -1 .. +1   Lenken
  float    y = 0.0f;        //  0 .. +1   Geschwindigkeit (negativ = Halt)
  uint32_t lastPacketMs = 0;
  bool     connected = false;
};
