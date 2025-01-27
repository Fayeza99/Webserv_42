#include "RequestParser.hpp"
#include "utils.hpp"
#include <sstream>
#include <cstdlib>
#include <set>
#include <algorithm>
#include <filesystem>

RequestParser::RequestParser(const std::string &request) : _request(request), _isUpload()
{
	// print_log(BLUE, "RequestParser Constructor");
	parseRequest(_request);
}

RequestParser::~RequestParser()
{
	// print_log(BLUE, "RequestParser Destructor");
}

bool RequestParser::isValidMethod(const std::string &methodStr) const
{
	static const std::set<std::string> valid_methods = {
		"GET", "POST", "PUT", "DELETE"};
	return valid_methods.find(methodStr) != valid_methods.end();
}

bool RequestParser::isDirectory(const std::string &path)
{
	char realPath[PATH_MAX];
	if (realpath(path.c_str(), realPath) == NULL)
		return false;
	return std::filesystem::is_directory(path);
}

void RequestParser::parseHeaders(std::istringstream &request_stream, std::unordered_map<std::string, std::string> &h)
{
	std::string line;
	while (std::getline(request_stream, line))
	{
		if (line == "\r" || line == "")
			break;
		line = trim(line);
		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		size_t colon = line.find(':');
		if (colon != std::string::npos)
		{
			std::string header_name = trim(line.substr(0, colon));
			std::string header_value = trim(line.substr(colon + 1));
			if (!header_name.empty() && !header_value.empty())
			{
				// std::transform(header_name.begin(), header_name.end(), header_name.begin(), ::tolower);
				h[header_name] = header_value;
			}
			else
			{
				throw std::runtime_error("Malformed header line1: " + line);
			}
		}
		else
		{
			throw std::runtime_error("Malformed header line2: " + line);
		}
	}
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
			throw std::runtime_error("Unknown HTTP method: " + method);
		if (httpVersion != "HTTP/1.1" && httpVersion != "HTTP/1.0")
			throw std::runtime_error("Unsupported HTTP version: " + httpVersion);
		if (uri.length() > 1 && uri.back() == '/')
			uri.pop_back();
	}
	else
	{
		throw std::runtime_error("Empty request");
	}

	parseHeaders(request_stream, headers);

	unsigned long content_length = 0;
	auto it = headers.find("Content-Length");
	if (it != headers.end())
	{
		try
		{
			content_length = std::stoi(it->second);
			if (content_length < 0)
				throw std::runtime_error("Invalid Content-Length value: " + it->second);
			while (std::getline(request_stream, line))
				body += line + '\n';
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error("Error parsing Content-Length: " + std::string(e.what()));
		}
	}
	else
	{
		if (request_stream.peek() != EOF)
		{
			body.assign(std::istreambuf_iterator<char>(request_stream), {});
			if (body.empty() && request_stream.fail())
			{
				throw std::runtime_error("Failed to read the request body");
			}
		}
	}
	if (headers["Content-Type"].find("multipart/form-data") != std::string::npos)
		_isUpload = true;
}

std::string const &RequestParser::getMethod() const { return method; }

std::string const &RequestParser::getRequest() const { return _request; }

std::string const &RequestParser::getHttpVersion() const { return httpVersion; }

std::string const &RequestParser::getUri() const { return uri; }

std::unordered_map<std::string, std::string> const &RequestParser::getHeaders() const { return headers; }

std::string const &RequestParser::getBody() const { return body; }

std::string RequestParser::trim(const std::string &str)
{
	const std::string whitespace = " \t\r\n";
	size_t start = str.find_first_not_of(whitespace);
	if (start == std::string::npos)
		return "";
	size_t end = str.find_last_not_of(whitespace);
	return str.substr(start, end - start + 1);
}
