#include "ConfigManager.hpp"
#include "ActionLoader.hpp"
#include "PropertyLoader.hpp"
#include "RailroadLoader.hpp"
#include "UtilityLoader.hpp"
#include "TaxLoader.hpp"
#include "SpecialLoader.hpp"
#include "MiscLoader.hpp"

using namespace std;

ConfigManager::ConfigManager(const string& dir) : configDir(dir) {}

ConfigManager::~ConfigManager() {}

void ConfigManager::loadAll() {
    SpecialLoader special(configDir + "/special.txt");
    MiscLoader misc(configDir + "/misc.txt");
    TaxLoader tax(configDir + "/tax.txt");
    RailroadLoader railroad(configDir + "/railroad.txt");
    UtilityLoader utility(configDir + "/utility.txt");

    special.loadConfig();
    misc.loadConfig();
    tax.loadConfig();
    railroad.loadConfig();
    utility.loadConfig();

    config.goSalary = special.getGoSalary();
    config.jailFine = special.getJailFine();
    config.maxTurn = misc.getMaxTurn();
    config.startingBalance = misc.getStartingBalance();
    config.pphFlat = tax.getPphFlat();
    config.pphPercentage = tax.getPphPercentage();
    config.pbmFlat = tax.getPbmFlat();
    config.railroadRentTable = railroad.getRentTable();
    config.utilityMultipliers = utility.getMultipliers();

    propertyLoader = make_unique<PropertyLoader>(configDir + "/property.txt");
    propertyLoader->loadConfig();

    actionLoader = make_unique<ActionLoader>(configDir + "/action.txt");
    actionLoader->loadConfig();
}

const GameConfig& ConfigManager::getConfig() const {
    return config;
}

PropertyLoader& ConfigManager::getPropertyLoader() const {
    return *propertyLoader;
}

ActionLoader& ConfigManager::getActionLoader() const {
    return *actionLoader;
}
