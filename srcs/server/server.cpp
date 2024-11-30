// #include "server.hpp"

// Server::Server() : kq(-1) {}

// /**
//  * @brief this function calls the readConfigFile and
//  * then parses the content of the config file and store
//  * it in serverConfig variale
//  *
//  * @param configFilePath comes from the main argv[1]
//  */
// void Server::configure(const std::string& configFilePath) {
// 	Parser parser(readConfigFile(configFilePath));
// 	GlobalConfig globalConfig = parser.parse();
// 	printGlobalConfig(globalConfig);
// 	serverConfigs = globalConfig.servers;
// }

// /**
//  * @brief function to set a soket to non-blocking mode
//  *
//  * @param fd file descriptor of the socket
//  */
// void Server::setNonBlocking(int fd) {
// 	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
// 		throw std::runtime_error("Failed to set socket to non-blocking");
// 	}
// }

// /**
//  * @brief It sets up the server by:
//  * 1. creating a socket
//  * 2. Set socket options
//  * 3. Configure server addres structure
//  * 4. Bind to all available interfaces (0.0.0.0)
//  * 5. Bind the socket to the specified port
//  * 6. Start listenting for incoming connections
//  *
//  */
// void Server::setup() {
// 	kq = kqueue();
// 	if (kq == -1) {
// 		throw std::runtime_error("Failed to create kqueue");
// 	}


// 	for (const ServerConfig& config : serverConfigs) {

// 		int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
// 		if (serverSocket == -1) {
// 			throw std::runtime_error("Failed to create socket");
// 		}

// 		setNonBlocking(serverSocket);

// 		int opt = 1;
// 		if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
// 			close(serverSocket);
// 			throw std::runtime_error("Failed to set socket options");
// 		}

// 		struct sockaddr_in serverAddr;
// 		memset(&serverAddr, 0, sizeof(serverAddr));
// 		serverAddr.sin_family = AF_INET;
// 		serverAddr.sin_addr.s_addr = INADDR_ANY;
// 		serverAddr.sin_port = htons(config.listen_port);

// 		if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
// 			close(serverSocket);
// 			throw std::runtime_error("Failed to bind socket");
// 		}

// 		if (listen(serverSocket, SOMAXCONN) == -1) {
// 			close(serverSocket);
// 			throw std::runtime_error("Failed to listen on socket");
// 		}

// 		struct kevent change;
// 		EV_SET(&change, serverSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
// 		if (kevent(kq, &change, 1, NULL, 0, NULL) == -1) {
// 			close(serverSocket);
// 			throw std::runtime_error("Failed to register server socket with kqueue");
// 		}

// 		serverSockets[serverSocket] = config;

// 		std::cout << "Server is listening on port " << config.listen_port << std::endl;
// 	}
// }

// /**
//  * @brief Function to remove a client
//  *
//  * Unegisters read and write events for the client socket
//  * Closes the client socket
//  * Erases the clients state from clients
//  *
//  * @param clientSocket
//  */
// void Server::removeClient(int clientSocket) {
// 	struct kevent change;

// 	EV_SET(&change, clientSocket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
// 	if (kevent(this->kq, &change, 1, NULL, 0, NULL) == -1) {
// 		std::cerr << "Failed to remove read event for client " << clientSocket << ": " << strerror(errno) << std::endl;
// 	}

// 	EV_SET(&change, clientSocket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
// 	if (kevent(this->kq, &change, 1, NULL, 0, NULL) == -1) {
// 		std::cerr << "Failed to remove write event for client " << clientSocket << ": " << strerror(errno) << std::endl;
// 	}

// 	close(clientSocket);

// 	clients.erase(clientSocket);
// }

// /**
//  * @brief function to register or modify events with kqueue
//  *
//  * @param fd file descriptor to monitor.
//  * @param filter event filter (EVFILT_READ, EVFILT_WRITE, etc.)
//  * @param flags Action Flags (EV_ADD, EV_DELETE, EV_ENABLE, EV_DISABLE, EV_CLEAR)
//  */
// void Server::registerEvent(int fd, int filter, short flags) {
// 	struct kevent change;
// 	EV_SET(&change, fd, filter, flags, 0, 0, nullptr);
// 	if (kevent(kq, &change, 1, nullptr, 0, nullptr) == -1) {
// 		std::cerr << "Failed to register event for fd " << fd << ": " << strerror(errno) << std::endl;
// 	}
// }

// /**
//  * @brief Handles accepting new client connections
//  *
//  * Ensures that I/O operations on the client socket do not block the server
//  * Registers the client socket for read events (EVFILT_READ)
//  * Initializes a ClientState for tracking request and response data
//  *
//  */
// void Server::handleAccept(int serverSocket) {
// 	struct sockaddr_in clientAddr;
// 	socklen_t clientAddrLen = sizeof(clientAddr);
// 	int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

// 	if (clientSocket == -1) {
// 		std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
// 		return;
// 	}

// 	try {
// 		setNonBlocking(clientSocket);
// 	} catch (const std::exception &e) {
// 		std::cerr << "Failed to set client socket to non-blocking: " << e.what() << std::endl;
// 		close(clientSocket);
// 		return;
// 	}

// 	registerEvent(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE | EV_CLEAR);

// 	const ServerConfig& config = serverSockets[serverSocket];
// 	clients.emplace(clientSocket, ClientState(config));

// 	std::cout	<< "Accepted new connection from "
// 				<< inet_ntoa(clientAddr.sin_addr) << ":"
// 				<< ntohs(clientAddr.sin_port)
// 				<< ", socket " << clientSocket << std::endl;
// }

// /**
//  * @brief Function to process incoming data from clients
//  *
//  * It reads the incoming data and appends it to the client' request buffer.
//  * Detects the end HTTP headers (\r\n\r\n)
//  * Prepares a response and stores it in the response buffer
//  * Registers the client socket for write events(EVFILT_WRITE) to send the response.
//  *
//  * @param clientSocket
//  */
// void Server::handleRead(int clientSocket) {
// 	char buffer[4096];
// 	ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

// 	if (bytesRead > 0) {
// 		buffer[bytesRead] = '\0';
// 		std::cout << "Received request from the client " << clientSocket << ":\n" << buffer << std::endl;

// 		clients[clientSocket].lastActive = time(nullptr);
// 		clients[clientSocket].requestBuffer += buffer;

// 		size_t pos = clients[clientSocket].requestBuffer.find("\r\n\r\n");
// 		if (pos != std::string::npos) {
// 			// std::string response =
// 				// "HTTP/1.1 200 OK\r\n"
// 				// "Content-Type: text/plain\r\n"
// 				// "Content-Length: 13\r\n"
// 				// "\r\n"
// 				// "Hello, world!";

// 			RequestParser	p(buffer);
// 			Response		r(p, "www/");
// 			clients[clientSocket].responseBuffer = r.get_response();

// 			registerEvent(clientSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR);

// 		}
// 	} else if (bytesRead == 0) {
// 		std::cout << "Client " << clientSocket << " disconnected" << std::endl;
// 		removeClient(clientSocket);
// 	} else {
// 		std::cerr << "recv() failed for client " << clientSocket << ": " << strerror(errno) << std::endl;
// 		removeClient(clientSocket);
// 	}

// }

// /**
//  * @brief Sneds reponses to clients when their sockets are ready for writing
//  *
//  * Sends the reponse store in the client's response buffer
//  * Handles cases where only part of the data is sent, updating buffer accordingly
//  * Removes the write event filter once all the data has been sent
//  *
//  * @param clientSocket
//  */
// void Server::handleWrite(int clientSocket) {
// 	std::string& response = clients[clientSocket].responseBuffer;
// 	if (!response.empty()) {
// 	std::cout << "RESPONSE:\n" << response << std::endl;
// 		ssize_t bytesSent = send(clientSocket, response.c_str(), response.size(), 0);
// 		if (bytesSent > 0) {
// 			response.erase(0, bytesSent);
// 			clients[clientSocket].lastActive = time(nullptr);
// 		}

// 		if (bytesSent == -1) {
// 			std::cerr << "send() failed for client " << clientSocket << ": " << strerror(errno) << std::endl;
// 			removeClient(clientSocket);
// 			return;
// 		}

// 		if (response.empty()) {
// 			registerEvent(clientSocket, EVFILT_WRITE, EV_DELETE);
// 		}
// 	}
// }

// /**
//  * @brief Determines the type of event and delegates handling to the appropriate functions
//  *
//  * Handles any errors associated with the events
//  * Determines whether the event is a read or a write event and calls the corresponding handler
//  *
//  * @param event
//  */
// void Server::processEvent(struct kevent& event) {
// 	int fd = static_cast<int>(event.ident);

// 	if (event.flags & EV_ERROR) {
// 		if (serverSockets.count(fd)) {
// 			std::cerr << "Error on server socket: " << strerror(static_cast<int>(event.data)) << std::endl;
// 		} else {
// 			int clientSocket = static_cast<int>(event.ident);
// 			std::cerr << "Error on client socket " << clientSocket << ": " << strerror(static_cast<int>(event.data)) << std::endl;
// 			removeClient(clientSocket);
// 		}
// 		return;
// 	}

// 	if (event.filter == EVFILT_READ) {
// 		if (serverSockets.count(fd)) {
// 			handleAccept(fd);
// 		} else {
// 			handleRead(fd);
// 		}
// 	}

// 	if (event.filter == EVFILT_WRITE) {
// 			handleWrite(fd);
// 	}
// }

// /**
//  * @brief Closes connections that have been idle beyond a specified timeout duration
//  *
//  * Defines how long a client can remain idle before being disconnected
//  * Safely iterates through the clients map, removing timed-out clients
//  *
//  */
// void Server::checkTimeouts() {
// 	time_t currentTime = time(nullptr);
// 	const int TIMEOUT_DURATION = 30; // seconds

// 	for (auto it = clients.begin(); it != clients.end();) {
// 		if (currentTime - it->second.lastActive > TIMEOUT_DURATION) {
// 			int clientSocket = it->first;
// 			std::cout << "Client " << clientSocket << " timed out" << std::endl;
// 			removeClient(clientSocket);
// 			it = clients.erase(it);
// 		} else {
// 			++it;
// 		}
// 	}
// }

// /**
//  * @brief Putting everything together
//  *
//  * Continously eaits for and processes incoming events.
//  * Delegates each event to the processEvent function
//  * Regularly cleans up inactive client connections
//  */
// void Server::run() {
// 	const int MAX_EVENTS = 1024;
// 	struct kevent eventList[MAX_EVENTS];

// 	while (true) {
// 		int nev = kevent(kq, nullptr, 0, eventList, MAX_EVENTS, nullptr);
// 		if (nev == -1) {
// 			std::cerr << "kevent error: " << strerror(errno) << std::endl;
// 			break;
// 		}

// 		for (int i = 0; i < nev; ++i) {
// 			processEvent(eventList[i]);
// 		}

// 		checkTimeouts();
// 	}
// }
