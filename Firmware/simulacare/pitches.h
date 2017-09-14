/*************************************************
 * Public Constants
 *************************************************/
#define se 6
#define tr 4
#define co 3
#define SMbpm 150

//
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

int tonesA[] = {NOTE_E5, NOTE_E5,0,NOTE_E5,0, NOTE_C5, NOTE_E5,0, NOTE_G5,0,0, NOTE_G4,0,0};
int tonesB[] = {NOTE_C5,0,0, NOTE_G4,0,NOTE_E4,0,0,NOTE_A4,0, NOTE_B4,0,NOTE_AS4, NOTE_A4,0,NOTE_G4,NOTE_E5,NOTE_G5,NOTE_A5,0,NOTE_F5,NOTE_G5,0,NOTE_E5,0,NOTE_C5,NOTE_D5,NOTE_B4,0};
int tonesC[] = {NOTE_C3,0,NOTE_G5,NOTE_FS5,NOTE_F5,NOTE_DS5,NOTE_C4,NOTE_E5,NOTE_F3,NOTE_GS4,NOTE_A4,NOTE_C5,NOTE_C4,NOTE_A4,NOTE_C5,NOTE_D5};
int tonesD[] = {NOTE_C3,0,NOTE_G5,NOTE_FS5,NOTE_F5,NOTE_DS5,NOTE_G3,NOTE_E5,0,NOTE_C6,0,NOTE_C6,NOTE_C6,0,NOTE_G3,0};
int tonesE[] = {NOTE_C3,0,NOTE_DS5,0,0,NOTE_D5,0,NOTE_C5,0,0,NOTE_G3,NOTE_G3,0,NOTE_C3,0};
int tonesF[] = {NOTE_C5,NOTE_C5,0,NOTE_C5,0,NOTE_C5,NOTE_D5,0,NOTE_E5,NOTE_C5,0,NOTE_A4,NOTE_G4,0,NOTE_G2,0};
int tonesG[] = {NOTE_C5,NOTE_C5,0,NOTE_C5,0,NOTE_C5,NOTE_D5,NOTE_E5,NOTE_F3,0,0,NOTE_C3,0,NOTE_G2,0};
int tonesH[] = {NOTE_E5,NOTE_E5,0,NOTE_E5,0,NOTE_C5,NOTE_E5,0,NOTE_G5,0,0,NOTE_G4,0,0};
int tonesI[] = {NOTE_E5,NOTE_C5,0,NOTE_G4,NOTE_G3,0,NOTE_GS4,0,NOTE_A4,NOTE_F5,NOTE_F3,NOTE_F5,NOTE_A4,NOTE_C4,NOTE_F3,0};
int tonesJ[] = {NOTE_B4,NOTE_A5,NOTE_A5,NOTE_A5,NOTE_G5,NOTE_F5,NOTE_E5,NOTE_C5,NOTE_G3,NOTE_A4,NOTE_G4,NOTE_C4,NOTE_G3,0};
int tonesK[] = {NOTE_B4,NOTE_F5,0,NOTE_F5,NOTE_F5,NOTE_E5,NOTE_D5,NOTE_C5,NOTE_E4,NOTE_G3,NOTE_E4,NOTE_C4,0,0};

int durationA[] = {co,co,co,co,co,co,co,co,co,co,se,co,co,se};
int durationB[] = {co,co,co,co,se,co,co,co,co,co,co,co,co,co,co,tr,tr,tr,co,co,co,co,co,co,co,co,co,co,se};
int durationC[] = {co,co,co,co,co,co,co,co,co,co,co,co,co,co,co,co};
int durationD[] = {co,co,co,co,co,co,co,co,co,co,co,co,co,co,co,co};
int durationE[] = {co,co,co,co,co,co,se,co,co,co,co,co,co,co,co};
int durationF[] = {co,co,co,co,co,co,co,co,co,co,co,co,co,co,co,co};
int durationG[] = {co,co,co,co,co,co,co,co,co,co,co,co,se,co,co};
int durationH[] = {co,co,co,co,co,co,co,co,co,co,se,co,co,se};
int durationI[] = {co,co,co,co,co,co,co,co,co,co,co,co,co,co,co,co};
int durationJ[] = {tr,tr,tr,tr,tr,tr,co,co,co,co,co,co,co,co};
int durationK[] = {co,co,co,co,tr,tr,tr,co,co,co,co,co,co,se};

char melody[] = "abbcdcecdcefgfhbbijikijikfgfhijik";
 
// Número de notas
int numberA = 14;
int numberB = 29;
int numberC = 16;
int numberD = 16;
int numberE = 15;
int numberF = 16;
int numberG = 15;
int numberH = 14;
int numberI = 16;
int numberJ = 14;
int numberK = 14;
int melodynum = 33;

extern const int BUZZER;

// Função que recebe uma faixa (notas e duraçõess) e o número de notas
void play(int tones[], int duration[], int number, int bpm){
  int i;
 
  for (i = 0; i < number; i++) {
 
    int lenght = bpm*duration[i];
    // Rinaldo tone(BUZZER, tones[i],lenght);
 
    // Pausas para que as notas nÃ£o fiquem coladas umas Ã s outras
    if(duration[i]==se)
      delay(bpm/0.5);
    if(duration[i]==tr)
      delay(bpm/.75);
    if(duration[i]==co)
      delay(bpm);
 
    // PÃ¡ra de tocar a nota
    //Rinaldo noTone(BUZZER);
   }
}
 
// Função que recebe uma string que contÃ©m as faixas para criar uma música (e o seu tamanho)
void playall(char melody[], int num){
 
  int i;
 
  for(i=0;i < num; i++) {
    if(melody[i]=='a')
      play(tonesA, durationA, numberA, SMbpm);
    if(melody[i]=='b')
      play(tonesB, durationB, numberB, SMbpm);
    if(melody[i]=='c')
      play(tonesC, durationC, numberC, SMbpm);
    if(melody[i]=='d')
      play(tonesD, durationD, numberD, SMbpm);
    if(melody[i]=='e')
      play(tonesE, durationE, numberE, SMbpm);
    if(melody[i]=='f')
      play(tonesF, durationF, numberF, SMbpm);
    if(melody[i]=='g')
      play(tonesG, durationG, numberG, SMbpm);
    if(melody[i]=='h')
      play(tonesH, durationH, numberH, SMbpm);
    if(melody[i]=='i')
      play(tonesI, durationI, numberI, SMbpm);
    if(melody[i]=='j')
      play(tonesJ, durationJ, numberJ, SMbpm);
    if(melody[i]=='k')
      play(tonesK, durationK, numberK, SMbpm);
    }
}
 
