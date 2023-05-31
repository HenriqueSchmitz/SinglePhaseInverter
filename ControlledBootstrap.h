#ifndef ControlledBootstrap_h
#define ControlledBootstrap_h

class ControlledBootstrap {
  public:
    ControlledBootstrap(unsigned char controlPin, unsigned int onTime, unsigned int offTime);
    void refresh(unsigned long currentTime);

  private:
    unsigned char controlPin;
    unsigned int onTime;
    unsigned int offTime;
    unsigned long cycleStartTime;

    bool hasCompletedCycle(unsigned long currentTime);
    bool hasRechedTimeToTurnOn(unsigned long currentTime);
};

#endif
