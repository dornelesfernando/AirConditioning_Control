//Bibliotecas para o uso do LCD 16x2 I2C
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//Bibliotecas para o uso do sensor de temperartura 18b20
#include <OneWire.h>
#include <DallasTemperature.h>

//Definição das variáveis, suas respectivas portas no microcontrolador e sua função
#define estado 33  //Relé 1 - Alterna entre o estado quente e frio do compressor - HIGH --> Quente e LOW --> Frio
#define ligaCompressor 23  //Relé 2 - Responsável por ligar e desligar o compressor
#define velocidade1 19  //Relé 3 - Controla o ventilador interno na velocidade 1
#define velocidade2 18  //Relé 4 - Controla o ventilador interno na velocidade 2
#define velocidade3 17  //Relé 5 - Controla o ventilador interno na velocidade 3

//Definição das variáveis e suas respectivas portas responsáveis pelos push-buttons
#define botao1_OnQuente 32  //Botão responsável por ligar/desligar o Ar-condicionado no estado Quente
#define botao2_OnFrio 16  //Botão responsável por ligar/desligar o Ar-condicionado no estado Frio
#define botao3_v1 25  //Botão responsável por ligar o ventilador interno na velocidade 1
#define botao4_v2 26  //Botão responsável por ligar o ventilador interno na velocidade 2
#define botao5_v3 27  //Botão responsável por ligar o ventilador interno na velocidade 3

//Definição da variável e sua respectiva porta responsável pelo sensor de temperartura 18b20
#define sensorTemperatura 14

//Variáveis responsaveis por cuidar do estado do compressor
bool quente = false;
bool frio = false;

bool ligado = false;  //Variável que indica se o sistema está ou não ligado

int estadoRele1 = LOW;  //Cuida o estado da relé 1
int estadoRele2 = LOW;  //Cuida o estado da relé 2
int estadoRele3 = LOW;  //Cuida o estado da relé 3
int estadoRele4 = LOW;  //Cuida o estado da relé 4
int estadoRele5 = LOW;  //Cuida o estado da relé 5

int btEstado1 = LOW;  //Cuida o estado do botão botao1_OnQuente
int btEstado2 = LOW;  //Cuida o estado do botão botao2_OnFrio
int btEstado3 = LOW;  //Cuida o estado do botão botao3_v1
int btEstado4 = LOW;  //Cuida o estado do botão botao4_v2
int btEstado5 = LOW;  //Cuida o estado do botão botao5_v3

//Variáveis de controle¹
int temperaturaDesejadaFrio = 23;  //Define a temperatura padrão no estado frio como 23°C
int temperaturaDesejadaQuente = 25;  //Define a temperatura padrão no estado quente como 25°C
int limiteEvaporador = 5;  //Dá um limite de temperatura que o evaporador estará operante em °C

//Variáveis utilizadas para evitar que um relé fique ativando e desativando rapidamente, utilizando o conceito de histerese
int controleDiferenca = 2;
int controleLimiteEvaporador = 5;
int somaDiferencaAmbienteQuente = temperaturaDesejadaQuente - controleDiferenca;
int somaDiferencaAmbienteFrio = temperaturaDesejadaFrio + controleDiferenca;
int somaDiferencaLimiteEvaporador = limiteEvaporador + controleLimiteEvaporador;

//Variáveis de controle²
int veloPadrao = 0;
int controleLcd = 0;
int controleLcd1 = 0;
int esperaInicio = 0;
bool auxiliarDeInversao = false;
int guardaBotaoApertado = 0;
/*
  A variável guardaBotaoApertado é uma variável que serve para que aconteça inversão
  de estados sem ocorrer o desligamento do sistema de forma indesejada

  0 --> Neutro
  1 --> O Botão anteriormente pressionado foi o botão liga_quente
  2 --> O Botão anteriormente pressionado foi o botão liga_frio
*/

LiquidCrystal_I2C lcd(0x27, 16, 2);  //Biblioteca lcd(endereço I2C, Linhas, Colunas)

//Define o sensor
OneWire oneWire(sensorTemperatura);
DallasTemperature sensors(&oneWire);

void setup() {
  //Inicialização do display LCD
  lcd.init();
  lcd.backlight();

  //Define os pinos de saída
  pinMode(estado, OUTPUT);
  pinMode(ligaCompressor, OUTPUT);
  pinMode(velocidade1, OUTPUT);
  pinMode(velocidade2, OUTPUT);
  pinMode(velocidade3, OUTPUT);

  //Define os pinos de entrada
  pinMode(botao1_OnQuente, INPUT_PULLUP);
  pinMode(botao2_OnFrio, INPUT_PULLUP);
  pinMode(botao3_v1, INPUT_PULLUP);
  pinMode(botao4_v2, INPUT_PULLUP);
  pinMode(botao5_v3, INPUT_PULLUP);

  //Inicializa os sensores
  sensors.begin();
}

void loop() {
  if (esperaInicio == 0)  //Se o arCondicionado foi ligado pela primeira vez
  {
    lcd.print("Aguarde 5s...");
    delay(5000);
    lcd.clear();
    esperaInicio = 1;  //Definindo como 1 para indicar que o processo foi concluído
  }

  if (controleLcd == 0)
  {
    lcd.clear();
    lcd.print("Desligado");
    controleLcd = 1;  //Definindo como 1 para indicar que o processo foi concluído
    /*int*/ controleLcd1 = 0;  //Definindo como 0 para que possa ser executado posteriormente
  }


  while (ligado)  //Loop principal enquanto o arCondicionado está ligado
  {
    if (veloPadrao == 0) //Define a velocidade 1 como padrão
    {
      estadoRele3 = !estadoRele3;
      veloPadrao = 1;  //Definindo como 1 para indicar que o processo foi concluído
    }

    if (controleLcd1 == 0) //Organiza as informações a serem apresentadas no LCD
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Evaporador:");
      lcd.setCursor(0, 1);
      lcd.print("Ambiente:");
      controleLcd1 = 1;  //Definindo como 1 para indicar que o processo foi concluído
    }

    //Lê as temperaturas dos sensores
    sensors.requestTemperatures();

    //Obtém as temperaturas lidas individualmente pelos endereços dos sensores
    DeviceAddress sensor1Address;
    DeviceAddress sensor2Address;

    sensors.getAddress(sensor1Address, 0);
    sensors.getAddress(sensor2Address, 1);

    float temp1_evaporador = sensors.getTempC(sensor1Address);
    float temp2_ambiente = sensors.getTempC(sensor2Address);

    //Imprime as temperaturas no monitor serial
    lcd.setCursor(11, 0);
    lcd.print(temp1_evaporador);
    lcd.setCursor(11, 1);
    lcd.print(temp2_ambiente);

    //Condições de ativação e desativação das velocidaddes fazendo que apenas 1 velocidade esteja ativa por vez
    btEstado3 = digitalRead(botao3_v1);
    if (btEstado3 == HIGH)
    {
      if (estadoRele3 == LOW)
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
        //delay(200);
      }
    }

    btEstado4 = digitalRead(botao4_v2);
    if (btEstado4 == HIGH)
    {
      if (estadoRele4 == LOW)
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
        //delay(200);
      }
    }

    btEstado5 = digitalRead(botao5_v3);
    if (btEstado5 == HIGH)
    {
      if (estadoRele5 == LOW)
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
        //delay(200);
      }
    }

    //Condições referente ao estado no qual o compressor se encontra
    if (frio)
    {
      estadoRele1 = LOW;  //liga o compressor no estado frio

      //Condições de desativação do compressor
      if (temp1_evaporador <= limiteEvaporador || temp2_ambiente <= temperaturaDesejadaFrio)
      {
        if (estadoRele2 == HIGH)
        {
          estadoRele2 = !estadoRele2;
          somaDiferencaAmbienteFrio = temperaturaDesejadaFrio + controleDiferenca;  //Ativa o conceito de Histerese
        }
      }

      //Condições de ativação do compressor
      if (temp2_ambiente > somaDiferencaAmbienteFrio)
      {
        if (temp1_evaporador > somaDiferencaLimiteEvaporador)
        {
          if (estadoRele2 == LOW)
          {
            estadoRele2 = !estadoRele2;
          }
        }
      }
    }

    if (quente)
    {
      estadoRele1 = HIGH;  //liga o compressor no estado quente

      //Condição de desativação do compressor
      if (temp2_ambiente >= temperaturaDesejadaQuente)
      {
        if (estadoRele2 == HIGH) {
          estadoRele2 = !estadoRele2;
          somaDiferencaAmbienteQuente = temperaturaDesejadaQuente - controleDiferenca;  //Ativa o conceito de Histerese
        }
      }

      //Condição de ativação do compressor
      if (temp2_ambiente < somaDiferencaAmbienteQuente)
      {
        if (estadoRele2 == LOW) {
          estadoRele2 = !estadoRele2;
        }
      }
    }

    //Atualização do estado dos pinos de saída
    digitalWrite(estado, estadoRele1);
    digitalWrite(velocidade1, estadoRele3);
    digitalWrite(velocidade2, estadoRele4);
    digitalWrite(velocidade3, estadoRele5);
    digitalWrite(ligaCompressor, estadoRele2);

    varLigaDesliga(); //Verifica as condições do sistema como Ligado/desligado ou estado do compressor
    inversao();  //Função de intervalo para troca de estado ou desligamento
  }

  //Atualização do estado dos pinos de saída para LOW enquanto o sistema estiver desligado
  digitalWrite(estado, LOW);
  digitalWrite(velocidade1, LOW);
  digitalWrite(velocidade2, LOW);
  digitalWrite(velocidade3, LOW);
  digitalWrite(ligaCompressor, LOW);

  varLigaDesliga();
  inversao();
}

void varLigaDesliga()  //Função utilizada para controlar a ativação e desativação do sistema
{
  btEstado1 = digitalRead(botao1_OnQuente);
  if (btEstado1 == HIGH)
  {
    if (guardaBotaoApertado == 1)
    {
      if (ligado) {
        auxiliarDeInversao = true;  //Ativa a condição para a Função de intervalo
      }
      ligado = !ligado;  //Desliga o sistema
      quente = false;
    }
    else {
      if (guardaBotaoApertado == 0)
      {
        ligado = !ligado;  //Liga o sistema
        quente = true;  //Define o estado do compressor como quente
      }
      else {
        if (ligado)
        {
          auxiliarDeInversao = true;
        }
        quente = true;
      }
    }

    guardaBotaoApertado = 1;  //Define que o botão 1 foi pressionado anteriormente
    frio = false;
    somaDiferencaAmbienteQuente = somaDiferencaAmbienteQuente + controleDiferenca;
    //delay(200);
  }

  btEstado2 = digitalRead(botao2_OnFrio);
  if (btEstado2 == HIGH)
  {
    if (guardaBotaoApertado == 2)
    {
      if (ligado)
      {
        auxiliarDeInversao = true;  //Ativa a condição para a Função de intervalo
      }

      ligado = !ligado;  //Desliga o sisema
      frio = false;
    }
    else {
      if (guardaBotaoApertado == 0)
      {
        ligado = !ligado;  //Liga o sisema
        frio = true;  //Define o estado do compressor como frio
      }
      else
      {
        if (ligado)
        {
          auxiliarDeInversao = true;
        }
        frio = true;
      }
    }

    guardaBotaoApertado = 2;  //Define que o botão 2 foi pressionado anteriormente
    quente = false;
    somaDiferencaAmbienteFrio = somaDiferencaAmbienteFrio - controleDiferenca;
    //delay(200);
  }
}

void inversao()  //Função de Intervalo usada para que não haja a ativação e desativação
{ //do compressor em um curto periodo de tempo para evitar possíveis danos

  if (auxiliarDeInversao)
  {
    digitalWrite(ligaCompressor, LOW);

    unsigned long tempoEsperaInversao = 60000; //1 minuto
    unsigned long tempoInicialInversao = millis();
    unsigned int contagemRegressiva = 60;

    auxiliarDeInversao = false;  //Desativa a condição para a Função de intervalo
    controleLcd = 0;  //Definindo como 0 para que possa ser executado posteriormente
    controleLcd1 = 0;

    if (ligado)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Invertend Estado");
    }

    if (ligado == false)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Desligando");
    }

    lcd.setCursor(0, 1);
    lcd.print("Aguarde: ");

    while (millis() - tempoInicialInversao < tempoEsperaInversao)
    {
      unsigned int segundosRestantes = (tempoEsperaInversao - (millis() - tempoInicialInversao)) / 1000;
      if (segundosRestantes != contagemRegressiva)
      {
        contagemRegressiva = segundosRestantes;
        lcd.setCursor(9, 1);  //Posição no LCD para exibir a contagem regressiva
        lcd.print(contagemRegressiva);
      }
    }
  }
}
