#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <unordered_map>

class RequestParser {
	private:
		std::string method;
		std::string uri;
		std::string httpVersion;
		std::unordered_map<std::string, std::string> headers;
		std::string body;

		void parseRequest(const std::string& request);

	public:
		RequestParser(const std::string& request);
		std::string const& getMethod() const;
		std::string const& getUri() const;
		std::string const& getHttpVersion() const;
		std::string const& getBody() const;
		std::unordered_map<std::string, std::string> const& getHeaders() const;

};