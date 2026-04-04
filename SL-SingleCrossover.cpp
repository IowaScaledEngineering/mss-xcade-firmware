#include "SL-SingleCrossover.h"

#define ID_SIGNAL_A  "sig1A"
#define ID_SIGNAL_B  "sig1B"
#define ID_SIGNAL_C  "sig1C"
#define ID_SIGNAL_D  "sig1D"


void SingleCrossover::reconfigure(JsonDocument& signalConfig)
{
  Serial.printf("Starting 1xovr reconfigure()\n");

  // Make sure all four signal masts are registered with the rule manager
  signalRuleManager.registerSignal(ID_SIGNAL_A);
  signalRuleManager.registerSignal(ID_SIGNAL_B);
  signalRuleManager.registerSignal(ID_SIGNAL_C);
  signalRuleManager.registerSignal(ID_SIGNAL_D);

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

  // Add heads to masts - everything's a double here, they can go single if they want
  signalMastA.addSignalHeads(&xcade->signals.A1, &xcade->signals.A2);
  signalMastB.addSignalHeads(&xcade->signals.B1, &xcade->signals.B2);
  signalMastC.addSignalHeads(&xcade->signals.C1, &xcade->signals.C2);
  signalMastD.addSignalHeads(&xcade->signals.D1, &xcade->signals.D2);

  // Set signal rules for each mast
  signalMastA.setDoubleHeadRules(signalRuleManager.getSignalRules(ID_SIGNAL_A), signalRuleManager.getSignalRulesLen(ID_SIGNAL_A));
  signalMastB.setDoubleHeadRules(signalRuleManager.getSignalRules(ID_SIGNAL_B), signalRuleManager.getSignalRulesLen(ID_SIGNAL_B));
  signalMastC.setDoubleHeadRules(signalRuleManager.getSignalRules(ID_SIGNAL_C), signalRuleManager.getSignalRulesLen(ID_SIGNAL_C));
  signalMastD.setDoubleHeadRules(signalRuleManager.getSignalRules(ID_SIGNAL_D), signalRuleManager.getSignalRulesLen(ID_SIGNAL_D));

  // Set signal types
  xcade->signals.A1.setSignalHeadType(getJsonBool(signalConfig, "sig1A-searchlight-u")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.A2.setSignalHeadType(getJsonBool(signalConfig, "sig1A-searchlight-l")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.B1.setSignalHeadType(getJsonBool(signalConfig, "sig1B-searchlight-u")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.B2.setSignalHeadType(getJsonBool(signalConfig, "sig1B-searchlight-l")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.C1.setSignalHeadType(getJsonBool(signalConfig, "sig1C-searchlight-u")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.C2.setSignalHeadType(getJsonBool(signalConfig, "sig1C-searchlight-l")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.D1.setSignalHeadType(getJsonBool(signalConfig, "sig1D-searchlight-u")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);
  xcade->signals.D2.setSignalHeadType(getJsonBool(signalConfig, "sig1D-searchlight-l")?SIGNAL_HEAD_SEARCHLIGHT:SIGNAL_HEAD_THREE_LIGHT);

  approachLighting = getJsonBool(signalConfig, "approach-lighting");
  twoBlockApproach = getJsonBool(signalConfig, "two-block-approach");
  tACInvert = getJsonBool(signalConfig, "tAC-invert");
  tBDInvert = getJsonBool(signalConfig, "tBD-invert");

  Serial.printf("Ending 1xovr reconfigure()\n");

}

void SingleCrossover::setup(XCade* xcade)
{
  // This xcade is global, but just save it locally for encapsulation
  // This is the hardware we're literally running on
  this->xcade = xcade;
}


  // This is our example track layout - a single crossover between double track

  //     A1 |--O          OO--| C1/C2
  //	    BD=S1   IR=S2    BD=S3
  //	A ----------- | ------------- C
  //                 /  TO=GPIO1
  //                / 
  //      TO=GPIO2 / 
  //	B ----------- | ------------- D
  //  B1/B2 |--OO         O--| D1
  //	    BD=S4   IR=S5    BD=S6

  //  Sensor S1 - The DCC current detector for block A
  //  Sensor S2 - The IR detector at the block boundary between A and C (in the turnout)
  //  Sensor S3 - The DCC current detector for block C
  //  Sensor S4 - The DCC current detector for block B
  //  Sensor S5 - The IR detector at the block boundary between B and D (in the turnout)
  //  Sensor S6 - The DCC current detector for block D
  //  GPIO1     - turnout A/C position (low = normal, high = reverse)
  //  GPIO2     - turnout B/D position (low = normal, high = reverse)

void SingleCrossover::loop()
{
  uint32_t currentTime = millis();
  static uint32_t lastReadTime = 0;  

	if (!(((uint32_t)currentTime - lastReadTime) > SingleCrossover::LOOP_UPDATE_TIME_MS))
    return;

  // First, read the input state from the hardware
  xcade->updateInputs();

  bool turnoutACNormal = xcade->gpio.digitalRead(1) ^ tACInvert;
  bool turnoutBDNormal = xcade->gpio.digitalRead(2) ^ tBDInvert;

  bool blockAOccupancy = xcade->gpio.digitalRead(SENSOR_1_PIN);
  bool blockBOccupancy = xcade->gpio.digitalRead(SENSOR_3_PIN);
  bool blockCOccupancy = xcade->gpio.digitalRead(SENSOR_5_PIN);
  bool blockDOccupancy = xcade->gpio.digitalRead(SENSOR_7_PIN);
  bool blockP1Occupancy = xcade->gpio.digitalRead(SENSOR_9_PIN);
  bool blockP2Occupancy = xcade->gpio.digitalRead(SENSOR_10_PIN);

  bool irAOccupancy = xcade->gpio.digitalRead(SENSOR_2_PIN);
  bool irBOccupancy = xcade->gpio.digitalRead(SENSOR_4_PIN);
  bool irCOccupancy = xcade->gpio.digitalRead(SENSOR_6_PIN);
  bool irDOccupancy = xcade->gpio.digitalRead(SENSOR_8_PIN);

  // Start with all signals and ports at stop.  All routes not valid are invalid
  signalMastA.setIndication(INDICATION_STOP);
  signalMastB.setIndication(INDICATION_STOP);
  signalMastC.setIndication(INDICATION_STOP);
  signalMastD.setIndication(INDICATION_STOP);

  xcade->mssPortA.cascadeFromIndication(INDICATION_STOP);  
  xcade->mssPortB.cascadeFromIndication(INDICATION_STOP);  
  xcade->mssPortC.cascadeFromIndication(INDICATION_STOP);  
  xcade->mssPortD.cascadeFromIndication(INDICATION_STOP);  

  // Set basic occupancy - even though a port at stop will send approach, we also may
  //  have something tripping the occupancy sensors, which would mean we need to send stop
  xcade->mssPortA.setLocalOccupancy(blockAOccupancy || irAOccupancy);
  xcade->mssPortB.setLocalOccupancy(blockBOccupancy || irBOccupancy);
  xcade->mssPortC.setLocalOccupancy(blockCOccupancy || irCOccupancy);
  xcade->mssPortD.setLocalOccupancy(blockDOccupancy || irDOccupancy);

  bool approachOccupancy = false;
  
  if (approachLighting)
  {
    if (twoBlockApproach)
    {
      approachOccupancy = xcade->mssPortA.getDoubleBlockApproach() || xcade->mssPortB.getDoubleBlockApproach() 
        || xcade->mssPortC.getDoubleBlockApproach() || xcade->mssPortD.getDoubleBlockApproach();
    } else {
      approachOccupancy = xcade->mssPortA.getSingleBlockApproach() || xcade->mssPortB.getSingleBlockApproach() 
        || xcade->mssPortC.getSingleBlockApproach() || xcade->mssPortD.getSingleBlockApproach();
    }
    approachOccupancy |= blockP1Occupancy || blockP2Occupancy;
  }

  if (turnoutACNormal && turnoutBDNormal)
  {
    // Crossover is normal, A<-->C and B<-->D
    xcade->mssPortA.cascadeFromPort(xcade->mssPortC);
    xcade->mssPortC.cascadeFromPort(xcade->mssPortA);
    xcade->mssPortA.setLocalOccupancy(blockAOccupancy || irAOccupancy || irCOccupancy || blockP1Occupancy);
    xcade->mssPortC.setLocalOccupancy(blockCOccupancy || irAOccupancy || irCOccupancy || blockP1Occupancy);

    xcade->mssPortB.cascadeFromPort(xcade->mssPortD);
    xcade->mssPortD.cascadeFromPort(xcade->mssPortB);
    xcade->mssPortB.setLocalOccupancy(blockBOccupancy || irBOccupancy || irDOccupancy || blockP2Occupancy);
    xcade->mssPortD.setLocalOccupancy(blockDOccupancy || irBOccupancy || irDOccupancy || blockP2Occupancy);

    signalMastA.setIndication(xcade->mssPortC, NOT_DIVERGING, !approachLighting || approachOccupancy);
    signalMastC.setIndication(xcade->mssPortA, NOT_DIVERGING, !approachLighting || approachOccupancy);
    signalMastB.setIndication(xcade->mssPortD, NOT_DIVERGING, !approachLighting || approachOccupancy);
    signalMastD.setIndication(xcade->mssPortB, NOT_DIVERGING, !approachLighting || approachOccupancy);
  }
  else if (!turnoutACNormal && !turnoutBDNormal)
  {
    // Crossover is reversed, B<-->D
    xcade->mssPortB.cascadeFromPort(xcade->mssPortC, true);
    xcade->mssPortC.cascadeFromPort(xcade->mssPortB, true);
    xcade->mssPortB.setLocalOccupancy(blockBOccupancy || irBOccupancy || irCOccupancy || blockP1Occupancy || blockP2Occupancy);
    xcade->mssPortC.setLocalOccupancy(blockCOccupancy || irBOccupancy || irCOccupancy || blockP1Occupancy || blockP2Occupancy);

    signalMastC.setIndication(xcade->mssPortB, DIVERGING_FULL_SPEED, !approachLighting || approachOccupancy);
    signalMastB.setIndication(xcade->mssPortC, DIVERGING_FULL_SPEED, !approachLighting || approachOccupancy);
  }

  // Now that all state is computed, send the outputs to the hardware
  xcade->updateOutputs();

}


void SingleCrossover::getStatusJson(JsonObject& statusResponse)
{
  // The JS on the other side is expecting things in the form of:
  //  mss1a-s-in, mss1a-a-out, as booleans - true for active, false for inactive

  // This is really, hideously not threadsafe
  mssPortToStatusJson(statusResponse, xcade->mssPortA, "mss1a");
  mssPortToStatusJson(statusResponse, xcade->mssPortB, "mss1b");
  mssPortToStatusJson(statusResponse, xcade->mssPortC, "mss1c");
  mssPortToStatusJson(statusResponse, xcade->mssPortD, "mss1d");

  mssGPIOToJson(statusResponse, xcade->gpio, "gpio1", 2);
  mssSensorsToJson(statusResponse, xcade->gpio, "sensor1", 10);

  return;
}

