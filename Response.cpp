#include "Response.hpp"

void	send_response(std::string str, int fd) {
	send(fd, str.c_str(), str.length(), 0);
}

// to be deleted later
// respond to user-agent request (parsing the request body)
void	user_agent(std::string req, int fd)
{
	std::string	response;
	std::string	user;

	user = req.substr(req.find("User-Agent: ") + 12);
	user = user.substr(0, user.find("\r\n"));
	response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ";
	response += std::to_string(user.length());
	response += "\r\n\r\n";
	response += user;
	send_response(response, fd);
}

// respond to a request with echo/...
void	echo_request(std::string req, int fd) {
	std::string	response;
	std::string	echo;

	echo = req.substr(10);
	echo = echo.substr(0, echo.find(" "));
	response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ";
	response += std::to_string(echo.length());
	response += "\r\n\r\n";
	response += echo;
	send_response(response, fd);
}

// respond to .py request (CGI)
void	exec_script(std::string req, int fd) {
	std::string	script_name;
	pid_t		pid;

	script_name = req.substr(req.find("/cgi-bin/") + 1);
	script_name = script_name.substr(0, script_name.find(".py ") + 3);
	pid = fork();
	
	if (pid == 0) {
		dup2(fd, STDOUT_FILENO);
		char *args[] = {(char *)"/Users/asemsey/.brew/bin/python3", (char *)script_name.c_str(), NULL};
		execve(args[0], args, NULL);
		std::cout << "cgi-script error\n";
		exit(1);
	}
	else if (pid > 0)
		waitpid(pid, NULL, 0);
}

void	handle_request(std::string req, int client_fd) {
	std::string	response;
	if (req.find("GET / ") != std::string::npos)
		send_response("HTTP/1.1 200 OK\r\n\r\n", client_fd);
	else if (req.find("GET /echo/") != std::string::npos)
		echo_request(req, client_fd);
	else if (req.find("GET /user-agent") != std::string::npos && req.find("User-Agent: ") != std::string::npos)
		user_agent(req, client_fd);
	else if (req.find(".py ") != std::string::npos)
		exec_script(req, client_fd);
	else
		response = "HTTP/1.1 404 Not Found\r\n\r\n";
}
