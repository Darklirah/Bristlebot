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
//  Protokoll Client -> Roboter (Textzeilen, kommagetrennt):
//    J,<x>,<y>                 Joystick, x/y jeweils -1 .. +1
//    M,<0|1>                   Betriebsart 0 = manuell, 1 = autonom
//    A,<0|1>                   1 = freigeben (arm), 0 = sperren
//    X                         Nothalt
//    C                         Kalibrierfahrt starten
//    R                         Kalibrierung loeschen
//    T,<kp>,<kd>,<base>,<min>  Tuningwerte setzen
//    P                         Ping (haelt den Totmannschalter wach)
//
//  Roboter -> Client: eine JSON-Zeile pro Telemetrieintervall.
// =====================================================================
#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <WebSocketsServer.h>
#include "Config.h"
#include "RobotState.h"

// Anfragen der App, die die Hauptschleife abholt und dabei quittiert.
struct RcRequests {
  bool   haveMode   = false;  Mode mode = Mode::Manual;
  bool   haveArm    = false;  bool arm  = false;
  bool   estop      = false;
  bool   calibrate  = false;
  bool   resetCal   = false;
  bool   haveTuning = false;  Tuning tuning;
};

class RemoteControl {
public:
  void begin();
  void loop();

  const RemoteInput& input() const { return _in; }
  bool  clientConnected() const { return _clients > 0; }
  uint8_t clientCount() const { return _clients; }

  // Liefert die offenen Anfragen und setzt sie zurueck.
  RcRequests takeRequests();

  void sendTelemetry(const char* json);
  void publishTuning(const Tuning& t) { _lastTuning = t; }

private:
  void handleText(const char* line);

  WebServer        _http{80};
  WebSocketsServer _ws{WS_PORT};
  DNSServer        _dns;

  RemoteInput _in;
  RcRequests  _req;
  Tuning      _lastTuning;
  uint8_t     _clients = 0;
};
