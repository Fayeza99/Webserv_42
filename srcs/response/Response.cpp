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

//     env["GATEWAY_INTERFACE"] = "CGI/1.1";
//     env["SERVER_PROTOCOL"] = "HTTP/1.1";
//     env["REQUEST_METHOD"] = request_method;
//     env["QUERY_STRING"] = query_string;
//     env["SCRIPT_NAME"] = script_name;
//     env["SERVER_NAME"] = server_name;
//     env["SERVER_PORT"] = std::to_string(server_port);
//     env["REMOTE_ADDR"] = "127.0.0.1"; // Example; replace with actual client address.
//     env["REMOTE_PORT"] = "12345";     // Example; replace with actual client port.
//     env["PATH_INFO"] = path_info;
void	Response::set_env(void) {
	_environment["GATEWAY_INTERFACE"] = "CGI/1.1";
	_environment["SERVER_PROTOCOL"] = "HTTP/1.1";
	_environment["REQUEST_METHOD"] = _request.getMethod();
	_environment["QUERY_STRING"] = _request.getUri();
}

std::string	Response::get_response(void) {
	if (_request.getMethod() == "GET") {
		if (_request.getUri() == "/")
			return (_request.getHttpVersion() + " 200 OK\r\n\r\n");
		else if (!_request.getUri().compare(_request.getUri().length() - 3, 3, ".py"))
			return (exec_script());
	}
	return (_request.getHttpVersion() + " 404 Not Found\r\n\r\n");
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

// -------------------------------------------------------------------------------------------

Response::Response(RequestParser &req, const std::string& documentRoot) 
	: _request(req), _documentRoot(documentRoot) {}

Response::Response(Response &other) : _request(other.get_request()) {}

Response::~Response() {}

RequestParser&	Response::get_request(void) {return _request;}
