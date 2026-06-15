#include <Arduino.h>
#include <WiFi.h>
#include "../.credentials/wifi_credentials.h"
#include <ArduinoOTA.h>
#include "DuaLogger.h"

static DuaLogger logger;

// ── Pin Definitions ──────────────────────────────────────────────────────────
static constexpr uint8_t LED_PIN = 2;        // On-board LED (GPIO2)


// ── Constant Definitions ──────────────────────────────────────────────────────────
static constexpr uint32_t STATUS_LED_INTERVAL_MS = 500;
static constexpr uint32_t RELAY_INTERVAL_MS = 1000;
// Safe output GPIOs on the DOIT ESP32 DevKit V1 (38-pin).
// Avoided: 0 (boot), 1/3 (UART0), 2 (LED), 6-11 (flash SPI), 12 (flash voltage strapping),
//          34/35/36/39 (input-only), 5 (strapping — goes LOW at boot, would briefly pulse relay).
// GPIO 15 is a strapping pin but must be HIGH at boot, matching the active-low relay OFF state.
static constexpr uint8_t RELAY_PINS[] = {4, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};


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

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        logger.println(F("Connection Failed! Rebooting..."));
        delay(5000);
        ESP.restart();
    }


    // Configure ArduinoOTA
    ArduinoOTA.setHostname("esp32_relayboard"); // Friendly network name

    ArduinoOTA.onStart([]() {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        logger.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        logger.println(F("\nEnd"));
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        logger.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        logger.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)         logger.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)   logger.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) logger.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) logger.println("Receive Failed");
        else if (error == OTA_END_ERROR)     logger.println("End Failed");
    });

    ArduinoOTA.begin();
    logger.begin();

    logger.println("\n\nOTA Ready");
    logger.print("IP address: ");
    logger.println(WiFi.localIP());
    logger.println(F("\n\n=== ESP32 Relay Controller ==="));
}


// ── Main Loop ────────────────────────────────────────────────────────────────
static unsigned long lastStatusLEDms = 0;
static unsigned long lastRelayms = 0;
static bool isStatusLEDOn = false;
static uint8_t currentRelayIndex = 0;
static uint8_t prevRelayIndex = 0;

void loop() {
    ArduinoOTA.handle();

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
        logger.printf("Activated relay %i on GPIO %i\n", (currentRelayIndex + 1), RELAY_PINS[currentRelayIndex]);
        if (prevRelayIndex != currentRelayIndex) {
            digitalWrite(RELAY_PINS[prevRelayIndex], HIGH);
        }
        prevRelayIndex = currentRelayIndex;
        currentRelayIndex = (currentRelayIndex + 1) % (sizeof(RELAY_PINS) / sizeof(RELAY_PINS[0]));
    }
}
