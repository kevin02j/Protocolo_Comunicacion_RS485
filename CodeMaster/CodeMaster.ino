const int ledPin =  13; // Numero del pin para el Led     
const int EnTxPin =  2; // HIGH:TX y LOW:RX
void setup() 
{ 
  Serial.begin(9600);
  Serial.setTimeout(100); //establecemos un tiempo de espera de 100ms
  // inicializamos los pines
  pinMode(ledPin, OUTPUT);
  pinMode(EnTxPin, OUTPUT);
  digitalWrite(ledPin, LOW); 
  digitalWrite(EnTxPin, HIGH); //RS485 como Transmisor
} 
 
void loop() 
{ 
   
  int lectura = analogRead(0);//leemos el valor del potenciómetro (de 0 a 1023) 
  int angulo= map(lectura, 0, 1023, 0, 180); // escalamos la lectura a un valor de ángulo (entre 0 y 180)
  //---enviamos el ángulo para mover el servo------
  Serial.print("I"); //inicio de trama
  Serial.print("102");//dirección del esclavo
  Serial.print("S"); //función  S para indicarle que vamos a mover el servo
  Serial.print(angulo); //ángulo  o dato
  Serial.print("F"); //fin de trama
  //----------------------------
  delay(50); 
  //---solicitamos una lectura del sensor----------
  Serial.print("I"); //inicio de trama
  Serial.print("102");//direccion del esclavo
  Serial.print("L"); //L para indicarle que vamos a Leer el sensor
  Serial.print("F"); //fin de trama
  Serial.flush();    //Esperamos hasta que se envíen los datos
  //----Leemos la respuesta del Esclavo-----
  digitalWrite(EnTxPin, LOW); //RS485 como receptor
  if(Serial.find("i")) //esperamos el inicio de trama
  {
      int esclavo=Serial.parseInt();  //recibimos la direccion del esclavo
      int dato=Serial.parseInt();  //recibimos el dato
      if(Serial.read()=='f'&&esclavo==102) //si fin de trama y direccion son los correctos
      {
         funcion(dato);   //realizamos la acción con el dato recibido
      }
  }
      digitalWrite(EnTxPin, HIGH); //RS485 como Transmisor
  //----------fin de la respuesta----------
  
} 
void funcion(int dato)
{
  //hjhdjwhjwdhdw
  if(dato>500)
  digitalWrite(ledPin, HIGH); 
  else{
    digitalWrite(ledPin, LOW); 
  }

}