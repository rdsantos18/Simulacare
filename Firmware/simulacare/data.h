#define DEBUG 1
#define TIMER_180MS 9
#define TIMER_420MS 30
#define TIMER_1S    50
#define TIMER_240MS 12
#define TAM_BUF 1500
#define MODE_AP 0x00
#define MODE_STA 0x01
#define TIME_POWEROFF 30000        // 30 segundos em modo Power-Safe
#define BUG_ADC 1.87               // Fator Correcao A/D Bateria
#define BUG_ADC_INT 177
#define BAT_LEVEL_MIN_1 2233      // Level 3.60V
#define BAT_LEVEL_MIN_2 2109      // Level 3.40V

//
int SwitchState = HIGH;
uint8_t mode_wifi = 0;
uint8_t powersafe = 0;
uint8_t lastpowersafe = 0;
float VBattery = 0.00;       // battery voltage from ESP32 ADC read
uint64_t chipid;  
/* Set these to your desired credentials. */
const char *ssid = "SimulaCare-RPC";
const char *password = "12345678";

volatile int ButtonCount = 0;
int Count = 0;
unsigned long lastmillis = 0;
unsigned long lasteventsw1 = 0;
unsigned long lasteventsw2 = 0;
unsigned long lasteventsw1sw2 = 0;
unsigned long lasteventcompressao = 0;
unsigned long lasteventrespiracao = 0;

// Variaveis Analogicas
uint16_t sensorVbat = 0;

// Variaveis SW
// SW1
int SW1State = HIGH;
// SW2
int SW2State = HIGH;
// SW3
int buzzerState = HIGH;
int buttonSW3State;
int lastSW3State = LOW;
unsigned long lastDebounceSW3 = 0;
// PosMao
int MaoState = HIGH;
int buttonMaoState1;
int buttonMaoState2;
int lastMaoState1 = LOW;
int lastMaoState2 = LOW;
unsigned long lastDebounceMao = 0;
unsigned long lastDebounceSW1SW2 = 0;
unsigned long debounceDelay = 20;
unsigned long debounceDelay2 = 50;
unsigned int flagtecla = 3;
unsigned int StartMassagem = 0;

int keyIndex = 0;
int wifi_status = WL_IDLE_STATUS;

volatile uint32_t isrCounter = 0;
volatile uint32_t lastIsrAt = 0;
volatile uint32_t tmr_180ms = 0;
volatile uint32_t tmr_1seg = 0;
volatile uint32_t tmr_240ms = 0;
volatile uint8_t StateBateria = 0;
volatile int ledState = HIGH;
volatile int ledBateria = HIGH;
int LastStateButton1 = 0;
int LastStateButton2 = 0;
int LastStateButton3 = 0;
int LastStateButton4 = 0;
uint16_t EstadoChave = 0;

volatile int segundos = 0;
volatile int minutos = 0;
volatile int hora = 0;

uint16_t data_compressao[TAM_BUF];
unsigned long time_compressao[TAM_BUF];
unsigned long lastcompressao = 0;
uint16_t data_respiracao[TAM_BUF];
unsigned long time_respiracao[TAM_BUF];
unsigned long lastrespiracao = 0;
uint8_t flag_compressao = 0;
uint8_t flag_respiracao = 0;
uint16_t qtd_compressao = 0;
uint16_t qtd_respiracao = 0;
uint8_t flag_teste = 0;
unsigned long lastteste = 0;
unsigned long time_teste = 0;

uint16_t min_resp = 0;
uint16_t max_resp = 10;
uint16_t min_compre = 0;
uint16_t max_compre = 10;

uint8_t temp_farenheit;
float temp_celsius;

