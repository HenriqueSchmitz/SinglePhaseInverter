#include"ControlledBootstrap.h"
#include"Arduino.h"

ControlledBootstrap::ControlledBootstrap(unsigned char controlPin, unsigned int onTime, unsigned int offTime){
  this->controlPin = controlPin;
  this->onTime = onTime;
  this->offTime = offTime;
  pinMode(this->controlPin, OUTPUT);
  // The first cycle is started with the bootstrap charging
  // to ensure it has reached proper voltage before trying to
  // provide it to the system.
  digitalWrite(this->controlPin, HIGH);
  this->cycleStartTime = micros();
}

void ControlledBootstrap::refresh(unsigned long currentTime) {
  if(this->hasCompletedCycle(currentTime)) {
    digitalWrite(this->controlPin, LOW);
    this->cycleStartTime += this->offTime + this->onTime;
  } else if(this->hasRechedTimeToTurnOn(currentTime)){
    digitalWrite(this->controlPin, HIGH);
  }
}

bool ControlledBootstrap::hasCompletedCycle(unsigned long currentTime){
  return currentTime >= (this->cycleStartTime + this->offTime + this->onTime);
}

bool ControlledBootstrap::hasRechedTimeToTurnOn(unsigned long currentTime){
  return currentTime >= (this->cycleStartTime + this->offTime);
}
