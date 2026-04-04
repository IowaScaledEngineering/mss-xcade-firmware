#ifndef _SIGNAL_RULE_MANAGER_H_
#define _SIGNAL_RULE_MANAGER_H_

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <algorithm>

#include "mss-xcade.h"

constexpr size_t RULE_COUNT = 14;
constexpr std::array<IndicationRule_t, RULE_COUNT> BASE_RULES = {{
  { INDICATION_STOP,                  SignalMast::DIVMASK_NOT_DIVERGING, ASPECT_RED,       ASPECT_RED,       ASPECT_OFF },
  { INDICATION_RESTRICTING,           SignalMast::DIVMASK_NOT_DIVERGING, ASPECT_FL_RED,    ASPECT_RED,       ASPECT_OFF },
  { INDICATION_APPROACH,              SignalMast::DIVMASK_NOT_DIVERGING, ASPECT_YELLOW,    ASPECT_RED,       ASPECT_OFF },
  { INDICATION_APPROACH_DIVERGING,    SignalMast::DIVMASK_NOT_DIVERGING, ASPECT_YELLOW,    ASPECT_YELLOW,    ASPECT_OFF },
  { INDICATION_ADVANCE_APPROACH,      SignalMast::DIVMASK_NOT_DIVERGING, ASPECT_FL_YELLOW, ASPECT_RED,       ASPECT_OFF },
  { INDICATION_APPROACH_DIVERGING_AA, SignalMast::DIVMASK_NOT_DIVERGING, ASPECT_FL_YELLOW, ASPECT_RED,       ASPECT_OFF },
  { INDICATION_CLEAR,                 SignalMast::DIVMASK_NOT_DIVERGING, ASPECT_GREEN,     ASPECT_RED,       ASPECT_OFF },
  { INDICATION_STOP,                  SignalMast::DIVMASK_ALL_DIVERGING, ASPECT_RED,       ASPECT_RED,       ASPECT_OFF },
  { INDICATION_RESTRICTING,           SignalMast::DIVMASK_ALL_DIVERGING, ASPECT_RED,       ASPECT_FL_RED,    ASPECT_OFF },
  { INDICATION_APPROACH,              SignalMast::DIVMASK_ALL_DIVERGING, ASPECT_RED,       ASPECT_YELLOW,    ASPECT_OFF },
  { INDICATION_APPROACH_DIVERGING,    SignalMast::DIVMASK_ALL_DIVERGING, ASPECT_RED,       ASPECT_YELLOW,    ASPECT_OFF },
  { INDICATION_ADVANCE_APPROACH,      SignalMast::DIVMASK_ALL_DIVERGING, ASPECT_RED,       ASPECT_FL_YELLOW, ASPECT_OFF },
  { INDICATION_APPROACH_DIVERGING_AA, SignalMast::DIVMASK_ALL_DIVERGING, ASPECT_RED,       ASPECT_FL_YELLOW, ASPECT_OFF },
  { INDICATION_CLEAR,                 SignalMast::DIVMASK_ALL_DIVERGING, ASPECT_RED,       ASPECT_GREEN,     ASPECT_OFF }
}};

class SignalRuleManager 
{
  private:
    struct SignalData 
    {
        std::string name;
        std::array<IndicationRule_t, RULE_COUNT> rules;
    };
    std::vector<SignalData> signals;
    SignalData& getOrCreateSignal(const char* name) ;
    SignalData& getOrCreateSignal(std::string_view name) ;
    const char* getDivStr(uint8_t divMask) const;
    const char* getAspectStr(SignalAspect_t aspect) const;
    const char* getIndicationStr(MSSPortIndication_t ind) const;
    


  public:
    SignalRuleManager();
    static SignalAspect_t jsonToAspect(const char* aspectStr);
    static SignalAspect_t jsonToAspect(std::string_view aspectStr);
    void applyOverride(const char* configKeyStr, const char* aspectStr);
    void applyOverride(std::string_view  configKeyStr, std::string_view aspectStr);
    IndicationRule_t* getSignalRules(const std::string& name);
    uint16_t getSignalRulesLen(const std::string& name);
    void registerSignal(const std::string& name);
    void dumpRules() const;
};

#endif