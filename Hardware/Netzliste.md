# Netzliste

Vollständige Verbindungsliste als Vorlage für die Schaltplaneingabe in KiCad.
Bauteilkürzel wie in der [Stückliste](../Doku/03_Stueckliste-BOM.md).

---

## Netze

### `VBAT` — Akkuspannung, 3,0 … 4,2 V

| Von | Nach |
|---|---|
| BT1 (+) über J2 Pin 1 | U2 `BAT+` |
| U2/U3-Schutzausgang `OUT+` | S1 Pin 2 (Mitte) |
| S1 Pin 1 | U5 `VIN+` |

### `+5V` — Ausgang des Step-Up

| Von | Nach |
|---|---|
| U5 `VOUT+` | U1 `VIN` |
| U5 `VOUT+` | C1 (+), C2 |
| U5 `VOUT+` | D3 Kathode *(optional, Zener 5,6 V)* |
| U5 `VOUT+` | M1 Anschluss A |
| U5 `VOUT+` | M2 Anschluss A |

> Revision laut [Review B2](../Doku/01_Schaltplan-Review.md): M1/M2 stattdessen
> an `VBAT` anschließen und in `Config.h` `MOTOR_SUPPLY_V = 4.2f` setzen.

### `+3V3` — bordeigener Regler des DevKits

| Von | Nach |
|---|---|
| U1 `3V3` | R5, R6 (Vorwiderstände der IR-LEDs) |
| U1 `3V3` | OS1 Kollektor, OS2 Kollektor |

### `GND`

Sternpunkt am Minuspol des Akkus bzw. am GND von U5. Drei getrennte Abgänge,
Begründung in der [Layout-Empfehlung §2](../Doku/04_PCB-Layout-Empfehlung.md):

| Abgang | Angeschlossen |
|---|---|
| **Leistung** | Emitter Q1, Emitter Q2, C1 (−), C2, C3, C4 |
| **Logik** | U1 `GND` (mindestens 2 Pins), U2/U3/U4 GND, U5 GND, S1-Gehäuse |
| **Sensorik** | R7, R8, OS1 Anode, OS2 Anode, R9–R12 (LED-Kathoden) |

---

## Ladeschaltung

| Netz | Verbindung |
|---|---|
| `USB_VBUS` | J1 VBUS (A4, A9, B4, B9) → U2 `VCC` |
| `USB_CC1` | J1 A5 → R13 → GND |
| `USB_CC2` | J1 B5 → R14 → GND |
| `PROG` | U2 `PROG` → R15 (4,7 kΩ) → GND |
| `BAT` | U2 `BAT` → U3 `VDD`, BT1 (+) |
| Schutzkette | U3 `OD`/`OC` → Gates von U4; U4 Source-Paar in der Minusleitung der Zelle |

> Beim **Fertigmodul (BOM 2b)** entfallen J1, R13, R14, U3 und U4 — sie sind
> enthalten. Nur R_prog muss getauscht werden.

---

## Motorzweig links (rechts identisch mit R2/R4/Q2/D2/C4/M2)

| Von | Nach |
|---|---|
| U1 `GPIO32` | R1 (1 kΩ) |
| R1 | Q1 Basis |
| Q1 Basis | R3 (10 kΩ) → GND |
| `+5V` | M1 Anschluss A |
| M1 Anschluss B | Q1 Kollektor |
| M1 Anschluss B | D1 **Kathode** |
| M1 Anschluss A | D1 **Anode** |
| Q1 Kollektor | C4 (100 nF) → GND |
| Q1 Emitter | GND (Leistungsabgang) |

> D1 liegt **in Sperrrichtung parallel zum Motor**: Kathode an die Seite, die
> zum Kollektor geht, Anode an +5 V. Im Betrieb sperrt sie, beim Abschalten
> übernimmt sie den Induktionsstrom.

---

## Sensorzweig links (rechts identisch mit R6/R8/OS2)

| Von | Nach |
|---|---|
| `+3V3` | R5 (150 Ω) |
| R5 | OS1 IR-LED **Anode** |
| OS1 IR-LED **Kathode** | GND (Sensorabgang) |
| `+3V3` | OS1 Fototransistor **Kollektor** |
| OS1 Fototransistor **Emitter** | U1 `GPIO34` |
| OS1 Fototransistor **Emitter** | R7 (10 kΩ) → GND |

Emitterfolger-Schaltung: die Spannung an GPIO 34 steigt mit der einfallenden
IR-Menge. Heller Untergrund = hoher ADC-Wert, schwarze Linie = niedriger Wert.

**TCRT5000-Pinbelegung:** die vier Beine sind von außen nicht eindeutig
beschriftet. Das klare Gehäuse ist die IR-LED, das schwarze/dunkle der
Fototransistor. Vor dem Einlöten mit der Dioden-Messfunktion prüfen: die LED
zeigt ≈ 1,1 V in Vorwärtsrichtung, der Fototransistor in beiden Richtungen
nichts.

---

## LEDs

| GPIO | Widerstand | LED | Position |
|---|---|---|---|
| 14 | R9 220 Ω | LED1 gelb | vorne links |
| 27 | R10 220 Ω | LED2 gelb | vorne rechts |
| 26 | R11 220 Ω | LED3 rot | hinten links |
| 25 | R12 220 Ω | LED4 rot | hinten rechts |

Jeweils GPIO → Widerstand → LED-Anode, LED-Kathode → GND (Sensorabgang).

---

## Optional

| Netz | Verbindung | Aktivierung |
|---|---|---|
| Akkumessung | `VBAT` → R16 (100 kΩ) → Knoten → R17 (100 kΩ) → GND; Knoten → U1 `GPIO39`, dazu 100 nF nach GND | `FEATURE_BATTERY_MONITOR 1` |
| Modustaster | U1 `GPIO4` → SW2 → GND | `FEATURE_MODE_BUTTON 1` |

---

## Nicht belegte DevKit-Pins

`EN`, `GPIO36`, `GPIO12`, `GPIO13`, `GPIO23`, `GPIO22`, `GPIO21`, `GPIO19`,
`GPIO18`, `GPIO17`, `GPIO16`, `GPIO5`, `GPIO4`, `GPIO2`, `GPIO15`, `TX0`, `RX0`

`GPIO12` und `GPIO13` bleiben bewusst frei — Begründung in der
[Pinbelegung](../Doku/05_Pinbelegung.md). `TX0`/`RX0` sind von der
USB-Konsole belegt.

---

## I²C-Bus (Nachtrag: Lage- und Abstandssensor)

Beide Sensoren teilen sich einen Bus. Die Adressen kollidieren nicht.

| Netz | Verbindung |
|---|---|
| `SDA` | U1 `GPIO21` → U6 `SDA`, U7 `SDA` |
| `SCL` | U1 `GPIO22` → U6 `SCL`, U7 `SCL` |
| `+3V3` | U1 `3V3` → U6 `VCC`, U7 `VIN` |
| `GND` | Sensorabgang → U6 `GND`, U7 `GND` |

| Sensor | Adresse | Platz |
|---|---|---|
| U6 · MPU-6050 (GY-521) | `0x68` (AD0 offen/GND) | flach auf dem Chassis |
| U7 · VL53L0X (GY-530) | `0x29` | auf dem Mast, nach vorn blickend |

**Pull-ups:** beide Breakouts bringen eigene 4,7-kΩ-Pull-ups auf SDA und SCL
mit. Zwei Module parallel ergeben also rechnerisch 2,35 kΩ — bei 400 kHz
völlig unkritisch, es sind keine weiteren Widerstände nötig. Kommt ein
drittes Modul dazu, auf einem die Pull-ups auslöten.

**Nicht belegt:** `AD0` (Adresswahl des MPU-6050), `INT` beider Module,
`XSHUT` des VL53L0X. XSHUT bräuchte man nur, wenn ein zweiter VL53L0X
dazukäme — zwei Sensoren haben dieselbe feste Adresse und müssen beim Start
nacheinander aufgeweckt und umadressiert werden.

### Mast

| | |
|---|---|
| Werkstoff | CFK-Rundstab Ø 3 mm |
| Länge | ca. 60 mm über der Platine |
| Position | vorn mittig, zwischen den beiden TCRT5000 |
| Blickrichtung | waagerecht nach vorn, **nicht** zum Boden geneigt |
| Befestigung | in eine Ø-3,2-mm-Bohrung geklebt, zusätzlich mit Heißkleber verkeilt |

**Kabelführung:** die vier Litzen **verdrillt** am Mast entlangführen und am
Fuß eine kleine Schlaufe lassen. Ein straff gespanntes Kabel an einem
vibrierenden Mast bricht an der Lötstelle — die Schlaufe nimmt die Bewegung
auf. Mit zwei Tropfen Heißkleber am Mast fixieren, nicht an der Platine
ziehen lassen.
