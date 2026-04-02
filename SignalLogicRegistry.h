#ifndef SIGNAL_LOGIC_REGISTRY_H
#define SIGNAL_LOGIC_REGISTRY_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include "SignalLogic.h"

class SignalLogicRegistry 
{
  public:
      using CreatorFunc = std::function<std::unique_ptr<SignalLogic>()>;

      void registerType(const char* shortName, const char* longName, CreatorFunc creator);
      
      std::unique_ptr<SignalLogic> create(const char* shortName) const;
      
      bool shortNameExists(std::string shortName);
      
      size_t getNumLogicModules();
      
      const char* getShortName(uint32_t n);
      
      const char* getLongName(uint32_t n);

  private:
      std::unordered_map<std::string, CreatorFunc> creatorRegistry;
      std::vector<const char*> shortNames;
      std::vector<const char*> longNames;
};

extern SignalLogicRegistry signalLogicRegistry;

#endif