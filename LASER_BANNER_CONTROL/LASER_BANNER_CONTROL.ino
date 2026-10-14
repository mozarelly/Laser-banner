// ============================================
//  LASER BANNER CONTROL & CALIBRATION
//  ESP32 RGB Laser Projector Controller with Web App
//  WiFi AP: "LaserProjector" (pass: "laser1234") -> 192.168.4.1
// ============================================

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>  // Install: "WebSockets" by Markus Sattler
#include <Preferences.h>

// --- Preferences for NVS Flash Persistence ---
Preferences prefs;
void loadCalibrationFromNVS();
void saveCalibrationToNVS();

// --- Forward Declarations ---
void setupWeb();
void handleWeb();
void read_serial_key();
void print_color(int pointer);
void drawCalibrationSquare(const uint8_t *coordsX, const uint8_t *coordsY, const uint8_t *colorPoints, int num_samples);
void drawShape(const uint8_t *coordsX, const uint8_t *coordsY, const uint8_t *colorPoints, int num_samples);

// --- Pin Configuration ---
const int DAC_X = 26;   // DAC channel 1 -> X galvo mirror
const int DAC_Y = 25;   // DAC channel 2 -> Y galvo mirror

// Note: If Red and Blue are swapped on your physical wiring, switch 32 and 14 here
const int red_pin   = 32;
const int green_pin = 33;
const int blue_pin  = 14;

// --- Operational Globals ---
int brightness = 80;        // 0-255 (laser PWM duty)
int brake = 860;            // microseconds per point
int color_pointer = 1;      // active color (1-7)
int laser_power = 1;        // 1 = ON, 0 = OFF
int current_mode = 1;       // 0 = Normal Shape, 1 = Calibration Square
int selected_vertex = 0;    // 0=BL, 1=BR, 2=TR, 3=TL
int move_step = 2;          // points per nudge step

// --- WiFi AP Settings ---
const char* AP_SSID = "LaserProjector";
const char* AP_PASS = "laser1234";

// --- Server Instances ---
WebServer server(80);
WebSocketsServer ws(81);

// --- External Data ---
extern const int CALIB_SAMPLES;
extern uint8_t Calib_CoordsX[];
extern uint8_t Calib_CoordsY[];
extern uint8_t Calib_Colors[];

extern const int SHAPE_SAMPLES;
extern const uint8_t PROGMEM Shape_CoordsX[];
extern const uint8_t PROGMEM Shape_CoordsY[];
extern const uint8_t PROGMEM Shape_Colors[];

void loadCalibrationFromNVS() {
    prefs.begin("laser_cal", true); // Read-only mode
    if (prefs.isKey("c0x")) {
        Calib_CoordsX[0] = prefs.getUChar("c0x", 30);
        Calib_CoordsY[0] = prefs.getUChar("c0y", 30);
        Calib_CoordsX[1] = prefs.getUChar("c1x", 225);
        Calib_CoordsY[1] = prefs.getUChar("c1y", 30);
        Calib_CoordsX[2] = prefs.getUChar("c2x", 225);
        Calib_CoordsY[2] = prefs.getUChar("c2y", 225);
        Calib_CoordsX[3] = prefs.getUChar("c3x", 30);
        Calib_CoordsY[3] = prefs.getUChar("c3y", 225);
        Calib_CoordsX[4] = Calib_CoordsX[0];
        Calib_CoordsY[4] = Calib_CoordsY[0];
        Serial.println("Loaded 4-corner calibration from Flash (NVS)!");
    } else {
        Serial.println("No saved calibration found in Flash. Using default corners.");
    }
    prefs.end();
}

void saveCalibrationToNVS() {
    prefs.begin("laser_cal", false); // Read-write mode
    prefs.putUChar("c0x", Calib_CoordsX[0]);
    prefs.putUChar("c0y", Calib_CoordsY[0]);
    prefs.putUChar("c1x", Calib_CoordsX[1]);
    prefs.putUChar("c1y", Calib_CoordsY[1]);
    prefs.putUChar("c2x", Calib_CoordsX[2]);
    prefs.putUChar("c2y", Calib_CoordsY[2]);
    prefs.putUChar("c3x", Calib_CoordsX[3]);
    prefs.putUChar("c3y", Calib_CoordsY[3]);
    prefs.end();
    Serial.println(">>> Calibration permanently saved to Flash memory (NVS)! <<<");
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Laser Banner & Calibration Control ===");

    // Load saved calibration from flash immediately on boot
    loadCalibrationFromNVS();

    // --- Configure PWM on laser pins ---
    ledcAttach(red_pin,   5000, 8);
    ledcAttach(green_pin, 5000, 8);
    ledcAttach(blue_pin,  5000, 8);
    ledcWrite(red_pin,   255);   // OFF (active low)
    ledcWrite(green_pin, 255);
    ledcWrite(blue_pin,  255);

    // --- Start WiFi Access Point ---
    WiFi.softAP(AP_SSID, AP_PASS);
    Serial.print("WiFi AP started: ");
    Serial.println(AP_SSID);
    Serial.print("Open browser at: http://");
    Serial.println(WiFi.softAPIP());

    // --- Setup web server and WebSocket ---
    setupWeb();

    Serial.println("System Ready!");
}

void loop() {
    if (laser_power == 1) {
        if (current_mode == 1) {
            // Calibration Mode: 4-corner square with active-vertex highlight
            drawCalibrationSquare(Calib_CoordsX, Calib_CoordsY, Calib_Colors, CALIB_SAMPLES);
        } else {
            // Normal Mode: draw shape
            drawShape(Shape_CoordsX, Shape_CoordsY, Shape_Colors, SHAPE_SAMPLES);
        }
    } else {
        print_color(0); // Laser OFF
        handleWeb();
        read_serial_key();
    }
}
