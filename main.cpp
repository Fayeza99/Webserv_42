#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "parsing/GlobalConfig.hpp" // Assuming this is the header file for your config classes
#include "parsing/Parser.hpp"		// Assuming this is the header file where your Parser class is defined
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "Response.hpp"
#include <cstdlib>
#include <cstring>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

// int main(int argc, char **argv)
int main(void)
{
	// parsing config
	// if (argc < 2)
	// {
	// 	std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
	// 	return 1;
	// }
	// std::string configFile = argv[1];
	// std::ifstream file(configFile);
	// if (!file.is_open())
	// {
	// 	std::cerr << "Failed to open file: " << configFile << std::endl;
	// 	return 1;
	// }
	// std::string configContent((std::istreambuf_iterator<char>(file)),
	// 							std::istreambuf_iterator<char>());
	// try
	// {
	// 	Parser parser(configContent);
	// 	GlobalConfig config = parser.parse();
	// 	std::cout << "Configuration parsed successfully." << std::endl;
	// 	// Further processing with `config`...
	// }
	// catch (const std::exception &e)
	// {
	// 	std::cerr << "Failed to parse configuration: " << e.what() << std::endl;
	// 	return 1;
	// }


	// server - anna
	// server socket to listen on: ----------------------------------------------------
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		std::cerr << "Failed to create server socket\n";
		return 1;
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(4221);
	if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
		std::cerr << "Failed to bind to port 4221\n";
		return 1;
	}
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		std::cerr << "listen failed\n";
		return 1;
	}

	// client sockets: ----------------------------------------------------------------
	struct sockaddr_in	client_addr;
	int					client_addr_len = sizeof(client_addr);
	int					client_fd;
	struct pollfd		fds[100];
	int					nfds;

	fds[0].fd = server_fd;
	fds[0].events = POLLIN;
	nfds = 1;

	// connection loop: ---------------------------------------------------------------
	char		buffer[4096];
	int			request_len;
	std::string	response;
	std::memset(buffer, 0, sizeof(buffer));

	std::cout << "ready for requests\n";
	while (1)
	{
		if (poll(fds, nfds, -1) < 0) {
			std::cout << "poll error\n";
			break ;
		}
		for (int i = 0; i < nfds; i++) {
			if (fds[i].revents & POLLIN) {
				if (fds[i].fd == server_fd) {
					client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
					if (client_fd >= 0) {
						std::cout << "client connected\n";
						fds[nfds].fd = client_fd;
						fds[nfds++].events = POLLIN;
					}
				} else {
					std::cout << "handling request\n";
					client_fd = fds[i].fd;
					request_len = read(client_fd, buffer, sizeof(buffer) - 1);
					if (request_len > 0) {
						buffer[request_len] = '\0';
						handle_request(std::string(buffer), client_fd);
						std::cout << "response sent\n";
					} else {
						std::cout << "connection closed\n";
						close(client_fd);
						fds[i--] = fds[nfds - 1];
						nfds--;
					}
				}
			}
		}
	}
	for (int i = 0; i < nfds; i++)
		close(fds[i].fd);
	return 0;
}
