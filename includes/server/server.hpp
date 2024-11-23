#pragma once
#include "../../srcs/parsing/GlobalConfig.hpp"
#include "utils.hpp"
#include "Response.hpp"
#include "../../srcs/parsing/Parser.hpp"
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>      // For close()
#include <fcntl.h>       // For fcntl()
#include <arpa/inet.h>   // For inet_ntoa()
#include <netinet/in.h>  // For sockaddr_in
#include <cstring>       // For memset()
#include <cerrno>        // For errno
#include <ctime>         // For time()
#include <map>           // For std::map
#include <string>        // For std::string
#include <iostream>      // For input/output
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
		Server(const ServerConfig& config);
		void setup();
		void run();
		int getServerSocket() const;

};