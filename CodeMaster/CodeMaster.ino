#include <SoftwareSerial.h>
#include <stdint.h>
#include <Wire.h>

#define RS485_BAUD 9600   // Velocidad de comunicación RS-485
#define RS485_PIN_MODE 2  // HIGH: Tx; LOW: Rx

SoftwareSerial RS485Serial(3, 4);  // RX, TX

void setup() {
  Serial.begin(9600);
  pinMode(RS485_PIN_MODE, OUTPUT);
  RS485Serial.begin(RS485_BAUD);
}

void loop() {
  if (Serial.available()) {
    String frame = Serial.readStringUntil('\n');

    if (frame.startsWith("AA")) {
      String dataHex = frame.substring(2, 6);  //<ID><CMD>
      String dataInt = frame.substring(6);     //<DATE>
      long aux = strtol(dataHex.c_str(), NULL, 16);
      String BinaryDateHex = String(aux, BIN);
      long aux2 = strtol(dataInt.c_str(), NULL, 10);
      String BinaryDateInt = String(aux2, BIN);

      String dataFrameBinary;
      if (BinaryDateInt.length() == 1) {
        // CMD sin datos
        dataFrameBinary = BinaryDateHex;
      } else {
        // CMD con datos
        dataFrameBinary = BinaryDateHex + BinaryDateInt;
      }
      // Serial.println(dataFrameBinary);
      String CRC_Frame_Send = OrderCRC(dataFrameBinary);
      String frameComplete = frame + CRC_Frame_Send;
      Serial.println("Trama con CRC: " + String(frameComplete));
      enviarComando(frameComplete);
      delay(1000);

      int rttaSlave = respuestaSlave();
      switch (rttaSlave) {
        case 3:
          Serial.println("No se recibio respuesta");
          break;
        case 2:
          Serial.println("Formato incorrecto");
          break;
        case 1:
          Serial.println("Successful Action");
          break;
        case 0:
          Serial.println("Error Action");
          break;
        default:
          Serial.print("Distancia: ");
          Serial.print(rttaSlave);
          Serial.println(" CM");
          break;
      }
    } else {
      Serial.println("Formato incorrecto");
    }
  }
}

void enviarComando(String frame) {
  digitalWrite(RS485_PIN_MODE, HIGH);  // modo tx
  RS485Serial.println(frame);
  // Serial.println(frame);
  RS485Serial.flush();
  digitalWrite(RS485_PIN_MODE, LOW);  // modo rx
}

int respuestaSlave() {
  if (RS485Serial.available()) {
    String frameSlave = RS485Serial.readStringUntil('\n');
    if (frameSlave.startsWith("AA")) {
      // Serial.println(frameSlave);
      String AB = frameSlave.substring(2);
      uint8_t sizeS = AB.length();
      switch (sizeS) {
        case 6:
          {
            String State6 = frameSlave.substring(2, 3);
            String StateCRC6 = frameSlave.substring(3, 4);
            int A6 = State6.toInt();
            int B6 = StateCRC6.toInt();

            String CompleteBinary6 = convertInt_toBinary(State6, StateCRC6);

            String CRC_6_Calculate = OrderCRC(CompleteBinary6);
            String CRC_6_Received = frameSlave.substring(4);

            Serial.println("CRC recibido: " + CRC_6_Received);
            Serial.println("CRC calculado: " + CRC_6_Calculate);

            if (CRC_6_Calculate = CRC_6_Received) {
              Serial.println("CRC Slave CORRECT");
            } else {
              Serial.println("CRC Slave INCORRECT");
            }
            if (B6 == 2) {
              Serial.println("Information not error");
            } else {
              Serial.println("Information error");
            }
            return A6;
          }
          break;
        case 7:
          {
            String State7 = frameSlave.substring(2, 4);
            String StateCRC7 = frameSlave.substring(4, 5);
            int A7 = State7.toInt();
            int B7 = StateCRC7.toInt();

            String CompleteBinary7 = convertInt_toBinary(State7, StateCRC7);

            String CRC_7_Calculate = OrderCRC(CompleteBinary7);
            String CRC_7_Received = frameSlave.substring(5);

            Serial.println("CRC recibido: " + CRC_7_Received);
            Serial.println("CRC calculado: " + CRC_7_Calculate);

            if (CRC_7_Calculate = CRC_7_Received) {
              Serial.println("CRC Slave CORRECT");
            } else {
              Serial.println("CRC Slave INCORRECT");
            }
            if (B7 == 2) {
              Serial.println("Information not error");
            } else {
              Serial.println("Information error");
            }
            return A7;
          }
          break;
        case 8:
          {
            String State8 = frameSlave.substring(2, 5);
            String StateCRC8 = frameSlave.substring(5, 6);
            int A8 = State8.toInt();
            int B8 = StateCRC8.toInt();

            String CompleteBinary8 = convertInt_toBinary(State8, StateCRC8);
            Serial.println(CompleteBinary8);
            String CRC_8_Calculate = OrderCRC(CompleteBinary8);
            String CRC_8_Received = frameSlave.substring(6);

            Serial.println("CRC recibido: " + CRC_8_Received);
            Serial.println("CRC calculado: " + CRC_8_Calculate);

            if (CRC_8_Calculate = CRC_8_Received) {
              Serial.println("CRC Slave CORRECT");
            } else {
              Serial.println("CRC Slave INCORRECT");
            }
            if (B8 == 2) {
              Serial.println("Information not error");
            } else {
              Serial.println("Information error");
            }
            return A8;
          }
          break;
        default:
          {
            break;
          }
      }
    } else {
      return 2;  // Formato incorrecto
    }
  } else {
    return 3;  // No se recibió respuesta
  }
}

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

String OrderCRC(String bin) {
  char buf[33];
  bin.toCharArray(buf, 33);
  int sz = bin.length();
  uint16_t crc = crc16((uint8_t *)buf, sz);
  String CRCHex = String(crc, HEX);
  CRCHex.toUpperCase();
  return CRCHex;
}

String convertInt_toBinary(String dataInt_1, String dataInt_2) {
  long aux = strtol(dataInt_1.c_str(), NULL, 10);
  String BinaryDateHex = String(aux, BIN);
  long aux2 = strtol(dataInt_2.c_str(), NULL, 10);
  String BinaryDateInt = String(aux2, BIN);
  String BinaryComplete = BinaryDateHex + BinaryDateInt;
  return BinaryComplete;
}

String convertHex_int_toBinary(String dataHex, String dataInt) {
  long aux = strtol(dataHex.c_str(), NULL, 16);
  String BinaryDateHex = String(aux, BIN);
  long aux2 = strtol(dataInt.c_str(), NULL, 10);
  String BinaryDateInt = String(aux2, BIN);
  String BinaryComplete = BinaryDateHex + BinaryDateInt;
  return BinaryComplete;
}