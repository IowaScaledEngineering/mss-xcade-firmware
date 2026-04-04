#include "SignalLogic.h"

extern XCade xcade;

void DiagnosticLogic::setup(XCade* xcade)
{
  this->xcade = xcade;
}

void DiagnosticLogic::loop()
{
  uint32_t currentTime = millis();
  static uint32_t lastReadTime = 0;  
  static uint8_t aspect = 0;
  // Just change the lights every 2 seconds
	if (!(((uint32_t)currentTime - lastReadTime) > 2000))
    return;

  lastReadTime = currentTime;

  aspect += 2;
  if (aspect >= ASPECT_LUNAR)
    aspect = 1;

  xcade->updateInputs();

  xcade->signals.A1.setAspect((SignalAspect_t)aspect);
  xcade->signals.B1.setAspect((SignalAspect_t)aspect);
  xcade->signals.C1.setAspect((SignalAspect_t)aspect);
  xcade->signals.D1.setAspect((SignalAspect_t)aspect);
  xcade->signals.A2.setAspect((SignalAspect_t)aspect);
  xcade->signals.B2.setAspect((SignalAspect_t)aspect);
  xcade->signals.C2.setAspect((SignalAspect_t)aspect);
  xcade->signals.D2.setAspect((SignalAspect_t)aspect);
  
  xcade->updateOutputs();


}

void DiagnosticLogic::getStatusJson(JsonObject& root)
{

}

void DiagnosticLogic::reconfigure(JsonDocument& signalConfig)
{ 

}
