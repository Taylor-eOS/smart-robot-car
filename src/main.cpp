#include <Arduino.h>

#define ENA_PIN 4
#define ENB_PIN 1
#define L_IN1 2
#define L_IN2 14
#define R_IN1 15
#define R_IN2 13
#define ENA_CH 0
#define ENB_CH 1
#define LEDC_FREQ 5000
#define LEDC_RES 8

class L298N {
public:
  enum Direction { FORWARD, BACKWARD, STOP };
  typedef void(*CallBackFunction)();
  L298N(uint8_t pinEnable, uint8_t pinIN1, uint8_t pinIN2, uint8_t ledcChannel) :  
    _pinEnable(pinEnable), _pinIN1(pinIN1), _pinIN2(pinIN2), _ledcChannel(ledcChannel) {
    _pwmVal = 0;
    _isMoving = false;
    _canMove = true;
    _lastMs = 0;
    _direction = STOP;
    pinMode(_pinIN1, OUTPUT);
    pinMode(_pinIN2, OUTPUT);
    if(_pinEnable != (uint8_t)-1) {
      ledcSetup(_ledcChannel, LEDC_FREQ, LEDC_RES);
      ledcAttachPin(_pinEnable, _ledcChannel);
      ledcWrite(_ledcChannel, 0);
    }
  }

  void setSpeed(unsigned short pwmVal) {
    if(pwmVal > 255) pwmVal = 255;
    _pwmVal = pwmVal;
  }

  unsigned short getSpeed() {
    return this->isMoving() ? _pwmVal : 0;
  }

  uint8_t getLedcChannel() {
    return _ledcChannel;
  }

  void forward() {
    digitalWrite(_pinIN1, LOW);
    digitalWrite(_pinIN2, HIGH);
    if(_pinEnable != (uint8_t)-1) ledcWrite(_ledcChannel, _pwmVal);
    _direction = FORWARD;
    _isMoving = true;
  }

  void backward() {
    digitalWrite(_pinIN1, HIGH);
    digitalWrite(_pinIN2, LOW);
    if(_pinEnable != (uint8_t)-1) ledcWrite(_ledcChannel, _pwmVal);
    _direction = BACKWARD;
    _isMoving = true;
  }

  void run(Direction direction) {
    switch(direction) {
      case BACKWARD:
        this->backward();
        break;
      case FORWARD:
        this->forward();
        break;
      case STOP:
        this->stop();
        break;
    }
  }

  void runFor(unsigned long delayMs, Direction direction, CallBackFunction callback) {
    if((_lastMs == 0) && _canMove) {
      _lastMs = millis();
      switch(direction) {
        case FORWARD:
          this->forward();
          break;
        case BACKWARD:
          this->backward();
          break;
        case STOP:
        default:
          this->stop();
          break;
      }
    }
    if(((millis() - _lastMs) > delayMs) && _canMove) {
      this->stop();
      _lastMs = 0;
      _canMove = false;
      callback();
    }
  } 

  void runFor(unsigned long delayMs, Direction direction) {
    this->runFor(delayMs, direction, fakeCallback);
  } 

  void forwardFor(unsigned long delayMs, CallBackFunction callback) {
    this->runFor(delayMs, FORWARD, callback);
  } 

  void forwardFor(unsigned long delayMs) {
    this->runFor(delayMs, FORWARD);
  } 

  void backwardFor(unsigned long delayMs, CallBackFunction callback) {
    this->runFor(delayMs, BACKWARD, callback);
  } 

  void backwardFor(unsigned long delayMs) {
    this->runFor(delayMs, BACKWARD);
  } 

  void stop() {
    digitalWrite(_pinIN1, LOW);
    digitalWrite(_pinIN2, LOW);
    if(_pinEnable != (uint8_t)-1) ledcWrite(_ledcChannel, 0);
    _direction = STOP;
    _isMoving = false;

    _pwmVal = 0;
  } 

  void reset() {
    _canMove = true;
  } 

  boolean isMoving() {
    return _isMoving;
  } 

  Direction getDirection() {
    return _direction;
  } 

  static void fakeCallback() {} 

private:
  uint8_t _pinEnable;
  uint8_t _pinIN1;
  uint8_t _pinIN2;
  uint8_t _ledcChannel;
  unsigned short _pwmVal;
  boolean _isMoving;
  boolean _canMove;
  unsigned long _lastMs;
  Direction _direction;
};

L298N leftMotor(ENA_PIN, L_IN1, L_IN2, ENA_CH);
L298N rightMotor(ENB_PIN, R_IN1, R_IN2, ENB_CH);

void setBothSpeedsImmediate(int left, int right) {
  leftMotor.setSpeed(left);
  rightMotor.setSpeed(right);
  ledcWrite(leftMotor.getLedcChannel(), left);
  ledcWrite(rightMotor.getLedcChannel(), right);
}

void rampBothMotors(int leftTarget, int rightTarget, int stepDelay = 8) {
  int currentLeft = leftMotor.getSpeed();
  int currentRight = rightMotor.getSpeed();
  while(currentLeft != leftTarget || currentRight != rightTarget) {
    if (currentLeft < leftTarget) currentLeft++;
    else if (currentLeft > leftTarget) currentLeft--;
    if (currentRight < rightTarget) currentRight++;
    else if (currentRight > rightTarget) currentRight--;
    setBothSpeedsImmediate(currentLeft, currentRight);
    delay(stepDelay);
  }
}

void driveCurveHold(int leftTarget, int rightTarget, int holdMs) {
  leftMotor.run(L298N::FORWARD);
  rightMotor.run(L298N::FORWARD);
  rampBothMotors(leftTarget, rightTarget);
  unsigned long t = millis();
  while(millis() - t < holdMs) {
    delay(10);
  }
}

void setup() {
  pinMode(L_IN1, OUTPUT);
  pinMode(L_IN2, OUTPUT);
  pinMode(R_IN1, OUTPUT);
  pinMode(R_IN2, OUTPUT);
  leftMotor.stop();
  rightMotor.stop();
  delay(400);

}

void loop() {
  driveCurveHold(200, 120, 2000);
  driveCurveHold(120, 200, 2000);
  rampBothMotors(0, 0);
  leftMotor.stop();
  rightMotor.stop();
  delay(5000);
}

