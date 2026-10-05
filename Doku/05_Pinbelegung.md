# Pinbelegung — ESP32 DevKit V1, 30-Pin

Maßgeblich ist `Firmware/bristlebot/Config.h`. Diese Tabelle muss damit
übereinstimmen; im Zweifel gilt die Header-Datei.

---

## Belegte Pins

| GPIO | Richtung | Funktion | Bauteile | Kennwerte |
|---|---|---|---|---|
| **32** | PWM out | Motor **links** | R1 100 Ω → Gate Q1, R3 100 kΩ nach GND | 20 kHz, 10 bit, Duty ≤ 613 |
| **33** | PWM out | Motor **rechts** | R2 100 Ω → Gate Q2, R4 100 kΩ nach GND | 20 kHz, 10 bit, Duty ≤ 613 |
| **34** | Analog in | Liniensensor **links** | Emitter OS1, R7 10 kΩ nach GND | ADC1_CH6, 12 bit, 11 dB |
| **35** | Analog in | Liniensensor **rechts** | Emitter OS2, R8 10 kΩ nach GND | ADC1_CH7, 12 bit, 11 dB |
| **14** | Digital out | LED gelb vorne **links** | R9 220 Ω → LED1 → GND | ≈ 6 mA |
| **27** | Digital out | LED gelb vorne **rechts** | R10 220 Ω → LED2 → GND | ≈ 6 mA |
| **26** | Digital out | LED rot hinten **links** | R11 220 Ω → LED3 → GND | ≈ 6 mA |
| **25** | Digital out | LED rot hinten **rechts** | R12 220 Ω → LED4 → GND | ≈ 6 mA |
| 39 | Analog in | *optional* Akkuspannung | R16/R17 je 100 kΩ Teiler | ADC1_CH3, `FEATURE_BATTERY_MONITOR` |
| **21** | I2C SDA | Lage- und Abstandssensor | U6 GY-521, U7 GY-530 | 400 kHz, Pull-ups auf den Modulen |
| **22** | I2C SCL | dto. | dto. | |
| 4 | Digital in | *optional* Taster Betriebsart | gegen GND, interner Pullup | `FEATURE_MODE_BUTTON` |

## Versorgung

| Pin | Funktion |
|---|---|
| **VIN (5V)** | Ausgang von U5 (MT3608, auf 5,00 V getrimmt) |
| **3V3** | Ausgang des bordeigenen AMS1117. Versorgt die IR-LEDs von OS1/OS2 (über R5/R6) und die Kollektoren der Fototransistoren |
| **GND** | gemeinsame Masse, mindestens zwei Pins anbinden |
| EN | nicht belegt |

---

## Warum diese Pins — und nicht die aus dem ersten Entwurf

### Motoren: 32/33 statt 12/13

GPIO 12 ist der Strapping-Pin **MTDI**. Der ROM-Bootloader liest ihn beim Reset
und legt daraus die Flash-Betriebsspannung fest. Wird der Pin in diesem Moment
hochgezogen, erwartet der Chip 1,8-V-Flash und bootet nicht. Eine Transistorbasis
über 1 kΩ ist genau so ein möglicher Pfad.

GPIO 32 und 33 haben keine Strapping-Funktion, geben beim Booten keinen Takt aus
und sind voll LEDC-fähig.

### Sensoren: nur ADC1

Der ESP32 hat zwei ADC-Blöcke. **ADC2 ist bei aktivem WLAN gesperrt** — der
WLAN-Treiber belegt ihn, `analogRead()` liefert dann Müll oder blockiert. Da die
Fernsteuerung ein WLAN-Access-Point ist, sind nur ADC1-Pins nutzbar:

> GPIO 32, 33, 34, 35, 36, 39

GPIO 32/33 sind für die Motoren vergeben. Übrig bleiben 34, 35, 36, 39 — alle
vier sind **reine Eingänge** ohne interne Pull-Widerstände und ohne
Ausgangstreiber, also ideal für Sensoren und für nichts anderes verwendbar.
Gewählt: 34 und 35, mit 39 als Reserve für die Akkumessung.

### LEDs: unverändert

14 / 27 / 26 / 25 wie in deinem Entwurf. Einzige Einschränkung: GPIO 14 gibt
während des Flash-Bootvorgangs kurz ein Taktsignal aus, die gelbe LED vorne
links flackert deshalb beim Einschalten einmal auf. Das ist kosmetisch; wer es
nicht will, legt LED1 auf GPIO 18 oder 19 (beide frei).

---

## Freie Pins für Erweiterungen

| GPIO | Eignung |
|---|---|
| 18, 19, 23 | uneingeschränkt, Ausgang oder Eingang |
| 16, 17 | uneingeschränkt (UART2, aber hier unbenutzt) |
| 4 | frei, aber ADC2 — nicht analog nutzen |
| 36, 39 | nur Eingang, ADC1 — gut für weitere Analogsensoren |
| 5, 2, 15 | nutzbar, aber Strapping-/Boot-Glitch-Pins — nur für unkritische Ausgänge |
| 1, 3 | UART0, belegt durch USB-Konsole |
| 12, 13 | **meiden** (siehe oben) |

---

## Signalwege und Vorzeichen

```
Linie liegt rechts  ->  Sensor rechts sieht mehr Linie
                    ->  LineSensor::error() > 0
                    ->  drive(base, steer > 0)
                    ->  Motor LINKS stärker, Motor RECHTS schwächer
                    ->  Roboter dreht nach rechts
                    ->  gelbe LED VORNE RECHTS blinkt
```

Dreht der fertige Roboter in die falsche Richtung, sind die Sensoren oder die
Motoren vertauscht. Statt umzulöten lässt sich das in `Config.h` korrigieren,
indem `PIN_LINE_L` und `PIN_LINE_R` getauscht werden.
