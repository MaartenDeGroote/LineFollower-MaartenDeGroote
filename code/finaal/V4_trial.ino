
#include "SerialCommand.h"
#include "EEPROMAnything.h"

#define SerialPort Serial1 //Serial 1 = Bluetooth ///// Serial = terminal pc
#define Baudrate 9600

SerialCommand sCmd(SerialPort);
int powerLed = 11;


int leftForward = 6;
int leftBackward = 5;
int rightForward = 9;
int rightBackward = 10;
int Ledon = 7;


bool debug; //1
bool runState; // 2
int buttonState =0;
const int button =8;

const int sensor[] = {A0, A1, A2, A3, A4, A5}; //4

unsigned long previous, calculationTime; //3


struct param_t
{
unsigned long cycleTime; 
int black[6]; //
int white[6]; //
int power; //
float diff; //
float kp; //
float ki; //
// andere parameters voor eeprom hier tovoegen 
} params;


float debugPosition;//

void onUnknownCommand(char * command);
void onSet();
void onDebug();
void onCalibrate();
void onRun();
void onStop();

void setup ()
{

  SerialPort.begin(Baudrate);
  sCmd.addCommand("set", onSet);
  sCmd.addCommand("debug", onDebug);
  sCmd.addCommand("calibrate", onCalibrate);
  sCmd.addCommand("run", onRun);
  sCmd.addCommand("stop", onStop);
  sCmd.setDefaultHandler(onUnknownCommand);

  EEPROM_readAnything(0, params);
  SerialPort.println("ready");
  
  pinMode(Ledon, OUTPUT);
  pinMode(powerLed, OUTPUT);

  pinMode(leftForward, OUTPUT);
  pinMode(leftBackward, OUTPUT);
  pinMode(rightForward, OUTPUT);
  pinMode(rightBackward, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  
}
void loop()
{
 sCmd.readSerial();

 

 
 unsigned long current = micros();
 if(current - previous >= params.cycleTime)
 {
  previous = current;
  
  digitalWrite(Ledon, HIGH);
  buttonState = digitalRead(button);
  
  int normalised[6];//
  int index = 0;
  float position = 0;
  bool run = false;
  
  for (int i = 0; i < 6; i++)
  {
    normalised[i] = map(analogRead(sensor[i]), params.black[i], params.white[i], 0, 1000);
  }
 
 for (int i = 1; i < 6; i++) if (normalised[i] < normalised[index]) index = i;

 
 if (normalised[index] > 3000) run = false;
 
  if (buttonState != lastButtonState) {
    
    if (buttonState == HIGH) {
      // if the current state is HIGH then the button went from off to on:
      buttonPushCounter++;
      Serial.println("on");
      Serial.print("number of button pushes: ");
      Serial.println(buttonPushCounter);
    } else {
      // if the current state is LOW then the button went from on to off:
      Serial.println("off");
    }
    // Delay a little bit to avoid bouncing
    delay(50);
  }
  
  lastButtonState = buttonState;

  

 // if (buttonPushCounter % 2 == 0) {
  //  runState = true;
 // } else {
    //runState = false;
//  }
  

 if (index == 0) position = -30;
 else if (index == 5) position = 30;
 else
 {
  int sNul = normalised[index];
  int sMinEen = normalised[index-1];
  int sPlusEen = normalised[index+1];

  float b = sPlusEen - sMinEen;
  b = b / 2;

  float a = sPlusEen - b - sNul;

  position = -b / (2 * a);
  position += index;
  position -= 2.5;

  position *= 15;
 
 }
 debugPosition = position;
 
 float error = -position;
// Proportioneel regelen
 float output = error * params.kp;

 output = constrain(output, -510, 510); //pwm value zie proportioneel regelen

 // intigerend regelen
/* float iTerm;
 iTerm += params.ki * error;
 iTerm = constrain(iTerm, -510, 510);
 output +=iTerm;
 output = constrain(output, -510, 510);
 */
 int powerleft = 0;
 int powerRight = 0;
 
 if (runState) if(output >= 0)
 {
  powerleft = constrain(params.power + params.diff * output, -255, 255);
  powerRight = constrain(powerleft - output, -255, 255);
  powerleft = powerRight + output;
  digitalWrite(powerLed, HIGH);
  
 }
 else
 {
  powerRight = constrain(params.power - params.diff * output, -255, 255);
  powerleft = constrain(powerRight + output, -255, 255);
  powerRight = powerleft - output;
 }

analogWrite(leftBackward, powerleft > 0? powerleft : 0);
analogWrite(leftForward, powerleft < 0? -powerleft : 0);
analogWrite(rightBackward, powerRight > 0? powerRight : 0);
analogWrite(rightForward , powerRight < 0? -powerRight : 0);
 
 }
  unsigned long difference = micros() - current;
 if (difference > calculationTime) calculationTime = difference;
}

void onSet()
{
  char* param = sCmd.next();
  char* value = sCmd.next();

  if (strcmp(param, "cycle") == 0) 
 /* {
    long newCycleTime = atol(value);
    float ratio = ((float) newCycleTime) / ((float) params.cycleTime);

    params.ki *= ratio;

    params.cycleTime = newCycleTime;
  }
  else if (strcmp(param, "ki") == 0)
  {
    float cycleTimeInSec = ((float) params.cycleTime) / 1000000;
    params.ki = atof(value) * cycleTimeInSec;
  }*/

  params.cycleTime = atol(value);
  else if(strcmp(param, "power") == 0) params.power = atol(value);
  else if(strcmp(param, "diff") == 0) params.diff = atof(value);
  else if(strcmp(param, "kp") == 0) params.kp = atof(value);
 
  EEPROM_writeAnything(0,params);

}
void onDebug()
{
   SerialPort.print("position: ");
  SerialPort.println(debugPosition);

  SerialPort.print("cycle time: ");
  SerialPort.println(params.cycleTime);

  float cycleTimeInSec = ((float) params.cycleTime) / 1000000;
  float ki = params.ki / cycleTimeInSec;
 
  SerialPort.print("black: ");
  for (int i = 0; i < 6; i++)
  {
    SerialPort.print(params.black[i]);
    SerialPort.println(" ");
  }
  
  SerialPort.println(" ");
  
  SerialPort.print("white: ");
  for (int i = 0; i < 6; i++)
  {
    SerialPort.print(params.white[i]);
    SerialPort.println(" ");
  }
  
  SerialPort.println(" ");
  
  SerialPort.print("calculation time: ");
  SerialPort.println(calculationTime);
  calculationTime = 0;

  SerialPort.println(" ");
  
  SerialPort.print("normalised values: ");

  
  SerialPort.println(" ");

  

  SerialPort.println(" ");

  SerialPort.print("kp: ");
  SerialPort.println(params.kp);

  SerialPort.println(" ");

  // SerialPort.print("Ki: ");
 // SerialPort.println(ki);

  //SerialPort.println(" ");

  SerialPort.print("diff: ");
  SerialPort.println(params.diff);

  SerialPort.println(" ");

  SerialPort.print("power: ");
  SerialPort.println(params.power);


}
void onUnknownCommand(char *command)
{
  SerialPort.print("unknown command: \"");
  SerialPort.print(command);
  SerialPort.println("\"");
}

void onCalibrate()
{
  char* param = sCmd.next();

  if (strcmp(param, "black") == 0)
  {
    SerialPort.print("start calibrating black... ");
    for (int i = 0; i < 6; i++) params.black[i]=analogRead(sensor[i]);
    SerialPort.println("done");
  }
  else if (strcmp(param, "white") == 0)
  {
    SerialPort.print("start calibrating white... ");    
    for (int i = 0; i < 6; i++) params.white[i]=analogRead(sensor[i]);  
    SerialPort.println("done");      
  }

  EEPROM_writeAnything(0, params);
}
void onRun()
{
 runState = true;
}
void onStop()
{
  runState = false;
}
