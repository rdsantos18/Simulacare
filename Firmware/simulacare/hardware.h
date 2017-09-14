#define COR_VERMELHO  0x00
#define COR_VERDE     0x01
#define COR_LARANJA   0x02
#define COR_APAGADO   0x03
#define LED1          0x01
#define LED2          0x02

// Leds
const int ledVM1 = 32;      // Led Falha Respiração Verde Correto; Laranja Excessivo; Vermelho
const int ledVD1 = 33;      // Led Falha Respiração
const int ledVM2 = 25;      // Led Profundidade Compressao
const int ledVD2 = 26;      // Led Profundidade Compressao Verde - Correto; Laranja Excessivo; Vermelho
const int ledVM3 = 27;      // Led Posição Incorreta das Maos
const int ledVM4 = 14;      // Led Insuflação Muito Rapida
const int ledAZ1 = 12;      // Led de Ritmo
const int ledVM5 = 23;      // Led Bateria Baixa
const int ledATV = 02;      // Led Atividade

// Entradas Analogicas
const int POT1 = 34;
const int POT2 = 39;
const int VBAT = 36;        // Entrada Medicao Bateria (2.6V...4.2V)

// Entrada Digitais Chaves
const int SW1 = 17;
const int SW2 = 16;
const int SW3 = 35;

// Buzzer
const int BUZZER = 4;

// Selecao VL6180x
const int CS0 = 19;
const int CS1 = 18;

