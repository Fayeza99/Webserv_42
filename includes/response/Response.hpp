#pragma once

# include <string>
# include <iostream>
# include <unistd.h>
# include <sys/socket.h>
# include <vector>
# include "RequestParser.hpp"
# include <map>

class Response {
public:
	Response(RequestParser &req, const std::string& documentRoot);
	Response(Response &other);
	~Response();

	std::string		get_response(void);
	RequestParser&	get_request(void);

private:
	std::string		exec_script(void);

	std::string	serve_static_file(void);
	std::string get_content_type(const std::string& path) const;

	RequestParser&	_request;
	std::string _documentRoot;
};

