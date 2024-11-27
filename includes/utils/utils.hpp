#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <GlobalConfig.hpp>


std::string readConfigFile(const std::string& configFilePath);
void printGlobalConfig(const GlobalConfig& config, int indent);