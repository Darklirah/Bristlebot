#!/usr/bin/env bash
# Baut die Firmware und legt das fertige Flash-Abbild nach docs/firmware/,
# wo der Web-Installer es erwartet.
#
# Aufruf aus dem Projektverzeichnis:
#   bash Firmware/baue_und_veroeffentliche.sh [version]
#
# Warum ein Skript: zwischen "kompiliert" und "mit dem Installer
# installierbar" liegen zwei Schritte, die man leicht vergisst -- das
# Zusammenfuegen der vier Teilabbilder und das Nachziehen der Version im
# manifest.json. Fehlt das Zusammenfuegen, laedt der Installer nur die
# Anwendung ohne Bootloader, und das Geraet bootet nicht mehr.
set -e

PIO="/c/Users/Frank-PC-AMD/.platformio/penv/Scripts/pio.exe"
PIOPY="/c/Users/Frank-PC-AMD/.platformio/penv/Scripts/python.exe"
ESPTOOL="/c/Users/Frank-PC-AMD/.platformio/packages/tool-esptoolpy/esptool.py"
BOOTAPP0="/c/Users/Frank-PC-AMD/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin"
UMGEBUNG="esp32dev_core2"
BAU="Firmware/.pio/build/$UMGEBUNG"
ZIEL="docs/firmware/marsrover-esp32.bin"

[ -f Firmware/platformio.ini ] || { echo "Aus dem Projektverzeichnis aufrufen."; exit 1; }

# --- Umgebung dieses Rechners -----------------------------------------
# Ein TLS-aufbrechender Scanner sitzt zwischen PlatformIO und GitHub. Ohne
# ein Wurzelzertifikatsbuendel, das dessen Wurzel enthaelt, bricht jeder
# Paketdownload mit CERTIFICATE_VERIFY_FAILED ab. Das Buendel wird beim
# ersten Lauf aus dem Windows-Zertifikatsspeicher erzeugt.
export UV_SYSTEM_CERTS=1 UV_NATIVE_TLS=1
# Vorwaertsschraegstriche: mit 'C:\pioc' legt Git Bash den Cache als
# relativen Ordner neben das Projekt. Windows nimmt beides.
export PLATFORMIO_CACHE_DIR='C:/pioc'
BUENDEL="$HOME/.platformio/win-root-ca.pem"
if [ ! -f "$BUENDEL" ]; then
  echo "== Wurzelzertifikate aus dem Windows-Speicher sammeln =="
  "$PIOPY" - "$BUENDEL" <<'PYEOF'
import ssl, sys, certifi
teile = [open(certifi.where(), "rb").read()]
for laden in ("ROOT", "CA"):
    for der, _t, _u in ssl.enum_certificates(laden):
        try:
            teile.append(ssl.DER_cert_to_PEM_cert(der).encode())
        except Exception:
            pass
open(sys.argv[1], "wb").write(b"\n".join(teile))
PYEOF
fi
export REQUESTS_CA_BUNDLE="$BUENDEL" CURL_CA_BUNDLE="$BUENDEL" SSL_CERT_FILE="$BUENDEL"

echo "== Bauen =="
( cd Firmware && "$PIO" run -e "$UMGEBUNG" )

echo "== Teilabbilder zusammenfuegen =="
# Der Installer bekommt im manifest.json genau einen Teil bei Offset 0.
# Also muss hier alles hinein, was ein jungfraeulicher ESP32 braucht:
#   0x01000 Bootloader
#   0x08000 Partitionstabelle
#   0x0e000 boot_app0 (zeigt auf die erste Anwendungspartition)
#   0x10000 die Anwendung selbst
"$PIOPY" "$ESPTOOL" --chip esp32 merge_bin -o "$BAU/marsrover-esp32.bin" \
  --flash_mode dio --flash_freq 40m --flash_size 4MB \
  0x1000  "$BAU/bootloader.bin" \
  0x8000  "$BAU/partitions.bin" \
  0xe000  "$BOOTAPP0" \
  0x10000 "$BAU/firmware.bin"

mkdir -p docs/firmware
cp "$BAU/marsrover-esp32.bin" "$ZIEL"

# --- Version im Manifest nachziehen -----------------------------------
if [ -n "$1" ]; then
  "$PIOPY" - "$1" <<'PYEOF'
import io, json, sys
p = "docs/manifest.json"
d = json.load(io.open(p, encoding="utf-8"))
d["version"] = sys.argv[1]
io.open(p, "w", encoding="utf-8", newline="\n").write(
    json.dumps(d, indent=2, ensure_ascii=False) + "\n")
print("manifest.json: Version auf %s gesetzt" % sys.argv[1])
PYEOF
fi

echo
echo "fertig:"
ls -l "$ZIEL"
echo
echo "Noch zu tun: committen und pushen -- erst dann sieht der Installer"
echo "auf darklirah.github.io die neue Fassung."
