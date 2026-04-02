#ifndef SIGNALLOGIC_H
#define SIGNALLOGIC_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

class SignalLogic
{
  public:
    SignalLogic() = default;
    ~SignalLogic() = default;
    virtual void setup() = 0;
    virtual void loop() = 0;
    virtual void getStatusJson(JsonObject& root) = 0;
    static inline const char* shortName = "";
    static inline const char* longName = "";
};

class DiagnosticLogic : public SignalLogic
{
  public:
    void setup() override;
    void loop() override;
    void getStatusJson(JsonObject& root) override;
    static inline const char* shortName = "none";
    static inline const char* longName = "Diagnostic Mode";
};

class DoubleCrossover : public SignalLogic
{
  public:
    void setup();
    void loop();
    void getStatusJson(JsonObject& root) override;
    static inline const char* shortName = "2xovr";
    static inline const char* longName = "Double Crossover";
};

#endif