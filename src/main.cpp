#include <Arduino.h>


// ── Pin Definitions ──────────────────────────────────────────────────────────
static constexpr uint8_t LED_PIN = 2;        // On-board LED (GPIO2)


// ── Constant Definitions ──────────────────────────────────────────────────────────
static constexpr uint32_t STATUS_LED_INTERVAL_MS = 500; 
static constexpr uint32_t RELAY_INTERVAL_MS = 100;
static constexpr uint8_t RELAY_PINS[] = {13, 32, 14, 27, 26, 25}; // GPIO pins connected to the relays (adjust as needed)


// ── Setup ────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);

    // Initialize the status LED pin and relay pins as outputs, and set all relays to the default state (HIGH = off for active-low relays)
    pinMode(LED_PIN, OUTPUT);
    for (uint8_t i = 0; i < sizeof(RELAY_PINS) / sizeof(RELAY_PINS[0]); i++) {
        pinMode(RELAY_PINS[i], OUTPUT);
        digitalWrite(RELAY_PINS[i], HIGH);
    }

    delay(200);
    Serial.println(F("\n\n=== ESP32 Relay Controller ==="));
}


// ── Main Loop ────────────────────────────────────────────────────────────────
static unsigned long lastStatusLEDms = 0;
static unsigned long lastRelayms = 0;
static bool isStatusLEDOn = false;
static uint8_t currentRelayIndex = 0;
static uint8_t prevRelayIndex = 0;

void loop() {

    // Non-blocking timing using millis() to manage the status LED and relay switching
    unsigned long now = millis();

    // Flash the status LED at a regular interval to indicate the program is running
    if (now - lastStatusLEDms >= STATUS_LED_INTERVAL_MS) {
        lastStatusLEDms = now;
        isStatusLEDOn = !isStatusLEDOn;
        digitalWrite(LED_PIN, isStatusLEDOn);
    }

    // Drive relays in a round-robin fashion, activating one relay at a time
    if (now - lastRelayms >= RELAY_INTERVAL_MS) {
        lastRelayms = now;

        digitalWrite(RELAY_PINS[currentRelayIndex], LOW);
        if (prevRelayIndex != currentRelayIndex) {
            digitalWrite(RELAY_PINS[prevRelayIndex], HIGH);
        }
        prevRelayIndex = currentRelayIndex;
        currentRelayIndex = (currentRelayIndex + 1) % (sizeof(RELAY_PINS) / sizeof(RELAY_PINS[0]));
    }
}
