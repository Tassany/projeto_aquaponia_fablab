#include <Arduino.h>
#include <SPI.h>
#include <aqua_pins.hpp>

uint8_t aqua_inputs() {
    digitalWrite(AQ_PIN_PL, true);
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE2));
    uint8_t data = SPI.transfer(0);
    SPI.endTransaction();
    digitalWrite(AQ_PIN_PL, false);
    return data;
}
