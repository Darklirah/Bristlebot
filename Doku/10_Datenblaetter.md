# Datenblätter

Linkliste zu allen verwendeten Bauteilen. **Die PDFs selbst liegen nicht im
Repository** — sie gehören ihren Herstellern, und Weiterverbreitung erlauben
längst nicht alle. Heruntergeladen gehören sie lokal nach `Datenblaetter/`,
dieser Ordner ist in `.gitignore` eingetragen.

Stand: 05.10.2026. Legende: ✓ = liegt bereits lokal vor.

---

## Rechenkern

| Bauteil | Hersteller | Link | |
|---|---|---|---|
| ESP32 DevKit C V4 | AZ-Delivery | [Produktseite → Downloads](https://www.az-delivery.de/products/esp-32-dev-kit-c-v4) | ✓ |
| ESP32-WROOM-32 (Modul) | Espressif | [esp32-wroom-32_datasheet_en.pdf](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf) | ✓ |
| ESP32 Technical Reference Manual | Espressif | [esp32_technical_reference_manual_en.pdf](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf) | ✓ |
| ESP32 (SoC) Datasheet | Espressif | [esp32_datasheet_en.pdf](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf) | |

> Das Technical Reference Manual ist für den Alltag zu dick, aber es ist die
> einzige Quelle, die die **Strapping-Pins** vollständig erklärt — also
> warum GPIO 12 nicht als Ausgang taugt und GPIO 6–11 gar nicht benutzbar
> sind.

## Sensorik

| Bauteil | Hersteller | Link | |
|---|---|---|---|
| TCRT5000 Reflexkoppler | Vishay | [tcrt5000.pdf](https://www.vishay.com/docs/83760/tcrt5000.pdf) | ✓ |
| MPU-6050 Lagesensor | TDK InvenSense | [MPU-6000-Datasheet1.pdf](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf) | ✓ |
| MPU-6050 Register Map | TDK InvenSense | [MPU-6000-Register-Map1.pdf](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf) | |
| VL53L0X Abstandssensor | STMicroelectronics | [vl53l0x.pdf](https://www.st.com/resource/en/datasheet/vl53l0x.pdf) | |

> Die **Register Map** des MPU-6050 wird gebraucht, nicht nur das
> Datenblatt: Die Einstellungen, die `Imu.cpp` vornimmt — DLPF auf 21 Hz,
> Messbereiche ±500 °/s und ±8 g — stehen nur dort.

## Halbleiter

| Bauteil | Hersteller | Link | |
|---|---|---|---|
| BC337-40 NPN | onsemi | [bc337-d.pdf](https://www.onsemi.com/download/data-sheet/pdf/bc337-d.pdf) | |
| 1N5819 Schottky | Vishay | [1N5817…1N5819](https://www.vishay.com/docs/88525/1n5817.pdf) | |
| 1N4148 (Alternative) | Vishay | [1n4148.pdf](https://www.vishay.com/docs/81857/1n4148.pdf) | ✓ |
| LED 3 mm gelb / rot | — | herstellerabhängig, beim Kauf mitnehmen | |

> Beim **BC337** ist die Stromverstärkungsgruppe entscheidend: Wir brauchen
> die **-40er**-Variante (h_FE ≥ 250), damit der Transistor mit 2,55 mA
> Basisstrom bei 100 mA Kollektorstrom sicher in Sättigung geht.

## Stromversorgung

| Bauteil | Hersteller | Link | |
|---|---|---|---|
| TP4056 Lade-IC | Nanjing Top Power | [TP4056 (SparkFun-Spiegel)](https://cdn.sparkfun.com/datasheets/Prototyping/TP4056.pdf) | |
| DW01A Schutz-IC | Fortune Semiconductor | [DW01A (LCSC)](https://datasheet.lcsc.com/lcsc/1811151452_PUOLOP-DW01A_C351410.pdf) | |
| FS8205A Doppel-MOSFET | Fortune Semiconductor | [FS8205A (LCSC)](https://datasheet.lcsc.com/lcsc/1811081328_Fortune-Semicon-FS8205A_C32254.pdf) | |
| MT3608 Step-Up | Aerosemi | [MT3608 (Olimex-Spiegel)](https://www.olimex.com/Products/Breadboarding/BB-PWR-3608/resources/MT3608.pdf) | |

> Für **TP4056, DW01A, FS8205A und MT3608** gibt es keine offiziellen
> Herstellerseiten im westlichen Netz — die verlinkten Spiegel bei SparkFun,
> LCSC und Olimex sind die üblichen Quellen.
>
> Das **TP4056-Datenblatt** wird konkret gebraucht: Dort steht die Formel
> für den Ladestrom, `I = 1200 / R_prog`. Daraus kommen die 4,7 kΩ für
> 255 mA, mit denen die 500-mAh-Zelle mit 0,5 C geladen wird statt mit den
> 1 A, die die Fertigmodule ab Werk einstellen.

## Mechanik und Verbinder

| Bauteil | Hersteller | Link | |
|---|---|---|---|
| JST PH 2,0 mm | JST | [eph.pdf](https://www.jst-mfg.com/product/pdf/eng/ePH.pdf) | |
| USB-C-Buchse | herstellerabhängig | beim Kauf mitnehmen | |
| Vibrationsmotor | herstellerabhängig | beim Kauf mitnehmen | |
| Schiebeschalter SPDT | herstellerabhängig | beim Kauf mitnehmen | |

> Bei **Vibrationsmotor und LEDs** lohnt sich das Mitnehmen des Datenblatts
> wirklich: Beim Motor bestimmen Nennspannung und Stromaufnahme den Wert
> `MOTOR_RATED_V` in `Config.h` und damit die Duty-Begrenzung. Bei den LEDs
> bestimmt die Flussspannung den Vorwiderstand.

---

## Zwei, die sich nicht automatisch laden lassen

**onsemi** (BC337) und **STMicroelectronics** (VL53L0X) weisen skriptgesteuerte
Zugriffe ab — 403 beziehungsweise Zeitüberschreitung. Beide Links funktionieren
im Browser einwandfrei, man muss sie nur von Hand anklicken.

---

## Warum die PDFs nicht ins Repository gehören

Herunterladen und benutzen darf man alle diese Datenblätter. **Weitergeben**
erlauben ausdrücklich nur manche Hersteller — Espressif und ST etwa sind
großzügig, andere schweigen dazu oder untersagen es. In einem öffentlichen
Repository wäre das Weiterverbreitung.

Dazu kämen 20–40 MB in ein Projekt, das sonst aus Text besteht.

Eine Linkliste erreicht dasselbe: Jeder kommt an jedes Dokument, und zwar an
die **aktuelle** Fassung beim Hersteller statt an eine eingefrorene Kopie.
