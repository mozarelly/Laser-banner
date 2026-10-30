// ============================================
//  Serial Console Routine (Keyboard control)
// ============================================

extern int selected_vertex;
extern int move_step;
extern uint8_t Calib_CoordsX[];
extern uint8_t Calib_CoordsY[];
extern int brake;
extern int color_pointer;
extern int laser_power;
extern int current_mode;

void nudgeVertex(int dx, int dy);
void broadcastCalibration();
void print_color(int pointer);
extern void saveCalibrationToNVS();

void read_serial_key() {
    while (Serial.available() > 0) {
        char cmd = Serial.read();

        // Corner Selection: 1=TL, 2=TR, 3=BR, 4=BL, 5=All
        if (cmd == '1') {
            selected_vertex = 2; // Arriba-Izquierda (TL)
            current_mode = 1;    // switch to calibration mode
            Serial.println("Selected: Esquina 1 [Arriba-Izquierda / TL] -> (" + String(Calib_CoordsX[2]) + ", " + String(Calib_CoordsY[2]) + ")");
            broadcastCalibration();
        }
        else if (cmd == '2') {
            selected_vertex = 3; // Arriba-Derecha (TR)
            current_mode = 1;
            Serial.println("Selected: Esquina 2 [Arriba-Derecha / TR] -> (" + String(Calib_CoordsX[3]) + ", " + String(Calib_CoordsY[3]) + ")");
            broadcastCalibration();
        }
        else if (cmd == '3') {
            selected_vertex = 0; // Abajo-Derecha (BR)
            current_mode = 1;
            Serial.println("Selected: Esquina 3 [Abajo-Derecha / BR] -> (" + String(Calib_CoordsX[0]) + ", " + String(Calib_CoordsY[0]) + ")");
            broadcastCalibration();
        }
        else if (cmd == '4') {
            selected_vertex = 1; // Abajo-Izquierda (BL)
            current_mode = 1;
            Serial.println("Selected: Esquina 4 [Abajo-Izquierda / BL] -> (" + String(Calib_CoordsX[1]) + ", " + String(Calib_CoordsY[1]) + ")");
            broadcastCalibration();
        }
        else if (cmd == '5' || cmd == '0') {
            selected_vertex = 4;
            current_mode = 1;
            Serial.println("Selected: ENTIRE CALIBRATION FRAME (Move All 4 Corners)");
            broadcastCalibration();
        }

        // Nudge controls: W (Up), S (Down), A (Left), D (Right)
        else if (cmd == 'w' || cmd == 'W') {
            nudgeVertex(0, move_step);
            if (selected_vertex == 4) {
                Serial.println("ENTIRE FRAME UP -> dy=+" + String(move_step));
            } else {
                String lbl = (selected_vertex == 2) ? "1 [TL]" : (selected_vertex == 3) ? "2 [TR]" : (selected_vertex == 0) ? "3 [BR]" : "4 [BL]";
                Serial.println("Esquina " + lbl + " UP -> (" + String(Calib_CoordsX[selected_vertex]) + ", " + String(Calib_CoordsY[selected_vertex]) + ")");
            }
        }
        else if (cmd == 's' || cmd == 'S') {
            nudgeVertex(0, -move_step);
            if (selected_vertex == 4) {
                Serial.println("ENTIRE FRAME DOWN -> dy=-" + String(move_step));
            } else {
                String lbl = (selected_vertex == 2) ? "1 [TL]" : (selected_vertex == 3) ? "2 [TR]" : (selected_vertex == 0) ? "3 [BR]" : "4 [BL]";
                Serial.println("Esquina " + lbl + " DOWN -> (" + String(Calib_CoordsX[selected_vertex]) + ", " + String(Calib_CoordsY[selected_vertex]) + ")");
            }
        }
        else if (cmd == 'a' || cmd == 'A') {
            nudgeVertex(move_step, 0); // On this projector, increasing X moves physically LEFT on wall
            if (selected_vertex == 4) {
                Serial.println("ENTIRE FRAME LEFT -> dx=+" + String(move_step));
            } else {
                String lbl = (selected_vertex == 2) ? "1 [TL]" : (selected_vertex == 3) ? "2 [TR]" : (selected_vertex == 0) ? "3 [BR]" : "4 [BL]";
                Serial.println("Esquina " + lbl + " LEFT -> (" + String(Calib_CoordsX[selected_vertex]) + ", " + String(Calib_CoordsY[selected_vertex]) + ")");
            }
        }
        else if (cmd == 'd' || cmd == 'D') {
            nudgeVertex(-move_step, 0); // Decreasing X moves physically RIGHT on wall
            if (selected_vertex == 4) {
                Serial.println("ENTIRE FRAME RIGHT -> dx=-" + String(move_step));
            } else {
                String lbl = (selected_vertex == 2) ? "1 [TL]" : (selected_vertex == 3) ? "2 [TR]" : (selected_vertex == 0) ? "3 [BR]" : "4 [BL]";
                Serial.println("Esquina " + lbl + " RIGHT -> (" + String(Calib_CoordsX[selected_vertex]) + ", " + String(Calib_CoordsY[selected_vertex]) + ")");
            }
        }

        // Print Coordinates: P
        else if (cmd == 'p' || cmd == 'P') {
            Serial.println("\n--- Current Corner Coordinates ---");
            Serial.println("1 [Arriba-Izquierda / TL]: (" + String(Calib_CoordsX[2]) + ", " + String(Calib_CoordsY[2]) + ")");
            Serial.println("2 [Arriba-Derecha / TR]  : (" + String(Calib_CoordsX[3]) + ", " + String(Calib_CoordsY[3]) + ")");
            Serial.println("3 [Abajo-Derecha / BR]   : (" + String(Calib_CoordsX[0]) + ", " + String(Calib_CoordsY[0]) + ")");
            Serial.println("4 [Abajo-Izquierda / BL] : (" + String(Calib_CoordsX[1]) + ", " + String(Calib_CoordsY[1]) + ")");
            Serial.println("----------------------------------\n");
        }

        // Save Calibration: K
        else if (cmd == 'k' || cmd == 'K') {
            saveCalibrationToNVS();
            Serial.println("Saved calibration to Flash (NVS)!");
        }

        // Color: C
        else if (cmd == 'c' || cmd == 'C') {
            color_pointer++;
            if (color_pointer > 7) color_pointer = 1;
            print_color(color_pointer);
            Serial.println("Color: " + String(color_pointer));
        }

        // Speed: Q (slower) / E (faster)
        else if (cmd == 'q' || cmd == 'Q') {
            brake = constrain(brake + 100, 100, 5000);
            Serial.println("Brake: " + String(brake));
        }
        else if (cmd == 'e' || cmd == 'E') {
            brake = constrain(brake - 100, 100, 5000);
            Serial.println("Brake: " + String(brake));
        }
    }
}
