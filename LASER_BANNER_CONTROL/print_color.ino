// ============================================
//  RGB Laser color control with PWM brightness
//  Active LOW: pin LOW = laser ON
// ============================================

extern int brightness;
extern const int red_pin;
extern const int green_pin;
extern const int blue_pin;

void print_color(int pointer) {
    uint8_t r = 255;  // OFF by default (active low: 255 = HIGH = OFF)
    uint8_t g = 255;
    uint8_t b = 255;

    // Active low: lower value = brighter
    uint8_t on = 255 - brightness;

    switch (pointer) {
        case 0: break;                          // Black (all off)
        case 1: r = on; break;                  // Red
        case 2: g = on; break;                  // Green
        case 3: b = on; break;                  // Blue
        case 4: r = on; g = on; break;          // Yellow  (R + G)
        case 5: r = on; b = on; break;          // Magenta (R + B)
        case 6: g = on; b = on; break;          // Cyan    (G + B)
        case 7: r = on; g = on; b = on; break;  // White   (R + G + B)
    }

    ledcWrite(red_pin,   r);
    ledcWrite(green_pin, g);
    ledcWrite(blue_pin,  b);
}
