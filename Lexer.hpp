#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include <cctype>
#include <cstdlib>

enum class TokenType {
    STRING, NUMBER, OPEN_BRACE, CLOSE_BRACE, SEMICOLON, EOF_TOKEN, SLASH,
    SERVER, LISTEN, HOSTNAME, LOCATION, ALLOW // Add new tokens here
};

class Token {
public:
    TokenType type;
    std::string value;

    Token(TokenType type = TokenType::EOF_TOKEN, std::string value = "") : type(type), value(value) {}
};

class Lexer {
private:
    std::string input;
    size_t pos = 0;

    char nextChar() {
        return pos < input.size() ? input[pos++] : '\0';
    }

    void skipWhitespace() {
        while (isspace(input[pos]) && pos < input.size()) pos++;
    }

public:
    Lexer(const std::string& input) : input(input) {}
size_t getPosition() {
        return pos;  // Returns the current position in the input string
    }
    Token nextToken() {
    skipWhitespace();

    if (pos >= input.size()) return Token(TokenType::EOF_TOKEN);

    char c = nextChar();

    // Skip comments starting with '#'
    if (c == '#') {
        while (c != '\n' && pos < input.size()) {
            c = nextChar();  // Keep moving until end of line
        }
        return nextToken(); // Restart tokenization after skipping the comment
    }
	std::cout << c << std::endl;

    switch (c) {
        case '{': return Token(TokenType::OPEN_BRACE);
        case '}': return Token(TokenType::CLOSE_BRACE);
        case ';': return Token(TokenType::SEMICOLON);
		case '/': return Token(TokenType::SLASH);
        default:
            if (isalpha(c)) {
                std::string word(1, c);

                // Read the full word, allowing dots in hostnames
                while ((isalnum(input[pos]) || input[pos] == '.') && pos < input.size()) {
                    word += nextChar();
                }
                // Match against known keywords
				std::cout << "WORD: " << word << std::endl;
                if (word == "server") return Token(TokenType::SERVER);
                if (word == "listen") return Token(TokenType::LISTEN);
                if (word == "hostname") return Token(TokenType::HOSTNAME);
                if (word == "location") return Token(TokenType::LOCATION);
                // if (word == "allow") {
					
				// 	return Token(TokenType::STRING, "allow");
				// }
                // Default to a generic string token if no match
                Token token(TokenType::STRING, word);
                std::cout << "Lexer - Token generated: Type=" << static_cast<int>(token.type) << ", Value=" << token.value << std::endl;
                return token;
            }
            else if (isdigit(c)) {
                // Process numbers
                std::string number(1, c);
                while (isdigit(input[pos]) && pos < input.size()) {
                    number += nextChar();
                }
                Token token(TokenType::NUMBER, number);
                std::cout << "Lexer - Token generated: Type=" << static_cast<int>(token.type) << ", Value=" << token.value << std::endl;
                return token;
            }
            break;
    }

    // If no recognized token was found, throw an error or return an invalid token
    throw std::runtime_error("Unexpected character in input");
}

};
