/*
  Criar uma variável para controlar se o ar-condicinado está ligado ou desligado
  --Para assim remover o controle desta função da relé e passar para um comando

  bool control = false

  if(button == pressed){
    control = !control
  }

  while(contrle == true){
    C o d e
  }
*/


//Bibliotecas para o uso do LCD 16x2 I2C
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//Bibliotecas para o uso do sensor de temperartura 18b20
#include <OneWire.h>
#include <DallasTemperature.h>

//Definição das variáveis e suas respectivas portas responsáveis pelos relés
#define evaporador 33  //Relé 1
#define velocidade1 19  //Relé 2
#define velocidade2 18  //Relé 3
#define velocidade3 17  //Relé 4

//Definição das variáveis e suas respectivas portas responsáveis pelos botões
#define botao1_OnOff 16
#define botao2_v1 25
#define botao3_v2 26
#define botao4_v3 27

//Definição da variável e sua respectiva porta responsável pelo sensor de temperartura 18b20
#define sensorTemperatura 14 

bool liga_desliga = false;

int btsEstado1 = LOW;  //Status do botão botao1_OnOff
int btsEstado2 = LOW;  //Status do botão botao2_v1
int btsEstado3 = LOW;  //Status do botão botao3_v2
int btsEstado4 = LOW;  //Status do botão botao4_v3

int estadoRele2 = LOW;
int estadoRele3 = LOW; 
int estadoRele4 = LOW;
int estadoRele5 = LOW;

/* int auxEstadoRele1 = 0;*/
int temperaturaDesejada = 23;  // Alterar para padrão
int limiteEvaporador = 5;
int controleDiferenca = 2;
int controleLimite = 5; 

int somaDiferencaAmbiente = temperaturaDesejada + controleDiferenca;
int somaDiferencaLimite = limiteEvaporador + controleLimite;

LiquidCrystal_I2C lcd(0x27, 16, 2);

OneWire oneWire(sensorTemperatura);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  pinMode(evaporador, OUTPUT);
  pinMode(velocidade1, OUTPUT);
  pinMode(velocidade2, OUTPUT);
  pinMode(velocidade3, OUTPUT);

  pinMode(botao1_OnOff, INPUT_PULLUP);
  pinMode(botao2_v1, INPUT_PULLUP);
  pinMode(botao3_v2, INPUT_PULLUP);
  pinMode(botao4_v3, INPUT_PULLUP);

  lcd.setCursor(0,0);
  lcd.print("Evaporador:");
  lcd.setCursor(0,1);
  lcd.print("Ambiente:");

  estadoRele3 = !estadoRele3;

  // Inicializa os sensores
  sensors.begin();
}

void loop() {
  varLigaDesliga();
  
  while (liga_desliga == true)
  {
    // Lê as temperaturas dos sensores
    sensors.requestTemperatures();

    // Obtém as temperaturas lidas individualmente pelos endereços dos sensores
    DeviceAddress sensor1Address;
    DeviceAddress sensor2Address;

    sensors.getAddress(sensor1Address, 0);
    sensors.getAddress(sensor2Address, 1);

    float temp1_evaporador = sensors.getTempC(sensor1Address);
    float temp2_ambiente = sensors.getTempC(sensor2Address);
  
    // Imprime as temperaturas no monitor serial
    lcd.setCursor(11, 0);
    lcd.print(temp1_evaporador);
   
    lcd.setCursor(11, 1);
    lcd.print(temp2_ambiente);
    /*Alterar essa forma, perguntar de inicio se o condensador está operente ou não, 
    o que tira o or(||) desta função e pergunta apenas se a temperatura desejada foi atingida*/
    if(temp1_evaporador <= limiteEvaporador || temp2_ambiente <= temperaturaDesejada)  
    {
      if(estadoRele2 == HIGH){
        estadoRele2 = !estadoRele2;
      }
    }

    if (temp2_ambiente > somaDiferencaAmbiente)
    {
      if(temp1_evaporador > somaDiferencaLimite)
      {
         if(estadoRele2 == LOW){
           estadoRele2 = !estadoRele2;
        } 
      }
    }
    
     btsEstado2 = digitalRead(botao2_v1);
    if (btsEstado2 == HIGH)
    {
      if(estadoRele3 == LOW)
      {
        if (estadoRele4 == HIGH)
          {
            estadoRele4 = !estadoRele4;
          }
    
          if (estadoRele5 == HIGH)
          {
            estadoRele5 = !estadoRele5;
          }
        estadoRele3 = !estadoRele3;
        delay(200);
      }
    }
  
    btsEstado3 = digitalRead(botao3_v2);
    if (btsEstado3 == HIGH)
    {
     if(estadoRele4 == LOW)
     {
        if (estadoRele3 == HIGH)
        {
          estadoRele3 = !estadoRele3;
        }
  
        if (estadoRele5 == HIGH)
        {
          estadoRele5 = !estadoRele5;
        }
  
        estadoRele4 = !estadoRele4;
        delay(200);
      }
    }
  
    btsEstado4 = digitalRead(botao4_v3);
      if (btsEstado4 == HIGH)
      {
        if(estadoRele5 == LOW) 
        {
          if (estadoRele3 == HIGH)
          {
            estadoRele3 = !estadoRele3;
          }
  
          if (estadoRele4 == HIGH)
          {
            estadoRele4 = !estadoRele4;
          }

          estadoRele5 = !estadoRele5;
          delay(200);
        }
      }
    digitalWrite(velocidade1, estadoRele3);
    digitalWrite(velocidade2, estadoRele4);
    digitalWrite(velocidade3, estadoRele5);
    digitalWrite(evaporador, estadoRele2);

    varLigaDesliga();
  }
    digitalWrite(velocidade1, LOW);
    digitalWrite(velocidade2, LOW);
    digitalWrite(velocidade3, LOW);
    digitalWrite(evaporador, LOW);
}

void varLigaDesliga(){
  btsEstado1 = digitalRead(botao1_OnOff);
  if (btsEstado1 == HIGH)  // Se o botão de ligar for pressionado
  {
    somaDiferencaAmbiente = somaDiferencaAmbiente - controleDiferenca;
    liga_desliga = !liga_desliga;
    delay(200);
  }
}