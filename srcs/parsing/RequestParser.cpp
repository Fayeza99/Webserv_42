#include "RequestParser.hpp"

RequestParser::RequestParser(const std::string& request) {
	parseRequest(request);
}

void RequestParser::parseRequest(const std::string& request) {
	std::istringstream request_stream(request);
	std::string line;

	// Parse the request line
	if (std::getline(request_stream, line)) {
		std::istringstream line_stream(line);
		line_stream >> method >> uri >> httpVersion;
	}

	// Parse the headers
	while (std::getline(request_stream, line) && line != "\r") {
		if (line.empty() || line == "\r")
			break;
		size_t colon = line.find(':');
		if (colon != std::string::npos) {
			std::string header_name = line.substr(0, colon);
			std::string header_value = line.substr(colon + 1);
			header_value.erase(0, header_value.find_first_not_of(" \t"));
			// Remove possible leading whitespaces
			// Remove possible trailing carriage return
			if (!header_value.empty() && header_value.back() == '\r') {
				header_value.pop_back();
			}
			headers[header_name] = header_value;
		}
	}

	// Parse the body (if any)
	if (request_stream.peek() != EOF) {
		body = std::string(std::istreambuf_iterator<char>(request_stream), {});
	}
}

std::string const& RequestParser::getMethod() const {
	return (method);
}

std::string const& RequestParser::getHttpVersion() const {
	return (httpVersion);
}

std::string const& RequestParser::getUri() const {
	return (uri);
}

std::unordered_map<std::string, std::string> const& RequestParser::getHeaders() const{
	return (headers);
}

std::string const& RequestParser::getBody() const {
	return (body);
}