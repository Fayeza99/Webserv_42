#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include <cctype>
#include <cstdlib>

enum class TokenType
{
	STRING,
	NUMBER,
	OPEN_BRACE,
	CLOSE_BRACE,
	SEMICOLON,
	EOF_TOKEN,
	SERVER,
	LISTEN,
	ERROR_PAGE,
	AUTOINDEX,
	URI,
	HOSTNAME,
	LOCATION,
	ALLOW
};

class Token
{
public:
	TokenType type;
	std::string value;

	Token(TokenType type = TokenType::EOF_TOKEN, std::string value = "");
};

class Lexer
{
private:
	std::string input;
	size_t pos = 0;

	char nextChar();
	void skipWhitespace();


public:
	Lexer(const std::string &input);
	size_t getPosition();
	Token nextToken();

};