#pragma once

# include <string>
# include <iostream>
# include <unistd.h>
# include <fcntl.h>
# include <sys/socket.h>
# include <sys/wait.h>
# include <vector>
# include <algorithm>
# include "RequestParser.hpp"
# include <fstream>
# include <map>
# include "ClientState.hpp"
# include "utils.hpp"

# define PATH_MAX 1024
# define BUFFER_SIZE 1024

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
	std::string cgi_parent(int in_pipe[2], int out_pipe[2], pid_t pid);
	void cgi_child(int in_pipe[2], int out_pipe[2]);
	void set_env(void);

	std::string serve_static_file(void);
	std::string handle_delete(void);
	std::string get_error_response(const int errorCode);
	bool method_allowed(void);
	std::string get_content_type(const std::string& path) const;

	RequestParser& _request;
	std::string _response;
	ClientState& _clientState;
	LocationConfig _location;
	std::string _documentRoot;
	std::string _filePath;

	std::map<std::string, std::string> _headers;
	std::string _body;
	int _statuscode;
	std::vector<std::string> _env;
	std::vector<char *> _environment;
	// char **_argv;

};

void	test_1(ClientState& clientstate);
void	test_2(ClientState& clientstate);
void	test_3(ClientState& clientstate);
void	test_4(ClientState& clientstate);
void	test_5(ClientState& clientstate);

// issues:
// 		env completely missing
// 		status codes not set
// 		logging would be nice
// 		non blocking io (fcntl or kqueue)
// 		Break Down the CGI Processing into States?
// 		match nginx response structure
// 		chunked requests?
// 		response headers?

