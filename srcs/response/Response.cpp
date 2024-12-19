#include "Response.hpp"

/**
 * @brief The function uses appropriate methods to generate response for specific requestes.
 *
 * @return std::string reponse in the form of string
 */
std::string	Response::get_response(void) {
	if (_location.redirect == true)
		return handle_redir();
	if (!method_allowed())
		return get_error_response(405);
	if (_request.getMethod() == "DELETE")
		return handle_delete();
	return serve_static_file();
}

/**
 * @brief Handles the delete requests and generate error responses if any.
 *
 * @return std::string Response
 */
std::string Response::handle_delete(void) {
	char realPath[PATH_MAX];
	if (realpath(_filePath.c_str(), realPath) == NULL)
		return get_error_response(404);
	if (!std::remove(_filePath.c_str()))
		return (_request.getHttpVersion() + " 204 No Content\r\n\r\n");
	return (get_error_response(403));
}

/**
 * @brief Handles file uploads.
 *
 * @return std::string Response
 */
std::string Response::handle_upload(void) {
	std::ostringstream response;
	bool success = false;
	for (const FileUpload& file : _request.getUpload()) {
		if (file.filename.empty())
			continue ;
		std::string openfile = _filePath + "/" + file.filename;
		// std::cout << "FILE TO WRITE: " << openfile << std::endl;
		std::ofstream outfile(openfile);
		outfile << file.content << std::endl;
		outfile.close();
		success = true;
	}
	if (success) {
		response << _request.getHttpVersion() << " 201 Created\r\nContent-Type: text/plain\r\n\r\nFile uploaded successfully.";
		return (response.str());
	}
	return (get_error_response(500));
}

/**
 * @brief Handles redirections
 *
 * @return std::string Response
 */
std::string Response::handle_redir(void) {
	std::ostringstream response;
	if (_request.getUri() == _location.redirect_uri)
		return serve_static_file();
	response << _request.getHttpVersion()
			 << " 302 Found\r\n"
			 << "Location: " << _location.redirect_uri << "\r\n"
			 << "Connection: close\r\n\r\n";
	return response.str();
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

/**
 * @brief Checks if the request method is allowed for the specific location
 *
 * @return true if allowed
 * @return false if not allowed
 */
bool Response::method_allowed(void) {
	bool isSupported = std::find(
		_location.supported_methods.begin(),
		_location.supported_methods.end(),
		_request.getMethod()
	) != _location.supported_methods.end();
	return isSupported;
}

/**
 * @brief Provides the correct error file on the bases of errorCode provided
 *
 * @param errorCode used for deciding which file to serve
 * @return std::string file in the form of string as response
 */
std::string Response::get_error_response(const int errorCode) {
	std::string errorMessage;
	std::string errorFilePath;

	std::map<int, std::string> default_error_pages = _clientState.serverConfig.error_pages;

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

	auto it = default_error_pages.find(errorCode);
	if (it != default_error_pages.end()) {
		std::string customErrorPath = it->second;
		char resolvedPath[PATH_MAX];
		if (realpath(customErrorPath.c_str(), resolvedPath) != NULL) {
			errorFilePath = customErrorPath;
		}
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

/**
 * @brief Serves a static file in response to an HTTP GET request.
 *
 * This function handles HTTP GET requests by locating and serving the requested static file
 * from the server's document root. It performs the following operations:
 * - Validates that the HTTP method is allowed and is specifically a GET request.
 * - Resolves the absolute path of the requested file to prevent directory traversal attacks.
 * - Ensures that the resolved file path is within the server's document root.
 * - Opens and reads the file content into memory.
 * - Determines the appropriate MIME type based on the file's extension.
 * - Constructs and returns a complete HTTP response with the file's content.
 *
 * If any validation fails or an error occurs during file access, the function returns an
 * appropriate HTTP error response (e.g., 403 Forbidden, 404 Not Found, 405 Method Not Allowed).
 *
 * @return std::string A complete HTTP response string, including status line, headers, and body.
 *
 * @note
 * - This function assumes that `_filePath` and `_documentRoot` are properly set before invocation.
 * - The function uses `realpath` to resolve absolute paths, which can fail if the file does not exist
 *   or if there are insufficient permissions.
 *
 * @see get_error_response(int), get_content_type(const std::string&), method_allowed()
 */
std::string Response::serve_static_file() {
	std::string uri = _request.getUri();
	std::ostringstream response;

	if (!method_allowed() || _request.getMethod() != "GET") {
		return get_error_response(405);
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

	response	<< _request.getHttpVersion() << " 200 OK\r\n"
				<< "Content-Type: " << contentType << "\r\n"
				<< "Content-Length: " << body.size() << "\r\n"
				<< "Connection: close\r\n\r\n"
				<< body;
	return response.str();
}

/**
 * @brief Sets the file path according to the location
 *
 */
void Response::setFilePath() {
	if (_request.getUri() == _location.uri) {
		if (_location.default_files.size() < 1 && !_request.isUpload())
			_filePath = "";
		else
			_filePath = _documentRoot + "/" + _location.default_files[0];
	} else {
		std::string uri = _request.getUri().substr(_location.uri.length());
		_filePath = _documentRoot + "/" + uri;
	}
}

Response::Response(RequestParser &req, ClientState& clientState)
	: _request(req), _clientState(clientState), _statuscode(200) {
	_location = getLocation(_clientState.serverConfig, _request.getUri());
	_documentRoot = _location.document_root;
	if (!_location.redirect)
		setFilePath();
}

Response::~Response() {
	_env.clear();
	for (char* e : _environment) {
		free(e);
	}
	_environment.clear();
}

RequestParser&	Response::get_request(void) {return _request;}

int Response::get_status(void) {return _statuscode;}
