#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <unordered_map>
#include <vector>

class RequestParser
{
private:
	std::string _request;
	std::string method;
	std::string uri;
	std::string httpVersion;
	std::unordered_map<std::string, std::string> headers;
	std::string body;

	void parseRequest(const std::string &request);

public:
	bool _isUpload;
	bool isValidMethod(const std::string &methodStr) const;
	static std::string trim(const std::string &str);
	static bool isDirectory(const std::string &path);
	static void parseHeaders(std::istringstream &request_stream, std::unordered_map<std::string, std::string> &headers);

	RequestParser(const std::string &request);
	~RequestParser();
	std::string const &getRequest() const;
	std::string const &getMethod() const;
	std::string const &getUri() const;
	std::string const &getHttpVersion() const;
	std::string const &getBody() const;
	std::unordered_map<std::string, std::string> const &getHeaders() const;
};
