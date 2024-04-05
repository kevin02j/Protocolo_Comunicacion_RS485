/*
  Authors: Kevin Certuche & Wilson Guerrero 
  Date: 4/04/2024
  Description: Arduino code for RS-485 communication between master and slave devices.
  Board: Arduino UNO Rev 3
*/

#include <SoftwareSerial.h>
#include <stdint.h>
#include <Wire.h>

#define RS485_BAUD 9600   // RS-485 communication speed
#define RS485_PIN_MODE 3  // HIGH: Tx; LOW: Rx

SoftwareSerial RS485Serial(4, 2);  // RX, TX

//Variables para almacenar la rtta del esclavo
String ByteState;
String binaryByteState;
String Information;
String binaryInfo;
String CompleteBinary;
String CRC_Calculate;
String CRC_Received;
int Checksum;

void setup() {
  Serial.begin(9600);
  pinMode(RS485_PIN_MODE, OUTPUT);
  RS485Serial.begin(RS485_BAUD);
}

void loop() {
  if (Serial.available()) {
    String frame = Serial.readStringUntil('\n');

    if (frame.startsWith("AA")) {
      String dataFrameBinary = getDataFrameBinary(frame);
      String CRC_Frame_Send = OrderCRC(dataFrameBinary);
      String frameComplete = frame + CRC_Frame_Send;
      Serial.println("Frame to send : " + String(frameComplete));
      send_Frame(frameComplete);
      delay(1000);
      respuestaSlave();
    } else {
      Serial.println("The frame does not have the predefined header. Error!");
    }
  }
}

// Function to send frame over RS-485
void send_Frame(String frameComplete) {
  digitalWrite(RS485_PIN_MODE, HIGH);  // Modo Tx
  RS485Serial.println(frameComplete);
  RS485Serial.flush();
  digitalWrite(RS485_PIN_MODE, LOW);  // Modo Rx
}

// Function to handle response from slave
void respuestaSlave() {
  if (RS485Serial.available()) {
    String frameSlave = RS485Serial.readStringUntil('\n');
    if (frameSlave.startsWith("AA")) {
      String usefulFrame = frameSlave.substring(2);
      uint8_t sizeS = usefulFrame.length();
      switch (sizeS) {
        case 6:
          {
            // Handle 6-byte frame
            String ByteState = usefulFrame.substring(0, 2);
            binaryByteState = hexToBinary(ByteState);
            CRC_Calculate = OrderCRC(binaryByteState);
            CRC_Received = usefulFrame.substring(2);
            Serial.println();
            Serial.println("CRC Received: " + CRC_Received);
            Serial.println("CRC Calculate: " + CRC_Calculate);
            compareCRCs(CRC_Received, CRC_Calculate);
            if(Checksum == 0){
              evaluateBits(binaryByteState);
            }else {
              Serial.println("Data transmission error (slave-master)");
            }
          }
          break;
        case 7:
          {
            // Handle 7-byte frame
            String ByteState = usefulFrame.substring(0, 2);
            binaryByteState = hexToBinary(ByteState);
            Information = usefulFrame.substring(2, 3);
            binaryInfo = IntToBinary(Information);
            CompleteBinary = binaryByteState + binaryInfo;
            CRC_Calculate = OrderCRC(CompleteBinary);
            CRC_Received = usefulFrame.substring(3);
            Serial.println();
            Serial.println("CRC Received: " + CRC_Received);
            Serial.println("CRC Calculate: " + CRC_Calculate);
            compareCRCs(CRC_Received, CRC_Calculate);
            if(Checksum == 0){
              evaluateBits(binaryByteState);
              showInfo(Information);
            }else {
              Serial.println("Data transmission error (slave-master)");
            }
          }
          break;
        case 8:
          {
            // Handle 8-byte frame
            String ByteState = usefulFrame.substring(0, 2);
            binaryByteState = hexToBinary(ByteState);
            Information = usefulFrame.substring(2, 4);
            binaryInfo = IntToBinary(Information);
            CompleteBinary = binaryByteState + binaryInfo;
            CRC_Calculate = OrderCRC(CompleteBinary);
            CRC_Received = usefulFrame.substring(4);
            Serial.println();
            Serial.println("CRC Received: " + CRC_Received);
            Serial.println("CRC Calculate: " + CRC_Calculate);
            compareCRCs(CRC_Received, CRC_Calculate);
            if(Checksum == 0){
              evaluateBits(binaryByteState);
              showInfo(Information);
            }else {
              Serial.println("Data transmission error (slave-master)");
            }
          }
          break;
        case 9: 
          {
            // Handle 9-byte frame
            String ByteState = usefulFrame.substring(0, 2);
            binaryByteState = hexToBinary(ByteState);
            Information = usefulFrame.substring(2, 5);
            binaryInfo = IntToBinary(Information);
            CompleteBinary = binaryByteState + binaryInfo;
            CRC_Calculate = OrderCRC(CompleteBinary);
            CRC_Received = usefulFrame.substring(5);
            Serial.println();
            Serial.println("CRC Received: " + CRC_Received);
            Serial.println("CRC Calculate: " + CRC_Calculate);
            compareCRCs(CRC_Received, CRC_Calculate);
            if(Checksum == 0){
              evaluateBits(binaryByteState);
              showInfo(Information);
            }else {
              Serial.println("Data transmission error (slave-master)");
            }
          }
          break;
        case 10: 
          {
            // Handle 10-byte frame
            String ByteState = usefulFrame.substring(0, 2);
            binaryByteState = hexToBinary(ByteState);
            Information = usefulFrame.substring(2, 6);
            binaryInfo = IntToBinary(Information);
            CompleteBinary = binaryByteState + binaryInfo;
            CRC_Calculate = OrderCRC(CompleteBinary);
            CRC_Received = usefulFrame.substring(6);
            Serial.println();
            Serial.println("CRC Received: " + CRC_Received);
            Serial.println("CRC Calculate: " + CRC_Calculate);
            compareCRCs(CRC_Received, CRC_Calculate);
            if(Checksum == 0){
              evaluateBits(binaryByteState);
              showInfo(Information);
            }else {
              Serial.println("Data transmission error (slave-master)");
            }
          }
          break;
        default:
          {
            Serial.print("The frame size is out of range. Error!!");
          }
          break;
      }
    } else {
      Serial.println("The frame does not have the predefined header. Error!");
    }
  } else {
    Serial.println("No response received");
  }
}

// CRC-16 calculation
uint16_t crc16(uint8_t *data, uint16_t len) {
  uint16_t crc = 0xFFFF;
  for (uint16_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; ++j) {
      if (crc & 0x0001) {
        crc >>= 1;
        crc ^= 0xA001; 
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

// Calculate CRC and return hexadecimal string
String OrderCRC(String bin) {
  char buf[33];
  bin.toCharArray(buf, 33);
  int sz = bin.length();
  uint16_t crc = crc16((uint8_t *)buf, sz);
  String CRCHex = String(crc, HEX);
  CRCHex.toUpperCase();
  return CRCHex;
}

// Convert received frame to binary format
String getDataFrameBinary(String frame) {
  String dataHex = frame.substring(2, 6);  
  String dataInt = frame.substring(6);     
  String BinaryDateHex = String(strtoul(dataHex.c_str(), NULL, 16), BIN);
  String BinaryDateInt = String(strtoul(dataInt.c_str(), NULL, 10), BIN);
  String dataFrameBinary;
  if(dataInt.length() == 0){
    dataFrameBinary = BinaryDateHex;
    return dataFrameBinary;
  }else{
    dataFrameBinary = BinaryDateHex + BinaryDateInt;
    return dataFrameBinary;
  }
}


// Display information received from slave
void showInfo(String info) {
  Serial.println();
  Serial.println("Information received from slave: " + info);
}

// Compare received and calculated CRC-16
void compareCRCs(String CRC_Received, String CRC_Calculate) {
  long receivedValue = strtol(CRC_Received.c_str(), NULL, 16);
  long calculateValue = strtol(CRC_Calculate.c_str(), NULL, 16);
  if (receivedValue == calculateValue) {
    Checksum = 0;
  } else {
    Checksum = 1;
  }
}

// Evaluate individual bits in status byte
void evaluateBits(String byteEstado) {
  Serial.println();
  Serial.println("Status byte result");
  String acciones[] = {"MoveDone", "ChecksumError", "OverCurrent", "PowerOn", "ConfigError", "ValorLimit1", "ValorLimit2", "statePinHigh"};
  int longitud = byteEstado.length();
  for (int i = 0; i < longitud; i++) {
    char bit = byteEstado.charAt(i);
    int valorBit = bit - '0'; 
    if (valorBit == 1) {
      Serial.println("Bit " + String(i) + " is 1: " + acciones[i]);
    } else if (valorBit == 0) {
      Serial.println("Bit " + String(i) + " is 0");
    } else {
      Serial.println("Bit " + String(i) + " no es válido");
    }
  }
}

// Convert integer data to binary format
String convertInt_toBinary(String dataInt_1, String dataInt_2) {
  long aux = strtol(dataInt_1.c_str(), NULL, 10);
  String BinaryDateHex = String(aux, BIN);
  long aux2 = strtol(dataInt_2.c_str(), NULL, 10);
  String BinaryDateInt = String(aux2, BIN);
  String BinaryComplete = BinaryDateHex + BinaryDateInt;
  return BinaryComplete;
}

// Convert hexadecimal and integer data to binary format
String convertHex_int_toBinary(String dataHex, String dataInt) {
  long aux = strtol(dataHex.c_str(), NULL, 16);
  String BinaryDateHex = String(aux, BIN);
  long aux2 = strtol(dataInt.c_str(), NULL, 10);
  String BinaryDateInt = String(aux2, BIN);
  String BinaryComplete = BinaryDateHex + BinaryDateInt;
  return BinaryComplete;
}

// Convert hexadecimal data to binary format
String hexToBinary(String hexString) {
  String binaryString = "";
  for (int i = 0; i < hexString.length(); i++) {
    char hexChar = hexString.charAt(i);
    int nibbleValue;
    if (hexChar >= '0' && hexChar <= '9') {
      nibbleValue = hexChar - '0';
    } else if (hexChar >= 'A' && hexChar <= 'F') {
      nibbleValue = hexChar - 'A' + 10; 
    } else if (hexChar >= 'a' && hexChar <= 'f') {
      nibbleValue = hexChar - 'a' + 10; 
    } 
    for (int j = 3; j >= 0; j--) {
      if (nibbleValue & (1 << j)) {
        binaryString += "1";
      } else {
        binaryString += "0";
      }
    }
  }
  return binaryString;
}

// Convert integer data to binary format
String IntToBinary(String intString) {
  long aux = strtol(intString.c_str(), NULL, 10);
  String BinaryDateInt = String(aux, BIN);
  return BinaryDateInt;
}
