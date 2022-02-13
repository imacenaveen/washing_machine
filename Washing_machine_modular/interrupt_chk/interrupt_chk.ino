#include <Servo.h>
#include <TimerOne.h>

Servo myservo;  // create servo object to control a servo

const int washing_motor = 8;     
const int water_drain_servo = 9;
const int water_inlet_solenoid = 10; 
const int trigPin = 11;    // TRIG pin
const int echoPin = 12;    // ECHO pin     
const int begin_wash = 8;
const int water_inlet_indicator = 7;
const int water_drain_indicator = 6;
const int wash_motor_indicator = 5;


const int water_high_level = 20; // In cm
const int water_low_level = 80; // In cm
const int time_out = 60; // 1 mins

float duration_us, distance_cm;

// variables will change:
int pos = 0;    // variable to store the servo position
int water_level = 0;         // variable for reading the pushbutton status
int begin_state = 0;         // variable for reading the pushbutton status
// the setup function runs once when you press reset or power the board
unsigned long water_fill_count = 0; // use volatile for shared variables
unsigned long water_fill_count_copy = 0; // use volatile for shared variables

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Timer1.initialize(1000);
  Timer1.attachInterrupt(fill_timer); // fill_timer to run every 1 seconds
  pinMode(washing_motor,OUTPUT);
  pinMode(water_drain_servo,OUTPUT);
  pinMode(water_inlet_solenoid,OUTPUT);
  pinMode(water_inlet_indicator,OUTPUT);
  pinMode(water_drain_indicator,OUTPUT);
  pinMode(wash_motor_indicator,OUTPUT);  
  // configure the trigger pin to output mode
  pinMode(trigPin, OUTPUT);
  // configure the echo pin to input mode
  pinMode(echoPin, INPUT);
  pinMode(begin_wash,INPUT_PULLUP);
  myservo.attach(water_drain_servo);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600);
}

void fill_timer(void){
  water_fill_count = water_fill_count + 1;  // increase every second  
}

void loop(){
  Serial.print("Interrupt water_fill_count:");
  Serial.println(water_fill_count);
  delay(1000);
}
