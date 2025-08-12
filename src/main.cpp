#include <Arduino.h>

const int R_IN1 = 2;
const int R_IN2 = 14;
const int L_IN1 = 15;
const int L_IN2 = 13;
const int EN_R = -1;
const int EN_L = -1;

void stopRight();
void stopLeft();
void stopAll();
void setRight(int a, int b);
void setLeft(int a, int b);

void setup() {
  Serial.begin(115200);
  pinMode(R_IN1, OUTPUT);
  pinMode(R_IN2, OUTPUT);
  pinMode(L_IN1, OUTPUT);
  pinMode(L_IN2, OUTPUT);
  if (EN_R != -1) {
    pinMode(EN_R, OUTPUT);
    digitalWrite(EN_R, LOW);
  }
  if (EN_L != -1) {
    pinMode(EN_L, OUTPUT);
    digitalWrite(EN_L, LOW);
  }
  stopAll();
  delay(1000);
  Serial.println("Starting motor test sequence");
}

void loop() {
  Serial.println("Testing right motor: polarity A (IN1 HIGH, IN2 LOW)");
  setRight(HIGH, LOW);
  delay(2000);
  stopRight();
  delay(500);

  Serial.println("Testing right motor: polarity B (IN1 LOW, IN2 HIGH)");
  setRight(LOW, HIGH);
  delay(2000);
  stopRight();
  delay(1000);

  Serial.println("Testing left motor: polarity A (IN1 HIGH, IN2 LOW)");
  setLeft(HIGH, LOW);
  delay(2000);
  stopLeft();
  delay(500);

  Serial.println("Testing left motor: polarity B (IN1 LOW, IN2 HIGH)");
  setLeft(LOW, HIGH);
  delay(2000);
  stopLeft();
  delay(1000);

  Serial.println("Testing both motors forward (same polarity)");
  setRight(HIGH, LOW);
  setLeft(HIGH, LOW);
  delay(3000);
  stopAll();
  delay(1000);

  Serial.println("Testing both motors reverse (opposite polarity)");
  setRight(LOW, HIGH);
  setLeft(LOW, HIGH);
  delay(3000);
  stopAll();
  Serial.println("Sequence complete. Pausing before repeating.");
  delay(5000);
}

void setRight(int a, int b) {
  digitalWrite(R_IN1, a);
  digitalWrite(R_IN2, b);
  if (EN_R != -1) digitalWrite(EN_R, HIGH);
}

void setLeft(int a, int b) {
  digitalWrite(L_IN1, a);
  digitalWrite(L_IN2, b);
  if (EN_L != -1) digitalWrite(EN_L, HIGH);
}

void stopRight() {
  digitalWrite(R_IN1, LOW);
  digitalWrite(R_IN2, LOW);
  if (EN_R != -1) digitalWrite(EN_R, LOW);
}

void stopLeft() {
  digitalWrite(L_IN1, LOW);
  digitalWrite(L_IN2, LOW);
  if (EN_L != -1) digitalWrite(EN_L, LOW);
}

void stopAll() {
  stopRight();
  stopLeft();
}

