const int PWMFreq = 5000;
const int PWMChannelLeft = 0;
const int PWMPinLeft = 27;
const int HighMosfetPinLeft = 12;
const int PWMChannelRight = 1;
const int PWMPinRight = 26;
const int HighMosfetPinRight = 13;
const int PWMResolution = 12;
const int MAX_DUTY_CYCLE = (int)(pow(2, PWMResolution) - 1);
const int microsInSecond = 1000000;
float startingFrequency = 0;
float targetFrequency = 0;
float outputFrequency = 0;
float outputPeriodMicros = (1/outputFrequency)*microsInSecond;
unsigned long cycleStartTime = 0;
unsigned long bootstrapStartTime = 0;
unsigned long rampStartTime = 0;
const int rampTimeMicros = 5*microsInSecond;
const int bootstrapOffTime = 100000; //us
const int bootstrapOnTime = 2000; //us
const int bootstrapPin = 15;
const float transitionDeadband = 0.1;

int lastCycle = 0;

TaskHandle_t pwmSineGeneratorTask;

int *degreeDutyCycles;
int *degreeCycles;

void setup()
{  
  Serial.begin(115200);
  pinMode(bootstrapPin, OUTPUT);
  digitalWrite(bootstrapPin, HIGH);
  pinMode(HighMosfetPinLeft, OUTPUT);
  digitalWrite(HighMosfetPinLeft, LOW);
  pinMode(HighMosfetPinRight, OUTPUT);
  digitalWrite(HighMosfetPinRight, LOW);
  degreeDutyCycles = calculateDegreeDutyCycles();
  degreeCycles = calculateDegreeCycles();
  ledcSetup(PWMChannelLeft, PWMFreq, PWMResolution);
  ledcAttachPin(PWMPinLeft, PWMChannelLeft);
  ledcSetup(PWMChannelRight, PWMFreq, PWMResolution);
  ledcAttachPin(PWMPinRight, PWMChannelRight);
  xTaskCreatePinnedToCore(
      pwmLoop, /* Function to implement the task */
      "pwmSineGenerator", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &pwmSineGeneratorTask,  /* Task handle. */
      0); /* Core where the task should run */
  cycleStartTime = micros();
  bootstrapStartTime = cycleStartTime;
}

void loop()
{
  unsigned long currentTime = micros();
  readSerialControl(currentTime);
  implementRamp(currentTime);
  vTaskDelay(1);
}

void pwmLoop(void * pvParameters) {
  while(1) {
    unsigned long currentTime = micros();
    updatePwm(currentTime);
    controlBootstrap(currentTime);
    vTaskDelay(1);
  }
}

int *calculateDegreeDutyCycles() {
  int *dutyCycles = new int[360];
  for(int degree = 0; degree < 360; degree++) {
    float angleSine = sin(toRadians(degree));
    float normalizedSine = angleSine >= 0 ? angleSine : -1*angleSine;
    if(normalizedSine <= transitionDeadband) {
      normalizedSine = 0;
    }
    int dutyCycle = (int) ((1 - normalizedSine) * MAX_DUTY_CYCLE);
    dutyCycles[degree] = dutyCycle;
  }
  return dutyCycles;
}

int *calculateDegreeCycles() {
  int *cycles = new int[360];
  for(int degree = 0; degree < 360; degree++) {
    float angleSine = sin(toRadians(degree));
    if(angleSine > transitionDeadband) {
      cycles[degree] = 1;
    } else if(angleSine < (-1*transitionDeadband)) {
      cycles[degree] = -1;
    } else {
      cycles[degree] = 0;
    }
  }
  return cycles;
}

float toRadians(float valueInDegrees) {
  return (valueInDegrees/360)*(2*PI);
}

void updatePwm(unsigned long currentTime) {
  if(outputFrequency > 0) {
    while(currentTime > (cycleStartTime + outputPeriodMicros)){
      cycleStartTime += outputPeriodMicros;
    }
    float degreeWithinCycle = ((currentTime - cycleStartTime)/outputPeriodMicros)*360;
    int degree = (int)round(degreeWithinCycle);
    int dutyCycle = degreeDutyCycles[degree];
    int cycle = degreeCycles[degree];
    if(cycle != lastCycle){
      if(cycle == 1) {
        digitalWrite(HighMosfetPinLeft, LOW);
        ledcWrite(PWMChannelRight, MAX_DUTY_CYCLE);
        delayMicroseconds(2);
        digitalWrite(HighMosfetPinRight, HIGH);
      } else if(cycle == -1) {
        digitalWrite(HighMosfetPinRight, LOW);
        ledcWrite(PWMChannelLeft, MAX_DUTY_CYCLE);
        delayMicroseconds(2);
        digitalWrite(HighMosfetPinLeft, HIGH);
      } else {
        digitalWrite(HighMosfetPinLeft, LOW);
        digitalWrite(HighMosfetPinRight, LOW);
        ledcWrite(PWMChannelLeft, MAX_DUTY_CYCLE);
        ledcWrite(PWMChannelRight, MAX_DUTY_CYCLE);
        delayMicroseconds(2);
      }
    }
    if(cycle == 1) {
      ledcWrite(PWMChannelLeft, dutyCycle);
    } else if(cycle == -1) {
      ledcWrite(PWMChannelRight, dutyCycle);
    }
    lastCycle = cycle;
//    delayMicroseconds(200);
  } else {
    ledcWrite(PWMChannelLeft, MAX_DUTY_CYCLE);
    ledcWrite(PWMChannelRight, MAX_DUTY_CYCLE);
  }
}

void readSerialControl(unsigned long currentTime) {
  if (Serial.available() > 0) {
    startingFrequency = outputFrequency;
    String incomingValue = Serial.readString();
    targetFrequency = incomingValue.toFloat();
    Serial.println("Command received");
    Serial.print("Frequency: ");
    Serial.print(targetFrequency);
    Serial.println("Hz");
    if(outputFrequency == 0) {
      cycleStartTime = currentTime;
    }
    rampStartTime = currentTime;
  }
}

void implementRamp(unsigned long currentTime) {
  if(currentTime < (rampStartTime + rampTimeMicros)) {
    float rampProgression = ((float)(currentTime - rampStartTime))/(float)rampTimeMicros;
    outputFrequency = startingFrequency + (targetFrequency - startingFrequency)*rampProgression;
    outputPeriodMicros = (1/outputFrequency)*microsInSecond;
  } else {
    outputFrequency = targetFrequency;
    outputPeriodMicros = (1/outputFrequency)*microsInSecond;
  }
}

void controlBootstrap(unsigned long currentTime) {
  if(currentTime >= (bootstrapStartTime + bootstrapOffTime + bootstrapOnTime)) {
    digitalWrite(bootstrapPin, LOW);
    bootstrapStartTime += bootstrapOffTime + bootstrapOnTime;
  } else if(currentTime >= (bootstrapStartTime + bootstrapOffTime)){
    digitalWrite(bootstrapPin, HIGH);
  }
}
