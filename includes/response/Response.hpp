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
# include "ClientState.hpp"
# include "utils.hpp"
class Response {
public:
	Response(RequestParser &req, ClientState& clientState);
	Response(Response &other);
	~Response();

	std::string get_response(void);
	RequestParser& get_request(void);
	int get_status(void);

private:
	std::string exec_script(void);
	std::string serve_static_file(void);
	std::string get_error_response(const int errorCode);
	std::string get_content_type(const std::string& path) const;
	void set_env(void);

	RequestParser& _request;
	std::string _response;
	ClientState& _clientState;

	std::map<std::string, std::string> _headers;
	std::string _body;
	int _statuscode;
	std::map<std::string, std::string> _env;
	// char **_environment;
	// char **_argv;

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

