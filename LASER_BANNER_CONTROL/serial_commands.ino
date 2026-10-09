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

void read_serial_key() {
    while (Serial.available() > 0) {
        char cmd = Serial.read();

        // Corner Selection: 1, 2, 3, 4
        if (cmd == '1') {
            selected_vertex = 0;
            current_mode = 1; // switch to calibration mode
            Serial.println("Selected: Vertex 1 [Bottom-Right] -> (" + String(Calib_CoordsX[0]) + ", " + String(Calib_CoordsY[0]) + ")");
            broadcastCalibration();
        }
        else if (cmd == '2') {
            selected_vertex = 1;
            current_mode = 1;
            Serial.println("Selected: Vertex 2 [Bottom-Left] -> (" + String(Calib_CoordsX[1]) + ", " + String(Calib_CoordsY[1]) + ")");
            broadcastCalibration();
        }
        else if (cmd == '3') {
            selected_vertex = 2;
            current_mode = 1;
            Serial.println("Selected: Vertex 3 [Top-Left] -> (" + String(Calib_CoordsX[2]) + ", " + String(Calib_CoordsY[2]) + ")");
            broadcastCalibration();
        }
        else if (cmd == '4') {
            selected_vertex = 3;
            current_mode = 1;
            Serial.println("Selected: Vertex 4 [Top-Right] -> (" + String(Calib_CoordsX[3]) + ", " + String(Calib_CoordsY[3]) + ")");
            broadcastCalibration();
        }

        // Nudge controls: W (Up), S (Down), A (Left), D (Right)
        else if (cmd == 'w' || cmd == 'W') {
            nudgeVertex(0, move_step);
            Serial.println("V" + String(selected_vertex + 1) + " UP -> (" + String(Calib_CoordsX[selected_vertex]) + ", " + String(Calib_CoordsY[selected_vertex]) + ")");
        }
        else if (cmd == 's' || cmd == 'S') {
            nudgeVertex(0, -move_step);
            Serial.println("V" + String(selected_vertex + 1) + " DOWN -> (" + String(Calib_CoordsX[selected_vertex]) + ", " + String(Calib_CoordsY[selected_vertex]) + ")");
        }
        else if (cmd == 'a' || cmd == 'A') {
            nudgeVertex(move_step, 0); // On this projector, increasing X moves physically LEFT on wall
            Serial.println("V" + String(selected_vertex + 1) + " LEFT -> (" + String(Calib_CoordsX[selected_vertex]) + ", " + String(Calib_CoordsY[selected_vertex]) + ")");
        }
        else if (cmd == 'd' || cmd == 'D') {
            nudgeVertex(-move_step, 0); // Decreasing X moves physically RIGHT on wall
            Serial.println("V" + String(selected_vertex + 1) + " RIGHT -> (" + String(Calib_CoordsX[selected_vertex]) + ", " + String(Calib_CoordsY[selected_vertex]) + ")");
        }

        // Print Coordinates: P
        else if (cmd == 'p' || cmd == 'P') {
            Serial.println("\n--- Current Vertex Coordinates ---");
            Serial.println("V1 (Bottom-Right): (" + String(Calib_CoordsX[0]) + ", " + String(Calib_CoordsY[0]) + ")");
            Serial.println("V2 (Bottom-Left) : (" + String(Calib_CoordsX[1]) + ", " + String(Calib_CoordsY[1]) + ")");
            Serial.println("V3 (Top-Left)    : (" + String(Calib_CoordsX[2]) + ", " + String(Calib_CoordsY[2]) + ")");
            Serial.println("V4 (Top-Right)   : (" + String(Calib_CoordsX[3]) + ", " + String(Calib_CoordsY[3]) + ")");
            Serial.println("----------------------------------\n");
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
