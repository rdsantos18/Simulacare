#include <VL6180X.h>

extern uint16_t data_compressao[];
extern uint16_t data_respiracao[];
extern unsigned long time_respiracao[];
extern unsigned long time_compressao[];
extern unsigned long lastcompressao;
extern unsigned long lastrespiracao;
extern unsigned long lasteventcompressao;
extern unsigned long lasteventrespiracao;
extern uint8_t powersafe;
extern uint16_t qtd_compressao;
extern uint16_t qtd_respiracao;
extern uint8_t flag_compressao;
extern uint8_t flag_respiracao;
extern const int CS0;
extern const int CS1;

// Define modo de operacao
// Mode = 1 Single
// Mode = 0 Continuo
#define MODE  0

// To try different scaling factors, change the following define.
// Valid scaling factors are 1, 2, or 3.
#define SCALING 1

VL6180X compressao;
VL6180X respiracao;

void init_vl6180()
{
  digitalWrite(CS0, LOW);
  delay(50);
#if MODE == 1  
  Serial.println("VL6180X Single Mode");
  compressao.init();
  compressao.configureDefault();
  compressao.setScaling(SCALING);
  compressao.setTimeout(500);
  compressao.setAddress(0x54);
  Serial.println(compressao.readReg(0x212),HEX);        // read I2C address
#else
  Serial.println("VL6180X Continuos Mode");
  compressao.init();
  compressao.configureDefault();
  compressao.setAddress(0x54);
  Serial.println(compressao.readReg(0x212),HEX);        // read I2C address
  
  // Reduce range max convergence time and ALS integration
  // time to 30 ms and 50 ms, respectively, to allow 10 Hz
  // operation (as suggested by Table 6 ("Interleaved mode
  // limits (10 Hz operation)") in the datasheet).
  compressao.writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
  compressao.writeReg16Bit(VL6180X::SYSALS__INTEGRATION_PERIOD, 50);

  compressao.setTimeout(500);

   // stop continuous mode if already active
  compressao.stopContinuous();
  compressao.setScaling(SCALING);
  // in case stopContinuous() triggered a single-shot
  // measurement, wait for it to complete
  delay(300);
  // start interleaved continuous mode with period of 100 ms
  compressao.startInterleavedContinuous(100);
#endif

  // Respiracao
  digitalWrite(CS1, LOW);
  delay(50);
#if MODE == 1  
  respiracao.init();
  respiracao.configureDefault();
  respiracao.setScaling(SCALING);
  respiracao.setTimeout(500);
  respiracao.setAddress(0x29);
  Serial.println(respiracao.readReg(0x212),HEX);        // read I2C address
#else
  respiracao.init();
  respiracao.configureDefault();
  respiracao.setAddress(0x29);
  Serial.println(respiracao.readReg(0x212),HEX);        // read I2C address

  // Reduce range max convergence time and ALS integration
  // time to 30 ms and 50 ms, respectively, to allow 10 Hz
  // operation (as suggested by Table 6 ("Interleaved mode
  // limits (10 Hz operation)") in the datasheet).
  respiracao.writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
  respiracao.writeReg16Bit(VL6180X::SYSALS__INTEGRATION_PERIOD, 50);

  respiracao.setTimeout(500);

   // stop continuous mode if already active
  respiracao.stopContinuous();
  respiracao.setScaling(SCALING);
  // in case stopContinuous() triggered a single-shot
  // measurement, wait for it to complete
  delay(300);
  // start interleaved continuous mode with period of 100 ms
  respiracao.startInterleavedContinuous(100);
#endif
}

void get_compressao()
{
  unsigned long tst;
  unsigned long tempo;

  tempo = millis();
  
  digitalWrite(CS0, LOW);
  delay(1);
#if MODE == 1  
  compressao.init();
  compressao.configureDefault();
  compressao.setScaling(SCALING);
  compressao.setTimeout(500);
#else
  compressao.init();
  compressao.configureDefault();

  // Reduce range max convergence time and ALS integration
  // time to 30 ms and 50 ms, respectively, to allow 10 Hz
  // operation (as suggested by Table 6 ("Interleaved mode
  // limits (10 Hz operation)") in the datasheet).
  compressao.writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
  compressao.writeReg16Bit(VL6180X::SYSALS__INTEGRATION_PERIOD, 50);

  compressao.setTimeout(500);

   // stop continuous mode if already active
  compressao.stopContinuous();
  // in case stopContinuous() triggered a single-shot
  // measurement, wait for it to complete
  delay(300);
  // start interleaved continuous mode with period of 100 ms
  compressao.startInterleavedContinuous(100);
#endif
 
  delay(1);

#if MODE == 1
  data_compressao[qtd_compressao] = compressao.readRangeSingleMillimeters();
#else
  data_compressao[qtd_compressao] = compressao.readRangeContinuousMillimeters();
#endif
    
  if(flag_compressao == 0) {
    flag_compressao = 1;
    lastcompressao = tempo;
    time_compressao[qtd_compressao] = 0;
  }
  else {
    tst = tempo - lastcompressao;
    lastcompressao = tempo;
    time_compressao[qtd_compressao] = tst;
  }  
  // Debug
  Serial.print("Compressao: ");
  Serial.print( data_compressao[qtd_compressao] );
  Serial.print(" mm\t");
  Serial.print("Time: ");
  Serial.print( time_compressao[qtd_compressao] );
  //Serial.println();
  //
  if (compressao.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  digitalWrite(CS0, HIGH);
  
  qtd_compressao++;
  if(qtd_compressao > 1500) qtd_compressao = 0;
  lasteventcompressao = tempo;
  powersafe = 0;
  delay(1);
}

void get_respiracao()
{
  unsigned long tst;
  unsigned long tempo;

  tempo = millis();
  
  // Respiracao
  digitalWrite(CS1, LOW);
  delay(1);
#if MODE == 1  
  respiracao.init();
  respiracao.configureDefault();
  respiracao.setScaling(SCALING);
  respiracao.setTimeout(500);
#else
  respiracao.init();
  respiracao.configureDefault();

  // Reduce range max convergence time and ALS integration
  // time to 30 ms and 50 ms, respectively, to allow 10 Hz
  // operation (as suggested by Table 6 ("Interleaved mode
  // limits (10 Hz operation)") in the datasheet).
  respiracao.writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
  respiracao.writeReg16Bit(VL6180X::SYSALS__INTEGRATION_PERIOD, 50);

  respiracao.setTimeout(500);

   // stop continuous mode if already active
  respiracao.stopContinuous();
  // in case stopContinuous() triggered a single-shot
  // measurement, wait for it to complete
  delay(300);
  // start interleaved continuous mode with period of 100 ms
  respiracao.startInterleavedContinuous(100);
#endif
 
  delay(1);

#if MODE == 1
  data_respiracao[qtd_respiracao] = respiracao.readRangeSingleMillimeters();
#else
  data_respiracao[qtd_respiracao] = respiracao.readRangeContinuousMillimeters();
#endif
  
  if(flag_respiracao == 0) {
    flag_respiracao = 1;
    lastrespiracao = tempo;
    time_respiracao[qtd_respiracao] = 0;
  }
  else {
    tst = tempo - lastrespiracao;
    lastrespiracao = tempo;
    time_respiracao[qtd_respiracao] = tst;
  }
  // Debug
  Serial.print("\tRespiracao: ");
  Serial.print( data_respiracao[qtd_respiracao] );
  Serial.print(" mm\t");
  Serial.print("Time: ");
  Serial.print( time_respiracao[qtd_respiracao] );
  Serial.println();
  //
  if (respiracao.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  digitalWrite(CS1, HIGH);
  
  qtd_respiracao++;
  if(qtd_respiracao > 1500) qtd_respiracao = 0;
  lasteventrespiracao = tempo;
  powersafe = 0;
}

void read_compressao()
{
  unsigned long tst;
  unsigned long tempo;

  tempo = millis();
  
#if MODE == 1
  data_compressao[qtd_compressao] = compressao.readRangeSingleMillimeters();
#else
  data_compressao[qtd_compressao] = compressao.readRangeContinuousMillimeters();
#endif

  if(flag_compressao == 0) {
    flag_compressao = 1;
    lastcompressao = tempo;
    time_compressao[qtd_compressao] = 0;
  }
  else {
    tst = tempo - lastcompressao;
    lastcompressao = tempo;
    time_compressao[qtd_compressao] = tst;
  }  
  // Debug
  Serial.print("\tCompressao: ");
  Serial.print( data_compressao[qtd_compressao] );
  Serial.print(" mm\t");
  Serial.print("Time: ");
  Serial.print( time_compressao[qtd_compressao] );
  Serial.println();
  //
  if (compressao.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  
  qtd_compressao++;
  if(qtd_compressao > 1500) qtd_compressao = 0;
  lasteventcompressao = tempo;
  powersafe = 0;
}

void read_respiracao()
{
  unsigned long tst;
  unsigned long tempo;
  
  tempo = millis();
  
#if MODE == 1
  data_respiracao[qtd_respiracao] = respiracao.readRangeSingleMillimeters();
#else
  data_respiracao[qtd_respiracao] = respiracao.readRangeContinuousMillimeters();
#endif


  if(flag_respiracao == 0) {
    flag_respiracao = 1;
    lastrespiracao = tempo;
    time_respiracao[qtd_respiracao] = 0;
  }
  else {
    tst = tempo - lastrespiracao;
    lastrespiracao = tempo;
    time_respiracao[qtd_respiracao] = tst;
  }  
  // Debug
  Serial.print("Respiracao: ");
  Serial.print( data_respiracao[qtd_respiracao] );
  Serial.print(" mm\t");
  Serial.print("Time: ");
  Serial.print( time_respiracao[qtd_respiracao] );
  //Serial.println();
  //
  if (respiracao.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  
  qtd_respiracao++;
  if(qtd_respiracao > 1500) qtd_respiracao = 0;
  lasteventrespiracao = tempo;
  powersafe = 0;
}

