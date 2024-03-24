#include <SoftwareSerial.h>

// Trama maestro-esclavo
#define HEAD 0xAA
#define MY_SLAVE_ID 0x23
#define TAIL 0xFE

//Comandos soportados
#define CMD_LED_ON      0x01
#define CMD_LED_OFF     0x02
#define CMD_READ_POT    0x03

#define LED_BUILTIN     13

// Formato de Trama: <HEAD> <SLAVE_ID> <CMD> <TAIL>

#define RS485_PIN_MODE 9         // HIGH -> Transmision; LOW-> recepcion

SoftwareSerial RS485Serial(10, 8);    // RX, TX

byte buff[5], idx;
float potenciometro;

void enviarRespuesta( float x ){
  buff[0] = HEAD;
  buff[1] = MY_SLAVE_ID;
  buff[2] = (byte)x;
  buff[3] = TAIL;
  digitalWrite( RS485_PIN_MODE, HIGH ); // poner en modo Tx
  RS485Serial.write( buff, 4 );               // transmitir mensaje
  RS485Serial.flush();
  digitalWrite( RS485_PIN_MODE, LOW);   // poner en modo Rx
  
}

void ejecutarComando(){
  Serial.println("Ejectutando comando!!!");
  if ( buff[1] != MY_SLAVE_ID ) // el mensaje es para otro esclavo
    return;

  switch( buff[2] ){                      // ejecutar comando

    case CMD_LED_ON:                      // Encender Led
      digitalWrite( LED_BUILTIN, HIGH );  
      break;

    case CMD_LED_OFF:                     // Apagar Led
      digitalWrite( LED_BUILTIN, LOW );
      break;
    
    case CMD_READ_POT:
      potenciometro = analogRead(0);
      enviarRespuesta( potenciometro );
      break;

    default:                              // Comando Inva'lido
      break;
  }
}


void setup() {
  // Configurar Serial a 19200 baudios (para el monitor serie)
  Serial.begin(19200);

  // Configurar para utilizar el bus 485 a 9600
  RS485Serial.begin(9600);

  // configrar pines
  pinMode( LED_BUILTIN, OUTPUT );
  pinMode( RS485_PIN_MODE, OUTPUT );
  digitalWrite( LED_BUILTIN, LOW );   // apagar led
  digitalWrite( RS485_PIN_MODE, LOW );// poner en modo de recepcion
  idx = 0;
}


void loop() {

  if( !RS485Serial.available() )
    return;

  byte incoming = RS485Serial.read();
  Serial.print("Recibido: ");
  Serial.println(incoming);
    
  if( idx == 0 ){           // principio de trama
    if( incoming != HEAD ) // trama incorrecta
      return;

    buff[idx] = incoming;
    idx++;
  }
  else if ( idx > 0 && idx < 4 ){ // 
    buff[idx++] = incoming;      //
     
    if ( idx == 4 ){                // fin de trama
      if( buff[3] == TAIL )         // verificar que termine bien
        ejecutarComando();
      idx = 0;
    }
  }
}