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
int contadorInversao = 0;
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

WiFiClient client;
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
            client.print("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
            client.print("<link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/css2?family=Material+Symbols+Sharp:opsz,wght,FILL,GRAD@20..48,100..700,0..1,-50..200\" />");
            
            // configuração de estilo do site
            client.print("<style type=\"text/css\">* { margin: 0; padding: 0; color: white; }  body{ background-color: black; background-size: cover; font-family: Arial, Helvetica, sans-serif; }  .main { display: flex; flex-direction: column; justify-content: space-between; background-color: #131819; height: 100%; }  .display #on-off{ position: absolute; padding: 5%; font-weight: normal; }  .display .temperatura{ display: flex; justify-content: center; align-items: center; margin-top: 30px; }  .display .temperatura #temp{ font-size: 150px; font-weight: bold; }  .display .temperatura #celcius{ position: absolute; font-size: 30px; margin-bottom: 75px; margin-left: 200px; font-weight: normal; }  .display #info-estado{ position: absolute; font-size: 25px; margin-bottom: -75px; margin-left: 300px; font-weight: normal; }  .display .textInfo-box{ display: flex; flex-direction: column; margin-left: 25%; margin-bottom: 20px; }  .display .textInfo{ font-size: 1.5rem; font-weight: normal; }  .display .textInfo-box .textInfo .refresh{position: absolute;margin-left: 15px;color: blue;} .estado, .ajusteTemp{ padding: 5px; height: 100px; display: flex; justify-content: space-evenly; align-items: center; text-align: center; background-color: black; border-bottom: 2px solid #131819; }  .estado .icon-box { width: 50%; }  .estado .icon-box.right-border{ border-right: 1px solid #131819; }  .estado .icon-box.left-border{ border-left: 1px solid #131819; }  .estado .icon, .ajusteTemp .icon{ font-size: 100px; }  .estado .icon-box .text-estado{ position: absolute; margin: 40px 0; font-size: 30px; display: none; }  .estado .icon-box .text-estado.text-quente{ background: linear-gradient(75deg, red 3.5%, yellow 95%); background-clip: text; -webkit-background-clip: text; -webkit-text-fill-color: transparent; }  .estado .icon-box .text-estado.text-frio{ margin-left: 25px; background: linear-gradient(75deg, white 3.5%, blue 95%); background-clip: text; -webkit-background-clip: text; -webkit-text-fill-color: transparent; }  .estado .icon.frio:hover + .text-frio{ display: inline; }  .estado .icon.quente:hover + .text-quente{ display: inline; }  .ajusteTemp .info-temp{ font-size: 24px; }  .ajust-left{ margin-left: 25px; }  .ajust-right{ margin-right: 25px; }  .ajusteTemp .aumentar .text-temp, .ajusteTemp .diminuir .text-temp{ position: absolute; margin: 40px 0; font-size: 30px; display: none; }  .ajusteTemp .icon.icon-aumentar:hover + .text-aumentar{ display: inline; }  .ajusteTemp .icon.icon-diminuir:hover + .text-diminuir{ display: inline; }  .velocidades{ display: flex; justify-content: space-evenly; align-items: center; height: 135px; font-size: 24px; background-color: black; }  .velocidades .radio, .velocidades #ajust-radio{ margin-right: 10px; cursor: pointer; } @media screen and (max-width: 450px){ .display #info-estado{ display: none; }  .display{ padding-top: 20px; }  .display .textInfo-box{ margin-left: 0; }  .display .textInfo{ font-size: 1.2rem; }  .estado .icon.frio:hover + .text-frio, .estado .icon.quente:hover + .text-quente, .ajusteTemp .icon.icon-aumentar:hover + .text-aumentar, .ajusteTemp .icon.icon-diminuir:hover + .text-diminuir { display: none; pointer-events: none; }  .velocidades{ flex-direction: column; justify-content: space-between; height: auto; }  .velocidades .radio{ margin-right: 50px; }  .velocidades #ajust-radio { margin: 35px 15px 35px 0; } }  @media screen and (min-width:450px) and (max-width: 600px){ .display{ padding-top: 20px; }  .display .textInfo-box{ margin-left: 25px; }  .display .textInfo{ font-size: 1.2rem; }  .estado .icon.frio:hover + .text-frio, .estado .icon.quente:hover + .text-quente, .ajusteTemp .icon.icon-aumentar:hover + .text-aumentar,.ajusteTemp .icon.icon-diminuir:hover + .text-diminuir {display: none;pointer-events: none;}.velocidades{flex-direction: column;justify-content: space-between;height: auto;}.velocidades .radio{margin-right: 175px;}.velocidades #ajust-radio {margin: 35px 15px 35px 0;}}@media screen and (min-width: 600px) and (max-width: 900px){.display .textInfo-box{margin-left: 25px;}.ajusteTemp .icon.icon-aumentar:hover + .text-aumentar,.ajusteTemp .icon.icon-diminuir:hover + .text-diminuir {display: none;pointer-events: none;}}@media screen and (min-width: 900px) and (max-width: 1100px){.display .textInfo-box{margin-left: 100px;}}@media screen and (min-width: 1100px) and (max-width: 1300px){.display .textInfo-box{margin-left: 150px;}}</style>");

            client.print("<title>Controle Ar Condicionado</title>");
            client.print("</head>");

            // o conteúdo do cabeçalho HTTP
            client.print("<body>");
            client.print("<div class=\"main\">");
            client.print("<div class=\"display\">");
            if (ligado)
            {
              client.print("<h1 id=\"on-off\">Ligado</h1>");
            }else{
              client.print("<h1 id=\"on-off\">Desligado</h1>");
            }
            client.print("<div class=\"temperatura\">");
            client.print("<h1 id=\"temp\">");
            if (ligado)
            {
                if (quente)
                {
                    client.print(temperaturaDesejadaQuente);
                }if(frio){
                    client.print(temperaturaDesejadaFrio);
                }                         
            }else{
                client.print("00");
            }   
            client.print("</h1>");
            client.print("<h1 id=\"celcius\">°C</h1>");
            if (ligado)
            {
              if(quente){
                client.print("<h1 id=\"info-estado\">Aquecer</h1>");
              }
              if(frio){                 
                client.print("<h1 id=\"info-estado\">Refrigerar</h1>");
              }
            }
            client.print("</div>");
            client.print("<div class=\"textInfo-box\">");
            client.print("<h1 class=\"textInfo\">Temperatura Ambiente: ");
            if (ligado)
            {
              client.print(temp2_ambiente);
            }else{
              client.print("~");
            }
            client.print("°C<a href=\"/refresh\"><span class=\"material-symbols-sharp refresh\">refresh</span></h1><br>");
            
            client.print("<h1 class=\"textInfo\">Velocidade Atual: ");
            if (ligado)
            {
              if (radio1 == 1)
              {
              client.print("01");
              }if (radio2 == 1)
              {
              client.print("02");
              }
              if (radio3 == 1)
              {
              client.print("03");
              }
            }else{
              client.print("~");
            }
            client.print("</h1>");
            client.print("</div>");
            client.print("</div>");
            if (auxiliarDeInversao)
            {            
              client.print("auxiliarDeInversao");
            }                        
            client.print("<div class=\"estado\">");
            client.print("<div class=\"icon-box right-border\">");
            client.print("<a href=\"/quente\">");
            client.print("<span class=\"material-symbols-sharp icon quente\">");
            client.print("local_fire_department");
            client.print("</span>");
            client.print("<span class=\"text-estado text-quente\">Quente</span>");
            client.print("</a>");
            client.print("</div>");
            client.print("<div class=\"icon-box left-border\">");
            client.print("<a href=\"/frio\">");
            client.print("<span class=\"material-symbols-sharp icon frio\">");
            client.print("ac_unit");
            client.print("</span>");
            client.print("<span class=\"text-estado text-frio\">Frio</span>");
            client.print("</a>");
            client.print("</div>");
            client.print("</div>");
            client.print("<div class=\"ajusteTemp\">");
            client.print("<div class=\"aumentar\">");
            client.print("<a href=\"/mais\">");
            client.print("<span class=\"material-symbols-sharp icon ajust-left icon-aumentar\">");
            client.print("add");
            client.print("</span>");
            client.print("<span class=\"text-temp text-aumentar\">Aumentar</span>");
            client.print("</a>");
            client.print("</div>");
            client.print("<div class=\"info-temp\">temp.</div>");
            client.print("<div class=\"diminuir\">");
            client.print("<a href=\"/menos\">");
            client.print("<span class=\"material-symbols-sharp icon ajust-right icon-diminuir\">");
            client.print("remove");
            client.print("</span>");
            client.print("<span class=\"text-temp text-diminuir\">Diminuir</span>");
            client.print("</a>");
            client.print("</div>");
            client.print("</div>");
            client.print("<div class=\"velocidades\">");
            client.print("<a href=\"/v1\" class=\"radio\">");
            if(quente){if (radio1 == 1)
            {
              client.print("<input id=\"ajust-radio\" type=\"radio\" name=\"radio\" id=\"radio1\" checked>");
            }} else {
              client.print("<input id=\"ajust-radio\" type=\"radio\" name=\"radio\" id=\"radio1\">");
            }
            client.print("Velocidade 01");
            client.print("</a>");
            client.print("<a href=\"/v2\" class=\"radio\">");
            if(quente){if (radio2 == 1)
            {
              client.print("<input id=\"ajust-radio\" type=\"radio\" name=\"radio\" id=\"radio2\" checked>");
            }} else {
              client.print("<input id=\"ajust-radio\" type=\"radio\" name=\"radio\" id=\"radio2\" >");
            }
            client.print("Velocidade 02");
            client.print("</a>");  
            client.print("<a href=\"/v3\" class=\"radio\">");
            if(quente){if (radio3 == 1)
            {
              client.print("<input id=\"ajust-radio\" type=\"radio\" name=\"radio\" id=\"radio3\" checked>");
            }} else {
              client.print("<input id=\"ajust-radio\" type=\"radio\" name=\"radio\" id=\"radio3\">");
            }
            client.print("Velocidade 03");
            client.print("</a>");
            client.print("</div>");
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
          if (frio)
            {
              temperaturaDesejadaFrio++;
            }
          if (quente)
            {
              temperaturaDesejadaQuente++;
            }
        }
        if (currentLine.endsWith("GET /menos"))
        {
          if (frio)
            {
              temperaturaDesejadaFrio--;
            }
            if (quente)
            {
              temperaturaDesejadaQuente--;
            }
        }

        inversao();
        varLigaDesliga();

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
      //Serial.println(temp1_evaporador);
      //Serial.println(temp2_ambiente);
      //Serial.println(temperaturaDesejadaQuente);
      //Serial.println(temperaturaDesejadaFrio);
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
    if (contadorInversao == 1)
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
    if (contadorInversao == 0){contadorInversao = 1;}else{contadorInversao = 1;}
  }
}