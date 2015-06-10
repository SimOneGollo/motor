#include <SoftwareSerial.h>
#include <AFMotor.h>
//istanzio le classi della libreria AFMotor
AF_DCMotor motor1(1, MOTOR12_64KHZ); // create motor #2, 64KHz pwm
AF_DCMotor motor2(2, MOTOR12_64KHZ); // create motor #2, 64KHz pwm
//istanzio la classe della libreria SoftwareSerial
SoftwareSerial mySerial(52,53); // RX, TX
//trig
const int TRIG1 = 42;  //sensore avanti
const int TRIG2 = 34;  //sensore sinistra
const int TRIG3 = 39;  //sensore destra
//echo
const int ECHO1 = 43;  //sensore avanti
const int ECHO2 = 35;  //sensore sinistra
const int ECHO3 = 38;  //sensore destra
//direzione
const int FRONT = 3;
const int LEFT_FRONT = 2;
const int LEFT = 1;
const int RIGHT_BOTTOM = 6;
const int BOTTOM = 7;
const int BOTTOM_LEFT = 8;
const int RIGHT = 5;
const int FRONT_RIGHT= 4;
//variabile per il bluetooth
String command = ""; 
//altre variabili
int key=22,Idirection,btLed=28,Icommand;
boolean stato=true,partenza=false;

void setup() {
   pinMode(key,OUTPUT);//key per il bluetooth serve per configuralo
   pinMode(btLed,OUTPUT);//notifica la ricezione di qualcosa
   digitalWrite(key,HIGH);//attiva key ma serve solo in fase iniziale
   //inizializzazione porta seriale definendo il baud
   Serial.begin(38400);
   Serial.println("Type AT commands!");
   // SoftwareSerial "com port"
   mySerial.begin(38400);   
}
 
void loop(){
  // Read device output if available.
  if (mySerial.available()) 
  { 
    digitalWrite(btLed,HIGH);
    while(mySerial.available()){
      //leggo i valori ricevuti dal bluetooth
      command += (char)mySerial.read();
    }
    Icommand=commandToInt(command);//converte comando in int
    Serial.println(Icommand);
    Serial.println(command);
    switch (Icommand){
        case 1:
          partenza = true;//flag per la partenza 
        break;
        case 2:
          partenza = false;//flag per lo stop
        break;
        case -1:
          if (partenza){
            //converte la stringa in intero
            Idirection=StrToInt(command);
            manuale(Idirection);  
          }
          else{stopAuto();}
        break;
     }
     command = ""; // No repeats
  }
  else{
    digitalWrite(btLed,LOW);
    if (partenza)
      automatico();
    else
      stopAuto();    
  }
  // manda linput scritto dall' utente
  if (Serial.available()){
    delay(10); // The DELAY!
    mySerial.write(Serial.read());
  }
  //delay di sincronizzazione con l' app
  delay(100);
}//chiuso loop

      /*
      *
      *
      *funzioni
      *
      *
      */
int commandToInt(String commands){
  if(commands=="start")
    return 1;
  else
    if(commands=="stop")
      return 2;
    else
      return -1;  
}
//conversione da stringa a intero
int StrToInt(String str){  
  int length=str.length();
  int potenza = 0;
  int somma=0;
  for(int i=length-1;i>=0;i--)  {
    somma+= pow(10,potenza) * ((char)str[i]-48);
    potenza++;  
  }
   return somma; 
} 


void stopAuto(){//ferma la macchina
    Serial.println("FERMO");
    motor1.setSpeed(0);                                                                         
    motor2.setSpeed(0);  
}
void manuale(int direct){
    Serial.println("MANUALE");
    switch (direct){
      case FRONT:
          motor1.setSpeed(205);                                                                         
          motor2.setSpeed(240);
          motor1.run(FORWARD);                           
          motor2.run(FORWARD);
      break;
      case LEFT_FRONT:  
          motor1.setSpeed(120);                                                                         
          motor2.setSpeed(235);
          motor1.run(FORWARD);                            
          motor2.run(FORWARD);
      break;
      case LEFT:
          motor1.setSpeed(175);                                                                         
          motor2.setSpeed(195);
          motor1.run(BACKWARD);                            
          motor2.run(FORWARD);
      break;
      case BOTTOM_LEFT:
          motor1.setSpeed(120);                                                                         
          motor2.setSpeed(235);
          motor1.run(BACKWARD);                            
          motor2.run(BACKWARD);
      break;
      case BOTTOM:
          motor1.setSpeed(205);                                                                        
          motor2.setSpeed(235);
          motor1.run(BACKWARD);                           
          motor2.run(BACKWARD);
      break;
      case RIGHT_BOTTOM:
          motor1.setSpeed(235);                                                                         
          motor2.setSpeed(135);
          motor1.run(BACKWARD);                            
          motor2.run(BACKWARD);
      break;
      case RIGHT:
          motor1.setSpeed(175);                                                                         
          motor2.setSpeed(195);
          motor1.run(FORWARD);                           
          motor2.run(BACKWARD);
      break;
      case FRONT_RIGHT:
          motor1.setSpeed(235);                                                                         
          motor2.setSpeed(135);
          motor1.run(FORWARD);                           
          motor2.run(FORWARD);
      break;
    default:
      Serial.println("errore di codice");
    break;
  }  
}//chiuso manuale
void automatico (){
    Serial.println("AUTOMATICO");
    long front,left,right;
    //richiamo la funzione del sensore, ritorna la distanza in cm
    front = sch04(TRIG1,ECHO1);
    delay(100);
    motor1.setSpeed(205); //imposto velocità                                                                        
    motor2.setSpeed(235); //imposto velocità                                                               
    motor1.run(FORWARD);  //parte, parametro->direzione                         
    motor2.run(FORWARD);
    
    if(front<20){ 
      while(stato){         
        motor1.run(BACKWARD); 
        motor2.run(BACKWARD);
        delay(300);
        left=sch04(TRIG2,ECHO2);
        delay(100);
        right=sch04(TRIG3,ECHO3);
        delay(100);
        if(left>right){
          //giro a destra e continuo la marcia
          motor1.run(BACKWARD); 
          motor2.run(FORWARD);
          delay(500);
          stato=false;  
        }
        else{
          if(left<right){
            //giro a sinistra e continuo la marcia
            motor1.run(FORWARD); 
            motor2.run(BACKWARD);
            delay(500);
            stato=false;  
          }
          else{stato=true;}
        }
      }
    }
    stato=true;
}// chiuso automatico

long microsecondsToCentimeters(long microseconds){
  //la velocità del suono è 340 m/s o 29 millisecondi per centimetro.
  // nel calcolo viene diviso per 2 perché il la distanza rilevata è pari allo spazioe dell' andata e del ritorno del suono
  return microseconds / 29 / 2;
}
//fnzione per trovare la distanza
long sch04(int trigPin, int echoPin ){
  long duration, cm;
  //emissione del suono
  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  //ricezione del suono
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);

  //conversione del tempo in spazio
  cm = microsecondsToCentimeters(duration);
  //feedback su monitor seriale
  switch (trigPin){
    case TRIG2:
      Serial.print("left");
      break;
      
    case TRIG1:
      Serial.print("front");
      break;
      
    case TRIG3:
      Serial.print("right");
      break;
  }
  Serial.print(cm);
  Serial.print("cm");
  Serial.println();
  
  //ritorno il valore
  
  return cm;
}
