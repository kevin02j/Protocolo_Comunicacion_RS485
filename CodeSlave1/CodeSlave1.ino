#include <SoftwareSerial.h>
#include <Servo.h>
#include <NewPing.h>
#include <Arduino.h>
#include <String.h>
#include <EEPROM.h>

#define TRIG_PIN A5
#define ECHO_PIN A4
#define Distancia_Maxima 200
NewPing sensor(TRIG_PIN, ECHO_PIN, Distancia_Maxima);

#define PIN_IN1 3
#define PIN_IN2 4
#define PIN_IN3 5
#define PIN_IN4 6
#define PIN_OUT1 7
#define PIN_OUT2 8
#define PIN_OUT3 9
#define PIN_OUT4 10

#define PIN_TENSION A0
const float minTension = 1.5;
const float maxTension = 5.0;

#define RS485_BAUD 9600    // Velocidad de comunicación RS-485
#define RS485_PIN_MODE 13  // HIGH: Tx; LOW: Rx
#define MY_SLAVE_ID 0
#define SERVO_PIN 2


SoftwareSerial RS485Serial(12, 11);  // RX, TX
Servo myservo;

//Variables del byte de estado
int MoveDone;
int ChecksumError;
int OverCurrent;
int powerOn;
int positionError;
int valorLimit1;
int valorLimit2;
int homeInProgress;
int Info_to_Send;
String ByteState;

//Variables para comprobar el CRC
String dateHex;
String dateInt;
String dateBinary;
String CRC_Received;
String CRC_Calculate;
String info;

int lastSensorValue = 0;

void setup() {
  Serial.begin(9600);
  pinMode(RS485_PIN_MODE, OUTPUT);
  RS485Serial.begin(RS485_BAUD);
  digitalWrite(RS485_PIN_MODE, LOW);  // Rx
  myservo.attach(SERVO_PIN);
  int PruebaID = EEPROM.read(MY_SLAVE_ID);
  Serial.println("my Id: " + String(PruebaID));
}

void loop() {
  if (RS485Serial.available()) {
    MoveDone = 0;
    ChecksumError = 0;
    OverCurrent = 0;
    powerOn = 0;
    positionError = 0;
    valorLimit1 = 0;
    valorLimit2 = 0;
    homeInProgress = 0;
    Info_to_Send = 0;

    String receivedFrame = RS485Serial.readStringUntil('\n');
    
    if (receivedFrame.startsWith("AA")) {
      String ID_Slave = receivedFrame.substring(2, 4);
      int IdEEprom = EEPROM.read(MY_SLAVE_ID);
      int idInt = ID_Slave.toInt();
      if (idInt == IdEEprom) {
        String dataUseful = receivedFrame.substring(2);
        checksum(dataUseful);
        //Comprobar si hubo errores en la transmision
        if (ChecksumError == 0) {
          String cmd = receivedFrame.substring(4, 6);
          ejecutarAccion(cmd);
        }
        verificarTension();
        ByteState = String(MoveDone) + String(ChecksumError) + String(OverCurrent) + String(powerOn) + String(positionError) + String(valorLimit1) + String(valorLimit2) + String(homeInProgress);
        if (Info_to_Send == 0) {
          String rttaCompleteBinary = ByteState;
          String CRC_Send_Calculate = OrderCRC(rttaCompleteBinary);
          enviarRtta(CRC_Send_Calculate);
        } else {
          String Info_to_Send_Binary = String(Info_to_Send, BIN);
          String rttaCompleteBinary = ByteState + Info_to_Send_Binary;
          String CRC_Send_Calculate = OrderCRC(rttaCompleteBinary);
          enviarRtta(CRC_Send_Calculate);
        }
      } else {
        Serial.println("Information is not for this slave");
      }
    } else {
      Serial.println("The frame does not have the predefined header. Error!");
    }
  }
}

void ejecutarAccion(String cmd) {
  int cmdInt = cmd.toInt();
  int inf = info.toInt();
  switch (cmdInt) {
    case 1:
      {
        //Asignar Id al esclavo
        Serial.println("Informacion String: " + String(info));
        Serial.println(inf);
        if (inf >= 0 && inf <= 255) {
          EEPROM.write(0, inf);  // 0-255
          MoveDone = 1;
        } else {
          Serial.println("El nuevo ID no está en el rango válido (0-255)");
          MoveDone = 0;
        }
      }
      break;
    case 2:
      {
        //Ultima lectura del sensor
        if (lastSensorValue == 0) {
          MoveDone = 0;
        } else {
          MoveDone = 1;
          Info_to_Send = lastSensorValue;
        }
      }
      break;
    case 3:
      {
        //Cantidad de respuestas
        for (int i = 0; i < inf; i++) {
          int distancia = lecturaSensor();
          lastSensorValue = distancia;
          delay(50);
          Info_to_Send = distancia;
        }
      }
      break;
    case 4:
      {
        pinMode(PIN_IN1, INPUT);
        pinMode(PIN_IN2, INPUT);
        pinMode(PIN_IN3, INPUT);
        pinMode(PIN_IN4, INPUT);
        pinMode(PIN_OUT1, OUTPUT);
        pinMode(PIN_OUT2, OUTPUT);
        pinMode(PIN_OUT3, OUTPUT);
        pinMode(PIN_OUT4, OUTPUT);
        MoveDone = 1;
      }
      break;
    case 5:
      {
        switch (inf) {
          case 3:
            {
              if (digitalRead(PIN_IN1) == HIGH) {
                Info_to_Send = 1;
              }
              else{
                Info_to_Send = 2;
              }
            }
            break;
          case 4:
            {
            }
            break;
          case 5:
            {
            }
            break;
          case 6:
            {
            }
            break;
          default:
            {
              break;
            }
        }
      }
      break;
    case 6:
      {
      }
      break;
    case 7:
      {
        if (inf >= 90 && inf <= 180) {
          myservo.write(inf);
          MoveDone = 1;
        } else {
          MoveDone = 0;
        }
      }
      break;
    case 8:
      {
        int distancia = lecturaSensor();
        lastSensorValue = distancia;
        MoveDone = 1;
        Info_to_Send = distancia;
      }
      break;
    default:
      break;
  }
}

void checksum(String checkFrame) {
  uint8_t sizeF = checkFrame.length();
  switch (sizeF) {
    case 9:
      {
        dateHex = checkFrame.substring(0, 4);
        dateBinary = hexToBinary(dateHex);
        CRC_Received = checkFrame.substring(4);
        CRC_Calculate = OrderCRC(dateBinary);
        Serial.println("CRC received: " + CRC_Received);
        Serial.println("CRC calculate: " + CRC_Calculate);
        if (CRC_Calculate = CRC_Received) {
          ChecksumError = 0;
        } else {
          ChecksumError = 1;
        }
      }
      break;
    case 10:
      {
        dateHex = checkFrame.substring(0, 4);
        dateInt = checkFrame.substring(4, 5);
        dateBinary = convertHex_int_toBinary(dateHex, dateInt);
        String CRC_Received = checkFrame.substring(5);
        String CRC_Calculate = OrderCRC(dateBinary);
        info = dateInt;
        Serial.println("CRC received: " + CRC_Received);
        Serial.println("CRC calculate: " + CRC_Calculate);
        if (CRC_Calculate = CRC_Received) {
          ChecksumError = 0;
        } else {
          ChecksumError = 1;
        }
      }
      break;
    case 11:
      {
        dateHex = checkFrame.substring(0, 4);
        dateInt = checkFrame.substring(4, 6);
        dateBinary = convertHex_int_toBinary(dateHex, dateInt);
        String CRC_Received = checkFrame.substring(6);
        String CRC_Calculate = OrderCRC(dateBinary);
        info = dateInt;
        Serial.println("CRC received: " + CRC_Received);
        Serial.println("CRC calculate: " + CRC_Calculate);
        if (CRC_Calculate = CRC_Received) {
          ChecksumError = 0;
        } else {
          ChecksumError = 1;
        }
      }
      break;
    case 12:
      {
        dateHex = checkFrame.substring(0, 4);
        dateInt = checkFrame.substring(4, 7);
        dateBinary = convertHex_int_toBinary(dateHex, dateInt);
        String CRC_Received = checkFrame.substring(7);
        String CRC_Calculate = OrderCRC(dateBinary);
        info = dateInt;
        Serial.println("CRC received: " + CRC_Received);
        Serial.println("CRC calculate: " + CRC_Calculate);
        if (CRC_Calculate = CRC_Received) {
          ChecksumError = 0;
        } else {
          ChecksumError = 1;
        }
      }
      break;
    default:
      {
        Serial.println("The frame size is out of range. Error!!");
      }
      break;
  }
}

void enviarRtta(String CRC) {
  String StateByte_Send = convertBinary_toHex(ByteState);
  String frame;
  if (Info_to_Send == 0) {
    frame = "AA" + StateByte_Send + CRC + "\n";
  }else{
    frame = "AA" + StateByte_Send + String(Info_to_Send) + CRC + "\n";
  }
  Serial.println("Frame rtta Slave: " + frame);
  digitalWrite(RS485_PIN_MODE, HIGH);  // modo tx
  RS485Serial.print(frame);
  RS485Serial.flush();
  digitalWrite(RS485_PIN_MODE, LOW);  // modo rx
}

int lecturaSensor() {
  int cm = sensor.ping_cm();  // Medir la distancia en centímetros
  if (cm == 0) {
    //Serial.println("¡No se detectó ningún objeto!");
    return 250;
  } else {
    return cm;
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

void verificarTension() {
  int lectura = analogRead(PIN_TENSION);
  float tension = lectura * (5.0 / 1023.0);
  if (tension >= minTension && tension <= maxTension) {
    powerOn = 1;
  } else {
    powerOn = 0;
  }
}

String hexToBinary(String hexString) {
  unsigned int hexValue = strtol(hexString.c_str(), NULL, 16);
  String binaryString = String(hexValue, BIN);
  return binaryString;
}

String convertHex_int_toBinary(String dataHex, String dataInt) {
  long aux = strtol(dataHex.c_str(), NULL, 16);
  String BinaryDateHex = String(aux, BIN);
  long aux2 = strtol(dataInt.c_str(), NULL, 10);
  String BinaryDateInt = String(aux2, BIN);
  String BinaryComplete = BinaryDateHex + BinaryDateInt;
  return BinaryComplete;
}

String convertInt_toBinary(String dataInt_1, String dataInt_2) {
  long aux = strtol(dataInt_1.c_str(), NULL, 10);
  String BinaryDateHex = String(aux, BIN);
  long aux2 = strtol(dataInt_2.c_str(), NULL, 10);
  String BinaryDateInt = String(aux2, BIN);
  String BinaryComplete = BinaryDateHex + BinaryDateInt;
  return BinaryComplete;
}

String convertBinary_toHex(String binario) {
  unsigned long decimal = strtol(binario.c_str(), NULL, 2);
  char hex[9];
  sprintf(hex, "%X", decimal);
  String resultadoHex = String(hex);
  return resultadoHex;
}
