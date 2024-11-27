#include "Parser.hpp"
#include <sys/socket.h> // For socket, bind, listen
#include <arpa/inet.h>  // For htons

std::string tokenTypeToString(TokenType type)
{
	switch (type)
	{
	case TokenType::STRING:
		return "STRING";
	case TokenType::NUMBER:
		return "NUMBER";
	case TokenType::OPEN_BRACE:
		return "OPEN_BRACE";
	case TokenType::CLOSE_BRACE:
		return "CLOSE_BRACE";
	case TokenType::SEMICOLON:
		return "SEMICOLON";
	case TokenType::EOF_TOKEN:
		return "EOF_TOKEN";
	case TokenType::SLASH:
		return "SLASH";
	case TokenType::SERVER:
		return "SERVER";
	case TokenType::LISTEN:
		return "LISTEN";
	case TokenType::HOSTNAME:
		return "HOSTNAME";
	case TokenType::LOCATION:
		return "LOCATION";
	case TokenType::ALLOW:
		return "ALLOW";
	default:
		return "UNKNOWN";
	}
}

Parser::Parser(const std::string &input) : lexer(Lexer(input))
{
	currentToken = lexer.nextToken();
}

void Parser::eat(TokenType type)
{
	if (currentToken.type != type && type != TokenType::EOF_TOKEN)
	{
		std::cerr << "Parsing error at position " << lexer.getPosition() << ": expected token of type "
				  << static_cast<int>(type) << " (" << tokenTypeToString(type) << ")"
				  << ", but found token of type " << static_cast<int>(currentToken.type)
				  << " (" << tokenTypeToString(currentToken.type) << ") with value '"
				  << currentToken.value << "'" << std::endl;
		throw std::runtime_error("Syntax error in configuration file.");
	}
	currentToken = lexer.nextToken();
}

void Parser::parseHostname(ServerConfig &server)
{
	eat(TokenType::HOSTNAME);
	while (currentToken.type == TokenType::STRING)
	{
		server.hostnames.push_back(currentToken.value);
		eat(TokenType::STRING);
	}
	eat(TokenType::SEMICOLON);
}

void Parser::parseListen(ServerConfig &server)
{
	eat(TokenType::LISTEN);
	if (currentToken.type == TokenType::NUMBER)
	{
		server.listen_port = std::stoi(currentToken.value);
		eat(TokenType::NUMBER);
	}
	else
	{
		throw std::runtime_error("Expected port number after 'listen'");
	}
	eat(TokenType::SEMICOLON);
}

void Parser::parseErrorPages(ServerConfig &server)
{
	eat(TokenType::STRING); // Assuming "error_pages" as token
	while (currentToken.type == TokenType::NUMBER)
	{
		int code = std::stoi(currentToken.value);
		eat(TokenType::NUMBER);
		if (currentToken.type == TokenType::STRING)
		{
			server.error_pages[code] = currentToken.value;
			eat(TokenType::STRING);
		}
	}
	eat(TokenType::SEMICOLON);
}

LocationConfig Parser::parseLocation()
{
	LocationConfig location;
	eat(TokenType::LOCATION);

	if (currentToken.type == TokenType::SLASH)
	{
		eat(TokenType::SLASH);

		location.uri = "/";
		if (currentToken.type == TokenType::STRING)
		{
			location.uri += currentToken.value;
			eat(TokenType::STRING);
		}
	}

	eat(TokenType::OPEN_BRACE);

	while (currentToken.type != TokenType::CLOSE_BRACE)
	{

		if (currentToken.type == TokenType::STRING)
		{
			std::string directive = currentToken.value;
			eat(TokenType::STRING);
			if (directive == "root")
			{
				std::string path;
				if (currentToken.type == TokenType::SLASH)
				{
					path += "/";
					eat(TokenType::SLASH);
				}
				while (currentToken.type == TokenType::STRING || currentToken.type == TokenType::SLASH)
				{
					if (currentToken.type == TokenType::SLASH)
					{
						path += "/";
						eat(TokenType::SLASH);
					}
					else
					{
						path += currentToken.value;
						eat(TokenType::STRING);
					}
				}
				location.document_root = path;
				eat(TokenType::SEMICOLON);
			}
			else if (directive == "index")
			{
				if (currentToken.type == TokenType::STRING)
				{
					location.default_files.push_back(currentToken.value);
					eat(TokenType::STRING);
					eat(TokenType::SEMICOLON);
				}
			}
			else if (directive == "allow")
			{
				while (currentToken.type == TokenType::STRING)
				{
					location.supported_methods.insert(currentToken.value);
					eat(TokenType::STRING);
					if (currentToken.type == TokenType::SEMICOLON)
						break;
				}
				eat(TokenType::SEMICOLON);
			}
			else
			{
				throw std::runtime_error("Unexpected directive: " + directive);
			}
		}
		else
		{
			throw std::runtime_error("Expected a directive inside location block, found something else.");
		}
	}

	eat(TokenType::CLOSE_BRACE);
	return location;
}

ServerConfig Parser::parseServer()
{
	ServerConfig server;
	eat(TokenType::SERVER);
	eat(TokenType::OPEN_BRACE);

	while (currentToken.type != TokenType::CLOSE_BRACE)
	{
		if (currentToken.type == TokenType::LISTEN)
		{
			parseListen(server);
		}
		else if (currentToken.type == TokenType::HOSTNAME)
		{
			parseHostname(server);
		}
		else if (currentToken.type == TokenType::LOCATION)
		{
			server.locations.push_back(parseLocation());
		}
		else if (currentToken.type == TokenType::STRING && currentToken.value == "error_pages")
		{
			parseErrorPages(server);
		}
		else
		{
			std::cerr << "Error: Unexpected token within server block: " << static_cast<int>(currentToken.type) << std::endl;
			throw std::runtime_error("Parsing error: Unexpected token within server block.");
		}
	}

	eat(TokenType::CLOSE_BRACE);
	return server;
}

GlobalConfig Parser::parse()
{
	GlobalConfig config;
	while (currentToken.type != TokenType::EOF_TOKEN)
	{
		if (currentToken.type == TokenType::SERVER)
		{
			ServerConfig server = parseServer();
			config.servers.push_back(server);
		}
		else
		{
			std::cerr << "Error: Unexpected token " << static_cast<int>(currentToken.type) << std::endl;
			throw std::runtime_error("Parsing error: Unexpected token.");
		}
	}
	return config;
}
