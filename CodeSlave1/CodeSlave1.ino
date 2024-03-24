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


#define RS485_BAUD 9600    // Velocidad de comunicación RS-485
#define RS485_PIN_MODE 13  // HIGH: Tx; LOW: Rx
#define MY_SLAVE_ID 0
#define SERVO_PIN 2

SoftwareSerial RS485Serial(12, 11);  // RX, TX
Servo myservo;

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
    String receivedFrame = RS485Serial.readStringUntil('\n');

    if (receivedFrame.startsWith("AA")) {
      //<HEAD><ID><CMD><DATE>
      int IdEEprom = EEPROM.read(MY_SLAVE_ID);
      String ID_Slave = receivedFrame.substring(2, 4);
      int idInt = ID_Slave.toInt();

      if (idInt == IdEEprom) {
        String data = receivedFrame.substring(2);
        int stateCRC = checksum(data);
        if (stateCRC == 2) {
          //Sin errorres en la comunicacion
          String cmd = receivedFrame.substring(4, 6);
          String rtta = ejecutarAccion(cmd);

          String CRCss = String(stateCRC);
          String rttaCompleteBinary = convertInt_toBinary(rtta, CRCss);
          String CRC_Send_Calculate = OrderCRC(rttaCompleteBinary);
          enviarRtta(rtta, 2, CRC_Send_Calculate);
        } else {
          String crcprueba = "AAAA";
          String Sc = "0";
          enviarRtta(Sc, 3, crcprueba);
        }
      } else {
        Serial.println("Error ID incorrecto");
      }
    } else {
      Serial.println("Error");
    }
  }
}

String ejecutarAccion(String cmd) {
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
          return "1";
        } else {
          Serial.println("El nuevo ID no está en el rango válido (0-255)");
          return "0";
        }
      }
      break;
    case 2:
      {
        //Ultima lectura del sensor
        if (lastSensorValue == 0) {
          return "0";
        } else {
          return String(lastSensorValue);
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
          return String(distancia);
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
     }
     break;
    case 5:
     {
      switch (inf) {
        case 3:
        {
          int conteo = 0;
          if (digitalRead(PIN_IN1) == HIGH){
            conteo++;
        return String(conteo);
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
          return "1";
        } else {
          return "0";
        }
      }
      break;
    case 8:
      {
        int distancia = lecturaSensor();
        lastSensorValue = distancia;
        return String(distancia);
      }
      break;
    default:
      break;
  }
}

int checksum(String checkFrame) {
  uint8_t sizeF = checkFrame.length();
  //Serial.println("lenght: " + String(sizeF));
  switch (sizeF) {
    case 9:
      {
        String datoHex9 = checkFrame.substring(0, 4);
        long aux9 = strtol(datoHex9.c_str(), NULL, 16);
        String datoBinario9 = String(aux9, BIN);

        String CRC_9_Received = checkFrame.substring(4);
        String CRC_9_Calculate = OrderCRC(datoBinario9);

        Serial.println("CRC recibido: " + CRC_9_Received);
        Serial.println("CRC calculado: " + CRC_9_Calculate);
        if (CRC_9_Calculate = CRC_9_Received) {
          return 2;
        } else {
          return 3;
        }
      }
      break;
    case 10:
      {
        String datoH_10 = checkFrame.substring(0, 4);
        String datoI_10 = checkFrame.substring(4, 5);

        String dateBinary10 = convertHex_int_toBinary(datoH_10, datoI_10);

        String CRC_10_Received = checkFrame.substring(5);
        info = checkFrame.substring(4, 5);
        String CRC_10__Calculate = OrderCRC(dateBinary10);

        Serial.println("CRC recibido: " + CRC_10_Received);
        Serial.println("CRC calculado: " + CRC_10__Calculate);
        if (CRC_10__Calculate = CRC_10_Received) {
          return 2;
        } else {
          return 3;
        }
      }
      break;
    case 11:
      {
        String datoH_11 = checkFrame.substring(0, 4);
        String datoI_11 = checkFrame.substring(4, 6);
        String dateBinary11 = convertHex_int_toBinary(datoH_11, datoI_11);

        String CRC_11_Received = checkFrame.substring(6);
        info = checkFrame.substring(4, 6);
        String CRC_11__Calculate = OrderCRC(dateBinary11);

        Serial.println("CRC recibido: " + CRC_11_Received);
        Serial.println("CRC calculado: " + CRC_11__Calculate);
        if (CRC_11__Calculate = CRC_11_Received) {
          return 2;
        } else {
          return 3;
        }
      }
      break;
    case 12:
      {
        String datoH_12 = checkFrame.substring(0, 4);
        String datoI_12 = checkFrame.substring(4, 7);
        String dateBinary12 = convertHex_int_toBinary(datoH_12, datoI_12);
        String CRC_12_Calculate = OrderCRC(dateBinary12);
        String CRC_12_Received = checkFrame.substring(7);
        info = checkFrame.substring(4, 7);
        Serial.println("CRC recibido: " + CRC_12_Received);
        Serial.println("CRC calculado: " + CRC_12_Calculate);
        if (CRC_12_Calculate = CRC_12_Received) {
          return 2;
        } else {
          return 3;
        }
      }
      break;
    default:
      {
        return 4;
        Serial.println("Error");
      }
      break;
  }
}

void enviarRtta(String state, int stateCRC, String CRC) {
  String frame = "AA" + state + String(stateCRC) + CRC + "\n";
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
