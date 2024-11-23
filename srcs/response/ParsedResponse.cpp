#include "Response.hpp"

// respond to a request with echo/...
std::string	echo_request(RequestParser &req) {
	std::ostringstream	response;
	std::string	echo;

	echo = req.getUri().substr(6);
	response << req.getHttpVersion() << " 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
			<< std::to_string(echo.length()) << "\r\n\r\n" << echo;
	return (response.str());
}

// respond to .py request (CGI)
std::string	exec_script(RequestParser &req) {
	const std::string	&uri = "." + req.getUri();
	int					in_pipe[2];// redirect request body to stdin of script
	int					out_pipe[2];// redirect script output to stringstream

	
	// check that script exists and chdir
	if (FILE *file = fopen(uri.c_str(), "r"))
		fclose(file);
	else
		return (req.getHttpVersion() + " 404 Not Found\r\n\r\n");
	if (chdir((uri.substr(0, uri.find_last_of("/"))).c_str()) == -1)
		return (req.getHttpVersion() + " 404 Not Found\r\n\r\n");
	std::string	script_name = uri.substr(uri.find_last_of("/") + 1);

	// pipes
	if (pipe(in_pipe) == -1 || pipe(out_pipe) == -1)
		return (req.getHttpVersion() + " 404 Not Found\r\n\r\n");

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
		if (!req.getBody().empty()) {
			if (write(in_pipe[1], req.getBody().c_str(), req.getBody().size()) == -1)
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
	return (req.getHttpVersion() + " 404 Not Found\r\n\r\n");
}

// void	handle_post(std::string req, int fd) {
// 	std::string	body = req.substr(req.find("\r\n\r\n") + 4);
// 	std::string	response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ";
// 	response += std::to_string(body.length());
// 	response += "\r\n\r\n";
// 	response += body;
// 	send_response(response, fd);
// 	// std::cout << body << std::endl;
// }

std::string	response(RequestParser &req) {
	const std::string	&uri = req.getUri();
	if (req.getMethod() == "GET") {
		if (uri == "/")
			return (req.getHttpVersion() + " 200 OK\r\n\r\n");
		else if (uri.substr(0, 6) == "/echo/")
			return (echo_request(req));
		else if (!uri.compare(uri.length() - 3, 3, ".py"))
			return (exec_script(req));
	}
	return (req.getHttpVersion() + " 404 Not Found\r\n\r\n");
}
