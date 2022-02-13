
#include <Servo.h>
#include <TimerOne.h>

Servo myservo;  // create servo object to control a servo

const int washing_motor = 8;     
const int water_drain_servo = 9;
const int water_inlet_solenoid = 10; 
const int trigPin = 11;    // TRIG pin
const int echoPin = 12;    // ECHO pin     

const int water_inlet_indicator = 7;
const int water_drain_indicator = 6;
const int wash_motor_indicator = 5;
const int buzzer = 4;
const int begin_wash = 2;
const int skip_soaking = 3;

const int stage1 = A1;
const int stage2 = A2;
const int stage3 = A3;

const int water_high_level = 22; // In cm
const int water_low_level = 58; // In cm
const int time_out = 150; // 2.5 mins

float duration_us, distance_cm;

// variables will change:
int fill_status = 0;
int pos = 0;    // variable to store the servo position
int water_level = 0;         // variable for reading the pushbutton status
int begin_state = 0;         // variable for reading the pushbutton status
int skip_soaking_state = 0;
int pre_state = 0;
// the setup function runs once when you press reset or power the board
volatile unsigned long water_fill_count = 0; // use volatile for shared variables
unsigned long water_fill_count_copy = 0; // use volatile for shared variables
volatile int set_stage = 0;

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
//  Timer1.initialize(1000000);
//  Timer1.attachInterrupt(fill_timer); // fill_timer to run every 1 seconds
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
  pinMode(buzzer, OUTPUT);
  pinMode(stage1,OUTPUT);
  pinMode(stage2,OUTPUT);
  pinMode(stage3,OUTPUT);
  digitalWrite(stage1, LOW);
  digitalWrite(stage2, LOW);
  digitalWrite(stage3, LOW);
  pinMode(begin_wash,INPUT_PULLUP);
  pinMode(skip_soaking,INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(begin_wash), increment_stage, FALLING);
  myservo.attach(water_drain_servo);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600);
}

void increment_stage(void){
//  noInterrupts();
//  set_stage += 1;  
//  interrupts();    
}

void buzz(int stat){
  if (stat == 0){
    //Error Buzz
    for (int i = 0; i <= 10; i += 1) { 
      digitalWrite(buzzer, HIGH);
      delay(600);
      digitalWrite(buzzer, LOW);
      delay(400);
  }
  }
  else if(stat == 1){
    //Completed Buzz
    for (int i = 0; i <= 10; i += 1) { 
      digitalWrite(buzzer, HIGH);
      delay(1000);
      digitalWrite(buzzer, LOW);
      delay(1000);
  }
  }
  else{
    //Error Buzz
    for (int i = 0; i <= 5; i += 1) { 
      digitalWrite(buzzer, HIGH);
      delay(600);
      digitalWrite(buzzer, LOW);
      delay(400);
    }
  }
}

int get_water_level(void){
  int avg_dist = 0;
  // Taking average of 5 readings
  for(int i=0;i<5;i++){
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
  
    // measure duration of pulse from ECHO pin
    duration_us = pulseIn(echoPin, HIGH);
  
    // calculate the distance
    distance_cm = 0.017 * duration_us;
    avg_dist += distance_cm;
    delay(400); 
  }
  distance_cm = avg_dist/5;
  // print the value to Serial Monitor
  Serial.print("distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");
  //delay(500);
  if ((distance_cm == 0) || (distance_cm > 80)){
    buzz(2); //Error Occured, 
    distance_cm = 100;
  }
  return distance_cm;
}

void fill_timer(void){
  water_fill_count = water_fill_count + 1;  // increase every second
  Serial.print("Interrupt water_fill_count:");
  Serial.println(water_fill_count);
}

int fill_water(void){    
  water_level = get_water_level();
  int prev_water_level = water_level;
  Serial.print("Water Level = ");
  Serial.println(water_level);    
  while ((water_level > water_high_level) && (water_fill_count_copy < time_out)){                
        digitalWrite(water_inlet_solenoid, HIGH);
        digitalWrite(water_inlet_indicator, HIGH);   // Fill water top up
        delay(2000);                       // wait for 2 seconds    
        water_level = get_water_level();
        Serial.print("Water Level = ");
        Serial.println(water_level);            
        if (prev_water_level <=  water_level){
          water_fill_count_copy += 2;
        }
        else{
          prev_water_level = water_level;
        }
        Serial.print("water_fill_count_copy = ");
        Serial.println(water_fill_count_copy);                    
      }  
  digitalWrite(water_inlet_solenoid, LOW);
  digitalWrite(water_inlet_indicator, LOW); 
  Serial.println("Water Level Full");   
  if (water_fill_count_copy >= time_out){
     water_fill_count_copy = 0;
     return 0; // Error Occured
  }
  else{
    water_fill_count_copy = 0;
    return 1;
  }
}

void wait_sometime(int setpoint){
  Serial.print("Seconds, Waiting:");
  Serial.println(setpoint);
  for (int dl = 0; dl <= setpoint; dl += 1) { 
      delay(1000);
      stage_led();
      }
}

void start_washing(int setpoint){
  digitalWrite(washing_motor, HIGH);
  digitalWrite(wash_motor_indicator, HIGH);    
  Serial.print("Seconds, Water Motor Running for:");
  Serial.println(setpoint);
  for (int dl = 0; dl <= setpoint; dl += 1) { 
  delay(1000);
  }
  digitalWrite(washing_motor, LOW);
  digitalWrite(wash_motor_indicator, LOW); 
}
void check_servo(void){
  myservo.write(170);              // tell servo to Open drain
  delay(2000);
  Serial.println("Closing Drain");      
  myservo.write(10);              // tell servo to close drain
  delay(2000);                   // waits 2s for the servo to reach the position 
}

void stage_one(void){
    fill_status = fill_water();
    if (fill_status == 0){
      Serial.println("Error Occured");
      buzz(fill_status);
      exit(0);
    }
    skip_soaking_state = digitalRead(skip_soaking);
    if(skip_soaking_state == LOW){           
      Serial.println("Waiting to soak");
      wait_sometime(300);
    }
    else{
      delay(1000);
    }
    Serial.println("Starting Washing Motor");
    start_washing(600);
    Serial.println("Opening Drain");
    myservo.write(170);             // tell servo to open drain
    //delay(2000);                    // waits 2s for the servo to
    wait_sometime(140);              // Waiting to drain water
    water_level = get_water_level();
    if(water_level < water_high_level+5){
      Serial.println("Water Drain Error");
      buzz(0);
      exit(0);
    }
    Serial.println("Closing Drain");
    myservo.write(10);              // tell servo to close drain
    delay(2000);                   // waits 1s for the servo to reach the position            
}

void stage_two(void){
      Serial.println("Running Second Loop");      
      fill_status = fill_water();
      if (fill_status == 0){
        Serial.println("Error Occured");
        buzz(fill_status);
        exit(0);
      }     
      digitalWrite(water_inlet_solenoid, HIGH);
      digitalWrite(water_inlet_indicator, HIGH);   // Fill water
      wait_sometime(30);              // Adding extra water
      digitalWrite(water_inlet_solenoid, LOW);
      digitalWrite(water_inlet_indicator, LOW);   // Stop Filling water 
      delay(1000);
      Serial.println("Starting Washing Motor");
      start_washing(300);
      delay(1000);
      Serial.println("Opening Drain");
      myservo.write(170);             // tell servo to open drain
      delay(2000);                    // waits 2s for the servo to
      wait_sometime(140);              // Waiting to drain water
      water_level = get_water_level();
      if(water_level < water_high_level+5){
        Serial.println("Water Drain Error");
        buzz(0);
        exit(0);
      }      
      Serial.println("Closing Drain");
      myservo.write(10);              // tell servo to close drain
      delay(2000);                   // waits 1s for the servo to reach the position            
}

void stage_three(void){
    Serial.println("Running Third Loop");  
    fill_status = fill_water();
    if (fill_status == 0){
      Serial.println("Error Occured");
      buzz(fill_status);
      exit(0);
    }       
    delay(1000);
//      Serial.println("Starting Washing Motor");
//      start_washing(60);
    Serial.println("Washing Completed");
    buzz(1);
}

void stage_led(void){
  if (set_stage == 1){    
    digitalWrite(stage1, HIGH);
    Serial.print("Stage 1");
  }
  else if(set_stage == 2){
    digitalWrite(stage2, HIGH); 
    Serial.print("Stage 2");
  }
  else if(set_stage == 3){
    digitalWrite(stage3, HIGH); 
    Serial.print("Stage 3");
  }  
}
// the loop function runs over and over again forever
void loop(void) {                
    // read the state of the pushbutton value:
    // check if the pushbutton is pressed. If it is, the buttonState is LOW:
    begin_state = digitalRead(begin_wash);            
    if(begin_state == LOW){
      for (int i=0;i<10;i++){
        begin_state = digitalRead(begin_wash); 
        if (begin_state == LOW && pre_state == 0){
            if (set_stage < 3){
              set_stage += 1;
            }
            Serial.print("Stage Increment:");
            Serial.println(set_stage);
            delay(1000);      
          }
        else if(begin_state == LOW){
            pre_state = 0;
            delay(1000);
          }
        else{
          delay(1000);
          pre_state = 0;
        }
        stage_led();
      }
    }
    //if (begin_state == LOW){
    if (set_stage > 0){
      Serial.println("Begin Washing");            
      stage_led();
      if (set_stage == 1){      
        stage_one();
        set_stage = 2;
      }
      stage_led();
      if(set_stage == 2){
        stage_two();
        set_stage = 3;
      }
      stage_led();
      if (set_stage == 3){
        stage_three();      
      }
      digitalWrite(washing_motor, HIGH);
      digitalWrite(wash_motor_indicator, HIGH);
      exit(0);
    }
    else{
         Serial.println("Waiting To Begin");
         delay(500);
    }
    }
