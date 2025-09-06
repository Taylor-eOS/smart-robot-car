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

const int MIN_INNER_PWM = 20;
const int BURST_PWM = 200;
const int BURST_MS = 80;
bool rightFlip = false;
int curLeft = 0;
int curRight = 0;

int clampSigned(int v) {
  if (v > 255) return 255;
  if (v < -255) return -255;
  return v;
}

void setDirectionPins(int leftDir, int rightDir) {
  int l = leftDir;
  int r = rightDir;
  if (r != 0 && rightFlip) r = -r;
  if (l > 0) { digitalWrite(L_IN1, LOW); digitalWrite(L_IN2, HIGH); }
  else if (l < 0) { digitalWrite(L_IN1, HIGH); digitalWrite(L_IN2, LOW); }
  else { digitalWrite(L_IN1, LOW); digitalWrite(L_IN2, LOW); }
  if (r > 0) { digitalWrite(R_IN1, LOW); digitalWrite(R_IN2, HIGH); }
  else if (r < 0) { digitalWrite(R_IN1, HIGH); digitalWrite(R_IN2, LOW); }
  else { digitalWrite(R_IN1, LOW); digitalWrite(R_IN2, LOW); }
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
  curLeft = 0;
  curRight = 0;
}

void applyPwmBoth(uint8_t leftVal, uint8_t rightVal) {
  ledcWrite(ENA_CH, leftVal);
  ledcWrite(ENB_CH, rightVal);
  curLeft = leftVal;
  curRight = rightVal;
}

void rampToTargetsWithBurst(int leftTarget, int rightTarget, int stepDelay = 8) {
  leftTarget = clampSigned(leftTarget);
  rightTarget = clampSigned(rightTarget);
  int lSign = (leftTarget > 0) - (leftTarget < 0);
  int rSign = (rightTarget > 0) - (rightTarget < 0);
  int absL = abs(leftTarget);
  int absR = abs(rightTarget);
  if (absL == 0 && absR > 0) absL = min(MIN_INNER_PWM, absR);
  if (absR == 0 && absL > 0) absR = min(MIN_INNER_PWM, absL);
  setDirectionPins(lSign, rSign);
  applyPwmBoth(0, 0);
  int burstL = absL > 0 ? BURST_PWM : 0;
  int burstR = absR > 0 ? BURST_PWM : 0;
  if (burstL || burstR) {
    applyPwmBoth(burstL, burstR);
    delay(BURST_MS);
  }
  int l = min(burstL, absL);
  int r = min(burstR, absR);
  while (l != absL || r != absR) {
    if (l < absL) l++;
    else if (l > absL) l--;
    if (r < absR) r++;
    else if (r > absR) r--;
    applyPwmBoth(l, r);
    delay(stepDelay);
  }
}

void holdMs(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) delay(10);
}

void stopBothImmediate() {
  applyPwmBoth(0, 0);
  setDirectionPins(0, 0);
}

void setup() {
  pinMode(L_IN1, OUTPUT);
  pinMode(L_IN2, OUTPUT);
  pinMode(R_IN1, OUTPUT);
  pinMode(R_IN2, OUTPUT);
  Serial.begin(115200);
  attachAndZeroPwm();
  stopBothImmediate();
  unsigned long start = millis();
  Serial.println("Ready. Type 'i' within 3s to invert right motor");
  while (millis() - start < 3000 && Serial.available() == 0) delay(10);
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'i' || c == 'I') { rightFlip = !rightFlip; Serial.print("rightFlip="); Serial.println(rightFlip); }
  }
}

void loop() {
  rampToTargetsWithBurst(200, 120);
  holdMs(1000);
  rampToTargetsWithBurst(120, 200);
  holdMs(1000);
  rampToTargetsWithBurst(0, 0, 6);
  stopBothImmediate();
  delay(4000);
}

