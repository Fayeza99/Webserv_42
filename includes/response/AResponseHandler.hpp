#pragma once

#include <string>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <map>

#include "RequestParser.hpp"
#include "ClientState.hpp"
#include "utils.hpp"

#define PATH_MAX 1024
#define BUFFER_SIZE 1024

class AResponseHandler
{
public:
	AResponseHandler(ClientState &client);
	virtual ~AResponseHandler(void);

	virtual void getResponse(void) = 0;
	virtual void setFilePath() = 0;

	std::string const &getRequest() const;
	std::string const &getMethod() const;
	std::string const &getUri() const;
	std::string const &getHttpVersion() const;
	std::string const &getBody() const;
	std::unordered_map<std::string, std::string> const &getHeaders() const;
	std::map<int, std::string> const &getErrorPages() const;
	bool methodAllowed() const;

	ClientState &_client;
	RequestParser &_request;
	std::string _filePath;
	LocationConfig _location;
};