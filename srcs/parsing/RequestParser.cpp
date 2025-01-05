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

void RequestParser::parse_headers(std::istringstream &request_stream, std::unordered_map<std::string, std::string> &h)
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
				throw std::runtime_error("Malformed header line: " + line);
			}
		}
		else
		{
			throw std::runtime_error("Malformed header line: " + line);
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

	parse_headers(request_stream, headers);

	auto it = headers.find("Content-Length");
	if (it != headers.end())
	{
		try
		{
			int content_length = std::stoi(it->second);
			if (content_length < 0)
			{
				throw std::runtime_error("Invalid Content-Length value: " + it->second);
			}
			body.resize(content_length);
			request_stream.read(&body[0], content_length);
			if (static_cast<int>(request_stream.gcount()) < content_length)
			{
				throw std::runtime_error("Incomplete request body: Expected " +
										 std::to_string(content_length) + " bytes, but got " +
										 std::to_string(request_stream.gcount()));
			}
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
	// if (headers["Content-Type"].find("multipart/form-data") != std::string::npos)
	// {
	// 	parseUpload();
	// 	_isUpload = true;
	// }
}

bool RequestParser::isUpload(void) { return _isUpload; }

void RequestParser::set_boundary(void)
{
	_boundary = headers["Content-Type"];
	_boundary = _boundary.substr(_boundary.find("boundary=") + 9);
	_boundary = trim(_boundary);
}

FileUpload::FileUpload(std::string body) : body_stream(body) {}

FileUpload::FileUpload(const FileUpload &other)
{
	body_stream = std::istringstream(other.body_stream.str());
	content = other.content;
	name = other.name;
	filename = other.filename;
}

void FileUpload::set_name(void)
{
	size_t pos = headers["Content-Disposition"].find("name=\"");
	if (pos == std::string::npos)
	{
		name = "";
		return;
	}
	name = headers["Content-Disposition"].substr(pos + 6);
	name = name.substr(0, name.find("\""));
}

void FileUpload::set_filename(void)
{
	size_t pos = headers["Content-Disposition"].find("filename=\"");
	if (pos == std::string::npos)
	{
		filename = "";
		return;
	}
	filename = headers["Content-Disposition"].substr(pos + 10);
	filename = filename.substr(0, filename.find("\""));
}

std::istringstream &FileUpload::get_stream(void) { return body_stream; }

void RequestParser::parseUpload(void)
{
	std::string b(body);
	std::vector<std::string> parts;
	set_boundary();
	while (!b.empty())
	{
		size_t next = b.find(_boundary);
		parts.push_back(b.substr(0, next));
		if (next != std::string::npos)
			b = b.substr(next + _boundary.length());
		else
			b = "";
	}
	for (std::string p : parts)
	{
		p = trim(p);
		if (p == "--")
			continue;
		FileUpload upload(p);
		parse_headers(upload.get_stream(), upload.headers);
		upload.content = upload.get_stream().str();
		upload.set_name();
		upload.set_filename();
		_upload.push_back(upload);
	}
}
// FORMAT: -- , boundary , headers , \r\n\r\n , content , -- , boundary , [...] , boundary , --

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

std::vector<FileUpload> const &RequestParser::getUpload() const { return _upload; }

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

bool RequestParser::isCgiRequest(void) {
	if (uri.find(".py") != std::string::npos) {
		return true;
	}
	return false;
}
