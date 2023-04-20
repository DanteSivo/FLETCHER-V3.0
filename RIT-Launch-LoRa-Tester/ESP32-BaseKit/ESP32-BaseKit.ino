 /*********
  Modified from the examples of the Arduino LoRa library
  More resources: https://randomnerdtutorials.com
*********/

#include <SPI.h>
#include <LoRa.h>
#include "BluetoothSerial.h"

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2

#define BLE true

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

String command;
String var;
String val;

char buffer[7];
int PW = 0; // current TX-PW rating
int SF = 12; // current SF
long BW = 150E3; // current BW
int LG = 6; // LNA Gain

BluetoothSerial SerialBT;

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

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  SerialBT.begin("RIT-BaseKit-RX"); //Bluetooth device name
  while (!Serial);
  println("LoRa Receiver");

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
 
 //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(915E6)) {
    println(".");
    delay(500);
  }

  LoRa.setSpreadingFactor(SF);
  LoRa.setSignalBandwidth(BW);
  LoRa.setGain(LG);
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  LoRa.dumpRegisters(Serial);
  LoRa.dumpRegisters(SerialBT);
  println("LoRa Initializing OK!");
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

void loop() {
    if (Serial.available()) {
      updateRegisters(Serial);
    } else if (SerialBT.available()) {
       updateRegisters(SerialBT);
    }
    else {
      // try to parse packet
      int packetSize = LoRa.parsePacket();
      if (packetSize) {
        // received a packet
        Serial.print("Received packet '");
    
        // read packet
        while (LoRa.available()) {
          String LoRaData = LoRa.readString();
          print(LoRaData); 
        }
    
        // print RSSI of packet
        print("' with RSSI ");
        println(itoa(LoRa.packetRssi(), buffer, 10));
      }
  }
}
