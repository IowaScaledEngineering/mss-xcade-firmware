#ifndef SL_CPANDERS_H
#define SL_CPANDERS_H

#include "SignalLogic.h"
#include "configuration.h"
#include "utilities.h"

class CPAndersLogic : public SignalLogic
{
  public:
    void setup(XCade* xcade) override;
    void loop() override;
    void reconfigure(JsonDocument& signalConfig) override;
    void getStatusJson(JsonObject& statusResponse) override;
    static inline const char* shortName = "anders";
    static inline const char* longName = "CP Anders";
    
  private:
    XCade *xcade, *xcadeExpander1;
    XCade xcadeExpander1Obj;

    DigitalDelay irPCDelay;
    DigitalDelay ir1CDelay;
    DigitalDelay ir2ADelay;
    DigitalDelay ir2BDelay;

    SignalMast signalMast1A;
    SignalMast signalMast1B;
    SignalMast signalMast1C;
    SignalMast signalMast1D;
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
    bool t6Invert;

    bool turnout1APThrown;
    bool turnout1BPXoverThrown;
    bool turnout1CPXoverThrown;
    bool turnout1D2BThrown;
    bool turnoutP2AXoverThrown;
    bool turnoutP2BXoverThrown;

    static const uint32_t LOOP_UPDATE_TIME_MS = 50;
};

#endif