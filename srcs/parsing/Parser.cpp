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
	case TokenType::ERROR_PAGE:
		return "ERROR_PAGE";
	case TokenType::AUTOINDEX:
		return "AUTOINDEX";
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

void Parser::parseErrorPage(ServerConfig &server)
{
	eat(TokenType::ERROR_PAGE);

	if (currentToken.type != TokenType::NUMBER)
	{
		std::cerr << "Parsing error at position " << lexer.getPosition()
				  << ": expected error code (NUMBER) after 'error_page', but found "
				  << tokenTypeToString(currentToken.type) << std::endl;
		throw std::runtime_error("Syntax error in 'error_page' directive.");
	}
	int error_code = std::stoi(currentToken.value);
	eat(TokenType::NUMBER);

	if (currentToken.type != TokenType::URI)
	{
		std::cerr << "Parsing error at position " << lexer.getPosition()
				  << ": expected URI (STRING) after error code in 'error_page', but found "
				  << tokenTypeToString(currentToken.type) << std::endl;
		throw std::runtime_error("Syntax error in 'error_page' directive.");
	}
	std::string uri = currentToken.value;
	eat(TokenType::URI);

	server.error_pages[error_code] = uri;

	eat(TokenType::SEMICOLON);
}

void Parser::parseAutoIndex(ServerConfig &server) {
	eat(TokenType::AUTOINDEX);
	if ((currentToken.type != TokenType::STRING) || (currentToken.value != "on" && currentToken.value != "off")) {
		std::cerr << "Parsing error at position " << lexer.getPosition()
				  << ": expected on/off (String) after 'autoindex', but found "
				  << tokenTypeToString(currentToken.type) << std::endl;
		throw std::runtime_error("Syntax error in 'autoindex' directive.");
	}

	if (currentToken.value == "on") {
		server.autoIndex = true;
	} else {
		server.autoIndex = false;
	}

	eat(TokenType::STRING);
	eat(TokenType::SEMICOLON);
}

LocationConfig Parser::parseLocation()
{
	LocationConfig location;
	location.autoIndex = false;
	eat(TokenType::LOCATION);

	if (currentToken.type == TokenType::URI) {
		location.uri = currentToken.value;
		eat(TokenType::URI);
	} else {
		throw std::runtime_error("Expected URI starting with '/' in location block.");
	}

	eat(TokenType::OPEN_BRACE);

	while (currentToken.type != TokenType::CLOSE_BRACE)
	{
		if (currentToken.type == TokenType::STRING || currentToken.type == TokenType::AUTOINDEX)
		{
			std::string directive = currentToken.value;
			if (currentToken.type == TokenType::STRING)
				eat(TokenType::STRING);
			std::string path;
			if (directive == "root")
			{
				if (currentToken.type == TokenType::STRING) {
					path += currentToken.value;
					eat(TokenType::STRING);
				}
				if (currentToken.type == TokenType::URI) {
					path += currentToken.value;
					eat(TokenType::URI);
				}
				location.document_root = path;
				eat(TokenType::SEMICOLON);
			}
			else if (directive == "return") {
				std::string redirectURI;
				if (currentToken.type == TokenType::STRING) {
					redirectURI += currentToken.value;
					eat(TokenType::STRING);
				}
				if (currentToken.type == TokenType::URI) {
					path += currentToken.value;
					eat(TokenType::URI);
				}
				location.redirect = true;
				location.redirect_uri = redirectURI;
				eat(TokenType::SEMICOLON);
			} else if (currentToken.type == TokenType::AUTOINDEX) {
				eat(TokenType::AUTOINDEX);
				if ((currentToken.type != TokenType::STRING) || (currentToken.value != "on" && currentToken.value != "off")) {
					std::cerr << "Parsing error at position " << lexer.getPosition()
							  << ": expected on/off (String) after 'autoindex', but found "
							  << tokenTypeToString(currentToken.type) << std::endl;
					throw std::runtime_error("Syntax error in 'location (autoindex)' directive.");
				}
				if (currentToken.value == "on") {
					location.autoIndex = true;
				} else {
					location.autoIndex = false;
				}
				eat(TokenType::STRING);
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
	server.autoIndex = false;
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
		else if (currentToken.type == TokenType::ERROR_PAGE)
		{
			parseErrorPage(server);
		}
		else if (currentToken.type == TokenType::AUTOINDEX) {
			parseAutoIndex(server);
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
