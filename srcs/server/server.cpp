#include "server.hpp"

Server::Server() {}

void Server::configure(const std::string& configFilePath) {
	Parser parser(readConfigFile(configFilePath));
	// globalConfig = parser.parse();
	serverConfig =  parser.parseServer();
}

ServerConfig Server::getServerConfig() const {
	return serverConfig;
}