#include "RemoteControl.h"
#include "WebUI.h"

// Empfangspuffer. Statisch, weil ein komplettes Fahrprogramm hineinpasst
// und das auf dem Stack der WebSocket-Rueckrufe zu viel waere. Die
// Rueckrufe laufen alle im Kontext von loop(), also einfaedig -- ein
// gemeinsamer Puffer ist hier ungefaehrlich.
static char s_line[RC_LINE_MAX + 1];
static char s_out[PROG_MAX_CHARS + 128];

// Netznamen zusammensetzen. Eigener Name aus dem NVS, sonst die letzten
// vier Stellen der MAC -- so hat jedes Geraet ab Werk einen eigenen Namen,
// und mehrere Rover stoeren sich nicht gegenseitig.
void RemoteControl::buildSsid() {
  Preferences p;
  p.begin(NVS_NAMESPACE, true);
  String own = p.getString("apname", "");
  p.end();

  if (own.length() > 0) {
    snprintf(_ssid, sizeof(_ssid), "%s_%s", AP_PREFIX, own.c_str());
    snprintf(_apName, sizeof(_apName), "%s", own.c_str());
  } else {
    uint8_t mac[6] = {0};
    WiFi.softAPmacAddress(mac);
    snprintf(_ssid, sizeof(_ssid), "%s_%02X%02X", AP_PREFIX, mac[4], mac[5]);
    _apName[0] = '\0';                 // leer = automatische Kennung
  }
}

bool RemoteControl::setApName(const char* name) {
  char clean[AP_NAME_MAX + 1];
  uint8_t o = 0;
  // Nur das, was in einem Netznamen nicht stoert. Leerzeichen werden zu
  // Unterstrichen, alles Uebrige faellt weg.
  for (const char* p = name; *p && o < AP_NAME_MAX; p++) {
    const char c = *p;
    if (isalnum((unsigned char)c))      clean[o++] = c;
    else if (c == ' ' || c == '-' || c == '_') clean[o++] = '_';
  }
  clean[o] = '\0';

  Preferences p;
  p.begin(NVS_NAMESPACE, false);
  if (o > 0) p.putString("apname", clean);
  else       p.remove("apname");
  p.end();
  return true;
}

void RemoteControl::begin() {
  buildSsid();

  WiFi.mode(WIFI_AP);
  // Sendeleistung bewusst reduziert: spart Strom und die Reichweite im
  // Zimmer ist ohnehin mehr als ausreichend.
  WiFi.softAP(_ssid, AP_PASSWORD, AP_CHANNEL, 0 /*nicht versteckt*/, 4 /*max 4 Clients*/);
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
      case WStype_CONNECTED:
        _clients++;
        _in.connected = true;
        _in.lastPacketMs = millis();
        // Der frischen Verbindung sofort sagen, wie der Roboter steht:
        // Tuningwerte und Belegung der Speicherplaetze.
        sendCfg(num);
        sendSlots(num);
        break;

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

      case WStype_TEXT:
        if (len == 0 || len > RC_LINE_MAX) break;
        memcpy(s_line, payload, len);
        s_line[len] = '\0';
        _in.lastPacketMs = millis();
        handleText(num, s_line);
        break;

      default:
        break;
    }
  });
}

// =====================================================================
//  Antworten an die App
// =====================================================================

// Nur was JSON zwingend braucht. UTF-8-Bytes ab 0x80 gehen unveraendert
// durch, das ist gueltiges JSON. Steuerzeichen fliegen raus.
void RemoteControl::jsonEscape(const char* in, char* out, size_t outLen) {
  size_t o = 0;
  for (const char* p = in; *p && o + 2 < outLen; p++) {
    const unsigned char c = (unsigned char)*p;
    if (c == '"' || c == '\\') { out[o++] = '\\'; out[o++] = (char)c; }
    else if (c >= 0x20)        { out[o++] = (char)c; }
  }
  out[o] = '\0';
}

void RemoteControl::sendCfg(uint8_t num) {
  char buf[256];
  char esc[AP_NAME_MAX * 2 + 2];
  jsonEscape(_apName, esc, sizeof(esc));
  snprintf(buf, sizeof(buf),
           "{\"t\":\"cfg\",\"kp\":%.3f,\"kd\":%.3f,\"base\":%.3f,\"min\":%.3f,"
           "\"ssid\":\"%s\",\"apname\":\"%s\"}",
           _lastTuning.kp, _lastTuning.kd, _lastTuning.base, _lastTuning.minLevel,
           _ssid, esc);
  _ws.sendTXT(num, buf);
}

// Namen und Belegung aller Speicherplaetze. Daraus baut die App die
// Slot-Auswahl samt "x von 48 Schritten".
void RemoteControl::sendSlots(int16_t num) {
  if (!_prog) return;
  size_t n = snprintf(s_out, sizeof(s_out),
                      "{\"t\":\"slots\",\"a\":%u,\"max\":%u,\"n\":[",
                      _prog->activeSlot(), Program::maxSteps());
  char esc[PROG_NAME_MAX * 2 + 2];
  for (uint8_t i = 0; i < PROG_SLOTS; i++) {
    jsonEscape(_prog->slotName(i), esc, sizeof(esc));
    n += snprintf(s_out + n, sizeof(s_out) - n, "%s\"%s\"", i ? "," : "", esc);
  }
  n += snprintf(s_out + n, sizeof(s_out) - n, "],\"c\":[");
  for (uint8_t i = 0; i < PROG_SLOTS; i++)
    n += snprintf(s_out + n, sizeof(s_out) - n, "%s%u", i ? "," : "", _prog->slotSteps(i));
  snprintf(s_out + n, sizeof(s_out) - n, "]}");

  if (num < 0) _ws.broadcastTXT(s_out);
  else         _ws.sendTXT((uint8_t)num, s_out);
}

void RemoteControl::broadcastSlots() { sendSlots(-1); }

void RemoteControl::sendProg(uint8_t num, uint8_t slot) {
  if (!_prog) return;
  char text[PROG_MAX_CHARS + 1];
  _prog->serialize(text, sizeof(text));
  char esc[PROG_NAME_MAX * 2 + 2];
  jsonEscape(_prog->slotName(slot), esc, sizeof(esc));
  snprintf(s_out, sizeof(s_out),
           "{\"t\":\"prog\",\"slot\":%u,\"name\":\"%s\",\"p\":\"%s\"}", slot, esc, text);
  _ws.sendTXT(num, s_out);
}

void RemoteControl::sendResult(uint8_t num, bool ok, const char* msg) {
  char esc[160];
  jsonEscape(msg, esc, sizeof(esc));
  char buf[200];
  snprintf(buf, sizeof(buf), "{\"t\":\"%s\",\"m\":\"%s\"}", ok ? "ok" : "err", esc);
  _ws.sendTXT(num, buf);
}

// =====================================================================
//  Befehle der App
// =====================================================================
void RemoteControl::handleText(uint8_t num, const char* line) {
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
        const int m = (int)(a + 0.5f);
        _req.haveMode = true;
        _req.mode = (m == 2) ? Mode::Program : (m == 1) ? Mode::Auto : Mode::Manual;
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
    case 'G': _req.zeroGyro  = true; break;

    case 'O':
      if (sscanf(line + 1, ",%f,%f", &a, &b) == 2) {
        _req.haveGuard = true;
        _req.guard   = (a >= 0.5f);
        _req.guardMm = (uint16_t)constrain((int)b, (int)DIST_MIN_MM, (int)DIST_MAX_MM);
      }
      break;
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

    // ---- Fahrprogramm starten / stoppen ----
    case 'B':
      if (sscanf(line + 1, ",%f", &a) == 1) {
        _req.haveRun = true;
        _req.run = (a >= 0.5f);
        // Starten schaltet die Betriebsart gleich mit um -- der Startknopf
        // ist ohnehin nur in der Programmansicht sichtbar.
        if (_req.run) { _req.haveMode = true; _req.mode = Mode::Program; }
      }
      break;

    // ---- Speicherplatz laden ----
    case 'L':
      if (_prog && sscanf(line + 1, ",%f", &a) == 1) {
        const uint8_t slot = (uint8_t)constrain((int)a, 0, PROG_SLOTS - 1);
        if (_prog->loadSlot(slot)) sendProg(num, slot);
        else                       sendResult(num, false, _prog->lastError());
      }
      break;

    // ---- Speicherplatz schreiben:  W,<slot>|<name>|<text> ----
    case 'W': {
      if (!_prog) break;
      const char* p1 = strchr(line, '|');
      if (!p1) { sendResult(num, false, "Befehl unvollstaendig"); break; }
      const char* p2 = strchr(p1 + 1, '|');
      if (!p2) { sendResult(num, false, "Befehl unvollstaendig"); break; }

      const int slotNo = atoi(line + 2);
      if (slotNo < 0 || slotNo >= PROG_SLOTS) { sendResult(num, false, "Ungueltiger Speicherplatz"); break; }

      char name[PROG_NAME_MAX + 1];
      size_t nl = (size_t)(p2 - p1 - 1);
      if (nl > PROG_NAME_MAX) nl = PROG_NAME_MAX;
      memcpy(name, p1 + 1, nl);
      name[nl] = '\0';

      if (_prog->saveSlot((uint8_t)slotNo, name, p2 + 1)) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Gespeichert: %u Schritte", _prog->count());
        sendResult(num, true, msg);
        broadcastSlots();
      } else {
        sendResult(num, false, _prog->lastError());
      }
      break;
    }

    // ---- Funktionstest starten / stoppen ----
    case 'S':
      if (sscanf(line + 1, ",%f", &a) == 1) {
        _req.haveTest = true;
        _req.test = (a >= 0.5f);
      }
      break;

    // ---- eigenen Netznamen setzen:  N|<name> ----
    case 'N': {
      const char* bar = strchr(line, '|');
      if (!bar) { sendResult(num, false, "Name fehlt"); break; }
      setApName(bar + 1);
      char msg[96];
      if (*(bar + 1)) snprintf(msg, sizeof(msg), "Netzname gespeichert -- nach dem Neustart aktiv");
      else            snprintf(msg, sizeof(msg), "Eigener Name geloescht -- wieder automatische Kennung");
      sendResult(num, true, msg);
      break;
    }

    // ---- Neustart ----
    case 'Z':
      sendResult(num, true, "Neustart...");
      delay(120);                 // der Meldung noch Zeit zum Rausgehen geben
      ESP.restart();
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
