#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiKeep.hpp>
#include <Timeout.h>
#include <aqua_pins.hpp>
#include <aqua_inputs.hpp>
#include <AdafruitIO.h>
#include <Adafruit_MQTT.h>
#include <ArduinoHttpClient.h>
#include <AdafruitIO_WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>


//credênciais para acessar o adafruit
// #define WIFI_SSID "VIVOFIBRA-8F30"
// #define WIFI_PASS "C79A4B8B90"
#define IO_USERNAME "Aquaponia"
#define IO_KEY "aio_wqgf02BSXpiBHQQA5VIpx5dzbh30"



//tempo
const long utcOffsetInSeconds = -10800;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

//vazão
float sensorVazao;
int contaPulso; //Variável para armazenar a quantidade de pulsos do sensor de vazão
void IRAM_ATTR incpulso()
{ 
  contaPulso++; //Incrementa a variável de contagem dos pulsos
}
//motor
bool statusMotor;
bool flag_acionamento = 0;

//Nível
bool sensorNivel;


const struct {
    const char *ssid;
    const char *password;
} networks[] = {
    { "DELL_FABLAB", "computadores" }
};



AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, "VIVOFIBRA-8F30", "C79A4B8B90");



//Nível
//int sensorNivel;
AdafruitIO_Feed *sensor_nivel = io.feed("sensor_nivel");
AdafruitIO_Feed *feed_vazao = io.feed("vazao");

WiFiKeeper wfk([](String ssid){
    for (auto net : networks) {
        if (ssid == net.ssid) {
            WiFi.begin(net.ssid, net.password);
            return true;
        }
    }

    return false;
});

Timeout to_kb;
Timeout to_flow;
bool oldFlow;
Every<1000> do_print;
Every<60000> do_publish;
unsigned ctr_flow = 0;

void setup() {

    
    Serial.begin(115200);
    Serial.println("\n\nBoot complete.");

    Serial.print("Connecting to Adafruit IO");
    io.connect();//tentando conectar ao adafruit 

    while(io.status() < AIO_CONNECTED) {
        Serial.print(".");//enquanto não conectar printa um "."
        delay(500);
    }
    // Se saiu do loop, conectou 
    Serial.println();
    Serial.println(io.statusText());
  

    pinMode(AQ_PIN_BUTTON, INPUT_PULLUP);

    pinMode(AQ_PIN_PUMP0, OUTPUT);
    digitalWrite(AQ_PIN_PUMP0, 0);

    pinMode(AQ_PIN_PUMP1, OUTPUT);
    digitalWrite(AQ_PIN_PUMP1, 0);

    pinMode(AQ_PIN_FEEDER, OUTPUT);
    digitalWrite(AQ_PIN_FEEDER, 0);

    pinMode(AQ_PIN_PL, OUTPUT);
    digitalWrite(AQ_PIN_PL, 0);

    //attachInterrupt((AQ_X6), incpulso, RISING); //Configura o pino para trabalhar como interrupção
    SPI.begin();
    timeClient.begin();
    
    
}

void loop() {
    io.run();
    timeClient.update();
    //wfk();

    uint8_t inputs = aqua_inputs();

    if (!digitalRead(AQ_PIN_BUTTON) && to_kb.expired()) {
        to_kb.start(10000000);
        WiFi.beginSmartConfig();
        Serial.println("SmartConfig");
    }

    digitalWrite(AQ_PIN_PUMP0, ~inputs & AQ_X0);
    digitalWrite(AQ_PIN_PUMP1, ~inputs & AQ_X1);
    digitalWrite(AQ_PIN_FEEDER, ~inputs & AQ_X2);

    

    //Acionamento do motor quando necessário
    


    if(timeClient.getHours()==7 || timeClient.getHours() == 17)
    {
        digitalWrite(AQ_PIN_PUMP0, HIGH);
    }
    else
    {
        digitalWrite(AQ_PIN_PUMP0, LOW);

    }

    // if((timeClient.getSeconds() > 30) && (timeClient.getSeconds() < 40) )
    // {
    //     digitalWrite(AQ_PIN_PUMP0, HIGH);

    // }
    // else
    // {
    //     digitalWrite(AQ_PIN_PUMP0, LOW);
    // }
    sensorNivel = inputs & AQ_X7;
    //sensorVazao = inputs & AQ_X6;

    //verifica se a vazão atual é igual a anterior, se for espera 
    // if(sensorVazao == oldFlow){
    //     to_flow.start(20);
    // }else if(to_flow.expired()){
    //     ctr_flow++;
    //     oldFlow = sensorVazao;
    //     to_flow.start(20);
    // }

    // if(do_publish){
    //     static unsigned oldCtrFLow = 0;
    //     feed_vazao->save((ctr_flow -oldCtrFLow )/7.5);
    //     oldCtrFLow = ctr_flow;
    //     sensor_nivel->save(sensorNivel);
    // }

digitalWrite(AQ_PIN_PUMP1, !sensorNivel);
    if(do_print){
        Serial.println(sensorNivel);
        Serial.println(sensorVazao);
        Serial.print(timeClient.getHours());
        Serial.print(":");
        Serial.print(timeClient.getMinutes());
        Serial.print(":");
        Serial.println(timeClient.getSeconds());
    }


}
