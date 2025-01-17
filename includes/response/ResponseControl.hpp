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
#include "AResponseHandler.hpp"
#include "StaticHandler.hpp"
#include "UploadHandler.hpp"
#include "DeleteHandler.hpp"
#include "CgiHandler.hpp"

class ResponseControl
{
public:
	ResponseControl(ClientState &client);
	~ResponseControl(void);

	void setHandler(void);
	bool isCgiRequest(void);
	AResponseHandler *getHandler(void);
	void getResponse(void);

private:
	LocationConfig _location;
	AResponseHandler *_handler;
	ClientState &_client;
	RequestParser &_request;
};
