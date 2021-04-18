

/*
Versão de teste(sensor de vazão, sensor de nível, acionamento do relé, acionamento do motor, biblioteca NTPtimeESP) 
*/

//Biblioteca NTPtimeESP
#include <NTPtimeESP.h>

NTPtime NTPch("br.pool.ntp.org");   // Choose server pool as required
char *ssid      = "VIVOFIBRA-8F30";               // Set you WiFi SSID
char *password  = "C79A4B8B90";               // Set you WiFi password

/*
struct strDateTime
{
  byte hour;
  byte minute;
  byte second;
  int year;
  byte month;
  byte day;
  byte dayofWeek;
  boolean valid;
};
*/

strDateTime dateTime;

//Vazão
float vazao; //Variável para armazenar o valor da vazão em L/min
int contaPulso; //Variável para armazenar a quantidade de pulsos do sensor de vazão
int pinoVazao = 13; //D7

//Nível
bool sensorNivel;
int pinoNivel = 15; //D8

//Relé
int pinoRele = 12; //D6

//Motor
int pinoMotor = 14; //D5 
bool statusMotor;
bool flag_acionamento = 0;
int tempo_acionamento = 3450;

//Tempo relé
unsigned long verif_anterior_rele;
unsigned long verif_seguinte_rele;
unsigned long delay_rele = 10000;

void IRAM_ATTR incpulso()
{ 
  contaPulso++; //Incrementa a variável de contagem dos pulsos
}

void setup()
{ 
  Serial.begin(9600); //Inicia a serial com um baud rate de 9600

  //Setup biblioteca NTPtimeESP
  Serial.println();
  Serial.println("Booted");
  Serial.println("Connecting to Wi-Fi");

  WiFi.mode(WIFI_STA);
  WiFi.begin (ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("WiFi connected");
  
  //Setup sensor de vazão para o pino D7 (GPIO 13)
  pinMode(pinoVazao, INPUT);
  attachInterrupt(digitalPinToInterrupt(pinoVazao), incpulso, RISING); //Configura o pino para trabalhar como interrupção

  //Setup sensor de nivel
  pinMode(pinoRele,INPUT);

  //Setup relé
  pinMode(pinoRele,OUTPUT);

  //Setup motor
  pinMode(pinoMotor,OUTPUT);

  //Setup tempo relé
  verif_anterior_rele = millis();
  verif_seguinte_rele = verif_anterior_rele + delay_rele;

  Serial.begin(9600); //Inicia a serial com um baud rate de 9600
} 


void loop ()
{
  //Monitorar o tempo a fim de acionar o motor
  dateTime = NTPch.getNTPtime(-3, 0);

  //Caso queira ver o horário atual
  /*
  if(dateTime.valid)
  {
    Serial.print(dateTime.hour);
    Serial.print(" - ");
    Serial.print(dateTime.minute);
    Serial.print(" - ");
    Serial.println(dateTime.second);
  }
  */

  //Acionamento do motor quando necessário
  if((dateTime.second > 30) && (dateTime.second < 33) )
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
  
  //Detecta a situação do sensor de nível através do pino D8 (GPIO 15)
  sensorNivel = digitalRead(pinoNivel);
  Serial.print("Sensor nivel:");
  Serial.println(sensorNivel);

  //Verificar se é necessário desligar ou ligar o relé
  verif_anterior_rele = millis();
  if(verif_seguinte_rele < verif_anterior_rele)
  {
    if(sensorNivel == 1)
    {
      digitalWrite(pinoRele,LOW);
      verif_seguinte_rele = verif_anterior_rele + delay_rele;
    }
    if(sensorNivel == 0)
    {
      digitalWrite(pinoRele,HIGH);
    }
  }
}
