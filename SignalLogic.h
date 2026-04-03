#ifndef SIGNALLOGIC_H
#define SIGNALLOGIC_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "mss-xcade.h"
#include "SignalRuleManager.h"

class SignalLogic
{
  public:
    SignalLogic() = default;
    ~SignalLogic() = default;
    virtual void setup(XCade* xcade) = 0;
    virtual void loop() = 0;
    virtual void reconfigure(JsonDocument& signalConfig) = 0;
    virtual void getStatusJson(JsonObject& root) = 0;
    static inline const char* shortName = "";
    static inline const char* longName = "";
};

class DiagnosticLogic : public SignalLogic
{
  public:
    void setup(XCade* xcade) override;
    void loop() override;
    void getStatusJson(JsonObject& root) override;
    void reconfigure(JsonDocument& signalConfig) override;
    static inline const char* shortName = "none";
    static inline const char* longName = "Diagnostic Mode";
};

class DoubleCrossover : public SignalLogic
{
  public:
    void setup(XCade* xcade);
    void loop();
    void getStatusJson(JsonObject& root) override;
    void reconfigure(JsonDocument& signalConfig) override;
    static inline const char* shortName = "2xovr";
    static inline const char* longName = "Double Crossover";
};

#endif