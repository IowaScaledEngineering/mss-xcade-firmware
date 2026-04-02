#include "SignalLogicRegistry.h"
// Include the actual definition of SignalLogic here
// #include "SignalLogic.h" 

SignalLogicRegistry signalLogicRegistry;


void SignalLogicRegistry::registerType(const char* shortName, const char* longName, CreatorFunc creator) {
    shortNames.push_back(shortName);
    longNames.push_back(longName);
    creatorRegistry[std::string(shortName)] = std::move(creator);
}

std::unique_ptr<SignalLogic> SignalLogicRegistry::create(const char* shortName) const {
    auto it = creatorRegistry.find(std::string(shortName));
    if (it != creatorRegistry.end()) {
        return it->second();
    }

    // Default fallback logic
    auto fallback = creatorRegistry.find("none");
    if (fallback != creatorRegistry.end()) {
        return fallback->second();
    }
    
    return nullptr;
}

bool SignalLogicRegistry::shortNameExists(std::string shortName) {
    return creatorRegistry.find(shortName) != creatorRegistry.end();
}

size_t SignalLogicRegistry::getNumLogicModules() {
    return shortNames.size();
}

const char* SignalLogicRegistry::getShortName(uint32_t n) {
    if (n < shortNames.size()) {
        return shortNames[n];
    }
    return nullptr;
}

const char* SignalLogicRegistry::getLongName(uint32_t n) {
    if (n < longNames.size()) {
        return longNames[n];
    }
    return nullptr;
}