#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include <cctype>
#include <cstdlib>

class ServerConfig;

class LocationConfig
{
public:
	std::string uri;
	std::string document_root;
	bool autoIndex;
	bool redirect;
	std::string redirect_uri;
	std::vector<std::string> default_files;
	std::set<std::string> supported_methods;
	std::map<std::string, std::string> cgi_paths;

	LocationConfig();
	LocationConfig(const LocationConfig &c);
	void getLocation(const ServerConfig &serverConfig, const std::string &uri);
	LocationConfig &operator=(const LocationConfig &c);
};

class ServerConfig
{
public:
	int listen_port;
	bool autoIndex;
	std::vector<std::string> hostnames;
	std::map<int, std::string> error_pages;
	std::vector<LocationConfig> locations;

	ServerConfig();
};

class GlobalConfig
{
public:
	std::vector<ServerConfig> servers;
	unsigned long client_max_body_size;

	GlobalConfig();
};
