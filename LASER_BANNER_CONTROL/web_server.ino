// ============================================
//  Web Server + WebSocket + HTML UI with Live Calibration
// ============================================

#include <Arduino.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

extern WebServer server;
extern WebSocketsServer ws;

extern int brightness;
extern int brake;
extern int color_pointer;
extern int laser_power;
extern int current_mode;
extern int selected_vertex;
extern int move_step;
extern uint8_t Calib_CoordsX[];
extern uint8_t Calib_CoordsY[];

void print_color(int pointer);
void nudgeVertex(int dx, int dy);
void broadcastCalibration();
void saveCalibrationToNVS();

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<title>Laser Banner Control</title>
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body { 
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; 
    background: #101221; color: #eee; 
    padding: 12px; max-width: 440px; margin: 0 auto;
  }
  h1 { text-align: center; color: #ff3b69; margin: 8px 0 16px; font-size: 21px; }
  .dot { display: inline-block; width: 10px; height: 10px; border-radius: 50%; 
         background: #555; margin-left: 8px; vertical-align: middle; }
  .dot.on { background: #00e676; box-shadow: 0 0 8px #00e676; }
  .panel { background: #191b36; border-radius: 12px; padding: 14px; margin-bottom: 12px; border: 1px solid #292d54; }
  .panel h2 { font-size: 12px; color: #8e95bf; margin-bottom: 10px; 
              text-transform: uppercase; letter-spacing: 1px; }
  
  /* Mode Toggle */
  .mode-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 8px; }
  .mbtn {
    padding: 12px; border: 2px solid #333866; border-radius: 8px;
    background: #0f1228; color: #aaa; font-size: 14px; font-weight: bold;
    cursor: pointer; text-align: center; transition: all 0.15s;
  }
  .mbtn.act { border-color: #ff3b69; color: #fff; background: #38122c; }

  /* Power Buttons */
  .power-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 8px; }
  .pbtn {
    padding: 14px; border: 2px solid #333866; border-radius: 8px;
    background: #0f1228; color: #aaa; font-size: 15px; font-weight: bold;
    cursor: pointer; text-align: center; transition: all 0.15s;
  }
  .pbtn.on-act { border-color: #00e676; color: #fff; background: #005a2b; box-shadow: 0 0 10px rgba(0,230,118,0.3); }
  .pbtn.off-act { border-color: #ff3b69; color: #fff; background: #6b0c23; box-shadow: 0 0 10px rgba(255,59,105,0.3); }

  /* Sliders */
  input[type=range] { 
    -webkit-appearance: none; width: 100%; height: 8px; 
    border-radius: 4px; background: #0f1228; outline: none; 
  }
  input[type=range]::-webkit-slider-thumb { 
    -webkit-appearance: none; width: 28px; height: 28px; 
    border-radius: 50%; background: #ff3b69; cursor: pointer; 
  }
  .val { text-align: center; font-size: 24px; color: #ff3b69; margin-top: 6px; font-weight: bold; }

  /* Colors */
  .colors { display: grid; grid-template-columns: repeat(4, 1fr); gap: 8px; }
  .cbtn { 
    height: 46px; border: 3px solid #333866; border-radius: 8px; 
    cursor: pointer; transition: border-color 0.15s, box-shadow 0.15s; 
  }
  .cbtn.act { border-color: #fff; box-shadow: 0 0 12px rgba(255,255,255,0.5); }

  /* ========================================================
     CUADRO VISUAL DE CALIBRACION CON ESQUINAS 1, 2, 3, 4
     ======================================================== */
  .calib-box-wrap {
    position: relative;
    width: 100%;
    max-width: 320px;
    height: 200px;
    margin: 8px auto 14px;
    background: #0d0f22;
    border: 2px dashed #3a3f6e;
    border-radius: 14px;
    display: flex;
    align-items: center;
    justify-content: center;
    box-shadow: inset 0 0 20px rgba(0,0,0,0.5);
  }
  .calib-inner-border {
    position: absolute;
    top: 26px; bottom: 26px; left: 26px; right: 26px;
    border: 2px solid #2d3460;
    border-radius: 8px;
    pointer-events: none;
  }
  .corner-node {
    position: absolute;
    width: 50px;
    height: 50px;
    border-radius: 50%;
    background: #161a35;
    border: 3px solid #4a548c;
    color: #fff;
    font-size: 18px;
    font-weight: 800;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    cursor: pointer;
    transition: all 0.2s ease;
    user-select: none;
    -webkit-tap-highlight-color: transparent;
    z-index: 2;
  }
  .corner-node .corner-lbl {
    font-size: 8px;
    font-weight: 700;
    color: #8c9ac4;
    margin-top: 1px;
    letter-spacing: 0.5px;
  }
  .corner-node.act {
    border-color: #00e676;
    background: #005a2b;
    color: #fff;
    box-shadow: 0 0 16px rgba(0, 230, 118, 0.7);
    transform: scale(1.12);
  }
  .corner-node.act .corner-lbl {
    color: #9effc7;
  }

  /* Posiciones de las 4 esquinas */
  .c-tl { top: 4px; left: 4px; }       /* Esquina 3: Arriba-Izquierda */
  .c-tr { top: 4px; right: 4px; }      /* Esquina 4: Arriba-Derecha */
  .c-bl { bottom: 4px; left: 4px; }    /* Esquina 2: Abajo-Izquierda */
  .c-br { bottom: 4px; right: 4px; }   /* Esquina 1: Abajo-Derecha */

  /* Botón central: Mover todo el cuadro completo */
  .center-node {
    padding: 10px 16px;
    background: #161a35;
    border: 2px solid #4a548c;
    border-radius: 10px;
    color: #a4b3d6;
    font-size: 12px;
    font-weight: 700;
    cursor: pointer;
    text-align: center;
    z-index: 2;
    transition: all 0.2s ease;
    user-select: none;
  }
  .center-node.act {
    border-color: #00d2ff;
    background: #004566;
    color: #fff;
    box-shadow: 0 0 14px rgba(0, 210, 255, 0.6);
    transform: scale(1.08);
  }

  /* Indicador del elemento en ajuste */
  .target-indicator {
    text-align: center;
    font-size: 13px;
    color: #b2bfe6;
    background: #111429;
    border-radius: 8px;
    padding: 8px;
    margin-bottom: 12px;
    border: 1px solid #232747;
  }
  .target-indicator strong { color: #00e676; }

  /* D-Pad Directional Controls */
  .dpad {
    display: grid;
    grid-template-columns: 75px 75px 75px;
    grid-template-rows: 55px 55px;
    gap: 8px;
    justify-content: center;
    margin: 8px auto 14px;
  }
  .dbtn {
    background: #252854; border: 2px solid #3d437a; border-radius: 10px;
    color: #fff; font-size: 20px; font-weight: bold; cursor: pointer;
    display: flex; align-items: center; justify-content: center;
    user-select: none; -webkit-tap-highlight-color: transparent;
    transition: 0.1s;
  }
  .dbtn:active { background: #ff3b69; border-color: #fff; transform: scale(0.95); }
  .dbtn-up { grid-column: 2; grid-row: 1; }
  .dbtn-left { grid-column: 1; grid-row: 2; }
  .dbtn-down { grid-column: 2; grid-row: 2; }
  .dbtn-right { grid-column: 3; grid-row: 2; }

  /* Step size selector */
  .step-row { display: flex; justify-content: space-around; align-items: center; margin-bottom: 12px; }
  .step-btn {
    padding: 6px 14px; background: #0f1228; border: 1px solid #333866;
    border-radius: 6px; color: #aaa; font-size: 13px; font-weight: bold; cursor: pointer;
  }
  .step-btn.act { background: #ff3b69; color: #fff; border-color: #fff; }

  /* Coordinate Readouts */
  .coords-box {
    background: #0d0f1e; border: 1px solid #282d52; border-radius: 8px;
    padding: 10px 12px; margin-top: 8px; font-family: monospace; font-size: 13px;
    line-height: 1.7; color: #889ac2;
  }
  .c-row.act {
    color: #00e676;
    font-weight: bold;
  }

  .reset-btn {
    width: 100%; margin-top: 8px; padding: 11px; background: #281224;
    border: 1px solid #6b143c; border-radius: 8px; color: #ff6b91;
    font-size: 13px; font-weight: bold; cursor: pointer;
  }
  .save-btn {
    width: 100%; margin-top: 10px; padding: 13px; background: #007a3d;
    border: 2px solid #00e676; border-radius: 8px; color: #fff;
    font-size: 14px; font-weight: bold; cursor: pointer; transition: all 0.2s;
    box-shadow: 0 0 10px rgba(0,230,118,0.25);
  }
  .save-btn:active { background: #00a854; transform: scale(0.98); }
  .save-toast {
    margin-top: 6px; text-align: center; color: #00e676; font-size: 13px; font-weight: bold;
  }
</style>
</head>
<body>

<h1>Laser Banner <span class="dot" id="status"></span></h1>

<!-- Power Switch -->
<div class="panel">
  <h2>Laser Power</h2>
  <div class="power-grid">
    <button class="pbtn on-act" id="btnOn" onclick="setP(1)">ON</button>
    <button class="pbtn" id="btnOff" onclick="setP(0)">OFF</button>
  </div>
</div>

<!-- Mode Switch -->
<div class="panel">
  <h2>Operating Mode</h2>
  <div class="mode-grid">
    <button class="mbtn act" id="mShape" onclick="setMode(0)">Shape Mode</button>
    <button class="mbtn" id="mCalib" onclick="setMode(1)">Calibration Mode</button>
  </div>
</div>

<!-- Interactive Calibration Panel -->
<div class="panel" id="calibPanel">
  <h2>Calibración de Cuadro y Esquinas</h2>
  
  <!-- Cuadro Visual con esquinas 1, 2, 3, 4 -->
  <div class="calib-box-wrap">
    <div class="calib-inner-border"></div>

    <!-- Esquina 3: Arriba-Izquierda (TL) -->
    <button class="corner-node c-tl" id="vb2" onclick="setVertex(2)" title="Esquina 3">
      3
      <span class="corner-lbl">TL</span>
    </button>

    <!-- Esquina 4: Arriba-Derecha (TR) -->
    <button class="corner-node c-tr" id="vb3" onclick="setVertex(3)" title="Esquina 4">
      4
      <span class="corner-lbl">TR</span>
    </button>

    <!-- Centro: Mover el Cuadro Completo -->
    <button class="center-node" id="vb4" onclick="setVertex(4)" title="Mover todo el cuadro">
      &#10021; MOVER TODO
    </button>

    <!-- Esquina 2: Abajo-Izquierda (BL) -->
    <button class="corner-node c-bl" id="vb1" onclick="setVertex(1)" title="Esquina 2">
      2
      <span class="corner-lbl">BL</span>
    </button>

    <!-- Esquina 1: Abajo-Derecha (BR) -->
    <button class="corner-node c-br act" id="vb0" onclick="setVertex(0)" title="Esquina 1">
      1
      <span class="corner-lbl">BR</span>
    </button>
  </div>

  <!-- Indicador de elemento seleccionado -->
  <div class="target-indicator" id="targetIndicator">
    Ajustando: <strong>Esquina 1 (Abajo-Derecha)</strong>
  </div>

  <!-- Controles Direccionales D-Pad -->
  <div class="dpad">
    <button class="dbtn dbtn-up" onclick="nudge(0, moveStep)">&#9650;</button>
    <button class="dbtn dbtn-left" onclick="nudge(moveStep, 0)">&#9664;</button>
    <button class="dbtn dbtn-down" onclick="nudge(0, -moveStep)">&#9660;</button>
    <button class="dbtn dbtn-right" onclick="nudge(-moveStep, 0)">&#9654;</button>
  </div>

  <!-- Tamaño de Paso -->
  <div class="step-row">
    <span style="font-size: 12px; color: #8e95bf;">Paso:</span>
    <button class="step-btn" onclick="setStep(1, this)">1</button>
    <button class="step-btn act" onclick="setStep(2, this)">2</button>
    <button class="step-btn" onclick="setStep(5, this)">5</button>
    <button class="step-btn" onclick="setStep(10, this)">10</button>
    <button class="step-btn" onclick="setStep(20, this)">20</button>
  </div>

  <!-- Coordenadas en Vivo -->
  <div class="coords-box" id="coordsDisplay">
    <div id="cRow0" class="c-row act">V1 (Abajo-Der): X=30, Y=30</div>
    <div id="cRow1" class="c-row">V2 (Abajo-Izq): X=225, Y=30</div>
    <div id="cRow2" class="c-row">V3 (Arriba-Izq): X=225, Y=225</div>
    <div id="cRow3" class="c-row">V4 (Arriba-Der): X=30, Y=225</div>
  </div>

  <button class="save-btn" id="btnSave" onclick="saveCalib()">💾 Guardar calibración</button>
  <div id="saveToast" class="save-toast" style="display:none;">✓ Calibración guardada en memoria flash</div>
  <button class="reset-btn" onclick="resetCalib()">↺ Regresar a calibración original</button>
</div>

<!-- Brightness Slider -->
<div class="panel">
  <h2>Brightness</h2>
  <input type="range" min="0" max="255" value="80" id="bSlider" 
         oninput="send('B:'+this.value); byId('bVal').textContent=this.value">
  <div class="val" id="bVal">80</div>
</div>

<!-- Speed Slider -->
<div class="panel">
  <h2>Speed (point delay)</h2>
  <input type="range" min="100" max="5000" step="50" value="860" id="sSlider" 
         oninput="send('S:'+this.value); byId('sVal').textContent=this.value+' \u00B5s'">
  <div class="val" id="sVal">860 &micro;s</div>
</div>

<!-- Color Selection -->
<div class="panel">
  <h2>Laser Color</h2>
  <div class="colors">
    <button class="cbtn act" data-c="1" style="background:#e00" onclick="setC(1)" title="Rojo"></button>
    <button class="cbtn" data-c="2" style="background:#0b0" onclick="setC(2)" title="Verde"></button>
    <button class="cbtn" data-c="3" style="background:#22f" onclick="setC(3)" title="Azul"></button>
    <button class="cbtn" data-c="4" style="background:#dd0" onclick="setC(4)" title="Amarillo"></button>
    <button class="cbtn" data-c="5" style="background:#d0d" onclick="setC(5)" title="Magenta"></button>
    <button class="cbtn" data-c="6" style="background:#0cc" onclick="setC(6)" title="Cyan"></button>
    <button class="cbtn" data-c="7" style="background:#ddd" onclick="setC(7)" title="Blanco"></button>
  </div>
</div>

<script>
var ws, byId = document.getElementById.bind(document);
var moveStep = 2;

function connect() {
  ws = new WebSocket('ws://' + location.hostname + ':81');
  ws.onopen  = function() { byId('status').classList.add('on'); };
  ws.onclose = function() { byId('status').classList.remove('on'); setTimeout(connect, 1500); };
  ws.onmessage = function(e) {
    var p = e.data.split(':');
    if (p[0]=='B') { byId('bSlider').value=p[1]; byId('bVal').textContent=p[1]; }
    if (p[0]=='S') { byId('sSlider').value=p[1]; byId('sVal').textContent=p[1]+' \u00B5s'; }
    if (p[0]=='C') { setCUI(p[1]); }
    if (p[0]=='P') { setPUI(p[1]); }
    if (p[0]=='M') { setModeUI(p[1]); }
    if (p[0]=='V') { setVertexUI(parseInt(p[1])); }
    if (p[0]=='CAL') {
      byId('cRow0').textContent = 'V1 (Abajo-Der): X=' + p[1] + ', Y=' + p[2];
      byId('cRow1').textContent = 'V2 (Abajo-Izq): X=' + p[3] + ', Y=' + p[4];
      byId('cRow2').textContent = 'V3 (Arriba-Izq): X=' + p[5] + ', Y=' + p[6];
      byId('cRow3').textContent = 'V4 (Arriba-Der): X=' + p[7] + ', Y=' + p[8];
    }
    if (p[0]=='SAVED') {
      var t = byId('saveToast');
      t.style.display = 'block';
      setTimeout(function() { t.style.display = 'none'; }, 2500);
    }
  };
}

function send(msg) { if (ws && ws.readyState==1) ws.send(msg); }

function setPUI(on) {
  byId('btnOn').classList.toggle('on-act', on == 1);
  byId('btnOff').classList.toggle('off-act', on == 0);
}
function setP(on) { send('P:' + on); setPUI(on); }

function setModeUI(m) {
  byId('mShape').classList.toggle('act', m == 0);
  byId('mCalib').classList.toggle('act', m == 1);
}
function setMode(m) { send('M:' + m); setModeUI(m); }

function setVertexUI(v) {
  for (var i = 0; i <= 4; i++) {
    var el = byId('vb' + i);
    if (el) el.classList.toggle('act', i == v);
  }
  for (var j = 0; j < 4; j++) {
    var row = byId('cRow' + j);
    if (row) row.classList.toggle('act', j == v);
  }
  var ind = byId('targetIndicator');
  if (ind) {
    if (v == 0) ind.innerHTML = 'Ajustando: <strong>Esquina 1 (Abajo-Derecha)</strong>';
    else if (v == 1) ind.innerHTML = 'Ajustando: <strong>Esquina 2 (Abajo-Izquierda)</strong>';
    else if (v == 2) ind.innerHTML = 'Ajustando: <strong>Esquina 3 (Arriba-Izquierda)</strong>';
    else if (v == 3) ind.innerHTML = 'Ajustando: <strong>Esquina 4 (Arriba-Derecha)</strong>';
    else if (v == 4) ind.innerHTML = 'Ajustando: <strong>Posición de Todo el Cuadro</strong>';
  }
}
function setVertex(v) { send('V:' + v); setVertexUI(v); }

function nudge(dx, dy) { send('N:' + dx + ':' + dy); }

function setStep(s, btn) {
  moveStep = s;
  document.querySelectorAll('.step-btn').forEach(b => b.classList.remove('act'));
  btn.classList.add('act');
}

function resetCalib() { send('R:1'); }

function saveCalib() { send('SAVE:1'); }

function setCUI(n) {
  document.querySelectorAll('.cbtn').forEach(function(b) { 
    b.classList.toggle('act', b.getAttribute('data-c') == n); 
  });
}
function setC(n) { send('C:' + n); setCUI(n); }

connect();
</script>
</body>
</html>
)rawliteral";

// ============================================
//  WebSocket Broadcast helper
// ============================================
void broadcastCalibration() {
    String msg = "CAL:" + String(Calib_CoordsX[0]) + ":" + String(Calib_CoordsY[0]) +
                 ":" + String(Calib_CoordsX[1]) + ":" + String(Calib_CoordsY[1]) +
                 ":" + String(Calib_CoordsX[2]) + ":" + String(Calib_CoordsY[2]) +
                 ":" + String(Calib_CoordsX[3]) + ":" + String(Calib_CoordsY[3]);
    ws.broadcastTXT(msg);
}

// ============================================
//  Nudge selected vertex or entire frame
// ============================================
void nudgeVertex(int dx, int dy) {
    if (selected_vertex >= 0 && selected_vertex < 4) {
        // Mover esquina individual
        int nx = Calib_CoordsX[selected_vertex] + dx;
        int ny = Calib_CoordsY[selected_vertex] + dy;
        Calib_CoordsX[selected_vertex] = constrain(nx, 0, 255);
        Calib_CoordsY[selected_vertex] = constrain(ny, 0, 255);
        if (selected_vertex == 0) {
            Calib_CoordsX[4] = Calib_CoordsX[0];
            Calib_CoordsY[4] = Calib_CoordsY[0];
        }
    } else if (selected_vertex == 4) {
        // Mover la posición de todo el cuadro de calibración completo
        int minX = 255, maxX = 0, minY = 255, maxY = 0;
        for (int i = 0; i < 4; i++) {
            if (Calib_CoordsX[i] < minX) minX = Calib_CoordsX[i];
            if (Calib_CoordsX[i] > maxX) maxX = Calib_CoordsX[i];
            if (Calib_CoordsY[i] < minY) minY = Calib_CoordsY[i];
            if (Calib_CoordsY[i] > maxY) maxY = Calib_CoordsY[i];
        }

        // Limitar desplazamiento dentro del rango 0-255 sin deformar
        if (minX + dx < 0) dx = -minX;
        if (maxX + dx > 255) dx = 255 - maxX;
        if (minY + dy < 0) dy = -minY;
        if (maxY + dy > 255) dy = 255 - maxY;

        for (int i = 0; i < 4; i++) {
            Calib_CoordsX[i] += dx;
            Calib_CoordsY[i] += dy;
        }
        Calib_CoordsX[4] = Calib_CoordsX[0];
        Calib_CoordsY[4] = Calib_CoordsY[0];
    }
    broadcastCalibration();
}

// ============================================
//  WebSocket Event Handler
// ============================================
void onWebSocketEvent(uint8_t clientNum, WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED:
            Serial.printf("[WS] Client %u connected\n", clientNum);
            // Send current state to new client
            ws.sendTXT(clientNum, String("B:") + brightness);
            ws.sendTXT(clientNum, String("S:") + brake);
            ws.sendTXT(clientNum, String("C:") + color_pointer);
            ws.sendTXT(clientNum, String("P:") + laser_power);
            ws.sendTXT(clientNum, String("M:") + current_mode);
            ws.sendTXT(clientNum, String("V:") + selected_vertex);
            {
                String msg = "CAL:" + String(Calib_CoordsX[0]) + ":" + String(Calib_CoordsY[0]) +
                             ":" + String(Calib_CoordsX[1]) + ":" + String(Calib_CoordsY[1]) +
                             ":" + String(Calib_CoordsX[2]) + ":" + String(Calib_CoordsY[2]) +
                             ":" + String(Calib_CoordsX[3]) + ":" + String(Calib_CoordsY[3]);
                ws.sendTXT(clientNum, msg);
            }
            break;

        case WStype_DISCONNECTED:
            Serial.printf("[WS] Client %u disconnected\n", clientNum);
            break;

        case WStype_TEXT: {
            String msg = String((char*)payload);
            int colonIdx = msg.indexOf(':');
            if (colonIdx < 0) return;

            String cmd = msg.substring(0, colonIdx);

            if (cmd == "B") {
                brightness = constrain(msg.substring(colonIdx + 1).toInt(), 0, 255);
                Serial.println("Brightness: " + String(brightness));
            }
            else if (cmd == "S") {
                brake = constrain(msg.substring(colonIdx + 1).toInt(), 100, 5000);
                Serial.println("Speed: " + String(brake));
            }
            else if (cmd == "C") {
                color_pointer = constrain(msg.substring(colonIdx + 1).toInt(), 1, 7);
                Serial.println("Color: " + String(color_pointer));
            }
            else if (cmd == "P") {
                laser_power = (msg.substring(colonIdx + 1).toInt() == 1) ? 1 : 0;
                if (laser_power == 0) print_color(0);
                Serial.println("Laser Power: " + String(laser_power ? "ON" : "OFF"));
            }
            else if (cmd == "M") {
                current_mode = constrain(msg.substring(colonIdx + 1).toInt(), 0, 1);
                Serial.println("Mode: " + String(current_mode ? "Calibration" : "Shape"));
            }
            else if (cmd == "V") {
                selected_vertex = constrain(msg.substring(colonIdx + 1).toInt(), 0, 4);
                if (selected_vertex == 4) {
                    Serial.println("Selected: ENTIRE CALIBRATION FRAME");
                } else {
                    Serial.println("Selected Vertex: " + String(selected_vertex + 1));
                }
            }
            else if (cmd == "N") {
                // Format: N:dx:dy
                int secondColon = msg.indexOf(':', colonIdx + 1);
                if (secondColon > 0) {
                    int dx = msg.substring(colonIdx + 1, secondColon).toInt();
                    int dy = msg.substring(secondColon + 1).toInt();
                    nudgeVertex(dx, dy);
                    if (selected_vertex == 4) {
                        Serial.printf("Entire Frame moved -> dx=%d, dy=%d\n", dx, dy);
                    } else {
                        Serial.println("V" + String(selected_vertex + 1) + " nudged -> (" + 
                                       String(Calib_CoordsX[selected_vertex]) + ", " + 
                                       String(Calib_CoordsY[selected_vertex]) + ")");
                    }
                }
            }
            else if (cmd == "R") {
                // Reset calibration corners to default
                Calib_CoordsX[0] = 30;  Calib_CoordsY[0] = 30;
                Calib_CoordsX[1] = 225; Calib_CoordsY[1] = 30;
                Calib_CoordsX[2] = 225; Calib_CoordsY[2] = 225;
                Calib_CoordsX[3] = 30;  Calib_CoordsY[3] = 225;
                Calib_CoordsX[4] = 30;  Calib_CoordsY[4] = 30;
                broadcastCalibration();
                Serial.println("Esquinas de regreso a calibracion inicial");
            }
            else if (cmd == "SAVE") {
                saveCalibrationToNVS();
                ws.broadcastTXT("SAVED:OK");
            }
            break;
        }
        default: break;
    }
}

// ============================================
//  Setup WiFi AP + Web Server + WebSocket
// ============================================
void setupWeb() {
    server.on("/", []() {
        server.send_P(200, "text/html", INDEX_HTML);
    });
    server.begin();
    Serial.println("HTTP server started on port 80");

    ws.begin();
    ws.onEvent(onWebSocketEvent);
    Serial.println("WebSocket server started on port 81");
}

void handleWeb() {
    server.handleClient();
    ws.loop();
}
