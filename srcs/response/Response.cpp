#include "Response.hpp"

// respond to .py request (CGI)
std::string	Response::exec_script() {
	const std::string	&uri = "." + _request.getUri();
	int					in_pipe[2];// redirect request body to stdin of script
	int					out_pipe[2];// redirect script output to stringstream

	// check that script exists and chdir
	if (FILE *file = fopen(uri.c_str(), "r"))
		fclose(file);
	else
		return (_request.getHttpVersion() + " 404 Not Found\r\n\r\n");
	if (chdir((uri.substr(0, uri.find_last_of("/"))).c_str()) == -1)
		return (_request.getHttpVersion() + " 404 Not Found\r\n\r\n");
	std::string	script_name = uri.substr(uri.find_last_of("/") + 1);

	// pipes
	if (pipe(in_pipe) == -1 || pipe(out_pipe) == -1)
		return (_request.getHttpVersion() + " 404 Not Found\r\n\r\n");

	pid_t	pid = fork();
	if (pid == 0) {
		close(in_pipe[1]);//write
		close(out_pipe[0]);//read
		if (dup2(in_pipe[0], STDIN_FILENO) == -1 || dup2(out_pipe[1], STDOUT_FILENO) == -1)
			exit(1);
		close(in_pipe[0]);//read
		close(out_pipe[1]);//write
		char *args[] = {(char *)"/Users/asemsey/.brew/bin/python3", (char *)script_name.c_str(), NULL};
		execve(args[0], args, NULL);
	}
	else if (pid > 0) {
		std::ostringstream	response;
		char				buffer[4096];
		size_t				bytes_read = 1;

		close(in_pipe[0]);//read
		close(out_pipe[1]);//write
		if (!_request.getBody().empty()) {
			if (write(in_pipe[1], _request.getBody().c_str(), _request.getBody().size()) == -1)
				std::cerr << "write failed\n";
		}
		while (bytes_read > 0) {
			bytes_read = read(out_pipe[0], buffer, sizeof(buffer));
			buffer[bytes_read] = '\0';
			response << buffer;
		}
		close(in_pipe[1]);//write
		close(out_pipe[0]);//read
		waitpid(pid, NULL, 0);
		return (response.str());
	}
	return (_request.getHttpVersion() + " 404 Not Found\r\n\r\n");
}

std::string	Response::get_response(void) {
	if (_request.getMethod() == "GET") {
		if (_request.getUri().find(".py"))
			return (exec_script());
		return serve_static_file();
	}
	return serve_static_file();
}

std::string get_scriptname(const std::string &uri) {
	std::string name = uri;
	while (true) {
		if (name.find(".py") != std::string::npos && name.find("/") != std::string::npos
			&& name.find(".py") < name.find("/"))
			return name.substr(0, name.find(".py") + 2);
		if (name.find(".py") != std::string::npos && name.find("/") == std::string::npos)
			return name;
		name = name.substr(name.find("/"));
	}
	return name;
}

void Response::set_env(void) {
	_env["PATH_INFO"] = 
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env["REQUEST_METHOD"] = _request.getMethod();
	if (_request.getUri().find("?") != std::string::npos)
		_env["QUERY_STRING"] = _request.getUri().substr(_request.getUri().find("?") + 1);
	if (_request.getUri().find(".py") != std::string::npos)
		_env["SCRIPT_NAME"] = get_scriptname(_request.getUri());
	// _env["SERVER_NAME"] = server_name;
	// _env["SERVER_PORT"] = std::to_string(server_port);
	// _env["REMOTE_ADDR"] = _request.getHeaders()[""];
	// _env["REMOTE_PORT"] = "12345";
	// _environment = (char **)malloc(sizeof(char *) * (x));
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
		errorMessage = " 403 Frobidden\r\n";
		errorFilePath = "documents/error_pages/403.html";
		break;
	case 404:
		errorMessage = " 404 Not Found\r\n";
		errorFilePath = "documents/error_pages/404.html";
		break;
	case 500:
		errorMessage = " 500 Internal Server Error\r\n";
		errorFilePath = "documents/error_pages/500.html";
		break;
	default:
		errorMessage = " 500 Internal Server Error\r\n";
		errorFilePath = "documents/error_pages/500.html";
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

	// If URI ends with '/', append the first default file
	if (!uri.empty() && uri.back() == '/') {
		std::vector<std::string> defaultFiles; // create method to get approriate defaultFiles fromt the strucutre
		defaultFiles.push_back("index.html");
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

Response::Response(RequestParser &req, const std::string& documentRoot) 
	: _request(req), _statuscode(200), _documentRoot(documentRoot) {}

Response::Response(Response &other) : _request(other.get_request()) {}

Response::~Response() {}

RequestParser&	Response::get_request(void) {return _request;}

int Response::get_status(void) {return _statuscode;}
