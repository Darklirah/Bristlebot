#include "Program.h"

// NVS-Schlüssel: Text "pt0".."pt3", Name "pn0".."pn3", aktiver Slot "pact"
static void keyText(char* k, uint8_t slot) { snprintf(k, 8, "pt%u", slot); }
static void keyName(char* k, uint8_t slot) { snprintf(k, 8, "pn%u", slot); }

// =====================================================================
//  Start
// =====================================================================
void Program::begin() {
  loadSlotMeta();
  installDemoIfEmpty();
  _nvs.begin(NVS_NAMESPACE, true);
  _active = _nvs.getUChar("pact", 0);
  _nvs.end();
  if (_active >= PROG_SLOTS) _active = 0;
  loadSlot(_active);
}

// Namen und Schrittzahlen aller Slots einlesen -- die App zeigt damit
// an, was in welchem Platz liegt, ohne jeden Slot laden zu müssen.
void Program::loadSlotMeta() {
  char kt[8], kn[8];
  char buf[PROG_MAX_CHARS + 1];

  _nvs.begin(NVS_NAMESPACE, true);
  for (uint8_t s = 0; s < PROG_SLOTS; s++) {
    keyName(kn, s);
    String n = _nvs.getString(kn, "");
    if (n.length() == 0) snprintf(_names[s], sizeof(_names[s]), "Programm %u", s + 1);
    else                 snprintf(_names[s], sizeof(_names[s]), "%s", n.c_str());

    keyText(kt, s);
    size_t len = _nvs.getString(kt, buf, sizeof(buf));
    _slotSteps[s] = (len > 1) ? countSteps(buf) : 0;
  }
  _nvs.end();
}

// Sind alle vier Plätze leer, ist das ein fabrikneues Gerät. Dann wandert
// ein lauffähiges Beispiel in Platz 1 -- so gibt es sofort etwas zum
// Starten und zum Umbauen, statt einer leeren Liste.
//
//   Rücklichter an, geradeaus 1,5 s
//   Rechtsbogen mit Blinker 2,5 s, Linksbogen mit Blinker 2,5 s
//   geradeaus 1,5 s, anhalten, Lichter aus, 1 s Pause, von vorn
//
//  Das "wa,1000" ganz am Ende ist kein Zierrat: ohne eine Wartezeit
//  nach dem Anhalten springt die Wiederholung sofort weiter und die
//  Pause wäre nicht zu sehen.
void Program::installDemoIfEmpty() {
  for (uint8_t s = 0; s < PROG_SLOTS; s++)
    if (_slotSteps[s] > 0) return;

  static const char DEMO[] =
    "ld,5,1;ge,55;wa,1500;"
    "re,4,55;ld,1,2;wa,2500;ld,1,0;"
    "li,4,55;ld,0,2;wa,2500;ld,0,0;"
    "ge,55;wa,1500;st;ld,6,0;wa,1000;lo,0";

  if (!parse(DEMO)) return;          // darf nicht passieren, aber still bleiben

  _nvs.begin(NVS_NAMESPACE, false);
  _nvs.putString("pt0", DEMO);
  _nvs.putString("pn0", "Beispiel: Achter");
  _nvs.putUChar("pact", 0);
  _nvs.end();

  _slotSteps[0] = _count;
  snprintf(_names[0], sizeof(_names[0]), "Beispiel: Achter");
}

const char* Program::slotName(uint8_t slot) const {
  return slot < PROG_SLOTS ? _names[slot] : "";
}

uint8_t Program::slotSteps(uint8_t slot) const {
  return slot < PROG_SLOTS ? _slotSteps[slot] : 0;
}

// Schritte zählen, ohne zu prüfen: nur die nichtleeren ';'-Abschnitte.
uint8_t Program::countSteps(const char* text) {
  uint8_t n = 0;
  bool inStep = false;
  for (const char* p = text; *p; p++) {
    if (*p == ';' || *p == ' ') { inStep = false; continue; }
    if (!inStep) { inStep = true; if (n < 255) n++; }
  }
  return n;
}

// =====================================================================
//  Slots
// =====================================================================
bool Program::loadSlot(uint8_t slot) {
  if (slot >= PROG_SLOTS) { setError("Ungültiger Speicherplatz"); return false; }

  char kt[8];
  char buf[PROG_MAX_CHARS + 1];
  keyText(kt, slot);

  _nvs.begin(NVS_NAMESPACE, true);
  const size_t len = _nvs.getString(kt, buf, sizeof(buf));
  _nvs.end();

  _active = slot;
  if (len <= 1) { _count = 0; _err[0] = 0; return true; }   // leerer Slot ist kein Fehler
  return parse(buf);
}

bool Program::saveSlot(uint8_t slot, const char* name, const char* text) {
  if (slot >= PROG_SLOTS) { setError("Ungültiger Speicherplatz"); return false; }
  if (_running)           { setError("Programm läuft -- erst anhalten"); return false; }
  if (strlen(text) > PROG_MAX_CHARS) { setError("Programm zu lang"); return false; }

  // Erst prüfen, dann speichern. Ein ungültiges Programm landet nie im Flash.
  if (!parse(text)) return false;

  // Kanonische Form schreiben, nicht den Rohtext der App
  char canon[PROG_MAX_CHARS + 1];
  serialize(canon, sizeof(canon));

  char kt[8], kn[8];
  keyText(kt, slot);
  keyName(kn, slot);

  char safeName[PROG_NAME_MAX + 1];
  snprintf(safeName, sizeof(safeName), "%s", (name && *name) ? name : "");

  _nvs.begin(NVS_NAMESPACE, false);
  _nvs.putString(kt, canon);
  if (*safeName) _nvs.putString(kn, safeName);
  _nvs.putUChar("pact", slot);
  _nvs.end();

  _active = slot;
  _slotSteps[slot] = _count;
  if (*safeName) snprintf(_names[slot], sizeof(_names[slot]), "%s", safeName);
  return true;
}

// =====================================================================
//  Prüfen
// =====================================================================
void Program::setError(const char* msg, int step) {
  if (step >= 0) snprintf(_err, sizeof(_err), "Schritt %d: %s", step + 1, msg);
  else           snprintf(_err, sizeof(_err), "%s", msg);
}

bool Program::parse(const char* text) {
  _count  = 0;
  _err[0] = 0;
  bool hasWait = false;
  bool hasLoop = false;

  const char* p = text;
  while (*p) {
    while (*p == ';' || *p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
    if (!*p) break;

    if (_count >= PROG_MAX_STEPS) {
      snprintf(_err, sizeof(_err), "Mehr als %u Schritte", PROG_MAX_STEPS);
      return false;
    }
    if (hasLoop) {
      setError("Nach 'wiederhole' darf kein Befehl mehr kommen", _count);
      return false;
    }

    // Opcode einlesen (zwei Buchstaben)
    char op[4] = {0};
    uint8_t i = 0;
    while (*p && *p != ',' && *p != ';' && i < 3) op[i++] = *p++;
    op[i] = '\0';

    // bis zu zwei Zahlenargumente
    long arg[2] = {0, 0};
    uint8_t nargs = 0;
    while (*p == ',' && nargs < 2) {
      p++;
      char* end = nullptr;
      arg[nargs] = strtol(p, &end, 10);
      if (end == p) { setError("Zahl erwartet", _count); return false; }
      p = end;
      nargs++;
    }

    Step& s = _steps[_count];
    s.a = 0; s.b = 0; s.c = 0;

    if (!strcmp(op, "ge")) {
      if (nargs < 1 || arg[0] < 1 || arg[0] > 100) { setError("Geschwindigkeit muss 1..100 sein", _count); return false; }
      s.op = (uint8_t)Op::Straight; s.a = (uint8_t)arg[0];
    }
    else if (!strcmp(op, "li") || !strcmp(op, "re")) {
      if (nargs < 2)                                   { setError("Kurve braucht Radius und Geschwindigkeit", _count); return false; }
      if (arg[0] < 1 || arg[0] > PROG_RADIUS_LEVELS)   { setError("Radius muss 1..10 sein", _count); return false; }
      if (arg[1] < 1 || arg[1] > 100)                  { setError("Geschwindigkeit muss 1..100 sein", _count); return false; }
      s.op = (uint8_t)(op[0] == 'l' ? Op::Left : Op::Right);
      s.a  = (uint8_t)arg[0];
      s.b  = (uint8_t)arg[1];
    }
    else if (!strcmp(op, "st")) { s.op = (uint8_t)Op::Stop; }
    else if (!strcmp(op, "wa")) {
      if (nargs < 1 || arg[0] < PROG_WAIT_MIN_MS || arg[0] > PROG_WAIT_MAX_MS) { setError("Wartezeit muss 0,05 .. 60 s sein", _count); return false; }
      s.op = (uint8_t)Op::Wait; s.c = (uint16_t)arg[0];
      hasWait = true;
    }
    else if (!strcmp(op, "ld")) {
      if (nargs < 2 || arg[0] < 0 || arg[0] >= LT_COUNT || arg[1] < 0 || arg[1] > 2) { setError("LED-Befehl unvollständig", _count); return false; }
      s.op = (uint8_t)Op::Led; s.a = (uint8_t)arg[0]; s.b = (uint8_t)arg[1];
    }
    else if (!strcmp(op, "bf")) {
      if (nargs < 1 || arg[0] < 1 || arg[0] > 10) { setError("Blinkfrequenz muss Stufe 1..10 sein", _count); return false; }
      s.op = (uint8_t)Op::BlinkFreq; s.a = (uint8_t)arg[0];
    }
    else if (!strcmp(op, "lo")) {
      if (nargs < 1 || arg[0] < 0 || arg[0] > 10) { setError("Wiederholungen müssen 0..10 sein", _count); return false; }
      s.op = (uint8_t)Op::Loop; s.a = (uint8_t)arg[0];
      hasLoop = true;
    }
    else {
      setError("Unbekannter Befehl", _count);
      return false;
    }

    _count++;
  }

  // Ein wiederholtes Programm ohne Wartezeit würde die Hauptschleife
  // mit Volldampf im Kreis laufen lassen.
  if (hasLoop && !hasWait) {
    setError("Ein wiederholtes Programm braucht mindestens ein 'warte'");
    _count = 0;
    return false;
  }
  return true;
}

void Program::serialize(char* out, size_t outLen) const {
  size_t n = 0;
  out[0] = '\0';
  for (uint8_t i = 0; i < _count; i++) {
    const Step& s = _steps[i];
    char frag[24];
    switch ((Op)s.op) {
      case Op::Straight:  snprintf(frag, sizeof(frag), "ge,%u", s.a);         break;
      case Op::Left:      snprintf(frag, sizeof(frag), "li,%u,%u", s.a, s.b); break;
      case Op::Right:     snprintf(frag, sizeof(frag), "re,%u,%u", s.a, s.b); break;
      case Op::Stop:      snprintf(frag, sizeof(frag), "st");                 break;
      case Op::Wait:      snprintf(frag, sizeof(frag), "wa,%u", s.c);         break;
      case Op::Led:       snprintf(frag, sizeof(frag), "ld,%u,%u", s.a, s.b); break;
      case Op::BlinkFreq: snprintf(frag, sizeof(frag), "bf,%u", s.a);         break;
      case Op::Loop:      snprintf(frag, sizeof(frag), "lo,%u", s.a);         break;
      default: continue;
    }
    const size_t fl = strlen(frag);
    if (n + fl + (n ? 1 : 0) + 1 >= outLen) break;
    if (n) out[n++] = ';';
    memcpy(out + n, frag, fl);
    n += fl;
    out[n] = '\0';
  }
}

// =====================================================================
//  Ablauf
// =====================================================================

// Stufe 1 = engste Kurve (Lenkanteil 1,0, dreht fast auf der Stelle),
// Stufe 10 = weite Kurve (Lenkanteil 0,1).
float Program::radiusToSteer(uint8_t radius) {
  if (radius < 1) radius = 1;
  if (radius > PROG_RADIUS_LEVELS) radius = PROG_RADIUS_LEVELS;
  return (float)(PROG_RADIUS_LEVELS + 1 - radius) / (float)PROG_RADIUS_LEVELS;
}

void Program::resetOutputs() {
  _moving = false;
  _speed  = 0.5f;
  _steer  = 0.0f;
  _blinkLevel = BLINK_LEVEL_DEFAULT;
  for (uint8_t i = 0; i < LED_COUNT; i++) _led[i] = LedState::Off;
}

void Program::start() {
  if (_count == 0) { setError("Dieser Speicherplatz ist leer"); return; }
  resetOutputs();
  _pc        = 0;
  _pass      = 0;
  _waitUntil = 0;
  _done      = false;
  _running   = true;
}

void Program::stop() {
  _running = false;
  _moving  = false;
  _waitUntil = 0;
}

void Program::applyLedTarget(uint8_t target, LedState st) {
  switch (target) {
    case LT_FL:    _led[LED_FL] = st; break;
    case LT_FR:    _led[LED_FR] = st; break;
    case LT_RL:    _led[LED_RL] = st; break;
    case LT_RR:    _led[LED_RR] = st; break;
    case LT_FRONT: _led[LED_FL] = _led[LED_FR] = st; break;
    case LT_REAR:  _led[LED_RL] = _led[LED_RR] = st; break;
    case LT_ALL:   for (uint8_t i = 0; i < LED_COUNT; i++) _led[i] = st; break;
    default: break;
  }
}

void Program::update() {
  if (!_running) return;

  if (_waitUntil) {
    if ((int32_t)(millis() - _waitUntil) < 0) return;   // noch am Warten
    _waitUntil = 0;
  }

  // Sofortbefehle am Stück abarbeiten, aber mit Budget: ein Programm
  // darf die Hauptschleife nie länger als nötig aufhalten.
  for (uint8_t budget = PROG_INSTANT_BUDGET; budget > 0; budget--) {

    if (_pc >= _count) {            // Ende ohne 'wiederhole'
      _running = false;
      _done    = true;
      _moving  = false;
      return;
    }

    const Step s = _steps[_pc];
    _pc++;

    switch ((Op)s.op) {
      case Op::Straight:
        _steer = 0.0f;  _speed = s.a / 100.0f;  _moving = true;  break;
      case Op::Left:
        _steer = -radiusToSteer(s.a);  _speed = s.b / 100.0f;  _moving = true;  break;
      case Op::Right:
        _steer =  radiusToSteer(s.a);  _speed = s.b / 100.0f;  _moving = true;  break;
      case Op::Stop:      _moving = false;                          break;
      case Op::Led:       applyLedTarget(s.a, (LedState)s.b);       break;
      case Op::BlinkFreq: _blinkLevel = s.a;                        break;

      case Op::Wait:
        _waitUntil = millis() + s.c;
        if (_waitUntil == 0) _waitUntil = 1;   // 0 bedeutet "wartet nicht"
        return;

      case Op::Loop:
        if (s.a == 0 || _pass < s.a) {         // 0 = endlos, sonst N weitere Läufe
          _pass++;
          _pc = 0;
        } else {
          _running = false;
          _done    = true;
          _moving  = false;
          return;
        }
        break;

      default: break;
    }
  }
  // Budget aufgebraucht -- der nächste loop()-Durchlauf macht hier weiter.
}
