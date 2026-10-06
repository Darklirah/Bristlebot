#!/usr/bin/env python3
"""
Baut den Schaltplan des One-of-a-Kind Mars Rover auf.

GEDACHT ALS EINMALIGER AUFBAU, NICHT ALS DAUERWERKZEUG.
Danach wird der Schaltplan ueber den KiCAD-MCP-Server gepflegt --
gezielte Einzeleingriffe, damit Handaenderungen im Editor erhalten
bleiben. Ein zweiter Lauf dieses Skripts wuerde alles ueberschreiben.

Unterschied zum Vorgaenger gen_schematic.py: dort hingen die
Verbindungen an Netzlabels, die Bauteile standen ungeordnet auf dem
Blatt. Hier gilt:

  * Fuenf abgegrenzte Bereiche mit Rahmen und Ueberschrift
  * Verbindungen als GEZEICHNETE LEITUNGEN mit Knotenpunkten
  * Netzlabels kommen nicht mehr vor
  * Versorgung ueber Power-Symbole, wie in jedem Schaltplan ueblich --
    eine durchgezogene Leitung quer ueber das Blatt waere unleserlicher

Drei Dinge, die KiCad einem uebelnimmt und die hier geloest sind:

  * Alle Koordinaten muessen auf dem 1,27-mm-Raster liegen, sonst
    meldet ERC "endpoint_off_grid". Am Ende prueft das Skript das.
  * In .kicad_sym zeigt die Y-Achse nach OBEN, in .kicad_sch nach
    UNTEN. Pinkoordinaten muessen gespiegelt werden.
  * Viele Bibliothekssymbole erben per (extends ...) und haben selbst
    keine Pins. Die Vererbung wird hier aufgeloest.

Die Drehungen sind am lebenden KiCad ausgemessen, nicht geraten:

    rot   0 :  (sx + px, sy - py)
    rot  90 :  (sx - py, sy - px)
    rot 180 :  (sx - px, sy + py)
    rot 270 :  (sx + py, sy + px)
    mirror y:  x kippt      -> (sx - px, sy - py)
    mirror x:  y kippt      -> (sx + px, sy + py)

Aufruf:  "C:\Program Files\KiCad\10.0\bin\python.exe" build_schematic.py
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
PAPER = "A2"                  # 594 x 420 mm
SHEET_UUID = str(uuid.uuid4())
GRID = 1.27                   # KiCads Standardraster


# =====================================================================
#  Symbole aus den Bibliotheken holen
# =====================================================================
def lib_path(libname):
    return LOCAL_LIBS.get(libname) or KICAD_SYMBOLS / f"{libname}.kicad_sym"


def extract_symbol(libname, symname):
    """Den kompletten (symbol "...") Block holen -- Klammern zaehlen,
    nicht Regex: die Bloecke sind verschachtelt.
    Rueckgabe: (Block mit "Lib:Name", Name des Elternsymbols oder None)."""
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
    """{Pinnummer: (x, y)} im Koordinatensystem des Symbols (Y zeigt nach oben)."""
    out = {}
    for m in PIN_RE.finditer(block):
        x, y, _ang, num = m.groups()
        if num not in out:
            out[num] = (float(x), float(y))
    return out


# =====================================================================
#  Ausgabebausteine
# =====================================================================
def esc(s):
    return s.replace("\\", "\\\\").replace('"', '\\"')


def prop(name, value, x, y, hide=False, justify="left"):
    h = "\n\t\t\t\t(hide yes)" if hide else ""
    j = f"\n\t\t\t\t\t(justify {justify})" if justify else ""
    return (
        f'\t\t\t(property "{esc(name)}" "{esc(value)}"\n'
        f"\t\t\t\t(at {x:.2f} {y:.2f} 0)\n"
        f"\t\t\t\t(effects\n\t\t\t\t\t(font\n\t\t\t\t\t\t(size 1.27 1.27)\n"
        f"\t\t\t\t\t){j}{h}\n\t\t\t\t)\n\t\t\t)\n"
    )


def emit_wire(x1, y1, x2, y2):
    return (
        "\t(wire\n\t\t(pts\n"
        f"\t\t\t(xy {x1:.2f} {y1:.2f}) (xy {x2:.2f} {y2:.2f})\n"
        "\t\t)\n\t\t(stroke\n\t\t\t(width 0)\n\t\t\t(type default)\n\t\t)\n"
        f'\t\t(uuid "{uuid.uuid4()}")\n\t)\n'
    )


def emit_junction(x, y):
    return (
        f"\t(junction\n\t\t(at {x:.2f} {y:.2f})\n\t\t(diameter 0)\n"
        f'\t\t(color 0 0 0 0)\n\t\t(uuid "{uuid.uuid4()}")\n\t)\n'
    )


def emit_noconnect(x, y):
    return (f"\t(no_connect\n\t\t(at {x:.2f} {y:.2f})\n"
            f'\t\t(uuid "{uuid.uuid4()}")\n\t)\n')


def emit_rect(x1, y1, x2, y2):
    return (
        f"\t(rectangle\n\t\t(start {x1:.2f} {y1:.2f})\n\t\t(end {x2:.2f} {y2:.2f})\n"
        "\t\t(stroke\n\t\t\t(width 0.254)\n\t\t\t(type dash)\n\t\t)\n"
        "\t\t(fill\n\t\t\t(type none)\n\t\t)\n"
        f'\t\t(uuid "{uuid.uuid4()}")\n\t)\n'
    )


def emit_text(s, x, y, size=1.27, bold=False):
    b = "\n\t\t\t\t(bold yes)" if bold else ""
    return (
        f'\t(text "{esc(s)}"\n\t\t(exclude_from_sim no)\n'
        f"\t\t(at {x:.2f} {y:.2f} 0)\n"
        f"\t\t(effects\n\t\t\t(font\n\t\t\t\t(size {size} {size}){b}\n\t\t\t)\n"
        "\t\t\t(justify left bottom)\n\t\t)\n"
        f'\t\t(uuid "{uuid.uuid4()}")\n\t)\n'
    )


# =====================================================================
#  Das Blatt
# =====================================================================
class Sheet:
    def __init__(self):
        self.libs = {}          # (lib, sym) -> Symbolblock
        self.pins = {}          # (lib, sym) -> {Nr: (px, py)}
        self.body = []          # platzierte Symbole
        self.graph = []         # Leitungen, Knoten, Rahmen, Text
        self.pos = {}           # ref -> {Pinnr: (x, y)} absolut
        self.offgrid = []       # Rasterverstoesse, am Ende geprueft
        self.n_sym = self.n_wire = self.n_junc = self.n_nc = 0

    # ---- Bibliothek ------------------------------------------------
    def _collect(self, lib, sym):
        """Symbol einsammeln und Vererbung AUFLOESEN.

        Ein Symbol mit (extends "X") hat selbst weder Grafik noch Pins.
        Wer den Erbverweis einfach mitkopiert, bekommt ein Bauteil ohne
        Pins -- in der Diodenbibliothek betrifft das 701 von rund 1000
        Symbolen. Also: Block des Elternsymbols nehmen und auf den Namen
        des Kindes umschreiben, samt Untereinheiten (Name_0_1, Name_1_1).
        """
        key = (lib, sym)
        if key in self.libs:
            return self.pins[key]
        block, parent = extract_symbol(lib, sym)
        chain = []
        while parent:
            chain.append(parent)
            block, parent = extract_symbol(lib, chain[-1])
        if chain:
            root = chain[-1]
            block = block.replace('"%s:%s"' % (lib, root), '"%s:%s"' % (lib, sym))
            block = re.sub(r'"%s_(\d+_\d+)"' % re.escape(root), r'"%s_\1"' % sym, block)
        self.libs[key] = block
        self.pins[key] = symbol_pins(block)
        if not self.pins[key]:
            raise ValueError("%s:%s hat keine Pins -- Vererbung kaputt?" % (lib, sym))
        return self.pins[key]

    # ---- Geometrie -------------------------------------------------
    @staticmethod
    def _xform(px, py, sx, sy, rot, mirror):
        """Pinkoordinate des Symbols -> Blattkoordinate.
        Am lebenden KiCad ausgemessen, siehe Dateikopf."""
        if mirror == "y":
            return sx - px, sy - py
        if mirror == "x":
            return sx + px, sy + py
        if rot == 0:
            return sx + px, sy - py
        if rot == 90:
            return sx - py, sy - px
        if rot == 180:
            return sx - px, sy + py
        if rot == 270:
            return sx + py, sy + px
        raise ValueError("Drehung %s gibt es nicht" % rot)

    def _grid(self, *vals):
        for v in vals:
            if abs(round(v / GRID) * GRID - v) > 1e-6:
                self.offgrid.append(v)

    # ---- Platzieren ------------------------------------------------
    def place(self, ref, lib, sym, value, x, y, rot=0, mirror=None,
              fields="R", foff=5.08, hide_fields=False):
        """Bauteil setzen. fields: R/L = Beschriftung rechts/links
        gestapelt, V = oben/unten. foff = Abstand in mm."""
        pin_pos = self._collect(lib, sym)
        self._grid(x, y)

        abspins = {}
        for num, (px, py) in pin_pos.items():
            ax, ay = self._xform(px, py, x, y, rot, mirror)
            abspins[num] = (ax, ay)
            self._grid(ax, ay)
        self.pos[ref] = abspins

        if fields == "R":
            rx, ry, vx, vy, just = x + foff, y - 1.27, x + foff, y + 1.27, "left"
        elif fields == "L":
            rx, ry, vx, vy, just = x - foff, y - 1.27, x - foff, y + 1.27, "right"
        else:
            rx, ry, vx, vy, just = x, y - foff, x, y + foff, "left"

        m = "\t\t\t(mirror %s)\n" % mirror if mirror else ""
        s = ['\t\t(symbol\n\t\t\t(lib_id "%s:%s")\n' % (lib, sym),
             "\t\t\t(at %.2f %.2f %d)\n" % (x, y, rot), m, "\t\t\t(unit 1)\n",
             "\t\t\t(exclude_from_sim no)\n\t\t\t(in_bom yes)\n",
             "\t\t\t(on_board yes)\n\t\t\t(dnp no)\n",
             '\t\t\t(uuid "%s")\n' % uuid.uuid4(),
             prop("Reference", ref, rx, ry, hide=hide_fields, justify=just),
             prop("Value", value, vx, vy, hide=hide_fields, justify=just),
             prop("Footprint", "", x, y, hide=True),
             prop("Datasheet", "", x, y, hide=True)]
        for num in pin_pos:
            s.append('\t\t\t(pin "%s"\n\t\t\t\t(uuid "%s")\n\t\t\t)\n'
                     % (esc(num), uuid.uuid4()))
        s.append("\t\t\t(instances\n"
                 '\t\t\t\t(project "%s"\n' % PROJECT +
                 '\t\t\t\t\t(path "/%s"\n' % SHEET_UUID +
                 '\t\t\t\t\t\t(reference "%s")\n\t\t\t\t\t\t(unit 1)\n' % ref +
                 "\t\t\t\t\t)\n\t\t\t\t)\n\t\t\t)\n\t\t)\n")
        self.body.append("".join(s))
        self.n_sym += 1
        return abspins

    def pin(self, ref, num):
        return self.pos[ref][num]

    # ---- Versorgung ------------------------------------------------
    _pwr_n = [0]

    def power(self, net, x, y):
        """Power-Symbol setzen. Sein Pin liegt genau auf (x, y);
        die Grafik von +5V/+3V3 zeigt nach oben, die von GND nach unten."""
        self._pwr_n[0] += 1
        return self.place("#PWR%02d" % self._pwr_n[0], "power", net, net,
                          x, y, hide_fields=True)

    def pwrflag(self, x, y):
        """PWR_FLAG. Power-Symbole BENENNEN ihr Netz, sie TREIBEN es
        nicht -- ihr Pin ist vom Typ power_in. Ohne eine echte Quelle
        meldet ERC power_pin_not_driven. PWR_FLAG hat einen power_out-Pin
        und ist genau dafuer da."""
        self._pwr_n[0] += 1
        return self.place("#FLG%02d" % self._pwr_n[0], "power", "PWR_FLAG",
                          "PWR_FLAG", x, y, hide_fields=True)

    # ---- Leitungen -------------------------------------------------
    def wire(self, *points):
        """Leitungszug aus rechtwinkligen Teilstuecken."""
        pts = list(points)
        for px, py in pts:
            self._grid(px, py)
        for a, b in zip(pts, pts[1:]):
            if a == b:
                continue
            if abs(a[0] - b[0]) > 1e-9 and abs(a[1] - b[1]) > 1e-9:
                raise ValueError("Schraege Leitung %s -> %s" % (a, b))
            self.graph.append(emit_wire(a[0], a[1], b[0], b[1]))
            self.n_wire += 1

    def junction(self, x, y):
        self._grid(x, y)
        self.graph.append(emit_junction(x, y))
        self.n_junc += 1

    def noconnect(self, x, y):
        self.graph.append(emit_noconnect(x, y))
        self.n_nc += 1

    def area(self, x1, y1, x2, y2, title):
        self.graph.append(emit_rect(x1, y1, x2, y2))
        self.graph.append(emit_text(title, x1 + 3.81, y1 + 7.62, size=3.0, bold=True))

    def note(self, s, x, y):
        self.graph.append(emit_text(s, x, y, size=1.27))

    # ---- Datei -----------------------------------------------------
    def render(self):
        if self.offgrid:
            raise ValueError("%d Koordinaten nicht auf dem 1,27-mm-Raster, z.B. %s"
                             % (len(self.offgrid), sorted(set(self.offgrid))[:6]))
        out = ["(kicad_sch\n\t(version 20250114)\n",
               '\t(generator "marsrover")\n\t(generator_version "10.0")\n',
               '\t(uuid "%s")\n\t(paper "%s")\n' % (SHEET_UUID, PAPER),
               "\t(title_block\n"
               '\t\t(title "One-of-a-Kind Mars Rover")\n'
               '\t\t(company "Darklirah")\n'
               '\t\t(comment 1 "Bereiche: Rechenkern, Motortreiber, Sensoren, Akku und Ladevorrichtung")\n'
               '\t\t(comment 2 "Verdrahtung als gezeichnete Leitungen, Versorgung ueber Power-Symbole")\n'
               "\t)\n",
               "\t(lib_symbols\n"]
        for block in self.libs.values():
            out.append("\t\t" + block.replace("\n", "\n\t\t") + "\n")
        out.append("\t)\n")
        out.extend(self.graph)
        out.extend(self.body)
        out.append('\t(sheet_instances\n\t\t(path "/"\n\t\t\t(page "1")\n\t\t)\n\t)\n)\n')
        return "".join(out)


# =====================================================================
#  Fahrspuren im Gang zwischen Motorbereich und Rechenkern
#  Jedes Signal bekommt eine eigene Senkrechte, damit sich nichts kreuzt.
# =====================================================================
LANE_MOT_L = 218.44
LANE_MOT_R = 220.98
LANE_LINE_R = 223.52
LANE_LINE_L = 226.06
LANE_VBAT = 228.60
LANE_LED_FL = 231.14


def motorstufe(s, dy, ref_m, ref_q, ref_d, ref_c, ref_rs, ref_rp, motorname):
    """Eine Motorendstufe. dy verschiebt die ganze Stufe nach unten.

    Aufbau von oben nach unten:  +5V -> Motor -> Drain -> MOSFET -> GND.
    Die Freilaufdiode liegt links daneben in Sperrichtung ueber dem Motor,
    der Entstoerkondensator rechts daneben vom Drain nach Masse.
    Das Gate kommt von links ueber den Vorwiderstand, der Pulldown haelt
    es unten, solange der ESP32 den Pin noch nicht treibt.
    """
    x = 160.02                      # Hauptsaeule Motor/Drain
    tap5 = 35.56 + dy               # Abzweig auf der 5-V-Seite
    tapd = 58.42 + dy               # Abzweig auf der Drain-Seite
    ygate = 71.12 + dy

    s.power("+5V", x, 33.02 + dy)
    m = s.place(ref_m, "Motor", "Motor_DC", motorname, x, 43.18 + dy, foff=6.35)
    q = s.place(ref_q, "Transistor_FET", "Q_NMOS_GSD", "AO3400A",
                157.48, ygate, foff=7.62)
    d = s.place(ref_d, "Diode", "1N5819", "1N5819", 142.24, 46.99 + dy,
                rot=270, foff=3.81)
    c = s.place(ref_c, "Device", "C", "100n", 177.80, 66.04 + dy, foff=3.81)
    rs = s.place(ref_rs, "Device", "R", "100R", 133.35, ygate,
                 rot=90, fields="V", foff=3.81)
    rp = s.place(ref_rp, "Device", "R", "100k", 144.78, 78.74 + dy, foff=3.81)

    # 5-V-Saeule mit Abzweig zur Diodenkathode
    s.wire((x, 33.02 + dy), (x, tap5))
    s.wire((x, tap5), m["1"])
    s.wire(d["1"], (142.24, tap5), (x, tap5))
    s.junction(x, tap5)

    # Drain-Knoten: Motor, Diodenanode, Kondensator, MOSFET
    s.wire(m["2"], (x, tapd))
    s.wire((x, tapd), q["3"])
    s.wire(d["2"], (142.24, tapd), (x, tapd))
    s.wire(c["1"], (177.80, tapd), (x, tapd))
    s.junction(x, tapd)

    # Masse
    s.wire(c["2"], (177.80, 76.20 + dy))
    s.power("GND", 177.80, 76.20 + dy)
    s.wire(q["2"], (x, 83.82 + dy))
    s.power("GND", x, 83.82 + dy)

    # Gate: Vorwiderstand in Reihe, Pulldown nach Masse
    s.wire(rs["2"], (144.78, ygate))
    s.wire((144.78, ygate), q["1"])
    s.wire((144.78, ygate), rp["1"])
    s.junction(144.78, ygate)
    s.wire(rp["2"], (144.78, 86.36 + dy))
    s.power("GND", 144.78, 86.36 + dy)

    return rs["1"]                  # Einspeisepunkt fuer das Gate-Signal


def liniensensor(s, x0, ref_os, ref_rled, ref_re, name):
    """Ein Reflexkoppler als Emitterfolger.

    Die Infrarot-Diode haengt ueber 150 Ohm an 3,3 V, der Fototransistor
    mit dem Kollektor ebenfalls an 3,3 V. Am Emitter liegt der
    10-k-Widerstand nach Masse -- dort wird abgegriffen. Heller
    Untergrund = viel Reflexion = hoher Wert.
    """
    xa, xc = x0 - 2.54, x0 + 2.54
    rail = 177.80
    node = 219.71

    rled = s.place(ref_rled, "Device", "R", "150R", xa, 185.42, foff=3.81)
    os = s.place(ref_os, "MarsRover", "TCRT5000", name, x0, 203.20, foff=7.62)
    re = s.place(ref_re, "Device", "R", "10k", xc, 228.60, foff=3.81)

    # 3,3-V-Schiene ueber beide Zweige
    s.wire(rled["1"], (xa, rail))
    s.wire((xa, rail), (x0, rail))
    s.wire((x0, rail), (xc, rail))
    s.wire((xc, rail), os["3"])
    s.power("+3V3", x0, 170.18)
    s.wire((x0, 170.18), (x0, rail))
    s.junction(x0, rail)

    s.wire(rled["2"], os["1"])
    s.wire(os["2"], (xa, node))
    s.power("GND", xa, node)

    # Emitter -> Abgriff -> 10 k nach Masse
    s.wire(os["4"], (xc, node))
    s.wire((xc, node), re["1"])
    s.junction(xc, node)
    s.wire(re["2"], (xc, 236.22))
    s.power("GND", xc, 236.22)

    return (xc, node)               # Abgriffpunkt


def ledzweig(s, y, ref_r, ref_led, wert):
    """GPIO -> 220 Ohm -> LED -> Masse. Rund 5 mA, der ESP32 darf 29 mA.

    Kein MOSFET: Tabelle 14 des ESP32-WROOM-32-Datenblatts nennt fuer die
    Domaene VDD3P3_RTC typisch 40 mA je Pin, bei vier gleichzeitig
    treibenden Pins noch rund 29 mA. Alle vier LED-Pins liegen in dieser
    Domaene.
    """
    r = s.place(ref_r, "Device", "R", "220R", 414.02, y,
                rot=90, fields="V", foff=3.81)
    led = s.place(ref_led, "Device", "LED", wert, 433.07, y,
                  rot=180, fields="V", foff=3.81)
    s.wire(r["2"], led["2"])
    s.wire(led["1"], (444.50, y))
    s.power("GND", 444.50, y)
    return r["1"]                   # Einspeisepunkt


# =====================================================================
#  Aufbau
# =====================================================================
def build():
    s = Sheet()

    s.area(111.76, 20.32, 194.31, 163.83, "MOTORTREIBER")
    s.area(92.71, 166.37, 194.31, 259.08, "SENSOREN  Linienerkennung")
    s.area(233.68, 20.32, 316.23, 163.83, "RECHENKERN")
    s.area(318.77, 20.32, 400.05, 163.83, "SENSOREN  Lage und Abstand")
    s.area(406.40, 165.10, 459.74, 226.06, "SIGNAL-LEDS")
    s.area(20.32, 274.32, 240.03, 361.95, "AKKU UND LADEVORRICHTUNG")

    # ---- Rechenkern ------------------------------------------------
    u1 = s.place("U1", "Espressif", "ESP32-DevKitC", "ESP32-DevKitC V4",
                 274.32, 109.22, fields="V", foff=50.80)

    # 3,3 V kommt vom AMS1117 auf dem DevKit selbst
    s.wire(u1["1"], (276.86, 66.04))
    s.wire((276.86, 66.04), (276.86, 63.50))
    s.power("+3V3", 276.86, 63.50)
    s.wire((276.86, 66.04), (284.48, 66.04))
    s.pwrflag(284.48, 66.04)
    s.junction(276.86, 66.04)

    s.wire(u1["19"], (274.32, 68.58), (264.16, 68.58), (264.16, 60.96))
    s.power("+5V", 264.16, 60.96)

    s.wire(u1["14"], (274.32, 152.40))
    s.power("GND", 274.32, 152.40)

    for nr in ["2", "3", "13", "15", "16", "17", "18", "20", "21", "22",
               "23", "24", "25", "26", "27", "28", "29", "30", "31",
               "34", "35", "37"]:
        s.noconnect(*u1[nr])

    # ---- Motortreiber ----------------------------------------------
    gate_l = motorstufe(s, 0.0, "M1", "Q1", "D1", "C3", "R1", "R3",
                        "Vibrationsmotor links")
    gate_r = motorstufe(s, 68.58, "M2", "Q2", "D2", "C4", "R2", "R4",
                        "Vibrationsmotor rechts")

    # GPIO32 / GPIO33 laufen oberhalb bzw. zwischen den Stufen heran
    s.wire(u1["7"], (LANE_MOT_L, 83.82), (LANE_MOT_L, 26.67),
           (119.38, 26.67), (119.38, 71.12), gate_l)
    s.wire(u1["8"], (LANE_MOT_R, 86.36), (LANE_MOT_R, 91.44),
           (116.84, 91.44), (116.84, 139.70), gate_r)

    # ---- Liniensensoren --------------------------------------------
    tap_l = liniensensor(s, 101.60, "OS1", "R5", "R7", "TCRT5000 links")
    tap_r = liniensensor(s, 152.40, "OS2", "R6", "R8", "TCRT5000 rechts")

    s.wire(tap_l, (116.84, 219.71), (116.84, 256.54),
           (LANE_LINE_L, 256.54), (LANE_LINE_L, 104.14), u1["5"])
    s.wire(tap_r, (167.64, 219.71), (167.64, 251.46),
           (LANE_LINE_R, 251.46), (LANE_LINE_R, 106.68), u1["6"])

    # ---- Lage- und Abstandssensor am I2C-Bus ------------------------
    # Fertigmodule, deshalb als Steckverbinder gezeichnet. Die Pull-ups
    # bringen die Module mit, sie fehlen hier nicht.
    u6 = s.place("U6", "Connector_Generic", "Conn_01x04", "GY-521 MPU-6050",
                 360.68, 91.44, foff=12.70)
    u7 = s.place("U7", "Connector_Generic", "Conn_01x04", "GY-530 VL53L0X",
                 360.68, 127.00, foff=12.70)

    # SDA laeuft innen, SCL aussen herum -- so kreuzen sich die beiden
    # Zuleitungen nicht. Eine Kreuzung bleibt unten am Bus uebrig; die
    # ist bei einem Zweidrahtbus zu zwei Teilnehmern nicht vermeidbar.
    s.wire(u1["33"], (322.58, 101.60), (322.58, 80.01),
           (337.82, 80.01), (337.82, 96.52))
    s.wire((337.82, 96.52), (337.82, 132.08))
    s.wire((337.82, 96.52), u6["4"])
    s.junction(337.82, 96.52)
    s.wire((337.82, 132.08), u7["4"])
    s.junction(337.82, 132.08)

    s.wire(u1["36"], (325.12, 104.14), (325.12, 85.09),
           (332.74, 85.09), (332.74, 93.98))
    s.wire((332.74, 93.98), (332.74, 129.54))
    s.wire((332.74, 93.98), u6["3"])
    s.junction(332.74, 93.98)
    s.wire((332.74, 129.54), u7["3"])
    s.junction(332.74, 129.54)

    for mod, y1, y2 in ((u6, 88.90, 91.44), (u7, 124.46, 127.00)):
        s.wire(mod["1"], (347.98, y1), (347.98, y1 - 5.08))
        s.power("+3V3", 347.98, y1 - 5.08)
        s.wire(mod["2"], (347.98, y2))
        s.power("GND", 347.98, y2)

    # ---- Signal-LEDs ------------------------------------------------
    f_rr = ledzweig(s, 177.80, "R12", "LED4", "rot - hinten rechts")
    f_rl = ledzweig(s, 190.50, "R11", "LED3", "rot - hinten links")
    f_fr = ledzweig(s, 203.20, "R10", "LED2", "gelb - vorne rechts")
    f_fl = ledzweig(s, 215.90, "R9", "LED1", "gelb - vorne links")

    # Drei LEDs haengen an den rechten GPIOs, die vierte an GPIO14 links.
    # Die drei rechten laufen gestaffelt, damit sich keine zwei kreuzen.
    s.wire(u1["9"], (320.04, 109.22), (320.04, 168.91),
           (405.13, 168.91), (405.13, 177.80), f_rr)
    s.wire(u1["10"], (317.50, 111.76), (317.50, 171.45),
           (402.59, 171.45), (402.59, 190.50), f_rl)
    s.wire(u1["11"], (314.96, 114.30), (314.96, 173.99),
           (400.05, 173.99), (400.05, 203.20), f_fr)
    # GPIO14 sitzt auf der linken Seite des ESP32 und muss aussen herum.
    # Es bekommt die unterste Reihe, dann kreuzt es die anderen drei nicht.
    s.wire(u1["12"], (LANE_LED_FL, 96.52), (LANE_LED_FL, 266.70),
           (408.94, 266.70), (408.94, 215.90), f_fl)

    # ---- Akku und Ladevorrichtung -----------------------------------
    bt1 = s.place("BT1", "Device", "Battery_Cell", "LiPo 3,7V 500mAh",
                  38.10, 292.10, fields="R", foff=5.08)
    j2 = s.place("J2", "Connector_Generic", "Conn_01x02", "JST-PH 2,0",
                 66.04, 287.02, foff=7.62)

    s.wire(bt1["1"], (53.34, 287.02))
    s.wire((53.34, 287.02), j2["1"])
    s.junction(53.34, 287.02)
    s.wire(bt1["2"], (48.26, 294.64))
    s.wire((48.26, 294.64), (48.26, 289.56), j2["2"])
    s.wire((48.26, 294.64), (48.26, 298.45))
    s.junction(48.26, 294.64)
    s.wire((48.26, 298.45), (48.26, 302.26))
    s.power("GND", 48.26, 302.26)
    s.wire((48.26, 298.45), (40.64, 298.45))
    s.pwrflag(40.64, 298.45)
    s.junction(48.26, 298.45)

    u2 = s.place("U2", "Connector_Generic", "Conn_01x06", "TP4056-Modul",
                 104.14, 304.80, foff=12.70)

    # Akkuplus zum Lademodul, oben herum an den Masseabgriffen vorbei
    s.wire((53.34, 287.02), (53.34, 281.94), (91.44, 281.94),
           (91.44, 304.80), u2["3"])

    # Die drei Massepins auf eine gemeinsame Senkrechte
    s.wire(u2["2"], (83.82, 302.26))
    s.wire(u2["4"], (83.82, 307.34))
    s.wire(u2["6"], (83.82, 312.42))
    s.wire((83.82, 302.26), (83.82, 307.34))
    s.wire((83.82, 307.34), (83.82, 312.42))
    s.wire((83.82, 312.42), (83.82, 317.50))
    s.junction(83.82, 307.34)
    s.junction(83.82, 312.42)
    s.power("GND", 83.82, 317.50)

    # USB sitzt auf dem Fertigmodul, auf unserer Platine liegt hier nichts
    s.noconnect(*u2["1"])
    s.note("USB-C-Buchse sitzt auf dem TP4056-Fertigmodul,", 106.68, 290.83)
    s.note("nicht auf dieser Platine", 106.68, 293.37)

    s1 = s.place("S1", "Switch", "SW_SPDT", "Schiebeschalter",
                 132.08, 309.88, fields="V", foff=6.35)
    # Der geschuetzte Akkuplus verlaesst das Lademodul links und laeuft
    # OBERHALB der Gehaeuse zum Schalter. Direkt waagerecht ginge die
    # Leitung mitten durch U2 hindurch.
    s.wire(u2["5"], (72.39, 309.88), (72.39, 297.18),
           (127.00, 297.18), s1["2"])
    s.noconnect(*s1["3"])

    u5 = s.place("U5", "Connector_Generic", "Conn_01x04", "MT3608 auf 5,00V",
                 167.64, 307.34, foff=12.70)
    s.wire(s1["1"], (149.86, 307.34), (149.86, 304.80), u5["1"])
    s.wire(u5["2"], (154.94, 307.34))
    s.power("GND", 154.94, 307.34)
    s.wire(u5["4"], (154.94, 312.42))
    s.power("GND", 154.94, 312.42)

    # 5-V-Schiene mit Siebung, unterhalb des Step-up gefuehrt
    c1 = s.place("C1", "Device", "C_Polarized", "100u/16V", 187.96, 326.39,
                 foff=3.81)
    c2 = s.place("C2", "Device", "C", "10u", 203.20, 326.39, foff=3.81)
    s.wire(u5["3"], (146.05, 309.88), (146.05, 320.04), (187.96, 320.04))
    s.wire((187.96, 320.04), (203.20, 320.04))
    s.wire((203.20, 320.04), (218.44, 320.04))
    for cc, xx in ((c1, 187.96), (c2, 203.20)):
        s.wire((xx, 320.04), cc["1"])
        s.junction(xx, 320.04)
        s.wire(cc["2"], (xx, 334.01))
        s.power("GND", xx, 334.01)
    s.wire((218.44, 320.04), (218.44, 316.23))
    s.wire((218.44, 316.23), (218.44, 312.42))
    s.power("+5V", 218.44, 312.42)
    s.wire((218.44, 316.23), (226.06, 316.23))
    s.pwrflag(226.06, 316.23)
    s.junction(218.44, 316.23)

    # Akkuspannungsteiler, optional (FEATURE_BATTERY_MONITOR).
    # Haengt links am geschuetzten Akkuplus, weit weg vom Rest.
    r16 = s.place("R16", "Device", "R", "100k", 76.20, 322.58, foff=3.81)
    r17 = s.place("R17", "Device", "R", "100k", 76.20, 340.36, foff=3.81)
    s.wire((76.20, 297.18), r16["1"])
    s.junction(76.20, 297.18)
    s.wire(r16["2"], (76.20, 332.74))
    s.wire((76.20, 332.74), r17["1"])
    s.junction(76.20, 332.74)
    s.wire(r17["2"], (76.20, 347.98))
    s.power("GND", 76.20, 347.98)
    s.wire((76.20, 332.74), (68.58, 332.74), (68.58, 355.60),
           (LANE_VBAT, 355.60), (LANE_VBAT, 111.76), u1["4"])

    return s


if __name__ == "__main__":
    sheet = build()
    text = sheet.render()
    target = HERE / ("%s.kicad_sch" % PROJECT)
    target.write_text(text, encoding="utf-8")
    print("Geschrieben: %s  (%d Zeichen, Blatt %s)"
          % (target.name, len(text), PAPER))
    print("Symbole:      %d" % sheet.n_sym)
    print("Leitungen:    %d" % sheet.n_wire)
    print("Knotenpunkte: %d" % sheet.n_junc)
    print("Nicht-angeschlossen: %d" % sheet.n_nc)
    print("Netzlabels:   0")
