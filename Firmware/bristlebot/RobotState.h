// =====================================================================
//  Bristlebot -- RobotState.h
//  Gemeinsame Typen: Betriebsart, Unterzustand, Tuningparameter.
// =====================================================================
#pragma once
#include <Arduino.h>
#include "Config.h"

enum class Mode : uint8_t {
  Manual = 0,   // Joystick aus der Web-App
  Auto   = 1    // autonomer Linienfolger
};

enum class Phase : uint8_t {
  Disarmed = 0, // Motoren gesperrt -- Zustand nach dem Einschalten
  Running  = 1,
  Searching= 2, // Linie verloren, Suchpendeln
  Lost     = 3, // Suche aufgegeben
  Calibrating = 4,
  LowBattery  = 5,
  Idle        = 6   // freigegeben, aber kein Fahrbefehl (App still/weg)
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
