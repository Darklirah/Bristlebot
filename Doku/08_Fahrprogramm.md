# Fahrprogramme

Im dritten Modus fährt der Roboter einen gespeicherten Ablauf ab. Die
Programme werden in der Weboberfläche zusammengeklickt und liegen in **vier
Speicherplätzen im Flash** — sie überleben Stromausfall und ein Neuflashen
der Firmware.

---

## 1 · Das Zeitmodell — der wichtigste Punkt

**Jeder Befehl wirkt sofort und die nächste Zeile kommt unmittelbar dran.
Zeit verbraucht ausschließlich `warte`.**

Das klingt zunächst ungewohnt, ist aber genau das, was Fahren und Lichter
frei kombinierbar macht:

```
Kurve rechts, Stufe 4 (mittel), 55 %   <- schaltet die Fahrt um, dauert 0 s
LED vorne rechts blinken               <- schaltet den Blinker ein, dauert 0 s
warte 2,5 Sekunden                     <- JETZT vergehen 2,5 s, und zwar
                                          fahrend und blinkend
```

Daraus folgt die Regel, die am Anfang am häufigsten übersehen wird:

> **Ein Befehl ohne nachfolgendes `warte` ist unsichtbar.**

`anhalten` direkt vor `von vorn wiederholen` sieht man nie — der Ablauf
springt sofort zurück. Deshalb steht im mitgelieferten Beispiel nach dem
Anhalten noch ein `warte 1,0 Sekunden`.

---

## 2 · Die Befehle

### Fahren

Jeder Fahrbefehl trägt seine Geschwindigkeit selbst. Es gibt keinen
getrennten Tempo-Befehl — Tempowechsel heißt einfach, den nächsten
Fahrbefehl mit einem anderen Prozentwert einzufügen.

| Befehl | Werte | Was er tut |
|---|---|---|
| **geradeaus fahren** | Geschwindigkeit 1–100 % | Beide Motoren gleich stark, Roboter fährt an. |
| **Kurve nach links** | Radius 1–10, Geschwindigkeit 1–100 % | Fährt eine Linkskurve. **Stufe 1 = engste Kurve** (dreht fast auf der Stelle), Stufe 10 = weite, sanfte Kurve. |
| **Kurve nach rechts** | Radius 1–10, Geschwindigkeit 1–100 % | Spiegelbildlich. |
| **anhalten** | — | Motoren aus. Geschwindigkeit und Richtung bleiben gespeichert; ein späteres `geradeaus` fährt mit dem dort angegebenen Tempo wieder los. |

### Zeit

| Befehl | Werte | Was er tut |
|---|---|---|
| **warte** | 0,1 – 20,0 s | Der einzige Befehl, der Zeit verbraucht. Alles, was vorher eingestellt wurde, läuft dabei weiter. |

### Licht

| Befehl | Werte | Was er tut |
|---|---|---|
| **LED schalten** | Ziel + Zustand | Ziel: vorne links, vorne rechts, hinten links, hinten rechts, beide vorne, beide hinten, alle vier. Zustand: einschalten, ausschalten, blinken. |
| **Blinkfrequenz einstellen** | Stufe 1–10 | Gilt für alle blinkenden LEDs gemeinsam. Stufe 1 = 0,5 mal pro Sekunde, Stufe 10 = 5 mal pro Sekunde. |

### Ablauf

| Befehl | Werte | Was er tut |
|---|---|---|
| **von vorn wiederholen** | 0–10 | **0 = endlos.** 1–10 = so viele *weitere* Durchläufe; bei `3` läuft das Programm also insgesamt viermal. |

---

## 3 · Ablauf: ein Programm anlegen

So geht es in der Oberfläche, Schritt für Schritt.

**1. Modus wählen.** Oben auf **Fahrprogramm** tippen.

**2. Speicherplatz wählen.** Vier Kacheln, jede zeigt ihren Namen und wie
voll sie ist: *„17 von 48 Schritten"* oder *„leer"*. Antippen lädt den
Platz in den Editor.

**3. Namen vergeben.** Ins Namensfeld tippen, z. B. „Rechteck".

**4. Befehle einfügen.** Im Kasten *Befehl hinzufügen*:

- Befehl aus der Liste wählen — sie ist nach **Fahren / Zeit / Licht /
  Ablauf** gruppiert
- darunter erscheinen genau die passenden Schieberegler, immer mit Klartext
  daneben: *„Stufe 4 – mittel"*, *„2,0 mal pro Sekunde"*, *„endlos"*
- **+ Einfügen** antippen

Der Schritt landet unten in der Liste. Der Zähler darüber wandert mit:
*„5 von 48 belegt · 43 frei"*.

**5. Reihenfolge korrigieren.** Jede Zeile hat **↑ ↓ ✕** zum Verschieben
und Löschen. Die Farbe am linken Rand zeigt die Art: blau = Fahren,
grau = Warten, gelb = Licht, violett = Wiederholen.

**6. Speichern.** Der Roboter prüft das Programm, legt es im gewählten
Platz ab und meldet *„Gespeichert: 17 Schritte"*. Ist etwas faul, kommt
stattdessen eine konkrete Meldung wie *„Schritt 4: Radius muss 1..10 sein"*
und **nichts wird geschrieben** — ein kaputtes Programm landet nie im Flash.

**7. Freigeben und starten.** Oben **Freigeben**, dann unten
**▶ Programm starten**. Der laufende Schritt wird in der Liste **grün
hervorgehoben**, darunter steht *„Schritt 7 von 17 · Durchlauf 2"*.

**8. Stoppen.** Derselbe Knopf, jetzt rot: **■ Programm stoppen**. Oder der
**NOTHALT** ganz unten — der stoppt das Programm und sperrt zusätzlich die
Motoren.

> Zwei Dinge nimmt die Oberfläche einem von selbst ab: **von vorn
> wiederholen** gibt es nur einmal und immer als letzte Zeile — neue
> Befehle werden automatisch davor einsortiert. Und Tippfehler sind nicht
> möglich, weil nichts getippt wird.

---

## 4 · Das mitgelieferte Beispiel

Beim allerersten Start liegt in Platz 1 **„Beispiel: Achter"**. Zum Starten,
Anschauen und Umbauen gedacht.

| # | Zeile | Was dabei passiert |
|---|---|---|
| 1 | beide LEDs hinten einschalten | Rücklichter an, bleiben das ganze Programm über an |
| 2 | geradeaus mit 55 % | fährt an |
| 3 | warte 1,5 Sekunden | **1,5 s geradeaus** |
| 4 | Kurve rechts, Stufe 4 (mittel), 55 % | auf Rechtsbogen umschalten |
| 5 | LED vorne rechts blinken | Blinker rechts an |
| 6 | warte 2,5 Sekunden | **2,5 s Rechtsbogen mit Blinker** |
| 7 | LED vorne rechts ausschalten | Blinker aus |
| 8 | Kurve links, Stufe 4 (mittel), 55 % | Bogen umkehren |
| 9 | LED vorne links blinken | Blinker links an |
| 10 | warte 2,5 Sekunden | **2,5 s Linksbogen** — zusammen mit 6 ergibt das die Acht |
| 11 | LED vorne links ausschalten | |
| 12 | geradeaus mit 55 % | |
| 13 | warte 1,5 Sekunden | **1,5 s geradeaus** |
| 14 | anhalten | Motoren aus |
| 15 | alle vier LEDs ausschalten | |
| 16 | warte 1,0 Sekunden | **1 s sichtbare Pause** — ohne diese Zeile wäre vom Anhalten nichts zu sehen |
| 17 | von vorn wiederholen, endlos | zurück zu Zeile 1 |

### Zum Umbauen

- **Zu schnell?** Zeilen 2, 4, 8 und 12 auf 40 % ziehen.
- **Zu enge Acht?** Zeilen 4 und 8 auf Stufe 7 — weitere Bögen.
- **Langsamer in den Kurven?** Nur Zeilen 4 und 8 auf 40 %, der Rest bleibt
  bei 55 %. Genau dafür hängt die Geschwindigkeit am Fahrbefehl.
- **Nur dreimal fahren?** Zeile 17 auf 3 stellen (= vier Durchläufe).
- **Nur einmal?** Zeile 17 löschen. Ohne `wiederholen` endet das Programm
  nach dem letzten Schritt, und in der Oberfläche steht „Programm
  durchgelaufen".

---

## 5 · Zwei weitere Beispiele

### Rechteck fahren

```
geradeaus mit 50 %
warte 2,0 Sekunden
Kurve rechts, Stufe 1 (sehr eng), 70 %
warte 1,2 Sekunden
von vorn wiederholen, 3 mal
```

Fünf Schritte. Stufe 1 dreht fast auf der Stelle, die 1,2 Sekunden sind die
Vierteldrehung, und die 70 % geben ihr genug Schwung — eine Drehung auf der
Stelle braucht mehr Vibration als Geradeausfahrt. **Diese Zeit musst du
erfahren**: je nach Untergrund und Akkustand liegt sie zwischen etwa 0,8 und
2 Sekunden. Vorgehen: Wert eintragen, laufen lassen, zuschauen, anpassen.

### Lichtshow im Stand

```
Blinkfrequenz Stufe 8 (4,0 mal pro Sekunde)
alle vier LEDs blinken
warte 3,0 Sekunden
alle vier LEDs ausschalten
LED vorne links einschalten
warte 0,5 Sekunden
LED vorne rechts einschalten
warte 0,5 Sekunden
LED hinten rechts einschalten
warte 0,5 Sekunden
LED hinten links einschalten
warte 0,5 Sekunden
alle vier LEDs ausschalten
warte 1,0 Sekunden
von vorn wiederholen, endlos
```

Ohne jeden Fahrbefehl — ein Programm muss nicht fahren. Gut zum Üben des
Zeitmodells, weil man jeden Schritt direkt sieht.

---

## 6 · Regeln und Grenzen

| Regel | Warum |
|---|---|
| **48 Schritte** je Speicherplatz | Platz im Flash. Der Zähler in der Oberfläche zeigt jederzeit, wie viel frei ist. |
| **„von vorn wiederholen" nur als letzte Zeile**, höchstens einmal | Es gibt keine Sprungmarken — wiederholt wird immer das ganze Programm. Die Oberfläche sortiert neue Befehle automatisch davor ein. |
| Ein wiederholtes Programm **braucht mindestens ein `warte`** | Sonst liefe es ohne Pause im Kreis und würde die Hauptschleife des ESP32 blockieren. Die Firmware weist so ein Programm beim Speichern ab. |
| **Keine Freigabe, kein Start** | Ohne *Freigeben* läuft kein Motor; das Programm startet deshalb gar nicht erst. |
| Betriebsart wechseln **stoppt** das Programm | Sonst liefe im Hintergrund etwas weiter, das man nicht mehr sieht. |
| Funktionstest und Fahrprogramm **schließen sich aus** | Beide greifen auf Motoren und LEDs zu. Startet man das eine, stoppt das andere. |
| Name höchstens **20 Zeichen** | |

### Keine Wegmessung

Der Roboter hat weder Radgeber noch Kompass. Ein Fahrprogramm läuft rein
nach der Uhr — **dieselben 2 Sekunden sind je nach Akkustand, Untergrund
und Borstenzustand eine unterschiedliche Strecke.** Bei einem
vibrationsgetriebenen Antrieb ist diese Streuung erheblich, deutlich größer
als bei einem Roboter mit Rädern.

Praktisch heißt das: wiederholgenaue Figuren sind damit nicht zu machen.
Abläufe wie „anfahren, Bogen, blinken, anhalten" funktionieren gut. Ein
Rechteck, das nach vier Runden wieder am Start steht, wird es nicht geben —
dafür bräuchte es Sensorik, die der Roboter nicht hat.

Wer exakte Figuren will, nimmt den **Linienfolger**: die Linie auf dem
Boden ist die Wegmessung.

---

## 7 · Textformat

Intern und auf dem Draht ist ein Programm eine kompakte Zeichenkette. Du
musst das nicht kennen — die Oberfläche zeigt immer Klartext. Für eigene
Werkzeuge, Sicherungskopien oder zum Nachschauen im Log:

```
ld,5,1;ge,55;wa,1500;re,4,55;ld,1,2;wa,2500;ld,1,0;li,4,55;ld,0,2;wa,2500;ld,0,0;ge,55;wa,1500;st;ld,6,0;wa,1000;lo,0
```

| Kürzel | Bedeutung | Argumente |
|---|---|---|
| `ge` | geradeaus | Geschwindigkeit 1–100 |
| `li` / `re` | Kurve links / rechts | Radius 1–10, Geschwindigkeit 1–100 |
| `st` | anhalten | — |
| `wa` | warten | Millisekunden, 50–60000 |
| `ld` | LED schalten | Ziel 0–6, Zustand 0 = aus / 1 = an / 2 = blinken |
| `bf` | Blinkfrequenz | Stufe 1–10 |
| `lo` | wiederholen | 0 = endlos, 1–10 |

LED-Ziele: `0` vorne links, `1` vorne rechts, `2` hinten links,
`3` hinten rechts, `4` beide vorne, `5` beide hinten, `6` alle vier.

Die Firmware speichert immer ihre eigene, kanonische Fassung — nicht den
Rohtext aus der App. Das Protokoll der WebSocket-Befehle steht in
[06_Fernsteuerung-App.md](06_Fernsteuerung-App.md), der eingebaute
Funktionstest in [07_Inbetriebnahme-und-Tuning.md](07_Inbetriebnahme-und-Tuning.md).
