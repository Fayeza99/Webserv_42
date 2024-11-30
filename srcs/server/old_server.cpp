#include "server.hpp"

Server::Server() {}

/**
 * @brief this function calls the readConfigFile and
 * then parses the content of the config file and store
 * it in ServerConfig variale
 *
 * @param configFilePath comes from the main argv[1]
 */
void Server::configure(const std::string& configFilePath) {
	Parser parser(readConfigFile(configFilePath));
	globalConfig = parser.parse();
	serverConfig = parser.parseServer();
	// printGlobalConfig(globalConfig);
}

/**
 * @brief function to set a soket to non-blocking mode
 *
 * @param fd file descriptor of the socket
 */
void Server::setNonBlocking(int fd) {
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
		throw std::runtime_error("Failed to set socket to non-blocking");
	}
}


/**
 * @brief It sets up the server by:
 * 1. creating a socket
 * 2. Set socket options
 * 3. Configure server addres structure
 * 4. Bind to all available interfaces (0.0.0.0)
 * 5. Bind the socket to the specified port
 * 6. Start listenting for incoming connections
 *
 */
void Server::setup() {
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1) {
		throw std::runtime_error("Failed to create socket");
	}

	setNonBlocking(serverSocket);

	int opt = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		close(serverSocket);
		throw std::runtime_error("Failed to set socket options");
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;

	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(serverConfig.listen_port);

	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
		close(serverSocket);
		throw std::runtime_error("Failed to bind socket");
	}

	if (listen(serverSocket, SOMAXCONN) == -1) {
		close(serverSocket);
		throw std::runtime_error("Failed to listen on socket");
	}

	std::cout << "Server is listening on port " << serverConfig.listen_port << std::endl;
}

/**
 * @brief the funtion accept connection in a loop
 *
 */
void Server::run() {
	while (true) {
		struct sockaddr_in clientAddr;
		socklen_t clientAddrLen = sizeof(clientAddr);
		int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

		if (clientSocket == -1) {
			std::cerr << "Failed to accept connection" << std::endl;
			continue;
		}

		char buffer[1024];
		ssize_t received = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

		// Here the buffer contians the request. Run the parser on buffer

		if (received > 0) {
			buffer[received] = '\0';
			std::cout << "Received request:\n" << buffer << std::endl;

			// Currently providing the following response call the response creation logic here

			const char* response =
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: 13\r\n"
				"\r\n"
				"Hello, world!";

			send(clientSocket, response, strlen(response), 0);
		}

		close(clientSocket);
	}

	close(serverSocket);
}

ServerConfig Server::getServerConfig() const {
	return serverConfig;
}