#include "SL-SingleCrossover.h"

void SingleCrossover::reconfigure(JsonDocument& signalConfig)
{
  // Set signal types
  this->xcade->signals.B1.setSignalHeadType(SIGNAL_HEAD_THREE_LIGHT); // or SIGNAL_HEAD_SEARCHLIGHT

  this->signalMastA.setDefaultSignalRules();


  this->signalMastA.setDefaultSignalRules();
  this->signalMastA.addSignalHeads(&xcade.signals.A1);
  
  this->signalMastB.setDefaultSignalRules();
  this->signalMastB.addSignalHeads(&xcade.signals.B1, &xcade.signals.B2);
  
  this->signalMastC.setDefaultSignalRules();
  this->signalMastC.addSignalHeads(&xcade.signals.C1, &xcade.signals.C2);
  
  this->signalMastD.setDefaultSignalRules();
  this->signalMastD.addSignalHeads(&xcade.signals.D1);
}

void SingleCrossover::setup()
{

}

void SingleCrossover::loop()
{

}

void SingleCrossover::getStatusJson(JsonObject& root)
{

}

