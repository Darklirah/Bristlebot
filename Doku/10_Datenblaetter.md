# DatenblÃ¤tter

Linkliste zu allen verwendeten Bauteilen. **Die PDFs selbst liegen nicht im
Repository** â sie gehÃ¶ren ihren Herstellern, und Weiterverbreitung erlauben
lÃ¤ngst nicht alle. Heruntergeladen gehÃ¶ren sie lokal nach `Datasheets/`,
dieser Ordner ist in `.gitignore` eingetragen.

Stand: 05.10.2026. Legende: â = liegt bereits lokal vor.

---

## Rechenkern

| Bauteil | Hersteller | Link | |
|---|---|---|---|
| ESP32 DevKit C V4 | AZ-Delivery | [Produktseite â Downloads](https://www.az-delivery.de/products/esp-32-dev-kit-c-v4) | â |
| ESP32-WROOM-32 (Modul) | Espressif | [esp32-wroom-32_datasheet_en.pdf](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf) | â |
| ESP32 Technical Reference Manual | Espressif | [esp32_technical_reference_manual_en.pdf](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf) | â |
| ESP32 (SoC) Datasheet | Espressif | [esp32_datasheet_en.pdf](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf) | |

> Das Technical Reference Manual ist fÃ¼r den Alltag zu dick, aber es ist die
> einzige Quelle, die die **Strapping-Pins** vollstÃ¤ndig erklÃ¤rt â also
> warum GPIO 12 nicht als Ausgang taugt und GPIO 6â11 gar nicht benutzbar
> sind.

## Sensorik

| Bauteil | Hersteller | Link | |
|---|---|---|---|
| TCRT5000 Reflexkoppler | Vishay | [tcrt5000.pdf](https://www.vishay.com/docs/83760/tcrt5000.pdf) | â |
| MPU-6050 Lagesensor | TDK InvenSense | [MPU-6000-Datasheet1.pdf](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf) | â |
| MPU-6050 Register Map | TDK InvenSense | [MPU-6000-Register-Map1.pdf](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf) | |
| VL53L0X Abstandssensor | STMicroelectronics | [vl53l0x.pdf](https://www.st.com/resource/en/datasheet/vl53l0x.pdf) | |

> Die **Register Map** des MPU-6050 wird gebraucht, nicht nur das
> Datenblatt: Die Einstellungen, die `Imu.cpp` vornimmt â DLPF auf 21 Hz,
> Messbereiche Â±500 Â°/s und Â±8 g â stehen nur dort.

## Halbleiter

| Bauteil | Hersteller | Link | |
|---|---|---|---|
| AO3400A Logik-MOSFET | Alpha & Omega | [AO3400A.pdf](http://www.aosmd.com/pdfs/datasheet/AO3400A.pdf) | |
| 1N5819 Schottky | Vishay | [1N5817â¦1N5819](https://www.vishay.com/docs/88525/1n5817.pdf) | |
| 1N4148 (Alternative) | Vishay | [1n4148.pdf](https://www.vishay.com/docs/81857/1n4148.pdf) | â |
| LED 3 mm gelb / rot | â | herstellerabhÃ¤ngig, beim Kauf mitnehmen | |

> Beim **AO3400A** ist entscheidend, dass es ein **Logic-Level**-Typ ist:
> R_DS(on) ist bei 4,5 V Gate-Spannung spezifiziert, nicht bei 10 V. Die
> naheliegenden Bastelkisten-Typen 2N7000, BS170 und IRF540 sind bei 3,3 V
> praktisch zu und werden heiß — ein klassischer Anfängerfehler.

## Stromversorgung

| Bauteil | Hersteller | Link | |
|---|---|---|---|
| TP4056 Lade-IC | Nanjing Top Power | [TP4056 (SparkFun-Spiegel)](https://cdn.sparkfun.com/datasheets/Prototyping/TP4056.pdf) | |
| DW01A Schutz-IC | Fortune Semiconductor | [DW01A (LCSC)](https://datasheet.lcsc.com/lcsc/1811151452_PUOLOP-DW01A_C351410.pdf) | |
| FS8205A Doppel-MOSFET | Fortune Semiconductor | [FS8205A (LCSC)](https://datasheet.lcsc.com/lcsc/1811081328_Fortune-Semicon-FS8205A_C32254.pdf) | |
| MT3608 Step-Up | Aerosemi | [MT3608 (Olimex-Spiegel)](https://www.olimex.com/Products/Breadboarding/BB-PWR-3608/resources/MT3608.pdf) | |

> FÃ¼r **TP4056, DW01A, FS8205A und MT3608** gibt es keine offiziellen
> Herstellerseiten im westlichen Netz â die verlinkten Spiegel bei SparkFun,
> LCSC und Olimex sind die Ã¼blichen Quellen.
>
> Das **TP4056-Datenblatt** wird konkret gebraucht: Dort steht die Formel
> fÃ¼r den Ladestrom, `I = 1200 / R_prog`. Daraus kommen die 4,7 kÎ© fÃ¼r
> 255 mA, mit denen die 500-mAh-Zelle mit 0,5 C geladen wird statt mit den
> 1 A, die die Fertigmodule ab Werk einstellen.

## Mechanik und Verbinder

| Bauteil | Hersteller | Link | |
|---|---|---|---|
| JST PH 2,0 mm | JST | [eph.pdf](https://www.jst-mfg.com/product/pdf/eng/ePH.pdf) | |
| USB-C-Buchse | herstellerabhÃ¤ngig | beim Kauf mitnehmen | |
| Vibrationsmotor | herstellerabhÃ¤ngig | beim Kauf mitnehmen | |
| Schiebeschalter SPDT | herstellerabhÃ¤ngig | beim Kauf mitnehmen | |

> Bei **Vibrationsmotor und LEDs** lohnt sich das Mitnehmen des Datenblatts
> wirklich: Beim Motor bestimmen Nennspannung und Stromaufnahme den Wert
> `MOTOR_RATED_V` in `Config.h` und damit die Duty-Begrenzung. Bei den LEDs
> bestimmt die Flussspannung den Vorwiderstand.

---

## Einer, der sich nicht automatisch laden lässt

**STMicroelectronics** (VL53L0X) weist skriptgesteuerte
Zugriffe ab â 403 beziehungsweise ZeitÃ¼berschreitung. Beide Links funktionieren
im Browser einwandfrei, man muss sie nur von Hand anklicken.

---

## Warum die PDFs nicht ins Repository gehÃ¶ren

Herunterladen und benutzen darf man alle diese DatenblÃ¤tter. **Weitergeben**
erlauben ausdrÃ¼cklich nur manche Hersteller â Espressif und ST etwa sind
groÃzÃ¼gig, andere schweigen dazu oder untersagen es. In einem Ã¶ffentlichen
Repository wÃ¤re das Weiterverbreitung.

Dazu kÃ¤men 20â40 MB in ein Projekt, das sonst aus Text besteht.

Eine Linkliste erreicht dasselbe: Jeder kommt an jedes Dokument, und zwar an
die **aktuelle** Fassung beim Hersteller statt an eine eingefrorene Kopie.
