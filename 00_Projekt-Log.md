# Projekt-Log: One-of-a-Kind Mars Rover

Laufendes Protokoll. Neueste Einträge oben.

## 2026-10-07 — Platine geroutet: 27 von 27 Netzen, null unverbundene Elemente

**Gemacht**

- Platzierung nachgezogen: J2 als **liegender** Stecker
  (`JST_PH_S2B-PH-K_1x02_P2.00mm_Horizontal`) auf die **Unterseite**, weil der
  Akku unter die Platine kommt und ein stehender Stecker in den 5-mm-Spalt
  ragen würde; OS1/OS2 auf optimalen Abstand; U6 hinten mittig; S1 daneben.
- **Geroutet** mit [KiCadRoutingTools](https://github.com/drandyhaas/KiCadRoutingTools)
  (A\*-Router mit Rust-Kern, MIT, headless auf der `.kicad_pcb`).
  Ergebnis: **27/27 Netze, 91/91 Padpaare, 101 Durchkontaktierungen,
  0 unverbundene Elemente, 0 Abstandsverstöße, 0 Kurzschlüsse.**
- Massefläche auf **B.Cu**, eine zusammenhängende Insel, 1,0 mΩ über 86 mm.

**Entscheidungen**

- **Sensorabstand OS1/OS2:** Die Fototransistoren zeigen nach innen und stehen
  12,5 mm auseinander. Maßgeblich ist aber der **optische Messfleck**, der
  jeweils mittig zwischen LED und Transistor liegt — die stehen damit
  **18 mm** auseinander und liegen im von [Doku/04](Doku/04_PCB-Layout-Empfehlung.md)
  geforderten Fenster 15–20 mm für ein 19-mm-Band.
- **Kein Ausschnitt für den MPU-6050.** Ein Gyroskop misst die Drehrate eines
  starren Körpers an jedem Punkt gleich; nur der Beschleunigungsmesser sieht
  Zentrifugalterme, und die betragen bei 180 °/s und 30 mm Abstand 0,03 g —
  zwei Größenordnungen unter der Motorvibration. Ein 22 × 17-mm-Loch mitten in
  einer 1 mm dünnen Platte hätte also Steifigkeit und Massefläche gekostet,
  ohne messbar etwas zu bringen. Damit ist Regel 1 aus Doku/04 §6 („nahe der
  Drehachse") die **schwächste** der drei; Regel 2 (weit weg von den Motoren)
  und Regel 3 (weich ankoppeln) bleiben.
- **Keine Durchkontaktierung in einem Pad.** Der erste Lauf setzte 16 Vias
  mitten in SMD-Pads. Das verlangt IPC-4761 Typ VII (gefüllt und überplattet);
  beim Handlöten saugt es sonst das Lot in die Hülse. Mit
  `--same-net-pad-clearance 0.2` sind es null.

**Zwei Fehlschläge, beide Platzierung statt Routing**

- R2 Pad 1 hatte links und rechts je **0,8 mm** Padabstand — genau die Breite,
  die eine 0,4-mm-Leitung mit zweimal 0,2 mm Abstand braucht, also null
  Spielraum. Daran scheiterten beide Router. R2 um 0,5 mm und Q2 um 0,6 mm
  nach außen hat es gelöst. *(Erster Versuch mit 1 mm war zu viel und legte
  R2 Pad 2 auf das Massepad von Q2 — vom DRC gefunden.)*
- J2 deckte in der ersten Stellung die Unterseiten-Pads von LED3 und R11 ab.
  Auf der Unterseite ist das kein Gehäuse-, sondern ein **Löt**problem.

**Abweichungen von der Breitenvorgabe, bewusst**

| Netz | Soll | Dünnste Stelle | Länge dort |
|---|---|---|---|
| +5V | 1,0 mm | 0,40 mm | 10,3 von 194 mm |
| +3V3 | 0,8 mm | 0,40 mm | 5,6 von 177 mm |
| GND (Zuleitungen) | 1,0 mm | 0,33 mm | 33 von 66 mm |
| Net-(D1-A) | 1,0 mm | 0,40 mm | 15,7 von 35 mm |

Das sind **Einschnürungen am Pad**: eine 1-mm-Leitung passt nicht in ein
0805-Pad von 1,2 mm Breite, wenn der Nachbar 0,8 mm entfernt ist. 0,4 mm auf
35 µm tragen nach IPC-2221 **1,23 A** bei 10 K Erwärmung; die Motoren ziehen
zusammen rund 0,3 A. +3V3 wurde bewusst mit 0,8 statt 1,0 mm angefordert — es
trägt nur die Sensorik (gut 40 mA), und 1 mm hätte Querungen gefressen, die
die Signale brauchen.

**Offen**

- Siebdruck: 58 Überlappungen und 32 zu kleine Texte. Rein kosmetisch, aber
  vor der Bestellung aufzuräumen.
- 21 Courtyard-Überlappungen und 4 THT-Pads im Courtyard: alle unter dem
  gesockelten DevKit, also gewollt.

---

## 2026-10-07 — MT3608 bekommt einen Festteiler statt des Potis

**Gemacht**

- [Doku/07](Doku/07_Inbetriebnahme-und-Tuning.md): Schritt 1 in **1a und 1b**
  geteilt. 1a beschreibt den Modulumbau, 1b nur noch das Nachmessen.
- [Doku/03](Doku/03_Stueckliste-BOM.md): U5 ist jetzt als Umbauteil
  gekennzeichnet, die zwei Teilerwiderstände stehen als referenzlose Position
  in der Liste (sie gehören aufs Modul, nicht auf die Platine). Einkaufsfalle 3
  umformuliert.
- [Doku/04](Doku/04_PCB-Layout-Empfehlung.md): Siebdruck-Hinweis angepasst —
  „SET 5V0 neben dem Trimmer" stimmt nicht mehr, der Trimmer ist weg.

**Entscheidung: Trimmpoti am MT3608 auslöten, 110 kΩ / 15 kΩ einsetzen**

Zwei Gründe, der zweite ist der wichtigere:

1. Das Poti ist das höchste Bauteil des Moduls und steht im Weg, sobald das
   Modul als senkrechte Finne montiert wird.
2. Ein Schleifkontakt auf einem Fahrzeug, dessen Antriebsprinzip Vibration
   ist, verstellt sich. Verstellt er sich nach oben, hängt das DevKit an
   mehr als 5 V.

Gerechnet über die 0,6-V-Referenz des MT3608:
0,6 V × (1 + 110 kΩ / 15 kΩ) = 0,6 × 8,333 = **5,00 V**.

**Fallstrick, der dokumentiert ist:** Der werkseitige untere
Rückkopplungswiderstand des Moduls muss raus. Bleibt er liegen, steht er
parallel zu den 15 kΩ und die Ausgangsspannung steigt.

**Geprüft**

- Schaltplan nach der Nutzer-Bearbeitung gegengerechnet: Netzliste Knoten für
  Knoten gegen den Vorstand verglichen, **keine Abweichung außer den von mir
  nachträglich eingefügten Entkoppelkondensatoren**. ERC: 0 Fehler,
  2 bekannte kosmetische Warnungen an D1/D2.

---

## 2026-10-06 — Schaltplan neu aufgebaut: Bereiche und gezeichnete Leitungen

**Gemacht**

- Schaltplan vollstaendig neu aufgebaut, Blatt jetzt **A2** statt A3:
  **175 gezeichnete Leitungen, 25 Knotenpunkte, null Netzlabels**
- Fuenf beschriftete Bereiche: Rechenkern · Motortreiber · Sensoren
  Linienerkennung · Sensoren Lage und Abstand · Signal-LEDs · Akku und
  Ladevorrichtung
- Neues Werkzeug `pcb/build_schematic.py`. Die acht Lagen (vier Drehungen,
  zwei Spiegelungen) sind am lebenden KiCad **ausgemessen**, nicht geraten;
  das Skript prueft am Ende selbst, ob jede Koordinate auf dem
  1,27-mm-Raster liegt.
- `pcb/gen_schematic.py` ist **gesperrt** — ein Lauf bricht jetzt mit einer
  Meldung ab, statt den Schaltplan mit der alten Fassung zu ueberschreiben.
  Die Datei bleibt als Beleg der Netzliste liegen.
- Geprueft: ERC **0 Fehler**, keine verwaisten Leitungen, keine
  Ueberlappungen, Netzliste knotengenau gegen den Entwurf abgeglichen.
- Zwei echte Zeichenfehler gefunden und behoben: Leitungen liefen quer
  durch die Gehaeuse von U2 und U5 hindurch.

**Entscheidungen**

- **Keine MOSFETs fuer die LEDs.** Nachgesehen im ESP32-WROOM-32-Datenblatt
  v3.8, Tabelle 14: I_OH typisch **40 mA** je Pin in der Domaene
  VDD3P3_RTC, bei mehreren gleichzeitig treibenden Pins noch rund 29 mA
  (Fussnote 2). Alle vier LED-Pins — GPIO 14, 25, 26, 27 — liegen in dieser
  Domaene. Vorwiderstand bleibt **220 Ohm**, rund 5 mA je LED.
  Das spart zwoelf Bauteile auf einer Platine, deren groesstes Risiko die
  Masse ist. Die 40 mA gelten allerdings nur bei maximaler Treiberstaerke;
  die Voreinstellung von Arduino und ESP-IDF ist Stufe 2 mit etwa 20 mA.
- **Versorgung ueber Power-Symbole**, nicht als durchgezogene Leitung.
  GND, +5V und +3V3 quer ueber ein A2-Blatt zu ziehen waere unleserlicher,
  nicht lesbarer. Je Netz ein PWR_FLAG an der Quelle.
- **Netznamen** sind jetzt automatisch vergeben (`Net-(D1-A)` statt
  `MOT_L_D`). Fuer das Platinenlayout waeren sprechende Namen nuetzlich —
  ein Label auf einer bereits gezeichneten Leitung *benennt* nur, es
  *verbindet* nicht. Offen, siehe unten.
- Verbleibende Kreuzungen sind gewollt und ohne Knotenpunkt, also
  elektrisch nicht verbunden: zwei am I2C-Bus (bei zwei Teilnehmern an
  einem Zweidrahtbus nicht vermeidbar), zwei im Akkubereich, weil BAT+
  zwischen zwei Massepins des Lademoduls liegt.

**Offen**

- [ ] Sprechende Netznamen? Nur benennend, die Verdrahtung bleibt gezeichnet
- [ ] Zwei ERC-Warnungen `lib_symbol_mismatch` an D1/D2 — kosmetisch, Folge
      des Aufloesens der Symbolvererbung (1N5819 erbt von SB120)

**Als Naechstes**

Review durch den Nutzer in KiCad, dann das Platinenlayout.

---

## 2026-10-05 (7) — KiCad-MCP-Server, Schaltplan wird neu aufgebaut

**Gemacht**

- Schaltplan-Entwurf durchgesehen und **verworfen**: Bauteile wild über das
  Blatt verteilt, Verbindungen ausschließlich über Netzlabels
- [KiCAD-MCP-Server](https://github.com/mixelpixx/KiCAD-MCP-Server) v2.8.2
  (MIT) nach `C:\Users\Frank-PC-AMD\Tools\KiCAD-MCP-Server` installiert und
  in `~/.claude.json` unter dem Projekt „HA Claude" eingetragen.
  Geprüft: 244 Werkzeuge, `pcbnew 10.0.0` startet sauber
- KiCads Sperrdatei `*.lck` aus der Versionierung genommen — war versehentlich
  eingecheckt. Ebenso ignoriert: `.history/`, `pcb/Bilder/`

**Entscheidungen**

- **Der Schaltplan wird in Bereiche gegliedert:** Rechenkern, Motortreiber,
  Sensorik, Akku und Ladevorrichtung. Innerhalb der Bereiche **gezeichnete
  Leitungen**, keine Netzlabels. Vorgabe des Nutzers, nicht verhandelbar.
- **Werkzeugwechsel:** `pcb/gen_schematic.py` wird **nicht mehr ausgeführt**.
  Es erzeugt die Datei komplett neu und überschreibt damit jede Handarbeit.
  Es bleibt als Beleg der Netzliste liegen. Künftig gezielte Einzeleingriffe
  über den MCP-Server — dadurch überleben manuelle Änderungen im
  Schaltplaneditor.
- **Regel für die Zusammenarbeit an der Datei: immer nur einer schreibt.**
  Vor jedem Schreibzugriff wird geprüft, ob eine `*.lck` im `pcb/`-Ordner
  liegt; dann hat KiCad die Datei offen und sie wird nicht angefasst.
  Ungespeicherte Änderungen im Editor sind von außen unsichtbar — erst
  speichern, dann übergeben.
- KiCad **10** bleibt. Kein Wechsel auf 11: das ist derzeit ein Nightly, das
  Dateiformat ist eine Einbahnstraße, und das Platinenlayout läuft auf 10
  bereits live über die IPC-Schnittstelle.
- Der Review-Screenshot bleibt **lokal**, kommt nicht ins Repo.

**Offen — zum Projektabschluss, vom Nutzer bestellt**

- [ ] **HowTo-Datei** mit allen benutzten Schnittstellen und ihrer
      Einrichtung, damit Nachbauer dieselbe Umgebung herstellen können.
      Mindestens: PlatformIO samt der drei nötigen Umgebungsvariablen
      (`UV_SYSTEM_CERTS`, `UV_NATIVE_TLS`, `PLATFORMIO_CACHE_DIR`), der
      KiCAD-MCP-Server samt Konfiguration, ESP Web Tools und GitHub Pages,
      die Datenblattquellen aus [Doku/10](Doku/10_Datenblaetter.md)
- [ ] **Fertiger Prompt für Claude**, der alle benötigten Werkzeuge und
      Schritte ermöglicht, um diese Arbeiten durchzuführen — mit
      ausführlicher Erklärung jedes einzelnen Schritts. So geschrieben,
      dass jemand mit leerer Claude-Sitzung und leerem Rechner bis zum
      fahrenden Roboter kommt und dabei versteht, *warum* jeder Schritt
      nötig ist. Gehört mit der HowTo-Datei zusammen gedacht: die eine
      beschreibt die Umgebung, der andere setzt sie in Gang.

**Als Nächstes**

Schaltplan neu aufbauen, Bereich für Bereich, mit gezeichneten Leitungen.
Danach ERC, SVG zum Durchsehen, dann das Platinenlayout.

---

## 2026-10-05 (6) — MOSFET statt Bipolartransistor, Datenblätter, KiCad-Start

**Frage:** Wäre ein MOSFET als Ersatz für den BC337 nicht sinnvoller?

**Antwort: ja — aber nicht wegen des Wirkungsgrads.** Der entscheidende
Punkt ist der Spannungsabfall. Der BC337 frisst 0,25 V von der 5-V-Schiene,
das sind rund **8 % der Motorspannung**, die nicht in Vibration umgesetzt
werden. Bei einem Roboter, dessen größtes Risiko „zu schwer, fährt
vielleicht nur kriechend" lautet, wirkt das direkt auf die Schwachstelle.

**Entschieden: AO3400A** (SOT-23, 28 mΩ bei 4,5 V Gate, 5,7 A, ~10 ct).
Verworfen: IRLZ44N in TO-220 — elektrisch top, aber 2 g je Stück bei einem
Roboter, bei dem um Gramm gekämpft wird.

Gewarnt und dokumentiert: **2N7000, BS170 und IRF540 taugen hier nicht.**
Ihr R_DS(on) ist für 10 V Gate-Spannung spezifiziert; an 3,3 V sind sie kaum
durchgesteuert. Klassischer Anfängerfehler, steht jetzt in Review B10.

Nachgezogen: Stückliste, Netzliste (Gate/Drain/Source statt Basis/Kollektor/
Emitter), Pinbelegung, Layoutempfehlung, Datenblattliste, README und das
Generator-Skript. Gate-Widerstand 100 Ω statt 1 kΩ, Pulldown 100 kΩ statt
10 kΩ. **Die Firmware ändert sich nicht.**

**Datenblätter.** `docs/` ist der von GitHub Pages veröffentlichte Ordner,
keine Dokumentenablage — das dort abgelegte AZ-Datenblatt ist nach
`Datasheets/` gewandert und per `.gitignore` aus dem Repo gehalten.
Stattdessen eine Linkliste ([Doku 10](Doku/10_Datenblaetter.md)): jeder kommt
an jedes Dokument, und zwar an die aktuelle Fassung beim Hersteller statt an
eine eingefrorene Kopie.

**KiCad.** Live-Verbindung zur laufenden Instanz steht (IPC-API, `kipy`).
Ordner `pcb/` mit eigener Bibliothek (TCRT5000-Symbol, von KiCad geprüft)
und der offiziellen Espressif-Bibliothek. Generator-Skript für den
Schaltplan angelegt, Stufe 1 (Rechenkern + Motorstufe) ist beschrieben.

**Nächster Schritt:** Generator laufen lassen, Format gegen ERC prüfen, dann
die restlichen rund 30 Bauteile ergänzen.

---

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
