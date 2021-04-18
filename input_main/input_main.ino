#include "AdafruitIO_WiFi.h"
#include <ESP8266WiFi.h>
#include <AdafruitIO.h>
#include <Adafruit_MQTT.h>
#include <ArduinoHttpClient.h>

/*
Versão de teste(sensor de vazão, sensor de nível, acionamento do relé, acionamento do motor, biblioteca NTPtimeESP) 
*/

//LED para verificar se os inputs funcionam
#define LED_PIN D1


#define WIFI_SSID "VIVOFIBRA-8F30"
#define WIFI_PASS "C79A4B8B90"
#define IO_USERNAME "Aquaponia"
#define IO_KEY "aio_wwwi90bZabx4u75vuERr9cAxzba7"
#define TIMEZONE  -3 // Current time zone. Used for correction of server time
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

AdafruitIO_Feed *sensor_nivel = io.feed("sensor_nivel");
AdafruitIO_Feed *rele_vazao = io.feed("rele_vazao");
AdafruitIO_Feed *modulo_rele = io.feed("modulo_rele");
AdafruitIO_Feed *feed_vazao = io.feed("vazao");


String TimeDate = ""; // Variable used to store the data received from the server
int hours;    // Current hour
int minutes;  // Current minute
int seconds;  // Current second


//Vazão
float vazao; //Variável para armazenar o valor da vazão em L/min
int contaPulso; //Variável para armazenar a quantidade de pulsos do sensor de vazão
int pinoVazao = 13; //D7
bool releVazao;
   
//Nível
bool sensorNivel;
bool sensorNivelAnterior;
int pinoNivel = 15; //D8

//Relé
int pinoRele = 12; //D6
bool rele=0;

//Motor
int pinoMotor = 14; //D5 
bool statusMotor;
bool flag_acionamento = 0;
int tempo_acionamento = 3450;

//Tempo relé
unsigned long verif_anterior_rele;
unsigned long verif_seguinte_rele;
unsigned long delay_rele = 10000;

//contagem do tempo do nivel
int contNivel = 0;

void IRAM_ATTR incpulso()
{ 
  contaPulso++; //Incrementa a variável de contagem dos pulsos
}

void setup()
{ 
  io.connect();
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);

  Serial.println();
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  
  //Setup sensor de vazão para o pino D7 (GPIO 13)
  pinMode(pinoVazao, INPUT);
  attachInterrupt(digitalPinToInterrupt(pinoVazao), incpulso, RISING); //Configura o pino para trabalhar como interrupção

  //Setup sensor de nivel
  pinMode(pinoNivel,INPUT);

  //Setup relé
  pinMode(pinoRele,OUTPUT);

  //Setup motor
  pinMode(pinoMotor,OUTPUT);

  //Setup tempo relé
  verif_anterior_rele = millis();
  verif_seguinte_rele = verif_anterior_rele + delay_rele;

  getTime();
} 


void loop ()
{
  io.run();
  getTime();

  //Acionamento do motor quando necessário
  if((seconds > 30) && (seconds < 33) )
  {
    flag_acionamento = 1;
  }
  else
  {
    flag_acionamento = 0;
  }
  
  if((flag_acionamento == 1) && (statusMotor == 0))
  {
    digitalWrite(pinoMotor,HIGH);
    statusMotor = 1;
    delay(tempo_acionamento);
  }
  else
  {
    digitalWrite(pinoMotor,LOW);
    statusMotor = 0;
  }
  

  Serial.print("StatusMotor:");
  Serial.print(statusMotor);
  Serial.print(" ");


  //Sensor de vazão
  contaPulso = 0;   //Zera a variável para contar os giros por segundos
  sei();      //Habilita interrupção
  delay (1000); //Aguarda 1 segundo
  cli();      //Desabilita interrupção
  
  vazao = contaPulso / 7.5; //Converte para L/min
  Serial.print("Vazao:");
  Serial.print(vazao);
  Serial.print(" ");
  feed_vazao->save(vazao);
  
  //Detecta a situação do sensor de nível através do pino D8 (GPIO 15)
  sensorNivel = digitalRead(pinoNivel);
  Serial.print("Sensor nivel:");
  Serial.println(sensorNivel);
  sensor_nivel->save(sensorNivel);
  

  //Verificar se é necessário desligar ou ligar o relé
  verif_anterior_rele = millis();
  if(verif_seguinte_rele < verif_anterior_rele)
  {
    if(sensorNivel == 1)
    {
      digitalWrite(pinoRele,LOW);
      rele = 0;
      modulo_rele->save(rele);
      verif_seguinte_rele = verif_anterior_rele + delay_rele;
    
    } if( sensorNivel == 0 ){
      digitalWrite(pinoRele,HIGH);
      rele = 1;
      modulo_rele->save(rele);
      if(vazao == 0 ){
        Serial.print("Vazão baixa e rele ligado");
      }
    }
  }
    if(sensorNivelAnterior == sensorNivel){
      contNivel = contNivel + 1;
    }else{
      contNivel = 0;
    }

    if(contNivel > 5){
      Serial.print("Muito tempo com o mesmo nível!");
    }
    sensorNivelAnterior=sensorNivel;


}

void getTime() {

  WiFiClient client;
  while (!!!client.connect("google.com", 80)) {
    Serial.println("connection failed, retrying...");
  }

  client.print("HEAD / HTTP/1.1\r\n\r\n");

  delay(1000); //wait a few milisseconds for incoming message

  //if there is an incoming message
  if(client.available()){
    while(client.available()){
      if (client.read() == '\n') {    
        if (client.read() == 'D') {    
          if (client.read() == 'a') {    
            if (client.read() == 't') {    
              if (client.read() == 'e') {    
                if (client.read() == ':') {    
                  client.read();
                  String theDate = client.readStringUntil('\r');
                  client.stop();

                 TimeDate = theDate.substring(7);
                  
                  // time starts at pos 14
                  String strCurrentHour = theDate.substring(17,19);
                  String strCurrentMinute = theDate.substring(20,22);
                  String strCurrentSecond = theDate.substring(23,25);

                  hours = strCurrentHour.toInt() + TIMEZONE ;
                  minutes = strCurrentMinute.toInt();
                  seconds = strCurrentSecond.toInt();
                  
                  Serial.println(hours);
                 
                  Serial.println(minutes);
                 
                  Serial.println(seconds);
                  Serial.println(" ");
 
                }
              }
            }
          }
        }
      }
    }
  }
  //if no message was received (an issue with the Wi-Fi connection, for instance)
  else{
    seconds = 0;
    minutes += 1;
    if (minutes > 59) {
      minutes = 0;
      hours += 1;
      if (hours > 11) {
        hours = 0;
      }
    }
  }
}
