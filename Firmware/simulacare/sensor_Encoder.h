//#define ENCODER_DO_NOT_USE_INTERRUPTS
//#include <Encoder.h>

extern const int ENC1_A;
extern const int ENC1_B;
extern const int ENC2_A;
extern const int ENC2_B;

//Encoder encoderA(ENC1_A, ENC1_B);
//Encoder encoderB(ENC2_A, ENC2_B);

//long positionEnc1 = -999;
//long positionEnc2 = -999;

//void le_Encoder()
//{
//  long newPosA, newPosB;
//  newPosA = encoderA.read();
//  newPosB = encoderB.read();
//  if (newPosA != positionEnc1 || newPosB != positionEnc2) {
//    Serial.print("Encoder1 = ");
//    Serial.print(newPosA);
//    Serial.print(", Encoder2 = ");
//    Serial.print(newPosB);
//    Serial.println();
//    positionEnc1 = newPosA;
//    positionEnc2 = newPosB;
//  }
//}

