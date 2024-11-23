#pragma once
#include "GlobalConfig.hpp"
#include "utils.hpp"
#include "Parser.hpp"
<<<<<<< HEAD
#include "RequestParser.hpp"
#include "Response.hpp"
=======
>>>>>>> c3544417b1d16a1e0a62c823b5c05e02d6a65021
#include <sys/types.h>
#include <sys/event.h>
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
		std::vector<ServerConfig> serverConfigs;
		std::map<int, ServerConfig> serverSockets;
		struct sockaddr_in serverAddr;
		int kq;


		void setNonBlocking(int fd);
		void removeClient(int clientSocket);
		void handleAccept(int serverSocket);
		void handleRead(int clientSocket);
		void handleWrite(int clientSocket);
		void checkTimeouts();
		void registerEvent(int fd, int filter, short flags);
		void processEvent(struct kevent& event);

		struct ClientState {
			std::string requestBuffer;
			std::string responseBuffer;
			time_t lastActive;
			ServerConfig serverConfig;
			// Default constructor
    ClientState()
        : lastActive(time(NULL)), serverConfig() {}

    // Constructor with ServerConfig parameter
    ClientState(const ServerConfig& config)
        : lastActive(time(NULL)), serverConfig(config) {}
		};

		std::map<int, ClientState> clients;

	public:
		Server();
<<<<<<< HEAD
=======
		void configure(const std::string& configFilePath);
>>>>>>> c3544417b1d16a1e0a62c823b5c05e02d6a65021
		void setup();
		void run();
		void configure(const std::string& configFilePath);
		int getServerSocket() const;

};
