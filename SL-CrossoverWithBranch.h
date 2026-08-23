#ifndef SL_LEBANONJUNCTION_H
#define SL_LEBANONJUNCTION_H

#include "SignalLogic.h"
#include "configuration.h"
#include "utilities.h"

class CrossoverWithBranch : public SignalLogic
{
  public:
    void setup(XCade* xcade) override;
    void loop() override;
    void reconfigure(JsonDocument& signalConfig) override;
    void getStatusJson(JsonObject& statusResponse) override;
    const char* getShortName() override;
    const char* getLongName() override;
    void shutdown() override;
    static inline const char* shortName = "2to3";
    static inline const char* longName = "Crossover With Branch";
    
  private:
    XCade *xcade, *xcadeExpander1;
    XCade xcadeExpander1Obj;

    DigitalDelay ir1ADelay;
    DigitalDelay ir1BDelay;
    DigitalDelay ir1CDelay;
    DigitalDelay ir2ADelay;
    DigitalDelay ir2BDelay;

    SignalMast signalMast1A;
    SignalMast signalMast1B;
    SignalMast signalMast1C;
    SignalMast signalMast2A;
    SignalMast signalMast2B;
    SignalRuleManager signalRuleManager;
    bool approachLighting;
    bool twoBlockApproach;
    bool t1Invert;
    bool t2Invert;
    bool t3Invert;
    bool t4Invert;
    bool t5Invert;

    bool t1Thrown;
    bool t2Thrown;
    bool t3Thrown;
    bool t4Thrown;
    bool t5Thrown;

    static const uint32_t LOOP_UPDATE_TIME_MS = 50;
};

#endif