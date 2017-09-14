#include <WiFi.h>
#include <Wire.h>
#include <Preferences.h>
#include <Bounce2.h>
#include "EEPROM.h"
#include "MegunoLink.h",
#include "Filter.h"
#include "ToF.h"
#include "Web_Config.h"
#include "hardware.h"
#include "data.h"
#include "Bluetooth.h"
#include "pitches.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

// Instantiate a Bounce object :
Bounce switchSW1 = Bounce(); 
Bounce switchSW2 = Bounce(); 

// Create a new exponential filter with a weight of 5 and an initial value of 0. 
ExponentialFilter<long> FilterVbat(60, 0);

WiFiServer server(80);

hw_timer_t * timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

/* possible states */
typedef enum {
  SETUP = 1,
  RUNNING  
}State;

State state = SETUP;
Preferences preferences;

void IRAM_ATTR onTimer(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  isrCounter++;
  tmr_180ms++;
  tmr_240ms++;
  tmr_1seg++;
  lastIsrAt = micros();
  portEXIT_CRITICAL_ISR(&timerMux);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output
  if (tmr_180ms <= TIMER_180MS) {
     // Rinaldo digitalWrite(ledAZ1, HIGH);
     if(buzzerState){
        //Rinaldo digitalWrite(BUZZER, HIGH);             // activate beep
    }
  }
  if ((tmr_180ms == TIMER_180MS) && (tmr_180ms <= TIMER_420MS)) {
    digitalWrite(ledAZ1, LOW);
    digitalWrite(BUZZER, LOW);
  }
  else if (tmr_180ms == TIMER_420MS) {
     tmr_180ms = 0;
  }
  // Timer de 240ms
  if (tmr_240ms >= TIMER_240MS) {
    if(StateBateria == 0) digitalWrite(ledVM5, LOW);
    if(StateBateria == 1) {
      ledBateria = !ledBateria;
      digitalWrite(ledVM5, ledBateria);
      tmr_240ms = 0;
    }
  }
  // Timer de 1Segundo
  if (tmr_1seg >= TIMER_1S) {
    if(StateBateria == 2) {
      ledBateria = !ledBateria;
      digitalWrite(ledVM5, ledBateria);
    }
    // Timer de 1segundo
    tmr_1seg = 0;
    segundos++;
    if(segundos >= 60) {
      segundos = 0;
      minutos++;
    }
    if(minutos >= 60) {
      minutos = 0;
      hora++;
    }
    if(hora >= 24){
      hora = 0;
    }
  }
}

void setup() {
  uint16_t x;
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  config_hardware();
  for(x=0; x < TAM_BUF; x++) {
    data_compressao[x] = 0;
    data_respiracao[x] = 0;
    time_compressao[x] = 0;
    time_respiracao[x] = 0;
  }
  delay(100);
  Wire.begin();
  Serial.println();
  Serial.printf("Simulacare - Manequim RPC v.1.0.0");
  Serial.println();
  EEPROM.begin(64);
  load_config();
  chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf("ESP32 Chip ID = %04X",(uint16_t)(chipid>>32));//print High 2 bytes
  Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.
  temp_farenheit= temprature_sens_read();
  temp_celsius = ( temp_farenheit - 32 ) / 1.8;
  Serial.print("ESP32 Temperature: ");
  Serial.print(temp_celsius);
  Serial.println("°C");
  // Create semaphore to inform us when the timer has fired
  timerSemaphore = xSemaphoreCreateBinary();

  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  // info).
  timer = timerBegin(0, 80, true);

  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer, &onTimer, true);

  // Set Timer Interrupt every 20ms.
  // Set alarm to call onTimer function every 20ms (value in microseconds)
  // Repeat the alarm (third parameter)
  timerAlarmWrite(timer, 20000, true);
  
  // Start an alarm
  timerAlarmEnable(timer);

  // After setting up the button, setup the Bounce instance :
  switchSW1.attach(SW1);
  switchSW1.interval(25);
  // After setting up the button, setup the Bounce instance :
  switchSW2.attach(SW2);
  switchSW2.interval(25);

  mode_wifi = MODE_AP;
    
  if (mode_wifi == MODE_AP) {
    Serial.println("Configuring WIFI Server Mode AP...");
    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.softAP(ssid, password);
    Serial.print("Network: ");
    Serial.println(ssid);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("WiFi Mode AP, IP address: ");
    Serial.println(myIP);
    server.begin();                                                                                                                                                                                                           
    Serial.println("HTTP server started");
  }
  else {  
    Serial.println("Configuring WIFI Server Mode STA...");
    WiFi.begin(ssid, password);
    Serial.print("Attempting to connect to WPA network ");
    Serial.print(ssid);
    Serial.println("...");
    
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to network IP Address: ");
    Serial.println(WiFi.localIP());
    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
  }  
  Serial.println("Start Bluetooth");
  setup_BLE();
  Serial.println("Bluetooth BLE Started");
  analogReadResolution(10);
  /* setup Preferences */
  //preferences.begin("iotsharing", false);
  /* restoring state, if not exist return default SETUP sate */
  //state = (State)preferences.getUInt("state", SETUP);
}

void loop() 
{
  ledState = !ledState;
  digitalWrite(ledATV, ledState);
  Pos_Mao();
  //le_VBAT();
  le_SW3();
  if ( StartMassagem ) {
    get_compressao();
    get_respiracao();
  }  
 // Wifi_Http_Listen();
  powerdown();
}

void le_VBAT()
{
  sensorVbat = analogRead(VBAT);
  FilterVbat.Filter(sensorVbat);
  Serial.print("VBat = ");
  //Serial.print(FilterVbat.Current());
  Serial.print(sensorVbat);
  temp_farenheit= temprature_sens_read();
  temp_celsius = ( temp_farenheit - 32 ) / 1.8;
  Serial.print("\t\tESP32 Temperature: ");
  Serial.print(temp_celsius);
  Serial.println("°C");
  // Battery Voltage
  // (3.30 / 4095.0) = 0.0008058608
  //float VBattery = (float)(FilterVbat.Current() * (float)(3.30/4095.0));                // LiPo battery voltage in volts
  //Serial.print("Bateria = ");
  //Serial.println(VBattery);
  if(FilterVbat.Current() > BAT_LEVEL_MIN_1) {
    //StateBateria = 0;
  }
  if((FilterVbat.Current() <= BAT_LEVEL_MIN_1) && (FilterVbat.Current() >= BAT_LEVEL_MIN_2)) {
    //StateBateria = 1;
  }
  if(FilterVbat.Current() <= BAT_LEVEL_MIN_2) {
    //StateBateria = 2;    
  }

  delay(500);
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
  if ((micros() - lastDebounceSW3) >= debounceDelay) {
    if (readSW3 != buttonSW3State) {
      buttonSW3State = readSW3;
      // toggle State
      if (buttonSW3State == HIGH) {
        buzzerState = !buzzerState;
        Serial.print("SW3: ");
        Serial.println(buzzerState); 
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
  pinMode(ledATV, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(CS0, OUTPUT);
  pinMode(CS1, OUTPUT);
  // Analog Pins
  pinMode(POT1, INPUT_PULLUP);
  pinMode(POT2, INPUT_PULLUP);
  pinMode(VBAT, INPUT);
  // Pin Inputs (SW1,SW2,SW3)
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  
  digitalWrite(ledVM1, LOW);      // Desliga Led VM1
  digitalWrite(ledVD1, LOW);      // Desliga Led VD1
  digitalWrite(ledVM2, LOW);      // Desliga Led VM2
  digitalWrite(ledVD2, LOW);      // Desliga Led VD2
  digitalWrite(ledVM3, LOW);      // Desliga Led VM3
  digitalWrite(ledVM4, LOW);      // Desliga Led VM4
  digitalWrite(ledVM5, LOW);      // Desliga Led VM5
  digitalWrite(ledAZ1, LOW);      // Desliga Led AZ1
  digitalWrite(ledATV, LOW);      // Desliga Led Atividade
  //
  digitalWrite(CS0, HIGH);        // Selecao 0 VL6180X - Compressao Desabilitado
  digitalWrite(CS1, HIGH);        // Selecao 0 VL6180X - Respiracao Desabilitado
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
  digitalWrite(ledATV, HIGH);
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
  digitalWrite(ledATV, LOW);
  delay(500);
  Serial.println("Teste de Leds End.");
}

void Wifi_Http_Listen() {
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
          client.print("Compressao");
          client.print(" is ");
          client.print(data_compressao[0]);
          client.println("<br />");       
          //
          client.print("Respiracao");
          client.print(" is ");
          client.print(data_respiracao[0]);
          client.println("<br />");
          //
          client.print("Push-Button");
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
          client.print("Bateria");
          client.print(" is ");
          client.print(FilterVbat.Current());
          client.println("<br />");
          //
          client.print("PosMao");
          client.print(" is ");
          client.print(Count);
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
    else if (cor == COR_VERDE) {
      digitalWrite(ledVM1, LOW);
      digitalWrite(ledVD1, HIGH);
    }
    else if (cor == COR_LARANJA) {
      digitalWrite(ledVM1, HIGH);
      digitalWrite(ledVD1, HIGH);    
    }
    else {
      digitalWrite(ledVM1, LOW);
      digitalWrite(ledVD1, LOW);
    }
  }
  else if (led == LED2) {
    if (cor == COR_VERMELHO) { 
      digitalWrite(ledVM2, HIGH);
      digitalWrite(ledVD2, LOW);
    }
    else if (cor == COR_VERDE) {
      digitalWrite(ledVM2, LOW);
      digitalWrite(ledVD2, HIGH);
    }
    else if (cor == COR_LARANJA) {
      digitalWrite(ledVM2, HIGH);
      digitalWrite(ledVD2, HIGH);
    }
    else {
      digitalWrite(ledVM2, LOW);
      digitalWrite(ledVD2, LOW);
    }
  }
}

void mostra_relogio()
{
  if (millis() - lastmillis >= 1000){         /*Uptade every one second, this will be equal to reading frecuency (Hz).*/
    Serial.print("Hora: \t");
    if(hora < 10) Serial.print("0");
    Serial.print(hora);
    Serial.print(":");
    if(minutos < 10) Serial.print("0");
    Serial.print(minutos);
    Serial.print(":");
    if(segundos < 10) Serial.print("0");
    Serial.println(segundos);
    lastmillis = millis();  
  }
}

void Pos_Mao()
{
  // Update the Bounce instance :
  switchSW1.update();
  switchSW2.update();
  // Get the updated value :
  int value1 = switchSW1.read();
  int value2 = switchSW2.read();
 
  if ((value1 != LastStateButton1) || (value2 != LastStateButton2)) {
    lastDebounceSW1SW2 = millis();
  }
  if ((millis() - lastDebounceSW1SW2) >= debounceDelay2) {  
    // Retorna a ler chave depois de debounceDelay2 == 100ms
    switchSW1.update();
    switchSW2.update();
    int value1 = switchSW1.read();
    int value2 = switchSW2.read();
    
    // Determined by the state SW1 + SW2:
    if ( (value1 == LOW) && (value2 == LOW) && (flagtecla == 0)) {
      flagtecla = 3;
      StartMassagem = 1;
      lasteventsw1sw2 = millis();
      powersafe = 0;
      Count++;
      if(Count == 1) {
        // Zera Relogio
        noInterrupts();
        segundos = 0;
        minutos = 0;
        hora = 0;
        interrupts();
      }
      Serial.println("SW1+SW2 LOW");
      Serial.print("PosMao: ");
      Serial.print(MaoState);
      Serial.print("\t");
      Serial.print("Contador: ");
      Serial.print(Count);
      Serial.print("\t");
      Serial.print("Hora: \t");
      if(hora < 10) Serial.print("0");
      Serial.print(hora);
      Serial.print(":");
      if(minutos < 10) Serial.print("0");
      Serial.print(minutos);
      Serial.print(":");
      if(segundos < 10) Serial.print("0");
      Serial.print(segundos);
      Serial.println();
    } 
    else if ((value1 == HIGH) && (value2 == HIGH) && ((flagtecla == 3) || (flagtecla == 1) || (flagtecla == 2))) {
      if(flagtecla == 3) {
        StateBateria = 0;
        lasteventsw1sw2 = millis();
        powersafe = 0;
        Serial.println("SW1+SW2 HIGH");
      }
      if(flagtecla == 1) {
        lasteventsw1 = millis();
        powersafe = 0;  
        Serial.println("SW1 HIGH");
      }
      if(flagtecla == 2) {
        lasteventsw2 = millis();
        powersafe = 0;  
        Serial.println("SW2 HIGH");
      }
      flagtecla = 0;  
    }
    else if ((value1 == LOW) && (value2 == HIGH) && (flagtecla == 0) ) {
      flagtecla = 1;
      StateBateria = 1;
      lasteventsw1 = millis();
      powersafe = 0;
      Serial.println("SW1 LOW");
    }
    else if ((value2 == LOW) && (value1 == HIGH) && (flagtecla == 0)) {
      flagtecla = 2;
      StateBateria = 2;
      lasteventsw2 = millis();
      powersafe = 0;
      Serial.println("SW2 LOW");
    }
  }
  LastStateButton1 = value1;
  LastStateButton2 = value2;
}

void powerdown()
{
/*
  switch(state){
    case SETUP:
      Serial.println("SETUP state");
      state = RUNNING;
      break;
    case RUNNING:
      Serial.println("RUNNING state");
      // save current state
      preferences.putUInt("state", state);
      // Close the Preferences
      preferences.end();
      Serial.println("Bring ESP to sleep mode 3 minutos");
      ESP.deepSleep(180e7);
      break;
  }
 */ 
 if ( ((millis() - lasteventcompressao) > TIME_POWEROFF) && ((millis() - lasteventrespiracao) > TIME_POWEROFF) &&
   ((millis() - lasteventsw1sw2) > TIME_POWEROFF) && ((millis() - lasteventsw1) > TIME_POWEROFF) && ((millis() - lasteventsw2) > TIME_POWEROFF)) {
    if(powersafe == 0) {
      Serial.println("Entra em Modo Power-Save");
      powersafe = 1;  
    }
  }  
}

