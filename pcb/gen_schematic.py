#!/usr/bin/env python3
"""
Erzeugt den Schaltplan des One-of-a-Kind Mars Rover aus der Netzliste.

GEDACHT ALS STARTPUNKT, NICHT ALS DAUERWERKZEUG.
Einmal laufen lassen, danach den Schaltplan in KiCad von Hand pflegen --
sonst ueberschreibt ein zweiter Lauf alle manuellen Aenderungen.

Verdrahtet wird ueber Netznamen (Labels), nicht ueber gezeichnete Leitungen.
Das ist in KiCad voellig regulaer, bei dieser Schaltungsgroesse deutlich
uebersichtlicher und vor allem zuverlaessig erzeugbar -- Leitungsgeometrie
von Hand zu rechnen waere die Hauptfehlerquelle.

Aufruf:  "C:\\Program Files\\KiCad\\10.0\\bin\\python.exe" gen_schematic.py
"""

import re
import sys
import uuid
from pathlib import Path

HERE = Path(__file__).parent
KICAD_SYMBOLS = Path(r"C:\Program Files\KiCad\10.0\share\kicad\symbols")
LOCAL_LIBS = {
    "Espressif": HERE / "lib" / "Espressif.kicad_sym",
    "MarsRover": HERE / "MarsRover.kicad_sym",
}

PROJECT = "MarsRover"
PAPER = "A3"                 # 420 x 297 mm
SHEET_UUID = str(uuid.uuid4())


# =====================================================================
#  Symbole aus den Bibliotheken holen
# =====================================================================
def lib_path(libname: str) -> Path:
    if libname in LOCAL_LIBS:
        return LOCAL_LIBS[libname]
    return KICAD_SYMBOLS / f"{libname}.kicad_sym"


def extract_symbol(libname: str, symname: str) -> str:
    """Den kompletten (symbol "...") Block aus einer .kicad_sym holen.

    Klammern zaehlen statt Regex ueber mehrere Zeilen: die Bloecke sind
    verschachtelt, und eine Regex bekommt das nicht zuverlaessig hin.
    """
    path = lib_path(libname)
    if not path.exists():
        raise FileNotFoundError(f"Bibliothek fehlt: {path}")
    text = path.read_text(encoding="utf-8")

    needle = f'(symbol "{symname}"'
    start = text.find(needle)
    if start < 0:
        raise KeyError(f'Symbol "{symname}" nicht in {libname}')

    depth = 0
    i = start
    in_str = False
    while i < len(text):
        c = text[i]
        if in_str:
            if c == "\\":
                i += 2
                continue
            if c == '"':
                in_str = False
        elif c == '"':
            in_str = True
        elif c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                block = text[start : i + 1]
                # lib_id in der Instanz ist "Lib:Name" -- im lib_symbols-Abschnitt
                # muss der Symbolname genau so lauten.
                return block.replace(needle, f'(symbol "{libname}:{symname}"', 1)
        i += 1
    raise ValueError(f"Unausgeglichene Klammern bei {symname}")


def symbol_pins(block: str):
    """Pinnummern eines Symbolblocks, in Reihenfolge des Auftretens."""
    return re.findall(r'\(number "([^"]+)"', block)


# =====================================================================
#  Bauteilliste  (Stufe 1: Antriebsteil, zum Pruefen des Formats)
# =====================================================================
# ref, Bibliothek, Symbol, Wert, Position (mm), {Pin: Netz}
COMPONENTS = [
    # --- Rechenkern -------------------------------------------------
    ("U1", "Espressif", "ESP32-DevKitC", "ESP32-DevKitC V4", (80, 100), {
        "1":  "+3V3",
        "7":  "MOT_L_G",     # GPIO32
        "8":  "MOT_R_G",     # GPIO33
        "14": "GND",
        "19": "+5V",         # 5V-Pin des DevKits
        "32": "GND",
        "38": "GND",
    }),

    # --- Motorstufe links -------------------------------------------
    ("R1", "Device", "R", "100R",   (150, 90),  {"1": "MOT_L_G", "2": "MOT_L_G2"}),
    ("R3", "Device", "R", "100k", (165, 100), {"1": "MOT_L_G2", "2": "GND"}),
    ("Q1", "Device", "Q_NMOS_GSD", "AO3400A", (180, 90), {
        "1": "MOT_L_G2", "2": "GND", "3": "MOT_L_D"}),
    ("M1", "Device", "Motor_DC", "Vibrationsmotor L", (180, 65), {
        "1": "+5V", "2": "MOT_L_D"}),
    ("D1", "Diode", "1N5819", "1N5819", (200, 65), {
        "1": "MOT_L_D", "2": "+5V"}),
    ("C3", "Device", "C", "100n", (215, 90), {"1": "MOT_L_D", "2": "GND"}),

    # --- Motorstufe rechts ------------------------------------------
    ("R2", "Device", "R", "100R",   (150, 140), {"1": "MOT_R_G", "2": "MOT_R_G2"}),
    ("R4", "Device", "R", "100k", (165, 150), {"1": "MOT_R_G2", "2": "GND"}),
    ("Q2", "Device", "Q_NMOS_GSD", "AO3400A", (180, 140), {
        "1": "MOT_R_G2", "2": "GND", "3": "MOT_R_D"}),
    ("M2", "Device", "Motor_DC", "Vibrationsmotor R", (180, 115), {
        "1": "+5V", "2": "MOT_R_D"}),
    ("D2", "Diode", "1N5819", "1N5819", (200, 115), {
        "1": "MOT_R_D", "2": "+5V"}),
    ("C4", "Device", "C", "100n", (215, 140), {"1": "MOT_R_D", "2": "GND"}),
]


# =====================================================================
#  Schaltplan zusammensetzen
# =====================================================================
def esc(s: str) -> str:
    return s.replace("\\", "\\\\").replace('"', '\\"')


def prop(name, value, x, y, hide=False):
    h = "\n\t\t\t\t(hide yes)" if hide else ""
    return (
        f'\t\t\t(property "{esc(name)}" "{esc(value)}"\n'
        f"\t\t\t\t(at {x} {y} 0)\n"
        f"\t\t\t\t(effects\n\t\t\t\t\t(font\n\t\t\t\t\t\t(size 1.27 1.27)\n\t\t\t\t\t)"
        f"{h}\n\t\t\t\t)\n"
        f"\t\t\t)\n"
    )


def build():
    used = {}          # (lib, sym) -> block
    body = []
    labels = []

    for ref, lib, sym, value, (x, y), nets in COMPONENTS:
        key = (lib, sym)
        if key not in used:
            used[key] = extract_symbol(lib, sym)

        inst_uuid = str(uuid.uuid4())
        pins = symbol_pins(used[key])

        s = [f'\t\t(symbol\n\t\t\t(lib_id "{lib}:{sym}")\n']
        s.append(f"\t\t\t(at {x} {y} 0)\n\t\t\t(unit 1)\n")
        s.append("\t\t\t(exclude_from_sim no)\n\t\t\t(in_bom yes)\n")
        s.append("\t\t\t(on_board yes)\n\t\t\t(dnp no)\n")
        s.append(f'\t\t\t(uuid "{inst_uuid}")\n')
        s.append(prop("Reference", ref, x, y - 7.62))
        s.append(prop("Value", value, x, y + 7.62))
        s.append(prop("Footprint", "", x, y, hide=True))
        s.append(prop("Datasheet", "", x, y, hide=True))
        for p in pins:
            s.append(f'\t\t\t(pin "{esc(p)}"\n\t\t\t\t(uuid "{uuid.uuid4()}")\n\t\t\t)\n')
        s.append(
            "\t\t\t(instances\n"
            f'\t\t\t\t(project "{PROJECT}"\n'
            f'\t\t\t\t\t(path "/{SHEET_UUID}"\n'
            f'\t\t\t\t\t\t(reference "{ref}")\n\t\t\t\t\t\t(unit 1)\n'
            "\t\t\t\t\t)\n\t\t\t\t)\n\t\t\t)\n"
        )
        s.append("\t\t)\n")
        body.append("".join(s))

        # Netznamen als Textnotiz neben das Bauteil -- die eigentliche
        # Verdrahtung kommt in Stufe 2 als echte Labels an Pinkoordinaten.
        for pin, net in sorted(nets.items()):
            labels.append((ref, pin, net))

    out = ['(kicad_sch\n\t(version 20250114)\n\t(generator "marsrover")\n']
    out.append('\t(generator_version "10.0")\n')
    out.append(f'\t(uuid "{SHEET_UUID}")\n')
    out.append(f'\t(paper "{PAPER}")\n')
    out.append(
        "\t(title_block\n"
        '\t\t(title "One-of-a-Kind Mars Rover")\n'
        '\t\t(company "Darklirah")\n'
        '\t\t(comment 1 "Erzeugt aus Hardware/Netzliste.md -- danach von Hand gepflegt")\n'
        "\t)\n"
    )
    out.append("\t(lib_symbols\n")
    for block in used.values():
        out.append("\t\t" + block.replace("\n", "\n\t\t") + "\n")
    out.append("\t)\n")
    out.extend(body)
    out.append(
        "\t(sheet_instances\n\t\t(path \"/\"\n\t\t\t(page \"1\")\n\t\t)\n\t)\n"
    )
    out.append(")\n")

    return "".join(out), labels


if __name__ == "__main__":
    text, labels = build()
    target = HERE / f"{PROJECT}.kicad_sch"
    target.write_text(text, encoding="utf-8")
    print(f"Geschrieben: {target}  ({len(text)} Zeichen)")
    print(f"Bauteile:    {len(COMPONENTS)}")
    print(f"Pin-Netz-Zuordnungen vorgemerkt: {len(labels)}")
