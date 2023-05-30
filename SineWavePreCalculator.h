#ifndef SineWavePreCalculator_h
#define SineWavePreCalculator_h

// The sine wave pre-calculator is not a necessary architecture for
// the inverter to function, but calculating them before the execution
// allows avoiding costly sine calculations during execution and 
// a faster cycle time for better control.

// The transition dead band is used to create a small delay between
// turning off one direction of an H bridge and turning on the opposite
// direction. This is done by considering the small values near the
// transition as zero and is used to avoid shorting the sides of the
// H bridge.

class SineWavePreCalculator {
  public:
    SineWavePreCalculator(int maximumDutyCycle, float transitionDeadband);
    
    // Calculates the necessary power for all angles
    int *calculateDegreeDutyCycles();

    // Calculates the semi-cycle for all angles
    int *calculateDegreeCycles();

  private:
    int maximumDutyCycle;
    float transitionDeadband;

    float toRadians(float valueInDegrees);
    int calculateDutyCycleForAngle(int angle);
    int calculateCycleForAngle(int angle);
};

#endif
