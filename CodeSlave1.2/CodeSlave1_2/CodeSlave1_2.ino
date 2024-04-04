#include "BluetoothSerial.h"
#include <DHTesp.h>
BluetoothSerial SerialBT; // Crear objeto BluetoothSerial 
String MACadd = "00:22:12:01:83:9b"; // Dirección MAC del dispositivo Bluetooth al que se desea conectar
uint8_t address[6]  = {0x00, 0x22, 0x12, 0x01, 0x83, 0x9b};
String name = "MASTER_BT"; // Nombre del dispositivo Bluetooth al que se desea conectar
char *pin = "1234"; // Código PIN para la conexión Bluetooth (estándar)
bool connected; 

//Variables sensor
const int Dh11 = 19;
DHTesp dhtSensor;

void setup() {
  Serial.begin(115200); 
  SerialBT.begin("ESP32test", true); // Inicia la comunicación Bluetooth, el dispositivo se establece como maestro (true)
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
  dhtSensor.setup(Dh11, DHTesp::DHT22);
}

void loop() {
  if (SerialBT.available()) {
    String frameRecived = SerialBT.readString();
    int cmd = frameRecived.toInt();
    switch(cmd){
      case 1:
      {
        //Leer temperatura
        int Temperatura = dhtSensor.getTemperature();
        SerialBT.println(Temperatura);
      }
      break;
      case 2:
      {
        int humedad = dhtSensor.getHumidity();
         SerialBT.println(humedad);
        //Leer humedad
      }
      break;
      default:
      {
        Serial.println("Cmd incorrect");
      }
      break;
    }
  }
  delay(20); // Espera 20 milisegundos antes de repetir el bucle
}

