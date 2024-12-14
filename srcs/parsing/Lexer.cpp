#include "Lexer.hpp"

Token::Token(TokenType type, std::string value) : type(type), value(value) {}

Lexer::Lexer(const std::string &input) : input(input) {}

size_t Lexer::getPosition()
{
	return pos;
}

char Lexer::nextChar()
{
	return pos < input.size() ? input[pos++] : '\0';
}

void Lexer::skipWhitespace()
{
	while (pos < input.size() && isspace(input[pos]))
	{
		pos++;
	}
}

Token Lexer::nextToken()
{
	skipWhitespace();

	if (pos >= input.size())
		return Token(TokenType::EOF_TOKEN);

	char c = nextChar();

	if (c == '#')
	{
		while (c != '\n' && pos < input.size())
		{
			c = nextChar();
		}
		return nextToken();
	}


	switch (c)
	{
	case '{':
		return Token(TokenType::OPEN_BRACE);
	case '}':
		return Token(TokenType::CLOSE_BRACE);
	case ';':
		return Token(TokenType::SEMICOLON);
	case '/':
		{
			std::string uri;
			uri += c;
			while (pos < input.size())
			{
				char current = input[pos];
				if (isalnum(current) || current == '/' || current == '.' || current == '_' || current == '-' || current == '?'
					|| current == '=' || current == '&' || current == '%')
				{
					uri += nextChar();
				}
				else
				{
					break;
				}
			}
			return Token(TokenType::URI, uri);
		}
	default:
		if (isalpha(c))
		{
			std::string word(1, c);
			while ((isalnum(input[pos]) || input[pos] == '.' || input[pos] == '_' ) && pos < input.size())
			{
				word += nextChar();
			}
			if (word == "server")
				return Token(TokenType::SERVER);
			if (word == "listen")
				return Token(TokenType::LISTEN);
			if (word == "hostname")
				return Token(TokenType::HOSTNAME);
			if (word == "error_page")
				return Token(TokenType::ERROR_PAGE);
			if (word == "autoindex"){
				return Token(TokenType::AUTOINDEX);
			}
			if (word == "location")
				return Token(TokenType::LOCATION);
			return Token(TokenType::STRING, word);
		}
		else if (isdigit(c))
		{
			std::string number(1, c);
			while (isdigit(input[pos]) && pos < input.size())
			{
				number += nextChar();
			}
			return Token(TokenType::NUMBER, number);
		}
		else {
			nextChar();
			throw std::runtime_error(std::string("Unexpected character in input: ") + c);
		}
	}
}
