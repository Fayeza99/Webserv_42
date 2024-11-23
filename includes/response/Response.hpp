#pragma once

# include <string>
# include <iostream>
# include <unistd.h>
# include <sys/socket.h>
# include <vector>
# include "RequestParser.hpp"

// functions from Response.cpp:

void		send_response(std::string str, int fd);
void		handle_request(std::string req, int client_fd);

std::string	response(RequestParser &req);