#include <SoftwareSerial.h>

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
      // String Cmd = frame.substring(4, 6);
      // int intCmd = Cmd.toInt();
      // Serial.println(intCmd);
      enviarComando(frame);
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
          Serial.println("Move Done");
          break;
        case 0:
          Serial.println("Move Fail");
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
    // Serial.println(frameSlave);
    String State = frameSlave.substring(2);
    int A = State.toInt();
    // Serial.println(A);
    if (frameSlave.startsWith("AA")) {
      return A;
    } else {
      return 2;  // Formato incorrecto
    }
  } else {
    return 3;  // No se recibió respuesta
  }
}