// =====================================================================
//  Bristlebot -- RemoteControl.h
//
//  Der ESP32 spannt einen eigenen Access Point auf und liefert die
//  Bedienoberflaeche selbst aus. Damit laeuft die Fernsteuerung auf
//  iPhone und Android identisch im Browser -- ohne App-Installation.
//
//    Port 80  HTTP   : liefert die Web-App (WebUI.h), beantwortet
//                      zusaetzlich die Captive-Portal-Pruefadressen von
//                      iOS und Android, damit sich die Seite von selbst
//                      oeffnet.
//    Port 53  DNS    : loest jeden Namen auf 192.168.4.1 auf.
//    Port 81  WebSock: Steuerdaten hin, Telemetrie zurueck.
//
//  Protokoll Client -> Roboter (Textzeilen):
//    J,<x>,<y>                 Joystick, x/y jeweils -1 .. +1
//    M,<0|1|2>                 0 = manuell, 1 = Linienfolger, 2 = Programm
//    A,<0|1>                   1 = freigeben (arm), 0 = sperren
//    X                         Nothalt
//    C                         Kalibrierfahrt starten
//    R                         Kalibrierung loeschen
//    T,<kp>,<kd>,<base>,<min>  Tuningwerte setzen
//    P                         Ping (haelt den Totmannschalter wach)
//    B,<0|1>                   Fahrprogramm stoppen / starten
//    L,<slot>                  Speicherplatz laden und zuruecksenden
//    W,<slot>|<name>|<text>    Speicherplatz schreiben
//    S,<0|1>                   Funktionstest stoppen / starten
//    G                         Lagesensor nullen (Roboter ruhig halten)
//    O,<0|1>,<mm>              Hindernis-Stopp aus/ein und Abstand
//
//  Bei W trennt '|' die Felder, weil Name und Programmtext selbst Kommas
//  enthalten. Alle uebrigen Befehle bleiben kommagetrennt.
//
//  Roboter -> Client: eine JSON-Zeile je Telemetrieintervall, dazu die
//  Antworttypen "cfg", "slots", "prog", "ok" und "err".
// =====================================================================
#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <WebSocketsServer.h>
#include "Config.h"
#include "RobotState.h"
#include "Program.h"

// Anfragen der App, die die Hauptschleife abholt und dabei quittiert.
struct RcRequests {
  bool   haveMode   = false;  Mode mode = Mode::Manual;
  bool   haveArm    = false;  bool arm  = false;
  bool   estop      = false;
  bool   calibrate  = false;
  bool   resetCal   = false;
  bool   haveTuning = false;  Tuning tuning;
  bool   haveRun    = false;  bool run  = false;   // Fahrprogramm
  bool   haveTest   = false;  bool test = false;   // Funktionstest
  bool   zeroGyro   = false;                       // Lagesensor nullen
  bool   haveGuard  = false;  bool guard = true;   // Hindernis-Stopp
  uint16_t guardMm  = OBSTACLE_STOP_MM_DEFAULT;
};

class RemoteControl {
public:
  void begin();
  void loop();

  // Der Programmspeicher wird direkt von hier aus gelesen und geschrieben:
  // Laden und Speichern brauchen eine sofortige Antwort an genau den
  // Client, der gefragt hat. Das Starten und Stoppen laeuft dagegen ueber
  // RcRequests, damit die Zustandsmaschine die Kontrolle behaelt.
  void attachProgram(Program* p) { _prog = p; }

  const RemoteInput& input() const { return _in; }
  bool    clientConnected() const { return _clients > 0; }
  uint8_t clientCount() const { return _clients; }

  RcRequests takeRequests();

  void sendTelemetry(const char* json);
  void publishTuning(const Tuning& t) { _lastTuning = t; }
  void broadcastSlots();                      // nach dem Speichern

private:
  void handleText(uint8_t num, const char* line);
  void sendCfg(uint8_t num);
  void sendSlots(int16_t num);                // num < 0 = an alle
  void sendProg(uint8_t num, uint8_t slot);
  void sendResult(uint8_t num, bool ok, const char* msg);
  static void jsonEscape(const char* in, char* out, size_t outLen);

  WebServer        _http{80};
  WebSocketsServer _ws{WS_PORT};
  DNSServer        _dns;

  Program*    _prog = nullptr;
  RemoteInput _in;
  RcRequests  _req;
  Tuning      _lastTuning;
  uint8_t     _clients = 0;
};
