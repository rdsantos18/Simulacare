#include <VL6180X.h>

extern unsigned int data_compressao;
extern unsigned int data_respiracao;
extern const int CS0;
extern const int CS1;

// To try different scaling factors, change the following define.
// Valid scaling factors are 1, 2, or 3.
#define SCALING 2

VL6180X compressao;
VL6180X respiracao;

void Init_ToF()
{
  digitalWrite(CS0, LOW);
  compressao.init();
  compressao.configureDefault();
  compressao.setScaling(SCALING);
  compressao.setTimeout(500);
  digitalWrite(CS0, HIGH);
  //
  //digitalWrite(CS1, LOW);
  //respiracao.init();
  //respiracao.configureDefault();
  //respiracao.setScaling(SCALING);
  //respiracao.setTimeout(500);
  //digitalWrite(CS1, HIGH);
}

void get_compressao()
{
  digitalWrite(CS0, LOW);
  //Serial.print(compressao.getScaling());
  //Serial.print("x) ");
  Serial.print("Compressao: ");

  data_compressao = compressao.readRangeSingleMillimeters();
  Serial.print( data_compressao );
  if (compressao.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  digitalWrite(CS0, HIGH);
  
  Serial.println(" mm");

//  delay(300);
}

void get_respiracao()
{
  digitalWrite(CS1, LOW);
  //Serial.print(respiracao.getScaling());
  //Serial.print("x) ");
  Serial.print("Respiracao: ");

  data_respiracao = respiracao.readRangeSingleMillimeters();
  Serial.print( data_respiracao );
  if (respiracao.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  digitalWrite(CS1, HIGH);
  
  Serial.println(" mm");

//  delay(300);
}

