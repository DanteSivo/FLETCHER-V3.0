/******************************************************************************
Hardware_Interrupts.ino

https://github.com/sparkfun/SparkFun_Qwiic_6DoF_LSM6DSO
https://github.com/sparkfun/SparkFun_Qwiic_6DoF_LSM6DSO_Arduino_Library

Description:
This examples gets acclerometer data when an interrupt is deteced from "INT1"
and gyroscopic data when an interrupt is detected from "INT2". These two
interrupts can be put anywhere on your dev board, but for this example pins
two and three were used for "INT1" and INT2" respectively.

Interrupts can be changed to be active low: 
myIMU.configHardOutInt(INT_ACTIVE_LOW);
Interrupts are being configured automatically like so: 
myIMU.setInterruptOne(INT1_DRDY_XL_ENABLED);
myIMU.setInterruptTwo(INT2_DRDY_G_ENABLED);

Development environment tested:
Arduino IDE 1.8.2

This code is released under the [MIT License](http://opensource.org/licenses/MIT).
Please review the LICENSE.md file included with this example. If you have any questions 
or concerns with licensing, please contact techsupport@sparkfugn.com.
Distributed as-is; no warranty is given.
******************************************************************************/
#include "SparkFunLSM6DSO.h"

int accelInt = 4; 
int gyroInt = 2; 


LSM6DSO myIMU; //Default constructor is I2C, addr 0x6B

void setup() {

  pinMode(accelInt, INPUT);
  pinMode(gyroInt, INPUT);

  Serial.begin(115200);
  delay(500); 

  Wire.begin();
  delay(10);
  if( myIMU.begin() )
    Serial.println("Ready.");
  else { 
    Serial.println("Could not connect to IMU.");
    Serial.println("Freezing");
  }

  if( myIMU.initialize(BASIC_SETTINGS) )
    Serial.println("Loaded Settings.");

}


void loop()
{
  //Get all parameters
  if (digitalRead(accelInt)) {
    Serial.print("\nAccelerometer:\n");
    Serial.print(" X = ");
    Serial.println(myIMU.readFloatAccelX(), 3);
    Serial.print(" Y = ");
    Serial.println(myIMU.readFloatAccelY(), 3);
    Serial.print(" Z = ");
    Serial.println(myIMU.readFloatAccelZ(), 3);
  }

  if (digitalRead(gyroInt)) {
  Serial.print("\nGyroscope:\n");
  Serial.print(" X = ");
  Serial.println(myIMU.readFloatGyroX(), 3);
  Serial.print(" Y = ");
  Serial.println(myIMU.readFloatGyroY(), 3);
  Serial.print(" Z = ");
  Serial.println(myIMU.readFloatGyroZ(), 3);
  }
  
  Serial.print("\nThermometer:\n");
  Serial.print(" Degrees F = ");
  Serial.println(myIMU.readTempF(), 3);

  delay(1000);
}
