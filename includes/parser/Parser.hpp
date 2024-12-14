#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <map>
#include <set>
#include "Lexer.hpp"
#include "GlobalConfig.hpp"
std::string tokenTypeToString(TokenType type);

class Parser
{
private:
	Lexer lexer;
	Token currentToken;

	void eat(TokenType type);
	void parseHostname(ServerConfig &server);
	void parseListen(ServerConfig &server);
	void parseErrorPage(ServerConfig &server);
	LocationConfig parseLocation();

public:
	Parser(const std::string &input);
	GlobalConfig parse();
	ServerConfig parseServer();
	std::vector<int> initializeSockets(const GlobalConfig &config);

};