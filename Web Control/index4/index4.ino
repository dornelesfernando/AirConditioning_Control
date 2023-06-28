#include <WiFi.h>

//Bibliotecas para o uso do sensor de temperartura 18b20
#include <OneWire.h>
#include <DallasTemperature.h>

//Definição das variáveis, suas respectivas portas no microcontrolador e sua função
#define estado 33  //Relé 1 - Alterna entre o estado quente e frio do compressor - HIGH --> Quente e LOW --> Frio
#define ligaCompressor 16  //Relé 2 - Responsável por ligar e desligar o compressor
#define velocidade1 19  //Relé 3 - Controla o ventilador interno na velocidade 1
#define velocidade2 18  //Relé 4 - Controla o ventilador interno na velocidade 2
#define velocidade3 17  //Relé 5 - Controla o ventilador interno na velocidade 3

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

float temp1_evaporador;
float temp2_ambiente;

//Variáveis de controle²
int contadorTemp = 0;
int controladorDesligado = 0;
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

String currentLine = "";                // make a String to hold incoming data from the client

const char* ssid = "USINA 01";
const char* password = "usina105601";

//Define o sensor
OneWire oneWire(sensorTemperatura);
DallasTemperature sensors(&oneWire);

WiFiServer server(80);

void setup() {
  //Inicialização do display LCD
  Serial.begin(115200);

  //Define os pinos de saída
  pinMode(estado, OUTPUT);
  pinMode(ligaCompressor, OUTPUT);
  pinMode(velocidade1, OUTPUT);
  pinMode(velocidade2, OUTPUT);
  pinMode(velocidade3, OUTPUT);

  delay(10);

    Serial.println();
    Serial.println("Conectando a:");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.println(".");
    }

    Serial.println("");
    Serial.println("Conectado!");
    Serial.println("Endereço de IP: ");
    Serial.println(WiFi.localIP());

    server.begin();
    
    //Inicializa os sensores
    sensors.begin();
}

void loop() {
 WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    currentLine = "";                       // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
        if (client.available()) {             // if there's bytes to read from the client,
            char c = client.read();             // read a byte, then
            Serial.write(c); 
            if (c == '\n') {                    // if the byte is a newline character
                // if the current line is blank, you got two newline characters in a row.
                // that's the end of the client HTTP request, so send a response:
                if (currentLine.length() == 0) {
                    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                    // and a content-type so the client knows what's coming, then a blank line:
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println();
        
                // configuração de estilo do site
                // client.print("<style type=\"text/css\"> a{margin: 50px 50px; background: #000000; color: #ffffff; text-decoration: none; padding: 1% 20% 1%; border-radius: 10px; font-size: 8.0em;}</style>");

                // o conteúdo do cabeçalho HTTP
                    client.print("<a href=\"/quente\">Ligar Quente</a></br>");
                    client.print("<a href=\"/frio\">Ligar Frio</a></br>");
                    client.print("<a href=\"/mais\">Aumentar Temperatura</a></br>");
                    client.print("<a href=\"/menos\">Diminuir Temperatura</a></br>");
                    client.print("<a href=\"/v1\">Velocidade 01</a></br>");
                    client.print("<a href=\"/v2\">Velocidade 02</a></br>");
                    client.print("<a href=\"/v3\">Velocidade 03</a></br>");
        
                    // A resposta HTTP termina com outra linha em branco:
                    client.println();
                    // break out of the while loop:
                    break;
                } else {    // if you got a newline, then clear currentLine:
                currentLine = "";
                }
                } else if (c != '\r') {  // if you got anything else but a carriage return character,
                currentLine += c;      // add it to the end of the currentLine
                }
                //*******************************************************************
                if (esperaInicio == 0)  //Se o arCondicionado foi ligado pela primeira vez
                {
                  Serial.print("Aguarde 5s...");
                  delay(5000);
                  esperaInicio = 1;  //Definindo como 1 para indicar que o processo foi concluído
                }
                if (controleLcd == 0)
                {
                  Serial.print("Desligado");
                  controleLcd = 1;  //Definindo como 1 para indicar que o processo foi concluído
                }
                if (ligado)  //Loop principal enquanto o arCondicionado está ligado
                {
                  sistemaLigado();
                }
                else
                {
                  sistemaDesligado();
                }
              
/*      if (currentLine.endsWith("GET /quente"))
      {
        //digitalWrite(led1, HIGH);
      }
      if (currentLine.endsWith("GET /frio"))
      {
        //digitalWrite(led2, HIGH);
      }
        if(currentLine.endsWith("GET /mais"))
      {
//                  temperaturaDesejadaFrio++;
      }
      if(currentLine.endsWith("GET /menos"))
      {
//                  temperaturaDesejadaFrio--;
      }
      if (currentLine.endsWith("GET /v1"))
      {
//                  digitalWrite(led3, HIGH);
//                  digitalWrite(led4, LOW);
//                  digitalWrite(led5, LOW);
      }
      if (currentLine.endsWith("GET /v2"))
      {
//                  digitalWrite(led3, LOW);
//                  digitalWrite(led4, HIGH);
//                  digitalWrite(led5, LOW);
      }
      if (currentLine.endsWith("GET /v3"))
      {
//                  digitalWrite(led3, LOW);
//                  digitalWrite(led4, LOW);
//                  digitalWrite(led5, HIGH);
      }
//    Serial.println(temperaturaDesejadaFrio);*/
                //*******************************************************************
            }
        }   
    // termina a conexão com o cliente
    client.stop();
    Serial.println("Cliente desconectado.");
    }
}

void sistemaLigado() {
  if (veloPadrao == 0) //Define a velocidade 1 como padrão
  {
    estadoRele3 = !estadoRele3;
    veloPadrao = 1;  //Definindo como 1 para indicar que o processo foi concluído
  }

  if (contadorTemp == 10)
  {
    //Lê as temperaturas dos sensores
  sensors.requestTemperatures();

  //Obtém as temperaturas lidas individualmente pelos endereços dos sensores
  DeviceAddress sensor1Address;
  DeviceAddress sensor2Address;

  sensors.getAddress(sensor1Address, 0);
  sensors.getAddress(sensor2Address, 1);

   temp1_evaporador = sensors.getTempC(sensor1Address);
   temp2_ambiente = sensors.getTempC(sensor2Address);

  //Imprime as temperaturas no monitor serial
  //Serial.println(temp1_evaporador);
  //Serial.println(temp2_ambiente);
  Serial.print("Temperatura lida");
  contadorTemp = 0;
  }
  
  //Condições de ativação e desativação das velocidaddes fazendo que apenas 1 velocidade esteja ativa por vez
  if (currentLine.endsWith("GET /v1"))
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

  if (currentLine.endsWith("GET /v2"))
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

  if (currentLine.endsWith("GET /v3"))
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
    Serial.print("Frio 1");
    estadoRele1 = LOW;  //liga o compressor no estado frio

    //Condições de desativação do compressor
    if (temp1_evaporador <= limiteEvaporador || temp2_ambiente <= temperaturaDesejadaFrio)
    {
      if (estadoRele2 == HIGH)
      {
        Serial.print("Frio 2");
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
          Serial.print("Frio 3");
        }
      }
    }
  }

  if (quente)
  {
    estadoRele1 = HIGH;  //liga o compressor no estado quente
    Serial.print("Quente 1");
    //Condição de desativação do compressor
    if (temp2_ambiente >= temperaturaDesejadaQuente)
    {
      if (estadoRele2 == HIGH) {
        Serial.print("Quente 2");
        estadoRele2 = !estadoRele2;
        somaDiferencaAmbienteQuente = temperaturaDesejadaQuente - controleDiferenca;  //Ativa o conceito de Histerese
      }
    }

    //Condição de ativação do compressor
    if (temp2_ambiente < somaDiferencaAmbienteQuente)
    {
      if (estadoRele2 == LOW) {
        estadoRele2 = !estadoRele2;
        Serial.print("Quente 3");
      }
    }
  }

  //Atualização do estado dos pinos de saída
  digitalWrite(estado, estadoRele1);
  digitalWrite(velocidade1, estadoRele3);
  digitalWrite(velocidade2, estadoRele4);
  digitalWrite(velocidade3, estadoRele5);
  digitalWrite(ligaCompressor, estadoRele2);
  Serial.print("Sistema Ligado");
  contadorTemp++;

  varLigaDesliga(); //Verifica as condições do sistema como Ligado/desligado ou estado do compressor
  inversao();  //Função de intervalo para troca de estado ou desligamento
}

void sistemaDesligado(){
  if(controladorDesligado == 15){
  //Atualização do estado dos pinos de saída para LOW enquanto o sistema estiver desligado
  digitalWrite(estado, LOW);
  digitalWrite(velocidade1, LOW);
  digitalWrite(velocidade2, LOW);
  digitalWrite(velocidade3, LOW);
  digitalWrite(ligaCompressor, LOW);

  varLigaDesliga();
  inversao();
  controladorDesligado = 0;
  }
  controladorDesligado++;
}

void varLigaDesliga()  //Função utilizada para controlar a ativação e desativação do sistema
{    
  if (currentLine.endsWith("GET /quente"))
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
  
  if (currentLine.endsWith("GET /frio"))
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

    unsigned long tempoEsperaInversao = 10000; //1 minuto
    unsigned long tempoInicialInversao = millis();
    unsigned int contagemRegressiva = 10;

    auxiliarDeInversao = false;  //Desativa a condição para a Função de intervalo
    controleLcd = 0;  //Definindo como 0 para que possa ser executado posteriormente

    if (ligado)
    {
      Serial.print("Invertendo Estado");
    }

    if (ligado == false)
    {
      Serial.print("Desligando");
    }

    Serial.print("Aguarde: ");

    while (millis() - tempoInicialInversao < tempoEsperaInversao)
    {
      unsigned int segundosRestantes = (tempoEsperaInversao - (millis() - tempoInicialInversao)) / 1000;
      if (segundosRestantes != contagemRegressiva)
      {
        contagemRegressiva = segundosRestantes;
        Serial.println(contagemRegressiva);
      }
    }
  }
}
