#include "utils.hpp"
#include "GlobalConfig.hpp"

void print_log(const char *color, std::string msg)
{
	time_t timestamp;
	time(&timestamp);
	std::string time(ctime(&timestamp));
	time = time.substr(11, 8);
	std::cerr << color << time << " --- " << msg << RESET << std::endl;
}

std::string readConfigFile(const std::string &configFilePath)
{
	std::ifstream configFile(configFilePath);
	if (!configFile.is_open())
	{
		throw std::ios_base::failure("Failed to open file: " + configFilePath);
	}

	std::string configData((std::istreambuf_iterator<char>(configFile)), std::istreambuf_iterator<char>());
	configFile.close();

	return configData;
}

// Following are function to print the data recieved by the parser to be removed later

void printIndent(int indent)
{
	for (int i = 0; i < indent; ++i)
		std::cout << ' ';
}

void printLocationConfig(const LocationConfig &loc, int indent = 0)
{
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
	for (const std::string &file : loc.default_files)
	{
		printIndent(indent + 4);
		std::cout << "- " << file << std::endl;
	}

	printIndent(indent + 2);
	std::cout << "Supported Methods:" << std::endl;
	for (const std::string &method : loc.supported_methods)
	{
		printIndent(indent + 4);
		std::cout << "- " << method << std::endl;
	}

	// printIndent(indent + 2);
	// std::cout << "CGI Paths:" << std::endl;
	// for (const auto &cgi : loc.cgi_paths)
	// {
	// 	printIndent(indent + 4);
	// 	std::cout << cgi.first << ": " << cgi.second << std::endl;
	// }
}

void printServerConfig(const ServerConfig &server, int indent = 0)
{
	printIndent(indent);
	std::cout << "Server:" << std::endl;

	printIndent(indent + 2);
	std::cout << "Listen Port: " << server.listen_port << std::endl;

	printIndent(indent + 2);
	std::cout << "AutoIndex: " << server.autoIndex << std::endl;

	printIndent(indent + 2);
	std::cout << "Servernames:" << std::endl;
	for (const std::string &name : server.servernames)
	{
		printIndent(indent + 4);
		std::cout << "- " << name << std::endl;
	}

	printIndent(indent + 2);
	std::cout << "Error Pages:" << std::endl;
	for (const auto &error_page : server.error_pages)
	{
		printIndent(indent + 4);
		std::cout << error_page.first << ": " << error_page.second << std::endl;
	}

	printIndent(indent + 2);
	std::cout << "Locations:" << std::endl;
	for (const LocationConfig &loc : server.locations)
	{
		printLocationConfig(loc, indent + 4);
	}
}

void printGlobalConfig(const GlobalConfig &config, int indent = 0)
{
	printIndent(indent);
	std::cout << "Global Configuration:" << std::endl;

	printIndent(indent + 2);
	std::cout << "Client Max Body Size: " << config.client_max_body_size << std::endl;

	printIndent(indent + 2);
	std::cout << "Servers:" << std::endl;
	for (const ServerConfig &server : config.servers)
	{
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
std::string getDocumentRoot(const ServerConfig &serverConfig, const std::string &uri)
{
	std::string bestMatchRoot = "";
	size_t bestMatchLength = 0;

	for (std::vector<LocationConfig>::const_iterator it = serverConfig.locations.begin(); it != serverConfig.locations.end(); ++it)
	{
		const LocationConfig &loc = *it;

		if (uri.compare(0, loc.uri.size(), loc.uri) == 0)
		{
			if (loc.uri.size() > bestMatchLength)
			{
				bestMatchRoot = loc.document_root;
				bestMatchLength = loc.uri.size();
			}
		}
	}

	return bestMatchRoot;
}

std::string getContentType(const std::string &path)
{
	static std::map<std::string, std::string> mime_types = {
		{".html", "text/html"},
		{".htm", "text/html"},
		{".css", "text/css"},
		{".js", "application/javascript"},
		{".png", "image/png"},
		{".jpg", "image/jpeg"},
		{".jpeg", "image/jpeg"},
		{".gif", "image/gif"},
		{".txt", "text/plain"},
		{".pdf", "application/pdf"},
	};
	size_t dot = path.find_last_of('.');
	if (dot != std::string::npos)
	{
		std::string fileExtension = path.substr(dot);
		if (mime_types.find(fileExtension) != mime_types.end())
			return mime_types.find(fileExtension)->second;
	}
	return "application/octet-stream";
}
