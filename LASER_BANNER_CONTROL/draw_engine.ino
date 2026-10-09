// ============================================
//  Galvo Drawing Engine
// ============================================

extern const int DAC_X;
extern const int DAC_Y;
extern int brake;
extern int color_pointer;
extern int selected_vertex;
void print_color(int pointer);
void handleWeb();
void read_serial_key();

// Draw mutable calibration square in RAM with selected vertex highlight
void drawCalibrationSquare(const uint8_t *coordsX, const uint8_t *coordsY, const uint8_t *colorPoints, int num_samples) {
    for (int i = 0; i < num_samples; i++) {
        uint8_t x     = coordsX[i];
        uint8_t y     = coordsY[i];
        uint8_t color = colorPoints[i];

        if (color_pointer > 0 && color > 0) {
            print_color(color_pointer);
        } else {
            print_color(color);
        }

        dacWrite(DAC_X, x);
        dacWrite(DAC_Y, 255 - y); // 255 - y ensures Y+ moves physically UP on wall

        // Dwell longer on selected vertex so it glows brighter on the wall
        if (i == selected_vertex || (i == 4 && selected_vertex == 0)) {
            delayMicroseconds(brake * 3);
        } else {
            delayMicroseconds(brake);
        }

        handleWeb();
        read_serial_key();
    }
}

// Draw static shapes stored in Flash (PROGMEM)
void drawShape(const uint8_t *coordsX, const uint8_t *coordsY, const uint8_t *colorPoints, int num_samples) {
    for (int i = 0; i < num_samples; i++) {
        uint8_t x     = pgm_read_byte_near(coordsX + i);
        uint8_t y     = pgm_read_byte_near(coordsY + i);
        uint8_t color = pgm_read_byte_near(colorPoints + i);

        if (color_pointer > 0 && color > 0) {
            print_color(color_pointer);
        } else {
            print_color(color);
        }

        dacWrite(DAC_X, x);
        dacWrite(DAC_Y, y);

        delayMicroseconds(brake);

        handleWeb();
        read_serial_key();
    }
}
