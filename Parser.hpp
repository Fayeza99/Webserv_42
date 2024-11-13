#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "Lexer.hpp"
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

class Parser
{
private:
	Lexer lexer;
	Token currentToken;

	void eat(TokenType type)
	{
		if (currentToken.type != type)
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
	void parseHostname(ServerConfig &server)
	{
		eat(TokenType::HOSTNAME);
		while (currentToken.type == TokenType::STRING)
		{
			server.hostnames.push_back(currentToken.value);
			eat(TokenType::STRING);
		}
		eat(TokenType::SEMICOLON);
	}

	void parseListen(ServerConfig &server)
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

	void parseErrorPages(ServerConfig &server)
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

	LocationConfig parseLocation()
	{
		LocationConfig location;
		eat(TokenType::LOCATION); // Consume the 'location' token

		// Check if there's a slash indicating a specific path
		if (currentToken.type == TokenType::SLASH)
		{
			eat(TokenType::SLASH); // Consume the slash '/'

			// Check if a string follows the slash, which would be the specific path
			if (currentToken.type == TokenType::STRING)
			{
				location.uri = currentToken.value;
				eat(TokenType::STRING); // Consume the path string
			}
		}

		// Now expect the open brace to start the location block
		eat(TokenType::OPEN_BRACE);

		// Parsing internal directives within the location block
		while (currentToken.type != TokenType::CLOSE_BRACE)
		{
						std::cout << "Current Token: " << currentToken.value << std::endl;

			if (currentToken.type == TokenType::STRING)
			{
				std::string directive = currentToken.value;
				eat(TokenType::STRING); // Consume the directive name
				if (directive == "root")
				{
					std::string path = "";
					// Ensure the first component (might be a slash or part of the path)
					if (currentToken.type == TokenType::SLASH)
					{
						path += "/";		   // Start path with root slash
						eat(TokenType::SLASH); // Consume the slash
					}
					// Continue building the path from string tokens
					while (currentToken.type == TokenType::STRING || currentToken.type == TokenType::SLASH)
					{
						if (currentToken.type == TokenType::SLASH)
						{
							path += "/";		   // Append slash to path
							eat(TokenType::SLASH); // Consume the slash
						}
						else
						{
							path += currentToken.value; // Append the next part of the path
							eat(TokenType::STRING);		// Consume the string
						}
					}
					location.document_root = path; // Set the full path
					eat(TokenType::SEMICOLON);
				}
				else if (directive == "index")
				{
					if (currentToken.type == TokenType::STRING)
					{
						location.default_files.push_back(currentToken.value);
						eat(TokenType::STRING);	   // Consume the index file name
						eat(TokenType::SEMICOLON); // Expect and consume the semicolon
					}
				}
				else if (directive == "allow")
				{
					while (currentToken.type == TokenType::STRING)
					{
						location.supported_methods.insert(currentToken.value);
						eat(TokenType::STRING); // Consume each allowed method
						if (currentToken.type == TokenType::SEMICOLON)
							break; // Check for semicolon to end the directive
					}
					eat(TokenType::SEMICOLON); // Expect and consume the semicolon
				}
				// else if (directive == "cgi") {
				// 	std::string ext;
				// 	std::string handler_path;
				// 	if (currentToken.type == TokenType::STRING) {
				// 		ext = currentToken.value;
				// 		eat(TokenType::STRING); // Consume the CGI extension
				// 	}
				// 	if (currentToken.type == TokenType::STRING) {
				// 		handler_path = currentToken.value;
				// 		eat(TokenType::STRING); // Consume the handler path
				// 	}
				// 	location.cgi_paths[ext] = handler_path;
				// 	eat(TokenType::SEMICOLON); // Expect and consume the semicolon
				// }
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

		eat(TokenType::CLOSE_BRACE); // Consume the closing brace of the location block
		return location;
	}

public:
	Parser(const std::string &input) : lexer(Lexer(input))
	{
		currentToken = lexer.nextToken(); // Start the parsing with the first token
	}

	GlobalConfig parse()
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

	ServerConfig parseServer()
	{
		ServerConfig server;
		eat(TokenType::SERVER);		// Consume the 'server' token
		eat(TokenType::OPEN_BRACE); // Consume the '{' token

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

		eat(TokenType::CLOSE_BRACE); // Consume the '}' token
		return server;
	}
};
