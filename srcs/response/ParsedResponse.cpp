#include "Response.hpp"

// respond to a request with echo/...
void	echo_request(RequestParser &req, int fd) {
	std::string	response;
	std::string	echo;

	echo = req.getUri().substr(6);
	response = req.getHttpVersion() + " 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ";
	response += std::to_string(echo.length());
	response += "\r\n\r\n";
	response += echo;
	send_response(response, fd);
}

// respond to .py request (CGI)
void	exec_script(RequestParser &req, int fd) {
	pid_t				pid;
	const std::string	&uri = req.getUri();
	std::string			script_name;
	int					pipe_fd[2];

	if (access(uri.c_str(), X_OK) == -1) {
		send_response(req.getHttpVersion() + " 404 Not Found\r\n\r\n", fd);
		return ;
	}
	if (chdir(("." + uri.substr(0, uri.find_last_of("/"))).c_str()) == -1) {
		send_response(req.getHttpVersion() + " 404 Not Found\r\n\r\n", fd);
		return ;
	}
	script_name = uri.substr(uri.find_last_of("/"));
	if (pipe(pipe_fd) == -1)
		return ;
	pid = fork();
	
	if (pid == 0) {
		close(pipe_fd[1]);//write
		if (dup2(pipe_fd[0], STDIN_FILENO) == -1)
			exit(1);
		close(pipe_fd[0]);//read
		dup2(fd, STDOUT_FILENO);
		char *args[] = {(char *)"/Users/asemsey/.brew/bin/python3", (char *)script_name.c_str(), NULL};
		execve(args[0], args, NULL);
		std::cerr << "execve failed\n";
		exit(1);
	}
	else if (pid > 0) {
		close(pipe_fd[0]);//read
		if (!req.getBody().empty()) {
			if (write(pipe_fd[1], req.getBody().c_str(), req.getBody().size()) == -1)
				std::cerr << "write failed\n";
		}
		close(pipe_fd[1]);//write
		waitpid(pid, NULL, 0);
	}
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

void	respond(RequestParser &req, int client_fd) {
	const std::string	&uri = req.getUri();
	if (req.getMethod() == "GET") {
		if (uri == "/")
			send_response(req.getHttpVersion() + " 200 OK\r\n\r\n", client_fd);
		else if (uri.substr(0, 6) == "/echo/")
			echo_request(req, client_fd);
		else if (uri.compare (uri.length() - 3, 3, ".py"))
			exec_script(req, client_fd);
	} else
		send_response(req.getHttpVersion() + " 404 Not Found\r\n\r\n", client_fd);
}
