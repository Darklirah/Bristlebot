# Stückliste (BOM)

Für **Variante A**: ESP32 DevKit V1 (30-Pin) gesteckt, 2× TCRT5000,
WLAN-Fernsteuerung. Preise sind Richtwerte in Euro für Einzelstück,
europäische Distributoren, Stand Oktober 2026, ohne Versand.

Bauteilkürzel (`U1`, `R5` …) entsprechen der
[Netzliste](../Hardware/Netzliste.md) und der [Pinbelegung](05_Pinbelegung.md).

---

## 1 · Rechenkern

| Pos | Bauteil | Menge | Bauform | € | Bemerkung |
|---|---|---|---|---|---|
| U1 | ESP32 DevKit V1, **30-Pin** | 1 | THT, gesteckt | 5,50 | Nicht die 36-Pin-Version — anderes Rastermaß und andere Pinreihenfolge |
| — | Buchsenleiste 15-pol, RM 2,54, gerade | 2 | THT | 0,60 | U1 steckbar halten, nicht einlöten |

> Das DevKit bringt USB-Buchse, CP2102/CH340 und den 3,3-V-Regler AMS1117 selbst
> mit. Deshalb fehlen in dieser Stückliste USB-UART-Brücke und Boot-Taster.

---

## 2 · Laden und Spannungsversorgung

### 2a · Variante „diskret auf eigenem PCB"

| Pos | Bauteil | Menge | Bauform | € | Bemerkung |
|---|---|---|---|---|---|
| U2 | TP4056 Lade-IC | 1 | SOP-8 | 0,45 | 1-zelliger LiPo-Lader, 4,2 V |
| U3 | DW01A Schutz-IC | 1 | SOT-23-6 | 0,35 | Tiefentladung, Überladung, Kurzschluss |
| U4 | FS8205A Doppel-MOSFET | 1 | SOT-23-6 | 0,45 | **Schaltelement für U3 — ohne das schützt U3 nichts** |
| J1 | USB-C-Buchse, 16-pol, nur Power | 1 | SMD | 0,45 | Mit Durchsteck-Haltelaschen wählen |
| R13, R14 | 5,1 kΩ, 1 % | 2 | 0805 | 0,02 | **Je einer von CC1 und CC2 nach GND. Pflicht, sonst keine 5 V** |
| R15 | **4,7 kΩ** (R_prog) | 1 | 0805 | 0,01 | Setzt 255 mA Ladestrom für eine 500-mAh-Zelle. Tabelle: [Review B9](01_Schaltplan-Review.md) |
| — | 10 µF + 100 nF an U2 | je 1 | 0805 | 0,05 | Ein- und Ausgangsentkopplung des Laders |
| — | LED 0805 rot + grün, je 1 kΩ | 2 | 0805 | 0,10 | Ladezustandsanzeige CHRG / STDBY, optional |

### 2b · Variante „Fertigmodul" — empfohlen für den Erstaufbau

| Pos | Bauteil | Menge | € | Bemerkung |
|---|---|---|---|---|
| U2–U4 | **TP4056-Ladeplatine mit USB-C und Schutzschaltung** | 1 | 1,50 | Enthält TP4056, DW01A, FS8205A, USB-C-Buchse und die CC-Widerstände |

> Beim Fertigmodul musst du trotzdem **R_prog tauschen**: die Module laden mit
> 1 A (1,2 kΩ, oft als `R3` bedruckt). Den 1,2-kΩ-Widerstand ablöten und 4,7 kΩ
> einsetzen. Sonst lädst du eine 500-mAh-Zelle mit 2 C.

### 2c · Schiene und Schalter

| Pos | Bauteil | Menge | Bauform | € | Bemerkung |
|---|---|---|---|---|---|
| U5 | **MT3608 Step-Up-Modul**, einstellbar | 1 | THT-Modul | 1,20 | **Vor dem Anschluss des ESP32 im Leerlauf auf 5,00 V trimmen** |
| S1 | Schiebeschalter SPDT, THT | 1 | THT | 0,45 | Zwischen Akku-Schutzausgang und Eingang von U5 — nicht direkt an die Zelle |
| C1 | 100 µF / 16 V Elektrolyt | 1 | THT, Ø 6,3 mm | 0,18 | Nahe den Motoren an der 5-V-Schiene |
| C2 | 10 µF Keramik / 16 V | 1 | 0805 | 0,12 | An der 5-V-Schiene, ergänzt C1 für schnelle Flanken |
| D3 | Zenerdiode BZX55C5V6 | 1 | THT | 0,08 | *Optional, aber empfohlen:* Notbremse gegen einen falsch eingestellten U5 |
| BT1 | LiPo 3,7 V, **500 mAh**, JST-PH 2.0 | 1 | — | 7,50 | Bauform 503035. 150 mAh ist zu klein, siehe [Review B8](01_Schaltplan-Review.md) |
| J2 | JST-PH-2.0-Buchse, 2-pol, THT | 1 | THT | 0,25 | Polung zweimal prüfen — die Zellen sind nicht genormt belegt |

---

## 3 · Motortreiber

| Pos | Bauteil | Menge | Bauform | € | Bemerkung |
|---|---|---|---|---|---|
| M1, M2 | Vibrationsmotor 3 V, Ø 10 mm Münze | 2 | — | 2,40 | ≈ 80 mA bei 3 V. Alternative: Zylinder 6 × 14 mm, kräftiger, aber je 2 g schwerer |
| Q1, Q2 | **AO3400A** Logik-MOSFET | 2 | SOT-23 | 0,20 | Low-Side-Schalter, 28 mΩ bei 4,5 V Gate, 5,7 A. **Muss ein Logic-Level-Typ sein** — 2N7000, BS170 und IRF540 sind bei 3,3 V Gate praktisch zu |
| R1, R2 | 100 Ω | 2 | 0805 | 0,02 | Gate-Widerstand, dämpft die Schaltflanke |
| R3, R4 | **100 kΩ** | 2 | 0805 | 0,02 | **Gate-Pulldown nach GND — verhindert Motorzucken beim Booten** |
| D1, D2 | **1N5819** Schottky | 2 | THT DO-41 | 0,16 | Freilauf, antiparallel zum Motor. 1N4148 wäre zulässig, aber schlechter |
| C3, C4 | 100 nF Keramik | 2 | 0805 | 0,04 | Direkt am Drain jedes MOSFETs nach GND |

---

## 4 · Liniensensorik

| Pos | Bauteil | Menge | Bauform | € | Bemerkung |
|---|---|---|---|---|---|
| OS1, OS2 | **TCRT5000** Reflexkoppler | 2 | THT, 4-pol | 0,90 | **Nackter Sensor, nicht das Modul mit LM393.** Der Regler braucht den Analogwert |
| R5, R6 | **150 Ω** | 2 | THT 1/4 W | 0,04 | Vorwiderstand der IR-LED: (3,3 − 1,25) / 150 = 13,7 mA |
| R7, R8 | 10 kΩ | 2 | THT 1/4 W | 0,04 | Emitter-Pulldown des Fototransistors. **Pflicht** — GPIO 34/35 haben keine internen Pull-Widerstände |

> **Arbeitspunkt einstellen:** Steht der Messwert über hellem Untergrund dauernd
> bei ~4095 (gesättigt), R7/R8 auf 4,7 kΩ verkleinern. Bleibt der Hub zwischen
> Linie und Untergrund unter ~150 Zählern, auf 22 kΩ vergrößern. Der Istwert ist
> in der Web-App als Balken live sichtbar.

---

## 5 · Signal-LEDs

| Pos | Bauteil | Menge | Bauform | € | Bemerkung |
|---|---|---|---|---|---|
| LED1, LED2 | LED gelb, 3 mm | 2 | THT | 0,20 | Vorne links/rechts, zeigen die Lenkbewegung. **3 mm statt 5 mm spart 0,8 g** |
| LED3, LED4 | LED rot, 3 mm | 2 | THT | 0,20 | Rücklichter, im Fahrbetrieb dauerhaft an |
| R9–R12 | 220 Ω | 4 | THT 1/4 W | 0,08 | ≈ 6 mA pro LED, unkritisch für die GPIOs |

---

## 6 · Platine und Mechanik

| Pos | Bauteil | Menge | € | Bemerkung |
|---|---|---|---|---|
| PCB1 | Leiterplatte 2-lagig, 50 × 38 mm, **1,0 mm FR4** | 1 | 1,40 | JLCPCB 5er-Charge ≈ 7 € gesamt. 1,0 mm statt 1,6 mm spart 2,2 g bei gleichem Preis |
| — | Zahnbürstenkopf / Borstenfeld | 2 | 3,00 | **Zwei getrennte Felder, je eines unter einem Motor** — Begründung in [02 §4](02_SMD-vs-THT-Wirtschaftlichkeit.md) |
| — | Doppelseitiges Schaumklebeband | 1 | 0,50 | Akku und Bürstenköpfe befestigen; entkoppelt auch die Vibration |

---

## 7 · Optionale Erweiterungen

| Pos | Bauteil | Menge | € | Aktivierung |
|---|---|---|---|---|
| R16, R17 | 100 kΩ, 1 % | 2 | 0,02 | Spannungsteiler VBAT → GPIO 39. `FEATURE_BATTERY_MONITOR 1` in `Config.h` |
| — | Zusätzlich 100 nF an GPIO 39 nach GND | 1 | 0,02 | Glättet den Teiler gegen Motorstörungen |
| SW2 | Taster, THT | 1 | 0,15 | Betriebsart-Umschalter an GPIO 4. `FEATURE_MODE_BUTTON 1` |

---

## Kostenübersicht

| | Einzelstück |
|---|---|
| Rechenkern | 6,10 € |
| Versorgung mit **Fertigmodul** (2b) + Schiene | 11,08 € |
| Motortreiber | 2,92 € |
| Sensorik | 0,98 € |
| LEDs | 0,48 € |
| PCB + Mechanik | 4,90 € |
| **Summe** | **≈ 26,50 €** |
| davon Akku | 7,50 € |

Mit der diskreten Ladeschaltung (2a) statt des Fertigmoduls wird es etwa 0,40 €
teurer, dafür flacher und 1 g leichter — aber es kommen vier SMD-Bauteile
und eine USB-C-Buchse dazu, die gelötet werden müssen.

Bei 10 Stück liegt der Materialpreis bei etwa **21 € pro Einheit**, bei 100 Stück
bei etwa **17 €** — in beiden Fällen dominiert der Akku den Preis.

---

## Einkaufsfallen, kurz zusammengefasst

1. **TCRT5000 nackt kaufen**, nicht als Modul mit Komparator.
2. **ESP32 DevKit mit 30 Pins**, nicht 36.
3. **MT3608 auf 5,00 V trimmen**, bevor der ESP32 dran kommt.
4. **R_prog am TP4056 tauschen** — Werkszustand 1 A ist für diese Zelle zu viel.
5. **USB-C braucht 2× 5,1 kΩ** an CC1 und CC2 nach GND.
6. **JST-PH-Polung prüfen.** LiPo-Konfektionierungen sind nicht genormt; falsch
   gesteckt zerstört es den Lader sofort.

---

## 8 · Sensorik am I²C-Bus (Nachtrag)

Beide Sensoren hängen am selben Bus (GPIO 21/22), die Adressen kollidieren
nicht: MPU-6050 auf `0x68`, VL53L0X auf `0x29`.

| Pos | Bauteil | Menge | € | Bemerkung |
|---|---|---|---|---|
| U6 | **GY-521** (MPU-6050) Breakout | 1 | 2,50 | Lagesensor. Pull-ups sind auf dem Modul. **Flach aufs Chassis**, nahe der Drehachse, möglichst weit weg von den Motoren |
| U7 | **GY-530 / VL53L0X** Breakout | 1 | 3,50 | Abstandssensor, Laufzeitmessung. Kommt auf den Mast |
| — | Mast: CFK-Rundstab Ø 3 mm, ca. 60 mm | 1 | 0,80 | Steif und leicht. Messing- oder Alurohr geht auch, wiegt aber mehr |
| — | Litze 0,14 mm², vieradrig, ca. 120 mm | 1 | 0,30 | Zum Sensor hinauf. **Verdrillt** und mit Schlaufe am Fuß, siehe unten |
| — | Schaumklebeband für U6 | 1 | — | aus dem Restbestand der Akkubefestigung |

**Zusammen ≈ 7,10 €**, Materialpreis steigt damit von 26,50 € auf **≈ 33,60 €**.

### Massezuwachs

| Position | Masse |
|---|---|
| GY-521 (Stiftleisten abgeknipst, flach verlötet) | 1,8 g |
| GY-530 auf dem Mast | 1,3 g |
| Mast CFK 3 mm × 60 mm | 0,6 g |
| Litze und Klebstoff | 0,8 g |
| **Summe** | **≈ 4,5 g** |

Damit steigt die Gesamtmasse von ~40 g auf **~45 g** — und davon sitzen 2 g
oben auf dem Mast. Das verschärft das in
[02 §4](02_SMD-vs-THT-Wirtschaftlichkeit.md) beschriebene Hauptrisiko.
Gegenrechnung: die dort genannte Abspeckliste (1,0-mm-PCB, 3-mm-LEDs,
300-mAh-Zelle) holt ~6,5 g zurück.

### Einkaufsfallen

1. **GY-521 mit abgeknipsten Stiftleisten verbauen.** Gesteckt wackelt das
   Modul im Vibrationsbetrieb, und genau dieses Wackeln misst der Sensor dann.
2. **VL53L0X, nicht VL53L1X.** Der L1X hat eine andere Bibliothek und eine
   andere Registerkarte. Module werden oft verwechselt.
3. **Beide Module vertragen 3,3 V.** Die üblichen Breakouts haben einen
   eigenen Regler und laufen auch an 5 V — am 3V3-Pin des DevKits sind sie
   aber sparsamer und störungsärmer.
