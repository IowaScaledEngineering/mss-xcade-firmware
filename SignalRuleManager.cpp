#include <Arduino.h>
#include "SignalRuleManager.h"

// The base rules defined as a constant array

SignalRuleManager::SignalData& SignalRuleManager::getOrCreateSignal(const char* nameStr) 
{
  return getOrCreateSignal(std::string_view(nameStr));
}

SignalRuleManager::SignalData& SignalRuleManager::getOrCreateSignal(std::string_view nameStr) 
{
  for (auto& sig : signals) 
  {
    if (sig.name == nameStr) {
      return sig;
    }
  }

  // Create new signal, initialize with base rules
  signals.push_back({std::string(nameStr), BASE_RULES});
  return signals.back();
}

SignalRuleManager::SignalRuleManager()
{
  getOrCreateSignal("default");
}

SignalAspect_t SignalRuleManager::jsonToAspect(const char* aspectStr)
{
  return jsonToAspect(std::string_view(aspectStr));
}

SignalAspect_t SignalRuleManager::jsonToAspect(std::string_view aspectStr)
{
  SignalAspect_t retval = ASPECT_OFF;
  if (aspectStr == "G")   
    retval = ASPECT_GREEN;
  else if (aspectStr == "FG")
    retval = ASPECT_FL_GREEN;
  else if (aspectStr == "Y")
    retval = ASPECT_YELLOW;
  else if (aspectStr == "FY")
    retval = ASPECT_FL_YELLOW;
  else if (aspectStr == "R")
    retval = ASPECT_RED;
  else if (aspectStr == "FR")
    retval = ASPECT_FL_RED;
  else if (aspectStr == "OFF")
    retval = ASPECT_OFF;

  return (retval);
}

// Takes the full JSON key (e.g., "sig1D-nor-clr-u") and applies new aspects
void SignalRuleManager::applyOverride(const char* configKeyStr, const char* aspectStr) 
{
  applyOverride(std::string_view(configKeyStr), std::string_view(aspectStr));
}

void SignalRuleManager::applyOverride(std::string_view configKey, std::string_view aspectStr) 
{
  std::string_view sigName;
  uint8_t divMask = 0xFF; // default invalid
  MSSPortIndication_t ind = INDICATION_END; // default invalid

  size_t start = 0;
  size_t end = configKey.find('-');
  int elementIndex = 0;
  uint8_t targetHead = 0;

  // Parse the key inline
  while (start != std::string_view::npos) 
  {
    std::string_view token = configKey.substr(start, end - start);

    switch (elementIndex) 
    {
      case 0: // Signal Name
        sigName = token;
        break;
      case 1: // Divergence
        if (token == "nor")
          divMask = SignalMast::DIVMASK_NOT_DIVERGING;
        else if (token == "div")
          divMask = SignalMast::DIVMASK_ALL_DIVERGING;
        break;
      case 2: // Indication
        if (token == "stop") 
          ind = INDICATION_STOP;
        else if (token == "a")  
          ind = INDICATION_APPROACH;
        else if (token == "ad") 
          ind = INDICATION_APPROACH_DIVERGING;
        else if (token == "aa") 
          ind = INDICATION_ADVANCE_APPROACH;
        else if (token == "aad")
          ind = INDICATION_APPROACH_DIVERGING_AA;
        else if (token == "clr")
          ind = INDICATION_CLEAR;
        break;
      case 3: // Target Head              
        if (token == "u")
          targetHead = 0;
        else if (token == "l")
          targetHead = 1;
        else if (token == "3")
          targetHead = 2;
        
        break;
    }

    if (end == std::string_view::npos)
      break;
    start = end + 1;
    end = configKey.find('-', start);
    elementIndex++;
  }

  // Apply the parsed override
  if (divMask != 0xFF && ind != INDICATION_END && !sigName.empty()) 
  {
    SignalRuleManager::SignalData& sigData = getOrCreateSignal(sigName);
    
    // Find the matching rule and update ONLY the requested head
    for (auto& rule : sigData.rules) 
    {
      if (rule.indication == ind && rule.divergingMask == divMask) 
      {
        SignalAspect_t aspect = jsonToAspect(aspectStr);
        if (targetHead == 0) {
          rule.head1Aspect = aspect;
        } else if (targetHead == 1) {
          rule.head2Aspect = aspect;
        } else if (targetHead == 2) {
          rule.head3Aspect = aspect;
        }
        break; // Found the rule, stop searching
      }
    }
  }
}

// Returns a raw pointer to the array of structs for the requested signal
// Returns nullptr if the signal hasn't been created yet.
IndicationRule_t* SignalRuleManager::getSignalRules(const std::string& name) 
{
  for (auto& sig : signals) {
    if (sig.name == name) {
      return sig.rules.data(); // Standard way to get raw C-array pointer
    }
  }
  for (auto& sig : signals) {
    if (sig.name == "default") {
      return sig.rules.data(); // Standard way to get raw C-array pointer
    }
  }
  return nullptr;
}

uint16_t SignalRuleManager::getSignalRulesLen(const std::string& name) 
{
  return RULE_COUNT;
}    

// Explicitly seed a signal so it exists even if there are no JSON overrides for it
void SignalRuleManager::registerSignal(const std::string& name) 
{
  getOrCreateSignal(name);
}



// Helper to convert internal indications back to strings for printing
const char* SignalRuleManager::getIndicationStr(MSSPortIndication_t ind) const {
    switch (ind) {
        case INDICATION_STOP:                  return "STOP";
        case INDICATION_APPROACH:              return "APPROACH";
        case INDICATION_APPROACH_DIVERGING:    return "APP_DIV";
        case INDICATION_ADVANCE_APPROACH:      return "ADV_APP";
        case INDICATION_APPROACH_DIVERGING_AA: return "APP_DIV_AA";
        case INDICATION_CLEAR:                 return "CLEAR";
        default:                               return "UNKNOWN";
    }
}

// Helper to convert internal aspects back to strings for printing
const char* SignalRuleManager::getAspectStr(SignalAspect_t aspect) const {
    switch (aspect) {
        case ASPECT_RED:       return "R";
        case ASPECT_YELLOW:    return "Y";
        case ASPECT_FL_YELLOW: return "FY";
        case ASPECT_GREEN:     return "G";
        case ASPECT_FL_GREEN:  return "FG";
        case ASPECT_FL_RED:    return "FR";
        case ASPECT_OFF:       return "OFF";
        default:               return "INV";
    }
}

// Helper to convert divergence mask to a string
const char* SignalRuleManager::getDivStr(uint8_t divMask) const {
    if (divMask == SignalMast::DIVMASK_NOT_DIVERGING) return "NOR";
    if (divMask == SignalMast::DIVMASK_ALL_DIVERGING) return "DIV";
    return "???";
}

// Dumps all registered signals and their active rules to the Serial monitor
void SignalRuleManager::dumpRules() const {
    if (signals.empty()) {
        Serial.printf("SignalRuleManager: No signals loaded.\n");
        return;
    }

    Serial.printf("\n=== SIGNAL RULES DUMP ===\n");
    for (const auto& sig : signals) {
        Serial.printf("Signal: %s  %p\n", sig.name.c_str(), sig.rules.data());
        Serial.printf("  | %-10s | %-3s | %-3s | %-3s | %-3s |\n", 
                      "Indication", "Rte", "H1", "H2", "H3");
        Serial.printf("  |------------|-----|-----|-----|-----|\n");
        
        for (const auto& rule : sig.rules) {
            Serial.printf("  | %-10s | %-3s | %-3s | %-3s | %-3s |\n",
                          getIndicationStr(rule.indication),
                          getDivStr(rule.divergingMask),
                          getAspectStr(rule.head1Aspect),
                          getAspectStr(rule.head2Aspect),
                          getAspectStr(rule.head3Aspect));
        }
        Serial.printf("\n");
    }
    Serial.printf("=========================\n\n");
}