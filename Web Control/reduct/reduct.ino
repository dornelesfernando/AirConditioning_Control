#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define estado 22         // Relé 1 - Alterna entre o estado quente e frio do compressor - HIGH --> Quente e LOW --> Frio
#define ligaCompressor 23 // Relé 2 - Responsável por ligar e desligar o compressor
#define velocidade1 19    // Relé 3 - Controla o ventilador interno na velocidade 1
#define velocidade2 18    // Relé 4 - Controla o ventilador interno na velocidade 2
#define velocidade3 17    // Relé 5 - Controla o ventilador interno na velocidade 3
#define sensorTemperatura 14
bool quente = false, frio = false, ligado = false;
int estadoRele1 = LOW, estadoRele2 = LOW, estadoRele3 = LOW, estadoRele4 = LOW, estadoRele5 = LOW;
int temperaturaDesejadaFrio = 23, temperaturaDesejadaQuente = 25, limiteEvaporador = 5;           
int controleDiferenca = 2, controleLimiteEvaporador = 5, somaDiferencaAmbienteQuente, somaDiferencaAmbienteFrio, somaDiferencaLimiteEvaporador = limiteEvaporador + controleLimiteEvaporador;
float temp1_evaporador, temp2_ambiente;

bool controlOff = true, auxControlOff = false, controlAtualiza = true, protecaoAtualiza = false, controlCompress = false, auxiliarDeInversao = false;
bool statsV1 = false, statsV2 = false, statsV3 = false;

int radio1 = 0, radio2 = 0, radio3 = 0, veloPadrao = 0, controleLcd = 0, guardaBotaoApertado = 0;

unsigned long previousMillis = 0, interval = 5000, previousMillis2 = 0, interval2 = 60000;

String currentLine = ""; // make a String to hold incoming data from the client

const char *ssid = "USINA 01";
const char *password = "usina105601";

// Define o sensor
OneWire oneWire(sensorTemperatura);
DallasTemperature sensors(&oneWire);

WiFiClient client;
WiFiServer server(80);

void setup()
{
  Serial.begin(115200);

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
    delay(500); Serial.print(".");
    }

  Serial.println("");
  Serial.println("Conectado!");
  Serial.println("Endereço de IP: ");
  Serial.println(WiFi.localIP());

  server.begin();

  sensors.begin();
}

void loop(){
  WiFiClient client = server.available(); // listen for incoming clients
  if (client){                                // if you get a client,
    Serial.println("New Client."); // print a message out the serial port
    currentLine = "";              // make a String to hold incoming data from the client
    while (client.connected()){ // loop while the client's connected
      if (client.available()){                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        Serial.write(c);        // print it out the serial monitor
        if (c == '\n'){
          if (currentLine.length() == 0){
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print("<!DOCTYPE html>");
            client.print("<html lang=\"pt-BR\">");
            client.print("<head>");
            client.print("<meta charset=\"UTF-8\">");
            client.print("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
            client.print("<link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/css2?family=Material+Symbols+Sharp:opsz,wght,FILL,GRAD@20..48,100..700,0..1,-50..200\" />");
            client.print("<style type=\"text/css\">* { margin: 0; padding: 0; color: white; }  body{ background-color: black; background-size: cover; font-family: Arial, Helvetica, sans-serif; }  .main { display: flex; flex-direction: column; justify-content: space-between; background-color: #131819; height: 100%; }  .display #on-off{ position: absolute; padding: 5%; font-weight: normal; }  .display .temperatura{ display: flex; justify-content: center; align-items: center; margin-top: 30px; }  .display .temperatura #temp{ font-size: 150px; font-weight: bold; }  .display .temperatura #celcius{ position: absolute; font-size: 30px; margin-bottom: 75px; margin-left: 200px; font-weight: normal; }  .display #info-estado{ position: absolute; font-size: 25px; margin-bottom: -75px; margin-left: 300px; font-weight: normal; }  .display .textInfo-box{ display: flex; flex-direction: column; margin-left: 25%; margin-bottom: 20px; }  .display .textInfo{ font-size: 1.5rem; font-weight: normal; }  .display .textInfo-box .textInfo .refresh{position: absolute;margin-left: 15px;color: blue;} .estado, .ajusteTemp{ padding: 5px; height: 100px; display: flex; justify-content: space-evenly; align-items: center; text-align: center; background-color: black; border-bottom: 2px solid #131819; }  .estado .icon-box { width: 50%; }  .estado .icon-box.right-border{ border-right: 1px solid #131819; }  .estado .icon-box.left-border{ border-left: 1px solid #131819; }  .estado .icon, .ajusteTemp .icon{ font-size: 100px; }  .estado .icon-box .text-estado{ position: absolute; margin: 40px 0; font-size: 30px; display: none; }  .estado .icon-box .text-estado.text-quente{ background: linear-gradient(75deg, red 3.5%, yellow 95%); background-clip: text; -webkit-background-clip: text; -webkit-text-fill-color: transparent; }  .estado .icon-box .text-estado.text-frio{ margin-left: 25px; background: linear-gradient(75deg, white 3.5%, blue 95%); background-clip: text; -webkit-background-clip: text; -webkit-text-fill-color: transparent; }  .estado .icon.frio:hover + .text-frio{ display: inline; }  .estado .icon.quente:hover + .text-quente{ display: inline; }  .ajusteTemp .info-temp{ font-size: 24px; }  .ajust-left{ margin-left: 25px; }  .ajust-right{ margin-right: 25px; }  .ajusteTemp .aumentar .text-temp, .ajusteTemp .diminuir .text-temp{ position: absolute; margin: 40px 0; font-size: 30px; display: none; }  .ajusteTemp .icon.icon-aumentar:hover + .text-aumentar{ display: inline; }  .ajusteTemp .icon.icon-diminuir:hover + .text-diminuir{ display: inline; }  .velocidades{ display: flex; justify-content: space-evenly; align-items: center; height: 135px; font-size: 24px; background-color: black; }  .velocidades .radio, .velocidades #ajust-radio{ margin-right: 10px; cursor: pointer; } @media screen and (max-width: 450px){ .display #info-estado{ display: none; }  .display{ padding-top: 20px; }  .display .textInfo-box{ margin-left: 0; }  .display .textInfo{ font-size: 1.2rem; }  .estado .icon.frio:hover + .text-frio, .estado .icon.quente:hover + .text-quente, .ajusteTemp .icon.icon-aumentar:hover + .text-aumentar, .ajusteTemp .icon.icon-diminuir:hover + .text-diminuir { display: none; pointer-events: none; }  .velocidades{ flex-direction: column; justify-content: space-between; height: auto; }  .velocidades .radio{ margin-right: 50px; }  .velocidades #ajust-radio { margin: 35px 15px 35px 0; } }  @media screen and (min-width:450px) and (max-width: 600px){ .display{ padding-top: 20px; }  .display .textInfo-box{ margin-left: 25px; }  .display .textInfo{ font-size: 1.2rem; }  .estado .icon.frio:hover + .text-frio, .estado .icon.quente:hover + .text-quente, .ajusteTemp .icon.icon-aumentar:hover + .text-aumentar,.ajusteTemp .icon.icon-diminuir:hover + .text-diminuir {display: none;pointer-events: none;}.velocidades{flex-direction: column;justify-content: space-between;height: auto;}.velocidades .radio{margin-right: 175px;}.velocidades #ajust-radio {margin: 35px 15px 35px 0;}}@media screen and (min-width: 600px) and (max-width: 900px){.display .textInfo-box{margin-left: 25px;}.ajusteTemp .icon.icon-aumentar:hover + .text-aumentar,.ajusteTemp .icon.icon-diminuir:hover + .text-diminuir {display: none;pointer-events: none;}}@media screen and (min-width: 900px) and (max-width: 1100px){.display .textInfo-box{margin-left: 100px;}}@media screen and (min-width: 1100px) and (max-width: 1300px){.display .textInfo-box{margin-left: 150px;}}</style>");
            client.print("<title>Controle Ar Condicionado</title>");
            client.print("</head> <body>");
            client.print("<div class=\"main\">");
            client.print("<div class=\"display\">");
            if (ligado){client.print("<h1 id=\"on-off\">Ligado</h1>");}
            else{client.print("<h1 id=\"on-off\">Desligado</h1>");}
            client.print("<div class=\"temperatura\">");
            client.print("<h1 id=\"temp\">");
            if (ligado){
              if (quente){client.print(temperaturaDesejadaQuente);}
              if (frio){client.print(temperaturaDesejadaFrio);}
            }
            else{client.print("00");}
            client.print("</h1>");
            client.print("<h1 id=\"celcius\">°C</h1>");
            if (ligado){
              if (quente){client.print("<h1 id=\"info-estado\">Aquecer</h1>");}
              if (frio){client.print("<h1 id=\"info-estado\">Refrigerar</h1>");}
            }
            client.print("</div>");
            client.print("<div class=\"textInfo-box\">");
            client.print("<h1 class=\"textInfo\">Temperatura Ambiente: ");
            if (ligado){client.print(temp2_ambiente);}
            else{client.print("~");}
            client.print("°C<a href=\"/refresh\"><span class=\"material-symbols-sharp refresh\">refresh</span></h1><br>");
            client.print("<h1 class=\"textInfo\">Velocidade Atual: ");
            if (ligado){
              if (radio1 == 1){client.print("01");}
              if (radio2 == 1){client.print("02");}
              if (radio3 == 1){client.print("03");}
            }
            else{client.print("~");}
            client.print("</h1> </div> </div>");
            client.print("<div class=\"estado\">");
            client.print("<div class=\"icon-box right-border\">");
            client.print("<a href=\"/quente\"");
            if (ligado){
              if (guardaBotaoApertado == 1){client.print("onclick=\"alert('Desligando. Aguarde Aprox. 60 segundos')\"");}
              if (guardaBotaoApertado == 2){client.print("onclick=\"alert('Alterando Estado. Aguarde Aprox. 60 segundos')\"");}
            }
            client.print(">");
            client.print("<span class=\"material-symbols-sharp icon quente\">");
            client.print("local_fire_department </span>");
            client.print("<span class=\"text-estado text-quente\">Quente</span> </a> </div>");
            client.print("<div class=\"icon-box left-border\">");
            client.print("<a href=\"/frio\"");
            if (ligado){
              if (guardaBotaoApertado == 2){client.print("onclick=\"alert('Desligando. Aguarde Aprox. 60 segundos')\"");}
              if (guardaBotaoApertado == 1){client.print("onclick=\"alert('Alterando Estado. Aguarde Aprox. 60 segundos')\"");}
            }
            client.print(">");
            client.print("<span class=\"material-symbols-sharp icon frio\">");
            client.print("ac_unit </span>");
            client.print("<span class=\"text-estado text-frio\">Frio</span>");
            client.print("</a> </div> </div>");
            client.print("<div class=\"ajusteTemp\">");
            client.print("<div class=\"aumentar\">");
            client.print("<a href=\"/mais\">");
            client.print("<span class=\"material-symbols-sharp icon ajust-left icon-aumentar\">");
            client.print("add </span>");
            client.print("<span class=\"text-temp text-aumentar\">Aumentar</span>");
            client.print("</a> </div>");
            client.print("<div class=\"info-temp\">temp.</div>");
            client.print("<div class=\"diminuir\">");
            client.print("<a href=\"/menos\">");
            client.print("<span class=\"material-symbols-sharp icon ajust-right icon-diminuir\">");
            client.print("remove </span>");
            client.print("<span class=\"text-temp text-diminuir\">Diminuir</span>");
            client.print("</a> </div> </div>");
            client.print("<div class=\"velocidades\">");
            client.print("<a href=\"/v1\" class=\"radio\">");
            if (quente){
              if (radio1 == 1){client.print("<input id=\"ajust-radio\" type=\"radio\" name=\"radio\" id=\"radio1\" checked>");}
            }
            else{client.print("<input id=\"ajust-radio\" type=\"radio\" name=\"radio\" id=\"radio1\">");}
            client.print("Velocidade 01 </a>");
            client.print("<a href=\"/v2\" class=\"radio\">");
            if (quente){
              if (radio2 == 1){client.print("<input id=\"ajust-radio\" type=\"radio\" name=\"radio\" id=\"radio2\" checked>");}
            }
            else{client.print("<input id=\"ajust-radio\" type=\"radio\" name=\"radio\" id=\"radio2\" >");}
            client.print("Velocidade 02 </a>");
            client.print("<a href=\"/v3\" class=\"radio\">");
            if (quente){
              if (radio3 == 1){client.print("<input id=\"ajust-radio\" type=\"radio\" name=\"radio\" id=\"radio3\" checked>");}
            }
            else{client.print("<input id=\"ajust-radio\" type=\"radio\" name=\"radio\" id=\"radio3\">");}
            client.print("Velocidade 03 </a>");
            client.print("</div> </div> </body> </html>");

            client.println();
            break;
          }
          else{currentLine = "";}
        }
        else if (c != '\r'){currentLine += c; }
        if (ligado){
          if (currentLine.endsWith("GET /v1")){statsV1 = true; radio1 = 1; radio2 = 0; radio3 = 0;}
          if (currentLine.endsWith("GET /v2")){statsV2 = true; radio1 = 0; radio2 = 1; radio3 = 0;}
          if (currentLine.endsWith("GET /v3")){statsV3 = true; radio1 = 0; radio2 = 0; radio3 = 1;}
          if (currentLine.endsWith("GET /mais")){
            if (frio){temperaturaDesejadaFrio++;somaDiferencaAmbienteFrio = temperaturaDesejadaFrio;}
            if (quente){temperaturaDesejadaQuente++;somaDiferencaAmbienteQuente = temperaturaDesejadaQuente;}
          }
          if (currentLine.endsWith("GET /menos")){
            if (frio){temperaturaDesejadaFrio--;somaDiferencaAmbienteFrio = temperaturaDesejadaFrio;}
            if (quente){temperaturaDesejadaQuente--;somaDiferencaAmbienteQuente = temperaturaDesejadaQuente;}
          }
        }
        inversao(); varLigaDesliga();
      }
    }
    client.stop();
    Serial.println("Cliente desconectado.");
  }
  if (controleLcd == 0){Serial.println("Desligado");controleLcd = 1;}

  if (ligado){
    if (auxControlOff){controlOff = true;auxControlOff = false;}
    if (veloPadrao == 0) {estadoRele3 = !estadoRele3;veloPadrao = 1;}
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval){
      previousMillis = currentMillis;
      sensors.requestTemperatures();
      DeviceAddress sensor1Address; DeviceAddress sensor2Address;
      sensors.getAddress(sensor1Address, 0); sensors.getAddress(sensor2Address, 1);
      temp1_evaporador = sensors.getTempC(sensor1Address); temp2_ambiente = sensors.getTempC(sensor2Address);
    }

    if (statsV1){
      controlAtualiza = true; statsV1 = false;
      if (estadoRele3 == LOW){
        if (estadoRele4 == HIGH){estadoRele4 = !estadoRele4;}
        if (estadoRele5 == HIGH){estadoRele5 = !estadoRele5;}
        estadoRele3 = !estadoRele3;
        }
    }

    if (statsV2){
      controlAtualiza = true;
      statsV2 = false;
      if (estadoRele4 == LOW){
        if (estadoRele3 == HIGH){estadoRele3 = !estadoRele3;}
        if (estadoRele5 == HIGH){estadoRele5 = !estadoRele5;}
        estadoRele4 = !estadoRele4;
      }
    }

    if (statsV3){
      controlAtualiza = true;
      statsV3 = false;
      if (estadoRele5 == LOW){
        if (estadoRele3 == HIGH){estadoRele3 = !estadoRele3;}
        if (estadoRele4 == HIGH){estadoRele4 = !estadoRele4;}
        estadoRele5 = !estadoRele5;
      }
    }

    if (frio){
      if (controlCompress){
        estadoRele1 = LOW; controlCompress = false; controlAtualiza = true;
      }

      if (temp1_evaporador <= limiteEvaporador || temp2_ambiente <= temperaturaDesejadaFrio){
        if (estadoRele2 == HIGH){
          estadoRele2 = !estadoRele2;
          somaDiferencaAmbienteFrio = temperaturaDesejadaFrio + controleDiferenca; 
          controlCompress = true; controlAtualiza = true;
        }
      }
      if (temp2_ambiente > somaDiferencaAmbienteFrio){
        if (temp1_evaporador > somaDiferencaLimiteEvaporador){
          if (estadoRele2 == LOW){
            estadoRele2 = !estadoRele2; controlCompress = true; controlAtualiza = true;
          }
        }
      }
    }

    if (quente){
      if (controlCompress){estadoRele1 = HIGH; controlCompress = false; controlAtualiza = true;}
      if (temp2_ambiente >= temperaturaDesejadaQuente){
        if (estadoRele2 == HIGH){
          estadoRele2 = !estadoRele2;
          somaDiferencaAmbienteQuente = temperaturaDesejadaQuente - controleDiferenca; // Ativa o conceito de Histerese
          controlCompress = true; controlAtualiza = true;
        }
      }

      if (temp2_ambiente < somaDiferencaAmbienteQuente){
        if (estadoRele2 == LOW){estadoRele2 = !estadoRele2; controlCompress = true; controlAtualiza = true;}
      }
    }

    if (protecaoAtualiza) {
      unsigned long currentMillis2 = millis();
      if (currentMillis2 - previousMillis2 >= interval2){
        previousMillis2 = currentMillis2; protecaoAtualiza = false;
      }
    }

    if (controlAtualiza){
      if (protecaoAtualiza == false){
        digitalWrite(estado, estadoRele1); digitalWrite(ligaCompressor, estadoRele2);
        digitalWrite(velocidade1, estadoRele3); digitalWrite(velocidade2, estadoRele4);
        digitalWrite(velocidade3, estadoRele5); controlAtualiza = false; protecaoAtualiza = true;
      }
    }
  }
  else{
    if (controlOff){
      // Atualização do estado dos pinos de saída para LOW enquanto o sistema estiver desligado
      digitalWrite(estado, LOW); digitalWrite(velocidade1, LOW);
      digitalWrite(velocidade2, LOW); digitalWrite(velocidade3, LOW);
      digitalWrite(ligaCompressor, LOW); controlOff = false; auxControlOff = true;
    }
  }
}

void varLigaDesliga(){
  if (currentLine.endsWith("GET /quente")){
    if (guardaBotaoApertado == 1){
      if (ligado){auxiliarDeInversao = true; quente = false;}
      else{quente = true;}
      ligado = !ligado; 
    }
    else{
      if (guardaBotaoApertado == 0){ligado = !ligado; quente = true;}
      else{if (ligado){auxiliarDeInversao = true;}quente = true;}
    }
    controlCompress = true; controlAtualiza = true; guardaBotaoApertado = 1; frio = false;
    somaDiferencaAmbienteQuente = temperaturaDesejadaQuente; somaDiferencaAmbienteFrio = temperaturaDesejadaFrio;
  }

  if (currentLine.endsWith("GET /frio")){
    if (guardaBotaoApertado == 2){
      if (ligado){auxiliarDeInversao = true; frio = false;}
      else{frio = true;}
      ligado = !ligado; // Desliga o sisema
    }
    else{
      if (guardaBotaoApertado == 0){ligado = !ligado;frio = true;}
      else{
        if (ligado){auxiliarDeInversao = true;}
        frio = true;
      }
    }
    controlCompress = true; controlAtualiza = true;
    guardaBotaoApertado = 2; quente = false;
    somaDiferencaAmbienteQuente = temperaturaDesejadaQuente; somaDiferencaAmbienteFrio = temperaturaDesejadaFrio;
  }
}

void inversao() {               
  if (auxiliarDeInversao){
    digitalWrite(ligaCompressor, LOW);
    unsigned long tempoEsperaInversao = 60000, tempoInicialInversao = millis();
    unsigned int contagemRegressiva = 60;
    auxiliarDeInversao = false; controleLcd = 0;            
    if (ligado){Serial.print("Invertendo Estado");}
    if (ligado == false){Serial.print("Desligando");}
    Serial.print("Aguarde: ");
    while (millis() - tempoInicialInversao < tempoEsperaInversao){
      unsigned int segundosRestantes = (tempoEsperaInversao - (millis() - tempoInicialInversao)) / 1000;
      if (segundosRestantes != contagemRegressiva){contagemRegressiva = segundosRestantes;Serial.println(contagemRegressiva);}
    }
  }
}
