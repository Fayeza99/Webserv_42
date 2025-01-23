#include "server.hpp"

Server::Server() : _request(nullptr), _response(nullptr) {}

Server::~Server()
{
	if (_request)
		delete _request;
	if (_response)
		delete _response;
}

/**
 * @brief this function calls the readConfigFile and
 * then parses the content of the config file and store
 * it in serverConfig variale
 *
 * @param configFilePath comes from the main argv[1]
 */
void Server::configure(const std::string &configFilePath)
{
	Parser parser(readConfigFile(configFilePath));
	GlobalConfig globalConfig = parser.parse();
	printGlobalConfig(globalConfig, 4);
	serverConfigs = globalConfig.servers;
}

/**
 * @brief function to set a soket to non-blocking mode
 *
 * @param fd file descriptor of the socket
 */
void Server::setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
	{
		throw std::runtime_error("Failed to set socket to non-blocking");
	}
}

/**
 * @brief Initializes the server by setting up the event notification system and creating server sockets.
 *
 * The `setup` function performs essential initialization tasks required before the server can start accepting client connections. Specifically, it:
 *
 * 1. **Initializes the `kqueue` Event Notification Mechanism:**
 *    - Creates a new kernel event queue using `kqueue()`.
 *    - Throws a `std::runtime_error` if `kqueue` fails to initialize.
 *
 * 2. **Checks for Duplicate Server Configurations:**
 *    - Iterates through the list of server configurations (`serverConfigs`) to identify duplicates based on listening ports and servernames.
 *    - For each unique configuration, invokes the `createServerSocket` method to establish a corresponding server socket.
 *
 * **Notes:**
 * - The current implementation compares each server configuration with itself, which might lead to all configurations being marked as duplicates. To rectify this, consider modifying the inner loop to skip self-comparisons.
 * - Optimizing the duplicate detection logic can enhance performance, especially when dealing with a large number of server configurations.
 *
 * @see Server::createServerSocket
 * @see kqueue()
 */
void Server::setup()
{
	// Initializing Kqueue
	KqueueManager::initialize();

	// Checking for duplicate Server if the server doesn't already exist then make a new one
	bool serverDuplicate;
	for (auto config1 = serverConfigs.begin(); config1 != serverConfigs.end(); ++config1)
	{
		serverDuplicate = false;
		for (auto config2 = serverConfigs.begin(); config2 != serverConfigs.end(); ++config2)
		{
			if (config1 != config2 && config2->listen_port == config1->listen_port)
			{
				for (const std::string &server_name1 : config1->servernames)
				{
					for (const std::string &server_name2 : config2->servernames)
					{
						if (server_name1 == server_name2)
						{
							serverDuplicate = true;
						}
					}
				}
			}
		}
		if (!serverDuplicate)
		{
			createServerSocket(*config1);
		}
	}
}

/**
 * @brief Create a new server with the provided server config
 *
 * @details
 * ->Creating Socket:
 *		- AF_INET stands for Address Family - Internet. It specifies that the socket will use the IPv4 addressing scheme.
 *		- SOCK_STREAM is a socket type that provides reliable, ordered, and error-checked delivery of a stream of bytes. It is typically associated with the TCP (Transmission Control Protocol).
 *
 * -> Setting Socket Options:
 *		- int opt: This variable will be used to enable a specific socket option 1 means set/on.
 *		- SOL_SOCKET refers to the socket layer itself. It's used to set options that are generic to sockets, regardless of the underlying protocol.
 *		- When developing and testing server applications, you might stop and restart the server
 *		  multiple times in quick succession. Without SO_REUSEADDR, each restart could fail because the port
 *		  remains occupied for a short period after the server stops. Enabling SO_REUSEADDR allows the new
 *		  server instance to bind to the same port immediately, facilitating a smoother development workflow.
 *
 * -> Initializing Server Address
 *		The `sockaddr_in` structure is used for handling internet addresses. The steps
 *		involved in initializing this structure are as follows:
 *
 *		1. **Declaration and Initialization:**
 *			- A `sockaddr_in` structure named `serverAddr` is declared to hold the
 *			  server's address information.
 *			- `memset` is used to zero out the entire structure to ensure that all
 *			  fields are initialized to zero, preventing any unintended behavior.
 *
 *		2. **Setting the Address Family:**
 *			- `sin_family` is set to `AF_INET`, indicating that the address family is
 *			  IPv4. This constant specifies that the socket will use IPv4 addressing.
 *
 *		3. **Setting the IP Address:**
 *			- `sin_addr.s_addr` is set to `INADDR_ANY`, which allows the server to accept
 *			  connections on any of the host's IP addresses. This is useful when the server
 *			  has multiple network interfaces or IP addresses.
 *
 *		4. **Setting the Port Number:**
 *			- `sin_port` is set using `htons(config.listen_port)`. The `htons` function
 *			  converts the port number from host byte order to network byte order, which is
 *			  necessary for proper communication over the network.
 *
 * -> Binding the server socket
 *		The `bind` function assigns the address specified by `serverAddr` to the socket
 *		identified by `serverSocket`. This involves associating the socket with an IP
 *		address and a port number, allowing the server to receive data sent to that
 *		address and port.
 *
 */
void Server::createServerSocket(ServerConfig &config)
{
	int serverSocket;

	// Creating the Socket
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
	{
		throw std::runtime_error("Failed to create a server socket!");
	}

	// Setting the Socket FD as non-blocking
	if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) == -1)
	{
		throw std::runtime_error("Failed to set socket to non-blocking");
	}

	// Setting the server options
	int opt = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		close(serverSocket);
		throw std::runtime_error("Failed to set socket options");
	}

	// Initializing Server Address
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(config.listen_port);

	// Binding the Server Socket
	if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
	{
		close(serverSocket);
		throw std::runtime_error("Failed to bind socket");
	}

	// Enable Listening on the docket
	if (listen(serverSocket, SOMAXCONN) == -1)
	{
		close(serverSocket);
		throw std::runtime_error("Failed to listen on socket");
	}

	// Registering the socket fd with kq
	KqueueManager::registerEvent(serverSocket, EVFILT_READ, EV_ADD | EV_ENABLE);

	// Adding the newly create server socket to the map
	serverSockets[serverSocket] = config;
	print_log(WHITE, "Server is listening on port " + std::to_string(config.listen_port));
}

/**
 * @brief Function to remove a client
 *
 * Unegisters read and write events for the client socket
 * Closes the client socket
 * Erases the clients state from clients
 *
 * @param clientSocket
 */
void Server::removeClient(int clientSocket)
{
	KqueueManager::deregisterEvent(clientSocket);
	close(clientSocket);
	clients.erase(clientSocket);
}

/**
 * @brief Handles accepting new client connections
 *
 * Ensures that I/O operations on the client socket do not block the server
 * Registers the client socket for read events (EVFILT_READ)
 * Initializes a ClientState for tracking request and response data
 *
 */
void Server::handleAccept(int serverSocket)
{
	// print_log(BLACK, "[FUNC] handleAccept");
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);

	if (clientSocket == -1)
	{
		std::cerr << "[ERROR] Failed to accept connection: " << strerror(errno) << std::endl;
		return;
	}
	try
	{
		setNonBlocking(clientSocket);
	}
	catch (const std::exception &e)
	{
		std::cerr << "[ERROR] Failed to set client socket to non-blocking: " << e.what() << std::endl;
		close(clientSocket);
		return;
	}

	KqueueManager::registerEvent(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE | EV_CLEAR);

	const ServerConfig &config = serverSockets[serverSocket];
	clients.emplace(clientSocket, ClientState(config, clientSocket)); //!!!

	std::string clientip(inet_ntoa(clientAddr.sin_addr));
	clients[clientSocket].clientIPAddress = clientip;
	clients[clientSocket].clientPort = ntohs(clientAddr.sin_port);

	print_log(WHITE, "Accepted new connection from " + clients[clientSocket].clientIPAddress + ":" + std::to_string(clients[clientSocket].clientPort) + ", socket " + std::to_string(clientSocket));
}

unsigned long getContentLength(const std::unordered_map<std::string, std::string> &headers)
{
	unsigned long CL = 0;
	auto isCL = headers.find("Content-Length");
	if (isCL != headers.end())
		CL = stoi(isCL->second);
	return CL;
}

bool isChunked(const std::unordered_map<std::string, std::string> &headers)
{
	auto isCh = headers.find("Transfer-Encoding");
	return (isCh != headers.end() && isCh->second == "chunked");
}

/**
 * @brief Function to process incoming data from clients
 *
 * It reads the incoming data and appends it to the client' request buffer.
 * Detects the end HTTP headers (\r\n\r\n)
 * Prepares a response and stores it in the response buffer
 * Registers the client socket for write events(EVFILT_WRITE) to send the response.
 *
 * @param clientSocket
 */
void Server::handleRead(int clientSocket)
{
	// print_log(BLACK, "[FUNC] handleRead");
	char buffer[BUFFER_SIZE];
	ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);

	if (bytesRead > 0)
	{
		buffer[bytesRead] = '\0';
		clients[clientSocket].lastActive = time(nullptr);
		clients[clientSocket].requestBuffer += buffer;
		_request = new RequestParser(clients[clientSocket].requestBuffer);

		auto isCL = (*_request).getHeaders().find("Content-Length");
		if (isCL != (*_request).getHeaders().end())
		{
			unsigned long CL = stoi(isCL->second);
			if (clients[clientSocket].serverConfig.client_max_body_size < CL)
			{
				print_log(RED, "Request body too long (413)");
				clients[clientSocket].responseBuffer = ErrorHandler::createResponse(413);
				KqueueManager::registerEvent(clientSocket, EVFILT_READ, EV_DELETE);
				KqueueManager::registerEvent(clientSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR);
				return;
			}
			if (CL > 0 && (*_request).getBody().length() < CL)
			{
				// print_log(RED, "Expecting another handleRead");
				delete _request;
				return;
			}
		}
		clients[clientSocket].request = _request;
		print_log(WHITE, "new Request: Method=" + _request->getMethod() + ", Uri=" + _request->getUri());
		_response = new ResponseControl(clients[clientSocket]);
		_response->getResponse();
		if (!(*_response).isCgiRequest())
		{
			delete _response;
			delete _request;
			clients[clientSocket].request = nullptr;
		}
	}
	else if (bytesRead == 0)
	{
		print_log(WHITE, "Client " + std::to_string(clientSocket) + " disconnected");
		removeClient(clientSocket);
	}
	else
	{
		print_log(RED, "Read error on client " + std::to_string(clientSocket) + ", removing client.");
		removeClient(clientSocket);
	}
}

/**
 * @brief Sneds reponses to clients when their sockets are ready for writing
 *
 * Sends the reponse store in the client's response buffer
 * Handles cases where only part of the data is sent, updating buffer accordingly
 * Removes the write event filter once all the data has been sent
 *
 * @param clientSocket
 */
void Server::handleWrite(int clientSocket)
{
	// print_log(BLACK, "[FUNC] handleWrite");
	std::string &response = clients[clientSocket].responseBuffer;
	size_t bytesSent = 0;
	if (!response.empty())
	{
		if (response.length() > BUFFER_SIZE)
		{
			bytesSent = send(clientSocket, response.c_str(), BUFFER_SIZE, 0);
			clients[clientSocket].responseBuffer = (clients[clientSocket].responseBuffer).substr(bytesSent);
			clients[clientSocket].lastActive = time(nullptr);
			return;
		}
		bytesSent = send(clientSocket, response.c_str(), response.size(), 0);
		clients[clientSocket].requestBuffer.clear();
		clients[clientSocket].responseBuffer.clear();
		// print_log(WHITE, "Response sent");
		if (bytesSent > 0)
			clients[clientSocket].lastActive = time(nullptr);
		else if (bytesSent == 0)
		{
			std::cerr << "[ERROR] Client " << clientSocket << " disconnected during write." << std::endl;
			removeClient(clientSocket);
			return;
		}
		else
		{
			std::cerr << "[ERROR] Write error on client " << clientSocket << ", removing client." << std::endl;
			removeClient(clientSocket);
			return;
		}
		if (response.empty())
			KqueueManager::registerEvent(clientSocket, EVFILT_WRITE, EV_DELETE);
	}
}

/**
 * @brief Processes a single event retrieved from the kqueue event loop.
 *
 * The `processEvent` function handles incoming events detected by the kqueue event notification
 * system. It determines the type of event (read or write) and the associated file descriptor,
 * then delegates the handling to the appropriate member functions (`handleAccept`, `handleRead`,
 * or `handleWrite`). Additionally, it manages error events by logging relevant error messages
 * and performing necessary cleanup.
 *
 * @param event A reference to a `kevent` structure representing the event to be processed.
 *
 * @see handleAccept(int), handleRead(int), handleWrite(int), removeClient(int)
 */
void Server::processEvent(struct kevent &event)
{
	// print_log(BLACK, "[FUNC] processEvent");
	int fd = static_cast<int>(event.ident);

	if (event.flags & EV_ERROR)
	{
		if (serverSockets.count(fd))
			std::cerr << "[ERROR] Error on server socket: " << strerror(static_cast<int>(event.data)) << std::endl;
		else
		{
			int clientSocket = static_cast<int>(event.ident);
			std::cerr << "[ERROR] Error on client socket " << clientSocket << ": " << strerror(static_cast<int>(event.data)) << std::endl;
			removeClient(clientSocket);
		}
		return;
	}
	else if (serverSockets.count(fd))
	{
		if (event.filter == EVFILT_READ)
			handleAccept(fd);
	}
	else if (clients.count(fd))
	{
		if (event.filter == EVFILT_READ)
			handleRead(fd);
		else if (event.filter == EVFILT_WRITE)
			handleWrite(fd);
	}
	else
	{
		ClientState *client = findClientByPipeFd(fd);
		if (!client)
		{
			std::cerr << "[ERROR] Unknown fd in processEvent: " << fd << std::endl;
			return;
		}

		if (event.filter == EVFILT_READ)
		{
			bool finished = CgiHandler::readFromCgiStdout(*client);
			if (finished)
			{
				delete _request;
				delete _response;
			}
		}
		else if (event.filter == EVFILT_WRITE)
		{
			CgiHandler::writeToCgiStdin(*client);
		}
	}
}

ClientState *Server::findClientByPipeFd(int fd)
{
	for (auto &client : clients)
	{
		if (client.second.cgiInputFd == fd || client.second.cgiOutputFd == fd)
		{
			return &client.second;
		}
	}
	return nullptr;
}

/**
 * @brief Closes connections that have been idle beyond a specified timeout duration
 *
 * Defines how long a client can remain idle before being disconnected
 * Safely iterates through the clients map, removing timed-out clients
 *
 */
void Server::checkTimeouts()
{
	time_t currentTime = time(nullptr);
	const int TIMEOUT_DURATION = 10; // seconds

	for (auto it = clients.begin(); it != clients.end();)
	{
		if (currentTime - it->second.lastActive > TIMEOUT_DURATION)
		{
			int clientSocket = it->first;
			print_log(WHITE, "Client " + std::to_string(clientSocket) + " timed out");
			KqueueManager::deregisterEvent(clientSocket);
			close(clientSocket);
			it = clients.erase(it);
		}
		else
		{
			++it;
		}
	}
}

/**
 * @brief Putting everything together
 *
 * Continously eaits for and processes incoming events.
 * Delegates each event to the processEvent function
 * Regularly cleans up inactive client connections
 */
void Server::run()
{
	const int MAX_EVENTS = 1024;
	struct kevent eventList[MAX_EVENTS];

	while (true)
	{
		int nev = kevent(KqueueManager::getKqFd(), nullptr, 0, eventList, MAX_EVENTS, nullptr);
		if (nev == -1)
		{
			std::cerr << "[ERROR] kevent error: " << strerror(errno) << std::endl;
			break;
		}
		for (int i = 0; i < nev; ++i)
		{
			processEvent(eventList[i]);
		}
		checkTimeouts();
	}
}
