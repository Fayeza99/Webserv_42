#include "Response.hpp"

Response::Response(ClientState& clientState) : _clientState(clientState), _location(), _statuscode(200) {
	print_log(BLUE, "Response Constructor");
	_request = clientState.request;
	_location.getLocation(_clientState.serverConfig, (*_request).getUri());
	_documentRoot = _location.document_root;

	if (!_location.redirect){
		setFilePath();
	}
	print_log(BLACK, "[REQUEST] Method=" + (*_request).getMethod() + ", Uri=" + (*_request).getUri() + ", Path=" + _filePath);
	std::cout << BLACK << "HEADERS";
	for (auto h : (*_request).getHeaders()) {
		std::cout << "\n\t" << h.first << ":" << h.second;
	}
	std::cout << "\nBODY\n" << (*_request).getBody() << "\n" << RESET;
}

Response::~Response() {
	print_log(BLUE, "Response Destructor");
}

/**
 * @brief The function uses appropriate methods to generate response for specific requestes.
 *
 * @return std::string reponse in the form of string
 */
std::string	Response::get_response(void) {
	if (_location.redirect == true)
		return handle_redir();
	if (!method_allowed())
		return ErrorHandler::createResponse(405, _clientState.serverConfig.error_pages);
	if ((*_request).getMethod() == "DELETE")
		return handle_delete();
	if ((*_request)._isUpload)
		return handle_upload();
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
		return ErrorHandler::createResponse(404, _clientState.serverConfig.error_pages);
	if (!std::remove(_filePath.c_str()))
		return ((*_request).getHttpVersion() + " 204 No Content\r\n\r\n");
	return ErrorHandler::createResponse(403, _clientState.serverConfig.error_pages);
}

/**
 * @brief Handles file uploads.
 *
 * @return std::string Response
 */
std::string Response::handle_upload(void) {
	std::ostringstream response;
	bool success = false;
	for (const FileUpload& file : (*_request).getUpload()) {
		if (file.filename.empty()) {
			continue ;
		}
		std::string path = _filePath + "/" + file.filename;
		std::ofstream outfile(path);
		outfile << file.content;
		outfile.close();
		success = true;
	}
	if (success) {
		response << (*_request).getHttpVersion()
			 << " 201 Created\r\n"
			 << "Content-Length: 0\r\n"
			 << "Connection: close\r\n\r\n";
		return (response.str());
	}
	print_log(RED, "[ERROR] handleUpload");
	return ErrorHandler::createResponse(500, _clientState.serverConfig.error_pages);
}

/**
 * @brief Handles redirections
 *
 * @return std::string Response
 */
std::string Response::handle_redir(void) {
	std::ostringstream response;
	if ((*_request).getUri() == _location.redirect_uri)
		return serve_static_file();
	response << (*_request).getHttpVersion()
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
		(*_request).getMethod()
	) != _location.supported_methods.end();
	return isSupported;
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
	std::string uri = (*_request).getUri();
	std::ostringstream response;

	if (!method_allowed() || (*_request).getMethod() != "GET") {
		return ErrorHandler::createResponse(405, _clientState.serverConfig.error_pages);
	}

	char resolvedPath[PATH_MAX];
	if (realpath(_filePath.c_str(), resolvedPath) == NULL) {
		return ErrorHandler::createResponse(404, _clientState.serverConfig.error_pages);
	}

	std::string resolvedFilePath(resolvedPath);

	char resolvedDocRoot[PATH_MAX];
	if (realpath(_documentRoot.c_str(), resolvedDocRoot) == NULL) {
		std::cerr << "[ERROR] Failed to resolve document root: " << _documentRoot << std::endl;
		return ErrorHandler::createResponse(500, _clientState.serverConfig.error_pages);
	}

	std::string resolvedDocRootStr(resolvedDocRoot);
	if (resolvedFilePath.find(resolvedDocRootStr) != 0) {
		return ErrorHandler::createResponse(403, _clientState.serverConfig.error_pages);
	}

	std::ifstream file(resolvedFilePath.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		return ErrorHandler::createResponse(404, _clientState.serverConfig.error_pages);
	}

	std::ostringstream bodyStream;
	bodyStream << file.rdbuf();
	std::string body = bodyStream.str();

	std::string contentType = get_content_type(resolvedFilePath);

	response	<< (*_request).getHttpVersion() << " 200 OK\r\n"
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
	if ((*_request).getUri() == _location.uri) {
		if (_location.default_files.size() < 1 && !(*_request)._isUpload) {
			_filePath = "";
			print_log(RED, "HERE");
		}
		else if (_location.default_files.size() < 1 && (*_request)._isUpload)
			_filePath = _documentRoot;
		else {
			_filePath = _documentRoot + "/" + _location.default_files[0];
		}
	} else {
		std::string uri = (*_request).getUri().substr(_location.uri.length());
		_filePath = _documentRoot + "/" + uri;
	}
}

void Response::prepareEnvironment(void) {
	std::string uri = (*_request).getUri();
	size_t queryPos = uri.find("?");

	envVars["GATEWAY_INTERFACE"] = "CGI/1.1";
	envVars["SERVER_PROTOCOL"] = "HTTP/1.1";
	envVars["REQUEST_METHOD"] = (*_request).getMethod();
	envVars["CONTENT_LENGTH"] = std::to_string((*_request).getBody().length());

	size_t pyPos = uri.find(".py");
	if (pyPos != std::string::npos) {
		std::string scriptName = uri.substr(0, pyPos + 3);
		size_t lastSlash = scriptName.find_last_of("/");
		if (lastSlash != std::string::npos) {
			scriptName = scriptName.substr(lastSlash + 1);
		}
		envVars["SCRIPT_NAME"] = scriptName;
	} else {
		envVars["SCRIPT_NAME"] = uri;
	}

	if (queryPos != std::string::npos) {
		envVars["PATH_INFO"] = uri.substr(0, queryPos);
		envVars["QUERY_STRING"] = uri.substr(queryPos + 1);
	} else {
		envVars["PATH_INFO"] = uri;
		envVars["QUERY_STRING"] = "";
	}

	envVars["SERVER_PORT"] = std::to_string(_clientState.serverConfig.listen_port);
	envVars["REMOTE_PORT"] = std::to_string(_clientState.clientPort);
	envVars["REMOTE_ADDR"] = _clientState.clientIPAddress;

	auto headers = (*_request).getHeaders();
	for (const auto& header : headers) {
		std::string key = header.first;
		std::string upper_key = key;
		for (auto &c : upper_key) { c = std::toupper((unsigned char)c); }
		std::replace(upper_key.begin(), upper_key.end(), '-', '_');
		envVars["HTTP_" + upper_key] = header.second;
	}

	for (const auto& pair : envVars) {
		envStrings.emplace_back(pair.first + "=" + pair.second);
	}
	for (auto& envStr : envStrings) {
		env.push_back(const_cast<char*>(envStr.c_str()));
	}

	env.push_back(nullptr);
	envp = env.data();
}

void Response::executeCgi() {
	cgiStdinPipe[0] = -1;
	cgiStdinPipe[1] = -1;
	cgiStdoutPipe[0] = -1;
	cgiStdoutPipe[1] = -1;
	prepareEnvironment();

	scriptFileName = (*_request).getUri().substr(0, ((*_request).getUri().find(".py") + 3));
	scriptFileName = scriptFileName.substr(scriptFileName.find_last_of("/") + 1);

	scriptDirectoryPath = _filePath.substr(0, _filePath.find_last_of("/")).c_str();

	// print_log(WHITE, "[DEBUG] cgi started. script: " + scriptFileName
	// 		+ ", path: " + scriptDirectoryPath);
	if (pipe(cgiStdinPipe) == -1 || pipe(cgiStdoutPipe) == -1) {
		throw std::runtime_error("Falied to create CGI pipes");
	}

	cgiPid = fork();
	_clientState.cgiPid = cgiPid;
	if (cgiPid == -1) {
		throw std::runtime_error("Failed to fork CGI process");
	}

	if (cgiPid == 0) {
		cgiChildProcess();
	} else {
		cgiParentProcess();
	}
}

void Response::cgiChildProcess() {
	dup2(cgiStdinPipe[0], STDIN_FILENO);
	dup2(cgiStdoutPipe[1], STDOUT_FILENO);

	close(cgiStdinPipe[1]);
	close(cgiStdoutPipe[0]);

	if (chdir(scriptDirectoryPath.c_str()) == -1) {
		perror("chdir failed");
		exit(1);
	}

	char *argv[] = {(char *)"/usr/bin/python3", (char *)scriptFileName.c_str(), NULL};
	print_log(WHITE, "[DEBUG] executing script with python...");
	execve("/usr/bin/python3", argv, envp);
	exit(1);
}

void Response::cgiParentProcess() {
	close(cgiStdinPipe[0]);
	close(cgiStdoutPipe[1]);

	fcntl(cgiStdinPipe[1], F_SETFL, O_NONBLOCK);
	fcntl(cgiStdoutPipe[0], F_SETFL, O_NONBLOCK);

	_clientState.cgiInputFd = cgiStdinPipe[1];
	_clientState.cgiOutputFd = cgiStdoutPipe[0];

	if (!(*_request).getBody().empty()) {
		KqueueManager::registerEvent(_clientState.cgiInputFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR);
	}
	KqueueManager::registerEvent(_clientState.cgiOutputFd, EVFILT_READ, EV_ADD | EV_ENABLE | EV_CLEAR);
}

void Response::writeToCgiStdin(ClientState& clientState) {
	// print_log(BLACK, "[FUNC] writeToCgiStdin");
	if (clientState.cgiInputFd < 0) return;

	if (clientState.request->getBody().empty()) {
		KqueueManager::registerEvent(clientState.cgiInputFd, EVFILT_WRITE, EV_DELETE);
		close(clientState.cgiInputFd);
		clientState.cgiInputFd = -1;
		return;
	}

	const std::string &body = clientState.request->getBody();
	ssize_t bytesSent = write(clientState.cgiInputFd, body.data(), body.size());

	if (bytesSent > 0) {
		KqueueManager::registerEvent(clientState.cgiInputFd, EVFILT_WRITE, EV_DELETE);
		close(clientState.cgiInputFd);
		clientState.cgiInputFd = -1;
	} else if (bytesSent == -1) {
		std::cerr << "[ERROR] Error Occurred while writing to cgi stdin" << std::endl;
		KqueueManager::registerEvent(clientState.cgiInputFd, EVFILT_WRITE, EV_DELETE);
		close(clientState.cgiInputFd);
		clientState.cgiInputFd = -1;
	}
}

bool isCgiFinished(ClientState& clientState) {

	if (clientState.cgiPid <= 0) return true;
	int status;
	// print_log(WHITE, "[DEBUG] calling waitpid...");
	pid_t result = waitpid(clientState.cgiPid, &status, WNOHANG);
	// print_log(WHITE, "[DEBUG] waitpid returned.");
	if (result == 0) {
		return false;
	} else if (result == clientState.cgiPid) {
		return true;
	} else {
		std::cerr << "[ERROR] Something went wrong with waitpid" << std::endl;
		return true;
	}
}

bool Response::readFromCgiStdout(ClientState& clientState) {
	// print_log(BLACK, "[FUNC] readFromCgiStdout");
	if (clientState.cgiOutputFd < 0) return false;//!!!

	char buffer[4096];
	ssize_t bytesRead = read(clientState.cgiOutputFd, buffer, sizeof(buffer));
	buffer[bytesRead] = '\0';

	// problems with kq here >>>
	std::string response = buffer;
	// print_log(RED, "cgi out: " + std::to_string(bytesRead));

	if (bytesRead > 0) {
		if (clientState.responseBuffer.empty())
			clientState.responseBuffer = "HTTP/1.1 200 OK\r\n";
		clientState.responseBuffer += response;
		return false;
	} else if (bytesRead == 0) {
		KqueueManager::registerEvent(clientState.cgiOutputFd, EVFILT_READ, EV_DELETE);//?
		KqueueManager::registerEvent(clientState.fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR);//?
		close(clientState.cgiOutputFd);
		clientState.cgiOutputFd = -1;

		if (!isCgiFinished(clientState)) {
			std::cerr << "[ERROR] Something went wrong in CGI, not finished" << std::endl;
		} else {
			print_log(WHITE, "[DEBUG] script finished. returning response.");
		}
		return true;
	} else {
		std::cerr << "[ERROR] Error Occurred while reading from cgi stdout" << std::endl;
		KqueueManager::registerEvent(clientState.cgiOutputFd, EVFILT_READ, EV_DELETE);
		clientState.responseBuffer.erase();
		print_log(RED, "[ERROR] readFromCgiStdout");
		clientState.responseBuffer = ErrorHandler::createResponse(500, clientState.serverConfig.error_pages);
		KqueueManager::registerEvent(clientState.fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR);//?
		close(clientState.cgiOutputFd);
		clientState.cgiOutputFd = -1;
		return false;
	}
}


// Getters
RequestParser&	Response::get_request(void) {return (*_request);}

int Response::get_status(void) {return _statuscode;}
