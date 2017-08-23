#include <WiFi.h>
#include <Wire.h>
#include "SimpleBLE.h"
#include "EEPROM.h"
#include "MegunoLink.h",.
#include "Filter.h"
#include "VL6180X.h"
//#include "sensor_Encoder.h"
#include "hardware.h"

// Create a new exponential filter with a weight of 10 and an initial value of 0. 
ExponentialFilter<long> FilterPot1(10, 0);
ExponentialFilter<long> FilterPot2(10, 0);
ExponentialFilter<long> FilterVbat(10, 0);

float VBattery;       // battery voltage from ESP32 ADC read

SimpleBLE ble;
WiFiServer server(80);

uint64_t chipid;  
/* Set these to your desired credentials. */
const char *ssid = "SimulacareRPC";
const char *password = "12345678";

// Variaveis Analogicas
int sensorPot1 = 0;
int sensorPot2 = 0;
int sensorVbat = 0;

// Variaveis SW
int buzzerState = HIGH;
int SW1State = HIGH;
int SW2State = HIGH;
int buttonSW1State;
int lastSW1State = LOW;
unsigned long lastDebounceSW1 = 0;
int buttonSW2State;
int lastSW2State = LOW;
unsigned long lastDebounceSW2 = 0;
int buttonSW3State;
int lastSW3State = LOW;
unsigned long lastDebounceSW3 = 0;
unsigned long debounceDelay = 1;

int keyIndex = 0;
int wifi_status = WL_IDLE_STATUS;
uint8_t state = 0;

unsigned int hasteA = 0;
unsigned int hasteB = 0;

hw_timer_t * timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile uint32_t isrCounter = 0;
volatile uint32_t lastIsrAt = 0;
volatile uint32_t tmr_180ms = 0;

void IRAM_ATTR onTimer(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  isrCounter++;
  tmr_180ms++;
  lastIsrAt = micros();
  portEXIT_CRITICAL_ISR(&timerMux);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output
  if (tmr_180ms <= 7) {
     digitalWrite(ledAZ1, HIGH);
     if(buzzerState){
        digitalWrite(BUZZER, HIGH);             // activate beep
    }
  }
  if ((tmr_180ms == 7) && (tmr_180ms <= 20)) {
    digitalWrite(ledAZ1, LOW);
    digitalWrite(BUZZER, LOW);
  }
  else if (tmr_180ms == 20) {
     tmr_180ms = 0;
  }   
}

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot() {
  //server.send(200, "text/html", "<h1>You are connected</h1>");
}

void onButton(){
    String out = "BLE RCP: ";
    out += String(millis() / 1000);
    Serial.println(out);
    ble.begin(out);
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  config_hardware();
  Wire.begin();
  Serial.println();
  Serial.printf("Simulacare - Manequim RPC v.1.0.0");
  Serial.println();
  EEPROM.begin(64);
  chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf("ESP32 Chip ID = %04X",(uint16_t)(chipid>>32));//print High 2 bytes
  Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.
  // Create semaphore to inform us when the timer has fired
  timerSemaphore = xSemaphoreCreateBinary();

  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  // info).
  timer = timerBegin(0, 80, true);

  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer, &onTimer, true);

  // Set alarm to call onTimer function every second (value in microseconds)
  // Repeat the alarm (third parameter)
  //timerAlarmWrite(timer, 1000000, true);
  
  // Set Timer Interrupt every 30ms.
  timerAlarmWrite(timer, 30000, true);
  

  // Start an alarm
  timerAlarmEnable(timer);
    
  Serial.println("Configuring WIFI Server...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();                                                                                                                                                                                                           
  Serial.println("HTTP server started");
  ble.begin("BLE RCP");
  btStart();
  Serial.println("Bluetooth BLE Started");
  //teste_leds();
  Init_VL6180();      // Inicia Sensor ToF
  led_bicolor(LED1, COR_VERDE);
}

void loop() {
//  pisca_led();
  le_POT1();
  le_POT2();
  le_VBAT();
  le_SW3();
  le_haste();
  Wifi_Listen();
}

void le_haste()
{
  //get_distance();
}

void pisca_led()
{
    if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE){
    uint32_t isrCount = 0, isrTime = 0;
    // Read the interrupt count and time
    portENTER_CRITICAL(&timerMux);
    isrCount = isrCounter;
    isrTime = lastIsrAt;
    portEXIT_CRITICAL(&timerMux);
    // Print it
    //Serial.print("onTimer no. ");
    //Serial.print(isrCount);
    //Serial.print(" at ");
    //Serial.print(isrTime);
    //Serial.println(" ms");
    if(state == 0) {
        digitalWrite(ledAZ1, HIGH);
        state = 255;
    }
    else {
        digitalWrite(ledAZ1, LOW);
        state = 0;      
    }
  }
}

void le_POT1()
{
  sensorPot1 = analogRead(POT1);
  FilterPot1.Filter(sensorPot1);
  //Serial.println(FilterPot1.Current());
}

void le_POT2()
{
  sensorPot2 = analogRead(POT2);
  FilterPot2.Filter(sensorPot2);
  //Serial.println(FilterPot2.Current());
}

void le_VBAT()
{
  sensorVbat = analogRead(VBAT);
  FilterVbat.Filter(sensorVbat);
  //Serial.println(FilterVbat.Current());
  // Battery Voltage
  VBattery = (120.0/20.0) * (float)(FilterVbat.Current()) / 1024.0;       // LiPo battery voltage in volts
}

void le_SW1()
{
  // Le Estado da Chave SW1
  int readSW1 = digitalRead(SW1);

  // if the switch changed, due to noise or pressing
  if (readSW1 != lastSW1State) {
    // Reset the Debounce Timer
    lastDebounceSW1 = micros();
  }
  if ((micros() - lastDebounceSW1) > debounceDelay) {
    if (readSW1 != buttonSW1State) {
      buttonSW1State = readSW1;
      // toggle State
      if (buttonSW1State == HIGH) {
        SW1State = !SW1State;
      }
    }
  }
  // Save the reading
  lastSW1State = readSW1;
}

void le_SW2()
{
  // Le Estado da Chave SW2
  int readSW2 = digitalRead(SW2);

  // if the switch changed, due to noise or pressing
  if (readSW2 != lastSW2State) {
    // Reset the Debounce Timer
    lastDebounceSW2 = micros();
  }
  if ((micros() - lastDebounceSW2) > debounceDelay) {
    if (readSW2 != buttonSW2State) {
      buttonSW2State = readSW2;
      // toggle State
      if (buttonSW2State == HIGH) {
        SW2State = !SW2State;
      }
    }
  }
  // Save the reading
  lastSW2State = readSW2;
}

void le_SW3()
{
  // Le Estado da Chave SW3
  int readSW3 = digitalRead(SW3);

  // if the switch changed, due to noise or pressing
  if (readSW3 != lastSW3State) {
    // Reset the Debounce Timer
    lastDebounceSW3 = micros();
  }
  if ((micros() - lastDebounceSW3) > debounceDelay) {
    if (readSW3 != buttonSW3State) {
      buttonSW3State = readSW3;
      // toggle State
      if (buttonSW3State == HIGH) {
        buzzerState = !buzzerState;
      }
    }
  }
  // Save the reading
  lastSW3State = readSW3;
}

void config_hardware()
{
  // Leds
  pinMode(ledVM1, OUTPUT);
  pinMode(ledVD1, OUTPUT);
  pinMode(ledVM2, OUTPUT);
  pinMode(ledVD2, OUTPUT);
  pinMode(ledVM3, OUTPUT);
  pinMode(ledVM4, OUTPUT);
  pinMode(ledVM5, OUTPUT);
  pinMode(ledAZ1, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(CS0, OUTPUT);
  pinMode(CS1, OUTPUT);
  // Analog Pins
  pinMode(POT1, INPUT);
  pinMode(POT2, INPUT);
  pinMode(VBAT, INPUT);
  // Pin Inputs (SW1,SW2,SW3)
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  // Pin Inputs Encoder
  //pinMode(ENC1_A, INPUT_PULLUP);
  //pinMode(ENC1_B, INPUT_PULLUP);
  //pinMode(ENC2_A, INPUT_PULLUP);
  //pinMode(ENC2_B, INPUT_PULLUP);

  digitalWrite(ledVM1, LOW);      // Desliga Led VM1
  digitalWrite(ledVD1, LOW);      // Desliga Led VD1
  digitalWrite(ledVM2, LOW);      // Desliga Led VM2
  digitalWrite(ledVD2, LOW);      // Desliga Led VD2
  digitalWrite(ledVM3, LOW);      // Desliga Led VM3
  digitalWrite(ledVM4, LOW);      // Desliga Led VM4
  digitalWrite(ledVM5, LOW);      // Desliga Led VM5
  digitalWrite(ledAZ1, LOW);      // Desliga Led AZ1
  //
  digitalWrite(CS0, LOW);         // Selecao 0 VL6180X
  digitalWrite(CS1, LOW);         // Selecao 1 VL6180X
  digitalWrite(BUZZER, LOW);      // Desliga Buzzer
}

void teste_leds()
{
  Serial.println("Teste de Leds Start");
  // Teste Led Acesso
  digitalWrite(ledVM1, HIGH);
  delay(500);
  digitalWrite(ledVM1, LOW);
  digitalWrite(ledVD1, HIGH);
  delay(500);
  digitalWrite(ledVM2, HIGH);
  delay(500);
  digitalWrite(ledVM2, LOW);
  digitalWrite(ledVD2, HIGH);
  delay(500);
  digitalWrite(ledVM3, HIGH);
  delay(500);
  digitalWrite(ledVM4, HIGH);
  delay(500); 
  digitalWrite(ledAZ1, HIGH);
  delay(500); 
  digitalWrite(ledVM5, HIGH);
  delay(500);
  // Teste Led Apagado
  digitalWrite(ledVM1, LOW);
  delay(500);
  digitalWrite(ledVD1, LOW);
  delay(500);
  digitalWrite(ledVM2, LOW);
  delay(500);
  digitalWrite(ledVD2, LOW);
  delay(500);
  digitalWrite(ledVM3, LOW);
  delay(500);
  digitalWrite(ledVM4, LOW);
  delay(500); 
  digitalWrite(ledAZ1, LOW);
  delay(500); 
  digitalWrite(ledVM5, LOW);
  delay(500);
  Serial.println("Teste de Leds End.");
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  //Serial.print("SSID: ");
  //Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.softAPIP();
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
          client.println("Refresh: 1");  // refresh the page automatically every 1 sec
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
            client.print("Bateria");
            client.print(" is ");
            client.print(FilterVbat.Current());
            client.println("<br />");
            //
            client.print("Tensao Bateria");
            client.print(" is ");
            client.print(VBattery);
            client.println("<br />");
            //
            client.print("Buzzer");
            //client.print(analogChannel);
            client.print(" is ");
            client.print(buzzerState);
            client.println("<br />");
            //
            client.print("SW1");
            client.print(" is ");
            client.print(SW1State);
            client.println("<br />");
            //
            client.print("SW2");
            client.print(" is ");
            client.print(SW2State);
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

void beep(uint16_t time)
{
    if(buzzerState){
        digitalWrite(BUZZER, HIGH); // activate beep
    }
    delay(time/2);
    digitalWrite(BUZZER, LOW);
}

void led_bicolor(uint8_t led, uint8_t cor)
{
  if (led == LED1) {
    if (cor == COR_VERMELHO) { 
      digitalWrite(ledVM1, HIGH);
      digitalWrite(ledVD1, LOW);
    }
    else {
      digitalWrite(ledVM1, LOW);
      digitalWrite(ledVD1, HIGH);
    }    
  }
  else if (led == LED2) {
    if (cor == COR_VERMELHO) { 
      digitalWrite(ledVM2, HIGH);
      digitalWrite(ledVD2, LOW);
    }
    else {
      digitalWrite(ledVM2, LOW);
      digitalWrite(ledVD2, HIGH);
    }
  }
}

