# Inbetriebnahme und Tuning

---

## 1 · Software vorbereiten

### Arduino IDE

1. Boardverwalter-URL eintragen und **esp32 by Espressif** installieren
   (Core 2.0.x oder 3.x — die Firmware ist für beide ausgelegt, die LEDC-API
   wird per `#if` umgeschaltet).
2. Bibliotheksverwalter: **WebSockets** von *Markus Sattler* („arduinoWebSockets"),
   Version ≥ 2.4.1. Das ist die **einzige** externe Abhängigkeit, alles andere
   ist Teil des Cores.
3. Ordner `Firmware/bristlebot/` öffnen (`bristlebot.ino` anklicken, die `.h`-
   und `.cpp`-Dateien erscheinen als Tabs).
4. Board: **ESP32 Dev Module**, Flash Size 4 MB, Partition Scheme
   *Default 4MB with spiffs*, Upload Speed 921600.

### PlatformIO

`Firmware/platformio.ini` ist vorbereitet — `pio run -t upload` genügt, die
Bibliothek wird automatisch geholt.

---

## 2 · Hardware schrittweise in Betrieb nehmen

> Jeden Schritt einzeln abschließen. Bei Vibrationsmotoren und einem knapp
> bemessenen Step-Up ist „alles zusammenstecken und einschalten" ein guter Weg,
> ein DevKit zu verlieren.

### Schritt 1 — Step-Up einstellen, **bevor** der ESP32 dran kommt

Akku an U5 anschließen, Ausgang **unbelastet** lassen, mit dem Multimeter messen
und den Trimmer auf **5,00 V** drehen. Die Module kommen mit beliebiger
Einstellung aus der Fabrik, oft über 20 V — das tötet das DevKit sofort.
Danach die optionale Zenerdiode D3 einlöten.

### Schritt 2 — Laden prüfen

USB-C anstecken. Lade-LED muss an sein. Sind die CC-Widerstände (R13/R14) nicht
gesetzt, passiert mit einem USB-C-Netzteil **nichts** — zum Gegentest ein
altes USB-A-Kabel verwenden, das funktioniert auch ohne CC.
Ladestrom am besten einmal in Reihe messen: soll ≈ 255 mA sein, nicht 1 A.

### Schritt 3 — DevKit allein, ohne Motoren und Sensoren

Firmware flashen. Serielle Konsole auf 115200 Baud, es muss erscheinen:

```
Bristlebot startet
AP   : Bristlebot / bristlebot
URL  : http://192.168.4.1/
Duty : max 613 von 1023 (3.0 V Motor an 5.0 V Schiene)
Kal. : FEHLT
```

Handy ins WLAN, Seite aufrufen, Statuspunkt muss grün werden. Wenn das steht,
funktioniert die halbe Anlage.

### Schritt 4 — LEDs

Noch gesperrt: die roten Rücklichter pulsen langsam im Sekundentakt. Nach
*Freigeben* leuchten sie dauerhaft. Tut eine LED nichts: Polung prüfen
(lange Anode an den Vorwiderstand).

### Schritt 5 — Sensoren

Noch ohne Motoren. In der Web-App auf *Autonom* schalten und die Sensorbalken
beobachten, während du die Platine über helles Papier und über ein schwarzes
Isolierband hältst.

* Balken bewegen sich nicht → Sensor falsch gepolt, oder R5/R6 fehlen
  (IR-LED bekommt keinen Strom), oder R7/R8 fehlen
* Balken dauerhaft am Anschlag → R7/R8 auf 4,7 kΩ verkleinern
* Hub zwischen Linie und Papier zu klein → R7/R8 auf 22 kΩ vergrößern, oder der
  Abstand zum Boden ist zu groß (Optimum ≈ 2,5 mm)

### Schritt 6 — Motoren

Jetzt erst Q1/Q2, Motoren, Dioden und Elko. Einschalten und **genau hinsehen:
die Motoren müssen beim Booten stillstehen.** Zucken sie, fehlen die
Basis-Pulldowns R3/R4.

Dann *Manuell*, *Freigeben*, Joystick langsam nach oben: beide Motoren müssen
gleichmäßig anlaufen.

### Schritt 7 — Funktionstest laufen lassen

Zum Abschluss der Montage und bei jeder späteren Fehlersuche: in der
Weboberfläche ganz unten **Funktionstest starten** (die Freigabe muss
gesetzt sein, weil Motoren anlaufen).

Der Test läuft in Schleife, bis du ihn stoppst, und zeigt den aktuellen
Abschnitt im Klartext an:

| Abschnitt | Dauer | Was zu sehen sein muss |
|---|---|---|
| LED vorne links | 0,7 s | nur LED1 leuchtet |
| LED vorne rechts | 0,7 s | LED1 **und** LED2 |
| LED hinten links | 0,7 s | dazu LED3 |
| LED hinten rechts | 0,7 s | alle vier leuchten |
| alle LEDs blinken | 3 s | alle vier im Gleichtakt, 2 Hz |
| Motor links | 3 s | **nur** der linke Motor, langsam anlaufend bis Vollgas; dabei leuchtet die vordere **linke** LED |
| Motor rechts | 3 s | dasselbe rechts, mit der vorderen **rechten** LED |

Dazwischen liegen kurze Pausen, damit die Abschnitte auseinanderzuhalten sind.

**Warum die LED beim Motortest mitleuchtet:** Sie zeigt, welche Seite gerade
*angesteuert* wird. Vibriert der rechte Motor, während die linke LED
leuchtet, sind Motor und Seite vertauscht verdrahtet — ein Fehler, der sich
im Fahrbetrieb nur als „lenkt falschherum" äußert und dort schwer
zuzuordnen ist.

Weitere typische Befunde:

| Beobachtung | Ursache |
|---|---|
| eine LED bleibt dunkel | Polung vertauscht, Vorwiderstand oder Lötstelle |
| beide Motoren laufen gleichzeitig | Transistoren oder PWM-Leitungen verbunden/vertauscht |
| Motor startet erst spät in der Rampe | normal — das ist genau die Haftreibung, gegen die die *Anlaufschwelle* eingestellt wird |
| Motor läuft gar nicht an | Transistor, Freilaufdiode verpolt, oder 5-V-Schiene bricht ein |
| ESP32 startet während der Motorrampe neu | Brownout: Step-Up am Anschlag. Siehe [Review B2](01_Schaltplan-Review.md) |

Der Funktionstest ist fest in der Firmware verdrahtet, belegt keinen der
vier Programmplätze und lässt sich nicht versehentlich löschen.

---

## 3 · Kalibrierung der Linie

Pflicht vor der ersten autonomen Fahrt — ohne gültige Kalibrierung fährt der
Roboter gar nicht und die Statuszeile zeigt „nicht kalibriert".

1. Betriebsart *Autonom*, Roboter **gesperrt** lassen.
2. **Linie kalibrieren** antippen. Alle vier LEDs blinken schnell.
3. Innerhalb der 5 Sekunden den Roboter langsam quer über die Linie hin und her
   schwenken, so dass **beide** Sensoren einmal die schwarze Linie und einmal
   den hellen Untergrund sehen.
4. Blinken endet. Statuszeile darf „nicht kalibriert" nicht mehr zeigen.

Die Werte liegen im NVS-Flash und überleben Stromausfall und Neuflashen der
Firmware. Nach einem Untergrundwechsel (anderes Papier, andere Beleuchtung)
neu kalibrieren. Mit **Reset** werden sie verworfen.

Schlägt die Kalibrierung fehl, war der Hub zu klein (unter 150 ADC-Zählern) —
siehe Schritt 5 oben.

---

## 4 · Regler abstimmen

Vorgehen in dieser Reihenfolge, immer nur **einen** Wert verändern:

### 4.1 Anlaufschwelle

Betriebsart *Manuell*, Joystick ganz leicht nach oben. Die Anlaufschwelle so
weit hochziehen, bis **beide** Motoren bei kleinstem Ausschlag sicher anlaufen —
dann einen Tick zurück. Zu hoch gewählt verliert man den langsamen Bereich, zu
niedrig bleibt ein Motor stehen und der Roboter zieht zur Seite.

Laufen die Motoren unterschiedlich an, liegt das an der Serienstreuung der
Motoren, nicht an der Elektronik. Die Schwelle gilt für beide gemeinsam.

### 4.2 Grundvibration

Mit **40 %** anfangen. Das ist bewusst langsam: ein zu schneller Bristlebot
schießt über die Linie hinaus, bevor der Regler reagieren kann.

### 4.3 Kp — Lenkstärke

Von 0,2 aus in Schritten von 0,1 erhöhen, bis der Roboter die Linie zügig
zurückgewinnt.

| Symptom | Ursache | Maßnahme |
|---|---|---|
| Fährt träge über die Linie hinaus | Kp zu klein | Kp erhöhen |
| Pendelt gleichmäßig um die Linie | Kp zu groß | Kp senken oder Kd erhöhen |
| Schlägt heftig von Seite zu Seite | Kp deutlich zu groß | Kp halbieren |

### 4.4 Kd — Dämpfung

Erst wenn Kp sitzt. Kd von 0 aus in Schritten von 0,02 erhöhen, bis das
Nachpendeln verschwindet. Zu groß gewählt wird die Lenkung zappelig, weil Kd auf
das Sensorrauschen reagiert.

### 4.5 Grundvibration nachziehen

Jetzt in 10-%-Schritten hochgehen, bis es wieder unruhig wird — und eine Stufe
zurück. Höhere Geschwindigkeit braucht meist etwas mehr Kd.

Alle Werte werden 1,5 Sekunden nach der letzten Änderung automatisch ins NVS
geschrieben. Das Verzögern ist Absicht: ein Schieberegler feuert sonst Dutzende
Flash-Schreibvorgänge pro Sekunde.

### Startwerte im Code

In [`Config.h`](../Firmware/bristlebot/Config.h) stehen die Vorgabewerte
(`CTRL_KP_DEFAULT` usw.) für den Fall, dass das NVS leer ist. Hat sich ein Satz
Werte bewährt, dort eintragen.

---

## 5 · Teststrecke

* **Linie:** schwarzes Isolierband, 19 mm, auf weißem Papier oder heller Platte.
  Mattes Material — Hochglanz spiegelt die IR-LED direkt in den Fototransistor
  und verfälscht die Messung.
* **Kurvenradius:** zum Anfangen nicht unter 150 mm. Ein Bristlebot hat wenig
  Lenkautorität, 90°-Ecken sind nichts für den ersten Versuch.
* **Untergrund:** glatt und hart. Teppich frisst die Vibration vollständig.
* **Beleuchtung:** gleichmäßig. Harte Schlagschatten und direkte Sonne (viel IR!)
  verschieben den Arbeitspunkt; dann neu kalibrieren.

---

## 6 · Fehlersuche

| Symptom | Wahrscheinliche Ursache |
|---|---|
| ESP32 bootet nicht, Konsole bleibt leer | Step-Up nicht auf 5 V eingestellt. Oder (bei eigenem Pinplan) ein Treiber an GPIO 12 |
| Dauernd Neustarts im Betrieb | Brownout: Akku zu klein oder Step-Up am Anschlag. Motoren auf VBAT legen ([Review B2](01_Schaltplan-Review.md)) |
| Motoren zucken beim Einschalten | R3/R4 (Basis-Pulldown 10 kΩ) fehlen |
| Sensorbalken zittern stark, nur wenn die Motoren laufen | Masseführung: Motorrückstrom läuft über die Sensormasse. [Layout §2](04_PCB-Layout-Empfehlung.md) |
| Roboter lenkt kaum, obwohl der Duty klar unterschiedlich ist | gemeinsame Bürstenfläche statt zwei getrennter Borstenfelder |
| Roboter lenkt in die falsche Richtung | Sensoren oder Motoren vertauscht. In `Config.h` `PIN_LINE_L` und `PIN_LINE_R` tauschen |
| Fährt gar nicht los, Duty steht aber an | zu schwer oder Borsten zu steif/zu senkrecht. [02 §4](02_SMD-vs-THT-Wirtschaftlichkeit.md) |
| WLAN bricht ständig ab | Kupfer unter dem Antennenbereich des DevKits |
| App verbindet, Joystick reagiert nicht | iOS-Portalfenster. Schließen und `http://192.168.4.1` in Safari aufrufen |
| `hz` in der Statuszeile unter 200 | ungewöhnlich — Blockierung in der Schleife, serielle Debugausgabe abschalten (`FEATURE_SERIAL_DEBUG 0`) |
| USB-C lädt nicht | R13/R14 (2× 5,1 kΩ an CC1/CC2) fehlen |

---

## 7 · Noch offen

Diese Punkte sind bewusst nicht erledigt und stehen im
[Projekt-Log](../00_Projekt-Log.md):

- [ ] **Nichts davon ist in Hardware getestet.** Firmware kompiliert gegen die
      dokumentierten Core-Versionen, aber ohne Aufbau gibt es keine Messwerte.
- [ ] Schaltplan und Layout in KiCad zeichnen — die
      [Netzliste](../Hardware/Netzliste.md) ist die Vorlage dafür
- [ ] Entscheidung, ob die Motoren auf VBAT statt auf die 5-V-Schiene gehen
- [ ] Borstenwinkel und Borstenhärte empirisch ermitteln
- [ ] Startwerte für Kp/Kd/Grundvibration nach dem ersten Fahrversuch in
      `Config.h` eintragen
