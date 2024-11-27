#include "utils.hpp"

std::string readConfigFile(const std::string& configFilePath) {
	std::ifstream configFile(configFilePath);
	if (!configFile.is_open()) {
		throw std::ios_base::failure("Failed to open file: " + configFilePath);
	}

	std::string configData((std::istreambuf_iterator<char>(configFile)), std::istreambuf_iterator<char>());
	configFile.close();

	return configData;
}

// Following are function to print the data recieved by the parser to be removed later

void printIndent(int indent) {
	for (int i = 0; i < indent; ++i)
		std::cout << ' ';
}

void printLocationConfig(const LocationConfig& loc, int indent = 0) {
	printIndent(indent);
	std::cout << "Location:" << std::endl;

	printIndent(indent + 2);
	std::cout << "URI: " << loc.uri << std::endl;

	printIndent(indent + 2);
	std::cout << "Document Root: " << loc.document_root << std::endl;

	printIndent(indent + 2);
	std::cout << "Default Files:" << std::endl;
	for (const std::string& file : loc.default_files) {
		printIndent(indent + 4);
		std::cout << "- " << file << std::endl;
	}

	printIndent(indent + 2);
	std::cout << "Supported Methods:" << std::endl;
	for (const std::string& method : loc.supported_methods) {
		printIndent(indent + 4);
		std::cout << "- " << method << std::endl;
	}

	printIndent(indent + 2);
	std::cout << "CGI Paths:" << std::endl;
	for (const auto& cgi : loc.cgi_paths) {
		printIndent(indent + 4);
		std::cout << cgi.first << ": " << cgi.second << std::endl;
	}
}

void printServerConfig(const ServerConfig& server, int indent = 0) {
	printIndent(indent);
	std::cout << "Server:" << std::endl;

	printIndent(indent + 2);
	std::cout << "Listen Port: " << server.listen_port << std::endl;

	printIndent(indent + 2);
	std::cout << "Hostnames:" << std::endl;
	for (const std::string& hostname : server.hostnames) {
		printIndent(indent + 4);
		std::cout << "- " << hostname << std::endl;
	}

	printIndent(indent + 2);
	std::cout << "Error Pages:" << std::endl;
	for (const auto& error_page : server.error_pages) {
		printIndent(indent + 4);
		std::cout << error_page.first << ": " << error_page.second << std::endl;
	}

	printIndent(indent + 2);
	std::cout << "Locations:" << std::endl;
	for (const LocationConfig& loc : server.locations) {
		printLocationConfig(loc, indent + 4);
	}
}

void printGlobalConfig(const GlobalConfig& config, int indent = 0) {
	printIndent(indent);
	std::cout << "Global Configuration:" << std::endl;

	printIndent(indent + 2);
	std::cout << "Client Max Body Size: " << config.client_max_body_size << std::endl;

	printIndent(indent + 2);
	std::cout << "Servers:" << std::endl;
	for (const ServerConfig& server : config.servers) {
		printServerConfig(server, indent + 4);
	}
}
