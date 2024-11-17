#pragma once
#include "../../srcs/parsing/GlobalConfig.hpp"
#include "utils.hpp"
#include "../../srcs/parsing/Parser.hpp"

class Server {
	private:
		GlobalConfig globalConfig;
		ServerConfig serverConfig;

	public:
		Server();
		void configure(const std::string& configFilePath);

		ServerConfig getServerConfig() const;

};