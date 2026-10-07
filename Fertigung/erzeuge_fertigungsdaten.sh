#!/usr/bin/env bash
# Erzeugt saemtliche Fertigungsdaten aus pcb/MarsRover.kicad_pcb.
#
# Aufruf aus dem Projektverzeichnis:   bash Fertigung/erzeuge_fertigungsdaten.sh
#
# Alles, was hier herauskommt, ist ERZEUGT. Die Quelle ist die Platinendatei;
# nach jeder Aenderung daran dieses Skript erneut laufen lassen, sonst liegen
# im Ordner Gerber, die nicht mehr zur Platine passen.
#
# Nullpunkt: absolut, also die Blattkoordinaten von KiCad. Gerber, Bohrdaten
# und Bestueckungsliste benutzen alle denselben -- das ist die Bedingung,
# damit der Fertiger die drei uebereinanderlegen kann.
set -e

CLI="/c/Program Files/KiCad/10.0/bin/kicad-cli.exe"
PYBIN="/c/Program Files/KiCad/10.0/bin/python.exe"   # KiCad bringt sein eigenes mit
PCB="pcb/MarsRover.kicad_pcb"
SCH="pcb/MarsRover.kicad_sch"
AUS="Fertigung"

[ -f "$PCB" ] || { echo "Aus dem Projektverzeichnis aufrufen."; exit 1; }
ls pcb/~*.lck >/dev/null 2>&1 && { echo "KiCad hat die Datei offen. Erst schliessen."; exit 1; }

rm -rf "$AUS/gerber" "$AUS/pastenschablone"
mkdir -p "$AUS/gerber" "$AUS/pastenschablone"

echo "== Gerber =="
"$CLI" pcb export gerbers -o "$AUS/gerber/" \
  --layers "F.Cu,B.Cu,F.Paste,B.Paste,F.Silkscreen,B.Silkscreen,F.Mask,B.Mask,Edge.Cuts" \
  --subtract-soldermask --no-protel-ext "$PCB" >/dev/null

echo "== Bohrdaten =="
"$CLI" pcb export drill -o "$AUS/gerber/" --format excellon \
  --drill-origin absolute --excellon-units mm \
  --generate-map --map-format gerberx2 --excellon-separate-th "$PCB" >/dev/null

echo "== Bestueckungsdaten =="
"$CLI" pcb export pos -o "$AUS/MarsRover-bestueckung.csv" \
  --format csv --units mm --side both "$PCB" >/dev/null

echo "== Stueckliste =="
"$CLI" sch export bom -o "$AUS/MarsRover-stueckliste.csv" \
  --fields "Reference,Value,Footprint,\${QUANTITY},Hinweis" \
  --labels "Referenzen,Wert,Fussabdruck,Menge,Hinweis" \
  --group-by "Value,Footprint" --sort-field "Reference" "$SCH" >/dev/null

echo "== Pastenschablone (nur Oberseite, unten sitzt kein SMD) =="
cp "$AUS/gerber/MarsRover-F_Paste.gbr" "$AUS/gerber/MarsRover-Edge_Cuts.gbr" \
   "$AUS/pastenschablone/"
"$CLI" pcb export svg -o "$AUS/pastenschablone/MarsRover-F_Paste.svg" \
  --layers "F.Paste,Edge.Cuts" --page-size-mode 2 "$PCB" >/dev/null
TMP="$AUS/pastenschablone/_dxf"
"$CLI" pcb export dxf -o "$TMP" --layers "F.Paste,Edge.Cuts" \
  --output-units mm --exclude-refdes --exclude-value "$PCB" >/dev/null
mv "$TMP"/*.dxf "$AUS/pastenschablone/" && rmdir "$TMP"

echo "== 3D =="
# Nur die Platine: eigene Geometrie, darf ins Repo. Fuer das Basismodul ist
# ohnehin die Kontur mit den Bohrungen gefragt, nicht die Bauteilmodelle.
"$CLI" pcb export step -o "$AUS/MarsRover-platine.step" \
  --no-components --subst-models --no-unspecified "$PCB" >/dev/null
# Mit Bauteilen: enthaelt Fremdmodelle, bleibt deshalb lokal (gitignoriert).
mkdir -p "local_Step Files"
"$CLI" pcb export step -o "local_Step Files/MarsRover-komplett.step" \
  --subst-models --no-unspecified --include-tracks --include-pads "$PCB" >/dev/null

echo "== Ansichten =="
mkdir -p pcb/_render
"$CLI" pcb export pdf -o pcb/_render/pcb_geroutet.pdf \
  --layers "F.Cu,B.Cu,Edge.Cuts,F.Silkscreen" --mode-multipage \
  --include-border-title "$PCB" >/dev/null
"$CLI" pcb render -o pcb/_render/3d_oben.png --side top --quality high \
  --background opaque --width 1600 --height 1200 "$PCB" >/dev/null
"$CLI" pcb render -o pcb/_render/3d_unten.png --side bottom --quality high \
  --background opaque --width 1600 --height 1200 "$PCB" >/dev/null
"$CLI" pcb render -o pcb/_render/3d_schraeg.png --side top --rotate "-30,0,25" \
  --perspective --zoom 0.9 --quality high --floor --background opaque \
  --width 1600 --height 1200 "$PCB" >/dev/null

echo "== Archiv fuer den Fertiger =="
# Git Bash bringt kein zip mit; Pythons zipfile tut es genauso und
# ist auf jedem Rechner da, auf dem KiCad laeuft.
"$PYBIN" - "$AUS" <<'ZIPEOF'
import os, sys, zipfile
aus = sys.argv[1]
for ordner, name in (("gerber", "MarsRover-gerber.zip"),
                     ("pastenschablone", "MarsRover-pastenschablone.zip")):
    ziel = os.path.join(aus, name)
    if os.path.exists(ziel):
        os.remove(ziel)
    q = os.path.join(aus, ordner)
    with zipfile.ZipFile(ziel, "w", zipfile.ZIP_DEFLATED) as z:
        for f in sorted(os.listdir(q)):
            z.write(os.path.join(q, f), f)
    print("  %s (%d Dateien)" % (name, len(os.listdir(q))))
ZIPEOF

echo
echo "fertig. Inhalt von $AUS:"
ls -1 "$AUS"
