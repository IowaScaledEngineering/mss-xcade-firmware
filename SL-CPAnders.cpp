#include "SL-CPAnders.h"

#define ID_SIGNAL_1A  "sig1A"
#define ID_SIGNAL_1B  "sig1B"
#define ID_SIGNAL_1C  "sig1C"
#define ID_SIGNAL_1D  "sig1D"
#define ID_SIGNAL_2A  "sig2A"
#define ID_SIGNAL_2B  "sig2B"

void CPAndersLogic::reconfigure(JsonDocument& signalConfig)
{
  Serial.printf("Starting Crossover w/Branch reconfigure()\n");

  // Make sure all five signal masts are registered with the rule manager
  signalRuleManager.registerSignal(ID_SIGNAL_1A);
  signalRuleManager.registerSignal(ID_SIGNAL_1B);
  signalRuleManager.registerSignal(ID_SIGNAL_1C);
  signalRuleManager.registerSignal(ID_SIGNAL_1D);
  signalRuleManager.registerSignal(ID_SIGNAL_2A);
  signalRuleManager.registerSignal(ID_SIGNAL_2B);

  // Go through all the config JSON for each signal to set up the rule manager
  // There's a better way to do this, but here's some AI slop that gets the job done
  JsonObject root = signalConfig.as<JsonObject>();
  for (JsonPair kv : root) 
  {
    // ArduinoJson gives us safe pointers to the internal string buffers
    const char* key = kv.key().c_str();
    // 1. Ensure the value is actually a string (not a nested object, int, or bool)
    // This prevents crashes if your JSON has something like "sig1D": { ... }
    if (kv.value().is<const char*>()) 
    {
      const char* val = kv.value().as<const char*>();
      // 2. Fast filter: check if the key looks like our target pattern
      // We expect exactly 3 hyphens in "sig1D-nor-clr-u"
      int hyphenCount = 0;
      for (const char* p = key; *p; ++p) 
      {
          if (*p == '-') hyphenCount++;
      }

      // 3. If it matches the pattern, hand it to the manager
      if (hyphenCount == 3) 
      {
        signalRuleManager.applyOverride(key, val);
      } else {
          Serial.printf("Skipping [%s]=>[%s]\n", key, val);
      }
    }
  }

  signalRuleManager.dumpRules();

  signalMast1A.addSignalHeads(&xcade->signals.A1, &xcade->signals.A2);
  signalMast1B.addSignalHeads(&xcade->signals.B1, &xcade->signals.B2);
  signalMast1C.addSignalHeads(&xcade->signals.C1, &xcade->signals.C2);
  signalMast1D.addSignalHeads(&xcade->signals.D1, &xcade->signals.D2);

  signalMast2A.addSignalHeads(&xcadeExpander1->signals.A1, &xcadeExpander1->signals.A2);
  signalMast2B.addSignalHeads(&xcadeExpander1->signals.B1, &xcadeExpander1->signals.B2);

  // Set signal rules for each mast
  signalMast1A.setDoubleHeadRules(signalRuleManager.getSignalRules(ID_SIGNAL_1A), signalRuleManager.getSignalRulesLen(ID_SIGNAL_1A));
  signalMast1B.setDoubleHeadRules(signalRuleManager.getSignalRules(ID_SIGNAL_1B), signalRuleManager.getSignalRulesLen(ID_SIGNAL_1B));
  signalMast1C.setDoubleHeadRules(signalRuleManager.getSignalRules(ID_SIGNAL_1C), signalRuleManager.getSignalRulesLen(ID_SIGNAL_1C));
  signalMast1D.setDoubleHeadRules(signalRuleManager.getSignalRules(ID_SIGNAL_1D), signalRuleManager.getSignalRulesLen(ID_SIGNAL_1D));
  signalMast2A.setDoubleHeadRules(signalRuleManager.getSignalRules(ID_SIGNAL_2A), signalRuleManager.getSignalRulesLen(ID_SIGNAL_2A));
  signalMast2B.setDoubleHeadRules(signalRuleManager.getSignalRules(ID_SIGNAL_2B), signalRuleManager.getSignalRulesLen(ID_SIGNAL_2B));

  // Set signal types
  xcade->signals.A1.setSignalHeadType(getJsonBool(signalConfig, "sig1A-searchlight-u")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.A2.setSignalHeadType(getJsonBool(signalConfig, "sig1A-searchlight-l")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.B1.setSignalHeadType(getJsonBool(signalConfig, "sig1B-searchlight-u")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.B2.setSignalHeadType(getJsonBool(signalConfig, "sig1B-searchlight-l")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.C1.setSignalHeadType(getJsonBool(signalConfig, "sig1C-searchlight-u")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.C2.setSignalHeadType(getJsonBool(signalConfig, "sig1C-searchlight-l")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.D1.setSignalHeadType(getJsonBool(signalConfig, "sig1D-searchlight-u")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.D2.setSignalHeadType(getJsonBool(signalConfig, "sig1D-searchlight-l")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  
  xcadeExpander1->signals.A1.setSignalHeadType(getJsonBool(signalConfig, "sig2A-searchlight-u")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcadeExpander1->signals.A2.setSignalHeadType(getJsonBool(signalConfig, "sig2A-searchlight-l")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcadeExpander1->signals.B1.setSignalHeadType(getJsonBool(signalConfig, "sig2B-searchlight-u")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcadeExpander1->signals.B2.setSignalHeadType(getJsonBool(signalConfig, "sig2B-searchlight-l")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);

  approachLighting = getJsonBool(signalConfig, "approach-lighting");
  twoBlockApproach = getJsonBool(signalConfig, "two-block-approach");
  t1Invert = getJsonBool(signalConfig, "t1-invert");
  t2Invert = getJsonBool(signalConfig, "t2-invert");
  t3Invert = getJsonBool(signalConfig, "t3-invert");
  t4Invert = getJsonBool(signalConfig, "t4-invert");
  t5Invert = getJsonBool(signalConfig, "t5-invert");
  t6Invert = getJsonBool(signalConfig, "t6-invert");

  uint32_t turnOnDelay = max(100UL, (uint32_t)(100.0 * getJsonFloat(signalConfig, "irdelay1s2-on", 0.1)));
  uint32_t turnOffDelay = max(1000UL, (uint32_t)(1000.0 * getJsonFloat(signalConfig, "irdelay1s2-off", 1.0)));
  irPCDelay.setDelays(turnOnDelay, turnOffDelay);


  turnOnDelay = max(100UL, (uint32_t)(100.0 * getJsonFloat(signalConfig, "irdelay1s4-on", 0.1)));
  turnOffDelay = max(1000UL, (uint32_t)(1000.0 * getJsonFloat(signalConfig, "irdelay1s4-off", 1.0)));
  ir1CDelay.setDelays(turnOnDelay, turnOffDelay);

  turnOnDelay = max(100UL, (uint32_t)(100.0 * getJsonFloat(signalConfig, "irdelay2s2-on", 0.1)));
  turnOffDelay = max(1000UL, (uint32_t)(1000.0 * getJsonFloat(signalConfig, "irdelay2s2-off", 1.0)));
  ir2ADelay.setDelays(turnOnDelay, turnOffDelay);

  turnOnDelay = max(100UL, (uint32_t)(100.0 * getJsonFloat(signalConfig, "irdelay2s4-on", 0.1)));
  turnOffDelay = max(1000UL, (uint32_t)(1000.0 * getJsonFloat(signalConfig, "irdelay2s4-off", 1.0)));
  ir2BDelay.setDelays(turnOnDelay, turnOffDelay);

  Serial.printf("Ending CP Anders reconfigure()\n");

}

void CPAndersLogic::setup(XCade* xcade)
{
  // This xcade is global, but just save it locally for encapsulation
  // This is the hardware we're literally running on
  this->xcade = xcade;
  this->xcadeExpander1 = &this->xcadeExpander1Obj;
  this->xcadeExpander1->begin(this->xcade, 1);
}

void CPAndersLogic::loop()
{
  uint32_t currentTime = millis();
  static uint32_t lastReadTime = 0;  

	if (!(((uint32_t)currentTime - lastReadTime) > SignalLogic::LOOP_UPDATE_TIME_MS))
    return;

  // First, read the input state from the hardware
  xcade->updateInputs();
  xcadeExpander1->updateInputs();

  // I'm not going to ASCII-ize the track diagram - just see the png
  // Read sensors
  turnout1APThrown = (!xcade->gpio.digitalRead(1)) ^ t1Invert;
  turnout1BPXoverThrown = (!xcade->gpio.digitalRead(2)) ^ t2Invert;
  turnout1CPXoverThrown = (!xcade->gpio.digitalRead(3)) ^ t3Invert;
  turnout1D2BThrown = (!xcade->gpio.digitalRead(4)) ^ t4Invert;
  turnoutP2AXoverThrown = (!xcade->gpio.digitalRead(5)) ^ t5Invert;
  turnoutP2BXoverThrown = (!xcade->gpio.digitalRead(6)) ^ t6Invert;

  bool block1AOccupancy = xcade->gpio.digitalRead(SENSOR_1_PIN);
  bool block1BOccupancy = xcade->gpio.digitalRead(SENSOR_3_PIN);
  bool block1COccupancy = xcade->gpio.digitalRead(SENSOR_5_PIN);
  bool block1DOccupancy = xcade->gpio.digitalRead(SENSOR_6_PIN);
  bool blockPOccupancy = xcade->gpio.digitalRead(SENSOR_9_PIN);
  bool block2AOccupancy = xcadeExpander1->gpio.digitalRead(SENSOR_1_PIN);
  bool block2BOccupancy = xcadeExpander1->gpio.digitalRead(SENSOR_3_PIN);

  bool irPCOccupancy = irPCDelay.update(xcade->gpio.digitalRead(SENSOR_2_PIN), currentTime);
  bool ir1COccupancy = ir1CDelay.update(xcade->gpio.digitalRead(SENSOR_4_PIN), currentTime);
  bool ir2AOccupancy = ir2ADelay.update(xcadeExpander1->gpio.digitalRead(SENSOR_2_PIN), currentTime);
  bool ir2BOccupancy = ir2BDelay.update(xcadeExpander1->gpio.digitalRead(SENSOR_4_PIN), currentTime);

  // Start with all signals and ports at stop.  All routes not valid are invalid
  signalMast1A.setIndication(INDICATION_STOP);
  signalMast1B.setIndication(INDICATION_STOP);
  signalMast1C.setIndication(INDICATION_STOP);
  signalMast1D.setIndication(INDICATION_STOP);
  signalMast2A.setIndication(INDICATION_STOP);
  signalMast2B.setIndication(INDICATION_STOP);

  xcade->mssPortA.cascadeFromIndication(INDICATION_STOP);  
  xcade->mssPortB.cascadeFromIndication(INDICATION_STOP);  
  xcade->mssPortC.cascadeFromIndication(INDICATION_STOP);  
  xcade->mssPortD.cascadeFromIndication(INDICATION_STOP);  
  xcadeExpander1->mssPortA.cascadeFromIndication(INDICATION_STOP);  
  xcadeExpander1->mssPortB.cascadeFromIndication(INDICATION_STOP);  

  // Since what's below only sets occupancy for workable routes, we still need to provide occupancy for 
  //  the non-viable ones as well.  So we need to at least set the occupancy for each port based on 
  //  detectors we know are associated with it.

  xcade->mssPortA.setLocalOccupancy(block1AOccupancy);
  xcade->mssPortB.setLocalOccupancy(block1BOccupancy);
  xcade->mssPortC.setLocalOccupancy(block1COccupancy || ir1COccupancy);
  xcade->mssPortD.setLocalOccupancy(block1DOccupancy);

  xcadeExpander1->mssPortA.setLocalOccupancy(block2AOccupancy || ir2AOccupancy);
  xcadeExpander1->mssPortB.setLocalOccupancy(block2BOccupancy || ir2BOccupancy);

  // Figure out if there's something on approach for approach lighting
  bool lightSignals = true;
  if (approachLighting)
  {
    if (twoBlockApproach)
    {
      lightSignals = xcade->mssPortA.getDoubleBlockApproach() || xcade->mssPortB.getDoubleBlockApproach() 
        || xcade->mssPortC.getDoubleBlockApproach() || xcade->mssPortD.getDoubleBlockApproach() || xcadeExpander1->mssPortA.getDoubleBlockApproach() 
        || xcadeExpander1->mssPortB.getDoubleBlockApproach();
    } else {
      lightSignals = xcade->mssPortA.getSingleBlockApproach() || xcade->mssPortB.getSingleBlockApproach() 
        || xcade->mssPortC.getSingleBlockApproach() || xcade->mssPortD.getSingleBlockApproach() || xcadeExpander1->mssPortA.getSingleBlockApproach()
        || xcadeExpander1->mssPortB.getSingleBlockApproach();
    }
    lightSignals |= blockPOccupancy || irPCOccupancy;
  }


  // Now, work through all tracks on the left, one by one, and connect up all valid routes
  // In there, if a route is valid, it should set it in both directions.  

  // Port 1A Valid Routes
  //  - 1A->2A
  //  - 1A->2B

  if (turnout1APThrown && !turnout1BPXoverThrown && !turnoutP2AXoverThrown)
  {
    // Route is 1A to 2A
    xcade->mssPortA.setLocalOccupancy(block1AOccupancy || blockPOccupancy || irPCOccupancy || ir2AOccupancy);      
    xcadeExpander1->mssPortA.setLocalOccupancy(blockPOccupancy || irPCOccupancy || ir2AOccupancy || block2AOccupancy);

    xcade->mssPortA.cascadeFromPort(xcadeExpander1->mssPortA, false);
    xcadeExpander1->mssPortA.cascadeFromPort(xcade->mssPortA, true);

    signalMast1A.setIndication(xcadeExpander1->mssPortA, lightSignals);
    signalMast2A.setIndication(xcade->mssPortA, DIVERGING_SLOW_SPEED, lightSignals);
  }
  else if (turnout1APThrown && !turnout1BPXoverThrown && turnoutP2AXoverThrown && turnoutP2BXoverThrown)
  {
    // Route is 1A to 2B
    xcade->mssPortA.setLocalOccupancy(block1AOccupancy || blockPOccupancy || irPCOccupancy || ir2BOccupancy);      
    xcadeExpander1->mssPortB.setLocalOccupancy(blockPOccupancy || irPCOccupancy || ir2BOccupancy || block2BOccupancy);

    xcade->mssPortA.cascadeFromPort(xcadeExpander1->mssPortB, true);
    xcadeExpander1->mssPortB.cascadeFromPort(xcade->mssPortA, true);

    signalMast1A.setIndication(xcadeExpander1->mssPortB, DIVERGING_MEDIUM_SPEED, lightSignals);
    signalMast2B.setIndication(xcade->mssPortA, DIVERGING_SLOW_SPEED, lightSignals);
  } 

  // Port 1B Valid Routes
  //  - 1B->2A
  //  - 1B->2B

  if (!turnout1APThrown && !turnout1BPXoverThrown && !turnoutP2AXoverThrown)
  {
    // Route is 1B to 2A
    xcade->mssPortB.setLocalOccupancy(block1BOccupancy || blockPOccupancy || irPCOccupancy || ir2AOccupancy);      
    xcadeExpander1->mssPortA.setLocalOccupancy(blockPOccupancy || irPCOccupancy || ir2AOccupancy || block2AOccupancy);

    xcade->mssPortB.cascadeFromPort(xcadeExpander1->mssPortA, false);
    xcadeExpander1->mssPortA.cascadeFromPort(xcade->mssPortB, false);

    signalMast1B.setIndication(xcadeExpander1->mssPortA, lightSignals);
    signalMast2A.setIndication(xcade->mssPortB, lightSignals);
  }
  else if (!turnout1APThrown && !turnout1BPXoverThrown && turnoutP2AXoverThrown && turnoutP2BXoverThrown)
  {
    // Route is 1B to 2B
    xcade->mssPortB.setLocalOccupancy(block1BOccupancy || blockPOccupancy || irPCOccupancy || ir2BOccupancy);      
    xcadeExpander1->mssPortB.setLocalOccupancy(blockPOccupancy || irPCOccupancy || ir2BOccupancy || block2BOccupancy);

    xcade->mssPortB.cascadeFromPort(xcadeExpander1->mssPortB, true);
    xcadeExpander1->mssPortB.cascadeFromPort(xcade->mssPortB, true);

    signalMast1B.setIndication(xcadeExpander1->mssPortB, DIVERGING_FULL_SPEED, lightSignals);
    signalMast2B.setIndication(xcade->mssPortB, DIVERGING_FULL_SPEED, lightSignals);
  }

  // 1C valid routes:
  //  - 1C to 2A via the crossover
  //  - 1C to 2B via both crossover legs
  //  - 1C to 2B straight through
  if (turnout1CPXoverThrown && turnout1BPXoverThrown && !turnoutP2AXoverThrown)
  {
    // Route is 1C to 2A
      xcade->mssPortC.setLocalOccupancy(block1COccupancy || blockPOccupancy || ir2AOccupancy || ir1COccupancy);      
      xcadeExpander1->mssPortA.setLocalOccupancy(block2AOccupancy || blockPOccupancy || ir2AOccupancy || ir1COccupancy);

      xcade->mssPortC.cascadeFromPort(xcadeExpander1->mssPortA, true);
      xcadeExpander1->mssPortA.cascadeFromPort(xcade->mssPortC, true);

      signalMast1C.setIndication(xcadeExpander1->mssPortA, DIVERGING_FULL_SPEED, lightSignals);
      signalMast2A.setIndication(xcade->mssPortC, DIVERGING_FULL_SPEED, lightSignals);
  } 
  else if (turnout1CPXoverThrown && turnout1BPXoverThrown && turnoutP2AXoverThrown && turnoutP2BXoverThrown)
  {
    // Route is the dumb route - 1C to 2B via both crossover legs
      xcade->mssPortC.setLocalOccupancy(block1COccupancy || blockPOccupancy || ir2BOccupancy || ir1COccupancy);      
      xcadeExpander1->mssPortB.setLocalOccupancy(block2BOccupancy || blockPOccupancy || ir2BOccupancy || ir1COccupancy);

      xcade->mssPortC.cascadeFromPort(xcadeExpander1->mssPortB, true);
      xcadeExpander1->mssPortB.cascadeFromPort(xcade->mssPortC, true);

      signalMast1C.setIndication(xcadeExpander1->mssPortB, DIVERGING_MEDIUM_SPEED, lightSignals);
      signalMast2B.setIndication(xcade->mssPortC, DIVERGING_MEDIUM_SPEED, lightSignals);
  } 
  else if (!turnout1CPXoverThrown && !turnoutP2BXoverThrown && !turnout1D2BThrown)
  {
    // Route is straight through - 1C to 2B
      xcade->mssPortC.setLocalOccupancy(block1COccupancy || ir1COccupancy || ir2BOccupancy);      
      xcadeExpander1->mssPortB.setLocalOccupancy(block2BOccupancy || ir1COccupancy || ir2BOccupancy);

      xcade->mssPortC.cascadeFromPort(xcadeExpander1->mssPortB, false);
      xcadeExpander1->mssPortB.cascadeFromPort(xcade->mssPortC, false);

      signalMast1C.setIndication(xcadeExpander1->mssPortB, lightSignals);
      signalMast2B.setIndication(xcade->mssPortC, lightSignals);
  }

  // 1D valid routes:
  //  - 1D to 2B
  if (turnout1D2BThrown && !turnoutP2BXoverThrown)
  {
    // Route is 1C to 2A
      xcade->mssPortD.setLocalOccupancy(block1DOccupancy || ir2BOccupancy);      
      xcadeExpander1->mssPortB.setLocalOccupancy(block2BOccupancy || ir2BOccupancy);      

      xcade->mssPortD.cascadeFromPort(xcadeExpander1->mssPortB, false);
      xcadeExpander1->mssPortB.cascadeFromPort(xcade->mssPortD, true);

      signalMast1D.setIndication(xcadeExpander1->mssPortB, lightSignals);
      signalMast2B.setIndication(xcade->mssPortD, DIVERGING_SLOW_SPEED, lightSignals);
  }



  // Now that all state is computed, send the outputs to the hardware
  xcade->updateOutputs();
  xcadeExpander1->updateOutputs();
}


void CPAndersLogic::getStatusJson(JsonObject& statusResponse)
{
  // The JS on the other side is expecting things in the form of:
  //  mss1a-s-in, mss1a-a-out, as booleans - true for active, false for inactive

  // This is really, hideously not threadsafe
  mssPortToStatusJson(statusResponse, xcade->mssPortA, "mss1a");
  mssPortToStatusJson(statusResponse, xcade->mssPortB, "mss1b");
  mssPortToStatusJson(statusResponse, xcade->mssPortC, "mss1c");
  mssPortToStatusJson(statusResponse, xcade->mssPortD, "mss1d");

  mssPortToStatusJson(statusResponse, xcadeExpander1->mssPortA, "mss2a");
  mssPortToStatusJson(statusResponse, xcadeExpander1->mssPortB, "mss2b");

  mssGPIOToJson(statusResponse, xcade->gpio, "gpio1", 6);

  mssSensorsToJson(statusResponse, xcade->gpio, "sensor1", 9);
  mssSensorsToJson(statusResponse, xcadeExpander1->gpio, "sensor2", 4);

  mssTurnoutToJson(statusResponse, "to1", turnout1APThrown);
  mssTurnoutToJson(statusResponse, "to2", turnout1BPXoverThrown);
  mssTurnoutToJson(statusResponse, "to3", turnout1CPXoverThrown);
  mssTurnoutToJson(statusResponse, "to4", turnout1D2BThrown);
  mssTurnoutToJson(statusResponse, "to5", turnoutP2AXoverThrown);
  mssTurnoutToJson(statusResponse, "to6", turnoutP2BXoverThrown);

  mssSignalHeadsToJson(statusResponse, ID_SIGNAL_1A, &xcade->signals.A1, &xcade->signals.A2);
  mssSignalHeadsToJson(statusResponse, ID_SIGNAL_1B, &xcade->signals.B1, &xcade->signals.B2);
  mssSignalHeadsToJson(statusResponse, ID_SIGNAL_1C, &xcade->signals.C1, &xcade->signals.C2);
  mssSignalHeadsToJson(statusResponse, ID_SIGNAL_1D, &xcade->signals.D1, &xcade->signals.D2);
  mssSignalHeadsToJson(statusResponse, ID_SIGNAL_2A, &xcadeExpander1->signals.A1, &xcadeExpander1->signals.A2);
  mssSignalHeadsToJson(statusResponse, ID_SIGNAL_2B, &xcadeExpander1->signals.B1, &xcadeExpander1->signals.B2);


  return;
}

