#include "GlobalConfig.hpp"
#include "utils.hpp"

ServerConfig::ServerConfig() : listen_port(80), autoIndex(false) {}

GlobalConfig::GlobalConfig() : client_max_body_size(8192) {}

LocationConfig::LocationConfig(const LocationConfig &c)
{
	*this = c;
}

LocationConfig::LocationConfig()
	: uri(""), document_root(""), autoIndex(false), redirect(false), redirect_uri("") {}

LocationConfig &LocationConfig::operator=(const LocationConfig &c)
{
	this->uri = c.uri;
	this->document_root = c.document_root;
	this->default_files = c.default_files;
	this->supported_methods = c.supported_methods;
	this->cgi_paths = c.cgi_paths;
	this->autoIndex = c.autoIndex;
	this->redirect = c.redirect;
	this->redirect_uri = c.redirect_uri;
	return *this;
}

void LocationConfig::getLocation(const ServerConfig &serverConfig, const std::string &match_uri)
{
	size_t bestMatchLength = 0;

	// choose the location from the config file, that fits the request best
	for (const LocationConfig &loc : serverConfig.locations)
	{
		std::string dir_uri = loc.uri;
		if (dir_uri.at(dir_uri.length()-1) != '/')
			dir_uri += '/';
		if (match_uri.compare(0, dir_uri.length(), dir_uri) == 0 || loc.uri == match_uri)
		{
			std::cerr << RED << "--" << match_uri << "--" << dir_uri << "--\n" << RESET;
			if (dir_uri.length() > bestMatchLength)
			{
				*this = loc;
				bestMatchLength = dir_uri.length();
			}
		}
	}
}
