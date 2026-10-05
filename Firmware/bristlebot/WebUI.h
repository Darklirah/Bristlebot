// =====================================================================
//  Bristlebot -- WebUI.h
//  Die komplette Bedienoberflaeche als eine Datei im Flash.
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
  --bg:#11141a; --panel:#1b2029; --panel2:#232a35; --line:#2e3746;
  --fg:#e8edf5; --dim:#8c97a8;
  --accent:#4da3ff; --ok:#39d98a; --warn:#ffc24b; --bad:#ff5c6c;
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
.row{display:flex;gap:10px;align-items:center}
.seg{display:flex;background:var(--panel2);border-radius:11px;padding:3px;gap:3px;flex:1}
.seg button{flex:1;border:0;background:transparent;color:var(--dim);font:inherit;font-weight:600;
            padding:10px 6px;border-radius:9px}
.seg button.sel{background:var(--accent);color:#06121f}
.big{width:100%;border:0;border-radius:13px;font:inherit;font-weight:700;font-size:16px;
     padding:16px;color:#06121f;background:var(--ok)}
.big.armed{background:var(--bad);color:#fff}
.btn{border:1px solid var(--line);background:var(--panel2);color:var(--fg);font:inherit;
     padding:11px 14px;border-radius:11px;font-weight:600}
.btn:active{background:#2c3543}
#pad{position:relative;width:100%;aspect-ratio:1/1;max-height:46vh;margin:0 auto;
     background:var(--panel2);
     border:1px solid var(--line);border-radius:22px;touch-action:none;overflow:hidden}
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
.bars{display:grid;grid-template-columns:auto 1fr;gap:5px 9px;align-items:center;font-size:12px;color:var(--dim)}
.bar{height:9px;background:var(--panel2);border-radius:5px;overflow:hidden;position:relative}
.bar i{display:block;height:100%;width:0;background:var(--accent);transition:width .1s linear}
.bar.err i{position:absolute;left:50%;background:var(--warn)}
#status{font-size:13px;color:var(--dim);min-height:18px}
#status.bad{color:var(--bad)} #status.warn{color:var(--warn)} #status.ok{color:var(--ok)}
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
    <div class="seg">
      <button id="mManual" class="sel">Manuell</button>
      <button id="mAuto">Autonom</button>
    </div>
  </div>

  <div class="card" id="panManual">
    <div id="pad"><div id="cross"></div><div id="knob"></div>
      <div id="padhint">hoch = schneller &middot; seitlich = lenken</div>
    </div>
    <p class="note">Rueckwaerts gibt es bauartbedingt nicht: ein Vibrationsmotor
       treibt den Roboter nur in Borstenrichtung. Joystick nach unten = Halt.</p>
  </div>

  <div class="card hide" id="panAuto">
    <label>Grundvibration <b id="vBase">55</b> %</label>
    <input type="range" id="sBase" min="0" max="100" value="55">
    <label>Kp &ndash; Lenkstaerke <b id="vKp">0.55</b></label>
    <input type="range" id="sKp" min="0" max="200" value="55">
    <label>Kd &ndash; Daempfung <b id="vKd">0.08</b></label>
    <input type="range" id="sKd" min="0" max="100" value="8">
    <div class="row">
      <button class="btn" id="bCal" style="flex:1">Linie kalibrieren</button>
      <button class="btn" id="bCalReset">Reset</button>
    </div>
    <p class="note">Kalibrieren: Knopf druecken und den Roboter 5 s langsam
       quer ueber die Linie hin und her schwenken, so dass beide Sensoren
       einmal die Linie und einmal den hellen Untergrund sehen.</p>
  </div>

  <div class="card">
    <label>Anlaufschwelle Motoren <b id="vMin">35</b> %</label>
    <input type="range" id="sMin" min="0" max="90" value="35">
    <p class="note">So weit hochdrehen, bis beide Motoren gerade sicher anlaufen.</p>
  </div>

  <div class="card">
    <div class="bars">
      <span>Sensor L</span><div class="bar"><i id="bL"></i></div>
      <span>Sensor R</span><div class="bar"><i id="bR"></i></div>
      <span>Ablage</span><div class="bar err"><i id="bE"></i></div>
      <span>Motor L</span><div class="bar"><i id="bML" style="background:var(--ok)"></i></div>
      <span>Motor R</span><div class="bar"><i id="bMR" style="background:var(--ok)"></i></div>
    </div>
    <div id="status" style="margin-top:10px">&mdash;</div>
  </div>

  <button class="big" id="bStop" style="background:var(--bad);color:#fff">NOTHALT</button>
</div>

<script>
"use strict";
var ws=null, armed=false, mode=0, jx=0, jy=0;
var tune={kp:0.55,kd:0.08,base:0.55,min:0.35};
function E(id){ return document.getElementById(id) }

/* ---------- Verbindung ---------- */
function connect(){
  ws=new WebSocket("ws://"+location.hostname+":81/");
  ws.onopen=function(){ E("dot").classList.add("on"); E("conn").textContent="verbunden"; };
  ws.onclose=function(){
    E("dot").classList.remove("on"); E("conn").textContent="getrennt";
    armed=false; paintArm(); setTimeout(connect,1000);
  };
  ws.onerror=function(){ try{ ws.close() }catch(e){} };
  ws.onmessage=function(ev){
    var d; try{ d=JSON.parse(ev.data) }catch(e){ return }
    if(d.t==="cfg"){ applyCfg(d); return }
    paint(d);
  };
}
function send(s){ if(ws && ws.readyState===1) ws.send(s) }

/* ---------- Telemetrie anzeigen ---------- */
var PHASE=["gesperrt","faehrt","sucht Linie","Linie verloren","kalibriert…",
           "Akku leer","wartet auf Befehl"];
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

/* ---------- Freigabe / Betriebsart ---------- */
function paintArm(){
  var b=E("arm");
  b.textContent = armed ? "Motoren sperren" : "Freigeben";
  b.classList.toggle("armed",armed);
}
function paintMode(){
  E("mManual").classList.toggle("sel",mode===0);
  E("mAuto").classList.toggle("sel",mode===1);
  E("panManual").classList.toggle("hide",mode!==0);
  E("panAuto").classList.toggle("hide",mode!==1);
}
E("arm").onclick     =function(){ armed=!armed; paintArm(); send("A,"+(armed?1:0)) };
E("mManual").onclick =function(){ mode=0; paintMode(); send("M,0") };
E("mAuto").onclick   =function(){ mode=1; paintMode(); send("M,1") };
E("bStop").onclick   =function(){ armed=false; jx=0; jy=0; knob(0,0); paintArm(); send("X") };
E("bCal").onclick    =function(){ send("C") };
E("bCalReset").onclick=function(){ send("R") };

/* ---------- Schieberegler ---------- */
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

/* ---------- Joystick ---------- */
var pad=E("pad"), active=false;
function knob(x,y){
  E("knob").style.transform="translate("+(x*50)+"%,"+(-y*50)+"%)";
}
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

/* ---------- Sendeschleife: 10 Hz, haelt auch den Totmannschalter wach ---------- */
setInterval(function(){
  if(!ws || ws.readyState!==1) return;
  if(mode===0) send("J,"+jx.toFixed(3)+","+jy.toFixed(3));
  else         send("P");
},100);

paintArm(); paintMode(); connect();
</script>
</body>
</html>
)BBUI";
