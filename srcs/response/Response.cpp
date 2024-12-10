#include "Response.hpp"

// this is the function that controls how the response is be created
std::string	Response::get_response(void) {
	// std::cerr << "response started\n";
	if (_request.getMethod() == "GET" || _request.getMethod() == "POST") {
		if (_request.getUri().find(".py") != std::string::npos)
			return (exec_script());
		return serve_static_file();
	}
	return serve_static_file();
}

// respond to .py request (CGI)
std::string	Response::exec_script() {
	std::string filePath = _documentRoot + _request.getUri();
	std::string	script_name = filePath.substr(filePath.find_last_of("/") + 1);
	int					in_pipe[2];// redirect request body to stdin of script
	int					out_pipe[2];// redirect script output to stringstream

	// std::cerr << "exec_script called on: " << filePath << std::endl;
	// check that script exists and set environment
	if (FILE *file = fopen(filePath.c_str(), "r"))
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
		if (chdir((filePath.substr(0, filePath.find_last_of("/"))).c_str()) == -1)
			exit(1);
		close(in_pipe[1]);//child doesnt write request body
		close(out_pipe[0]);//child doesnt read its own output (duh)
		if (dup2(in_pipe[0], STDIN_FILENO) == -1 || dup2(out_pipe[1], STDOUT_FILENO) == -1)
			exit(1);
		close(in_pipe[0]);//dup successful, fds no longer needed
		close(out_pipe[1]);//now acessible through STDIN and STDOUT
		char *args[] = {(char *)"/usr/bin/python3", (char *)script_name.c_str(), NULL};
		// std::cerr << "executing cgi script...\n";
		execve(args[0], args, _environment.data());
	}
	else if (pid > 0) {
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
	return (get_error_response(500));
}

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

/**
 * @brief Helper method to determine the MIME type based on the file extension
 *
 * Provides default MIME type is none matched
 *
 * @param path of the file to send
 * @return std::string MIME
 */
std::string Response::get_content_type(const std::string& path) const {
	static std::map<std::string, std::string> mime_types = {
		{".html", "text/html"},
		{".htm", "text/html"},
		{".css", "text/css"},
		{".js", "application/javascript"},
		{".png", "image/png"},
		{".jpg", "image/jpeg"},
		{".jpeg", "image/jpeg"},
		{".gif", "image/gif"},
		{".txt", "text/plain"},
		{".pdf", "application/pdf"},
	};

	size_t dot = path.find_last_of('.');
	if (dot != std::string::npos) {
		std::string fileExtension = path.substr(dot);
		if (mime_types.find(fileExtension) != mime_types.end()) {
			return mime_types.find(fileExtension)->second;
		}
	}
	return "application/octet-stream";
}

std::string Response::get_error_response(const int errorCode) {
	std::string errorMessage;
	std::string errorFilePath;

	switch (errorCode)
	{
	case 403:
		errorMessage = " 403 Forbidden\r\n";
		errorFilePath = "www/error/403.html";
		break;
	case 404:
		errorMessage = " 404 Not Found\r\n";
		errorFilePath = "www/error/404.html";
		break;
	case 500:
		errorMessage = " 500 Internal Server Error\r\n";
		errorFilePath = "www/error/500.html";
		break;
	default:
		errorMessage = " 500 Internal Server Error\r\n";
		errorFilePath = "www/error/500.html";
		break;
	}

	char resolvedPath[PATH_MAX];
	realpath(errorFilePath.c_str(), resolvedPath);
	std::ostringstream response;
	std::ifstream file(resolvedPath);
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string body = buffer.str();
	file.close();
	response	<< _request.getHttpVersion() << errorMessage
				<< "Content-Length: " << body.length() << "\r\n"
				<< "Connection: close\r\n\r\n"
				<< body;

	return response.str();
}

std::string Response::serve_static_file() {
	std::string uri = _request.getUri();
	std::string filePath = _documentRoot + uri;

	bool isSupported = std::find(
		_location.supported_methods.begin(),
		_location.supported_methods.end(),
		_request.getMethod()
	) != _location.supported_methods.end();

	if (!isSupported) {
		return get_error_response(403);
	}

	if (!uri.empty() && uri.back() == '/') {
		std::vector<std::string> defaultFiles = _location.default_files;
		if (!defaultFiles.empty()) {
			filePath += defaultFiles[0];
		} else {
			return (get_error_response(403));
		}
	}

	char resolvedPath[PATH_MAX];
	if (realpath(filePath.c_str(), resolvedPath) == NULL) {
		return get_error_response(404);
	}

	std::string resolvedFilePath(resolvedPath);

	char resolvedDocRoot[PATH_MAX];
	if (realpath(_documentRoot.c_str(), resolvedDocRoot) == NULL) {
		std::cerr << "Failed to resolve document root: " << _documentRoot << std::endl;
		return get_error_response(500);
	}

	std::string resolvedDocRootStr(resolvedDocRoot);
	if (resolvedFilePath.find(resolvedDocRootStr) != 0) {
		return get_error_response(403);
	}


	std::ifstream file(resolvedFilePath.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		return get_error_response(404);
	}

	std::ostringstream bodyStream;
	bodyStream << file.rdbuf();
	std::string body = bodyStream.str();


	std::string contentType = get_content_type(resolvedFilePath);

	std::ostringstream response;
	response	<< _request.getHttpVersion() << " 200 OK\r\n"
				<< "Content-Type: " << contentType << "\r\n"
				<< "Content-Length: " << body.size() << "\r\n"
				<< "Connection: close\r\n\r\n"
				<< body;

	return response.str();
}

// -------------------------------------------------------------------------------------------

Response::Response(RequestParser &req, ClientState& clientState)
	: _request(req), _clientState(clientState), _statuscode(200) {
	_location = getLocation(_clientState.serverConfig, _request.getUri());
	_documentRoot = _location.document_root;
}

// Response::Response(Response &other) : _request(other.get_request()) {}

Response::~Response() {
	_env.clear();
	for (char* e : _environment) {
		free(e);
	}
	_environment.clear();
}

RequestParser&	Response::get_request(void) {return _request;}

int Response::get_status(void) {return _statuscode;}
