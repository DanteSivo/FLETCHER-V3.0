/*
  Reading lat and long via UBX binary commands - no more NMEA parsing!
  By: Nathan Seidle
  SparkFun Electronics
  Date: January 3rd, 2019
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  This example shows how to query a u-blox module for its lat/long/altitude. We also
  turn off the NMEA output on the I2C port. This decreases the amount of I2C traffic 
  dramatically.

  Note: Long/lat are large numbers because they are * 10^7. To convert lat/long
  to something google maps understands simply divide the numbers by 10,000,000. We 
  do this so that we don't have to use floating point numbers.

  Leave NMEA parsing behind. Now you can simply ask the module for the datums you want!

  Feel like supporting open source hardware?
  Buy a board from SparkFun!
  ZED-F9P RTK2: https://www.sparkfun.com/products/15136
  NEO-M8P RTK: https://www.sparkfun.com/products/15005
  SAM-M8Q: https://www.sparkfun.com/products/15106

  Hardware Connections:
  Plug a Qwiic cable into the GNSS and a BlackBoard
  If you don't have a platform with a Qwiic connection use the SparkFun Qwiic Breadboard Jumper (https://www.sparkfun.com/products/14425)
  Open the serial monitor at 115200 baud to see the output
*/

#include <Wire.h> //Needed for I2C to GNSS
#include <SPI.h>
#include <LoRa.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

String command;
String var;
String val;

char buffer[7];
// EDIT ME
#define BLE true

int PW = 20; // current TX-PW rating
int SF = 12; // current SF
long BW = 500E3; // current BW
int LG = 6; // LNA Gain
// END EDIT ME
BluetoothSerial SerialBT;

// #include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
// SFE_UBLOX_GNSS myGNSS;

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2



long lastTime = 0; //Simple local timer. Limits amount if I2C traffic to u-blox module.
int counter = 0;

void print(String msg){
  Serial.print(msg);
  if (BLE)
    SerialBT.print(msg);
}

void println(String msg){
  Serial.println(msg);
  if (BLE)
    SerialBT.println(msg);
}

void setup()
{
  Serial.begin(115200);
  while (!Serial); //Wait for user to open terminal
  Serial.println("SparkFun u-blox Example");
  
  SerialBT.begin("RIT-FieldKit-TX"); //Bluetooth device name
  
  Wire.begin();
/*
  //myGNSS.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial

  if (myGNSS.begin() == false) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  }

  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR
*/
  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(915E6)) {
    Serial.println(".");
    delay(500);
  }

  LoRa.setSpreadingFactor(SF);
  LoRa.setSignalBandwidth(BW);
  LoRa.setTxPower(PW);
  LoRa.setGain(LG);
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  LoRa.dumpRegisters(Serial);
  LoRa.dumpRegisters(SerialBT);
  Serial.println("LoRa Initializing OK!");
  SerialBT.println("LoRa Initializing OK!");
}

void updateRegisters(Stream& in) {
    command = in.readStringUntil('\n');
    command.trim();
    var = command.substring(0,2);
    val = command.substring(3);
   
    print("Command: ");
    println(command);
    
    print("Got Command: ");
    println(var);
    
    print("Got Value: ");
    println(val);

    if(var.equals("PW")){ // Adjust TX Power
      print("PW=");
      itoa(PW, buffer, 10);
      print(buffer);
      print(" is now ");
      println(val);
      PW = val.toInt();
    } else if(var.equals("SF")){ // Adjust Spreading Factor
      print("SF=");
      itoa(SF, buffer, 10);
      print(buffer);
      print(" is now ");
      println(val);
      SF = val.toInt();
      LoRa.setSpreadingFactor(SF);
    } else if(var.equals("BW")){   // Adjust Bandwidth
      print("BW=");
      ltoa(BW, buffer, 10);
      print(buffer);
      print(" is now ");
      println(val);
      BW = val.toInt();
      LoRa.setSignalBandwidth(BW);
    } else if (var.equals("LG")) {
      print("LG=");
      itoa(LG, buffer, 10);
      print(buffer);
      print(" is now ");
      println(val);
      LG = val.toInt();
      LoRa.setGain(LG);
    } else if (var.equals("RR")) { // Dump Register contents
      println("Dumpuing register contents");
      LoRa.dumpRegisters(Serial);
      LoRa.dumpRegisters(SerialBT);
      print("BW=");
      ltoa(BW, buffer, 10);
      print(buffer);
      print(" SF=");
      itoa(SF, buffer, 10);
      print(buffer);
      print(" LG=");
      itoa(LG, buffer, 10);
      print(buffer);
      print(" PW=");
      itoa(PW, buffer, 10);
      println(buffer);
    }
      else {
      Serial.println("Command not recognizied");
    }
}

void loop()
{
  if (Serial.available()) {
      updateRegisters(Serial);
  } else if (SerialBT.available()) {
     updateRegisters(SerialBT);
  }
  else {
    /*
    //Query module only every second. Doing it more often will just cause I2C traffic.
    //The module only responds when a new position is available
    if (millis() - lastTime > 1000)
    {
    */
      LoRa.beginPacket();
      /*
      lastTime = millis(); //Update the timer
    
      uint8_t GMThour = myGNSS.getHour();
      if (GMThour > 4) {
        GMThour = GMThour - 4;
        Serial.print(GMThour);
        LoRa.print(GMThour);
        SerialBT.print(GMThour);
      } else {
        GMThour = 24-(4-GMThour);
        Serial.print(GMThour);
        LoRa.print(GMThour);
        SerialBT.print(GMThour);
      }
      Serial.print(":");
      LoRa.print(":");
      SerialBT.print(":");
    
      Serial.print(myGNSS.getMinute());
      LoRa.print(myGNSS.getMinute());
      SerialBT.print(myGNSS.getMinute());
      Serial.print(":");
      SerialBT.print(":");
      LoRa.print(":");
      
      Serial.print(myGNSS.getSecond());
      LoRa.print(myGNSS.getSecond());
      SerialBT.print(myGNSS.getSecond());
      Serial.print(":");
      SerialBT.print(":");
      LoRa.print(":");
      
      int32_t latitude = myGNSS.getLatitude();
      Serial.print(F(" Lat: "));
      Serial.print(latitude);
      SerialBT.print(F(" Lat: "));
      SerialBT.print(latitude);
      LoRa.print(F(" Lat: "));
      LoRa.print(latitude);
    
      int32_t longitude = myGNSS.getLongitude();
      Serial.print(F(" Long: "));
      Serial.println(longitude);
      Serial.print(F(" (degrees * 10^-7)"));
      LoRa.print(F(" Long: "));
      LoRa.println(longitude);
      LoRa.print(F(" (degrees * 10^-7)"));
      SerialBT.print(F(" Long: "));
      SerialBT.println(longitude);
      SerialBT.print(F(" (degrees * 10^-7)"));
    
      int32_t altitude = myGNSS.getAltitude();
      Serial.print(F(" Alt: "));
      Serial.print(altitude);
      Serial.print(F(" (mm)"));
      
    
      uint8_t SIV = myGNSS.getSIV();
      Serial.print(F(" SIV: "));
      Serial.print(SIV);
      LoRa.print(F(" SIV: "));
      LoRa.print(SIV);
      SerialBT.print(F(" SIV: "));
      SerialBT.print(SIV);
      */
      LoRa.print("The quick brown fox jumps over the lazy dog - ");
      LoRa.print(counter);
      print("The quick brown fox jumps over the lazy dog - ");
      Serial.println(counter);
      SerialBT.println(counter);
      LoRa.endPacket();
      counter++;
   }
}
