float vazao;
int contaPulso;
bool sensorNivel;
void IRAM_ATTR incpulso(){
  contaPulso++;
}

void medirVazao(){
  contaPulso =0;
  sei();
  delay(1000);
  cli();
  vazao = contaPulso/5.5;
}

void imprimirDados(){
  Serial.print("Vazao(L/min)");
  Serial.print(vazao);
  Serial.print("-");
  Serial.print("Estado do sensor de nivel:");
  Serial.print(sensorNivel);
}

void setup(){
  Serial.begin(9600);
  pinMode(13,INPUT);
  attachInterrupt(digitalPinToInterrupt(13),incpulso,RISING);
}
void loop(){
  medirVazao();
  sensorNivel = digitalRead(15);
  imprimirDados();  
}
