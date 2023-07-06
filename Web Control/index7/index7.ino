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

bool controlOff = true;
bool auxControlOff = false;
bool controlAtualiza = true;
bool controlCompress = false;
bool statsV1 = false;
bool statsV2 = false;
bool statsV3 = false;
bool statsMais = false;
bool statsMenos = false;

//Variáveis de controle²
int radio1 = 0;
int radio2 = 0;
int radio3 = 0;
int cP = 0;
int contadorTemp = 0;
int controladorDef = 0;
int controladorDesligado = 0;
int veloPadrao = 0;
int controleLcd = 0;
bool auxiliarDeInversao = false;
int guardaBotaoApertado = 0;
/*
  A variável guardaBotaoApertado é uma variável que serve para que aconteça inversão
  de estados sem ocorrer o desligamento do sistema de forma indesejada

  0 --> Neutro
  1 --> O Botão anteriormente pressionado foi o botão liga_quente
  2 --> O Botão anteriormente pressionado foi o botão liga_frio
*/

unsigned long previousMillis = 0;
unsigned long interval = 5000;

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

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
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
        if (cP == 0) {
          Serial.write(c);                    // print it out the serial monitor
          cP++;
        }
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            client.print("<!DOCTYPE html>");
            client.print("<html lang=\"pt-BR\">");
            client.print("<head>");
            client.print("<meta charset=\"UTF-8\">");

            // configuração de estilo do site
            client.print("<style type=\"text/css\"> body{background: #cccccc;font-family: Arial, Helvetica, sans-serif;}.pagina-wrapper {padding-top: 20px;}.estrutura {display: block;max-width: 250px;max-height: 500px;height: 500px;margin: 0 auto;background-color: #FFF;box-shadow: 0px 0px 10px #2e2e2e;border-radius: 5px;}.display {background-color: #a83737;justify-content: center;align-items: center;height: 150px;display: flex;}.display #number{padding-top: 50px;font-size: 100px;padding-left: 24px;}.info{background-color: #a83737;height: 40px;padding-left: 10px;}.info #info1{margin: 0;font-weight: normal;font-size: 14px;}.wrapper-ligaDesliga {background-color: #1671bd;height: 80px;}.quente{margin-top: 10px;display: flex;  margin-left: 20px;float: left;  }.frio{margin-top: 10px;margin-right: 20px;float: right;display: flex;}.icon{height: 60px;width: 60px;}.temperatura{background-color: #e7098b;height: 80px;}.aumentar{margin-top: 27.5px;display: flex;  margin-left:  40px;float: left; }.textTemp{position: absolute;margin-top: 32px;margin-left: 95px;font-size: 16px;}.diminuir{margin-top: 27.5px;margin-right: 40px;float: right;display: flex;}.icon2{height: 25px;width: 25px;}.velocidades{display: block;background-color: #1cff4d;height: 150px;}.radio{margin-top: 27px;margin-left: 20px;}</style>");

            client.print("<title>Controle Ar Condicionado</title>");
            client.print("</head>");

            // o conteúdo do cabeçalho HTTP
            client.print("<body>");
            client.print("<div class=\"pagina-wrapper\">");
            client.print("<form class=\"estrutura\">");
            client.print("<div class=\"display\">");
            client.print("<h1 id=\"number\">00</h1>");
            client.print("<h1>°C</h1>");
            client.print("</div>");
            client.print("<div class=\"info\">");
            client.print("<h1 id=\"info1\">Temperatura Ambiente:</h1>");
            client.print("<h1 id=\"info1\">Velocidade Atual</h1>");
            client.print("</div>");
            client.print("<div class=\"wrapper-ligaDesliga\">");
            client.print("<div class=\"quente\">");
            client.print("<a href=\"/quente\"><img class=\"icon\" src=\"img/54409.png\" alt=\"quente\"></a><br>");
            client.print("</div>");
            client.print("<div class=\"frio\">");
            client.print("<a href=\"/frio\"><img class=\"icon\" src=\"img/106057.png\" alt=\"frio\"></a><br>");
            client.print("</div>");
            client.print("</div>");
            client.print("<div class=\"temperatura\">");
            client.print("<div class=\"aumentar\">");
            client.print("<a href=\"/mais\"><img class=\"icon2\" src=\"img/mais.png\" alt=\"aumentar\"></a><br>");
            client.print("</div>");
            client.print("<div class=\"diminuir\">");
            client.print("<a href=\"/menos\"><img class=\"icon2\" src=\"img/menos.png\" alt=\"diminuir\"></a><br>");
            client.print("</div>");
            client.print("<div class=\"textTemp\">Temp.</div>");
            client.print("</div>");

            client.print("<div class=\"velocidades\">");
            client.print("<a href=\"/v1\">");
                        
           if (radio1 == 1)
            {
              client.print("<input  class=\"radio\" type=\"radio\" name=\"radio\" id=\"radio1\" onchange=\"redirecionar()\" checked>");
            } else {
              client.print("<input  class=\"radio\" type=\"radio\" name=\"radio\" id=\"radio1\" onchange=\"redirecionar()\">");
            }
            client.print("Velocidade 01");
            client.print("</a>");

            client.print("<a href=\"/v2\">");
            if (radio2 == 1)
            {
              client.print("<input class=\"radio\" type=\"radio\" name=\"radio\" id=\"radio2\" onchange=\"redirecionar()\" checked>");
            } else {
              client.print("<input class=\"radio\" type=\"radio\" name=\"radio\" id=\"radio2\" onchange=\"redirecionar()\">");
            }
            client.print("Velocidade 02");
            client.print("</a>");

            client.print("<a href=\"/v3\">");
            if (radio3 == 1)
            {
              client.print("<input class=\"radio\" type=\"radio\" name=\"radio\" id=\"radio3\"  onchange=\"redirecionar()\" checked>");
            } else {
              client.print("<input class=\"radio\" type=\"radio\" name=\"radio\" id=\"radio3\"  onchange=\"redirecionar()\">");
            }
            client.print("Velocidade 03");
            client.print("</a>");

            client.print("</div>");
            client.print("</form>");
            client.print("</div>");
            client.print("</body>");
            client.print("</html>");
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

        inversao();
        varLigaDesliga();

        if (currentLine.endsWith("GET /v1"))
        {
          statsV1 = true;
          radio1 = 1;
          radio2 = 0;
          radio3 = 0;

        }
        if (currentLine.endsWith("GET /v2"))
        {
          statsV2 = true;
          radio1 = 0;
          radio2 = 1;
          radio3 = 0;

        }
        if (currentLine.endsWith("GET /v3"))
        {
          statsV3 = true;
          radio1 = 0;
          radio2 = 0;
          radio3 = 1;
        }
        if (currentLine.endsWith("GET /mais"))
        {
          statsMais = true;
        }
        if (currentLine.endsWith("GET /menos"))
        {
          statsMenos = true;
        }

        //*******************************************************************
      }
    }
    // termina a conexão com o cliente
    client.stop();
    Serial.println("Cliente desconectado.");
  }
  if (controleLcd == 0)
  {
    Serial.println("Desligado");
    controleLcd = 1;  //Definindo como 1 para indicar que o processo foi concluído
  }

  if (ligado) //Loop principal enquanto o arCondicionado está ligado
  {
    if (auxControlOff)
    {
      controlOff = true;
      auxControlOff = false;
    }

    if (veloPadrao == 0) //Define a velocidade 1 como padrão
    {
      estadoRele3 = !estadoRele3;
      veloPadrao = 1;  //Definindo como 1 para indicar que o processo foi concluído
    }

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval)
    {
      previousMillis = currentMillis;

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
      Serial.println(temp1_evaporador);
      Serial.println(temp2_ambiente);
    }

    //Condições de ativação e desativação das velocidaddes fazendo que apenas 1 velocidade esteja ativa por vez
    if (statsV1)
    {
      controlAtualiza = true;
      statsV1 = false;
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

    if (statsV2)
    {
      controlAtualiza = true;
      statsV2 = false;
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

    if (statsV3)
    {
      controlAtualiza = true;
      statsV3 = false;
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

    if (statsMais)
    {
      statsMais = false;
      if (frio)
      {
        temperaturaDesejadaFrio++;
      }
      if (quente)
      {
        temperaturaDesejadaQuente++;
      }
    }
    if (statsMenos)
    {
      statsMenos = false;
      if (frio)
      {
        temperaturaDesejadaFrio--;
      }
      if (quente)
      {
        temperaturaDesejadaQuente--;
      }
    }

    //Condições referente ao estado no qual o compressor se encontra
    if (frio)
    {
      if (controlCompress)
      {
        estadoRele1 = LOW;  //liga o compressor no estado frio
        controlCompress = false;
      }

      //Condições de desativação do compressor
      if (temp1_evaporador <= limiteEvaporador || temp2_ambiente <= temperaturaDesejadaFrio)
      {
        if (estadoRele2 == HIGH)
        {
          estadoRele2 = !estadoRele2;
          somaDiferencaAmbienteFrio = temperaturaDesejadaFrio + controleDiferenca;  //Ativa o conceito de Histerese
          controlCompress = true;
          controlAtualiza = true;
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
            controlCompress = true;
            controlAtualiza = true;
          }
        }
      }
    }

    if (quente)
    {
      if (controlCompress)
      {
        estadoRele1 = HIGH;  //liga o compressor no estado quente
        controlCompress = false;
        controlAtualiza = true;
      }

      //Condição de desativação do compressor
      if (temp2_ambiente >= temperaturaDesejadaQuente)
      {
        if (estadoRele2 == HIGH) {
          estadoRele2 = !estadoRele2;
          somaDiferencaAmbienteQuente = temperaturaDesejadaQuente - controleDiferenca;  //Ativa o conceito de Histerese
          controlCompress = true;
          controlAtualiza = true;
        }
      }

      //Condição de ativação do compressor
      if (temp2_ambiente < somaDiferencaAmbienteQuente)
      {
        if (estadoRele2 == LOW) {
          estadoRele2 = !estadoRele2;
          controlCompress = true;
          controlAtualiza = true;
        }
      }
    }

    if (controlAtualiza)
    {
      //Atualização do estado dos pinos de saída
      digitalWrite(estado, estadoRele1);
      digitalWrite(velocidade1, estadoRele3);
      digitalWrite(velocidade2, estadoRele4);
      digitalWrite(velocidade3, estadoRele5);
      digitalWrite(ligaCompressor, estadoRele2);
      controlAtualiza = false;
    }
  }
  else {
    if (controlOff)
    {
      //Atualização do estado dos pinos de saída para LOW enquanto o sistema estiver desligado
      digitalWrite(estado, LOW);
      digitalWrite(velocidade1, LOW);
      digitalWrite(velocidade2, LOW);
      digitalWrite(velocidade3, LOW);
      digitalWrite(ligaCompressor, LOW);
      controlOff = false;
      auxControlOff = true;
    }
  }
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
        radio1 = 1;
      }
      else {
        if (ligado)
        {
          auxiliarDeInversao = true;
        }
        quente = true;
      }
    }

    controlCompress = true;
    controlAtualiza = true;
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
        radio1 = 1;
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

    controlCompress = true;
    controlAtualiza = true;
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