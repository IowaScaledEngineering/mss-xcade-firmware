#ifndef SL_SINGLECROSSOVER_H
#define SL_SINGLECROSSOVER_H

#include "SignalLogic.h"
#include "configuration.h"
#include "utilities.h"

class SingleCrossover : public SignalLogic
{
  public:
    void setup(XCade* xcade) override;
    void loop() override;
    void reconfigure(JsonDocument& signalConfig) override;
    void getStatusJson(JsonObject& statusResponse) override;
    static inline const char* shortName = "1xovr";
    static inline const char* longName = "Single Crossover";
    
  private:
    XCade *xcade;
    SignalMast signalMastA;
    SignalMast signalMastB;
    SignalMast signalMastC;
    SignalMast signalMastD;
    SignalRuleManager signalRuleManager;
    DigitalDelay ir1ADelay;
    DigitalDelay ir1BDelay;
    DigitalDelay ir1CDelay;
    DigitalDelay ir1DDelay;

    bool approachLighting;
    bool twoBlockApproach;
    bool t1Invert;
    bool t2Invert;
    bool turnout2Normal;
    bool turnout1Normal;
    static const uint32_t LOOP_UPDATE_TIME_MS = 50;
};

#endif