#include "RequestParser.hpp"
#include <sstream>
#include <cstdlib>
#include <set>
#include <algorithm>

RequestParser::RequestParser(const std::string &request) : _request(request)
{
	parseRequest(request);
}

bool RequestParser::isValidMethod(const std::string &methodStr)
{
	static const std::set<std::string> valid_methods = {
		"GET", "POST", "PUT", "DELETE"};
	return valid_methods.find(methodStr) != valid_methods.end();
}

void RequestParser::parseRequest(const std::string &request)
{
	std::istringstream request_stream(request);
	std::string line;

	if (std::getline(request_stream, line))
	{
		line = trim(line);
		if (!line.empty() && line.back() == '\r')
		{
			line.pop_back();
		}
		std::istringstream line_stream(line);
		if (!(line_stream >> method >> uri >> httpVersion))
		{
			throw std::runtime_error("Malformed request line");
		}
		if (!isValidMethod(method))
		{
			throw std::runtime_error("Unknown HTTP method: " + method);
		}
		if (httpVersion != "HTTP/1.1" && httpVersion != "HTTP/1.0")
		{
			throw std::runtime_error("Unsupported HTTP version: " + httpVersion);
		}
	}
	else
	{
		throw std::runtime_error("Empty request");
	}

	while (std::getline(request_stream, line))
	{
		if (line == "\r" || line == "")
		{
			break;
		}

		line = trim(line);
		if (!line.empty() && line.back() == '\r')
		{
			line.pop_back();
		}

		size_t colon = line.find(':');
		if (colon != std::string::npos)
		{
			std::string header_name = trim(line.substr(0, colon));
			std::string header_value = trim(line.substr(colon + 1));
			if (!header_name.empty() && !header_value.empty())
			{
				std::transform(header_name.begin(), header_name.end(), header_name.begin(), ::tolower);
				headers[header_name] = header_value;
			}
			else
			{
				throw std::runtime_error("Malformed header line: " + line);
			}
		}
		else
		{
			throw std::runtime_error("Malformed header line: " + line);
		}
	}
	auto it = headers.find("content-length");
	if (it != headers.end())
	{
		int content_length = std::stoi(it->second);
		if (content_length < 0)
		{
			throw std::runtime_error("Invalid Content-Length value");
		}
		body.resize(content_length);
		request_stream.read(&body[0], content_length);
		if (request_stream.gcount() < content_length)
		{
			throw std::runtime_error("Incomplete request body");
		}
	}
	else
	{
		if (request_stream.peek() != EOF)
		{
			body = std::string(std::istreambuf_iterator<char>(request_stream), {});
		}
	}
}

std::string const &RequestParser::getMethod() const
{
	return method;
}

std::string const &RequestParser::getRequest() const
{
	return _request;
}

std::string const &RequestParser::getHttpVersion() const
{
	return httpVersion;
}

std::string const &RequestParser::getUri() const
{
	return uri;
}

std::unordered_map<std::string, std::string> const &RequestParser::getHeaders() const
{
	return headers;
}

std::string const &RequestParser::getBody() const
{
	return body;
}

std::string RequestParser::trim(const std::string &str)
{
	const std::string whitespace = " \t\r\n";
	size_t start = str.find_first_not_of(whitespace);
	if (start == std::string::npos)
	{
		return "";
	}
	size_t end = str.find_last_not_of(whitespace);
	return str.substr(start, end - start + 1);
}
