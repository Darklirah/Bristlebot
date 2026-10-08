# One-of-a-Kind Mars Rover

Ein fahrender, programmierbarer Roboter, der sich **durch Vibration**
fortbewegt. Kein Rad, kein Getriebe, keine Achse: zwei kleine
Unwuchtmotoren rütteln zwei Zahnbürstenköpfe, und weil deren Borsten
schräg stehen, stützen sie sich in eine Richtung leichter ab als in die
andere. Daraus entsteht Vortrieb.

Gesteuert wird er vom Handy aus über ein eigenes WLAN, das der Roboter
selbst aufspannt — ohne App, ohne Heimnetz, auf iPhone und Android gleich.
Er kann einer Linie folgen, sich von Hand per Joystick fahren lassen oder
**ein selbst zusammengestelltes Fahrprogramm abspielen**.

> **Zum Namen.** Das Projekt heißt *One-of-a-Kind Mars Rover* — jedes
> gebaute Exemplar bekommt einen eigenen Namen und ein eigenes WLAN
> (`MarsRover_Petra`), damit mehrere gleichzeitig nebeneinander fahren
> können. In der gesamten Dokumentation heißt der Roboter deshalb
> durchgehend **Mars Rover**.

> **Verwandt mit dem Bristlebot.** Vielleicht kennst du den *Bristlebot*:
> jenen kleinen Vibrationsläufer aus einem Zahnbürstenkopf, einem
> Unwuchtmotor und einer Knopfzelle, der über den Tisch summt. Der Mars
> Rover fährt nach genau demselben Prinzip — nur kann er deutlich mehr:
>
> | Bristlebot | Mars Rover |
> |---|---|
> | ein Motor | **zwei** Motoren, und erst dadurch kann er lenken |
> | fährt, wohin ihn der Zufall trägt | **folgt einer Linie** und sieht, wo sie liegt |
> | keine Elektronik | ESP32 mit Lageerkennung und Abstandsmessung |
> | an/aus | **vom Handy steuerbar**, mit selbst zusammengestellten Fahrprogrammen |
>
> Kurz gesagt: ein Bristlebot fährt, wohin er will — dieser hier fährt,
> wohin du willst.

---

## 1 · Was ihn ausmacht

**Er hat keine Räder.** Das ist kein Mangel, sondern das Prinzip. Ein
Vibrationsantrieb hat keine beweglichen Teile außer dem Motor selbst, keine
Lager, keine Zahnräder, nichts, was sich verhaken kann. Er ist damit
mechanisch beinahe unzerstörbar — und zugleich schwer vorhersehbar, weil
der Vortrieb von Untergrund, Borstenstellung, Gewicht und Akkustand abhängt.

Genau das macht ihn interessant: **man muss ihn kennenlernen.** Ein
Roboter mit Rädern fährt 20 cm, wenn man ihm sagt, er solle 20 cm fahren.
Dieser hier fährt „zwei Sekunden lang", und wie weit das ist, findet man
durch Ausprobieren heraus. Für den Einstieg ins Programmieren ist das ein
Vorteil, kein Nachteil — Rückmeldung aus der echten Welt statt aus einer
Simulation.

**Er lenkt durch Ungleichheit.** Zwei Motoren, zwei getrennte
Borstenfelder. Vibriert links stärker als rechts, dreht er nach rechts. Es
gibt keinen Lenkmechanismus, nur zwei Zahlen.

---

## 2 · Aufbau

| Baugruppe | Was sie tut |
|---|---|
| **ESP32** | Rechner, WLAN-Zugangspunkt und Webserver in einem Modul |
| **2 Vibrationsmotoren** | Antrieb, über Transistoren geschaltet, in 1024 Stufen regelbar |
| **2 Zahnbürstenköpfe** | je einer unter einem Motor — getrennt, sonst mittelt sich die Lenkung weg |
| **2 Reflexkoppler** | schauen nach unten und erkennen die Linie |
| **Lagesensor** | misst Drehungen, damit „drehe um 90°" wirklich 90° sind |
| **Abstandssensor** | sitzt auf einem Mast und schaut nach vorn |
| **4 LEDs** | zwei gelbe vorn als Blinker, zwei rote hinten als Rücklicht |
| **LiPo-Akku + Ladeelektronik** | Laden über USB-C, Laufzeit rund eine Stunde |

Die Sensorik ist **vollständig optional**. Fehlt der Lagesensor, laufen
Drehbefehle zeitgesteuert statt geregelt. Fehlt der Abstandssensor,
beenden sich die Hindernisbefehle nach ihrer Zeitgrenze. Der Roboter sagt
in der Bedienoberfläche selbst, was er hat und was nicht. Man kann also
klein anfangen und später nachrüsten.

---

## 3 · Die Bedienoberfläche

Der Roboter **bringt seine eigene Bedienoberfläche mit**. Sie liegt im
Speicher des ESP32 und wird von ihm ausgeliefert, sobald man sich mit
seinem WLAN verbindet. Kein App-Store, keine Installation, kein Konto.

```
Einschalten  →  am Handy ins WLAN "MarsRover_…"  →  Seite öffnet sich
```

Auf dem iPhone klappt das Bedienfenster meist von selbst auf; sonst
`http://192.168.4.1` im Browser aufrufen.

Die Oberfläche ist auf Bedienbarkeit **ab 14 Jahren** ausgelegt: Klartext
statt Kürzel, große Tippflächen, Farbcodierung, und alles, was man zum
Fahren nicht braucht, liegt hinter einem zugeklappten Experten-Bereich.

### Was man sieht

| Bereich | Inhalt |
|---|---|
| **Freigeben** | Nach dem Einschalten ist der Roboter gesperrt. Ohne Freigabe läuft kein Motor |
| **Betriebsart** | Linie folgen · Selbst fahren · Fahrprogramm |
| **Messwerte** | Sensoren, Motoren, Kurs und Abstand — live |
| **Funktionstest** | prüft alles der Reihe nach durch |
| **Experten** | Reglerabstimmung, Kalibrierung, eigener Name |
| **NOTHALT** | hält alles an und entzieht die Freigabe |

---

## 4 · Die drei Betriebsarten

### Linie folgen

Zwei Reflexkoppler schauen nach unten, je einer links und rechts der
Linie. Sieht der linke mehr Linie als der rechte, liegt der Roboter zu
weit rechts — also stärker nach links lenken. Das erledigt ein
**PD-Regler**: der P-Anteil reagiert auf die Abweichung, der D-Anteil auf
deren Änderungsgeschwindigkeit und dämpft damit das Überschwingen.

Vor der ersten Fahrt muss **kalibriert** werden: Knopf drücken, den
Roboter fünf Sekunden quer über die Linie schwenken, fertig. Er merkt
sich, wie hell der Untergrund und wie dunkel die Linie ist. Die Werte
überleben das Ausschalten.

Verliert er die Linie, sucht er sie: erst in die Richtung, in der sie
zuletzt lag, dann pendelnd. Nach vier Sekunden gibt er auf und hält an.

### Selbst fahren

Ein Joystick auf dem Handy. Nach oben = schneller, zur Seite = lenken.
Steht er still und man schlägt voll zur Seite aus, dreht er auf der Stelle.

**Rückwärts gibt es nicht.** Ein Unwuchtmotor treibt den Roboter nur in
Borstenrichtung; die Drehrichtung des Motors ändert daran nichts. Joystick
nach unten heißt deshalb schlicht „Halt".

Ein **Totmannschalter** passt auf: kommt länger als eine halbe Sekunde
kein Befehl — Handy weggelegt, Browser im Hintergrund, WLAN abgerissen —
halten die Motoren an.

### Fahrprogramm

Der interessanteste Teil. Dazu das ganze nächste Kapitel.

---

## 5 · Programmieren

Das ist das Herzstück: Man stellt sich in der Weboberfläche einen Ablauf
zusammen, speichert ihn im Roboter und lässt ihn abspielen. Vier
Speicherplätze, je 48 Schritte, alles im Flash — es überlebt Stromausfall
und sogar ein Neuaufspielen der Firmware.

### 5.1 Der eine Gedanke, auf den es ankommt

**Jeder Befehl wirkt sofort. Zeit verbraucht nur `warte`.**

Das ist anders, als man es erwartet, und es ist der Punkt, an dem am
Anfang alle stolpern. Ein Fahrbefehl sagt nicht „fahre eine Weile
geradeaus". Er sagt: „ab jetzt geradeaus" — und ist in derselben
Tausendstelsekunde fertig. Erst `warte` lässt Zeit vergehen.

```
geradeaus mit 60 %        ← schaltet die Fahrt ein, dauert 0 Sekunden
LED vorne links blinken   ← schaltet den Blinker ein, dauert 0 Sekunden
warte 2,0 Sekunden        ← JETZT vergehen 2 Sekunden, fahrend und blinkend
```

Warum so? Weil sich damit **alles frei kombinieren lässt**. Fahren und
Blinken und Lichtwechsel laufen gleichzeitig, ohne dass die Sprache dafür
eine eigene Konstruktion bräuchte. Man schaltet Dinge ein, und dann lässt
man Zeit vergehen.

Daraus folgt die Regel, die man sich merken muss:

> **Ein Befehl ohne nachfolgendes `warte` ist unsichtbar.**

`anhalten` direkt vor `von vorn wiederholen` sieht man nie — der Ablauf
springt sofort zurück und fährt weiter. Will man eine Pause sehen, muss
nach dem Anhalten ein `warte` stehen.

**Die Ausnahmen:** Vier Befehle brauchen von sich aus Zeit, weil sie auf
einen Sensor warten — `drehen um … Grad`, `fahren bis Hindernis`,
`drehen bis frei` und `warten bis frei`. Sie sind fertig, wenn das Ziel
erreicht ist, und blockieren den Ablauf so lange.

### 5.2 Die Befehle

#### Fahren

| Befehl | Werte | Wirkung |
|---|---|---|
| **geradeaus fahren** | 1–100 % | beide Motoren gleich stark |
| **Kurve nach links** | Radius 1–10, 1–100 % | **Stufe 1 = engste Kurve**, dreht fast auf der Stelle. Stufe 10 = weiter, sanfter Bogen |
| **Kurve nach rechts** | dto. | |
| **anhalten** | — | Motoren aus |

Jeder Fahrbefehl trägt seine Geschwindigkeit selbst. Einen getrennten
Tempo-Befehl gibt es nicht: Tempowechsel heißt, den nächsten Fahrbefehl
mit einer anderen Zahl einzufügen. Das klingt umständlicher, ist beim
Lesen aber viel klarer — man sieht an jeder Zeile, wie schnell er dabei
ist.

#### Zeit

| Befehl | Werte | Wirkung |
|---|---|---|
| **warte** | 0,1–20,0 s | der einzige Befehl, der für sich genommen Zeit verbraucht |

#### Licht

| Befehl | Werte | Wirkung |
|---|---|---|
| **LED schalten** | Ziel + Zustand | Ziel: einzeln, beide vorne, beide hinten, alle vier. Zustand: ein, aus, blinken |
| **Blinkfrequenz** | Stufe 1–10 | 0,5 bis 5 mal pro Sekunde, gilt für alle blinkenden LEDs |

#### Mit Lagesensor

| Befehl | Werte | Wirkung |
|---|---|---|
| **drehen nach links/rechts um … Grad** | 5–360°, 1–100 % | dreht auf der Stelle, bis der Lagesensor den Winkel meldet |

Das ist der Unterschied zwischen *hoffen* und *wissen*. Ohne Lagesensor
müsste man schreiben „dreh 1,2 Sekunden nach rechts" und darauf
vertrauen, dass das etwa 90° werden — bei vollem Akku mehr, bei leerem
weniger. Mit Sensor dreht er, bis es 90° **sind**.

#### Mit Abstandssensor

| Befehl | Werte | Wirkung |
|---|---|---|
| **fahren bis Hindernis** | 40–1200 mm, 1–100 % | fährt geradeaus, bis etwas näher ist als der Wert |
| **drehen bis frei** | 40–1200 mm, 1–100 % | dreht, bis nach vorn wieder frei ist |
| **warten bis frei** | 40–1200 mm | steht still, bis der Weg frei ist |

#### Ablauf

| Befehl | Werte | Wirkung |
|---|---|---|
| **von vorn wiederholen** | 0–10 | **0 = endlos**, sonst so viele *weitere* Durchläufe |

Dieser Befehl darf nur als letzte Zeile stehen und nur einmal vorkommen.
Die Oberfläche sorgt selbst dafür — neue Befehle rutschen automatisch
davor.

### 5.3 Wie man vorgeht

**Erst das Ziel in Worte fassen, dann in Zeilen.** Das klingt banal, spart
aber die meiste Zeit.

Beispiel: *„Er soll ein Stück geradeaus fahren, rechts abbiegen und dabei
blinken, dann weiter, und das vier Mal."*

Daraus wird Zeile für Zeile:

```
geradeaus mit 50 %
warte 2,0 Sekunden
LED vorne rechts blinken          ← Blinker AN, bevor die Kurve beginnt
drehen nach rechts um 90°, 70 %
LED vorne rechts ausschalten
von vorn wiederholen, 3 mal
```

Sechs Zeilen für ein Quadrat. Drei Beobachtungen daran:

1. **Der Blinker steht vor der Drehung**, nicht danach — sonst blinkt er,
   wenn die Kurve schon vorbei ist.
2. **Nach der Drehung kein `warte`** — der Drehbefehl bringt seine Zeit
   selbst mit.
3. **`3 mal` ergibt vier Runden**: der erste Durchlauf plus drei
   Wiederholungen. Das ist genau ein Quadrat.

### 5.4 Vom Einfachen zum Schwierigen

#### Stufe 1 — nur Licht

```
Blinkfrequenz Stufe 8
alle vier LEDs blinken
warte 3,0 Sekunden
alle vier LEDs ausschalten
warte 1,0 Sekunden
von vorn wiederholen, endlos
```

Kein Fahrbefehl. Ideal zum Begreifen des Zeitmodells, weil man jeden
Schritt direkt sieht und nichts wegrollt.

#### Stufe 2 — geradeaus und zurück zum Stillstand

```
geradeaus mit 45 %
warte 1,5 Sekunden
anhalten
warte 1,0 Sekunden
von vorn wiederholen, endlos
```

Hier lernt man die Regel aus 5.1: Lässt man das letzte `warte` weg, sieht
man vom Anhalten nichts.

#### Stufe 3 — die Acht

```
beide LEDs hinten einschalten
geradeaus mit 55 %
warte 1,5 Sekunden
Kurve rechts, Stufe 4, 55 %
LED vorne rechts blinken
warte 2,5 Sekunden
LED vorne rechts ausschalten
Kurve links, Stufe 4, 55 %
LED vorne links blinken
warte 2,5 Sekunden
LED vorne links ausschalten
geradeaus mit 55 %
warte 1,5 Sekunden
anhalten
alle vier LEDs ausschalten
warte 1,0 Sekunden
von vorn wiederholen, endlos
```

Dieses Programm liegt beim ersten Einschalten schon auf Platz 1. Es ist
zum Auseinandernehmen gedacht: Geschwindigkeiten ändern, Radien ändern,
Wartezeiten ändern und zusehen, was passiert.

#### Stufe 4 — selbsttätig ausweichen

```
fahren bis Hindernis in 120 mm, 60 %
drehen nach rechts, bis 300 mm frei, 70 %
warte 0,3 Sekunden
von vorn wiederholen, endlos
```

Vier Zeilen, und er fährt selbständig durch einen Raum. Das kurze `warte`
gibt dem Abstandssensor nach der Drehung einen Moment, bevor die nächste
Anfahrt beginnt.

Das ist der Punkt, an dem aus einem Ablauf ein **Verhalten** wird: Das
Programm beschreibt nicht mehr eine Strecke, sondern eine Regel — *fahr,
bis etwas im Weg ist; dreh dich weg; von vorn*. Was dabei herauskommt,
hängt vom Raum ab, nicht vom Programm.

### 5.5 Was typischerweise schiefgeht

| Beobachtung | Ursache |
|---|---|
| Programm ist sofort durch, man sieht nichts | `warte` vergessen |
| Er fährt, aber die Lichter bleiben aus | LED-Befehl steht **nach** dem `warte` statt davor |
| Die letzte Aktion ist nie zu sehen | kein `warte` vor `wiederholen` |
| Die Drehung wird zu klein | Geschwindigkeit zu niedrig — eine Drehung auf der Stelle braucht mehr Vibration als Geradeausfahrt |
| Er dreht in die falsche Richtung | Lagesensor um 180° verdreht montiert. Der **Funktionstest** erkennt das und sagt es |
| Beim Speichern eine Fehlermeldung | Werte außerhalb des erlaubten Bereichs. Die Meldung nennt die Zeilennummer, und **nichts wird gespeichert** — das kaputte Programm landet nie im Roboter |

### 5.6 Die Grenze: er kann keine Strecken

Der Roboter weiß, **wie weit er sich gedreht** hat. Er weiß nicht, **wie
weit er gefahren** ist. Es gibt keine Radgeber, und die Beschleunigung
zweimal aufzuintegrieren ist bei einem Gerät, dessen Antriebsprinzip
Schütteln ist, nach ein bis zwei Sekunden wertlos.

**„Fahre 30 cm" gibt es also nicht.** Als Ersatz dienen zwei Dinge:

* `fahren bis Hindernis` — keine Strecke, aber ein definierter Endpunkt
* der **Linienfolger** — die Linie auf dem Boden ist die Wegmessung

Ein Quadrat, das nach vier Runden wieder exakt am Start steht, wird dieser
Roboter nicht fahren. Die Drehungen stimmen, die Strecken streuen.

### 5.7 Für alle, die tiefer wollen

Das Programm liegt im Roboter als kompakte Zeichenkette, zum Beispiel:

```
ld,5,1;ge,55;wa,1500;re,4,55;ld,1,2;wa,2500;…;lo,0
```

Man muss das nicht kennen — die Oberfläche zeigt immer Klartext. Aber es
ist dokumentiert ([08_Fahrprogramm.md](08_Fahrprogramm.md)), samt dem
WebSocket-Protokoll ([06](06_Fernsteuerung-App.md)). Wer eine eigene
Steuerung, ein Skript oder eine Fernbedienung mit echten Knöpfen bauen
will, hat alles, was er dazu braucht.

Und wer noch tiefer will: Der Interpreter steckt in `Program.cpp` und ist
rund 200 Zeilen lang. Er ist bewusst so geschrieben, dass man ihn lesen
kann.

---

## 6 · Weitere Funktionen

### Funktionstest

Ein eingebauter Selbsttest, der alles der Reihe nach durchläuft: die vier
LEDs einzeln, dann alle blinkend, dann Motor links von langsam auf
schnell, dann Motor rechts. Zum Schluss dreht er sich einmal nach rechts
und einmal nach links und prüft dabei den Lagesensor, und er zeigt den
Messwert des Abstandssensors.

Der Teil mit den beiden Drehungen ist der wertvollste: Er prüft nicht nur,
**ob** der Lagesensor antwortet, sondern auch, **in welche Richtung er
zählt**. Ein um 180° verdreht montierter Sensor würde sonst erst beim
ersten Fahrprogramm auffallen — und dort sieht es aus wie ein
Programmierfehler, nicht wie ein Montagefehler.

Beim Motortest leuchtet jeweils die LED der getesteten Seite. Vibriert der
rechte Motor, während links die LED leuchtet, sind die Anschlüsse
vertauscht.

### Schutzfunktionen

| Funktion | Was sie tut |
|---|---|
| **Freigabe** | Nach dem Einschalten ist alles gesperrt. Er fährt nicht von selbst vom Tisch |
| **Totmannschalter** | Im Handbetrieb: kein Befehl für 0,5 s → Motoren aus |
| **Kippschutz** | Liegt er auf der Seite oder wird er hochgehoben, gehen die Motoren aus |
| **Hindernis-Stopp** | Kommt er etwas zu nahe, hält er an. Abschaltbar |
| **NOTHALT** | Ein großer roter Knopf, immer sichtbar |

### Eigener Name

Jeder Rover bekommt einen eigenen Netznamen. Ab Werk hängt die Firmware
eine aus der Seriennummer des Chips abgeleitete Kennung an —
`MarsRover_A3F2` — damit mehrere Geräte nebeneinander funktionieren, ohne
dass jemand etwas einstellen muss.

Wer will, trägt im Experten-Bereich einen eigenen Namen ein. Dann heißt
der Zugangspunkt `MarsRover_Petra`. Der Name wird gespeichert und ist nach
einem Neustart aktiv, den die Oberfläche gleich mit auslöst.

---

## 7 · Technische Daten

| | |
|---|---|
| Rechner | ESP32, 240 MHz, WLAN-Zugangspunkt |
| Antrieb | 2 Vibrationsmotoren, 20 kHz PWM, 1024 Stufen |
| Liniensensorik | 2× TCRT5000, analog, 12 bit |
| Lagesensor | MPU-6050 (optional) |
| Abstandssensor | VL53L0X, 40–1200 mm (optional) |
| Anzeige | 4 LEDs |
| Energie | LiPo 500 mAh, Laden über USB-C |
| Laufzeit | ≈ 1,2 h fahrend, ≈ 2 h im Stand |
| Masse | ≈ 45 g |
| Fahrprogramme | 4 Speicherplätze, je 48 Schritte |
| Bedienung | Browser, iPhone und Android, keine App |
| Materialkosten | ≈ 34 € |

---

## 8 · Was noch kommt

### Bauanleitung mit Bildern

Eine Schritt-für-Schritt-Anleitung vom nackten Bauteil bis zum fahrenden
Roboter, bebildert. Noch nicht geschrieben — sie entsteht beim Bau des
ersten Exemplars, weil eine Anleitung nur dann etwas taugt, wenn sie
jemand beim tatsächlichen Zusammenbauen mitgeschrieben hat.

### 3D-gedrucktes Basismodul

Das Chassis soll gedruckt werden und drei Aufgaben erfüllen:

1. **Die Platine tragen** — mit definierten Befestigungspunkten
2. **Die Zahnbürstenköpfe aufnehmen** — zum Aufrasten, werkzeuglos
   wechselbar, weil die Borsten das Verschleißteil sind
3. **Die Vibrationsmotoren halten** — in definierter Lage, elastisch
   angekoppelt

Das ist **nicht** einfach ein Gehäuse. Am Basismodul hängen die
Eigenschaften, die über Fahren oder Nichtfahren entscheiden:

| Anforderung | Warum |
|---|---|
| Zwei **getrennte** Borstenaufnahmen | Auf einer gemeinsamen Fläche mittelt sich die Vibrationsdifferenz weg und er lenkt nicht mehr |
| Motoren möglichst weit **außen und vorn** | Der Lenkhebel wächst linear mit dem Abstand zur Längsachse |
| Motoren **elastisch** angekoppelt | Starr verschraubt geht die Vibration ins Board statt in die Borsten |
| So **leicht** wie möglich | Masse ist bei diesem Antrieb die Funktionsgrenze, nicht der Komfort |
| Sensoren vorn, definierte **Bodenfreiheit** | Die Reflexkoppler arbeiten bei 2–3 mm optimal |
| Aufnahme für den **Mast** | Abstandssensor, waagerecht nach vorn |
| Akku **mittig und tief** | Schwerpunkt auf der Längsachse, nicht kippelig |

**Platine und Basismodul müssen zusammen entworfen werden** — das eine
legt die Befestigungspunkte, Bauteilhöhen und Sensorpositionen des anderen
fest. Solange das Basismodul nicht steht, ist der endgültige Platinenumriss
offen.

Die heutigen Layoutvorgaben stehen in
[04_PCB-Layout-Empfehlung.md](04_PCB-Layout-Empfehlung.md); sie sind beim
Entwurf des Basismoduls gegenzulesen.

---

## 9 · Ehrlicher Stand

**Nichts davon ist je auf echter Hardware gelaufen.** Die Firmware ist
vollständig und dokumentiert, Schaltplan und Stückliste stehen, die
Bedienoberfläche ist am Bildschirm abgenommen — aber es gibt noch kein
gebautes Gerät.

Das größte offene Risiko ist die **Masse**: rund 45 g, während
funktionierende Roboter dieser Bauart bei 10–25 g liegen. Er wird fahren,
aber vermutlich kriechen statt flitzen. Die Gegenmaßnahmen sind
beschrieben und teilweise kostenlos (dünnere Platine, kleinere LEDs); der
größte Einzelhebel wäre später ein SMD-Modul statt des gesteckten
Entwicklerboards, was 8 g spart und weder Pinbelegung noch Firmware
ändert.

Alle Entscheidungen, auch die verworfenen, stehen mit Begründung im
[Projekt-Log](../00_Projekt-Log.md).
