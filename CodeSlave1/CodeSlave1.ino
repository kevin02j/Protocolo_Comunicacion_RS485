#include <SoftwareSerial.h>
#include <Servo.h>
#include <Arduino.h>
#include <String.h>
#include <EEPROM.h>
#include <avr/io.h>
#include <avr/wdt.h>

#define PIN_IN1 9
#define PIN_IN2 8
#define PIN_OUT1 7
#define PIN_OUT2 6

#define PIN_PWM1 5
#define PIN_PWM2 4

#define PIN_TENSION A0
#define PIN_POT_1 A6
const float minTension = 1.5;
const float maxTension = 5.0;

#define RS485_BAUD 9600    // Velocidad de comunicación RS-485
#define RS485_PIN_MODE 11  // HIGH: Tx; LOW: Rx
#define MY_SLAVE_ID 0
#define SERVO_LEFT_PIN 13
#define SERVO_RIGHT_PIN 3


SoftwareSerial RS485Serial(12, 10);  // RX, TX
// SoftwareSerial BTserial(3, 2);  // RX, TX
Servo servoRight;
Servo servoLeft;

//Variables del byte de estado
int MoveDone;
int ChecksumError;
int OverCurrent;
int powerOn;
int configError;
int valorLimit1;
int valorLimit2;
//int homeInProgress;
int StatePin;
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
int configIO_FLag = 0;
int configPWM_Flag = 0;
int configSERVO_Flag = 0;

void setup() {
  Serial.begin(9600);
  pinMode(RS485_PIN_MODE, OUTPUT);
  RS485Serial.begin(RS485_BAUD);
  // BTserial.begin(115200);
  digitalWrite(RS485_PIN_MODE, LOW);  // Rx

  int PruebaID = EEPROM.read(MY_SLAVE_ID);
  Serial.println("my Id: " + String(PruebaID));
}

void loop() {
  if (RS485Serial.available()) {
    MoveDone = 0;
    ChecksumError = 0;
    OverCurrent = 0;
    powerOn = 0;
    configError = 0;
    valorLimit1 = 0;
    valorLimit2 = 0;
    StatePin = 0;
    Info_to_Send = 0;

    String receivedFrame = RS485Serial.readStringUntil('\n');

    if (receivedFrame.startsWith("AA")) {
      String ID_Slave = receivedFrame.substring(2, 4);
      int idInt = hex_to_decimal(ID_Slave);
      uint8_t IdEEprom = EEPROM.get(MY_SLAVE_ID, IdEEprom);
      if (idInt == IdEEprom) {
        String dataUseful = receivedFrame.substring(2);
        checksum(dataUseful);
        //Comprobar si hubo errores en la transmision
        if (ChecksumError == 0) {
          String cmd = receivedFrame.substring(4, 6);
          ejecutarAccion(cmd);
        }
        verificarTension();
        ByteState = String(MoveDone) + String(ChecksumError) + String(OverCurrent) + String(powerOn) + String(configError) + String(valorLimit1) + String(valorLimit2) + String(StatePin);
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
  long hexValue = strtol(cmd.c_str(), NULL, 16);
  int cmdInt = static_cast<int>(hexValue);
  // int cmdInt = cmd.toInt();
  //Serial.println(cmdInt);
  int inf = info.toInt();
  switch (cmdInt) {
    case 1:
      {
        //Asignar Id al esclavo
        Serial.println("New Id: " + String(info));
        if (inf >= 1 && inf <= 247) {
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
        // if (lastSensorValue == 0) {
        //   MoveDone = 0;
        // } else {
        //   MoveDone = 1;
        //   Info_to_Send = lastSensorValue;
        // }
      }
      break;
    case 3:
      {
        pinMode(PIN_IN1, INPUT);
        pinMode(PIN_IN2, INPUT);
        pinMode(PIN_OUT1, OUTPUT);
        pinMode(PIN_OUT2, OUTPUT);
        MoveDone = 1;
        configIO_FLag = 1;
      }
      break;
    case 4:
      {
        if (configIO_FLag == 1) {
          //Identificar cual pin desea conocer el estado
          switch (inf) {
            case 1:
              {
                if (digitalRead(PIN_IN1) == HIGH) {
                  StatePin = 1;
                  MoveDone = 1;
                } else {
                  StatePin = 0;
                  MoveDone = 1;
                }
              }
              break;
            case 2:
              {
                if (digitalRead(PIN_IN2) == HIGH) {
                  StatePin = 1;
                  MoveDone = 1;
                } else {
                  StatePin = 0;
                  MoveDone = 1;
                }
              }
              break;
            default:
              {
                MoveDone = 0;
                break;
              }
          }
        } else {
          configError = 1;
        }
      }
      break;
    case 5:
      {
        if (configIO_FLag == 1) {
          int pin = info.substring(0, 1).toInt();
          int inf = info.substring(1, 2).toInt();
          if (inf == 0 || inf == 1) {
            switch (pin) {
              case 1:
                {
                  digitalWrite(PIN_OUT1, inf);
                  MoveDone = 1;
                }
                break;
              case 2:
                {
                  digitalWrite(PIN_OUT2, inf);
                  MoveDone = 1;
                }
                break;
              default:
                {
                  MoveDone = 0;
                  break;
                }
            }
          } else {
            MoveDone = 0;
          }
        } else {
          configError = 1;
        }
      }
      break;
    case 6:
      {
        switch (inf) {
          case 1:
            {
              int lecturaADC = analogRead(PIN_POT_1);
              int valorMapeado = map(lecturaADC, 0, 1023, 1, 255);
              Info_to_Send = valorMapeado;
              MoveDone = 1;
            }
            break;
          default:
            {
              Serial.println("Channel ADC incorrect");
              MoveDone = 0;
            }
            break;
        }
      }
      break;
    case 7:
      {
        // Config PWM
        pinMode(PIN_PWM1, OUTPUT);
        pinMode(PIN_PWM2, OUTPUT);
        analogWrite(PIN_PWM1, 1);
        analogWrite(PIN_PWM2, 1);
        MoveDone = 1;
        configPWM_Flag = 1;
      }
      break;
    case 8:
      {
        if (configPWM_Flag == 1) {
          int channelPWM = info.substring(0, 1).toInt();
          int PWM = info.substring(1).toInt();
          if (PWM >= 0 && PWM < 255) {
            switch (channelPWM) {
              case 1:
                {
                  analogWrite(PIN_PWM1, PWM);
                  MoveDone = 1;
                }
                break;
              case 2:
                {
                  analogWrite(PIN_PWM2, PWM);
                  MoveDone = 1;
                }
                break;
              default:
                {
                  Serial.println("channel incorrect");
                }
                break;
            }
          } else {
            MoveDone = 0;
          }
        } else {
          configError = 1;
        }
      }
      break;
    case 9:
      {
        servoRight.attach(SERVO_RIGHT_PIN);
        servoLeft.attach(SERVO_LEFT_PIN);
        servoRight.write(90);
        servoLeft.write(90);
        MoveDone = 1;
        configSERVO_Flag = 1;
      }
      break;
    case 10:
      {
        if (configSERVO_Flag == 1) {
          int controlServos = info.substring(0, 1).toInt();
          int angle = info.substring(1).toInt();
          switch (controlServos) {
            case 1:
              {
                //Servo DERECHA
                if (angle >= 30 && angle <= 150) {
                  servoRight.write(angle);
                  MoveDone = 1;
                }
              }
              break;
            case 2:
              {
                //Servo pinza
                if (angle >= 30 && angle <= 180) {
                  servoLeft.write(angle);
                  MoveDone = 1;
                }
              }
              break;
            default:
              {
                Serial.println("Channel servo error!!!");
              }
              break;
          }
        } else {
          configError = 1;
        }
        break;
        case 11:
          {
            //resetArduino();
          }
          break;
        default:
          {
            Serial.println("command limit out");
          }
          break;
      }
  }
}

// void resetArduino() {
//   wdt_enable(WDTO_15MS);  // Habilita el Watchdog Timer con un tiempo de espera de 15 ms
//   while (1) {}
// }

void checksum(String checkFrame) {
  //Serial.println(checkFrame);
  uint8_t sizeF = checkFrame.length();
  //Serial.println(sizeF);
  switch (sizeF) {
    case 9:
      {
        dateHex = checkFrame.substring(0, 4);
        dateBinary = hexToBinary(dateHex);
        CRC_Received = checkFrame.substring(4);
        CRC_Calculate = OrderCRC(dateBinary);
        Serial.println("CRC received: " + CRC_Received);
        Serial.println("CRC calculate: " + CRC_Calculate);
        compareCRCs(CRC_Received, CRC_Calculate);
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
        compareCRCs(CRC_Received, CRC_Calculate);
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
        compareCRCs(CRC_Received, CRC_Calculate);
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
        compareCRCs(CRC_Received, CRC_Calculate);
      }
      break;
    case 13:
      {
        dateHex = checkFrame.substring(0, 4);
        dateInt = checkFrame.substring(4, 8);
        dateBinary = convertHex_int_toBinary(dateHex, dateInt);
        String CRC_Received = checkFrame.substring(8);
        String CRC_Calculate = OrderCRC(dateBinary);
        info = dateInt;
        Serial.println("CRC received: " + CRC_Received);
        Serial.println("CRC calculate: " + CRC_Calculate);
        compareCRCs(CRC_Received, CRC_Calculate);
      }
      break;
    // case 13:
    //   {
    //     dateHex = checkFrame.substring(0, 4);
    //     dateInt = checkFrame.substring(4, 8);
    //     dateBinary = convertHex_int_toBinary(dateHex, dateInt);
    //     String CRC_Received = checkFrame.substring(8);
    //     String CRC_Calculate = OrderCRC(dateBinary);
    //     info = dateInt;
    //     Serial.println("CRC received: " + CRC_Received);
    //     Serial.println("CRC calculate: " + CRC_Calculate);
    //     if (CRC_Calculate = CRC_Received) {
    //       ChecksumError = 0;
    //     } else {
    //       ChecksumError = 1;
    //     }
    //   }
    //   break;
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
  } else {
    frame = "AA" + StateByte_Send + String(Info_to_Send) + CRC + "\n";
  }
  Serial.println("Frame rtta Slave: " + frame);
  digitalWrite(RS485_PIN_MODE, HIGH);  // modo tx
  RS485Serial.print(frame);
  RS485Serial.flush();
  digitalWrite(RS485_PIN_MODE, LOW);  // modo rx
}

// int lecturaSensor() {
//   int cm = sensor.ping_cm();  // Medir la distancia en centímetros
//   if (cm == 0) {
//     //Serial.println("¡No se detectó ningún objeto!");
//     return 250;
//   } else {
//     return cm;
//   }
// }

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

void compareCRCs(String CRC_Received, String CRC_Calculate) {
  // Convertir las cadenas hexadecimales a valores decimales
  long receivedValue = strtol(CRC_Received.c_str(), NULL, 16);
  long calculateValue = strtol(CRC_Calculate.c_str(), NULL, 16);

  // Comparar los valores decimales e imprimir el resultado
  if (receivedValue == calculateValue) {
    ChecksumError = 0;
  } else {
    ChecksumError = 1;
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

int hex_to_decimal(String hex_string) {
  unsigned int result = 0;
  for (int i = 0; i < hex_string.length(); i++) {
    char c = hex_string.charAt(i);
    int digit = 0;
    if (isdigit(c)) {
      digit = c - '0';
    } else {
      digit = 10 + (toupper(c) - 'A');
    }
    result = 16 * result + digit;
  }
  return result;
}
