#ifndef SL_SINGLECROSSOVER_H
#define SL_SINGLECROSSOVER_H

#include "SignalLogic.h"

class SingleCrossover : public SignalLogic
{
  public:
    void setup() override;
    void loop() override;
    void getStatusJson(JsonObject& root) override;
    static inline const char* shortName = "1xovr";
    static inline const char* longName = "Single Crossover";

  private:
    SignalMast signalMastA;
    SignalMast signalMastB;
    SignalMast signalMastC;
    SignalMast signalMastD;
};

#endif