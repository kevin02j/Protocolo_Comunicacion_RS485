#include <SoftwareSerial.h>  //Crea nuevos puertos serie

//Prueba
#define HEAD         0xAA  //Encabezado de la trama
#define TAIL         0xFE  //Bit de STOP
#define CMD_LED_ON   0x01  //Enceder Led
#define CMD_LED_OFF  0x02  //Apagar led
#define CMD_READ_POT 0x03  //Solicitar el valor del sensor
#define SLAVE        0x23  //Direccion especifica del esclavo

#define RS485_BAUD 9600   // Velocidad de comunicación RS-485
#define RS485_PIN_MODE 2  // HIGH: Tx; LOW: Rx

SoftwareSerial RS485Serial(3, 4);  // RX, TX
byte trama[5], idx;                //Cadena para concatenar la informacion

void setup() {
  Serial.begin(19200);  // Inicializar comunicación serial para monitor
  pinMode(RS485_PIN_MODE, OUTPUT);
  RS485Serial.begin(RS485_BAUD);
}

void loop() {

  if (Serial.available()) {
    char c = Serial.read();
    int h, t;
    switch (c) {
      case '0':
        enviarComando(SLAVE, CMD_LED_ON);
        break;

      case '1':
        enviarComando(SLAVE, CMD_LED_OFF);
        break;

      case '2':
        enviarComando(SLAVE, CMD_READ_POT);
        Serial.print("Potenciometro: ");
        t = recibirRespuesta(SLAVE);
        if (t == -1)
          Serial.println("No se recibio' respuesta");
        else
          Serial.println(t);
        break;

      default:
        break;
    }
  }
}

//Funcion para enviar un paquete de datos
void enviarComando(byte esclavo, byte cmd) {
  trama[0] = HEAD;
  trama[1] = esclavo;
  trama[2] = cmd;
  trama[3] = TAIL;
  digitalWrite(RS485_PIN_MODE, HIGH);  // modo tx
  RS485Serial.write(trama, 4);
  RS485Serial.flush();
  digitalWrite(RS485_PIN_MODE, LOW);  // modo rx
  Serial.print(trama[0]);
}

//Funcion para recibir la respuesta del esclavo
int recibirRespuesta(byte esclavo) {
  digitalWrite(RS485_PIN_MODE, LOW);  // modo rx
  delay(500);
  RS485Serial.readBytes(trama, 4);
  if (trama[0] != HEAD)  // error en la trama
    return -1;
  if (trama[1] != esclavo)  // respuesta de otro esclavo
    return -1;
  if (trama[3] != TAIL)  // error en trama
    return -1;
  return trama[2];
}