# Boardwahl: AZ-Delivery ESP32 DevKit C V4

Festgelegt am 05.10.2026. Ersetzt die bisherige Annahme eines
30-Pin-DevKits (DOIT V1).

[Produktseite](https://www.az-delivery.de/products/esp-32-dev-kit-c-v4)

---

## 1 · Was das Board ist

| | |
|---|---|
| Bauart | Nachbau des **Espressif ESP32-DevKitC V4** |
| Modul | ESP32-WROOM-32 bzw. -32D |
| Stiftleisten | **2 × 19 = 38 Pins** |
| Platinenmaß | 54,4 × 27,9 mm (mit PCB-Antenne) |
| Reihenabstand | **25,4 mm (1,0 Zoll)** laut Espressif-Gerberdaten |

## 2 · Was dadurch gelöst ist

**Die Pinbelegung ist keine offene Frage mehr.** Das DevKitC ist Espressifs
eigenes Design, und in der [offiziellen Espressif-KiCad-Bibliothek](https://github.com/espressif/kicad-libraries)
liegt das Symbol `ESP32-DevKitC` mit genau diesen 38 Pins. Keine Herleitung,
keine Websuche, keine Variantenfrage — amtliche Quelle.

Die Bibliothek liegt im Projekt unter `pcb/lib/Espressif.kicad_sym`.

**Die Firmware bleibt unverändert.** Alle benutzten GPIOs sind auf dem
38-Pin-Board vorhanden:

| Funktion | GPIO | Pin am DevKitC |
|---|---|---|
| Motor links / rechts | 32 / 33 | 7 / 8 |
| Liniensensor links / rechts | 34 / 35 | 5 / 6 |
| LED vorne links / rechts | 14 / 27 | 12 / 11 |
| LED hinten links / rechts | 26 / 25 | 10 / 9 |
| I²C SDA / SCL | 21 / 22 | 33 / 36 |
| Akkumessung (optional) | 39 | 4 |

## 3 · Was sich dadurch ändert

### Die Platine wird größer

Das Board ist **54,4 mm lang** statt der rund 51 mm des 30-Pin-DevKits. Der
in [04_PCB-Layout-Empfehlung.md](04_PCB-Layout-Empfehlung.md) vorgesehene
Umriss von 50 × 38 mm **passt nicht mehr** — ein 54,4 mm langes Modul hat
darauf keinen Platz.

Neuer Richtwert: **60 × 38 mm**. Endgültig festgelegt wird er zusammen mit
dem 3D-Basismodul.

### Masse steigt weiter

Das 38-Pin-Board wiegt rund 1 g mehr als das 30-Pin-Board, die größere
Platine nochmals etwa 1 g. Das sind **+2 g auf rund 47 g** — in die falsche
Richtung bei dem Risiko, das ohnehin das größte im Projekt ist (siehe
[02 §4](02_SMD-vs-THT-Wirtschaftlichkeit.md)).

Die dortige Abspeckliste bleibt gültig und holt das mehr als zurück:
1,0-mm-Platine statt 1,6 mm, 3-mm-LEDs statt 5 mm.

### Neue Stolperfalle: GPIO 6 bis 11

Das 38-Pin-Board führt die **Flash-Pins heraus** (D0, D1, D2, D3, CMD, CLK =
GPIO 6–11), beidseitig neben dem USB-Anschluss. Sie sind intern mit dem
SPI-Flash verbunden.

> **GPIO 6 bis 11 dürfen niemals benutzt werden.** Wer sie beschaltet, stört
> den Flash-Zugriff — das Board bootet dann nicht mehr oder stürzt
> sporadisch ab. Beim 30-Pin-Board gab es diese Falle nicht, weil die Pins
> dort gar nicht herausgeführt sind.

Im Layout als Sperrfläche behandeln und im Siebdruck kennzeichnen.

---

## 4 · Offener Punkt: Reihenabstand messen

**Vor der Platinenbestellung zu prüfen.**

Espressifs eigene Gerberdaten geben **25,4 mm (1,0")** zwischen den
Stiftleisten an. In Foren tauchen für Nachbauten aber auch **22,86 mm
(0,9")** auf. Beide Maße sind im Umlauf.

Für ein gestecktes Modul ist das kritisch: Stimmt der Abstand nicht, passt
das DevKit nicht auf die Platine, und die Platine ist Ausschuss.

**Prüfung:** Sobald das Board da ist, mit dem Messschieber den Abstand von
Lochmitte zu Lochmitte quer über die Platine messen. Dauert 30 Sekunden und
entscheidet über eine Leiterplattenbestellung.

Der Schaltplan hängt nicht daran — nur das Layout.

---

## 5 · Liniensensoren auf der Unterseite

Ebenfalls am 05.10.2026 festgelegt: Die beiden TCRT5000 kommen **auf die
Unterseite** der Platine und schauen direkt nach unten.

Vorteile gegenüber den zuvor erwogenen Varianten:

* keine Durchbrüche in der Platine nötig
* keine von Hand gebogenen Beinchen
* **beide Sensoren zwangsläufig exakt gleich hoch** — eine Schieflage würde
  sonst den Nullpunkt der Linienerkennung verschieben

**Geometrische Folge fürs Basismodul:** Der TCRT5000 baut 7,0 mm hoch und
hängt damit 7 mm unter der Platine. Darunter brauchen die Linsen ihren
Arbeitsabstand von rund 2,5 mm.

> Die Platinenunterseite muss **rund 9,5 mm über dem Boden** sitzen.

Das ist eine harte Vorgabe an die Konstruktion des 3D-Basismoduls und
bestimmt zusammen mit der Höhe der Bürstenköpfe die Gesamtgeometrie.
