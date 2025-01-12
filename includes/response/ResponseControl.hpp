#pragma once

# include <string>
# include <cstdio>
# include <iostream>
# include <unistd.h>
# include <fcntl.h>
# include <sys/socket.h>
# include <sys/wait.h>
# include <vector>
# include <algorithm>
# include <fstream>
# include <map>

# include "RequestParser.hpp"
# include "ClientState.hpp"
# include "ResponseHandler.hpp"
# include "utils.hpp"

# define PATH_MAX 1024
# define BUFFER_SIZE 1024

class ResponseControl
{
public:
	ResponseControl(void);
	~ResponseControl(void);

	void setHandler(void);
	void getResponse(void);

private:
	AResponseHandler *_handler;
	ClientState& _client;
	RequestParser& _request;
};

