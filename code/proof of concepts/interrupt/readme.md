# start/stop interrupt proof of concept
minimale hard- en software die de correcte werking van een start/stop drukknop aantoont, gebruik makend van een hardware interrupt

const byte Start  = 3;
int Led = 9;

boolean RUN;

void setup(){
 
  analogReference(DEFAULT);
  pinMode(Start, INPUT_PULLUP);
  pinMode(Led, OUTPUT);
 
  attachInterrupt(digitalPinToInterrupt(Start), ProgrammaRun , FALLING);
 
}

void loop(){
 
  if (RUN == true){
   
   digitalWrite(9, HIGH);
   delay(1000);
   digitalWrite(9, LOW);
   delay(1000);
  }
  
  else {
    digitalWrite(9, LOW);
  }
}

void ProgrammaRun(){
 
  RUN =! RUN;
 
}
