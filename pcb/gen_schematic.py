#!/usr/bin/env python3
"""
Erzeugt den Schaltplan des One-of-a-Kind Mars Rover aus der Netzliste.

GEDACHT ALS STARTPUNKT, NICHT ALS DAUERWERKZEUG.
Einmal laufen lassen, danach den Schaltplan in KiCad von Hand pflegen --
sonst ueberschreibt ein zweiter Lauf alle manuellen Aenderungen.

Verdrahtet wird ueber Netzlabels an den Pins, nicht ueber gezeichnete
Leitungen. Das ist in KiCad voellig regulaer, bei dieser Schaltungsgroesse
uebersichtlicher, und vor allem zuverlaessig erzeugbar -- Leitungsgeometrie
von Hand zu rechnen waere die Hauptfehlerquelle.

Zwei Dinge, die beim ersten Versuch schiefgingen und hier geloest sind:

  * Alle Koordinaten muessen auf KiCads 1,27-mm-Raster liegen, sonst meldet
    ERC "endpoint_off_grid". Deshalb rechnet alles in Rastereinheiten zu
    2,54 mm statt in ganzen Millimetern.

  * In .kicad_sym zeigt die Y-Achse nach OBEN, in .kicad_sch nach UNTEN.
    Pinkoordinaten muessen also in Y gespiegelt werden, sonst landen die
    Labels spiegelverkehrt neben dem Bauteil statt auf dem Pin.

Aufruf:  "C:\\Program Files\\KiCad\\10.0\\bin\\python.exe" gen_schematic.py
"""

import re
import uuid
from pathlib import Path

HERE = Path(__file__).parent
KICAD_SYMBOLS = Path(r"C:\Program Files\KiCad\10.0\share\kicad\symbols")
LOCAL_LIBS = {
    "Espressif": HERE / "lib" / "Espressif.kicad_sym",
    "MarsRover": HERE / "MarsRover.kicad_sym",
}

PROJECT = "MarsRover"
PAPER = "A3"
SHEET_UUID = str(uuid.uuid4())
G = 2.54                      # Rastereinheit in mm, Vielfaches von 1,27


# =====================================================================
#  Symbole aus den Bibliotheken holen
# =====================================================================
def lib_path(libname):
    return LOCAL_LIBS.get(libname) or KICAD_SYMBOLS / f"{libname}.kicad_sym"


def extract_symbol(libname, symname):
    """Den kompletten (symbol "...") Block holen -- Klammern zaehlen,
    nicht Regex: die Bloecke sind verschachtelt.

    Rueckgabe: (Block mit "Lib:Name", Name des Elternsymbols oder None).

    Viele Bibliotheken nutzen Vererbung: "1N5819" ist nur ein Alias auf
    "SB120" und hat selbst gar keine Pins. In der Dioden-Bibliothek
    betrifft das 701 von rund 1000 Symbolen. Wer das nicht behandelt,
    findet bei solchen Bauteilen keine Pins.
    """
    path = lib_path(libname)
    if not path.exists():
        raise FileNotFoundError(f"Bibliothek fehlt: {path}")
    text = path.read_text(encoding="utf-8")

    needle = f'(symbol "{symname}"'
    start = text.find(needle)
    if start < 0:
        raise KeyError(f'Symbol "{symname}" nicht in {libname}')

    depth, i, in_str = 0, start, False
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
                block = text[start:i + 1]
                m = re.search(r'\(extends\s+"([^"]+)"', block)
                parent = m.group(1) if m else None
                block = block.replace(needle, f'(symbol "{libname}:{symname}"', 1)
                return block, parent
        i += 1
    raise ValueError(f"Unausgeglichene Klammern bei {symname}")


PIN_RE = re.compile(
    r"\(pin\s+\S+\s+\S+\s*\(at\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s*\)"
    r".*?\(number\s+\"([^\"]+)\"",
    re.S,
)


def symbol_pins(block):
    """{Pinnummer: (x, y)} im Koordinatensystem des Symbols."""
    out = {}
    for m in PIN_RE.finditer(block):
        x, y, _ang, num = m.groups()
        if num not in out:
            out[num] = (float(x), float(y))
    return out


# =====================================================================
#  Bauteilliste
#  ref, Bibliothek, Symbol, Wert, (Spalte, Zeile) im Raster, {Pin: Netz}
# =====================================================================
COMPONENTS = [
    ("U1", "Espressif", "ESP32-DevKitC", "ESP32-DevKitC V4", (30, 40), {
        "1":  "+3V3",
        "4":  "VBAT_SENSE",   # GPIO39, optional
        "5":  "LINE_L",       # GPIO34
        "6":  "LINE_R",       # GPIO35
        "7":  "MOT_L_G",      # GPIO32
        "8":  "MOT_R_G",      # GPIO33
        "9":  "LED_RR",       # GPIO25
        "10": "LED_RL",       # GPIO26
        "11": "LED_FR",       # GPIO27
        "12": "LED_FL",       # GPIO14
        "14": "GND",
        "19": "+5V",
        "32": "GND",
        "33": "I2C_SDA",      # GPIO21
        "36": "I2C_SCL",      # GPIO22
        "38": "GND",
    }),

    # --- Motorstufe links -------------------------------------------
    ("R1", "Device", "R", "100R", (58, 30), {"1": "MOT_L_G", "2": "MOT_L_GT"}),
    ("R3", "Device", "R", "100k", (64, 34), {"1": "MOT_L_GT", "2": "GND"}),
    ("Q1", "Transistor_FET", "Q_NMOS_GSD", "AO3400A", (70, 30), {
        "1": "MOT_L_GT", "2": "GND", "3": "MOT_L_D"}),
    ("M1", "Motor", "Motor_DC", "Vibrationsmotor L", (70, 22), {
        "1": "+5V", "2": "MOT_L_D"}),
    ("D1", "Diode", "1N5819", "1N5819", (78, 22), {"1": "MOT_L_D", "2": "+5V"}),
    ("C3", "Device", "C", "100n", (84, 30), {"1": "MOT_L_D", "2": "GND"}),

    # --- Motorstufe rechts ------------------------------------------
    ("R2", "Device", "R", "100R", (58, 48), {"1": "MOT_R_G", "2": "MOT_R_GT"}),
    ("R4", "Device", "R", "100k", (64, 52), {"1": "MOT_R_GT", "2": "GND"}),
    ("Q2", "Transistor_FET", "Q_NMOS_GSD", "AO3400A", (70, 48), {
        "1": "MOT_R_GT", "2": "GND", "3": "MOT_R_D"}),
    ("M2", "Motor", "Motor_DC", "Vibrationsmotor R", (70, 40), {
        "1": "+5V", "2": "MOT_R_D"}),
    ("D2", "Diode", "1N5819", "1N5819", (78, 40), {"1": "MOT_R_D", "2": "+5V"}),
    ("C4", "Device", "C", "100n", (84, 48), {"1": "MOT_R_D", "2": "GND"}),
]

# Power-Symbole benennen das Netz. Sie TREIBEN es aber nicht: ihr Pin ist
# vom Typ "power_in". Solange keine echte Quelle am Netz haengt -- und der
# Step-Up sitzt in Stufe 2 noch nicht im Schaltplan -- meldet ERC
# "power_pin_not_driven". Dagegen gibt es PWR_FLAG, dessen Pin "power_out"
# ist. Beide stehen auf demselben Punkt; zwei Pins an derselben Koordinate
# sind in KiCad verbunden.
POWER = [
    ("#PWR01", "+3V3", (20, 62)),
    ("#PWR02", "+5V",  (26, 62)),
    ("#PWR03", "GND",  (32, 62)),
]

# Pins von U1, die wir bewusst nicht benutzen. Sie bekommen ein
# "nicht angeschlossen"-Zeichen, damit ERC weiss, dass das Absicht ist.
# GPIO 6..11 sind die Flash-Pins -- die duerfen gar nicht benutzt werden.
U1_UNUSED = ["2", "3", "13", "15", "16", "17", "18", "20", "21", "22",
             "23", "24", "25", "26", "27", "28", "29", "30", "31",
             "34", "35", "37"]


# =====================================================================
#  Ausgabe
# =====================================================================
def esc(s):
    return s.replace("\\", "\\\\").replace('"', '\\"')


def prop(name, value, x, y, hide=False):
    h = "\n\t\t\t\t(hide yes)" if hide else ""
    return (
        f'\t\t\t(property "{esc(name)}" "{esc(value)}"\n'
        f"\t\t\t\t(at {x:.2f} {y:.2f} 0)\n"
        f"\t\t\t\t(effects\n\t\t\t\t\t(font\n\t\t\t\t\t\t(size 1.27 1.27)\n"
        f"\t\t\t\t\t){h}\n\t\t\t\t)\n\t\t\t)\n"
    )


def emit_symbol(ref, lib, sym, value, sx, sy, pin_pos, hide_ref=False, hide_val=False):
    s = [f'\t\t(symbol\n\t\t\t(lib_id "{lib}:{sym}")\n']
    s.append(f"\t\t\t(at {sx:.2f} {sy:.2f} 0)\n\t\t\t(unit 1)\n")
    s.append("\t\t\t(exclude_from_sim no)\n\t\t\t(in_bom yes)\n")
    s.append("\t\t\t(on_board yes)\n\t\t\t(dnp no)\n")
    s.append(f'\t\t\t(uuid "{uuid.uuid4()}")\n')
    s.append(prop("Reference", ref, sx, sy - 10.16, hide=hide_ref))
    s.append(prop("Value", value, sx, sy + 10.16, hide=hide_val))
    s.append(prop("Footprint", "", sx, sy, hide=True))
    s.append(prop("Datasheet", "", sx, sy, hide=True))
    for num in pin_pos:
        s.append(f'\t\t\t(pin "{esc(num)}"\n\t\t\t\t(uuid "{uuid.uuid4()}")\n\t\t\t)\n')
    s.append(
        "\t\t\t(instances\n"
        f'\t\t\t\t(project "{PROJECT}"\n'
        f'\t\t\t\t\t(path "/{SHEET_UUID}"\n'
        f'\t\t\t\t\t\t(reference "{ref}")\n\t\t\t\t\t\t(unit 1)\n'
        "\t\t\t\t\t)\n\t\t\t\t)\n\t\t\t)\n\t\t)\n"
    )
    return "".join(s)


def emit_label(net, x, y):
    return (
        f'\t\t(label "{esc(net)}"\n'
        f"\t\t\t(at {x:.2f} {y:.2f} 0)\n"
        "\t\t\t(effects\n\t\t\t\t(font\n\t\t\t\t\t(size 1.27 1.27)\n\t\t\t\t)\n"
        "\t\t\t\t(justify left bottom)\n\t\t\t)\n"
        f'\t\t\t(uuid "{uuid.uuid4()}")\n\t\t)\n'
    )


def build():
    used, pins, body, labels, noconn = {}, {}, [], [], []

    def collect(lib, sym):
        """Symbol einsammeln und Vererbung AUFLOESEN.

        In einer .kicad_sym darf ein Symbol per (extends "X") von einem
        anderen erben und hat dann selbst weder Grafik noch Pins. Im
        Schaltplan erwartet KiCad dagegen vollstaendige Symbole. Wer den
        Erbverweis einfach mitkopiert, bekommt ein Bauteil ohne Pins --
        die Labels haengen dann in der Luft und ERC meldet zusaetzlich
        lib_symbol_mismatch.

        Also: Block des Elternsymbols nehmen und auf den Namen des Kindes
        umschreiben, samt der Untereinheiten (Name_0_1, Name_1_1).
        """
        key = (lib, sym)
        if key in used:
            return pins[key]

        block, parent = extract_symbol(lib, sym)
        chain = []
        while parent:                       # mehrstufige Vererbung folgen
            chain.append(parent)
            block, parent = extract_symbol(lib, chain[-1])

        if chain:
            root = chain[-1]
            block = block.replace(f'"{lib}:{root}"', f'"{lib}:{sym}"')
            block = re.sub(rf'"{re.escape(root)}_(\d+_\d+)"',
                           rf'"{sym}_\1"', block)

        used[key] = block
        pins[key] = symbol_pins(block)
        if not pins[key]:
            raise ValueError(f"{lib}:{sym} hat keine Pins -- Vererbung kaputt?")
        return pins[key]

    def place(ref, lib, sym, value, col, row, nets, hide_ref=False, hide_val=False):
        pin_pos = collect(lib, sym)
        sx, sy = col * G, row * G
        body.append(emit_symbol(ref, lib, sym, value, sx, sy, pin_pos,
                                hide_ref, hide_val))
        for num, net in nets.items():
            if num not in pin_pos:
                raise KeyError(f"{ref}: Pin {num} gibt es in {lib}:{sym} nicht "
                               f"(vorhanden: {sorted(pin_pos)[:12]} ...)")
            px, py = pin_pos[num]
            # Y spiegeln: Symbolkoordinaten zeigen nach oben, das Blatt nach unten
            labels.append(emit_label(net, sx + px, sy - py))

    for ref, lib, sym, value, (col, row), nets in COMPONENTS:
        place(ref, lib, sym, value, col, row, nets)
        # Bewusst unbenutzte Pins von U1 als "nicht angeschlossen" markieren
        if ref == "U1":
            pin_pos = collect(lib, sym)
            sx, sy = col * G, row * G
            for num in U1_UNUSED:
                if num not in pin_pos:
                    raise KeyError(f"U1: Pin {num} gibt es nicht")
                px, py = pin_pos[num]
                noconn.append(
                    f"\t\t(no_connect\n\t\t\t(at {sx + px:.2f} {sy - py:.2f})\n"
                    f'\t\t\t(uuid "{uuid.uuid4()}")\n\t\t)\n'
                )

    # Power-Symbol und PWR_FLAG auf denselben Punkt. Kein Label noetig --
    # ein Power-Symbol benennt sein Netz selbst, ein zusaetzliches Label
    # wuerde nur daneben haengen.
    for ref, net, (col, row) in POWER:
        place(ref, "power", net, net, col, row, {}, hide_ref=True, hide_val=True)
        place("#FLG" + ref[-2:], "power", "PWR_FLAG", "PWR_FLAG", col, row, {},
              hide_ref=True, hide_val=True)

    out = [
        "(kicad_sch\n\t(version 20250114)\n",
        '\t(generator "marsrover")\n\t(generator_version "10.0")\n',
        f'\t(uuid "{SHEET_UUID}")\n\t(paper "{PAPER}")\n',
        "\t(title_block\n"
        '\t\t(title "One-of-a-Kind Mars Rover")\n'
        '\t\t(company "Darklirah")\n'
        '\t\t(comment 1 "Erzeugt aus Hardware/Netzliste.md -- danach von Hand gepflegt")\n'
        "\t)\n",
        "\t(lib_symbols\n",
    ]
    for block in used.values():
        out.append("\t\t" + block.replace("\n", "\n\t\t") + "\n")
    out.append("\t)\n")
    out.extend(body)
    out.extend(labels)
    out.extend(noconn)
    out.append('\t(sheet_instances\n\t\t(path "/"\n\t\t\t(page "1")\n\t\t)\n\t)\n)\n')
    return "".join(out), len(labels), len(noconn)


if __name__ == "__main__":
    text, nlabels, nnc = build()
    target = HERE / f"{PROJECT}.kicad_sch"
    target.write_text(text, encoding="utf-8")
    print(f"Geschrieben: {target.name}  ({len(text):,} Zeichen)")
    print(f"Bauteile:    {len(COMPONENTS)} + {len(POWER)} Power-Symbole")
    print(f"Netzlabels:  {nlabels}")
    print(f"Nicht-angeschlossen-Zeichen: {nnc}")
