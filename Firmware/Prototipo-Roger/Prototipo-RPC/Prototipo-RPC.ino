#include <WiFi.h>
#include "SimpleBLE.h"
#include "EEPROM.h"
#include "MegunoLink.h"
#include "Filter.h"
 
// Create a new exponential filter with a weight of 5 and an initial value of 0. 
ExponentialFilter<long> FilterPot1(10, 0);
ExponentialFilter<long> FilterPot2(10, 0);

const char *ssid = "SimulacareRPC";
const char *password = "12345678";
int keyIndex = 0;
int wifi_status = WL_IDLE_STATUS;

const int POT1 = 36;        // Analog input pin , ours is connected to POT1
const int POT2 = 39;        // Analog input pin , ours is connected to POT2
const int SW1 = 35;         // Push-Button
const int ledPin = 12;      // the number of the LED 
const int ledVM = 32;       // the number of the LED Vermelho
const int ledVD = 33;       // the number of the LED Verde

int sensorPot1 = 0;        // value read from the adc POT1
int sensorPot2 = 0;        // value read from the adc POT1

// Variables will change:
int ledState = HIGH;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 1;    // the debounce time; increase if the output flickers

SimpleBLE ble;
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  Serial.println();
  pinMode(POT1, INPUT);
  pinMode(POT2, INPUT);
  pinMode(SW1, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(ledVM, OUTPUT);
  pinMode(ledVD, OUTPUT);
  // Apaga Todos os Leds
  digitalWrite(ledPin, LOW);
  digitalWrite(ledVM, LOW);
  digitalWrite(ledVD, LOW);
  //
  Serial.printf("Simulacare - Prototipo Manequim RPC v.1.0.0");
  Serial.println();
  EEPROM.begin(64);
  Serial.print("Configuring WIFI Server...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();
  printWifiStatus();                                                                                                                                                                                                           
  Serial.println("HTTP server started");
  ble.begin("BLE RCP");
  Serial.println("Bluetooth started");
  // Teste de Leds
  Serial.println("Testing Leds");
  digitalWrite(ledPin, HIGH);
  delay(1000);
  Serial.println("Red Button ON");
  digitalWrite(ledVM, HIGH);
  digitalWrite(ledVD, LOW);
  delay(1000);
  Serial.println("Red Bicolor ON");
  digitalWrite(ledVD, HIGH);
  digitalWrite(ledVM, LOW);
  delay(1000);
  Serial.println("Green Bicolor ON");
  digitalWrite(ledPin, LOW);
  delay(1000);
  Serial.println("Red Button OFF");
  digitalWrite(ledVM, LOW);
  delay(1000);
  Serial.println("Red Bicolor OFF");
  digitalWrite(ledVD, LOW);
  delay(1000);
  Serial.println("Green Bicolor OFF");
  Serial.println("Start Loop");
}

void loop() {
  // POT 1
  sensorPot1 = analogRead(POT1);
  FilterPot1.Filter(sensorPot1);
  // print the results to the serial monitor:
  Serial.print("POT1 = ");
  //Serial.println(sensorPot1);
  Serial.println(FilterPot1.Current());
  // POT 2
  sensorPot2 = analogRead(POT2);
  FilterPot2.Filter(sensorPot2);
  // print the results to the serial monitor:
  Serial.print("POT2 = ");
  //Serial.println(sensorPot2);
  Serial.println(FilterPot2.Current());

  // read the state of the switch into a local variable:
  int reading = digitalRead(SW1);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = micros();       // millis();
  }

  //if ((millis() - lastDebounceTime) > debounceDelay) {
   if ((micros() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        ledState = !ledState;
      }
    }
  }

  // set the LED:
  digitalWrite(ledPin, ledState);
  Serial.print("Led = ");
  Serial.println(ledState);
  
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;

  Wifi_Listen();
  
  delay(1);
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void Wifi_Listen() {
   // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          //client.println("Refresh: 1");  // refresh the page automatically every 1 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
          //for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
          //  int sensorReading = analogRead(analogChannel);
            client.print("input POT1");
            //client.print(analogChannel);
            client.print(" is ");
            client.print(FilterPot1.Current());
            client.println("<br />");       
          //}
            client.print("input POT2");
            //client.print(analogChannel);
            client.print(" is ");
            client.print(FilterPot2.Current());
            client.println("<br />");
            //
            client.print("Push-Button");
            //client.print(analogChannel);
            client.print(" is ");
            client.print(ledState);
            client.println("<br />");
            //         
            client.println("</html>");
           break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}

