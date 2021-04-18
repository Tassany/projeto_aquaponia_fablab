// Adafruit IO Analog Out Example
// Tutorial Link: https://learn.adafruit.com/adafruit-io-basics-analog-output
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"

/************************ Example Starts Here *******************************/

// this should correspond to a pin with PWM capability
#define LED_PIN D1
#define LED2 D2

// set up the 'analog' feed
AdafruitIO_Feed *amonia = io.feed("amonia");
AdafruitIO_Feed *ph = io.feed("ph");
AdafruitIO_Feed *nitrato = io.feed("nitrato");
AdafruitIO_Feed *nitrito = io.feed("nitrito");
AdafruitIO_Feed *oxigenio_dissolvido = io.feed("oxigenio_dissolvido");

void setup() {


  pinMode(LED_PIN, OUTPUT);
  pinMode(LED2, OUTPUT);


  // start the serial connection
  Serial.begin(115200);

  // wait for serial monitor to open
  while(! Serial);

  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  // set up a message handler for the 'amonia' feed.
  // the handleMessage function (defined below)
  // will be called whenever a message is
  // received from adafruit io.
  amonia->onMessage(messageAmonia);
  ph->onMessage(messagePH);
  nitrato->onMessage(messageNitrato);
  nitrito->onMessage(messageNitrito);
  oxigenio_dissolvido->onMessage(messageOxigenioDissolvido);

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  amonia->get();

}

void loop() {

  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();

}

// this function is called whenever an 'amonia' message
// is received from Adafruit IO. it was attached to
// the amonia feed in the setup() function above.

void messagePH(AdafruitIO_Data *data) {

  // convert the data to integer
  int reading = data->toInt();

  Serial.print("received <- ");
  Serial.println(reading);


  
  // write the current 'reading' to the led
  #if defined(ARDUINO_ARCH_ESP32)
    ledcWrite(1, reading); // ESP32 amoniaWrite()
  #else
    analogWrite(LED2, reading);
  #endif

}
void messageAmonia(AdafruitIO_Data *data) {

  // convert the data to integer
  int reading = data->toInt();

  Serial.print("received <- ");
  Serial.println(reading);
  analogWrite(LED_PIN, reading);

}
void messageNitrito(AdafruitIO_Data *data) {

  // convert the data to integer
  int reading = data->toInt();

  Serial.print("received <- ");
  Serial.println(reading);
  analogWrite(LED_PIN, reading);
}
void messageOxigenioDissolvido(AdafruitIO_Data *data) {

  // convert the data to integer
  int reading = data->toInt();

  Serial.print("received <- ");
  Serial.println(reading);
  analogWrite(LED_PIN, reading);
}
