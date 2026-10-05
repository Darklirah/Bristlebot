#include "RemoteControl.h"
#include "WebUI.h"

void RemoteControl::begin() {
  WiFi.mode(WIFI_AP);
  // Sendeleistung bewusst reduziert: spart Strom und die Reichweite im
  // Zimmer ist ohnehin mehr als ausreichend.
  WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, 0 /*nicht versteckt*/, 4 /*max 4 Clients*/);
  WiFi.setTxPower(WIFI_POWER_11dBm);

  const IPAddress ip = WiFi.softAPIP();

  // Jeden DNS-Namen auf uns selbst zeigen lassen -> Captive Portal
  _dns.setErrorReplyCode(DNSReplyCode::NoError);
  _dns.start(53, "*", ip);

  // Die eigentliche App
  _http.on("/", HTTP_GET, [this]() {
    _http.sendHeader("Cache-Control", "no-store");
    _http.send_P(200, "text/html; charset=utf-8", WEBUI_HTML);
  });
  // Alles andere ebenfalls mit der App beantworten. Damit schlaegt die
  // Captive-Portal-Erkennung von iOS (captive.apple.com) und Android
  // (/generate_204) an und das Bedienfenster klappt von selbst auf.
  _http.onNotFound([this]() {
    _http.sendHeader("Cache-Control", "no-store");
    _http.send_P(200, "text/html; charset=utf-8", WEBUI_HTML);
  });
  _http.begin();

  _ws.begin();
  _ws.onEvent([this](uint8_t num, WStype_t type, uint8_t* payload, size_t len) {
    switch (type) {
      case WStype_CONNECTED: {
        _clients++;
        _in.connected = true;
        _in.lastPacketMs = millis();
        // Aktuelle Tuningwerte an die frische Verbindung schicken,
        // damit die Schieberegler sofort richtig stehen.
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "{\"t\":\"cfg\",\"kp\":%.3f,\"kd\":%.3f,\"base\":%.3f,\"min\":%.3f}",
                 _lastTuning.kp, _lastTuning.kd, _lastTuning.base, _lastTuning.minLevel);
        _ws.sendTXT(num, buf);
        break;
      }
      case WStype_DISCONNECTED:
        if (_clients) _clients--;
        if (_clients == 0) {
          _in.connected = false;
          // Verbindung weg: Joystick auf Null, der Totmannschalter
          // in der Hauptschleife haelt die Motoren an.
          _in.x = 0.0f;
          _in.y = 0.0f;
        }
        break;
      case WStype_TEXT: {
        if (len == 0 || len > 95) break;
        char line[96];
        memcpy(line, payload, len);
        line[len] = '\0';
        _in.lastPacketMs = millis();
        handleText(line);
        break;
      }
      default:
        break;
    }
  });
}

void RemoteControl::handleText(const char* line) {
  float a = 0, b = 0, c = 0, d = 0;

  switch (line[0]) {
    case 'J':
      if (sscanf(line + 1, ",%f,%f", &a, &b) == 2) {
        _in.x = constrain(a, -1.0f, 1.0f);
        _in.y = constrain(b, -1.0f, 1.0f);
      }
      break;

    case 'M':
      if (sscanf(line + 1, ",%f", &a) == 1) {
        _req.haveMode = true;
        _req.mode = (a >= 0.5f) ? Mode::Auto : Mode::Manual;
      }
      break;

    case 'A':
      if (sscanf(line + 1, ",%f", &a) == 1) {
        _req.haveArm = true;
        _req.arm = (a >= 0.5f);
      }
      break;

    case 'X':
      _req.estop = true;
      _in.x = _in.y = 0.0f;
      break;

    case 'C': _req.calibrate = true; break;
    case 'R': _req.resetCal  = true; break;

    case 'T':
      if (sscanf(line + 1, ",%f,%f,%f,%f", &a, &b, &c, &d) == 4) {
        _req.haveTuning = true;
        _req.tuning.kp       = constrain(a, 0.0f, 3.0f);
        _req.tuning.kd       = constrain(b, 0.0f, 1.0f);
        _req.tuning.base     = constrain(c, 0.0f, 1.0f);
        _req.tuning.minLevel = constrain(d, 0.0f, 0.95f);
      }
      break;

    case 'P':   // Ping -- lastPacketMs wurde oben schon aktualisiert
    default:
      break;
  }
}

RcRequests RemoteControl::takeRequests() {
  RcRequests out = _req;
  _req = RcRequests{};
  return out;
}

void RemoteControl::sendTelemetry(const char* json) {
  if (_clients) _ws.broadcastTXT(json);
}

void RemoteControl::loop() {
  _dns.processNextRequest();
  _http.handleClient();
  _ws.loop();
}
