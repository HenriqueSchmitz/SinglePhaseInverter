#import"SineWavePreCalculator.h"
#import"Arduino.h"

SineWavePreCalculator::SineWavePreCalculator(int maximumDutyCycle, float transitionDeadband){
  this->maximumDutyCycle = maximumDutyCycle;
  this->transitionDeadband = transitionDeadband;
}

int *SineWavePreCalculator::calculateDegreeDutyCycles() {
  int *dutyCycles = new int[360];
  for(int angle = 0; angle < 360; angle++) {
    int dutyCycle = this->calculateDutyCycleForAngle(angle);
    dutyCycles[angle] = dutyCycle;
  }
  return dutyCycles;
}

int *SineWavePreCalculator::calculateDegreeCycles() {
  int *cycles = new int[360];
  for(int angle = 0; angle < 360; angle++) {
    cycles[angle] = this->calculateCycleForAngle(angle);
  }
  return cycles;
}

float SineWavePreCalculator::toRadians(float valueInDegrees) {
  return (valueInDegrees/360)*(2*PI);
}

int SineWavePreCalculator::calculateDutyCycleForAngle(int angle){
  float angleSine = sin(this->toRadians(angle));
  float normalizedSine = angleSine >= 0 ? angleSine : -1*angleSine;
  if(normalizedSine <= this->transitionDeadband) {
    normalizedSine = 0;
  }
  return (int) ((1 - normalizedSine) * this->maximumDutyCycle);
}

int SineWavePreCalculator::calculateCycleForAngle(int angle){
  float angleSine = sin(this->toRadians(angle));
  if(angleSine > this->transitionDeadband) {
    return 1;
  } else if(angleSine < (-1 * this->transitionDeadband)) {
    return -1;
  } else {
    return 0;
  }
}
