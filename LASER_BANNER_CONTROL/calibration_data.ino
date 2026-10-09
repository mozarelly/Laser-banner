// ============================================
//  Shape & Calibration Data
// ============================================

// --- 4-Corner Calibration Square (Mutable in RAM for live adjustments) ---
const int CALIB_SAMPLES = 5;

uint8_t Calib_CoordsX[CALIB_SAMPLES] = {
    30, 225, 225, 30, 30
};

uint8_t Calib_CoordsY[CALIB_SAMPLES] = {
    30, 30, 225, 225, 30
};

uint8_t Calib_Colors[CALIB_SAMPLES] = {
    1, 1, 1, 1, 1
};

// --- Default Shape (House / Polygon) ---
const int SHAPE_SAMPLES = 7;

const uint8_t PROGMEM Shape_CoordsX[SHAPE_SAMPLES] = {
    40, 40, 120, 200, 200, 40, 40
};

const uint8_t PROGMEM Shape_CoordsY[SHAPE_SAMPLES] = {
    40, 40, 40, 40, 200, 200, 40
};

const uint8_t PROGMEM Shape_Colors[SHAPE_SAMPLES] = {
    0, 1, 1, 1, 1, 1, 1
};
