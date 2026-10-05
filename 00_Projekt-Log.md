# Projekt-Log: One-of-a-Kind Mars Rover

Laufendes Protokoll. Neueste Einträge oben.

---

## 2026-10-05 (5) — Umbenannt, kompiliert, Web-Installer

**Projekt heißt jetzt „One-of-a-Kind Mars Rover".** „Bristlebot" bleibt in
der Technikdoku als Bezeichnung der Bauart stehen.

**Erfolgreich kompiliert — erstmals.**

```
RAM:   15.2 % (49.784 von 327.680 Bytes)
Flash: 69.8 % (914.437 von 1.310.720 Bytes)
0 Fehler, 0 Warnungen (mit -Wall)
```

Gebaut gegen `espressif32@6.9.0` (offizielle Plattform, Arduino-Core 2.x),
neue Umgebung `esp32dev_core2` in der `platformio.ini`. Damit ist auch der
Core-2.x-Pfad der Firmware bestätigt.

**Drei Hürden auf dem Weg dahin — alle in der Umgebung, keine im Code:**

1. **`uv` lehnt GitHub-Zertifikate ab** (`invalid peer certificate:
   UnknownIssuer`), während `curl` dieselbe Adresse mit HTTP 200 erreicht.
   Klassische Signatur eines TLS-aufbrechenden Virenscanners oder Proxys.
   Abhilfe: `UV_SYSTEM_CERTS=1` **und** `UV_NATIVE_TLS=1` (ältere
   uv-Versionen kennen nur die zweite). Die Plattform verschluckt den
   Fehler zusätzlich, weil sie die uv-Ausgabe nach DEVNULL wirft.
2. **Windows-Pfadgrenze**, exakt 260 Zeichen erreicht beim Entpacken von
   `esp32-arduino-libs`. `LongPathsEnabled` steht auf 0. Abhilfe ohne
   Registry-Eingriff: `PLATFORMIO_CACHE_DIR=C:\pioc`.
3. **Der pioarduino-Fork** (Plattform 55.03.37) scheitert reproduzierbar an
   seiner penv-Einrichtung, auch nachdem die Pakete manuell erfolgreich
   installiert wurden. Deshalb der Ausweichweg über die offizielle
   Plattform. Ungeklärt, aber umgangen.

**Neu: eindeutiger Netzname je Gerät.** Ab Werk `MarsRover_<MAC-Kennung>`,
damit mehrere Rover nebeneinander funktionieren, ohne dass jemand etwas
einstellt. Im Experten-Bereich lässt sich ein eigener Name vergeben
(`MarsRover_Petra`), gespeichert im NVS, aktiv nach einem Neustart, den die
Oberfläche gleich mit auslöst. Neue Protokollbefehle `N|<name>` und `Z`.

**Neu: Web-Installer.** `docs/` enthält eine Installationsseite mit ESP Web
Tools, `manifest.json` und die zusammengeführte Binärdatei
(`marsrover-esp32.bin`, 986 KB, Offset 0x0). GitHub Pages muss noch
eingeschaltet werden. Web Serial gibt es nur in Chrome, Edge und Opera auf
dem Desktop — nicht auf Handys, nicht in Firefox und Safari.

**Neu: `Doku/00_Projektbeschreibung.md`** — ausführliche Beschreibung des
Projekts und aller Funktionen, mit einem langen Kapitel zum Programmieren
(Zeitmodell, alle Befehle, Vorgehen, vier Beispiele vom Lichtspiel bis zum
selbsttätigen Ausweichen, typische Fehler, Grenzen).

**Nächste Sitzung:**

- [ ] GitHub Pages einschalten, Installationsseite prüfen
- [ ] Repo und lokalen Ordner auf `One-of-a-Kind-Mars-Rover` umbenennen
- [ ] 3D-Basismodul entwerfen — Anforderungen stehen in
      [Doku 00 §8](Doku/00_Projektbeschreibung.md). **Platine und Basismodul
      gehören zusammen entworfen**, der endgültige Platinenumriss ist bis
      dahin offen
- [ ] Bauanleitung mit Bildern, entsteht beim Bau des ersten Exemplars
- [ ] Teile bestellen, aufbauen, Funktionstest fahren

---

## 2026-10-05 (4) — Stromversorgung geprüft, LiPo bestätigt

**Frage:** Spart ein Halter mit Schalter für 3× AAA plus Buck-Boost Gewicht?

**Antwort: nein, er kostet ~36 g.** 3× AAA samt Halter wiegen ≈ 52 g gegen
≈ 14 g der LiPo-Lösung; die Gesamtmasse stiege von ~45 g auf ~85 g. Dazu
schlechtere Energiedichte (doppelte Energie für 3,7-fache Masse) und
0,9 Ω Innenwiderstand in Reihe, was bei den 1-A-Spitzen des ESP32 um 0,9 V
einbricht. Vollständige Rechnung in [Doku 02 §7](Doku/02_SMD-vs-THT-Wirtschaftlichkeit.md).

**Entscheidung des Auftraggebers:** LiPo bleibt wie geplant. Über den
**Buck-Boost direkt auf 3,3 V** (≈ 90 % statt ≈ 56 % Wirkungsgrad, rund ein
Drittel mehr Laufzeit, kein Mehrgewicht) wird erst entschieden, **wenn der
erste Aufbau fährt** — die Massefrage ist bislang gerechnet, nicht gemessen.

**Nicht erledigt, bewusst offen:**

- [ ] Buck-Boost auf 3,3 V nach dem ersten Fahrversuch bewerten. Haken dabei:
      Rückwärtseinspeisung in den AMS1117 des DevKits, USB und Akku dürfen
      dann nicht gleichzeitig anliegen
- [ ] Sicherheitsvorgabe für den Aufbau: geschützte Zelle, vollständig im
      Gehäuse, keine freien Kontakte, Laden nicht unbeaufsichtigt

---

---

## 2026-10-05 (3) — Lagesensor und Abstandssensor

**Frage des Auftraggebers:** Bringt ein MPU-6050 Genauigkeit? Und wäre ein
GY-511 (LSM303DLHC) auf einem Mast wie bei Curiosity besser?

**Antwort und Entscheidung**

- MPU-6050: **ja, für Drehungen — nein, für Strecken.** Zweifache Integration
  der Beschleunigung ist auf einem vibrationsgetriebenen Roboter binnen ein
  bis zwei Sekunden unbrauchbar.
- LSM303DLHC **abgelehnt**: hat kein Gyroskop, sondern Beschleunigung +
  Magnetometer. Für „drehe um 90°" braucht es die Drehrate. Der Mast wäre
  physikalisch richtig gedacht (Störfeld fällt mit 1/r³, 20 → 80 mm sind
  64× weniger), löst aber ein Problem, das bei Manövern von Sekunden gar
  nicht auftritt: die Gyro-Drift liegt dabei unter 1°.
- Stattdessen: **MPU-6050 flach aufs Chassis + VL53L0X auf den Mast.** Der
  ToF-Sensor verträgt die Mastschwingung, weil er über sein Messfenster
  mittelt — ein Magnetometer dagegen verlöre dort seine Ausrichtung, und
  die ist bei einem Kompass die Messgröße.

**Umgesetzte Funktionen** (vom Auftraggeber ausgewählt)

Lagesensor: Drehen nach Winkel · Geradeauslauf halten · Kipp- und
Aufheb-Erkennung. Kursgeführte Liniensuche wurde verworfen.
Abstandssensor: fahren bis Hindernis · drehen bis frei · warten bis frei,
dazu ein abschaltbarer Hindernis-Stopp als Schutzfunktion.

**Gebaut**

- `Imu.*` — MPU-6050 ohne Fremdbibliothek. DLPF auf 21 Hz und Messbereiche
  bewusst grob (±8 g), weil ein übersteuernder MEMS-Sensor unter Vibration
  einen Gleichanteil erzeugt, der wie echte Neigung aussieht
- `Distance.*` — VL53L0X über die Pololu-Bibliothek. Gelesen wird nur, wenn
  der Chip ein Ergebnis gemeldet hat, sonst würde der Lesebefehl bis zu
  33 ms in der Hauptschleife stehen
- vier Warte-Flags im Interpreter zu **einem** Zustand `Await`
  zusammengefasst, bevor die Zustandsmaschine unübersichtlich wurde
- Funktionstest um beide Sensoren erweitert — er dreht sich dabei selbst
  einmal nach rechts und einmal nach links und prüft damit auch die
  **Drehrichtung** des Kreisels. Ein um 180° verdreht montierter Sensor
  fällt sonst erst beim ersten Fahrprogramm auf und sieht dort wie ein
  Programmierfehler aus
- Urteile im Klartext unter dem Testknopf

**Beide Sensoren sind optional.** Fehlen sie, läuft alles weiter: Drehbefehle
fallen auf eine Zeitschätzung zurück, Abstandsbefehle enden nach ihrer
Zeitgrenze. Die Oberfläche sagt das auch.

**Massefolge:** +4,5 g auf ~45 g, davon 2 g oben auf dem Mast. Das verschärft
das Hauptrisiko aus Doku 02 §4. Die dortige Abspeckliste holt ~6,5 g zurück.

**Weiterhin ungetestet** — nichts davon lief je auf Hardware.

---

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
