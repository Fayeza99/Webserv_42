#include "StaticHandler.hpp"
#include "utils.hpp"
#include <dirent.h>

StaticHandler::StaticHandler(ClientState &client) : AResponseHandler(client)
{
	// print_log(BLUE, "StaticHandler Constructor");
	setFilePath();
}

StaticHandler::~StaticHandler(void)
{
	// print_log(BLUE, "StaticHandler Destructor");
}

void StaticHandler::getResponse(void)
{
	// print_log(BLUE, "getting response, filepath is -" + _filePath + "-");
	if (RequestParser::isDirectory(_filePath) && autoIndex())
		_client.responseBuffer = listDir();
	else
		_client.responseBuffer = responseString();
	KqueueManager::registerEvent(_client.fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR);
}

std::string StaticHandler::responseString(void) const
{
	std::ostringstream response;
	char resolvedPath[PATH_MAX];
	char resolvedDocRoot[PATH_MAX];
	if (_location.redirect && getUri() != _location.redirect_uri)
		return handleRedir();
	if (!methodAllowed()) {
		_client.statuscode = 405;
		return ErrorHandler::createResponse(405, getErrorPages());
	} else if (getMethod() == "POST") {
		_client.statuscode = 501;
		return ErrorHandler::createResponse(501, getErrorPages());
	} else if (realpath(_filePath.c_str(), resolvedPath) == NULL) {
		_client.statuscode = 404;
		return ErrorHandler::createResponse(404, getErrorPages());
	} else if (realpath(_location.document_root.c_str(), resolvedDocRoot) == NULL) {
		_client.statuscode = 500;
		return ErrorHandler::createResponse(500, getErrorPages());
	}

	std::string resolvedFilePath(resolvedPath);
	std::string resolvedDocRootStr(resolvedDocRoot);
	if (resolvedFilePath.find(resolvedDocRootStr) != 0) {
		_client.statuscode = 403;
		return ErrorHandler::createResponse(403, getErrorPages());
	}

	std::ifstream file(resolvedFilePath.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		_client.statuscode = 404;
		return ErrorHandler::createResponse(404, getErrorPages());
	}

	std::ostringstream bodyStream;
	bodyStream << file.rdbuf();
	std::string body = bodyStream.str();
	std::string contentType = getContentType(resolvedFilePath);
	response << getHttpVersion() << " 200 OK\r\n"
			 << "Content-Type: " << contentType << "\r\n"
			 << "Content-Length: " << body.size() << "\r\n"
			 << "Connection: close\r\n\r\n"
			 << body;
	_client.statuscode = 200;
	return response.str();
}

void StaticHandler::setFilePath(void)
{
	if (getUri() == _location.uri)
	{
		if (_location.default_files.size() > 0)
			_filePath = _location.document_root + "/" + _location.default_files[0];
		else
			_filePath = _location.document_root;
	}
	else
	{
		std::string uri = getUri().substr(_location.uri.length() - 1);
		_filePath = _location.document_root + uri;
	}
}

bool StaticHandler::autoIndex(void) const { return _location.autoIndex; }

std::string StaticHandler::listDirHtml(void) const
{
	std::ostringstream response;
	std::vector<std::string> files;
	DIR *dir = opendir(_filePath.c_str());
	std::string uri = getUri();

	if (uri.back() == '/')
		uri.pop_back();
	if (dir)
	{
		struct dirent *entry;
		while ((entry = readdir(dir)) != NULL)
		{
			std::string name(entry->d_name);
			if (name != ".")
				files.push_back(name);
		}
		closedir(dir);
	}
	response << "<!DOCTYPE html>\n<html>\n<head>\n<title>Directory Listing</title>\n</head>\n<body>\n"
			 << "<h1>" << getUri() << "</h1>\n<ul>\n";
	for (auto f : files)
		response << "<li><a href=\"" << uri << "/" << f << "\">" << f << "</a></li>\n";
	response << "</ul>\n</body>\n</html>\n";
	return response.str();
}

std::string StaticHandler::listDir(void) const
{
	std::ostringstream response;
	std::string html = listDirHtml();
	response << getHttpVersion() << " 200 OK\r\n"
			 << "Content-Type: text/html\r\n"
			 << "Content-Length: " << html.length() << "\r\n"
			 << "Connection: close\r\n\r\n"
			 << html;
	_client.statuscode = 200;
	return response.str();
}

std::string StaticHandler::handleRedir(void) const
{
	std::ostringstream response;
	response << getHttpVersion()
			 << " 302 Found\r\n"
			 << "Location: " << _location.redirect_uri << "\r\n"
			 << "Connection: close\r\n\r\n";
	_client.statuscode = 302;
	return response.str();
}
