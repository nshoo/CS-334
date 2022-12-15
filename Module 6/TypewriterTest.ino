#include <Servo.h>

#define STEP_PIN 5
#define DIR_PIN 16
#define LIMIT_PIN 14
#define SERVO_PIN 4

long stepper_steps = 0;
Servo arm_servo;
Servo finger_servo;

void pulse(int pin) {
  digitalWrite(pin, HIGH);
  delayMicroseconds(500);
  digitalWrite(pin, LOW);
  delayMicroseconds(500);
}

void stepper_step(bool dir) {
  digitalWrite(DIR_PIN, dir);
  pulse(STEP_PIN);
  if(!dir) stepper_steps++;
  else if(stepper_steps > 0) stepper_steps--;
  Serial.println(stepper_steps);
}

void stepper_find_home() {
  while (digitalRead(LIMIT_PIN) != LOW) {
    stepper_step(true);
    delay(4);
  }

  stepper_steps = 0;
}

void stepper_move(bool dir, long steps) {
  while(steps-- > 0) {
      stepper_step(dir);
      delayMicroseconds(500);
  }
}

long dist_to_steps(float mm) {
  return mm / 0.18;
}

void stepper_goto(float mm) {
  long dest = dist_to_steps(mm);
  bool dir = dest < stepper_steps;
  long diff = abs(dest - stepper_steps);
  stepper_move(dir, diff);
}

void setup() {
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(LIMIT_PIN, INPUT_PULLUP);

  digitalWrite(DIR_PIN, LOW);
  arm_servo.attach(12, 500, 1600);
  finger_servo.attach(13, 500, 2400);
  Serial.begin(9600);
  Serial.setTimeout(100000);

  arm_servo.write(20);
  finger_servo.write(140);

  stepper_find_home();
  
  //stepper_move(false, 100);
  stepper_goto(100);
}

long stepper_pos = 0, arm_pos = 0, finger_pos = 0;

void print_state() {
  Serial.print("Stepper: ");
  Serial.println(stepper_pos);
  Serial.print("Arm: ");
  Serial.println(arm_pos);
  Serial.print("Finger: ");
  Serial.println(finger_pos);
}

void loop() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if(c == 'p') { print_state(); break; }
    if(!(c == 's' || c == 'a' || c == 'f')) break;
    long loc = Serial.parseInt();

    switch(c) {
      case 's': stepper_goto(loc); stepper_pos = loc; break;
      case 'a': arm_servo.write(loc); arm_pos = loc; break;
      case 'f': finger_servo.write(loc); finger_pos = loc; break;
    }
  }
}
