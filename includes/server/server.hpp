#pragma once
#include "GlobalConfig.hpp"
#include "utils.hpp"
#include "Parser.hpp"
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
		ServerConfig serverConfig;
		int serverSocket;
		struct sockaddr_in serverAddr;
		int kq;


		void setNonBlocking(int fd);
		void removeClient(int clientSocket);
		void handleAccept();
		void handleRead(int clientSocket);
		void handleWrite(int clientSocket);
		void checkTimeouts();
		void registerEvent(int fd, int filter, short flags);
		void processEvent(struct kevent& event);

		struct ClientState{
			std::string requestBuffer;
			std::string responseBuffer;
			time_t lastActive;
			ClientState() : lastActive(time(NULL)) {}
		};

		std::map<int, ClientState> clients;

	public:
		Server();
		void configure(const std::string& configFilePath);
		void setup();
		void run();
		int getServerSocket() const;

};
