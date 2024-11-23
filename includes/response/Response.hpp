#pragma once

# include <string>
# include <iostream>
# include <unistd.h>
# include <sys/socket.h>
# include <vector>
# include "RequestParser.hpp"

class Response {
public:
	Response(RequestParser &req);
	Response(Response &other);
	~Response();

	std::string		get_response(void);
	RequestParser&	get_request(void);

private:
	std::string		exec_script(void);

	RequestParser&	_request;

};

