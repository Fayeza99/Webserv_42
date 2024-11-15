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

	std::cout << c << std::endl;

	switch (c)
	{
	case '{':
		return Token(TokenType::OPEN_BRACE);
	case '}':
		return Token(TokenType::CLOSE_BRACE);
	case ';':
		return Token(TokenType::SEMICOLON);
	case '/':
		return Token(TokenType::SLASH);
	default:
		if (isalpha(c))
		{
			std::string word(1, c);
			while ((isalnum(input[pos]) || input[pos] == '.') && pos < input.size())
			{
				word += nextChar();
			}
			if (word == "server")
				return Token(TokenType::SERVER);
			if (word == "listen")
				return Token(TokenType::LISTEN);
			if (word == "hostname")
				return Token(TokenType::HOSTNAME);
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
		break;
	}

	throw std::runtime_error("Unexpected character in input");
}
