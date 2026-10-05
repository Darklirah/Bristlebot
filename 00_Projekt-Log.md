# Projekt-Log: Bristlebot

Laufendes Protokoll. Neueste Einträge oben.

---

## 2026-10-05 (2) — Fahrprogramm und Funktionstest

**Entscheidungen (vom Auftraggeber bestätigt)**

| Frage | Entscheidung |
|---|---|
| Zeitmodell | eigener `warte`-Befehl; Fahrbefehle setzen nur den Zustand |
| Kurvenradius | Stufe 1 = engste Kurve, Stufe 10 = weite Kurve |
| Speicherung | vier Slots im ESP32-Flash, je 48 Schritte |
| Editor | Baukasten mit Auswahlfeldern, kein Texteditor |
| Geschwindigkeit | **am Fahrbefehl**, kein eigener Tempo-Befehl (Korrektur im Nachgang) |
| Oberfläche | einfach bedienbar, geeignet ab 14 Jahren |

**Gebaut**

- `Program.*` — Parser mit Klartext-Fehlermeldungen, vier NVS-Slots,
  Interpreter mit Schrittbudget gegen Blockieren der Hauptschleife
- `SelfTest.*` — eingebauter Funktionstest: LEDs einzeln, alle blinkend,
  Motor links und rechts als Rampe. Bewusst **kein** Fahrprogramm: einzelne
  Motoren und Rampen lassen sich mit der Programmsprache nicht ausdrücken,
  und ein Diagnosewerkzeug soll keinen Speicherplatz belegen
- `Leds.*` um Override erweitert; Sicherheitsanzeigen behalten Vorrang
- dritter Modus in `bristlebot.ino`, neue Protokollbefehle `B`, `L`, `W`, `S`
- Oberfläche: drei Betriebsarten, Baukasten-Editor mit Schrittzähler je
  Slot, Farbcodierung, grüne Markierung des laufenden Schritts,
  Experten-Bereich zugeklappt
- Beispielprogramm „Achter" wird beim ersten Start automatisch angelegt
- Doku 08 neu, 06 und 07 nachgezogen

**Methodik**

Die Oberfläche wurde vor dem Festschreiben mit einem Daten-Stub im Browser
gerendert und am Bildschirm abgenommen (`.preview/`, gitignoriert).

**Weiterhin ungetestet** — nichts davon lief je auf Hardware.

---

## 2026-10-05 — Entwurf geprüft, Firmware und Doku gebaut

**Entscheidungen (vom Auftraggeber bestätigt)**

| Frage | Entscheidung | Verworfene Alternativen |
|---|---|---|
| Fernsteuerung | WLAN-AP + eigene Web-App | BLE + fertige App (Dabble/Bluefruit); beides parallel |
| Liniensensorik | 2× TCRT5000 analog | 1× Fototransistor; 5-Kanal-Array |
| ESP32-Bauform | DevKit 30-Pin THT, gesteckt | ESP32-WROOM-32E SMD; ESP32-C3-MINI-1 SMD |

**Geprüft und geändert** (Details: [Doku/01](Doku/01_Schaltplan-Review.md))

Fünf Blocker im Ausgangsentwurf:

1. `BluetoothSerial` ist klassisches SPP und auf iOS ohne MFi-Lizenz nicht
   nutzbar → WLAN-AP
2. dem Fototransistor fehlte die beleuchtende IR-LED → TCRT5000
3. ein einzelner Sensor kann nicht differentiell lenken → zwei Sensoren
4. GPIO 12 ist Strapping-Pin MTDI und kann das Booten verhindern → Motoren auf
   GPIO 32/33
5. USB-C ohne 2× 5,1 kΩ an CC1/CC2 bekommt keine 5 V → nachgetragen

Dazu neun Verbesserungen: Basis-Pulldowns gegen Motorzucken beim Booten,
Schottky statt 1N4148, zusätzliche Entkopplung, R_prog an die Zellgröße,
Akku 500 statt 150 mAh, Schalterposition hinter dem Ladepfad, Duty-Begrenzung
für 3-V-Motoren an der 5-V-Schiene.

**Gebaut**

- Firmware, modular: `Config.h`, `RobotState.h`, `Motors.*`, `LineSensor.*`,
  `Leds.*`, `RemoteControl.*`, `WebUI.h`, `bristlebot.ino`
- Web-App komplett inline im Flash (kein CDN — im AP-Betrieb gibt es kein
  Internet), Captive Portal, Totmannschalter, Live-Tuning, Telemetriebalken
- Doku 01–07 plus Netzliste als KiCad-Vorlage
- `platformio.ini` als Alternative zur Arduino IDE

**Bewusst offengelassen**

- Motoren hängen wie vom Auftraggeber entworfen an der **5-V-Schiene**. Die
  bessere Variante (an VBAT, entlastet den knappen MT3608 um 1 W und hält die
  Motorspitzen aus der ESP32-Versorgung) ist dokumentiert und in `Config.h` eine
  Zeile — aber nicht eigenmächtig umgesetzt, weil es den Schaltplan ändert.
- Der 56-%-Wirkungsgrad des Pfads MT3608 → AMS1117 bleibt. Abhilfe wäre ein
  3,3-V-Buck-Boost, der aber das Auslöten des bordeigenen Reglers erfordert.

**Größtes Risiko**

Masse ≈ 40 g. Funktionierende Bristlebots liegen bei 10–25 g. Es wird fahren,
aber kriechen, und die Lenkautorität ist bei hoher Masse gering. Abspeckliste
in [Doku/02 §4](Doku/02_SMD-vs-THT-Wirtschaftlichkeit.md); sofort umsetzbar
und kostenlos: PCB in 1,0 mm statt 1,6 mm bestellen, LEDs 3 mm statt 5 mm.

**Nichts davon ist in Hardware getestet.**

**Nächste Schritte**

- [ ] Teile bestellen (Stückliste [Doku/03](Doku/03_Stueckliste-BOM.md))
- [ ] Firmware einmal probeweise kompilieren, damit die Toolchain-Seite geklärt ist
- [ ] Schaltplan und Layout in KiCad zeichnen, Vorlage: [Netzliste](Hardware/Netzliste.md)
- [ ] Entscheidung Motoren an VBAT statt 5 V
- [ ] Inbetriebnahme nach [Doku/07](Doku/07_Inbetriebnahme-und-Tuning.md),
      Schritt für Schritt — nicht alles auf einmal einschalten
- [ ] Bewährte Kp/Kd/Grundvibration in `Config.h` zurückschreiben

---

## 2026-10-05 — Projekt angelegt

**Gemacht**

- Verzeichnis `Bristlebot/` im Ordner „HA Claude" erstellt
- Grundgerüst: `README.md`, `00_Projekt-Log.md`, `.gitignore`, `LICENSE` (GPL-3.0)
- Öffentliches GitHub-Repo `Darklirah/Bristlebot` angelegt und ersten Commit gepusht

**Entscheidungen**

- Sichtbarkeit: **öffentlich**, wie die anderen Projekte
- Lizenz: **GPL-3.0**
- Struktur: zunächst nur Grundgerüst, Projektart offen gelassen
