# Schaltplan-Review

Prüfung deines Entwurfs vom 05.10.2026 auf Zusammenspiel der Komponenten.
Reihenfolge: zuerst die Punkte, die so **nicht funktionieren**, dann die, die
funktionieren, aber Leistung oder Lebensdauer kosten.

---

## A — Blocker: funktioniert so nicht

### A1 · BluetoothSerial läuft nicht auf dem iPhone

`BluetoothSerial` im ESP32-Core ist klassisches Bluetooth **SPP** (Serial Port
Profile). Apple gibt SPP auf iOS nicht für fremde Geräte frei; dafür bräuchtest
du eine MFi-Lizenz und einen Apple-Authentifizierungs-Chip im Gerät. Auf Android
funktioniert SPP, auf iOS grundsätzlich nicht.

**Entschieden:** eigener WLAN-Access-Point auf dem ESP32, der die
Bedienoberfläche selbst ausliefert. Läuft auf iPhone und Android identisch im
Browser, ohne Installation. Details in [06_Fernsteuerung-App.md](06_Fernsteuerung-App.md).

### A2 · Der Fototransistor hat keine Lichtquelle

Ein Fototransistor misst Licht, er erzeugt keines. Zur Linienerkennung brauchst
du Auflicht, das von hellem Untergrund reflektiert und von der schwarzen Linie
geschluckt wird. Im Entwurf fehlt die beleuchtende IR-LED komplett — der Sensor
würde nur Raumlicht messen und bei jedem Schatten kippen.

**Entschieden:** 2× **TCRT5000**. Das ist IR-LED und Fototransistor in einem
Gehäuse mit optischer Trennwand, genau für diesen Zweck. Kostet zusammen
etwa 1 €.

> **Achtung beim Einkauf:** Es gibt den nackten TCRT5000 (4 Beine) und
> *TCRT5000-Module* mit LM393-Komparator und Potentiometer. Die Module liefern
> nur ein Digitalsignal. Für den Analogwert, auf dem der PD-Regler arbeitet,
> brauchst du den **nackten Sensor**.

### A3 · Ein Sensor kann nicht differentiell lenken

Mit einem einzigen Sensor weiß der Roboter nur „Linie da / Linie weg", nicht
„Linie liegt links / rechts". Das zwingt zu Bang-Bang-Pendeln: immer abwechselnd
nach links und rechts drehen, bis die Linie wieder auftaucht. Sichtbares
Schlingern, und in Kurven verliert er die Linie.

**Entschieden:** zwei Sensoren links und rechts der Linie. Die Differenz der
beiden Analogwerte ist direkt der Regelfehler für den PD-Regler.

### A4 · GPIO 12 kann das Modul am Booten hindern

GPIO 12 ist beim ESP32 der Strapping-Pin **MTDI**. Beim Reset liest der ROM-
Bootloader diesen Pin und legt damit die Flash-Betriebsspannung fest: LOW = 3,3 V,
HIGH = 1,8 V. Eine Transistorbasis mit 1 kΩ gegen den Pin ist je nach Restzustand
ein Pfad nach oben — zieht etwas den Pin beim Reset hoch, stellt der Chip die
Flash-Spannung auf 1,8 V und bootet nicht mehr.

**Entschieden:** Motoren auf **GPIO 32 und 33**. Beide ohne Strapping-Funktion,
ohne Boot-Glitch, voll PWM-fähig.

### A5 · USB-C ohne CC-Widerstände bekommt keine 5 V

Eine USB-C-Buchse am TP4056 bleibt stromlos, wenn die Pins CC1 und CC2 offen
sind. Ein USB-C-Netzteil oder -Kabel erkennt daran, dass überhaupt eine Senke
angeschlossen ist.

**Nachzutragen:** **2× 5,1 kΩ**, je einer von CC1 und CC2 nach GND. Nicht einer
für beide gemeinsam — das wird von manchen Quellen als Audio-Adapter
fehlinterpretiert.

---

## B — Läuft, kostet aber Leistung oder Lebensdauer

### B1 · Die Spannungsaufbereitung verheizt fast die Hälfte

Der Weg LiPo → MT3608 auf 5 V → Vin → bordeigener AMS1117 auf 3,3 V hat zwei
Verluststufen hintereinander:

| Stufe | Wirkungsgrad |
|---|---|
| MT3608 Step-Up 3,7 V → 5 V | ≈ 85 % |
| AMS1117 linear 5 V → 3,3 V | 3,3/5 = **66 %** |
| **Gesamt** | **≈ 56 %** |

Der AMS1117 ist ein Linearregler: er verbrät die Differenz als Wärme. Bei
500 mA Spitzenstrom sind das 0,85 W Abwärme in einem SOT-223.

**Besser wäre** ein Buck-Boost direkt auf 3,3 V (TPS63020 o. ä., ≈ 90 %) und
Einspeisung am 3V3-Pin. Das setzt aber voraus, dass der AMS1117 des DevKits nicht
mitspielt — man müsste ihn auslöten. **Für den Prototyp bleibt es bei deinem
Weg**, der Mehrverbrauch ist dokumentiert und wird in der Laufzeitrechnung
([02](02_SMD-vs-THT-Wirtschaftlichkeit.md)) berücksichtigt.

### B2 · Der MT3608 ist knapp bemessen — und lebensgefährlich für das DevKit

**Lastrechnung der 5-V-Schiene im Spitzenfall:**

| Verbraucher | Leistung an 5 V |
|---|---|
| ESP32 über AMS1117, 500 mA Sendespitze | 2,5 W |
| 2 Vibrationsmotoren | ≈ 1,0 W |
| **Summe** | **≈ 3,5 W → 700 mA bei 5 V** |

Der MT3608 wird mit „2 A" bewerben, liefert bei 3,7 V Eingang und 5 V Ausgang
real etwa 600–800 mA. Du liegst damit **am Anschlag**.

> ### ⚠️ Der wichtigste Satz in diesem Dokument
> Die MT3608-Module sind **einstellbar** und kommen mit beliebiger
> Ausgangsspannung aus der Fabrik — oft 20 V und mehr. Stelle das Modul
> **vor dem ersten Anschluss** des ESP32 im Leerlauf mit dem Multimeter auf
> **5,00 V** ein. Andernfalls ist das DevKit beim Einschalten sofort tot.
> Als Absicherung kann eine **5,6-V-Zenerdiode** (BZX55C5V6) parallel zur
> 5-V-Schiene liegen; sie begrenzt einen Fehlfall, statt ihn ans Board
> durchzulassen.

**Empfohlene Revision (ein Draht und ein Zahlenwert):** Die Motoren direkt an
**VBAT** betreiben, also hinter dem Schalter, vor dem Step-Up. Vorteile:

* 1 W weniger Last auf dem knappen Step-Up
* die Stromspitzen der Motoren laufen nicht mehr durch die Versorgung des ESP32
* 3-V-Motoren sehen 3,0–4,2 V statt 5 V, also weniger Überspannung

In der Firmware ist das **eine Zeile**: in `Config.h` `MOTOR_SUPPLY_V` von `5.0f`
auf `4.2f` setzen. Die Duty-Begrenzung rechnet sich daraus selbst neu.

### B3 · 3-V-Motoren an 5 V

Handelsübliche Vibrationsmotoren (10-mm-Münzmotor, 4×12-mm-Zylinder) sind
**3 V**-Typen. Dauerbetrieb an 5 V bedeutet 67 % Überspannung: der Motor wird
heiß, die Bürsten verschleißen schnell, die Lebensdauer bricht ein.

**In der Firmware gelöst:** `MOTOR_SUPPLY_V` und `MOTOR_RATED_V` in `Config.h`
begrenzen den maximalen PWM-Duty automatisch:

```
MOTOR_DUTY_MAX = 1023 × 3,0 V / 5,0 V = 613   (von 1023)
```

Der Motor sieht also im Mittel nie mehr als seine 3 V. Wer 5-V-Motoren kauft,
setzt `MOTOR_RATED_V` auf `5.0f` und hat den vollen Bereich.

### B4 · Der Schalter sperrt auch das Laden

Trennt der Schiebeschalter den Akku **komplett** ab (zwischen Zelle und
TP4056-BAT), kann im ausgeschalteten Zustand nicht geladen werden — und der
TP4056 sieht eine offene Zelle.

**Richtige Position:** Schalter **zwischen TP4056-OUT (bzw. dem Ausgang der
DW01A-Schutzschaltung) und dem Eingang des Step-Up**. Dann ist der Roboter aus,
der Ladepfad bleibt aber intakt und du kannst ihn im ausgeschalteten Zustand
laden. Das ist der Normalfall.

### B5 · Freilaufdiode: 1N4148 reicht, Schottky ist besser

Der Motorstrom liegt bei 80–110 mA, die 1N4148 kann 200 mA Dauerstrom — passt
also. Eine **1N5819** (Schottky) hat aber nur ≈ 0,3 V Vorwärtsspannung statt
0,7 V und schaltet deutlich schneller. Bei 20 kHz PWM heißt das weniger
Verlustwärme im Transistor und weniger Störspitzen auf der Versorgung, die
sonst im ADC landen.

**Empfehlung:** 1N5819. Die 1N4148 ist eine zulässige Notlösung.

### B6 · Fehlender Basis-Pulldown — Motorzucken beim Booten

Nach dem Reset sind GPIO 32/33 Eingänge, also hochohmig und floatend. Der
Transistor kann in dieser Phase teilweise leiten und der Motor zuckt.

**Nachzutragen:** je **10 kΩ von Basis nach GND** (parallel zur
Basis-Emitter-Strecke). Der Stromverlust ist mit 75 µA bedeutungslos, der
Transistor ist aber definiert gesperrt, solange der GPIO nicht treibt.

### B7 · Nur ein Elko ist zu wenig Entkopplung

100 µF/16 V fängt den niederfrequenten Einbruch ab, ist aber für die steilen
Schaltflanken bei 20 kHz zu langsam (Eigeninduktivität).

**Nachzutragen:** je **100 nF Keramik direkt an jedem Transistor** (Kollektor
nach GND) und **10 µF Keramik** an der 5-V-Schiene. Kostet Cent-Beträge und
hält den ADC-Messwert sauber.

### B8 · Akku 150 mAh ist zu klein

Der ESP32 zieht im WLAN-Sendemoment bis zu 500 mA. Bei 150 mAh entspricht das
**3,3 C** nur für den Funk; mit Motoren kommt man auf über 7 C. Die Zellspannung
bricht ein, der Brownout-Detektor löst aus, der ESP32 bootet neu.

**Empfehlung: 400–500 mAh.** Rechnung und Gewichtsabwägung in
[02_SMD-vs-THT-Wirtschaftlichkeit.md](02_SMD-vs-THT-Wirtschaftlichkeit.md).

### B9 · TP4056: Ladestrom muss zur Zelle passen

Die üblichen TP4056-Module laden mit **1 A** (R_prog = 1,2 kΩ). Für eine 500-mAh-
Zelle sind das 2 C — weit über den zulässigen 0,5–1 C. Der Ladestrom wird über
R_prog gesetzt:

```
I_lade [A] = 1200 / R_prog [Ω]
```

| Zelle | Ziel (0,5 C) | R_prog | tatsächlich |
|---|---|---|---|
| 300 mAh | 150 mA | 8,2 kΩ | 146 mA |
| 400 mAh | 200 mA | 6,2 kΩ | 194 mA |
| **500 mAh** | **250 mA** | **4,7 kΩ** | **255 mA** |
| 1000 mAh | 500 mA | 2,4 kΩ | 500 mA |

### B10 · Logik-MOSFET statt Bipolartransistor

Nachgetragen am 05.10.2026. Der BC337-40 war richtig dimensioniert, aber ein
**AO3400A** ist an dieser Stelle in jeder Hinsicht besser.

| | BC337-40 | AO3400A |
|---|---|---|
| Ansteuerung | **2,55 mA Basisstrom**, dauernd während „ein" | praktisch 0 |
| Spannungsabfall bei 100 mA | 0,20–0,30 V (V_CE,sat) | 3 mV (28 mΩ) |
| Verlustleistung je Motor | ≈ 25 mW | ≈ 1 mW |

**Der Spannungsabfall ist der eigentliche Grund, nicht der Wirkungsgrad.**
Die Motoren laufen mit begrenztem Duty an der 5-V-Schiene; der
Bipolartransistor frisst davon 0,25 V — rund **8 % der Motorspannung**, die
nicht in Vibration umgesetzt werden. Bei einem Roboter, dessen größtes
Risiko „zu schwer, fährt vielleicht nur kriechend" lautet, ist das keine
Nebensache, sondern wirkt direkt auf die Schwachstelle.

Nebenbei entfallen 2 × 2,55 mA Belastung der GPIOs. Das Datenblatt des
DevKits nennt 15 mA je Pin — kritisch war es nicht, geschenkt ist es
trotzdem.

> **Es muss ein Logic-Level-Typ sein.** Bei **2N7000, BS170 und IRF540** ist
> R_DS(on) für 10 V Gate-Spannung spezifiziert. An 3,3 V sind sie kaum
> durchgesteuert, werden heiß und begrenzen den Strom — ein klassischer
> Anfängerfehler. Beim AO3400A gilt der Wert bei 4,5 V, bei 2,5 V sind es
> noch ≈ 45 mΩ.

**Beschaltung:** Gate-Widerstand **100 Ω** statt 1 kΩ Basiswiderstand
(dämpft die Flanke bei 20 kHz), Gate-Pulldown **100 kΩ** statt 10 kΩ. Die
Teilezahl bleibt gleich, die Firmware ändert sich nicht.

Einziger Nachteil: SOT-23 statt TO-92. Von Hand lötbar, und 0805-Widerstände
sowie die USB-C-Buchse sind in der Stückliste ohnehin schon SMD.

---

## C — Was unverändert bleibt, weil es passt

* ~~**Transistorstufe BC337-40 als Low-Side-Schalter.**~~ **Ersetzt am
  05.10.2026 durch einen Logik-MOSFET AO3400A** — siehe B10 unten. Die
  BC337-Dimensionierung war korrekt (I_B = 2,55 mA, Verhältnis 39 bei
  h_FE ≥ 250, also sicher in Sättigung), aber ein MOSFET ist hier schlicht
  die bessere Wahl.
* **GPIO 34 als Analogeingang.** Richtig gewählt: GPIO 34 liegt auf **ADC1**, und
  nur ADC1 ist bei aktivem WLAN nutzbar — ADC2 wird vom WLAN-Treiber belegt. Der
  zweite Sensor geht entsprechend auf GPIO 35 (ebenfalls ADC1).
* **Externer 10-kΩ-Pulldown am Sensorausgang.** Zwingend nötig, weil GPIO 34/35
  reine Eingänge ohne interne Pull-Widerstände sind. Gut erkannt.
* **TP4056 + DW01A.** Richtige Paarung: der TP4056 lädt, der DW01A schützt vor
  Tiefentladung, Überladung und Kurzschluss. Der DW01A braucht noch einen
  **FS8205A** Doppel-MOSFET als Schaltelement — in den Fertigmodulen ist er drin,
  beim eigenen PCB musst du ihn einplanen.
* **Schiebeschalter SPDT, THT.** Passt, siehe B4 zur Einbauposition.
* **LEDs auf GPIO 14/27/26/25 mit 220 Ω.** Übernommen wie von dir vorgegeben.
  Kleine Einschränkung: GPIO 14 gibt beim Booten kurz ein Taktsignal aus, die
  gelbe LED links flackert deshalb beim Einschalten einmal auf. Rein kosmetisch.
  Strom pro LED: (3,3 − 2,0) / 220 ≈ 6 mA bei gelb, bei rot (3,3 − 1,9) / 220
  ≈ 6,4 mA — unkritisch für die GPIOs (40 mA Grenze).

---

## Was sich gegenüber deinem Entwurf konkret ändert

| Punkt | Dein Entwurf | Jetzt |
|---|---|---|
| Funk | BluetoothSerial | WLAN-AP + eigene Web-App |
| Liniensensor | 1× Fototransistor | 2× TCRT5000 |
| Motor links | GPIO 12 | **GPIO 32** |
| Motor rechts | GPIO 13 | **GPIO 33** |
| Sensor | GPIO 34 | GPIO 34 **und 35** |
| LEDs | 14 / 27 / 26 / 25 | unverändert |
| USB-C | — | **+ 2× 5,1 kΩ an CC1/CC2** |
| Transistorbasis | 1 kΩ | **+ 10 kΩ Pulldown nach GND** |
| Freilaufdiode | 1N4148 | **1N5819** (1N4148 zulässig) |
| Entkopplung | 100 µF Elko | **+ 2× 100 nF, + 10 µF** |
| TP4056 | Modul wie gekauft | **R_prog auf Zellgröße anpassen** |
| Akku | 150–500 mAh | **400–500 mAh** |
| Motorschiene | 5 V | 5 V (**Revision auf VBAT empfohlen**, B2) |
