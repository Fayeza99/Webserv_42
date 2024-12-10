#include "Response.hpp"

// respond to .py request (CGI)
std::string	Response::exec_script() {
	int					in_pipe[2];// redirect request body to stdin of script
	int					out_pipe[2];// redirect script output to stringstream

	// std::cerr << "exec_script called on: " << filePath << std::endl;
	// check that script exists and set environment
	if (FILE *file = fopen(_filePath.c_str(), "r"))
		fclose(file);
	else
		return (get_error_response(404));
	set_env();

	// in_pipe:		parent writes request body to [1], script reads it from [0]
	// out_pipe:	script writes full response to [1], parent reads it from [0] and returns to server
	if (pipe(in_pipe) == -1 || pipe(out_pipe) == -1)
		return (get_error_response(500));

	pid_t	pid = fork();
	if (pid == 0) {
		cgi_child(in_pipe, out_pipe);
	} else if (pid > 0) {
		return (cgi_parent(in_pipe, out_pipe, pid));
	}
	return (get_error_response(500));
}

 /**
 * child process of the cgi
 * execute the script from its directory 
 */
void Response::cgi_child(int in_pipe[2], int out_pipe[2]) {
	std::string	script_name = _filePath.substr(_filePath.find_last_of("/") + 1);

	close(in_pipe[1]);//child doesnt write request body
	close(out_pipe[0]);//child doesnt read its own output (duh)
	if (dup2(in_pipe[0], STDIN_FILENO) == -1 || dup2(out_pipe[1], STDOUT_FILENO) == -1)
		exit(1);
	close(in_pipe[0]);//dup successful, fds no longer needed
	close(out_pipe[1]);//now acessible through STDIN and STDOUT

	if (chdir((_filePath.substr(0, _filePath.find_last_of("/"))).c_str()) == -1)
		exit(1);

	char *args[] = {(char *)"/usr/bin/python3", (char *)script_name.c_str(), NULL};
	// std::cerr << "executing cgi script: " << script_name << "\n";
	execve(args[0], args, _environment.data());
	exit(1);//execve failed
}

 /**
 * parent process of the cgi
 * write request body to script input
 * read response from STDOUT
 */
std::string Response::cgi_parent(int in_pipe[2], int out_pipe[2], pid_t pid) {
	std::ostringstream	response;
	char				buffer[BUFFER_SIZE];
	size_t				bytes_read = 1;

	close(in_pipe[0]);//parent doesnt read the request body
	close(out_pipe[1]);//parent doesnt have the response to write here
	if (!_request.getBody().empty()) {
		if (write(in_pipe[1], _request.getBody().c_str(), _request.getBody().size()) == -1)
			std::cerr << "write failed\n";
	}
	while (bytes_read > 0) {
		bytes_read = read(out_pipe[0], buffer, sizeof(buffer));
		buffer[bytes_read] = '\0';
		response << buffer;
	}
	// std::cerr << "parent finished reading from pipe\n";
	close(in_pipe[1]);//writitng to script input finished
	close(out_pipe[0]);//reading response also finished
	waitpid(pid, NULL, 0);
	return (response.str());
}


/**
 * returns just the name of the script from the requested uri
 */
std::string get_scriptname(const std::string &uri) {
	std::string name = uri.substr(0, (uri.find(".py") + 3));
	name = name.substr(name.find_last_of("/") + 1);
	return name;
}

/**
 * HTTP headers are prefixed with HTTP_, converted to uppercase
 * and hyphens (-) are replaced with underscores (_).
 */
std::string to_http_variable(const std::string& header) {
	std::ostringstream	http_var;
	std::string			http_header = header;

	std::transform(http_header.begin(), http_header.end(), http_header.begin(), ::toupper);
	http_var << "HTTP_";
	for (auto it = http_header.begin(); it != http_header.end(); it++) {
		if (*it == '-')
			*it = '_';
	}
	http_var << http_header;
	return http_var.str();
}

/**
 * sets all needed env for cgi scripts and
 * also sets request headers to env in correct format.
 * then copies vector to char **_environment.
 */
void Response::set_env(void) {
	std::string uri = _request.getUri();
	size_t queryPos = uri.find("?");
	_env.clear();
	_environment.clear();

	_env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	_env.push_back("SERVER_PROTOCOL=HTTP/1.1");
	_env.push_back("REQUEST_METHOD=" + _request.getMethod());
	_env.push_back("SCRIPT_NAME=" + get_scriptname(uri));
	_env.push_back("CONTENT_LENGTH=" + std::to_string(_request.getBody().size()));
	if (queryPos != std::string::npos) {
		_env.push_back("PATH_INFO=" + uri.substr(0, queryPos));
		_env.push_back("QUERY_STRING=" + uri.substr(queryPos + 1));
	} else {
		_env.push_back("PATH_INFO=" + uri);
		_env.push_back("QUERY_STRING=");
	}

	auto headers = _request.getHeaders();
	for (const auto& header : headers) {
		_env.push_back(to_http_variable(header.first) + "=" + header.second);
	}

	// Convert _env to char**
	for (const std::string& e : _env) {
		char* envString = strdup(e.c_str());
		_environment.push_back(envString);
	}
	_environment.push_back(nullptr);
}
