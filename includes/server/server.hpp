#pragma once
#include "GlobalConfig.hpp"
#include "utils.hpp"
#include "Parser.hpp"
#include "RequestParser.hpp"
#include "Response.hpp"
#include <sys/types.h>
// #include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <cerrno>
#include <ctime>
#include <map>
#include <string>
#include <iostream>

class Server {
	private:
		GlobalConfig globalConfig;
		ServerConfig serverConfig;
		int serverSocket;
		struct sockaddr_in serverAddr;

	public:
		Server();
		void setNonBlocking(int fd);
		void configure(const std::string& configFilePath);
		void setup();
		void run();
		ServerConfig getServerConfig() const;

};