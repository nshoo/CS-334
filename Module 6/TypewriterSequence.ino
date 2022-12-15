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

long hello[][3] = {
  {152, 85, 40},
  {205, 40, 50},
  {100, 80, 40},
  {100, 80, 40},
  {105, 30, 50},
  {50, 80, 40}
};

void play_step(long stepper, long arm, long finger) {
  // Retract
  finger_servo.write(140);
  stepper_goto(stepper);
  arm_servo.write(arm);
  delay(500);
  finger_servo.write(finger);
  delay(500);
  finger_servo.write(140);
  delay(500);
}

void play_sequence(long seq[][3], unsigned int len) {
  for(int i = 0; i < len; i++) {
    play_step(seq[i][0], seq[i][1], seq[i][2]);  
  }
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

  play_sequence(hello, sizeof(hello) / sizeof(hello[0]));
}

void loop() {
  
}
