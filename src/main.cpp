#include <Arduino.h>

#define ENA_PIN 4
#define ENB_PIN 16
#define L_IN1 2
#define L_IN2 14
#define R_IN1 15
#define R_IN2 13
#define ENA_CH 0
#define ENB_CH 1
#define LEDC_FREQ 5000
#define LEDC_RES 8

bool rightFlip = false;

void setDirectionRaw(bool isRight, int dir) {
  int eff = dir;
  if (isRight && rightFlip) eff = -eff;
  uint8_t a = isRight ? R_IN1 : L_IN1;
  uint8_t b = isRight ? R_IN2 : L_IN2;
  if (eff > 0) { digitalWrite(a, LOW); digitalWrite(b, HIGH); }
  else if (eff < 0) { digitalWrite(a, HIGH); digitalWrite(b, LOW); }
  else { digitalWrite(a, LOW); digitalWrite(b, LOW); }
}

void attachAndZeroPwm() {
  pinMode(ENA_PIN, OUTPUT);
  pinMode(ENB_PIN, OUTPUT);
  digitalWrite(ENA_PIN, LOW);
  digitalWrite(ENB_PIN, LOW);
  ledcSetup(ENA_CH, LEDC_FREQ, LEDC_RES);
  ledcSetup(ENB_CH, LEDC_FREQ, LEDC_RES);
  ledcAttachPin(ENA_PIN, ENA_CH);
  ledcAttachPin(ENB_PIN, ENB_CH);
  ledcWrite(ENA_CH, 0);
  ledcWrite(ENB_CH, 0);
}

void applyPwmBoth(int leftVal, int rightVal) {
  ledcWrite(ENA_CH, leftVal);
  ledcWrite(ENB_CH, rightVal);
}

void rampTo(int leftTarget, int rightTarget, int stepDelay = 6) {
  int curL = ledcRead(ENA_CH);
  int curR = ledcRead(ENB_CH);
  while (curL != leftTarget || curR != rightTarget) {
    if (curL < leftTarget) curL++;
    else if (curL > leftTarget) curL--;
    if (curR < rightTarget) curR++;
    else if (curR > rightTarget) curR--;
    applyPwmBoth(curL, curR);
    delay(stepDelay);
  }
}

void stopBoth() {
  applyPwmBoth(0, 0);
  setDirectionRaw(false, 0);
  setDirectionRaw(true, 0);
}

void setup() {
  pinMode(L_IN1, OUTPUT);
  pinMode(L_IN2, OUTPUT);
  pinMode(R_IN1, OUTPUT);
  pinMode(R_IN2, OUTPUT);
  Serial.begin(115200);
  attachAndZeroPwm();
  stopBoth();
  Serial.println("Ready. Type 'i' within 3s to invert right motor.");
  unsigned long start = millis();
  while (millis() - start < 3000 && Serial.available() == 0) delay(10);
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'i' || c == 'I') { rightFlip = !rightFlip; Serial.print("rightFlip="); Serial.println(rightFlip); }
  }
}

void loop() {
  setDirectionRaw(false, 1);
  setDirectionRaw(true, 1);
  rampTo(200, 200);
  delay(1000);
  stopBoth();
  delay(500);
  setDirectionRaw(false, -1);
  setDirectionRaw(true, 1);
  rampTo(200, 200);
  delay(1200);
  stopBoth();
  delay(800);
  setDirectionRaw(false, 1);
  setDirectionRaw(true, -1);
  rampTo(200, 200);
  delay(1200);
  stopBoth();
  delay(3000);
}

