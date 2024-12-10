#include "Response.hpp"

// this is the function that controls how the response is be created
std::string	Response::get_response(void) {
	if (!method_allowed())
		return get_error_response(405);
	if (_request.getUri().find(".py") != std::string::npos)
		return exec_script();
	if (_request.getMethod() == "DELETE")
		return handle_delete();
	return serve_static_file();
}

std::string Response::handle_delete(void) {
	if (FILE *file = fopen(_filePath.c_str(), "r"))
		fclose(file);
	else
		return (get_error_response(404));
	if (!remove(_filePath.c_str()))
		return (_request.getHttpVersion() + " 204 No Content\r\n\r\n");
	return (get_error_response(403));
}

/**
 * @brief Helper method to determine the MIME type based on the file extension
 *
 * Provides default MIME type is none matched
 *
 * @param path of the file to send
 * @return std::string MIME
 */
std::string Response::get_content_type(const std::string& path) const {
	static std::map<std::string, std::string> mime_types = {
		{".html", "text/html"},
		{".htm", "text/html"},
		{".css", "text/css"},
		{".js", "application/javascript"},
		{".png", "image/png"},
		{".jpg", "image/jpeg"},
		{".jpeg", "image/jpeg"},
		{".gif", "image/gif"},
		{".txt", "text/plain"},
		{".pdf", "application/pdf"},
	};

	size_t dot = path.find_last_of('.');
	if (dot != std::string::npos) {
		std::string fileExtension = path.substr(dot);
		if (mime_types.find(fileExtension) != mime_types.end()) {
			return mime_types.find(fileExtension)->second;
		}
	}
	return "application/octet-stream";
}

bool Response::method_allowed(void) {
	bool isSupported = std::find(
		_location.supported_methods.begin(),
		_location.supported_methods.end(),
		_request.getMethod()
	) != _location.supported_methods.end();
	return isSupported;
}

std::string Response::get_error_response(const int errorCode) {
	std::string errorMessage;
	std::string errorFilePath;

	switch (errorCode)
	{
	case 403:
		errorMessage = " 403 Forbidden\r\n";
		errorFilePath = "www/error/403.html";
		break;
	case 404:
		errorMessage = " 404 Not Found\r\n";
		errorFilePath = "www/error/404.html";
		break;
	case 405:
		errorMessage = " 405 Method Not Allowed\r\n";
		errorFilePath = "www/error/405.html";
		break;
	case 500:
		errorMessage = " 500 Internal Server Error\r\n";
		errorFilePath = "www/error/500.html";
		break;
	default:
		errorMessage = " 500 Internal Server Error\r\n";
		errorFilePath = "www/error/500.html";
		break;
	}

	char resolvedPath[PATH_MAX];
	realpath(errorFilePath.c_str(), resolvedPath);
	std::ostringstream response;
	std::ifstream file(resolvedPath);
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string body = buffer.str();
	file.close();
	response	<< _request.getHttpVersion() << errorMessage
				<< "Content-Length: " << body.length() << "\r\n"
				<< "Connection: close\r\n\r\n"
				<< body;

	return response.str();
}

std::string Response::serve_static_file() {
	std::string uri = _request.getUri();

	if (!method_allowed() || _request.getMethod() != "GET") {
		return get_error_response(405);
	}

	if (!uri.empty() && uri.back() == '/') {
		std::vector<std::string> defaultFiles = _location.default_files;
		if (!defaultFiles.empty()) {
			_filePath += defaultFiles[0];
		} else {
			return (get_error_response(403));
		}
	}

	char resolvedPath[PATH_MAX];
	if (realpath(_filePath.c_str(), resolvedPath) == NULL) {
		return get_error_response(404);
	}

	std::string resolvedFilePath(resolvedPath);

	char resolvedDocRoot[PATH_MAX];
	if (realpath(_documentRoot.c_str(), resolvedDocRoot) == NULL) {
		std::cerr << "Failed to resolve document root: " << _documentRoot << std::endl;
		return get_error_response(500);
	}

	std::string resolvedDocRootStr(resolvedDocRoot);
	if (resolvedFilePath.find(resolvedDocRootStr) != 0) {
		return get_error_response(403);
	}

	std::ifstream file(resolvedFilePath.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		return get_error_response(404);
	}

	std::ostringstream bodyStream;
	bodyStream << file.rdbuf();
	std::string body = bodyStream.str();

	std::string contentType = get_content_type(resolvedFilePath);

	std::ostringstream response;
	response	<< _request.getHttpVersion() << " 200 OK\r\n"
				<< "Content-Type: " << contentType << "\r\n"
				<< "Content-Length: " << body.size() << "\r\n"
				<< "Connection: close\r\n\r\n"
				<< body;
	return response.str();
}

// -------------------------------------------------------------------------------------------

Response::Response(RequestParser &req, ClientState& clientState)
	: _request(req), _clientState(clientState), _statuscode(200) {
	_location = getLocation(_clientState.serverConfig, _request.getUri());
	_documentRoot = _location.document_root;
	_filePath = _documentRoot + _request.getUri();
}

// Response::Response(Response &other) : _request(other.get_request()) {}

Response::~Response() {
	_env.clear();
	for (char* e : _environment) {
		free(e);
	}
	_environment.clear();
}

RequestParser&	Response::get_request(void) {return _request;}

int Response::get_status(void) {return _statuscode;}
