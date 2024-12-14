#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include <cctype>
#include <cstdlib>

class LocationConfig
{
public:
	std::string uri;
	std::string document_root;
	bool autoIndex;
	bool redirect = false;
	std::string redirect_uri;
	std::vector<std::string> default_files;
	std::set<std::string> supported_methods;
	std::map<std::string, std::string> cgi_paths;
	LocationConfig() : uri(""), document_root("") {}
};

class ServerConfig
{
public:
	int listen_port;
	bool autoIndex;
	std::vector<std::string> hostnames;
	std::map<int, std::string> error_pages;
	std::vector<LocationConfig> locations;

	ServerConfig() : listen_port(80) {}
};

class GlobalConfig
{
public:
	std::vector<ServerConfig> servers;
	unsigned long client_max_body_size;

	GlobalConfig() : client_max_body_size(8192) {}
};
