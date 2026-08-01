#include "SL-CrossoverWithBranch.h"

#define ID_SIGNAL_1A  "sig1A"
#define ID_SIGNAL_1B  "sig1B"
#define ID_SIGNAL_1C  "sig1C"
#define ID_SIGNAL_2A  "sig2A"
#define ID_SIGNAL_2B  "sig2B"

void CrossoverWithBranch::reconfigure(JsonDocument& signalConfig)
{
  Serial.printf("Starting CrossoverWithBranch reconfigure()\n");

  // Make sure all five signal masts are registered with the rule manager
  signalRuleManager.registerSignal(ID_SIGNAL_1A);
  signalRuleManager.registerSignal(ID_SIGNAL_1B);
  signalRuleManager.registerSignal(ID_SIGNAL_1C);
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

  signalMast2A.addSignalHeads(&xcadeExpander1->signals.A1, &xcadeExpander1->signals.A2, &xcadeExpander1->signals.C1);
  signalMast2B.addSignalHeads(&xcadeExpander1->signals.B1, &xcadeExpander1->signals.B2, &xcadeExpander1->signals.C2);

  // Set signal rules for each mast
  signalMast1A.setDoubleHeadRules(signalRuleManager.getSignalRules(ID_SIGNAL_1A), signalRuleManager.getSignalRulesLen(ID_SIGNAL_1A));
  signalMast1B.setDoubleHeadRules(signalRuleManager.getSignalRules(ID_SIGNAL_1B), signalRuleManager.getSignalRulesLen(ID_SIGNAL_1B));
  signalMast1C.setDoubleHeadRules(signalRuleManager.getSignalRules(ID_SIGNAL_1C), signalRuleManager.getSignalRulesLen(ID_SIGNAL_1C));
  signalMast2A.setTripleHeadRules(signalRuleManager.getSignalRules(ID_SIGNAL_2A), signalRuleManager.getSignalRulesLen(ID_SIGNAL_2A));
  signalMast2B.setTripleHeadRules(signalRuleManager.getSignalRules(ID_SIGNAL_2B), signalRuleManager.getSignalRulesLen(ID_SIGNAL_2B));

  // Set signal types
  xcade->signals.A1.setSignalHeadType(getJsonBool(signalConfig, "sig1A-searchlight-u")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.A2.setSignalHeadType(getJsonBool(signalConfig, "sig1A-searchlight-l")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.B1.setSignalHeadType(getJsonBool(signalConfig, "sig1B-searchlight-u")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.B2.setSignalHeadType(getJsonBool(signalConfig, "sig1B-searchlight-l")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.C1.setSignalHeadType(getJsonBool(signalConfig, "sig1C-searchlight-u")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.C2.setSignalHeadType(getJsonBool(signalConfig, "sig1C-searchlight-l")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  
  xcadeExpander1->signals.A1.setSignalHeadType(getJsonBool(signalConfig, "sig2A-searchlight-u")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcadeExpander1->signals.A2.setSignalHeadType(getJsonBool(signalConfig, "sig2A-searchlight-l")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcadeExpander1->signals.C1.setSignalHeadType(getJsonBool(signalConfig, "sig2A-searchlight-3")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);

  xcadeExpander1->signals.B1.setSignalHeadType(getJsonBool(signalConfig, "sig2B-searchlight-u")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcadeExpander1->signals.B2.setSignalHeadType(getJsonBool(signalConfig, "sig2B-searchlight-l")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcadeExpander1->signals.C2.setSignalHeadType(getJsonBool(signalConfig, "sig2B-searchlight-3")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);

  approachLighting = getJsonBool(signalConfig, "approach-lighting");
  twoBlockApproach = getJsonBool(signalConfig, "two-block-approach");
  t1Invert = getJsonBool(signalConfig, "t1-invert");
  t2Invert = getJsonBool(signalConfig, "t2-invert");
  t3Invert = getJsonBool(signalConfig, "t3-invert");
  t4Invert = getJsonBool(signalConfig, "t4-invert");
  t5Invert = getJsonBool(signalConfig, "t5-invert");

  Serial.printf("Ending Lebanon Junction reconfigure()\n");

}

void CrossoverWithBranch::setup(XCade* xcade)
{
  // This xcade is global, but just save it locally for encapsulation
  // This is the hardware we're literally running on
  this->xcade = xcade;
  this->xcadeExpander1 = &this->xcadeExpander1Obj;
  this->xcadeExpander1->begin(this->xcade, 1);
}

void CrossoverWithBranch::loop()
{
  uint32_t currentTime = millis();
  static uint32_t lastReadTime = 0;  

	if (!(((uint32_t)currentTime - lastReadTime) > SignalLogic::LOOP_UPDATE_TIME_MS))
    return;

  // First, read the input state from the hardware
  xcade->updateInputs();
  xcadeExpander1->updateInputs();

  // Read turnouts
  // This is a little screwy, because the logic was all written initially with normal being low and
  //  thrown being open/high.  I'm reversing that to match all the other logic

  bool t1Thrown = (!xcadeExpander1->gpio.digitalRead(1)) ^ t1Invert;
  bool t2Thrown = (!xcadeExpander1->gpio.digitalRead(2)) ^ t2Invert;
  bool t3Thrown = (!xcade->gpio.digitalRead(1)) ^ t3Invert;
  bool t4Thrown = (!xcade->gpio.digitalRead(2)) ^ t4Invert;
  bool t5Thrown = (!xcade->gpio.digitalRead(3)) ^ t5Invert;

  // Read sensors
  bool block1AOccupancy = xcade->gpio.digitalRead(SENSOR_1_PIN);
  bool block1BOccupancy = xcade->gpio.digitalRead(SENSOR_3_PIN);
  bool block1COccupancy = xcade->gpio.digitalRead(SENSOR_5_PIN);
  bool block2AOccupancy = xcadeExpander1->gpio.digitalRead(SENSOR_1_PIN);
  bool block2BOccupancy = xcadeExpander1->gpio.digitalRead(SENSOR_3_PIN);
  // P1 is the block between 1A, 1C, and 2A
  bool blockP1Occupancy = xcade->gpio.digitalRead(SENSOR_8_PIN);
  // P2 is the block between 1B and 2B
  bool blockP2Occupancy = xcade->gpio.digitalRead(SENSOR_10_PIN);

  bool ir1AOccupancy = ir1ADelay.update(xcade->gpio.digitalRead(SENSOR_2_PIN), currentTime);
  bool ir1BOccupancy = ir1BDelay.update(xcade->gpio.digitalRead(SENSOR_4_PIN), currentTime);
  bool ir1COccupancy = ir1CDelay.update(xcade->gpio.digitalRead(SENSOR_6_PIN), currentTime);
  bool ir2AOccupancy = ir2ADelay.update(xcadeExpander1->gpio.digitalRead(SENSOR_2_PIN), currentTime);
  bool ir2BOccupancy = ir2BDelay.update(xcadeExpander1->gpio.digitalRead(SENSOR_4_PIN), currentTime);


  // Figure out if there's something on approach for approach lighting
  bool lightSignals = true;
  if (approachLighting)
  {
    if (twoBlockApproach)
    {
      lightSignals = xcade->mssPortA.getDoubleBlockApproach() || xcade->mssPortB.getDoubleBlockApproach() 
        || xcade->mssPortC.getDoubleBlockApproach() || xcadeExpander1->mssPortA.getDoubleBlockApproach() 
        || xcadeExpander1->mssPortB.getDoubleBlockApproach();
    } else {
      lightSignals = xcade->mssPortA.getSingleBlockApproach() || xcade->mssPortB.getSingleBlockApproach() 
        || xcade->mssPortC.getSingleBlockApproach() || xcadeExpander1->mssPortA.getSingleBlockApproach()
        || xcadeExpander1->mssPortB.getSingleBlockApproach();
    }
    lightSignals |= blockP1Occupancy || blockP2Occupancy;
  }

  // Start with all signals and ports at stop.  All routes not valid are invalid
  signalMast1A.setIndication(INDICATION_STOP, lightSignals);
  signalMast1B.setIndication(INDICATION_STOP, lightSignals);
  signalMast1C.setIndication(INDICATION_STOP, lightSignals);
  signalMast2A.setIndication(INDICATION_STOP, lightSignals);
  signalMast2B.setIndication(INDICATION_STOP, lightSignals);

  xcade->mssPortA.cascadeFromIndication(INDICATION_STOP);  
  xcade->mssPortB.cascadeFromIndication(INDICATION_STOP);  
  xcade->mssPortC.cascadeFromIndication(INDICATION_STOP);  
  xcadeExpander1->mssPortA.cascadeFromIndication(INDICATION_STOP);  
  xcadeExpander1->mssPortB.cascadeFromIndication(INDICATION_STOP);  

  // Set basic occupancy - even though a port at stop will send approach, we also may
  //  have something tripping the occupancy sensors, which would mean we need to send stop
  xcade->mssPortA.setLocalOccupancy(block1AOccupancy || ir1AOccupancy);
  xcade->mssPortB.setLocalOccupancy(block1BOccupancy || ir1BOccupancy);
  xcade->mssPortC.setLocalOccupancy(block1COccupancy || ir1COccupancy);

  xcadeExpander1->mssPortA.setLocalOccupancy(block2AOccupancy || ir2AOccupancy);
  xcadeExpander1->mssPortB.setLocalOccupancy(block2BOccupancy || ir2BOccupancy);

  if (!t1Thrown && !t3Thrown && !t5Thrown)
  {
    // 2A->1A the smart way
    xcadeExpander1->mssPortA.setLocalOccupancy(blockP1Occupancy || ir2AOccupancy || ir1AOccupancy || block2AOccupancy);
    xcade->mssPortA.setLocalOccupancy(blockP1Occupancy || ir2AOccupancy || ir1AOccupancy || block1AOccupancy);

    xcadeExpander1->mssPortA.cascadeFromPort(xcade->mssPortA);
    xcade->mssPortA.cascadeFromPort(xcadeExpander1->mssPortA);

    signalMast2A.setIndication(xcade->mssPortA, lightSignals);
    signalMast1A.setIndication(xcadeExpander1->mssPortA, lightSignals);
  }
  else if (!t1Thrown && !t3Thrown && t5Thrown)
  {
    // 2A->1C the smart way
    xcadeExpander1->mssPortA.setLocalOccupancy(blockP1Occupancy || ir2AOccupancy || ir1AOccupancy || block2AOccupancy);
    xcade->mssPortC.setLocalOccupancy(blockP1Occupancy || ir2AOccupancy || ir1COccupancy || block1COccupancy);

    xcadeExpander1->mssPortA.cascadeFromPort(xcade->mssPortC, true);
    xcade->mssPortC.cascadeFromPort(xcadeExpander1->mssPortA, false);

    signalMast2A.setIndication(xcade->mssPortC, DIVERGING_SLOW_SPEED, lightSignals);
    signalMast1C.setIndication(xcadeExpander1->mssPortA, lightSignals);
  }
  else if (t1Thrown && t2Thrown && t3Thrown && t4Thrown && !t5Thrown)
  {
    // 2A->1A the dumb way
    xcadeExpander1->mssPortA.setLocalOccupancy(blockP1Occupancy || blockP2Occupancy || ir2AOccupancy || ir1AOccupancy || block2AOccupancy);
    xcade->mssPortA.setLocalOccupancy(blockP1Occupancy || blockP2Occupancy || ir2AOccupancy || ir1AOccupancy || block1AOccupancy);

    xcadeExpander1->mssPortA.cascadeFromPort(xcade->mssPortA, true);
    xcade->mssPortA.cascadeFromPort(xcadeExpander1->mssPortA, true);

    signalMast2A.setIndication(xcade->mssPortA, DIVERGING_FULL_SPEED, lightSignals);
    signalMast1A.setIndication(xcadeExpander1->mssPortA, DIVERGING_FULL_SPEED, lightSignals);
  }
  else if (t1Thrown && t2Thrown && t3Thrown && t4Thrown && t5Thrown)
  {
    // 2A->1C the dumb way
    xcadeExpander1->mssPortA.setLocalOccupancy(blockP1Occupancy || blockP2Occupancy || ir2AOccupancy || ir1COccupancy || block2AOccupancy);
    xcade->mssPortC.setLocalOccupancy(blockP1Occupancy || blockP2Occupancy || ir2AOccupancy || ir1COccupancy || block1COccupancy);

    xcadeExpander1->mssPortA.cascadeFromPort(xcade->mssPortC, true);
    xcade->mssPortC.cascadeFromPort(xcadeExpander1->mssPortA, true);

    signalMast2A.setIndication(xcade->mssPortC, DIVERGING_SLOW_SPEED, lightSignals);
    signalMast1C.setIndication(xcadeExpander1->mssPortA, DIVERGING_FULL_SPEED, lightSignals);
  }
  else if (t1Thrown && t2Thrown && !t4Thrown)
  {
    // 2A->1B 
    xcadeExpander1->mssPortA.setLocalOccupancy(blockP2Occupancy || ir2AOccupancy || ir1BOccupancy || block2AOccupancy);
    xcade->mssPortB.setLocalOccupancy(blockP2Occupancy || ir2AOccupancy || ir1BOccupancy || block1BOccupancy);

    xcadeExpander1->mssPortA.cascadeFromPort(xcade->mssPortB, true);
    xcade->mssPortB.cascadeFromPort(xcadeExpander1->mssPortA, true);

    signalMast2A.setIndication(xcade->mssPortB, DIVERGING_FULL_SPEED, lightSignals);
    signalMast1B.setIndication(xcadeExpander1->mssPortA, DIVERGING_FULL_SPEED, lightSignals);
  }

    // Routes for 2A:
  //  2B->1B
  //  2B->1A
  //  2B->1C

  if (!t2Thrown && !t4Thrown)
  {
    // 2B->1B straight through
    xcadeExpander1->mssPortB.setLocalOccupancy(blockP2Occupancy || ir2BOccupancy || ir1BOccupancy || block2BOccupancy);
    xcade->mssPortB.setLocalOccupancy(blockP2Occupancy || ir2BOccupancy || ir1BOccupancy || block1BOccupancy);

    xcadeExpander1->mssPortB.cascadeFromPort(xcade->mssPortB);
    xcade->mssPortB.cascadeFromPort(xcadeExpander1->mssPortB);

    signalMast2B.setIndication(xcade->mssPortB, lightSignals);
    signalMast1B.setIndication(xcadeExpander1->mssPortB, lightSignals);
  }
  else if (!t2Thrown && t4Thrown && t3Thrown && !t5Thrown)
  {
    // 2B->1A 
    xcadeExpander1->mssPortB.setLocalOccupancy(blockP1Occupancy || blockP2Occupancy || ir2BOccupancy || ir1AOccupancy || block2BOccupancy);
    xcade->mssPortA.setLocalOccupancy(blockP1Occupancy || blockP2Occupancy || ir2BOccupancy || ir1AOccupancy || block1AOccupancy);

    xcadeExpander1->mssPortB.cascadeFromPort(xcade->mssPortA, true);
    xcade->mssPortA.cascadeFromPort(xcadeExpander1->mssPortB, true);

    signalMast2B.setIndication(xcade->mssPortA, DIVERGING_FULL_SPEED, lightSignals);
    signalMast1A.setIndication(xcadeExpander1->mssPortB, DIVERGING_FULL_SPEED, lightSignals);
  }
  else if (!t2Thrown && t4Thrown && t3Thrown && t5Thrown)
  {
    // 2B->1C
    xcadeExpander1->mssPortB.setLocalOccupancy(blockP1Occupancy || blockP2Occupancy || ir2BOccupancy || ir1COccupancy || block2BOccupancy);
    xcade->mssPortC.setLocalOccupancy(blockP1Occupancy || blockP2Occupancy || ir2BOccupancy || ir1COccupancy || block1COccupancy);

    xcadeExpander1->mssPortB.cascadeFromPort(xcade->mssPortC, true);
    xcade->mssPortC.cascadeFromPort(xcadeExpander1->mssPortB, true);

    signalMast2B.setIndication(xcade->mssPortC, DIVERGING_SLOW_SPEED, lightSignals);
    signalMast1C.setIndication(xcadeExpander1->mssPortB, DIVERGING_FULL_SPEED, lightSignals);
  }


  // Now that all state is computed, send the outputs to the hardware
  xcade->updateOutputs();
  xcadeExpander1->updateOutputs();
}


void CrossoverWithBranch::getStatusJson(JsonObject& statusResponse)
{
  // The JS on the other side is expecting things in the form of:
  //  mss1a-s-in, mss1a-a-out, as booleans - true for active, false for inactive

  // This is really, hideously not threadsafe
  mssPortToStatusJson(statusResponse, xcade->mssPortA, "mss1a");
  mssPortToStatusJson(statusResponse, xcade->mssPortB, "mss1b");
  mssPortToStatusJson(statusResponse, xcade->mssPortC, "mss1c");
  mssPortToStatusJson(statusResponse, xcadeExpander1->mssPortA, "mss2a");
  mssPortToStatusJson(statusResponse, xcadeExpander1->mssPortB, "mss2b");

  mssGPIOToJson(statusResponse, xcade->gpio, "gpio1", 3);
  mssGPIOToJson(statusResponse, xcadeExpander1->gpio, "gpio2", 2);

  mssSensorsToJson(statusResponse, xcade->gpio, "sensor1", 10);
  mssSensorsToJson(statusResponse, xcadeExpander1->gpio, "sensor2", 4);
  return;
}

