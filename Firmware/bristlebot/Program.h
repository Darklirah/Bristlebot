// =====================================================================
//  Bristlebot -- Program.h
//  Fahrprogramme: Speicherung in vier Flash-Slots, Prüfung, Ablauf.
//
//  ZEITMODELL
//  Alle Befehle wirken sofort und gehen zur nächsten Zeile weiter --
//  Zeit verbraucht ausschliesslich "warte". Damit lassen sich Fahrt und
//  Lichter frei kombinieren:
//
//      LED vorne links blinken
//      geradeaus mit 60 %
//      warte 2,0 s
//      Kurve rechts, Stufe 3, mit 45 %
//      warte 1,5 s
//      anhalten
//      wiederhole endlos
//
//  Jeder Fahrbefehl trägt seine Geschwindigkeit selbst -- es gibt keinen
//  getrennten Tempo-Befehl.
//
//  TEXTFORMAT  (so liegt es im Flash und so geht es über den WebSocket;
//  die Web-App zeigt dem Benutzer stattdessen Klartext)
//
//      ge,<1..100>              geradeaus mit Geschwindigkeit in Prozent
//      li,<1..10>,<1..100>      Kurve links:  Radius, Geschwindigkeit
//      re,<1..10>,<1..100>      Kurve rechts: Radius, Geschwindigkeit
//                               Radius 1 = engste Kurve, 10 = weite Kurve
//      st                       anhalten
//      dl,<grad>,<1..100>       drehe links  um <grad> (5..360), geregelt
//      dr,<grad>,<1..100>       drehe rechts um <grad>, geregelt
//      wa,<ms>                  warten, 50 .. 60000 ms
//      ld,<ziel>,<zust>         LED: ziel 0..6 (LT_*), zust 0..2 (LedState)
//      bf,<1..10>               Blinkfrequenz, Stufe 1 = 0,5 Hz, 10 = 5 Hz
//      lo,<0..10>               wiederholen; 0 = endlos, N = N weitere Läufe
//
//  Mit Abstandssensor auf dem Mast:
//      fh,<mm>,<1..100>         fahre geradeaus, bis etwas naeher als <mm> ist
//      tl,<mm>,<1..100>         drehe links,  bis der Weg weiter als <mm> frei ist
//      tr,<mm>,<1..100>         drehe rechts, dto.
//      wf,<mm>                  stehe still,  bis der Weg weiter als <mm> frei ist
//
//  Schritte werden mit ';' getrennt. "lo" darf nur als letzter Schritt
//  stehen und ein wiederholtes Programm muss mindestens einen Befehl
//  enthalten, der Zeit verbraucht -- sonst wuerde es die Hauptschleife
//  blockieren. Das sind "wa", "dl", "dr", "fh", "tl", "tr" und "wf".
// =====================================================================
#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "Config.h"
#include "RobotState.h"

enum class Op : uint8_t {
  Straight = 0, Left, Right, Stop, Wait, Led, BlinkFreq, Loop,
  TurnLeft, TurnRight, DriveUntil, TurnClearLeft, TurnClearRight, WaitClear
};

struct Step {
  uint8_t  op;   // Op
  uint8_t  a;    // Geschwindigkeit / Radius / LED-Ziel / Stufe / Wiederholungen
  uint8_t  b;    // Geschwindigkeit bei Kurven, LED-Zustand
  uint16_t c;    // Wartezeit in ms, oder Drehwinkel in Grad
};

class Program {
public:
  void begin();                       // Slotnamen + Schrittzahlen laden, aktiven Slot öffnen

  // --- Slots ---
  bool        loadSlot(uint8_t slot);               // in den Arbeitsspeicher holen
  bool        saveSlot(uint8_t slot, const char* name, const char* text);
  uint8_t     activeSlot() const { return _active; }
  const char* slotName(uint8_t slot) const;
  uint8_t     slotSteps(uint8_t slot) const;        // belegte Schritte im Flash
  static uint8_t maxSteps() { return PROG_MAX_STEPS; }

  // --- Prüfen und Serialisieren ---
  bool        parse(const char* text);              // füllt _steps, false bei Fehler
  const char* lastError() const { return _err; }
  void        serialize(char* out, size_t outLen) const;

  // --- Ablauf ---
  void start();
  void stop();
  void update();                       // in jedem loop() aufrufen
  bool running() const { return _running; }
  bool finished() const { return _done; }

  // --- gewünschte Ausgänge, die die Hauptschleife abholt ---
  bool     moving()    const { return _moving; }
  float    speed()     const { return _speed; }     // 0 .. 1
  float    steer()     const { return _steer; }     // -1 .. +1
  LedState led(uint8_t i) const { return i < LED_COUNT ? _led[i] : LedState::Off; }
  uint8_t  blinkLevel() const { return _blinkLevel; }

  // --- Anzeige in der App ---
  uint8_t  pc()    const { return _pc; }
  uint8_t  count() const { return _count; }
  uint8_t  pass()  const { return _pass; }

  // --- Befehle, die auf einen Sensor warten ---
  //
  // Der Interpreter kennt weder Kurs noch Abstand. Er meldet nur an, was
  // ansteht; ausgefuehrt und beendet wird es in der Hauptschleife, die
  // die Sensoren hat. Zurueck kommt reportAwaitDone().
  //
  // Diese Befehle und "warte" sind die einzigen, die Zeit verbrauchen.
  enum class Await : uint8_t {
    None = 0,
    Turn,        // drehe um turnDegrees()
    Obstacle,    // fahre geradeaus, bis naeher als awaitMm()
    TurnClear,   // drehe in Richtung awaitDir(), bis weiter als awaitMm()
    WaitClear    // stehe still, bis weiter als awaitMm()
  };

  Await    awaiting()     const { return _await; }
  int16_t  turnDegrees()  const { return _turnDeg; }   // + = rechts
  uint16_t awaitMm()      const { return _awaitMm; }
  float    awaitSpeed()   const { return _awaitSpeed; }
  int8_t   awaitDir()     const { return _awaitDir; }  // -1 links, +1 rechts
  void     reportAwaitDone() { _await = Await::None; }

  // Zaehlt bei jedem Befehl hoch, der die Fahrt aendert. Die
  // Kursregelung erkennt daran, wann sie ihren Sollkurs neu setzen muss.
  uint8_t  motionSeq() const { return _motionSeq; }

private:
  static float radiusToSteer(uint8_t radius);
  void  setError(const char* msg, int step = -1);
  void  applyLedTarget(uint8_t target, LedState st);
  void  resetOutputs();
  void  loadSlotMeta();
  void  installDemoIfEmpty();         // Beispielprogramm beim allerersten Start
  static uint8_t countSteps(const char* text);

  Preferences _nvs;

  Step    _steps[PROG_MAX_STEPS];
  uint8_t _count  = 0;
  uint8_t _active = 0;

  char    _names[PROG_SLOTS][PROG_NAME_MAX + 1] = {{0}};
  uint8_t _slotSteps[PROG_SLOTS] = {0};

  char    _err[64] = {0};

  // Laufzeit
  bool     _running   = false;
  bool     _done      = false;
  uint8_t  _pc        = 0;
  uint8_t  _pass      = 0;
  uint32_t _waitUntil = 0;

  // Ausgänge
  bool     _moving = false;
  float    _speed  = 0.5f;
  float    _steer  = 0.0f;
  LedState _led[LED_COUNT] = { LedState::Off, LedState::Off,
                               LedState::Off, LedState::Off };
  uint8_t  _blinkLevel = BLINK_LEVEL_DEFAULT;

  Await    _await      = Await::None;
  int16_t  _turnDeg    = 0;
  uint16_t _awaitMm    = 100;
  float    _awaitSpeed = 0.5f;
  int8_t   _awaitDir   = 1;
  uint8_t  _motionSeq  = 0;
};
