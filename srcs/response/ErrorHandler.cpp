#include "ErrorHandler.hpp"

const std::map<int, std::string> ErrorHandler::defaultErrorMessages = {
	{400, "Bad Request"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},
	{413, "Payload Too Large"},
	{500, "Internal Server Error"},
	{501, "Not Implemented"}
};

const std::map<int, std::string> ErrorHandler::defaultErrorFilePaths = {
	{400, "www/error/400.html"},
	{403, "www/error/403.html"},
	{404, "www/error/404.html"},
	{405, "www/error/405.html"},
	{413, "www/error/413.html"},
	{500, "www/error/500.html"},
	{501, "www/error/501.html"}
};

std::string ErrorHandler::createResponse(int errorCode) {
	std::string statusLine = buildStatusLine(errorCode);

	std::string filePath = "www/error/500.html";

	if (defaultErrorFilePaths.find(errorCode) != defaultErrorFilePaths.end()) {
		filePath = defaultErrorFilePaths.at(errorCode);
	}

	std::string body = readFileContents(filePath);

	std::ostringstream response;
	response	<< statusLine
				<< "Content-Length: " << body.size() << "\r\n"
				<< "Content-Type: text/html\r\n"
				<< "Connection: close\r\n\r\n"
				<< body;

	return response.str();
}

std::string ErrorHandler::createResponse(int errorCode, const std::map<int, std::string>& customErrorPages) {
	std::string statusLine = buildStatusLine(errorCode);

	std::string filePath = "www/error/500.html";

	if (defaultErrorFilePaths.find(errorCode) != defaultErrorFilePaths.end()) {
		filePath = defaultErrorFilePaths.at(errorCode);
	}

	auto it = customErrorPages.find(errorCode);
	if (it != customErrorPages.end()) {
		char resolved[PATH_MAX];
		if (realpath(it->second.c_str(), resolved) != nullptr) {
			filePath = it->second;
		}
	}

	std::string body = readFileContents(filePath);

	std::ostringstream response;
	response	<< statusLine
				<< "Content-Length: " << body.size() << "\r\n"
				<< "Content-Type: text/html\r\n"
				<< "Connection: close\r\n\r\n"
				<< body;

	return response.str();
}

std::string ErrorHandler::buildStatusLine(int errorCode) {
	auto it = defaultErrorMessages.find(errorCode);
	if (it == defaultErrorMessages.end()) {
		return "HTTP/1.1 500 " + defaultErrorMessages.at(500) + "\r\n";
	}
	return "HTTP/1.1 " + std::to_string(errorCode) + " " + it->second + "\r\n";
}

std::string ErrorHandler::readFileContents(const std::string& filePath)
{
	std::ifstream file(filePath.c_str());
	if (!file.is_open()) {
		std::string logErrorMessage = "Failed to open error file: ";
		logErrorMessage += filePath;
		print_log(RED, logErrorMessage);
		return "<html><body><h1>500 Internal Server Error</h1></body></html>";
	}

	std::ostringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}