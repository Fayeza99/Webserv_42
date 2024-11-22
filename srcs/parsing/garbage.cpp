// int main(int argc, char** argv) {
//     if (argc < 2) {
//         std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
//         return 1;
//     }
//     std::string configFile = argv[1];
//     std::ifstream file(configFile);
//     if (!file.is_open()) {
//         std::cerr << "Failed to open file: " << configFile << std::endl;
//         return 1;
//     }
//     std::string configContent((std::istreambuf_iterator<char>(file)),
//                               std::istreambuf_iterator<char>());

//     try {
//         Parser parser(configContent);
//         GlobalConfig config = parser.parse();
//         std::cout << "Configuration parsed successfully." << std::endl;
//         // Further processing with `config`...
//     } catch (const std::exception& e) {
//         std::cerr << "Failed to parse configuration: " << e.what() << std::endl;
//         return 1;
//     }
//     return 0;
// }

// int main(int argc, char *argv[])
// {
// 	if (argc < 2)
// 	{
// 		std::cerr << "Usage: " << argv[0] << " <config_file_path>" << std::endl;
// 		return 1;
// 	}

// 	std::string configFilePath = argv[1];
// 	std::ifstream configFile(configFilePath);
// 	if (!configFile.is_open())
// 	{
// 		std::cerr << "Failed to open config file: " << configFilePath << std::endl;
// 		return 1;
// 	}

// 	// Read the entire configuration file into a string
// 	std::string configData((std::istreambuf_iterator<char>(configFile)),
// 						   std::istreambuf_iterator<char>());
// 	configFile.close();

// 	// Parsing configuration using your Parser class
// 	Parser parser(configData); // Assuming Parser class takes a string and parses it
// 	GlobalConfig globalConfig;
// 	try
// 	{
// 		globalConfig = parser.parse(); // Parse and get the configuration data
// 	}
// 	catch (const std::exception &e)
// 	{
// 		std::cerr << "Failed to parse configuration: " << e.what() << std::endl;
// 		return 1;
// 	}

// 	int port = globalConfig.servers.empty() ? 8080 : globalConfig.servers[0].listen_port;

// 	int server_fd, new_socket;
// 	struct sockaddr_in address;
// 	int opt = 1;
// 	int addrlen = sizeof(address);

// 	server_fd = socket(AF_INET, SOCK_STREAM, 0);
// 	if (server_fd == 0)
// 	{
// 		perror("socket failed");
// 		exit(EXIT_FAILURE);
// 	}

// 	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
// 	{
// 		perror("setsockopt");
// 		exit(EXIT_FAILURE);
// 	}

// 	address.sin_family = AF_INET;
// 	address.sin_addr.s_addr = INADDR_ANY;
// 	address.sin_port = htons(port);

// 	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
// 	{
// 		perror("bind failed");
// 		exit(EXIT_FAILURE);
// 	}

// 	if (listen(server_fd, 3) < 0)
// 	{
// 		perror("listen");
// 		exit(EXIT_FAILURE);
// 	}

// 	std::cout << "Listening on port " << port << std::endl;

// 	std::string indexPath = "hello.html"; // Adjust this path as necessary
// 	std::ifstream indexFile(indexPath);
// 	std::string htmlContent((std::istreambuf_iterator<char>(indexFile)),
// 							std::istreambuf_iterator<char>());
// 	indexFile.close();

// 	while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) >= 0)
// 	{
// 		char buffer[1024] = {0};
// 		read(new_socket, buffer, 1024);
// 		std::cout << "Received: " << buffer << std::endl;

// 		std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(htmlContent.size()) + "\r\n\r\n" + htmlContent;
// 		send(new_socket, httpResponse.c_str(), httpResponse.size(), 0);

// 		std::cout << "Response sent to client" << std::endl;

// 		close(new_socket);
// 	}

// 	close(server_fd);
// 	return 0;
// }
