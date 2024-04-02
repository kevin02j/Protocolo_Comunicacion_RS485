#include "BluetoothSerial.h"
BluetoothSerial SerialBT; // Crear objeto BluetoothSerial 
String MACadd = "00:22:12:01:83:9b"; // Dirección MAC del dispositivo Bluetooth al que se desea conectar
uint8_t address[6]  = {0x00, 0x22, 0x12, 0x01, 0x83, 0x9b};
String name = "MASTER_BT"; // Nombre del dispositivo Bluetooth al que se desea conectar
char *pin = "1234"; // Código PIN para la conexión Bluetooth (estándar)
bool connected; 

void setup() {
  Serial.begin(115200); 
  SerialBT.begin("ESP32test", false); // Inicia la comunicación Bluetooth, el dispositivo se establece como maestro (true)
  SerialBT.setPin(pin);
  Serial.println("The device started in slave mode, make sure remote BT device is on!"); 
  connected = SerialBT.connect(name);
  if(connected) { 
    Serial.println("Connected Succesfully!"); 
  } else { 
    while(!SerialBT.connected(10000)) { // Mientras no esté conectado y haya pasado menos de 10 segundos
      Serial.println("Failed to connect. Make sure remote device is available and in range, then restart app."); 
    }
  }
  // if (SerialBT.disconnect()) { 
  //   Serial.println("Disconnected Succesfully!"); 
  // SerialBT.connect(); 
}

void loop() {
  if (SerialBT.available()) {
    String frameRecived = SerialBT.read();
    Serial
  }
  delay(20); // Espera 20 milisegundos antes de repetir el bucle
}

