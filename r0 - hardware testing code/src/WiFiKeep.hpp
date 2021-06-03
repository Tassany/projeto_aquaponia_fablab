#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <functional>

class WiFiKeeper {
    unsigned long last;
    void (WiFiKeeper::*state)();
    std::function<bool(String)> connect;

    void to_reset() {
        last = millis();
    }
    bool to_expired(unsigned long timeout) {
        unsigned long e = millis() - last;
        return e > timeout;
    }

    void state_disconnected();
    void state_scanning();
    void state_conecting();
    void state_connected();

    public:
    WiFiKeeper(std::function<bool(String)> f) : connect(f) {
        last = millis();
        state = &WiFiKeeper::state_disconnected;
        WiFi.persistent(false);
    }
    void operator()() {
        (this->*state)();
    }
};

void WiFiKeeper::state_disconnected() {
    if (!to_expired(5000))
        return;

    to_reset();
    state = &WiFiKeeper::state_scanning;
    WiFi.scanNetworks(true);
    Serial.println("WFK: Scanning.");
}

void WiFiKeeper::state_scanning() {
    int n = WiFi.scanComplete();
    if (n >= 0) {
        Serial.print("WFK: Scan completed, found ");
        Serial.print(n);
        Serial.println(" networks...");
        for (int i=0; i<n; i++) {
            Serial.print("WFK: Checking \"");
            Serial.print(WiFi.SSID(i));
            Serial.println("\"...");
            if (connect(WiFi.SSID(i))) {
                to_reset();
                state = &WiFiKeeper::state_conecting;
                Serial.println("WFK: Connecting.");
                return;
            }
        }
        
    }

    if (!to_expired(60000))
        return;

    to_reset();
    state = &WiFiKeeper::state_disconnected;
    WiFi.disconnect();
    Serial.println("WFK: Disconnected.");
    return;
}

void WiFiKeeper::state_conecting() {
    if (WiFi.isConnected()) {
        Serial.println("WFK: Connected.");
        state = &WiFiKeeper::state_connected;
    }
    
    if (!to_expired(60000))
        return;

    to_reset();
    state = &WiFiKeeper::state_disconnected;
    WiFi.disconnect();
    Serial.println("WFK: Disconnected.");
}

void WiFiKeeper::state_connected() {
    if (WiFi.isConnected())
        return;
    
    to_reset();
    state = &WiFiKeeper::state_disconnected;
    WiFi.disconnect();
    Serial.println("WFK: Disconnected.");
}
