#include <WiFi.h>

const char* ssid = "USINA 01";
const char* password = "usina105601";

WiFiServer server(80);

void setup() {
  //Inicialização do display LCD
  Serial.begin(115200);

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
                    /*CODE CSS*/
                // o conteúdo do cabeçalho HTTP
                    /*CODE HTML*/
        
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

                    /*CODE C++*/
            }
        }   
    // termina a conexão com o cliente
    client.stop();
    Serial.println("Cliente desconectado.");
    }
}