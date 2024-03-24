#include <SoftwareSerial.h>
#include <Servo.h>
#include <NewPing.h>

#define TRIG_PIN A5
#define ECHO_PIN A4
#define Distancia_Maxima 200
NewPing sensor(TRIG_PIN, ECHO_PIN, Distancia_Maxima);

#define RS485_BAUD 9600    // Velocidad de comunicación RS-485
#define RS485_PIN_MODE 13  // HIGH: Tx; LOW: Rx
#define MY_SLAVE_ID 0x23
#define SERVO_PIN 2

SoftwareSerial RS485Serial(12, 11);  // RX, TX
Servo myservo;

void setup() {
  Serial.begin(9600);
  pinMode(RS485_PIN_MODE, OUTPUT);
  RS485Serial.begin(RS485_BAUD);
  digitalWrite(RS485_PIN_MODE, LOW);  // Rx
  myservo.attach(SERVO_PIN);
}

void loop() {
  if (RS485Serial.available()) {
    String receivedFrame = RS485Serial.readStringUntil('\n');

    if (receivedFrame.startsWith("AA")) {
      //<HEAD><ID><CMD><DATE>
      String ID_Slave = receivedFrame.substring(2, 4);
      int idHex = strtol(ID_Slave.c_str(), NULL, 16);

      if (idHex == MY_SLAVE_ID) {
        String cmd = receivedFrame.substring(4, 6);
        int cmdInt = cmd.toInt(); 
        int inf = receivedFrame.substring(6).toInt();
        int rtta = ejecutarAccion(cmdInt, inf);
        enviarRtta(rtta);
      } else {
        Serial.println("Error");
      }
    } else {
      Serial.println("Error");
    }
  }
}

int ejecutarAccion(int cmd, int inf) {
  switch (cmd) {
    case 1:
      if (inf >= 90 && inf <= 180) {
        myservo.write(inf);
        return 1;
      } else {
        return 0;
      }
      break;
    case 2:
      int distancia=lecturaSensor();
      return distancia;
      break;
    default:
      break;
  }
}

void enviarRtta(int state) {
  String frame = "AA" + String(state) + "\n";
  Serial.println(frame);
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