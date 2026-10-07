# Fertigungsdaten

**Alles in diesem Ordner ist erzeugt.** Die Quelle ist `pcb/MarsRover.kicad_pcb`.
Nach jeder Änderung an der Platine neu erzeugen:

```bash
bash Fertigung/erzeuge_fertigungsdaten.sh
```

Sonst liegen hier Gerber, die nicht mehr zur Platine passen — das ist der
klassische Weg, eine falsche Platine zu bestellen.

Stand: 07.10.2026, DRC 0 Fehler, 0 unverbundene Elemente.

---

## 1 · Was wohin gehört

| Datei | Wofür |
|---|---|
| `MarsRover-gerber.zip` | **Das lädst du beim Leiterplattenhersteller hoch.** Enthält alle Gerber, beide Bohrdateien und die Bohrpläne |
| `MarsRover-pastenschablone.zip` | Für den Schablonenhersteller: `F_Paste` + `Edge_Cuts` als Gerber, dazu DXF und SVG |
| `MarsRover-stueckliste.csv` | Einkaufsliste, nach Wert und Bauform gruppiert |
| `MarsRover-bestueckung.csv` | Bestückungsdaten (Referenz, Wert, Bauform, X, Y, Drehung, Seite) — nur nötig, wenn du maschinell bestücken lässt |
| `MarsRover-platine.step` | 3D-Modell der **nackten** Platine für den Entwurf des Basismoduls |
| `gerber/`, `pastenschablone/` | derselbe Inhalt unverpackt, zum Nachsehen |

Das vollständige 3D-Modell **mit** Bauteilen liegt unter
`local_Step Files/MarsRover-komplett.step` und bleibt lokal: es enthält
Fremdmodelle aus den KiCad- und Espressif-Bibliotheken, die nicht unsere zum
Weitergeben sind.

---

## 2 · Bestellparameter

| | |
|---|---|
| Lagen | **2** |
| Maße | **70 × 52 mm** |
| Dicke | **1,0 mm** — nicht 1,6 mm, das spart gut 4 g, und Masse ist auf diesem Gerät das Hauptproblem |
| Kupfer | 35 µm (1 oz) |
| Dünnste Leiterbahn | **0,40 mm** |
| Kleinster Abstand | **0,20 mm** |
| Durchkontaktierungen | 91 Stück: 0,8/0,4 mm (88×), 0,7/0,4 (2×), 0,6/0,4 (1×) |
| Kleinste Bohrung | **0,40 mm** (Durchkontaktierungen), 0,75 mm bei Bauteilen |
| Nicht durchkontaktierte Löcher | 6 (Mechanik von OS1/OS2 und S1) |
| Oberfläche | **noch nicht entschieden** — siehe unten |
| Lötstopp / Siebdruck | freie Wahl, elektrisch egal |

Diese Werte liegen alle im **Standardprozess** der günstigen Fertiger; nichts
davon löst einen Aufpreis aus.

> **Eine Entscheidung steht noch aus: die Oberfläche.** HASL ist am billigsten,
> ENIG kostet wenige Euro mehr und hat eine ebene Oberfläche. Auf dieser
> Platine sitzen 21 SMD-Bauteile, die über eine Pastenschablone bestückt
> werden — dafür ist eine ebene Fläche besser, weil HASL eine unregelmäßig
> dicke Lotbeule auf jedem Pad hinterlässt und die Schablone darauf kippelt.
> Bei 0805 und SOT-23 geht beides; bei ENIG geht es sicherer.

---

## 3 · Pastenschablone

Nur für die **Oberseite**. Auf der Unterseite sitzen nur die beiden TCRT5000
und der Akkustecker, und die sind bedrahtet — `B_Paste` ist deshalb leer.

- **44 Öffnungen**, verteilt auf 21 Bauteile: 9 Kondensatoren und 10
  Widerstände in 0805, 2 MOSFET in SOT-23.
- Rahmenlos bestellen (Framework/Frameless), Dicke **0,12 mm**. Das ist für
  0805 und SOT-23 der übliche Wert; dünner braucht man erst ab 0,5-mm-Raster.

---

## 4 · Nullpunkt

Gerber, Bohrdaten und Bestückungsliste benutzen **denselben, absoluten
Nullpunkt** (die Blattkoordinaten von KiCad). Das ist die Bedingung dafür,
dass der Fertiger die drei übereinanderlegen kann. Wenn ein Hersteller einen
Hilfsnullpunkt in einer Platinenecke verlangt, muss er für **alle drei**
Ausgaben umgestellt werden, nicht nur für eine.

---

## 5 · Was das DRC noch meldet, und warum es so bleibt

| Meldung | Anzahl | Warum das in Ordnung ist |
|---|---|---|
| `courtyards_overlap` | 21 | Alle unter U1. Das DevKit steckt auf Buchsenleisten gut 8,5 mm über der Platine; darunter liegt absichtlich alles Flache |
| `pth_inside_courtyard` | 4 | Die Pads von D1/D2, gleicher Grund |
| `lib_footprint_mismatch` | 4 | **Gewollte Abweichungen:** U1 trägt ein 3D-Modell, das die Bibliothek nicht hat, M1/M2 ein Feld „Hinweis" mit der Lötaugen-Begründung. Ohne Auswirkung auf die Fertigung — die Gerber kommen aus der Platinendatei, nicht aus der Bibliothek |

---

## 6 · Vor dem Bestellen prüfen

- [ ] **Reihenabstand des DevKits mit dem Messschieber nachmessen.** Soll:
      **25,4 mm**. Die 30-polige Version hat ein anderes Rastermaß, dann passt
      die Platine nicht. Siehe [Doku/09](../Doku/09_Boardwahl-DevKitC-V4.md)
- [ ] Oberfläche entscheiden (Abschnitt 2)
- [ ] Gerber in einem Betrachter gegenlesen — der Fertiger prüft nicht mit
- [ ] **MT3608 umbauen, bevor er eingelötet wird:** Poti raus, 110 kΩ / 15 kΩ
      rein. Siehe [Doku/07 Schritt 1a](../Doku/07_Inbetriebnahme-und-Tuning.md)
