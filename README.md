# One-of-a-Kind Mars Rover

> Dieses Projekt wurde mit Unterstützung von [Claude Code](https://claude.com/claude-code) entwickelt.

Linienfolgender, programmierbarer Zahnbürsten-Roboter auf ESP32 (Bauart:
Bristlebot), aufgebaut auf einer eigenen Platine. Jedes Exemplar bekommt
einen eigenen Namen und ein eigenes WLAN (`MarsRover_Petra`), damit mehrere
nebeneinander fahren können.

**[→ Firmware direkt im Browser installieren](https://darklirah.github.io/One-of-a-Kind-Mars-Rover/)**

Die ausführliche Projektbeschreibung samt Programmier-Kapitel steht in
[Doku/00_Projektbeschreibung.md](Doku/00_Projektbeschreibung.md).

Zwei Vibrationsmotoren treiben zwei getrennte Borstenfelder an;
differentielle Vibrationsintensität lenkt. Zwei Reflexkoppler lesen die Linie,
ein PD-Regler hält ihn darauf. Gesteuert wird er über einen WLAN-Access-Point,
den der ESP32 selbst aufspannt — die Bedienoberfläche liefert er gleich mit.

**Status:** Hardware entworfen und geprüft, Firmware vollständig und
**fehlerfrei kompiliert** (0 Fehler, 0 Warnungen mit `-Wall`;
RAM 15,2 %, Flash 69,8 %) — **aber noch nie auf echter Hardware gelaufen.**

---

## Eckdaten

| | |
|---|---|
| Rechenkern | ESP32 DevKit V1, 30-Pin, gesteckt |
| Antrieb | 2× Vibrationsmotor 3 V über BC337-40, 20 kHz PWM (LEDC) |
| Sensorik | 2× TCRT5000 analog an ADC1 (GPIO 34/35) |
| Lage | MPU-6050 am I²C: Drehen nach Winkel, Kurs halten, Kippschutz |
| Abstand | VL53L0X auf einem Mast: Hindernisse erkennen und umfahren |
| Anzeige | 4 LEDs: gelb vorne = Lenkrichtung, rot hinten = Status |
| Energie | LiPo 500 mAh, TP4056 über USB-C, MT3608 auf 5 V |
| Laufzeit | ≈ 1,2 h fahrend, ≈ 2,0 h Standby |
| Betriebsarten | Linie folgen · selbst fahren · Fahrprogramm |
| Fahrprogramme | 4 Speicherplätze im Flash, je 48 Schritte, Baukasten-Editor |
| Fernsteuerung | WLAN-AP + eigene Web-App, läuft auf iPhone **und** Android im Browser |
| Materialkosten | ≈ 26,50 € im Einzelstück |
| Masse | ≈ 40 g (größtes technisches Risiko, siehe Doku) |

## Bedienung in drei Schritten

1. Einschalten
2. Am Handy ins WLAN **`MarsRover_…`**, Passwort **`marsrover`**
3. Die Seite klappt von selbst auf — falls nicht: **`http://192.168.4.1`**

Nach dem Einschalten ist der Roboter **gesperrt**. Erst *Freigeben* in der App
lässt die Motoren laufen.

---

## Dokumentation

| Datei | Inhalt |
|---|---|
| [00 Projektbeschreibung](Doku/00_Projektbeschreibung.md) | Was das Projekt ist, alle Funktionen, ausführliches Kapitel zum Programmieren |
| [01 Schaltplan-Review](Doku/01_Schaltplan-Review.md) | Prüfung des Entwurfs: 5 Blocker, 9 Verbesserungen, Änderungsliste |
| [02 SMD vs. THT](Doku/02_SMD-vs-THT-Wirtschaftlichkeit.md) | Kostenvergleich bei 1/10/100 Stück, Massebudget, Laufzeitrechnung |
| [03 Stückliste](Doku/03_Stueckliste-BOM.md) | Vollständige BOM mit Bauteilkürzeln, Preisen und Einkaufsfallen |
| [04 PCB-Layout](Doku/04_PCB-Layout-Empfehlung.md) | Fertigungsparameter, Masseführung, Platzierung, Mechanik |
| [05 Pinbelegung](Doku/05_Pinbelegung.md) | Pinplan mit Begründungen, freie Pins, Vorzeichenkonventionen |
| [06 Fernsteuerung](Doku/06_Fernsteuerung-App.md) | Warum WLAN und nicht Bluetooth, Protokoll, fertige Apps als Rückfallebene |
| [07 Inbetriebnahme](Doku/07_Inbetriebnahme-und-Tuning.md) | Schrittweiser Aufbau, Kalibrierung, Reglerabstimmung, Fehlersuche |
| [Netzliste](Hardware/Netzliste.md) | Verbindungsliste als Vorlage für KiCad |
| [Projekt-Log](00_Projekt-Log.md) | Entscheidungen und offene Punkte |

## Firmware

```
Firmware/
  platformio.ini
  bristlebot/
    bristlebot.ino      Zustandsmaschine, PD-Regler, Telemetrie
    Config.h            Pins, Kennwerte, Feature-Flags  <- hier tunen
    RobotState.h        Betriebsart, Phase, Tuningstruktur
    Motors.*            PWM, Kennlinienspreizung, Kickstart, Duty-Begrenzung
    LineSensor.*        Median-Filter, Kalibrierung in NVS
    Leds.*              Blinkmuster
    RemoteControl.*     Access Point, Webserver, WebSocket, Protokoll
    WebUI.h             Bedienoberfläche, komplett inline
```

**Einzige externe Abhängigkeit:** `WebSockets` von Markus Sattler
(„arduinoWebSockets") ≥ 2.4.1. Läuft mit ESP32-Arduino-Core 2.x und 3.x — die
umgestellte LEDC-API wird per `#if` abgefangen.

Board: *ESP32 Dev Module*, 4 MB Flash, Partition *Default 4MB with spiffs*.

---

## Die wichtigsten vier Warnungen

1. **MT3608 vor dem ersten Anschluss des ESP32 im Leerlauf auf 5,00 V trimmen.**
   Die Module kommen mit beliebiger Einstellung, oft über 20 V. Das tötet das
   DevKit sofort.
2. **R_prog am TP4056 tauschen** — Werkszustand 1 A ist für eine 500-mAh-Zelle
   mehr als das Doppelte des Zulässigen. 4,7 kΩ ergibt 255 mA.
3. **USB-C braucht 2× 5,1 kΩ an CC1 und CC2 nach GND**, sonst liefert ein
   USB-C-Netzteil keine Spannung.
4. **Zwei getrennte Borstenfelder, eines unter jedem Motor.** Auf einer
   gemeinsamen Grundplatte mittelt sich die Vibrationsdifferenz weg und der
   Roboter lenkt praktisch nicht.

## Lizenz

[GNU General Public License v3.0](LICENSE)
