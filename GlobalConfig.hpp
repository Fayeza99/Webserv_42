#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include <cctype>
#include <cstdlib>


class LocationConfig {
public:
    std::string uri;
    std::string document_root;
    std::vector<std::string> default_files;
    std::set<std::string> supported_methods;
    std::map<std::string, std::string> cgi_paths;  // Key: extension, Value: handler path

    LocationConfig() : uri(""), document_root("") {}
};

class ServerConfig {
public:
    int listen_port;
    std::vector<std::string> hostnames;
    std::map<int, std::string> error_pages;
    std::vector<LocationConfig> locations;

    ServerConfig() : listen_port(80) {}
};

class GlobalConfig {
public:
    std::vector<ServerConfig> servers;
    unsigned long client_max_body_size;

    GlobalConfig() : client_max_body_size(8192) {}  // Default to 8192 bytes
};
