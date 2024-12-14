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
	std::cout << "Redirect: " << loc.redirect_uri << std::endl;

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
	std::cout << "AutoIndex: " << server.autoIndex << std::endl;

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



/**
 * @brief Get the Document Root
 *
 * Check if the location's URI is a prefix of the requested URI.
 * For example: loc.uri = "/data", uri = "/data/info"
 * uri.compare(0, loc.uri.size(), loc.uri) == 0 means uri starts with loc.uri
 * @param serverConfig
 * @param uri
 * @return std::string
 */
std::string getDocumentRoot(const ServerConfig &serverConfig, const std::string &uri) {
	std::string bestMatchRoot = "";
	size_t bestMatchLength = 0;

	for (std::vector<LocationConfig>::const_iterator it = serverConfig.locations.begin(); it != serverConfig.locations.end(); ++it) {
		const LocationConfig &loc = *it;

		if (uri.compare(0, loc.uri.size(), loc.uri) == 0) {
			if (loc.uri.size() > bestMatchLength) {
				bestMatchRoot = loc.document_root;
				bestMatchLength = loc.uri.size();
			}
		}
	}

	return bestMatchRoot;
}


LocationConfig getLocation(const ServerConfig &serverConfig, const std::string &uri) {
	LocationConfig bestMatchLocation;
	size_t bestMatchLength = 0;

	std::string _uri;
	std::string document_root;
	std::string redirect_uri;
	bool redirect;
	std::vector<std::string> default_files;
	std::set<std::string> supported_methods;
	std::map<std::string, std::string> cgi_paths;

	// choose the location from the config file, that fits the request best
	for (const LocationConfig &loc : serverConfig.locations) {
		if (uri.compare(0, loc.uri.size(), loc.uri) == 0) {

			if (loc.uri.size() > bestMatchLength) {
				_uri = loc.uri;
				document_root = loc.document_root;
				default_files = loc.default_files;
				supported_methods = loc.supported_methods;
				cgi_paths = loc.cgi_paths;
				redirect = loc.redirect;
				redirect_uri = loc.redirect_uri;
				bestMatchLength = loc.uri.size();
			}
		}
	}

	bestMatchLocation.uri = _uri;
	bestMatchLocation.document_root = document_root;
	bestMatchLocation.default_files = default_files;
	bestMatchLocation.supported_methods = supported_methods;
	bestMatchLocation.cgi_paths = cgi_paths;
	bestMatchLocation.redirect = redirect;
	bestMatchLocation.redirect_uri = redirect_uri;

	return bestMatchLocation;
}
