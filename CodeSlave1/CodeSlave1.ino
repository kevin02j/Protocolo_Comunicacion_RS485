#include <Servo.h> 
 
Servo myservo;  // creamos el objeto servo 
const int EnTxPin =  2;  // HIGH:TX y LOW:RX
const int mydireccion =101; //Direccion del esclavo
void setup() 
{ 
  Serial.begin(9600);  
  Serial.setTimeout(100);  //establecemos un tiempo de espera de 100ms
  myservo.attach(9);  // asignamos el pin 9 para el servo.
  pinMode(EnTxPin, OUTPUT);
  digitalWrite(EnTxPin, LOW); //RS485 como receptor
} 
 
void loop() 
{ 
  if(Serial.available())
  {
    if(Serial.read()=='I') //Si recibimos el inicio de trama
    {
        int direccion=Serial.parseInt(); //recibimos la direccion 
        if(direccion==mydireccion) //Si direccion es la nuestra
        {
            char funcion=Serial.read(); //leemos el carácter de función
          
            //---Si el carácter de función es una S entonces la trama es para mover el motor----------- 
            if(funcion=='S') 
             {
                 int angulo=Serial.parseInt(); //recibimos el ángulo
                 if(Serial.read()=='F') //Si el fin de trama es el correcto
                 {
                   if(angulo<=180) //verificamos que sea un valor en el rango del servo
                    {
                      myservo.write(angulo); //movemos el servomotor al ángulo correspondiente.
                    }   
                 }
             }
             //---Si el carácter de función  es L entonces el maestro está solicitando una lectura del sensor---
             else if(funcion=='L')
             {
                if(Serial.read()=='F') //Si el fin de trama es el correcto
                 {
                   int lectura = analogRead(0); //realizamos  la lectura del sensor     
                   digitalWrite(EnTxPin, HIGH); //rs485 como transmisor
                    Serial.print("i"); //inicio de trama  
                   Serial.print(mydireccion); //direccion   
                   Serial.print(",");       
                   Serial.print(lectura); //valor del sensor
                   Serial.print("f"); //fin de trama  
                   Serial.flush(); //Esperamos hasta que se envíen los datos
                   digitalWrite(EnTxPin, LOW); //RS485 como receptor             
                 }
             }
        }
    }
  }
  delay(10);
} 