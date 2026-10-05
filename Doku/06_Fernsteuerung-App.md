# Fernsteuerung und App

Anforderung: Steuerung muss **auf Android und iPhone** funktionieren.

---

## 1 · Warum es nicht BluetoothSerial geworden ist

`BluetoothSerial` im ESP32-Arduino-Core ist klassisches Bluetooth **SPP** (Serial
Port Profile). Apple gibt SPP auf iOS für Fremdgeräte nicht frei — dafür braucht
man eine MFi-Lizenz und einen Apple-Authentifizierungschip im Gerät. Auf Android
funktioniert SPP tadellos, auf dem iPhone grundsätzlich nicht.

Es bleiben damit genau zwei plattformübergreifende Wege: **BLE** oder **WLAN**.

| | WLAN-AP + eigene Web-App | BLE + fertige App |
|---|---|---|
| iPhone | ✅ Browser | ✅ |
| Android | ✅ Browser | ✅ |
| Installation nötig | **keine** | App aus dem Store |
| Oberfläche gestaltbar | **vollständig** | nur was die App anbietet |
| Telemetrie zurück | **beliebig** (Sensorbalken, Hz, Akku) | Textzeilen |
| Live-Tuning (Kp, Kd, Schwellen) | **ja, mit Schiebereglern** | nur über Zahleneingabe |
| Mehrverbrauch | ≈ 80 mA | ≈ 15 mA |
| Handy bleibt im Heim-WLAN | nein | ja |

**Gewählt: WLAN-AP mit eigener Web-App.** Der Mehrverbrauch kostet rund 20 %
Laufzeit, dafür gibt es eine passgenaue Oberfläche, Live-Telemetrie und
Reglerabstimmung im Betrieb — und null Installationsaufwand auf fremden Geräten.

---

## 2 · Die gebaute Web-App

Der ESP32 spannt einen eigenen Access Point auf und liefert die Oberfläche selbst
aus. Quelle: [`Firmware/bristlebot/WebUI.h`](../Firmware/bristlebot/WebUI.h),
Serverlogik in [`RemoteControl.cpp`](../Firmware/bristlebot/RemoteControl.cpp).

### Benutzung

1. Roboter einschalten.
2. Am Handy ins WLAN **`Bristlebot`** verbinden, Passwort **`bristlebot`**.
3. Die Bedienseite klappt meist von selbst auf (Captive-Portal-Erkennung).
   Falls nicht: Browser öffnen, **`http://192.168.4.1`** eingeben.

> Auf dem iPhone meldet iOS „Kein Internet" — das ist korrekt, der Roboter hat
> keine Internetverbindung. Die Meldung bestätigen und im Netz bleiben.
> Reagiert der Joystick im automatisch aufklappenden Fenster nicht, dieses
> Fenster schließen und die Seite in Safari direkt über die IP aufrufen. Das
> Portal-Fenster von iOS ist eine eingeschränkte WebView.

### Was die Oberfläche kann

| Element | Funktion |
|---|---|
| Statuspunkt oben | grün = WebSocket verbunden |
| **Freigeben / Motoren sperren** | Nach dem Einschalten ist der Roboter **gesperrt**. Ohne Freigabe läuft kein Motor — verhindert, dass er beim Einschalten vom Tisch fährt |
| Manuell / Autonom | Betriebsart. Beim Wechsel stoppen die Motoren immer erst |
| Joystick | hoch = schneller, seitlich = lenken. Loslassen = Stillstand. Bei Stillstand und vollem Seitenausschlag: Drehen auf der Stelle |
| Grundvibration | Basisgeschwindigkeit im autonomen Modus |
| Kp / Kd | Reglerabstimmung live, siehe [07](07_Inbetriebnahme-und-Tuning.md) |
| Anlaufschwelle | PWM-Mindestwert, ab dem die Motoren überhaupt anlaufen |
| Linie kalibrieren | 5-Sekunden-Kalibrierfahrt, Werte landen im NVS-Flash |
| Sensorbalken L/R, Ablage | Live-Messwerte — das wichtigste Werkzeug beim Einrichten |
| Motorbalken L/R | tatsächlich ausgegebener PWM-Duty |
| Statuszeile | Phase, Kalibrierzustand, Schleifenfrequenz, ggf. Akkuspannung |
| **NOTHALT** | sperrt sofort und hebt die Freigabe auf |

### Technische Eckpunkte

* **Alles inline.** Kein CDN, keine Webfonts, kein externes Skript — im
  AP-Betrieb gibt es keinen Internetzugang, nachgeladene Ressourcen würden
  einfach fehlen.
* **WebSocket auf Port 81** für Steuerbefehle und Telemetrie; die Seite selbst
  kommt per HTTP von Port 80. Die App sendet mit 10 Hz, der Roboter antwortet
  mit ca. 6,7 Hz Telemetrie.
* **Totmannschalter.** Kommt im Handbetrieb länger als 500 ms kein Paket, stoppen
  die Motoren. Deckt abgeschaltetes Display, Browser im Hintergrund,
  weggelaufene WLAN-Verbindung und geschlossenen Tab ab.
* **Captive Portal per DNS.** Ein DNS-Server auf Port 53 löst jeden Namen auf
  192.168.4.1 auf, und der Webserver antwortet auf *jede* URL mit der App. Damit
  schlägt die Portalerkennung von iOS (`captive.apple.com`) und Android
  (`/generate_204`) an und das Bedienfenster öffnet sich selbst.
* **iOS-Safari-Eigenheiten berücksichtigt:** Pointer Events statt Touch Events,
  `touch-action: none` auf dem Joystickfeld, `overscroll-behavior: none` gegen
  das Gummiband-Scrollen, `gesturestart` und `dblclick` abgefangen gegen
  Pinch- und Doppeltipp-Zoom, `maximum-scale=1`.
* **Maximal 4 Clients**, Sendeleistung auf 11 dBm reduziert (spart Strom, im
  Zimmer mehr als ausreichend).

### Passwort und SSID ändern

In [`Config.h`](../Firmware/bristlebot/Config.h):

```cpp
static const char AP_SSID[]     = "Bristlebot";
static const char AP_PASSWORD[] = "bristlebot";  // min. 8 Zeichen!
```

WPA2 braucht mindestens 8 Zeichen. Ein leerer String gibt einen offenen AP —
dann klappt das Portal etwas zuverlässiger auf, aber jeder in Reichweite kann
den Roboter fahren.

---

## 3 · Protokoll

Textzeilen über den WebSocket, kommagetrennt. Dokumentiert, falls du später eine
native App, ein Skript oder eine Fernsteuerung mit echten Knöppen anbinden willst.

### App → Roboter

| Befehl | Bedeutung |
|---|---|
| `J,<x>,<y>` | Joystick, je −1 … +1. `y ≤ 0` heißt Halt |
| `M,0` / `M,1` | Betriebsart manuell / autonom |
| `A,0` / `A,1` | sperren / freigeben |
| `X` | Nothalt |
| `C` | Kalibrierfahrt starten |
| `R` | Kalibrierung löschen |
| `T,<kp>,<kd>,<base>,<min>` | Tuningwerte setzen (werden 1,5 s später ins NVS gesichert) |
| `P` | Ping, hält den Totmannschalter wach |

### Roboter → App

Eine JSON-Zeile je Telemetrieintervall:

```json
{"p":1,"m":1,"a":1,"sL":0.82,"sR":0.11,"e":-0.71,
 "dL":512,"dR":430,"dMax":613,"calOk":1,"hz":2840}
```

| Feld | Bedeutung |
|---|---|
| `p` | Phase: 0 gesperrt, 1 fährt, 2 sucht Linie, 3 Linie verloren, 4 kalibriert, 5 Akku leer, 6 wartet auf Befehl |
| `m` | Betriebsart (0 manuell, 1 autonom) |
| `a` | freigegeben |
| `sL`, `sR` | „Linienanteil" je Sensor, 0 … 1 |
| `e` | Regelfehler, > 0 = Linie liegt rechts |
| `dL`, `dR`, `dMax` | PWM-Duty und zulässiges Maximum |
| `calOk` | Kalibrierung gültig |
| `hz` | Schleifenfrequenz — guter Gesundheitswert |
| `vb` | Akkuspannung, nur mit `FEATURE_BATTERY_MONITOR` |

Beim Verbindungsaufbau schickt der Roboter zusätzlich einmal
`{"t":"cfg","kp":…,"kd":…,"base":…,"min":…}`, damit die Schieberegler sofort
richtig stehen.

---

## 4 · Fertige Apps als Rückfallebene

Falls du den WLAN-Weg doch nicht willst — etwa weil du im Betrieb im Heim-WLAN
bleiben möchtest — sind das die beiden brauchbaren Store-Apps. Beide setzen
**BLE** voraus, also einen Umbau der Firmware von WLAN auf BLE-UART
(`NimBLE-Arduino`, Nordic-UART-Service). Das Protokoll aus Abschnitt 3 kann
dabei unverändert bleiben.

| App | iOS | Android | Eignung |
|---|---|---|---|
| **Dabble** (STEMpedia) | ab iOS 11 | ja | Gamepad-Modul mit **analogem Joystick**, dazu Terminal und Regler. Nächstes Äquivalent zur gebauten Oberfläche. Es gibt eine passende Arduino-Bibliothek `DabbleESP32`, die den BLE-Teil übernimmt. **Wichtig:** die iOS-Version unterstützt ausschließlich BLE |
| **Bluefruit Connect** (Adafruit) | ab iOS 15 | ab Android 4.4 | „Controller"-Modus mit 8-Tasten-Gamepad (digital, kein analoger Joystick) plus Beschleunigungssensor. Dazu ein UART-Terminal, mit dem sich das Protokoll von Hand testen lässt — dafür auch unabhängig vom Steuerweg nützlich |

Für die reine Entwicklungsarbeit lohnt **nRF Connect** (Nordic, iOS und Android):
damit siehst du rohe BLE-Dienste und -Charakteristiken und kannst Pakete
von Hand schicken.

Einschränkung von Dabble und Bluefruit gegenüber der eigenen Web-App: Beide
bieten kein frei gestaltbares Telemetrie-Display. Die Sensorbalken und die
Live-Schieberegler für Kp/Kd müsstest du aufgeben oder in Textzeilen im
Terminal nachbilden.

---

## Quellen

- [Dabble – Bluetooth Controller im App Store](https://apps.apple.com/gb/app/dabble-bluetooth-controller/id1472734455)
- [STEMpedia/DabbleESP32 (Arduino-Bibliothek, BLE)](https://github.com/STEMpedia/DabbleESP32)
- [Dabble – Erste Schritte (STEMpedia-Doku, iOS nur BLE)](https://ai.thestempedia.com/docs/dabble-app/getting-started-with-dabble/)
- [Bluefruit Connect im App Store](https://apps.apple.com/us/app/bluefruit-connect/id830125974)
- [Adafruit Learn: Bluefruit LE Connect – Controller-Modus](https://learn.adafruit.com/introducing-the-adafruit-bluefruit-le-uart-friend/controller)
