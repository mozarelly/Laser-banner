// ============================================
//  Galvo Drawing Engine & Warping
// ============================================

extern const int DAC_X;
extern const int DAC_Y;
extern int brake;
extern int color_pointer;
extern int selected_vertex;
extern uint8_t Calib_CoordsX[];
extern uint8_t Calib_CoordsY[];

void print_color(int pointer);
void handleWeb();
void read_serial_key();

// Warp raw (x, y) into the calibrated 4-corner frame
// Corner 0 = V1 = Bottom-Right
// Corner 1 = V2 = Bottom-Left
// Corner 2 = V3 = Top-Left
// Corner 3 = V4 = Top-Right
void warpPoint(uint8_t rawX, uint8_t rawY, uint8_t *outX, uint8_t *outY) {
    float u = (float)rawX / 255.0f;
    float v = (float)rawY / 255.0f;

    // Bottom edge interpolation (from BL to BR)
    float bX = (1.0f - u) * Calib_CoordsX[1] + u * Calib_CoordsX[0];
    float bY = (1.0f - u) * Calib_CoordsY[1] + u * Calib_CoordsY[0];

    // Top edge interpolation (from TL to TR)
    float tX = (1.0f - u) * Calib_CoordsX[2] + u * Calib_CoordsX[3];
    float tY = (1.0f - u) * Calib_CoordsY[2] + u * Calib_CoordsY[3];

    // Vertical interpolation between bottom and top
    float targetX = (1.0f - v) * bX + v * tX;
    float targetY = (1.0f - v) * bY + v * tY;

    *outX = (uint8_t)constrain((int)round(targetX), 0, 255);
    *outY = (uint8_t)constrain((int)round(targetY), 0, 255);
}

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
        dacWrite(DAC_Y, y); // Direct mapping to galvo

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

// Draw static shapes stored in Flash (PROGMEM) automatically warped inside the calibration frame
void drawShape(const uint8_t *coordsX, const uint8_t *coordsY, const uint8_t *colorPoints, int num_samples) {
    for (int i = 0; i < num_samples; i++) {
        uint8_t rawX  = pgm_read_byte_near(coordsX + i);
        uint8_t rawY  = pgm_read_byte_near(coordsY + i);
        uint8_t color = pgm_read_byte_near(colorPoints + i);

        // Auto-warp the point into the calibrated quadrilateral
        uint8_t calX, calY;
        warpPoint(rawX, rawY, &calX, &calY);

        if (color_pointer > 0 && color > 0) {
            print_color(color_pointer);
        } else {
            print_color(color);
        }

        dacWrite(DAC_X, calX);
        dacWrite(DAC_Y, calY);

        delayMicroseconds(brake);

        handleWeb();
        read_serial_key();
    }
}
