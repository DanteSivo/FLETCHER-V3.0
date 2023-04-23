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

unsigned long startTime = 0;
unsigned long endTime = 0;
unsigned long deltaT = 0;
float bitrate = 0;

float avgBitrate = 0;
unsigned long totalPacketsRecieved = 0;
unsigned long avgDeltaT = 0;

char buffer[7];
int PW = 0; // current TX-PW rating
int SF = 12; // current SF
long BW = 500E3; // current BW
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

  startTime = millis();
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
      print("Packets Recieved=");
      utoa(totalPacketsRecieved, buffer, 10);
      println(buffer);
      print("Average Time Delta=");
      utoa(avgDeltaT, buffer, 10);
      print(buffer);
      println("ms");
      print("Average Bitrate=");
      print(String(avgBitrate));
      println("bps");
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
        
        endTime = millis(); // GOT A PACKET! Timestamp.
        // received a packet
        Serial.print("Received packet '");
        
        // read packet
        while (LoRa.available()) {
          String LoRaData = LoRa.readString();
          print(LoRaData); 
        }
        println("'");

    
        // print RSSI of packet
        print("RSSI: ");
        print(itoa(LoRa.packetRssi(), buffer, 10));

        // print packet size
        print(" SIZE: ");
        print(itoa(packetSize*8, buffer, 10));
        print("bits");

        deltaT = (endTime - startTime);
        // Calculate new average time
        if (avgDeltaT != 0) {
          avgDeltaT = ((avgDeltaT*totalPacketsRecieved)+deltaT)/(totalPacketsRecieved+1);
        } else {
          avgDeltaT = deltaT;
        }
        print(" TIME:");
        print(utoa(deltaT, buffer, 10));
        print("ms ");
        startTime = endTime;
        

        if (deltaT > 0) {
          bitrate = ((float)packetSize*8)/((float)deltaT/1000);
          print(" BITRATE:");
          print(String(bitrate));
          print("bps ");

          if (bitrate != 0) {
            avgBitrate = ((avgBitrate*totalPacketsRecieved)+bitrate)/(totalPacketsRecieved+1);
          } else {
            avgBitrate = bitrate;
          }
        }
        
        totalPacketsRecieved+=1;
        println("");
      }
      
  }
}
