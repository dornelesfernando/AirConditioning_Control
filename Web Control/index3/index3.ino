#include <WiFi.h>

#define ligaDesliga 23
#define velocidade1 22
#define velocidade2 21
#define velocidade3 19

const char *ssid = "ssid_rede";
const char *password = "password";

WiFiServer server(80);

bool ligaDesliga_stats = 0;
bool velocidade1_stats = 0;
bool velocidade2_stats = 0;
bool velocidade3_stats = 0;

int contador = 4;
int led1 = 26, led2 = 25, led3 = 33, led4 = 32, led5 = 18, led6 = 17, led7 = 16;

void setup() {
    Serial.begin(115200);
    pinMode(ligaDesliga, OUTPUT);
    pinMode(velocidade1, OUTPUT);
    pinMode(velocidade2, OUTPUT);
    pinMode(velocidade3, OUTPUT);

    //*****************  
    pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);
    pinMode(led3, OUTPUT);
    pinMode(led4, OUTPUT);
    pinMode(led5, OUTPUT);
    pinMode(led6, OUTPUT);
    pinMode(led7, OUTPUT);
    //*****************

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
}

void loop() {
 WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
        if (client.available()) {             // if there's bytes to read from the client,
            char c = client.read();             // read a byte, then
            Serial.write(c);                    // print it out the serial monitor
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
                    client.print("<style type=\"text/css\"> body{background: #cccccc;font-family: Arial, Helvetica, sans-serif;}.pagina-wrapper {padding-top: 20px;}.estrutura {max-width: 250px;max-height: 500px;height: 500px;margin: 0 auto;background-color: #FFF;box-shadow: 0px 0px 10px #2e2e2e;border-radius: 5px;}.display {background-color: #a83737;height: 200px;}.wrapper-ligaDesliga {background-color: #1671bd;height: 80px;}.temperatura{background-color: #e7098b;height: 80px;}.velocidades{background-color: #1cff4d;height: 140px;} </style>");
                // o conteúdo do cabeçalho HTTP
                    client.print("<body>");
                    client.print("<div class=\"pagina-wrapper\">");
                        client.print("<form class=\"estrutura\">");
                            client.print("<div class=\"display\">");
                                client.print("a");
                            client.print("</div>");
                            client.print("<div class=\"wrapper-ligaDesliga\">");
                                client.print("<a href=\"/ligar\">Ligar/Desligar</a></br>");
                            client.print("</div>");
                            client.print("<div class=\"temperatura\">");
                                client.print("<a href=\"/mais\">Aumentar Temperatura</a></br>");
                                client.print("<a href=\"/menos\">Diminuir Temperatura</a></br>");
                            client.print("</div>");
                            client.print("<div class=\"velocidades\">");
                                client.print("<input type=\"radio\" name=\"radio\" id=\"radio1\" onchange=\"redirecionar()\">");
                                client.print("<label for=\"radio1\">Velocidade 01</label> </br>");
                                client.print("<input type=\"radio\" name=\"radio\" id=\"radio2\" onchange=\"redirecionar()\">");
                                client.print("<label for=\"radio2\">Velocidade 02</label> </br>");
                                client.print("<input type=\"radio\" name=\"radio\" id=\"radio3\" onchange=\"redirecionar()\">");
                                client.print("<label for=\"radio3\">Velocidade 03</label> </br>");
                            client.print("</div>");
                        client.print("</form>");
                    client.print("</div>");

                    client.print("<script>");
                        client.print("function redirecionar(){");
                            client.print("var opcaoSelecionada = document.querySelector('input[name=\"radio\"]:checked').id;");
                            client.print("var url;");

                            client.print("switch (opcaoSelecionada) {");
                                client.print("case \"radio1\":");
                                    client.print("url = \"http://192.168.0.31/v1\";");
                                client.print("break;");

                                client.print("case \"radio2\":");
                                    client.print("url = \"http://192.168.0.31/v2\";");
                                client.print("break;");

                                client.print("case \"radio3\":");
                                    client.print("url = \"http://192.168.0.31/v3\";");
                                client.print("break;");
                            
                                client.print("default:");
                                client.print("url = \"http://192.168.0.31\";");
                            client.print("}");
                            client.print("window.location.href = url;");
                        client.print("}");
                    client.print("</script>");
                    client.print("</body>");
        
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
 
                if (currentLine.endsWith("GET /ligar")) {
                if (ligaDesliga_stats == LOW) {
                digitalWrite(ligaDesliga, HIGH);
                ligaDesliga_stats = HIGH;
                } else {
                digitalWrite(ligaDesliga, LOW);
                ligaDesliga_stats = LOW;
                }
                }

                if (currentLine.endsWith("GET /v1")) {
                if (velocidade1_stats == LOW) {
                digitalWrite(velocidade1, HIGH);
                velocidade1_stats = HIGH;
                } else {
                digitalWrite(velocidade1, LOW);
                velocidade1_stats = LOW;
                }
                }

                if (currentLine.endsWith("GET /v2")) {
                if (velocidade2_stats == LOW) {
                digitalWrite(velocidade2, HIGH);
                velocidade2_stats = HIGH;
                } else {
                digitalWrite(velocidade2, LOW);
                velocidade2_stats = LOW;
                }
                }

                if (currentLine.endsWith("GET /v3")) {
                if (velocidade3_stats == LOW) {
                digitalWrite(velocidade3, HIGH);
                velocidade3_stats = HIGH;
                } else {
                digitalWrite(velocidade3, LOW);
                velocidade3_stats = LOW;
                }
                }
                
                // **************

                if (currentLine.endsWith("GET /mais")) {
                    contador++;
                }
                if (currentLine.endsWith("GET /menos")) {
                    contador--;
                }

                if (contador == 1)
                {
                    digitalWrite(led1, HIGH);
                    digitalWrite(led2, LOW);
                    digitalWrite(led3, LOW);
                    digitalWrite(led4, LOW);
                    digitalWrite(led5, LOW);
                    digitalWrite(led6, LOW);
                    digitalWrite(led7, LOW);
                }

                if (contador == 2)
                {
                    digitalWrite(led1, HIGH);
                    digitalWrite(led2, HIGH);
                    digitalWrite(led3, LOW);
                    digitalWrite(led4, LOW);
                    digitalWrite(led5, LOW);
                    digitalWrite(led6, LOW);
                    digitalWrite(led7, LOW);
                }

                if (contador == 3)
                {
                    digitalWrite(led1, HIGH);
                    digitalWrite(led2, HIGH);
                    digitalWrite(led3, HIGH);
                    digitalWrite(led4, LOW);
                    digitalWrite(led5, LOW);
                    digitalWrite(led6, LOW);
                    digitalWrite(led7, LOW);
                }

                if (contador == 4)
                {
                    digitalWrite(led1, HIGH);
                    digitalWrite(led2, HIGH);
                    digitalWrite(led3, HIGH);
                    digitalWrite(led4, HIGH);
                    digitalWrite(led5, LOW);
                    digitalWrite(led6, LOW);
                    digitalWrite(led7, LOW);
                }

                if (contador == 5)
                {
                    digitalWrite(led1, HIGH);
                    digitalWrite(led2, HIGH);
                    digitalWrite(led3, HIGH);
                    digitalWrite(led4, HIGH);
                    digitalWrite(led5, HIGH);
                    digitalWrite(led6, LOW);
                    digitalWrite(led7, LOW);
                }

                if (contador == 6)
                {
                    digitalWrite(led1, HIGH);
                    digitalWrite(led2, HIGH);
                    digitalWrite(led3, HIGH);
                    digitalWrite(led4, HIGH);
                    digitalWrite(led5, HIGH);
                    digitalWrite(led6, HIGH);
                    digitalWrite(led7, LOW);
                }

                if (contador == 7)
                {
                    digitalWrite(led1, HIGH);
                    digitalWrite(led2, HIGH);
                    digitalWrite(led3, HIGH);
                    digitalWrite(led4, HIGH);
                    digitalWrite(led5, HIGH);
                    digitalWrite(led6, HIGH);
                    digitalWrite(led7, HIGH);
                }
                //***************
            }
        }   
    // termina a conexão com o cliente
    client.stop();
    Serial.println("Cliente desconectado.");
    }
}
