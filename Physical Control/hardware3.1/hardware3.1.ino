/* 
  Criar um segundo código relacionado ao modo calor
    Este código foi feito para gelar, logo ele só desarma compressor quando 
    estiver menor ou igual a temperatura desejada
      Já no calor ele vai ter que ser maior ou igual a temperatura desejada
        Criar um código junto
    Temperatura padrão frio = 23 // temperatura padrão quente = 25


        V.4
          Adicionar a plataforma web para poder fazer as alterações dos valores
            Fazer a interação entre o código e o html por meio do /get

*/


//Bibliotecas para o uso do LCD 16x2 I2C
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//Bibliotecas para o uso do sensor de temperartura 18b20
#include <OneWire.h>
#include <DallasTemperature.h>

//Definição das variáveis e suas respectivas portas responsáveis pelos relés
#define evaporador 23   //Relé 1
#define velocidade1 19  //Relé 2
#define velocidade2 18  //Relé 3
#define velocidade3 17  //Relé 4

//Definição das variáveis e suas respectivas portas responsáveis pelos push-buttons
#define botao0_OnQuente 32
#define botao1_OnFrio 16
#define botao2_v1 25
#define botao3_v2 26
#define botao4_v3 27

//Definição da variável e sua respectiva porta responsável pelo sensor de temperartura 18b20
#define sensorTemperatura 14 

bool quente = false;
bool frio = false;
bool ligado = false;

int btsEstado0 = LOW;  //Status do botão botao0_OnQuente
int btsEstado1 = LOW;  //Status do botão botao1_OnFrio
int btsEstado2 = LOW;  //Status do botão botao2_v1
int btsEstado3 = LOW;  //Status do botão botao3_v2
int btsEstado4 = LOW;  //Status do botão botao4_v3

int estadoRele1 = LOW;
int estadoRele2 = LOW;
int estadoRele3 = LOW; 
int estadoRele4 = LOW;

int temperaturaDesejadaFrio = 23;  
int temperaturaDesejadaQuente = 25;
int limiteEvaporador = 5;
int controleDiferenca = 2;
int controleLimite = 5; 

int somaDiferencaAmbienteFrio = temperaturaDesejadaFrio + controleDiferenca;
int somaDiferencaLimite = limiteEvaporador + controleLimite;

// Variáveis de controle
int veloPadrao = 0;
int controleLcd = 0;
int controleLcd1 = 0;
int contador = 0;
int esperaInicio = 0;
bool auxiliarDeInversao = false;

int guardaBotaoApertado = 0; 
/*
a variável guardaBotaoApertado é uma variável que serve para que aconteça inversão
de estados sem ocorrer o desligamento do sistema de forma indesejada

0 --> Neutro
1 --> O Botão anteriormente pressionado foi o botão liga_quente
2 --> O Botão anteriormente pressionado foi o botão liga_frio

*/

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

  pinMode(botao0_OnQuente, INPUT_PULLUP);
  pinMode(botao1_OnFrio, INPUT_PULLUP);
  pinMode(botao2_v1, INPUT_PULLUP);
  pinMode(botao3_v2, INPUT_PULLUP);
  pinMode(botao4_v3, INPUT_PULLUP);

  // Inicializa os sensores
  sensors.begin();
}

void loop() {
  
    if (esperaInicio == 0)  // Se o arCondicionado foi ligado pela primeira vez
    {
        lcd.print("Aguarde 5s...");
        delay(5000);
        lcd.clear();
        esperaInicio = 1;
    }

    if(controleLcd == 0){
        lcd.clear();
        lcd.print("Desligado");
        controleLcd = 1;
        /*int*/ controleLcd1 = 0;
    }

    

    while (ligado == true)  // PARTE GERAL DO CÓDIGO
    {
        if(veloPadrao == 0){
            estadoRele2 = !estadoRele2;  //Define a velocidade 1 como padrão
            veloPadrao = 1;
        }
      
        if(controleLcd1 == 0){
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Evaporador:");
            lcd.setCursor(0,1);
            lcd.print("Ambiente:");
            controleLcd1 = 1;
        }
      
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

          btsEstado2 = digitalRead(botao2_v1);
        if (btsEstado2 == HIGH)
        {
            if(estadoRele2 == LOW)
            {
                if (estadoRele3 == HIGH)
                {
                    estadoRele3 = !estadoRele3;
                }
            
                if (estadoRele4 == HIGH)
                {
                    estadoRele4 = !estadoRele4;
                }
                estadoRele2 = !estadoRele2;
                //delay(200);
            }
        }
    
          btsEstado3 = digitalRead(botao3_v2);
        if (btsEstado3 == HIGH)
        {
            if(estadoRele3 == LOW)
            {
                if (estadoRele2 == HIGH)
                {
                estadoRele2 = !estadoRele2;
                }
        
                if (estadoRele4 == HIGH)
                {
                estadoRele4 = !estadoRele4;
                }
        
                estadoRele3 = !estadoRele3;
                //delay(200);
            }
        }
    
          btsEstado4 = digitalRead(botao4_v3);
        if (btsEstado4 == HIGH)
        {
            if(estadoRele4 == LOW) 
            {
                if (estadoRele2 == HIGH)
                {
                    estadoRele2 = !estadoRele2;
                }
        
                if (estadoRele3 == HIGH)
                {
                    estadoRele3 = !estadoRele3;
                }

                estadoRele4 = !estadoRele4;
                //delay(200);
            }
        }

        /*if (frio == true)
        {
            if(temp1_evaporador <= limiteEvaporador || temp2_ambiente <= temperaturaDesejadaFrio)  
            {
                if(estadoRele2 == HIGH){
                    estadoRele2 = !estadoRele2;
                }
            }

            if (temp2_ambiente > somaDiferencaAmbienteFrio)
            {
                if(temp1_evaporador > somaDiferencaLimite)
                {
                    if(estadoRele2 == LOW){
                        estadoRele2 = !estadoRele2;
                    } 
                }
            }
        }*/

        /*if (quente == true)
        {
            if(temp1_evaporador <= limiteEvaporador || temp2_ambiente >= temperaturaDesejadaQuente)  
            {
                if(estadoRele2 == HIGH){
                    estadoRele2 = !estadoRele2;
                }
            }

            if (temp2_ambiente > somaDiferencaAmbienteFrio)
            {
                if(temp1_evaporador > somaDiferencaLimite)
                {
                    if(estadoRele2 == LOW){
                        estadoRele2 = !estadoRele2;
                    } 
                }
            }
        }*/
        
        digitalWrite(evaporador, estadoRele1);
        digitalWrite(velocidade1, estadoRele2);
        digitalWrite(velocidade2, estadoRele3);
        digitalWrite(velocidade3, estadoRele4);

        varLigaDesliga();
        inversao();     
    }

    digitalWrite(velocidade1, LOW);
    digitalWrite(velocidade2, LOW);
    digitalWrite(velocidade3, LOW);
    digitalWrite(evaporador, LOW);

    varLigaDesliga();
    inversao();
}

void varLigaDesliga(){
  btsEstado0 = digitalRead(botao0_OnQuente);
  if (btsEstado0 == HIGH)  // Se o botão de ligar for pressionado
  {
    if (guardaBotaoApertado == 1)
    {
      if(ligado == true){
        auxiliarDeInversao = true;
      }
        ligado = !ligado;
        quente = false;
    }
    else{
      if(guardaBotaoApertado == 0){
        ligado = !ligado;
        quente = true;
      }
      else{
        if(ligado == true){
          auxiliarDeInversao = true;
        }
        quente = true;
      }
    }

    guardaBotaoApertado = 1;  //Define que o botão liga_quente foi pressionado
    frio = false;  //Desliga o compressor no estado frio*/
    /*somaDiferencaAmbienteQuente = somaDiferencaAmbienteQuente - controleDiferenca;*/
    //delay(200);
  }

  btsEstado1 = digitalRead(botao1_OnFrio);
  if (btsEstado1 == HIGH)  // Se o botão de ligar for pressionado
  {
    if (guardaBotaoApertado == 2)
    {
      if(ligado == true){
        auxiliarDeInversao = true;
      }
      
        ligado = !ligado;
        frio = false;
    }
    else{
      if(guardaBotaoApertado == 0){
        ligado = !ligado;
        frio = true;
      }
      else{
        if(ligado == true){
           auxiliarDeInversao = true;
        }
        frio = true;
      }
    }

    guardaBotaoApertado = 2;  //Define que o botão liga_frio foi pressionado
    quente = false;  //Desliga ou mantém Ligado o estado quente
    /*somaDiferencaAmbienteFrio = somaDiferencaAmbienteFrio - controleDiferenca;*/
    //delay(200);
  }
}
void inversao(){
    if (auxiliarDeInversao == true)
    {
        unsigned long tempoEsperaInversao = 10000; // 1 minuto
        unsigned long tempoInicialInversao = millis();
        unsigned int contagemRegressiva = 10;

        auxiliarDeInversao = false;
        controleLcd = 0;
        controleLcd1 = 0;

      if (ligado == true)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Invertend Estado");
        lcd.setCursor(0,1);
        lcd.print("Aguarde: ");
      }

      if (ligado == false)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Desligando");
        lcd.setCursor(0,1);
        lcd.print("Aguarde: ");
      }
      
        while (millis() - tempoInicialInversao < tempoEsperaInversao) 
        {
          unsigned int segundosRestantes = (tempoEsperaInversao - (millis() - tempoInicialInversao)) / 1000;
          if (segundosRestantes != contagemRegressiva)
          {
            contagemRegressiva = segundosRestantes;
            lcd.setCursor(9, 1);  // Posição no LCD para exibir a contagem regressiva
            lcd.print(contagemRegressiva);
          }
        }
    }
}
