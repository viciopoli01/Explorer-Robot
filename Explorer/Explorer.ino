#include <L298N.h>

/*
Defining MOTORS pins
*/
#define MAX_SPEED 255
#define MIN_SPEED 100
#define DEGREE_TIME 100

#define EN1 5
#define EN2 6
//Motor 1
#define IN1 8 
#define IN2 7
L298N motor1(EN1, IN1, IN2);

//Motor 2
#define IN3 9
#define IN4 13
L298N motor2(EN2, IN3, IN4);

/*
Defining HC-SR04 pins
*/
#define TRIG 11
#define ECHO 12
/*
Defining sensors pin output
*/
#define GAS_SENS A0 
#define LUCE_SENS A1
#define SUONO_SENS A2
/*
Defining led pin output
*/
#define GAS 2  
#define LUCE 3
#define SUONO 4

int wave[100];
#define SAMPLES 100
/*
Defining thresholds
*/
int luce_thr=0;
int gas_thr=0;
int suono_thr=0;


void setup() {
  /*
  *
  *   SPEED UP ADC
  *
  */
  ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits
  
  ADCSRA |= bit (ADPS0);                                //   2  

  
  Serial.begin(9600);
  Serial.println("Explorer robot starting...");
  
  pinMode(GAS, OUTPUT);
  pinMode(LUCE, OUTPUT);
  pinMode(SUONO, OUTPUT);
  
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  //Comment the following lines if you have already saved the thrs
  Serial.println("Thr setup starting...");
  set_thr();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  brain();
}

/*
 * thr is used to get the environment noises
*/
void set_thr(){
  delay(1000);
  report(GAS);
  int value = analogRead(GAS_SENS);
  gas_thr=value;
  Serial.println("Gas thr value : ");
  Serial.print(value);
  delay(2000);
  report(LUCE);
  value = analogRead(LUCE_SENS);
  luce_thr=value;
  Serial.println("Luce thr value : ");
  Serial.print(value);
  delay(2000);
  report(SUONO);
  value = analogRead(SUONO_SENS);
  suono_thr=value;
  Serial.println("Suono thr value : ");
  Serial.print(value);
  delay(2000);
}

/*
Base algorithm
*/
void brain(){
  if(readLight())report(0);
  delay(10);
  if(readSound())report(1);
  delay(10);
  if(readGas())report(2);
  delay(10);
  if(getDistance()<10){
    stop();
    escape();
  }
  forward();
}

int readGas(){
  int value = analogRead(GAS_SENS);
  if(value>gas_thr+100)return 1;
  return 0;
}

int readSound(){
  for(int i = 0;i<SAMPLES;i++){
    wave[i]=analogRead(SUONO_SENS);
    delayMicroseconds(80*i);//in this way it looks arduino is sampling at 1/0.00008 Hz, but to get the frequency it is not needed to reconstruct the signal
  }
  /*
  * To calculate the frequency of the signal, I find the two pitch of the wave to get the period
  * and then it's possible to find the frequency
  */
  int max1_index=findMax(wave,-1);
  int max2_index=findMax(wave,max1_index);

  double period = (max2_index-max1_index)*pow(10,-6);
  double frequency = 1.0/period;
  
  if(frequency>3500 && frequency<4500)return 1;
  return 0;
}
int findMax(int* values,int index){
  int Max = 0;
  int max_index=-1;
  if(index = -1){
    for(int i = 0;i<SAMPLES;i++){
      if(Max<values[i]){
        Max=values[i];
        max_index=i;
      }
    }
  }else{
    for(int i = index;i<SAMPLES;i++){
      if(Max<values[i]){
        Max=values[i];
        max_index=i;
      }
    }
  }
  return max_index;
}

int readLight(){
  int value = analogRead(LUCE_SENS);
  if(value>luce_thr+100)return 1;
  return 0;
}


/*
Functions to move the robot
*/
int escape_attempt=0;
void escape(){
  rotateLeft(DEGREE_TIME);
  if(getDistance()<10){
    rotateRight(2*DEGREE_TIME);
    if(getDistance()<10){
        escape_attempt++;
        if(escape_attempt>4){
          report(GAS);
          return;
        }
        escape();
    }      
  }
}

void stop(){
  motor1.stop();
  motor2.stop();
}

void forward(){
  motor1.setSpeed(MAX_SPEED);
  motor2.setSpeed(MAX_SPEED);
  
  motor1.forward();
  motor2.forward();
}

void backward(){
  motor1.setSpeed(MAX_SPEED);
  motor2.setSpeed(MAX_SPEED);
  
  motor1.backward();
  motor2.backward();
}

void rotateLeft(int rotation_time){
  motor1.setSpeed(MIN_SPEED);
  motor2.setSpeed(MIN_SPEED);
  
  motor1.forward();
  motor2.backward();

  delay(rotation_time);
}

void rotateRight(int rotation_time){
  motor1.setSpeed(MIN_SPEED);
  motor2.setSpeed(MIN_SPEED);
  
  motor1.forward();
  motor2.backward();

  delay(rotation_time);
}

/*
Function to get the distance
*/
int getDistance(){
  digitalWrite(TRIG, LOW); //To be sure to clean the remaining high signal
  delayMicroseconds(5);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  pinMode(ECHO, INPUT);
  long duration = pulseIn(ECHO, HIGH);
 
  // Convert the time into a distance
  delay(250);
  return (int)(duration/2) / 29.1; // return the distance in centimetres
}

/*
Functions to report sensors signals
*/

void report(int sensor){
  stop();
  switch(sensor){//0 luce, 1 suono, 2 gas
    case 0:
      blinkLed(LUCE);
    break;
    case 1:
      blinkLed(SUONO);
    break;
    case 2:
      blinkLed(GAS);
    break;
  }
}

void blinkLed(int led){//led is the pin where the led is plugged
  for(int i = 0; i<3;i++){
    digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(500);                       // wait for a second
    digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
    delay(500); 
  }
}

