#pragma once

# include <string>
# include <iostream>
# include <unistd.h>
# include <sys/socket.h>
# include <sys/wait.h>
# include <vector>
# include <unordered_map>
# include "RequestParser.hpp"
#include <fstream>
# include <map>
class Response {
public:
	Response(RequestParser &req, const std::string& documentRoot);
	Response(Response &other);
	~Response();

	std::string get_response(void);
	RequestParser& get_request(void);

private:
	std::string exec_script(void);
	std::string serve_static_file(void);
	std::string get_error_response(const int errorCode);
	std::string get_content_type(const std::string& path) const;
	void set_env(void);

	RequestParser&	_request;

	std::unordered_map<std::string, std::string> _headers;
	std::string _body;
	// int _statuscode;
	std::unordered_map<std::string, std::string> _environment;
	// char **_env;
	std::string _documentRoot;
};

// issues:
// 		env completely missing
// 		no response srtucture to save stuff to
// 		status codes not set
// 		logging would be nice
// 		non blocking io (fcntl or kqueue)
// 		Break Down the CGI Processing into States?
// 		match nginx response structure
// 		chunked requests?
// 		response headers?

