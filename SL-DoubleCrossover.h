#ifndef SL_DOUBLECROSSOVER_H
#define SL_DOUBLECROSSOVER_H

#include "SignalLogic.h"
#include "configuration.h"
#include "utilities.h"

class DoubleCrossover : public SignalLogic
{
  public:
    void setup(XCade* xcade) override;
    void loop() override;
    void reconfigure(JsonDocument& signalConfig) override;
    void getStatusJson(JsonObject& statusResponse) override;
    static inline const char* shortName = "2xovr";
    static inline const char* longName = "Double Crossover";
    
  private:
    XCade *xcade;
    SignalMast signalMastA;
    SignalMast signalMastB;
    SignalMast signalMastC;
    SignalMast signalMastD;
    SignalRuleManager signalRuleManager;
    bool approachLighting;
    bool twoBlockApproach;
    bool isCompactCrossover;
    bool t1Invert;
    bool t2Invert;
    bool t3Invert;
    bool t4Invert;
    static const uint32_t LOOP_UPDATE_TIME_MS = 50;
};

#endif