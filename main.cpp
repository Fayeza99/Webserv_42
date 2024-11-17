


int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <config_file_path>" << std::endl;
		return 1;
	}

	std::string configFilePath = argv[1];
	std::ifstream configFile(configFilePath);
	if (!configFile.is_open())
	{
		std::cerr << "Failed to open config file: " << configFilePath << std::endl;
		return 1;
	}

	// Read the entire configuration file into a string
	std::string configData((std::istreambuf_iterator<char>(configFile)),
						   std::istreambuf_iterator<char>());
	configFile.close();

	// Parsing configuration using your Parser class
	Parser parser(configData);
	GlobalConfig globalConfig;
	try
	{
		globalConfig = parser.parse(); // Parse and get the configuration data
	}
	catch (const std::exception &e)
	{
		std::cerr << "Failed to parse configuration: " << e.what() << std::endl;
		return 1;
	}

	std::vector<int> server_fds;
	try
	{
		server_fds = parser.initializeSockets(globalConfig);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Failed to initialize sockets: " << e.what() << std::endl;
		return 1;
	}

	// Proceed to accept connections
	while (true)
	{
		for (int server_fd : server_fds)
		{
			struct sockaddr_in address;
			int addrlen = sizeof(address);
			int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
			if (new_socket < 0)
			{
				perror("accept");
				continue;
			}

			char buffer[1024] = {0};
			read(new_socket, buffer, 1024);
			std::cout << "Received: " << buffer << std::endl;
			std::string htmlFilePath = "index.html"; // Path to your HTML file
			std::ifstream htmlFile(htmlFilePath);
			if (!htmlFile.is_open())
			{
				std::cerr << "Failed to open HTML file: " << htmlFilePath << std::endl;
				close(new_socket);
				continue;
			}

			// Read the file content into a string
			std::stringstream htmlStream;
			htmlStream << htmlFile.rdbuf();
			std::string htmlContent = htmlStream.str();
			htmlFile.close();

			std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " +
									   std::to_string(htmlContent.size()) + "\r\n\r\n" + htmlContent;

			// Send the response
			send(new_socket, httpResponse.c_str(), httpResponse.size(), 0);
			close(new_socket);
		}
	}

	// Close all server sockets before exiting
	for (int server_fd : server_fds)
	{
		close(server_fd);
	}

	return 0;
}
