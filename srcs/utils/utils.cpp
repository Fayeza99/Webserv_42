#include "utils.hpp"

std::string readConfigFile(const std::string& configFilePath) {
	std::ifstream configFile(configFilePath);
	if (!configFile.is_open()) {
		throw std::ios_base::failure("Failed to open file: " + configFilePath);
	}

	std::string configData((std::istreambuf_iterator<char>(configFile)), std::istreambuf_iterator<char>());
	configFile.close();

	return configData;
}
