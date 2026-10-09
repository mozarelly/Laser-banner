# ESP32 Laser Banner Control & Keystone Calibration

Full-featured firmware for ESP32 vector RGB laser galvo projectors.
Features a standalone Wi-Fi hotspot and real-time WebSocket web dashboard, offering wireless laser control as well as live 4-corner keystone calibration with an on-screen touch D-Pad.

---

## Features

### 1. Wireless Web Dashboard (`http://192.168.4.1`)
- **Independent Wi-Fi Hotspot**: Creates network `LaserProjector` (password: `laser1234`).
- **Laser Power**: Instant **ON / OFF** toggle.
- **Operating Modes**:
  - **Shape Mode**: Draws graphics and vector shapes.
  - **Calibration Mode**: Projects the 4-corner calibration square for projector alignment.
- **Interactive Keystone Calibration**:
  - Select corner:
    - **1: Bottom-Right**
    - **2: Bottom-Left**
    - **3: Top-Left**
    - **4: Top-Right**
  - **Touch D-Pad**: Tap **▲**, **▼**, **◀**, **▶** to nudge corners directly from your smartphone.
  - **Step Size Selector**: Choose between **1**, **2**, **5**, or **10** units per tap.
  - **Live Coordinate Readout**: Displays real-time `(X, Y)` values for all 4 corners.
  - **Visual Corner Highlight**: The currently selected corner dwells longer and glows brighter on the physical wall.
- **Real-Time Sliders**: Adjust brightness (5 kHz PWM, 0–255) and point scan delay/speed.
- **Color Selection**: 7 clean colors (Red, Green, Blue, Yellow, Magenta, Cyan, White).

### 2. Dual Control: Laptop Keyboard + Phone Web App
You can calibrate using the phone web UI OR using your laptop keyboard in the Serial Monitor:
- `1`, `2`, `3`, `4`: Select corners (1:BR, 2:BL, 3:TL, 4:TR)
- `W`, `A`, `S`, `D`: Nudge active corner Up, Left, Down, Right
- `P`: Print current coordinates to console
- `Q` / `E`: Scan speed slower / faster
- `C`: Cycle color

---

## Hardware Pinout

| ESP32 Pin | Function | Notes |
|-----------|----------|-------|
| **GPIO 26** (DAC1) | X Galvo Mirror | Horizontal axis (0–255) |
| **GPIO 25** (DAC2) | Y Galvo Mirror | Vertical axis (0–255) |
| **GPIO 32** | Red Laser Modulation | Active-LOW (LEDC PWM) |
| **GPIO 33** | Green Laser Modulation | Active-LOW (LEDC PWM) |
| **GPIO 14** | Blue Laser Modulation | Active-LOW (LEDC PWM) |

*(Note: If Red and Blue are swapped on your physical wiring, switch pins 32 and 14 in `LASER_BANNER_CONTROL.ino`).*

---

## Dependencies

In Arduino IDE Library Manager (`Ctrl + Shift + I`):
- **WebSockets** by *Markus Sattler*

---

## How to Run

1. Open `LASER_BANNER_CONTROL/LASER_BANNER_CONTROL.ino` in Arduino IDE.
2. Select **ESP32 Dev Module** and the correct COM port.
3. Click **Upload**.
4. Connect your phone to Wi-Fi **`LaserProjector`** (password: `laser1234`).
5. Open your browser and go to **`http://192.168.4.1`**.
