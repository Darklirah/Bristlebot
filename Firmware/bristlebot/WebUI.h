// =====================================================================
//  Bristlebot -- WebUI.h
//  Die komplette Bedienoberflaeche als eine Datei im Flash.
//
//  Gestaltungsvorgabe: einfach bedienbar, geeignet ab 14 Jahren.
//   * Klartext-Befehle, keine Kuerzel -- "warte 2,0 s", nicht "wa,2000"
//   * Baukasten statt Texteditor: Syntaxfehler sind nicht moeglich
//   * grosse Tippflaechen, Farbcodierung der Befehlsarten
//   * Reglerabstimmung (Kp/Kd) hinter einem zugeklappten Experten-Bereich
//   * jeder Speicherplatz zeigt belegte und freie Schritte an
//
//  Bewusst ohne jede externe Quelle (kein CDN, keine Webfonts): im
//  Access-Point-Betrieb gibt es keinen Internetzugang, alles was nicht
//  mitgeliefert wird, wuerde fehlen.
//
//  Abgestimmt auf die Eigenheiten von iOS Safari:
//   * Pointer Events statt Touch Events
//   * touch-action/overscroll gesperrt, damit das Wischen auf dem
//     Joystick nicht die Seite scrollt
//   * gesturestart abgefangen (kein Pinch-Zoom)
//   * maximum-scale=1, damit kein Doppeltipp-Zoom stoert
// =====================================================================
#pragma once
#include <Arduino.h>

static const char WEBUI_HTML[] PROGMEM = R"BBUI(<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no,viewport-fit=cover">
<meta name="theme-color" content="#11141a">
<meta name="apple-mobile-web-app-capable" content="yes">
<title>Bristlebot</title>
<style>
:root{
  --bg:#11141a; --panel:#1b2029; --panel2:#232a35; --panel3:#2b3340; --line:#2e3746;
  --fg:#e8edf5; --dim:#8c97a8;
  --accent:#4da3ff; --ok:#39d98a; --warn:#ffc24b; --bad:#ff5c6c; --purple:#b990ff;
  --pad:14px;
}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
html,body{margin:0;height:100%}
body{
  background:var(--bg); color:var(--fg);
  font:15px/1.4 -apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif;
  overscroll-behavior:none; user-select:none; -webkit-user-select:none;
  padding:env(safe-area-inset-top) env(safe-area-inset-right) env(safe-area-inset-bottom) env(safe-area-inset-left);
}
.wrap{max-width:560px;margin:0 auto;padding:var(--pad);display:flex;flex-direction:column;gap:var(--pad)}
header{display:flex;align-items:center;gap:10px}
h1{font-size:17px;margin:0;font-weight:600;letter-spacing:.2px;flex:1}
#dot{width:10px;height:10px;border-radius:50%;background:var(--bad);flex:0 0 auto;
     box-shadow:0 0 0 3px rgba(255,92,108,.15)}
#dot.on{background:var(--ok);box-shadow:0 0 0 3px rgba(57,217,138,.18)}
#conn{font-size:12px;color:var(--dim)}
.card{background:var(--panel);border:1px solid var(--line);border-radius:14px;padding:var(--pad)}
.card h2{font-size:13px;margin:0 0 10px;color:var(--dim);font-weight:600;
         text-transform:uppercase;letter-spacing:.06em}
.row{display:flex;gap:8px;align-items:center}

/* --- Betriebsart: drei grosse Flaechen --- */
.modes{display:grid;grid-template-columns:repeat(3,1fr);gap:8px}
.modes button{border:1px solid var(--line);background:var(--panel2);color:var(--dim);
  font:inherit;font-weight:600;font-size:13px;padding:14px 4px;border-radius:12px;line-height:1.25}
.modes button.sel{background:var(--accent);border-color:var(--accent);color:#06121f}
.modes button i{display:block;font-style:normal;font-size:20px;margin-bottom:4px;line-height:1}

.big{width:100%;border:0;border-radius:13px;font:inherit;font-weight:700;font-size:16px;
     padding:16px;color:#06121f;background:var(--ok)}
.big.armed{background:var(--bad);color:#fff}
.btn{border:1px solid var(--line);background:var(--panel2);color:var(--fg);font:inherit;
     padding:12px 14px;border-radius:11px;font-weight:600}
.btn:active{background:var(--panel3)}
.btn.pri{background:var(--accent);border-color:var(--accent);color:#06121f}
.btn.sm{padding:9px 11px;font-size:13px}

#pad{position:relative;width:100%;aspect-ratio:1/1;max-height:44vh;margin:0 auto;
     background:var(--panel2);border:1px solid var(--line);border-radius:22px;
     touch-action:none;overflow:hidden}
#cross:before,#cross:after{content:"";position:absolute;background:var(--line)}
#cross:before{left:8%;right:8%;top:50%;height:1px}
#cross:after{top:8%;bottom:8%;left:50%;width:1px}
#knob{position:absolute;width:30%;height:30%;left:35%;top:35%;border-radius:50%;
      background:var(--accent);opacity:.92;box-shadow:0 6px 18px #0008;
      transition:transform .08s ease-out}
#padhint{position:absolute;left:0;right:0;bottom:9px;text-align:center;font-size:11px;color:var(--dim)}

label{display:block;font-size:12px;color:var(--dim);margin:0 0 6px}
label b{color:var(--fg);font-variant-numeric:tabular-nums;font-weight:600}
input[type=range]{width:100%;margin:0 0 12px;accent-color:var(--accent);height:30px}
select,input[type=text]{width:100%;background:var(--panel2);color:var(--fg);font:inherit;
  border:1px solid var(--line);border-radius:10px;padding:11px;margin:0 0 10px}

/* --- Speicherplaetze --- */
.slots{display:grid;grid-template-columns:repeat(2,1fr);gap:8px;margin-bottom:12px}
.slots button{border:1px solid var(--line);background:var(--panel2);color:var(--fg);
  font:inherit;text-align:left;padding:10px 11px;border-radius:11px}
.slots button.sel{border-color:var(--accent);background:#1d2b3d}
.slots button span{display:block;font-weight:600;font-size:13.5px;
  overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.slots button em{display:block;font-style:normal;font-size:11px;color:var(--dim);margin-top:2px}
.slots button.sel em{color:var(--accent)}

/* --- Schrittliste --- */
#steps{list-style:none;margin:0 0 10px;padding:0;display:flex;flex-direction:column;gap:5px}
#steps li{display:flex;align-items:center;gap:8px;background:var(--panel2);
  border-radius:9px;padding:9px 8px 9px 0;border-left:4px solid var(--dim)}
#steps li.drive{border-left-color:var(--accent)}
#steps li.wait {border-left-color:var(--dim)}
#steps li.light{border-left-color:var(--warn)}
#steps li.loop {border-left-color:var(--purple)}
#steps li.now{background:#243a2e;box-shadow:inset 0 0 0 1px var(--ok)}
#steps li var{font-style:normal;color:var(--dim);font-size:11.5px;width:22px;
  text-align:right;font-variant-numeric:tabular-nums;flex:0 0 auto}
#steps li p{margin:0;flex:1;font-size:14px}
#steps li .acts{display:flex;gap:3px;flex:0 0 auto}
#steps li .acts button{border:0;background:var(--panel3);color:var(--dim);
  width:30px;height:30px;border-radius:8px;font-size:14px;font-weight:700;line-height:1;padding:0}
#steps li .acts button.del{color:var(--bad)}
#empty{color:var(--dim);font-size:13px;text-align:center;padding:14px 0 18px}

/* --- Schrittzaehler --- */
.gauge{margin:0 0 12px}
.gauge .t{display:flex;justify-content:space-between;font-size:12px;color:var(--dim);margin-bottom:5px}
.gauge .t b{color:var(--fg)}
.gauge .t b.full{color:var(--bad)}
.gauge .g{height:6px;background:var(--panel2);border-radius:3px;overflow:hidden}
.gauge .g i{display:block;height:100%;width:0;background:var(--accent);transition:width .15s}
.gauge .g i.full{background:var(--bad)}

/* --- Befehl hinzufuegen --- */
.add{background:var(--panel2);border:1px dashed var(--line);border-radius:12px;padding:12px;margin-bottom:12px}
.add h3{margin:0 0 9px;font-size:13px;color:var(--fg);font-weight:600}
.add select,.add input[type=text]{background:var(--panel);margin-bottom:9px}
.add .two{display:grid;grid-template-columns:1fr 1fr;gap:9px}

.bars{display:grid;grid-template-columns:auto 1fr;gap:5px 9px;align-items:center;font-size:12px;color:var(--dim)}
.bar{height:9px;background:var(--panel2);border-radius:5px;overflow:hidden;position:relative}
.bar i{display:block;height:100%;width:0;background:var(--accent);transition:width .1s linear}
.bar.err i{position:absolute;left:50%;background:var(--warn)}
#status{font-size:13px;color:var(--dim);min-height:18px}
#status.bad{color:var(--bad)} #status.warn{color:var(--warn)} #status.ok{color:var(--ok)}
#toast{font-size:13px;min-height:17px;margin-top:4px}
#toast.ok{color:var(--ok)} #toast.bad{color:var(--bad)}
#prun{font-size:13px;color:var(--ok);font-variant-numeric:tabular-nums;min-height:17px;margin-top:8px}
#imuline{font-size:12px;color:var(--dim);margin-top:9px;font-variant-numeric:tabular-nums}
#imuline.warn{color:var(--warn)}
#tres{font-size:12.5px;margin-top:6px}
#tres div{margin-top:2px;color:var(--dim)}
#tres div.ok{color:var(--ok)} #tres div.bad{color:var(--bad)} #tres div.warn{color:var(--warn)}
#tphase{font-size:13px;color:var(--warn);font-variant-numeric:tabular-nums;min-height:17px;margin-top:8px}

details{background:var(--panel2);border-radius:11px;padding:0}
details summary{padding:12px var(--pad);font-size:13px;font-weight:600;color:var(--dim);
  cursor:pointer;list-style:none}
details summary::-webkit-details-marker{display:none}
details summary:before{content:"\25B8  "}
details[open] summary:before{content:"\25BE  "}
details .inner{padding:0 var(--pad) 6px}

.hide{display:none!important}
.note{font-size:11.5px;color:var(--dim);margin:2px 0 0}
</style>
</head>
<body>
<div class="wrap">

  <header>
    <div id="dot"></div>
    <h1>Bristlebot</h1>
    <div id="conn">verbinde&hellip;</div>
  </header>

  <button id="arm" class="big">Freigeben</button>

  <div class="card">
    <h2>Was soll er machen?</h2>
    <div class="modes">
      <button id="m1"><i>&#9473;</i>Linie<br>folgen</button>
      <button id="m0" class="sel"><i>&#9679;</i>Selbst<br>fahren</button>
      <button id="m2"><i>&#9654;</i>Fahr-<br>programm</button>
    </div>
  </div>

  <!-- ============ Selbst fahren ============ -->
  <div class="card" id="panManual">
    <div id="pad"><div id="cross"></div><div id="knob"></div>
      <div id="padhint">hoch = schneller &middot; seitlich = lenken</div>
    </div>
    <p class="note">R&uuml;ckw&auml;rts gibt es bauartbedingt nicht: ein Vibrationsmotor
       treibt den Roboter nur in Borstenrichtung. Joystick nach unten = Halt.</p>
  </div>

  <!-- ============ Linie folgen ============ -->
  <div class="card hide" id="panAuto">
    <label>Grundgeschwindigkeit <b id="vBase">55</b> %</label>
    <input type="range" id="sBase" min="0" max="100" value="55">
    <button class="btn" id="bCal" style="width:100%">Linie kalibrieren</button>
    <p class="note">Antippen und den Roboter 5 Sekunden langsam quer &uuml;ber die
       Linie hin und her schwenken, so dass beide Sensoren einmal die Linie und
       einmal den hellen Untergrund sehen.</p>
  </div>

  <!-- ============ Fahrprogramm ============ -->
  <div class="card hide" id="panProg">
    <h2>Speicherplatz</h2>
    <div class="slots" id="slots"></div>

    <label>Name</label>
    <input type="text" id="pname" maxlength="20" placeholder="z.B. Achter fahren">

    <h2>Ablauf</h2>
    <ol id="steps"></ol>
    <div id="empty">Noch leer &mdash; unten einen Befehl hinzuf&uuml;gen.</div>

    <div class="gauge">
      <div class="t"><span>Schritte</span>
        <span><b id="gUsed">0</b> von <b id="gMax">48</b> belegt &middot; <b id="gFree">48</b> frei</span></div>
      <div class="g"><i id="gBar"></i></div>
    </div>

    <div class="add">
      <h3>Befehl hinzuf&uuml;gen</h3>
      <select id="aOp">
        <optgroup label="Fahren">
          <option value="ge">geradeaus fahren</option>
          <option value="li">Kurve nach links</option>
          <option value="re">Kurve nach rechts</option>
          <option value="dl">drehen nach links um &hellip; Grad</option>
          <option value="dr">drehen nach rechts um &hellip; Grad</option>
          <option value="fh">fahren bis Hindernis</option>
          <option value="tl">drehen nach links, bis frei</option>
          <option value="tr">drehen nach rechts, bis frei</option>
          <option value="wf">warten, bis der Weg frei ist</option>
          <option value="st">anhalten</option>
        </optgroup>
        <optgroup label="Zeit">
          <option value="wa">warten</option>
        </optgroup>
        <optgroup label="Licht">
          <option value="ld">LED schalten</option>
          <option value="bf">Blinkfrequenz einstellen</option>
        </optgroup>
        <optgroup label="Ablauf">
          <option value="lo">von vorn wiederholen</option>
        </optgroup>
      </select>

      <div id="pRad" class="hide">
        <label>Kurvenradius Stufe <b id="vRad">3</b> &ndash; <span id="tRad">eng</span></label>
        <input type="range" id="aRad" min="1" max="10" value="3">
      </div>
      <div id="pDeg" class="hide">
        <label>Drehwinkel <b id="vDeg">90</b>&deg;</label>
        <input type="range" id="aDeg" min="5" max="360" step="5" value="90">
      </div>
      <div id="pObs" class="hide">
        <label>anhalten bei <b id="vObs">150</b> mm Abstand</label>
        <input type="range" id="aObs" min="40" max="1200" step="10" value="150">
      </div>
      <div id="pSpd" class="hide">
        <label>Geschwindigkeit <b id="vSpd">60</b> %</label>
        <input type="range" id="aSpd" min="1" max="100" value="60">
      </div>
      <div id="pWa" class="hide">
        <label>Wartezeit <b id="vWa">2,0</b> Sekunden</label>
        <input type="range" id="aWa" min="1" max="200" value="20">
      </div>
      <div id="pLd" class="hide">
        <div class="two">
          <select id="aLdT">
            <option value="0">LED vorne links</option>
            <option value="1">LED vorne rechts</option>
            <option value="2">LED hinten links</option>
            <option value="3">LED hinten rechts</option>
            <option value="4">beide vorne</option>
            <option value="5">beide hinten</option>
            <option value="6">alle vier</option>
          </select>
          <select id="aLdS">
            <option value="1">einschalten</option>
            <option value="0">ausschalten</option>
            <option value="2">blinken</option>
          </select>
        </div>
      </div>
      <div id="pBf" class="hide">
        <label>Blinken Stufe <b id="vBf">4</b> &ndash; <span id="tBf">2,0</span> mal pro Sekunde</label>
        <input type="range" id="aBf" min="1" max="10" value="4">
      </div>
      <div id="pLo" class="hide">
        <label>Wiederholungen <b id="vLo">0</b> &ndash; <span id="tLo">endlos</span></label>
        <input type="range" id="aLo" min="0" max="10" value="0">
      </div>

      <button class="btn pri" id="bAdd" style="width:100%">+ Einf&uuml;gen</button>
    </div>

    <div class="row">
      <button class="btn pri" id="bSave" style="flex:1">Speichern</button>
      <button class="btn" id="bReload">Verwerfen</button>
      <button class="btn" id="bClear">Leeren</button>
    </div>
    <div id="toast"></div>

    <button class="big" id="bRun" style="margin-top:12px">&#9654;&ensp;Programm starten</button>
    <div id="prun"></div>
    <p class="note">Der Roboter hat keine Wegmessung. Ein Programm l&auml;uft rein
       nach der Zeit &ndash; dieselben 2 Sekunden sind je nach Akkustand und
       Untergrund eine unterschiedliche Strecke.</p>
  </div>

  <!-- ============ Messwerte ============ -->
  <div class="card">
    <h2>Messwerte</h2>
    <div class="bars">
      <span>Sensor L</span><div class="bar"><i id="bL"></i></div>
      <span>Sensor R</span><div class="bar"><i id="bR"></i></div>
      <span>Ablage</span><div class="bar err"><i id="bE"></i></div>
      <span>Motor L</span><div class="bar"><i id="bML" style="background:var(--ok)"></i></div>
      <span>Motor R</span><div class="bar"><i id="bMR" style="background:var(--ok)"></i></div>
    </div>
    <div id="imuline"></div>
    <div id="status" style="margin-top:10px">&mdash;</div>
  </div>

  <!-- ============ Funktionstest ============ -->
  <div class="card">
    <h2>Funktionstest</h2>
    <button class="big" id="bTest" style="background:var(--warn)">Funktionstest starten</button>
    <div id="tphase"></div>
    <div id="tres"></div>
    <p class="note">Pr&uuml;ft alles der Reihe nach: die vier LEDs einzeln, dann alle
       gemeinsam blinkend, dann Motor links von langsam auf schnell, dann Motor
       rechts. Beim Motortest leuchtet die LED der getesteten Seite &ndash; so f&auml;llt
       auf, wenn Motor und Seite vertauscht verdrahtet sind. Zum Schluss dreht er
       sich einmal nach rechts und einmal nach links und pr&uuml;ft dabei den
       Lagesensor &ndash; auch auf die Drehrichtung &ndash; und zeigt den Messwert des
       Abstandssensors. L&auml;uft in Schleife, bis du ihn stoppst. Braucht die
       Freigabe, weil Motoren anlaufen.</p>
  </div>

  <!-- ============ Experten ============ -->
  <details>
    <summary>Experten &ndash; Feinabstimmung</summary>
    <div class="inner">
      <label>Anlaufschwelle Motoren <b id="vMin">35</b> %</label>
      <input type="range" id="sMin" min="0" max="90" value="35">
      <label>Kp &ndash; Lenkst&auml;rke <b id="vKp">0.55</b></label>
      <input type="range" id="sKp" min="0" max="200" value="55">
      <label>Kd &ndash; D&auml;mpfung <b id="vKd">0.08</b></label>
      <input type="range" id="sKd" min="0" max="100" value="8">
      <label style="display:flex;align-items:center;gap:9px;margin-bottom:10px">
        <input type="checkbox" id="cGuard" checked style="width:20px;height:20px;accent-color:var(--accent)">
        <span style="color:var(--fg)">Hindernis-Stopp aktiv</span>
      </label>
      <label>anhalten unter <b id="vGuard">90</b> mm Abstand</label>
      <input type="range" id="sGuard" min="40" max="600" step="10" value="90">
      <button class="btn sm" id="bGyro" style="width:100%;margin-bottom:9px">Lagesensor nullen</button>
      <button class="btn sm" id="bCalReset" style="width:100%">Linien-Kalibrierung l&ouml;schen</button>
      <p class="note">Anlaufschwelle so weit hochdrehen, bis beide Motoren gerade
         sicher anlaufen. Kp und Kd nur im Modus &bdquo;Linie folgen&ldquo; wirksam.</p>
    </div>
  </details>

  <button class="big" id="bStop" style="background:var(--bad);color:#fff">NOTHALT</button>
</div>

<script>
"use strict";
var ws=null, armed=false, mode=0, jx=0, jy=0;
var tune={kp:0.55,kd:0.08,base:0.55,min:0.35};
var slots=[], slotSteps=[], activeSlot=0, maxSteps=48;
var prog=[], runPc=-1;
function E(id){ return document.getElementById(id) }

/* =================== Befehlsdefinitionen ===================
   Eine Stelle fuer Klartext, Farbe und Drahtformat.            */
var RADTXT=["","sehr eng","eng","eng","mittel","mittel","mittel","weit","weit","sehr weit","sehr weit"];
var LDT=["LED vorne links","LED vorne rechts","LED hinten links","LED hinten rechts",
         "beide LEDs vorne","beide LEDs hinten","alle vier LEDs"];
var LDS=["ausschalten","einschalten","blinken"];
var OPS={
  ge:{c:"drive",txt:function(s){ return "geradeaus mit "+s.a+" %" }},
  li:{c:"drive",txt:function(s){ return "Kurve links, Stufe "+s.a+" ("+RADTXT[s.a]+"), "+s.b+" %" }},
  re:{c:"drive",txt:function(s){ return "Kurve rechts, Stufe "+s.a+" ("+RADTXT[s.a]+"), "+s.b+" %" }},
  fh:{c:"drive",txt:function(s){ return "fahren bis Hindernis in "+s.c+" mm, "+s.a+" %" }},
  tl:{c:"drive",txt:function(s){ return "drehen nach links, bis "+s.c+" mm frei, "+s.a+" %" }},
  tr:{c:"drive",txt:function(s){ return "drehen nach rechts, bis "+s.c+" mm frei, "+s.a+" %" }},
  wf:{c:"wait", txt:function(s){ return "warten, bis "+s.c+" mm frei sind" }},
  dl:{c:"drive",txt:function(s){ return "drehen nach links um "+s.c+"°, "+s.a+" %" }},
  dr:{c:"drive",txt:function(s){ return "drehen nach rechts um "+s.c+"°, "+s.a+" %" }},
  st:{c:"drive",txt:function( ){ return "anhalten" }},
  wa:{c:"wait", txt:function(s){ return "warte "+(s.c/1000).toFixed(1).replace(".",",")+" Sekunden" }},
  ld:{c:"light",txt:function(s){ return LDT[s.a]+" "+LDS[s.b] }},
  bf:{c:"light",txt:function(s){ return "Blinken "+(s.a*0.5).toFixed(1).replace(".",",")+" mal pro Sekunde" }},
  lo:{c:"loop", txt:function(s){ return s.a===0 ? "von vorn wiederholen, endlos"
                                                : "von vorn wiederholen, "+s.a+" mal" }}
};
function wire(s){
  switch(s.op){
    case "ge": return "ge,"+s.a;
    case "li":
    case "re": return s.op+","+s.a+","+s.b;
    case "dl":
    case "dr":
    case "fh":
    case "tl":
    case "tr": return s.op+","+s.c+","+s.a;
    case "wf": return "wf,"+s.c;
    case "st": return "st";
    case "wa": return "wa,"+s.c;
    case "ld": return "ld,"+s.a+","+s.b;
    case "bf": return "bf,"+s.a;
    case "lo": return "lo,"+s.a;
  }
  return "";
}
function toWire(){ return prog.map(wire).join(";") }
function fromWire(t){
  var out=[];
  (t||"").split(";").forEach(function(seg){
    seg=seg.trim(); if(!seg) return;
    var p=seg.split(","), op=p[0];
    if(!OPS[op]) return;
    var s={op:op,a:0,b:0,c:0};
    if(op==="wa")                       s.c=parseInt(p[1],10)||0;
    else if(op==="wf")                  s.c=parseInt(p[1],10)||0;
    else if(op==="dl"||op==="dr"||op==="fh"||op==="tl"||op==="tr"){
      s.c=parseInt(p[1],10)||0; s.a=parseInt(p[2],10)||0;
    }
    else if(op==="ld"||op==="li"||op==="re"){
      s.a=parseInt(p[1],10)||0; s.b=parseInt(p[2],10)||0;
    }
    else if(op!=="st")                  s.a=parseInt(p[1],10)||0;
    out.push(s);
  });
  return out;
}

/* =================== Verbindung =================== */
function connect(){
  ws=new WebSocket("ws://"+location.hostname+":81/");
  ws.onopen=function(){ E("dot").classList.add("on"); E("conn").textContent="verbunden" };
  ws.onclose=function(){
    E("dot").classList.remove("on"); E("conn").textContent="getrennt";
    armed=false; paintArm(); setTimeout(connect,1000);
  };
  ws.onerror=function(){ try{ ws.close() }catch(e){} };
  ws.onmessage=function(ev){
    var d; try{ d=JSON.parse(ev.data) }catch(e){ return }
    if(d.t==="cfg")   { applyCfg(d);   return }
    if(d.t==="slots") { applySlots(d); return }
    if(d.t==="prog")  { applyProg(d);  return }
    if(d.t==="err")   { toast(d.m,false); return }
    if(d.t==="ok")    { toast(d.m,true);  return }
    paint(d);
  };
}
function send(s){ if(ws && ws.readyState===1) ws.send(s) }
function toast(m,good){
  var t=E("toast"); t.textContent=m; t.className=good?"ok":"bad";
  clearTimeout(toast._h); toast._h=setTimeout(function(){ t.textContent="" },4000);
}

/* =================== Telemetrie =================== */
var PHASE=["gesperrt","fährt","sucht Linie","Linie verloren","kalibriert…",
           "Akku leer","wartet auf Befehl","Programm beendet","Funktionstest",
           "umgekippt oder hochgehoben","Hindernis voraus"];
var TPH=["LED vorne links","LED vorne rechts","LED hinten links","LED hinten rechts",
         "alle LEDs blinken","Motor links","Motor rechts","Pause",
         "Lagesensor: Drehung rechts","Lagesensor: Drehung links","Abstandssensor"];
var VERD=["","in Ordnung","keine Reaktion – Sensor oder Antrieb",
          "Drehrichtung vertauscht","Sensor meldet sich nicht",
          "nichts im Messbereich"];
var VCLS=["","ok","bad","bad","bad","warn"];
function paint(d){
  E("bL").style.width=(d.sL*100).toFixed(0)+"%";
  E("bR").style.width=(d.sR*100).toFixed(0)+"%";
  var e=Math.max(-1,Math.min(1,d.e));
  E("bE").style.left=(50+Math.min(0,e)*50)+"%";
  E("bE").style.width=(Math.abs(e)*50)+"%";
  var mx=d.dMax||1;
  E("bML").style.width=(d.dL/mx*100).toFixed(0)+"%";
  E("bMR").style.width=(d.dR/mx*100).toFixed(0)+"%";

  if(d.a!==undefined && d.a!==(armed?1:0)){ armed=!!d.a; paintArm() }
  if(d.m!==undefined && d.m!==mode){ mode=d.m; paintMode() }

  /* Programmlauf: aktuellen Schritt hervorheben */
  var nowPc = d.pr ? (d.pc>0 ? d.pc-1 : 0) : -1;
  if(nowPc!==runPc){ runPc=nowPc; markStep() }
  E("bRun").textContent = d.pr ? "■ Programm stoppen" : "▶ Programm starten";
  E("bRun").style.background = d.pr ? "var(--bad)" : "var(--ok)";
  E("bRun").style.color      = d.pr ? "#fff" : "#06121f";
  E("prun").textContent = d.pr
    ? ("Schritt "+d.pc+" von "+d.pn+"  ·  Durchlauf "+(d.pass+1))
    : (d.p===7 ? "Programm durchgelaufen." : "");

  /* Funktionstest */
  var tb=E("bTest");
  tb.textContent = d.ts ? "■ Funktionstest stoppen" : "Funktionstest starten";
  tb.style.background = d.ts ? "var(--bad)" : "var(--warn)";
  tb.style.color      = d.ts ? "#fff" : "#06121f";
  var tx = TPH[d.tp]||"";
  if(d.tp===5||d.tp===6)      tx += "  ·  "+d.tv+" %";
  else if(d.tp===8||d.tp===9) tx += "  ·  "+(d.ti>0?"+":"")+d.ti+"°";
  else if(d.tp===10)          tx += "  ·  "+(d.ti>0 ? d.ti+" mm" : "nichts in Sicht");
  E("tphase").textContent = d.ts ? tx : "";

  /* Urteile der letzten Sensorpruefung, bleiben nach dem Test stehen */
  var tr=E("tres");
  if(d.tg||d.td){
    tr.innerHTML="";
    [["Lagesensor",d.tg],["Abstandssensor",d.td]].forEach(function(p){
      if(!p[1]) return;
      var e=document.createElement("div");
      e.className=VCLS[p[1]]||"";
      e.textContent=p[0]+": "+VERD[p[1]];
      tr.appendChild(e);
    });
  }

  /* Lagesensor */
  var il=E("imuline");
  if(d.imu){
    il.textContent = "Kurs "+(d.hd>0?"+":"")+d.hd.toFixed(0)+"°"
                   + (d.ic ? "" : "  ·  Nullpunkt fehlt")
                   + (d.dsp ? ("  ·  Abstand "+(d.ds?d.ds+" mm":"frei")) : "");
    il.className = d.ic ? "" : "warn";
  } else {
    il.textContent = "kein Lagesensor – Drehbefehle laufen zeitgesteuert"
                   + (d.dsp ? ("  ·  Abstand "+(d.ds?d.ds+" mm":"frei")) : "");
    il.className = "warn";
  }
  if(d.og!==undefined && document.activeElement!==E("sGuard")){
    E("cGuard").checked = !!d.og;
    if(+E("sGuard").value!==d.om){ E("sGuard").value=d.om; E("vGuard").textContent=d.om }
  }

  var s=E("status"), txt=PHASE[d.p]||"?";
  if(!d.calOk && d.m===1) txt+=" · nicht kalibriert";
  if(d.vb) txt+=" · "+d.vb.toFixed(2)+" V";
  txt+=" · "+d.hz+" Hz";
  s.textContent=txt;
  s.className = (d.p===3||d.p===5) ? "bad"
              : (d.p===2 || (!d.calOk && d.m===1)) ? "warn"
              : (d.p===1) ? "ok" : "";
}
function applyCfg(d){
  tune={kp:d.kp,kd:d.kd,base:d.base,min:d.min};
  E("sKp").value=Math.round(d.kp*100);     E("vKp").textContent=d.kp.toFixed(2);
  E("sKd").value=Math.round(d.kd*100);     E("vKd").textContent=d.kd.toFixed(2);
  E("sBase").value=Math.round(d.base*100); E("vBase").textContent=Math.round(d.base*100);
  E("sMin").value=Math.round(d.min*100);   E("vMin").textContent=Math.round(d.min*100);
}

/* =================== Speicherplaetze =================== */
function applySlots(d){
  slots=d.n; slotSteps=d.c; activeSlot=d.a; maxSteps=d.max;
  E("gMax").textContent=maxSteps;
  paintSlots();
  send("L,"+activeSlot);
}
function paintSlots(){
  var h=E("slots"); h.innerHTML="";
  slots.forEach(function(name,i){
    var b=document.createElement("button");
    if(i===activeSlot) b.className="sel";
    var n=slotSteps[i]||0;
    b.innerHTML="<span></span><em></em>";
    b.firstChild.textContent=name;
    b.lastChild.textContent = n===0 ? "leer" : (n+" von "+maxSteps+" Schritten");
    b.onclick=function(){ activeSlot=i; paintSlots(); send("L,"+i) };
    h.appendChild(b);
  });
}
function applyProg(d){
  activeSlot=d.slot;
  E("pname").value=d.name;
  prog=fromWire(d.p);
  slotSteps[d.slot]=prog.length;
  paintSlots(); paintSteps();
}

/* =================== Schrittliste =================== */
function paintSteps(){
  var ol=E("steps"); ol.innerHTML="";
  prog.forEach(function(s,i){
    var li=document.createElement("li");
    li.className=OPS[s.op].c;
    var v=document.createElement("var"); v.textContent=(i+1)+".";
    var p=document.createElement("p");   p.textContent=OPS[s.op].txt(s);
    var a=document.createElement("div"); a.className="acts";
    a.appendChild(mk("↑",function(){ swap(i,i-1) }));
    a.appendChild(mk("↓",function(){ swap(i,i+1) }));
    a.appendChild(mk("✕",function(){ prog.splice(i,1); paintSteps() },"del"));
    li.appendChild(v); li.appendChild(p); li.appendChild(a);
    ol.appendChild(li);
  });
  E("empty").classList.toggle("hide",prog.length>0);
  paintGauge(); markStep();
}
function mk(t,fn,cls){
  var b=document.createElement("button");
  b.textContent=t; b.onclick=fn; if(cls) b.className=cls;
  return b;
}
function swap(i,j){
  if(j<0||j>=prog.length) return;
  var t=prog[i]; prog[i]=prog[j]; prog[j]=t;
  paintSteps();
}
function markStep(){
  var ls=E("steps").children;
  for(var i=0;i<ls.length;i++) ls[i].classList.toggle("now",i===runPc);
}
function paintGauge(){
  var u=prog.length, f=maxSteps-u;
  E("gUsed").textContent=u;
  E("gFree").textContent=f;
  E("gFree").className = f===0 ? "full" : "";
  var bar=E("gBar");
  bar.style.width=(u/maxSteps*100)+"%";
  bar.className = f===0 ? "full" : "";
}

/* =================== Befehl hinzufuegen =================== */
var PANELS={ge:["pSpd"],li:["pRad","pSpd"],re:["pRad","pSpd"],st:[],
            dl:["pDeg","pSpd"],dr:["pDeg","pSpd"],fh:["pObs","pSpd"],
            tl:["pObs","pSpd"],tr:["pObs","pSpd"],wf:["pObs"],
            wa:["pWa"],ld:["pLd"],bf:["pBf"],lo:["pLo"]};
function paintAddForm(){
  var want=PANELS[E("aOp").value]||[];
  ["pRad","pDeg","pObs","pSpd","pWa","pLd","pBf","pLo"].forEach(function(id){
    E(id).classList.toggle("hide",want.indexOf(id)<0);
  });
}
E("aOp").onchange=paintAddForm;
function live(id,out,fn){ E(id).addEventListener("input",function(){ fn(this.value,E(out)) }) }
live("aSpd","vSpd",function(v,o){ o.textContent=v });
live("aDeg","vDeg",function(v,o){ o.textContent=v });
live("aObs","vObs",function(v,o){ o.textContent=v });
live("aRad","vRad",function(v,o){ o.textContent=v; E("tRad").textContent=RADTXT[v|0] });
live("aWa","vWa", function(v,o){ o.textContent=(v/10).toFixed(1).replace(".",",") });
live("aBf","vBf", function(v,o){ o.textContent=v; E("tBf").textContent=(v*0.5).toFixed(1).replace(".",",") });
live("aLo","vLo", function(v,o){ o.textContent=v; E("tLo").textContent = v==="0" ? "endlos" : v+" mal" });

E("bAdd").onclick=function(){
  if(prog.length>=maxSteps){ toast("Kein Platz mehr: "+maxSteps+" Schritte sind das Maximum.",false); return }
  var op=E("aOp").value, s={op:op,a:0,b:0,c:0};
  if(op==="ge") s.a=+E("aSpd").value;
  if(op==="li"||op==="re"){ s.a=+E("aRad").value; s.b=+E("aSpd").value }
  if(op==="dl"||op==="dr"){ s.c=+E("aDeg").value; s.a=+E("aSpd").value }
  if(op==="fh"||op==="tl"||op==="tr"){ s.c=+E("aObs").value; s.a=+E("aSpd").value }
  if(op==="wf"){ s.c=+E("aObs").value }
  if(op==="wa") s.c=Math.round(E("aWa").value*100);
  if(op==="ld"){ s.a=+E("aLdT").value; s.b=+E("aLdS").value }
  if(op==="bf") s.a=+E("aBf").value;
  if(op==="lo"){
    s.a=+E("aLo").value;
    /* "wiederhole" gibt es nur einmal und immer als letzte Zeile */
    prog=prog.filter(function(x){ return x.op!=="lo" });
    prog.push(s); paintSteps(); return;
  }
  /* alles andere vor ein vorhandenes "wiederhole" einsortieren */
  var at=prog.findIndex(function(x){ return x.op==="lo" });
  if(at<0) prog.push(s); else prog.splice(at,0,s);
  paintSteps();
};

E("bSave").onclick=function(){
  if(!prog.length){ toast("Das Programm ist leer.",false); return }
  var nm=E("pname").value.trim() || ("Programm "+(activeSlot+1));
  send("W,"+activeSlot+"|"+nm+"|"+toWire());
};
E("bReload").onclick=function(){ send("L,"+activeSlot) };
E("bClear").onclick =function(){ prog=[]; paintSteps() };
E("bRun").onclick   =function(){
  var running=E("bRun").textContent.indexOf("stoppen")>0;
  send("B,"+(running?0:1));
};

/* =================== Freigabe / Betriebsart =================== */
function paintArm(){
  var b=E("arm");
  b.textContent = armed ? "Motoren sperren" : "Freigeben";
  b.classList.toggle("armed",armed);
}
function paintMode(){
  E("m0").classList.toggle("sel",mode===0);
  E("m1").classList.toggle("sel",mode===1);
  E("m2").classList.toggle("sel",mode===2);
  E("panManual").classList.toggle("hide",mode!==0);
  E("panAuto").classList.toggle("hide",mode!==1);
  E("panProg").classList.toggle("hide",mode!==2);
}
E("arm").onclick =function(){ armed=!armed; paintArm(); send("A,"+(armed?1:0)) };
E("m0").onclick  =function(){ mode=0; paintMode(); send("M,0") };
E("m1").onclick  =function(){ mode=1; paintMode(); send("M,1") };
E("m2").onclick  =function(){ mode=2; paintMode(); send("M,2") };
E("bStop").onclick=function(){ armed=false; jx=0; jy=0; knob(0,0); paintArm(); send("X") };
E("bCal").onclick =function(){ send("C") };
E("bTest").onclick=function(){
  send("S,"+(E("bTest").textContent.indexOf("stoppen")>0?0:1));
};
E("bCalReset").onclick=function(){ send("R") };
E("bGyro").onclick    =function(){ send("G"); toast("Nullpunkt wird aufgenommen – Roboter ruhig halten.",true) };
function sendGuard(){ send("O,"+(E("cGuard").checked?1:0)+","+E("sGuard").value) }
E("cGuard").onchange=sendGuard;
E("sGuard").addEventListener("input",function(){ E("vGuard").textContent=this.value; sendGuard() });

/* =================== Feinabstimmung =================== */
function sendTune(){
  send("T,"+tune.kp.toFixed(3)+","+tune.kd.toFixed(3)+","+
       tune.base.toFixed(3)+","+tune.min.toFixed(3));
}
function slider(id,out,key,fmt){
  E(id).addEventListener("input",function(){
    var v=this.value/100; tune[key]=v;
    E(out).textContent=fmt(v);
    sendTune();
  });
}
slider("sKp","vKp","kp",function(v){ return v.toFixed(2) });
slider("sKd","vKd","kd",function(v){ return v.toFixed(2) });
slider("sBase","vBase","base",function(v){ return Math.round(v*100) });
slider("sMin","vMin","min",function(v){ return Math.round(v*100) });

/* =================== Joystick =================== */
var pad=E("pad"), active=false;
function knob(x,y){ E("knob").style.transform="translate("+(x*50)+"%,"+(-y*50)+"%)" }
function fromEvent(ev){
  var r=pad.getBoundingClientRect();
  var x=(ev.clientX-r.left)/r.width*2-1;
  var y=1-(ev.clientY-r.top)/r.height*2;
  var m=Math.sqrt(x*x+y*y);
  if(m>1){ x/=m; y/=m }            /* auf den Einheitskreis begrenzen */
  jx=x; jy=y; knob(x,y);
}
pad.addEventListener("pointerdown",function(ev){
  active=true;
  try{ pad.setPointerCapture(ev.pointerId) }catch(e){}
  fromEvent(ev); ev.preventDefault();
});
pad.addEventListener("pointermove",function(ev){
  if(active){ fromEvent(ev); ev.preventDefault() }
});
function release(ev){
  active=false; jx=0; jy=0; knob(0,0);
  if(ev && ev.preventDefault) ev.preventDefault();
}
pad.addEventListener("pointerup",release);
pad.addEventListener("pointercancel",release);
window.addEventListener("blur",function(){ release(null) });

/* Zoom-Gesten von Safari unterdruecken */
document.addEventListener("gesturestart",function(e){ e.preventDefault() });
document.addEventListener("dblclick",function(e){ e.preventDefault() });

/* =================== Sendeschleife: 10 Hz =================== */
setInterval(function(){
  if(!ws || ws.readyState!==1) return;
  if(mode===0) send("J,"+jx.toFixed(3)+","+jy.toFixed(3));
  else         send("P");
},100);

paintArm(); paintMode(); paintAddForm(); paintSteps(); connect();
</script>
</body>
</html>
)BBUI";
