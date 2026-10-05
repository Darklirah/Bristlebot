# Wirtschaftlichkeits- und Layout-Empfehlung: SMD vs. THT

Alle Preise sind **Richtwerte in Euro, Stand Oktober 2026, ohne Versand**, aus
europäischen Distributoren (Reichelt / Berrybase / Mouser). Bei AliExpress liegen
die Modulpreise rund 40 % darunter, dafür mit Lieferzeit und Qualitätsstreuung.

---

## 1 · Die drei realistischen Wege

| | **A — DevKit 30-Pin, gesteckt** | **B — ESP32-WROOM-32E, SMD** | **C — ESP32-C3-MINI-1, SMD** |
|---|---|---|---|
| Modul | NodeMCU DevKit V1 | WROOM-32E, kastelliert | C3-MINI-1 |
| Löten | Buchsenleiste, Lötkolben | Lötkolben oder Heißluft | Heißluft empfohlen |
| USB/Programmierung | **schon drauf** | CH340C + USB-C + 2 Taster | **im Chip** (USB-Serial-JTAG) |
| Masse Modulgruppe | ≈ **10,5 g** | ≈ **2,7 g** | ≈ **1,8 g** |
| Bauhöhe über PCB | ≈ 13 mm (mit Buchse) | ≈ 3,2 mm | ≈ 2,5 mm |
| Klassisches Bluetooth | ja | ja | **nein, nur BLE** |
| Pinplan aus deinem Entwurf | passt | passt | **muss neu** |

---

## 2 · Stückkosten des Elektronikpfads

Verglichen wird nur, was sich zwischen den Varianten **unterscheidet**: Modul,
Programmierweg und Spannungsaufbereitung. Sensoren, Motoren, Treiberstufe, LEDs
und Ladeschaltung sind in allen drei Varianten identisch und stehen in der
[Stückliste](03_Stueckliste-BOM.md).

### Einzelstück (n = 1)

| Position | A · DevKit | B · WROOM | C · C3-MINI |
|---|---|---|---|
| Modul | 5,50 | 2,80 | 1,90 |
| 2× Buchsenleiste 15-pol | 0,60 | — | — |
| CH340C + Quarzlos-Beschaltung | — | 0,45 | — |
| USB-C-Buchse + 2× 5,1 kΩ | — | 0,60 | 0,60 |
| Taster BOOT + EN, RC-Glied | — | 0,40 | 0,40 |
| 5-V-Step-Up MT3608 (Modul) | 1,20 | — | — |
| 3,3-V-Buck-Boost TPS63020 + L + C | — | 2,50 | 2,50 |
| **Summe** | **7,30** | **6,75** | **5,40** |

### Bei 10 Stück (pro Einheit)

| Position | A · DevKit | B · WROOM | C · C3-MINI |
|---|---|---|---|
| Modul | 4,90 | 2,40 | 1,60 |
| Peripherie wie oben | 1,70 | 3,60 | 3,20 |
| **Summe Teile** | **6,60** | **6,00** | **4,80** |
| PCB (JLCPCB, 5er-Charge umgelegt) | 0,70 | 0,70 | 0,70 |
| **Gesamt** | **7,30** | **6,70** | **5,50** |

### Bei 100 Stück (pro Einheit)

| Position | A · DevKit | B · WROOM | C · C3-MINI |
|---|---|---|---|
| Modul | 4,20 | 2,20 | 1,45 |
| Peripherie | 1,50 | 3,20 | 2,85 |
| PCB | 0,35 | 0,35 | 0,35 |
| Bestückung (JLCPCB PCBA, Setup umgelegt) | — *(Handlöten)* | 1,10 | 1,10 |
| Handlötzeit (bewertet mit 20 €/h) | 5,00 *(15 min)* | 1,70 *(5 min Nacharbeit)* | 1,70 |
| **Gesamt** | **11,05** | **8,55** | **7,45** |

---

## 3 · Die Zahlen gelesen

**Bei Einzelstück und Kleinserie kostet SMD praktisch dasselbe wie THT.** Das ist
das überraschende Ergebnis: das WROOM-Modul ist 2,70 € billiger als das DevKit,
aber USB-Buchse, USB-UART-Brücke, zwei Taster und ein ordentlicher Buck-Boost
fressen diesen Vorsprung fast vollständig auf. Die Kostenfrage entscheidet
also **nicht**.

Entschieden wird über drei andere Dinge:

**Risiko und Zeit.** Im DevKit-Pfad gibt es keine Inbetriebnahme der
USB-Schnittstelle: Kabel rein, Code drauf. Im SMD-Pfad kommen CH340C-Treiber,
Auto-Reset-Schaltung (DTR/RTS auf EN und IO0) und ein Boot-Taster dazu — alles
Standard, aber alles Dinge, die beim ersten PCB-Dreh schiefgehen können. Für
einen Erstaufbau ist das vermeidbares Risiko.

**Reparierbarkeit.** Ein gesteckter DevKit ist in zehn Sekunden getauscht. Ein
verlötetes WROOM-Modul bekommt man ohne Heißluft nicht mehr sauber herunter.

**Gewicht.** Und das ist bei diesem Projekt kein Komfortmerkmal, sondern die
Funktionsgrenze — siehe nächster Abschnitt.

### Wirtschaftliche Kippstelle

Die Handlötzeit macht den Unterschied. Pro Gerät 15 Minuten Buchsenleisten,
Widerstände, LEDs und Dioden einlöten rechnet sich bis etwa 10 Stück. Ab
**ca. 20 Stück** wird eine SMD-Baugruppe mit maschineller Bestückung billiger,
ab **50 Stück** ist es nicht mehr diskutabel.

---

## 4 · Massebudget — der eigentliche Knackpunkt

Ein Bristlebot bewegt sich durch Vibration: der Motor regt die schräggestellten
Borsten an, die sich in eine Richtung leichter abstützen als in die andere. Die
Antriebskraft steigt mit der Unwucht und **sinkt mit der Masse**. Typische
funktionierende Bristlebots liegen bei 10–25 g.

Massebudget für **Variante A** (dein gewählter Weg), 500 mAh, 1,6 mm PCB, 5-mm-LEDs:

| Position | Masse |
|---|---|
| ESP32 DevKit V1 + 2 Buchsenleisten | 10,5 g |
| PCB 50 × 38 mm, FR4 1,6 mm, 2 Lagen | 6,0 g |
| LiPo 500 mAh (503035) | 10,0 g |
| TP4056 + DW01A + FS8205 + USB-C | 1,5 g |
| MT3608-Modul | 1,2 g |
| Schiebeschalter | 1,0 g |
| 2× Vibrationsmotor (10 mm Münze) | 2,2 g |
| 2× TCRT5000 | 0,8 g |
| 4× LED 5 mm | 1,4 g |
| Widerstände, Dioden, Transistoren, Elko | 2,5 g |
| 2× Bürstenkopf + Klebepads | 4,0 g |
| JST-Buchse, Litzen | 1,5 g |
| **Summe** | **≈ 42,6 g** |

Das ist **rund doppelt so schwer wie ein klassischer Bristlebot**. Es wird
fahren, aber eher kriechen als flitzen, und die Lenkautorität der
Vibrationsdifferenz ist bei hoher Masse gering. Das ist das größte technische
Risiko im Projekt — nicht die Elektronik.

### Abspeckliste, nach Wirkung sortiert

| Maßnahme | Ersparnis | Kosten |
|---|---|---|
| **1,0 mm PCB statt 1,6 mm** bestellen | −2,2 g | 0 € (gleicher Preis) |
| SMD-WROOM statt DevKit (Variante B) | −7,8 g | Mehraufwand, siehe oben |
| LiPo 300 mAh statt 500 mAh | −3,5 g | −35 % Laufzeit |
| LEDs 3 mm statt 5 mm | −0,8 g | 0 € |
| PCB-Außenmaß auf 45 × 32 mm straffen | −1,4 g | engeres Layout |

Damit kommst du im DevKit-Pfad auf ≈ **37 g**, im SMD-Pfad auf ≈ **28 g**.

### Zwei Dinge, die mechanisch wichtiger sind als die letzten Gramm

1. **Zwei getrennte Bürstenköpfe, einer unter jedem Motor.** Auf einer
   gemeinsamen Bürstenfläche verteilt sich die Vibration über die ganze
   Grundplatte und die Differenz der beiden Motoren mittelt sich weitgehend weg
   — der Roboter lenkt dann kaum. Zwei mechanisch entkoppelte Borstenfelder,
   jedes direkt unter seinem Motor, sind die Voraussetzung dafür, dass
   differentielles Lenken überhaupt greift.
2. **Motoren so weit außen und so weit vorn wie möglich.** Der Lenkmoment-Hebel
   wächst linear mit dem Abstand zur Längsachse.

### Empfehlung

**Variante A bauen, so wie entschieden** — aber das PCB in **1,0 mm** bestellen
und 3-mm-LEDs setzen. Das kostet nichts und holt 3 g.

Falls der fertige Prototyp zu träge ist, ist das genau der Auslöser für einen
zweiten PCB-Dreh in **Variante B**: der Pinplan bleibt identisch, die Firmware
läuft unverändert, nur Modul und Spannungsaufbereitung ändern sich. Die 8 g
Ersparnis sind dann der größte Einzelhebel, den es gibt.

---

## 5 · Laufzeit

Mittlere Stromaufnahme aus dem Akku, gerechnet mit dem 56-%-Wirkungsgrad des
Pfads MT3608 + AMS1117 (siehe [Review B1](01_Schaltplan-Review.md)):

| Verbraucher | Leistung | aus dem Akku bei 3,7 V |
|---|---|---|
| ESP32, Access Point aktiv (Ø 110 mA @ 3,3 V) | 0,36 W | 174 mA |
| 2 Motoren bei 55 % Grundvibration | 0,50 W | 159 mA |
| 2 IR-LEDs der TCRT5000 (je 14 mA) | 0,09 W | 43 mA |
| 2 rote Rücklichter dauerhaft (je 6 mA) | 0,04 W | 29 mA |
| **Fahrbetrieb** | | **≈ 405 mA** |
| **Standby** (freigegeben, Motoren aus) | | **≈ 246 mA** |

| Akku | Fahrbetrieb | Standby |
|---|---|---|
| 300 mAh | ≈ 45 min | ≈ 1,2 h |
| **500 mAh** | **≈ 1,2 h** | **≈ 2,0 h** |
| 1000 mAh | ≈ 2,5 h | ≈ 4,1 h |

Mit dem 3,3-V-Buck-Boost aus Variante B sinkt der ESP32-Anteil von 174 auf
108 mA, die Fahrzeit mit 500 mAh steigt auf **≈ 1,6 h**. Der bessere
Wirkungsgrad bringt also rund **ein Drittel mehr Laufzeit** — ein zweites
Argument für die SMD-Revision, falls es je dazu kommt.

---

## 6 · Layout-Empfehlung in einem Satz

**2-lagig, 1,0 mm FR4, 50 × 38 mm, DevKit gesteckt und mit dem Antennenende
über die Platinenkante hinausragend, Motoren und Sensoren vorn außen, Akku
mittig unter dem DevKit.** Die Begründung und die Verlegungsregeln stehen in
[04_PCB-Layout-Empfehlung.md](04_PCB-Layout-Empfehlung.md).

---

## 7 · Geprüft und verworfen: 3× AAA statt LiPo

Frage vom 05.10.2026: Spart ein Batteriehalter mit Schalter für 3× AAA plus
Buck-Boost Gewicht und bringt er Vorteile?

**Gewicht: nein, im Gegenteil.**

| Position | LiPo (gewählt) | 3× AAA |
|---|---|---|
| Zellen | 10,0 g (500 mAh, 503035) | **34,5 g** (3× 11,5 g Alkaline) |
| Halter mit Schalter | — | **≈ 16 g** |
| Ladeelektronik TP4056 + USB-C | 1,5 g | entfällt |
| Wandler | 1,2 g (MT3608) | 2,0 g (Buck-Boost) |
| JST-Buchse, Schiebeschalter | 1,25 g | entfällt |
| **Summe** | **≈ 14 g** | **≈ 52 g** |

Die Gesamtmasse stiege damit von ~45 g auf **~85 g**. Bei einem Antrieb,
dessen Kraft mit der Masse sinkt, wäre das das Ende der Fahrleistung.

**Energiedichte ebenfalls schlechter:** 3× AAA liefern ≈ 3,6 Wh gegenüber
1,85 Wh beim LiPo — doppelte Energie für die 3,7-fache Masse. Fahrzeit ≈ 3 h
statt 1,6 h.

**Dazu ein elektrisches Problem:** AAA-Alkalizellen haben je ≈ 0,3 Ω
Innenwiderstand, in Reihe 0,9 Ω. Bei den 1-A-Spitzen des ESP32 bricht die
Spannung um 0,9 V ein. NiMH wäre mit ≈ 0,1 Ω je Zelle besser, wiegt aber
nochmals mehr.

### Was an dem Vorschlag trotzdem richtig war

**Der Buck-Boost — und zwar unabhängig von der Zellwahl.** Der heutige Weg
MT3608 → 5 V → bordeigener AMS1117 → 3,3 V hat ≈ 56 % Wirkungsgrad (siehe
[Review B1](01_Schaltplan-Review.md)). Ein Buck-Boost **direkt auf 3,3 V**,
eingespeist am 3V3-Pin, käme auf ≈ 90 % und brächte **rund ein Drittel mehr
Laufzeit bei null Mehrgewicht** — ~1,7 h statt ~1,2 h.

Haken: Man speist damit rückwärts in den Ausgang des AMS1117 ein. Das
funktioniert praktisch, ist aber außerhalb der Spezifikation, und **USB und
Akkuversorgung dürfen dann nicht gleichzeitig anliegen**. Sauberer wäre, den
AMS1117 auszulöten — dann entfällt aber auch das Flashen über USB.

**Entschieden:** LiPo bleibt wie geplant. Über den Buck-Boost wird erst
entschieden, wenn der erste Aufbau fährt — die Massefrage ist bislang
gerechnet, nicht gemessen.

### Sicherheitsaspekt, der bestehen bleibt

Gegen AAA sprach die Masse, nicht die Sicherheit. Der Einwand, dass eine
LiPo-Zelle in einem Projekt für Jugendliche mehr Sorgfalt verlangt, ist
berechtigt. Deshalb: **geschützte Zelle** (TP4056 + DW01A + FS8205 sind
bereits in der Stückliste), Zelle **vollständig im Gehäuse**, keine freien
Kontakte, und beim Laden nicht unbeaufsichtigt lassen.
