#include <SparkFun_VL6180X.h>

extern unsigned int hasteA;
extern unsigned int hasteB;
extern const int CS0;
extern const int CS1;

#define VL6180X_ADDRESS 0x29

VL6180xIdentification identification;
VL6180x sensor(VL6180X_ADDRESS);

void printIdentification(struct VL6180xIdentification *temp){
  Serial.print("VL6180X Model ID = ");
  Serial.println(temp->idModel);

  Serial.print("Model Rev = ");
  Serial.print(temp->idModelRevMajor);
  Serial.print(".");
  Serial.println(temp->idModelRevMinor);

  Serial.print("Module Rev = ");
  Serial.print(temp->idModuleRevMajor);
  Serial.print(".");
  Serial.println(temp->idModuleRevMinor);  

  Serial.print("Manufacture Date = ");
  Serial.print((temp->idDate >> 3) & 0x001F);
  Serial.print("/");
  Serial.print((temp->idDate >> 8) & 0x000F);
  Serial.print("/1");
  Serial.print((temp->idDate >> 12) & 0x000F);
  Serial.print(" Phase: ");
  Serial.println(temp->idDate & 0x0007);

  Serial.print("Manufacture Time (s)= ");
  Serial.println(temp->idTime * 2);
}

void Init_VL6180()
{
  digitalWrite(CS0, LOW);
  sensor.getIdentification(&identification); // Retrieve manufacture info from device memory
  printIdentification(&identification); // Helper function to print all the Module information
  if(sensor.VL6180xInit() != 0){
    Serial.println("VL6180X FAILED TO INITALIZE"); //Initialize device and check for errors
  }; 

  sensor.VL6180xDefautSettings(); //Load default settings to get started.
  
  delay(1000); // delay 1s
}

void get_distance()
{
  //Get Distance and report in mm
  Serial.print("Distance measured (mm) = ");
  Serial.println( sensor.getDistance() ); 

  delay(500); 
}

void get_haste()
{
  //Get Distance and report in mm
  //Serial.print("Distance measured (mm) = ");
  //Serial.println( sensor.getDistance() ); 
  hasteA = sensor.getDistance();
  delay(500); 
}

