#include "CgiHandler.hpp"

CgiHandler::CgiHandler(ClientState &client) : AResponseHandler(client)
{
	// print_log(BLUE, "CgiHandler Constructor");
	cgiStdinPipe[0] = -1;
	cgiStdinPipe[1] = -1;
	cgiStdoutPipe[0] = -1;
	cgiStdoutPipe[1] = -1;

	setFilePath();
	prepareEnvironment();
	scriptFileName = getUri().substr(0, (getUri().find(".py") + 3));
	scriptFileName = scriptFileName.substr(scriptFileName.find_last_of("/") + 1);
	scriptDirectory = _filePath.substr(0, _filePath.find_last_of("/")).c_str();
}

CgiHandler::~CgiHandler(void)
{
	// print_log(BLUE, "CgiHandler Destructor");
}

void CgiHandler::getResponse(void)
{
	char realPath[PATH_MAX];
	_client.statuscode = 200;
	if (realpath(_filePath.c_str(), realPath) == NULL)
	{
		_client.responseBuffer = ErrorHandler::createResponse(500, getErrorPages());
		_client.statuscode = 500;
		KqueueManager::registerEvent(_client.fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR);
		return;
	}
	try
	{
		executeCgi();
	}
	catch (const std::exception &e)
	{
		print_log(RED, "[ERROR] executeCgi");
		_client.responseBuffer = ErrorHandler::createResponse(500, getErrorPages());
		_client.statuscode = 500;
		KqueueManager::registerEvent(_client.fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR);
	}
}

void CgiHandler::setFilePath(void)
{
	if (getUri() == _location.uri)
	{
		if (_location.default_files.size() > 0)
			_filePath = _location.document_root + "/" + _location.default_files[0];
	}
	else
	{
		std::string uri = getUri().substr(_location.uri.length());
		_filePath = _location.document_root + "/" + uri;
	}
}

void CgiHandler::prepareEnvironment(void)
{
	size_t queryPos = getUri().find("?");

	envVars["GATEWAY_INTERFACE"] = "CGI/1.1";
	envVars["SERVER_PROTOCOL"] = "HTTP/1.1";
	envVars["REQUEST_METHOD"] = getMethod();
	envVars["CONTENT_LENGTH"] = std::to_string(getBody().length());

	size_t pyPos = getUri().find(".py");
	if (pyPos != std::string::npos)
	{
		std::string scriptName = getUri().substr(0, pyPos + 3);
		size_t lastSlash = scriptName.find_last_of("/");
		if (lastSlash != std::string::npos)
			scriptName = scriptName.substr(lastSlash + 1);
		envVars["SCRIPT_NAME"] = scriptName;
	}
	else
		envVars["SCRIPT_NAME"] = getUri();

	envVars["PATH_INFO"] = getUri();
	envVars["QUERY_STRING"] = "";
	if (queryPos != std::string::npos)
	{
		envVars["PATH_INFO"] = getUri().substr(0, queryPos);
		envVars["QUERY_STRING"] = getUri().substr(queryPos + 1);
	}

	envVars["SERVER_PORT"] = std::to_string(_client.serverConfig.listen_port);
	envVars["REMOTE_PORT"] = std::to_string(_client.clientPort);
	envVars["REMOTE_ADDR"] = _client.clientIPAddress;

	for (const auto &header : getHeaders())
	{
		std::string key = header.first;
		for (auto &c : key)
			c = std::toupper((unsigned char)c);
		std::replace(key.begin(), key.end(), '-', '_');
		envVars["HTTP_" + key] = header.second;
	}

	for (const auto &pair : envVars)
		envStrings.emplace_back(pair.first + "=" + pair.second);
	for (auto &envStr : envStrings)
		env.push_back(const_cast<char *>(envStr.c_str()));
	env.push_back(nullptr);
}

void CgiHandler::executeCgi(void)
{
	if (pipe(cgiStdinPipe) == -1 || pipe(cgiStdoutPipe) == -1)
		throw std::runtime_error("Falied to create CGI pipes");

	cgiPid = fork();
	_client.cgiPid = cgiPid;
	if (cgiPid == -1)
		throw std::runtime_error("Failed to fork CGI process");

	if (cgiPid == 0) {
		childProcess();
	}
	else
	{
		KqueueManager::registerTimer(cgiPid, 5);
		parentProcess();

	}
}

void CgiHandler::parentProcess(void)
{
	close(cgiStdinPipe[0]);
	close(cgiStdoutPipe[1]);

	fcntl(cgiStdinPipe[1], F_SETFL, O_NONBLOCK);
	fcntl(cgiStdoutPipe[0], F_SETFL, O_NONBLOCK);

	_client.cgiInputFd = cgiStdinPipe[1];
	_client.cgiOutputFd = cgiStdoutPipe[0];

	if (!getBody().empty())
		KqueueManager::registerEvent(_client.cgiInputFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR);
	KqueueManager::registerEvent(_client.cgiOutputFd, EVFILT_READ, EV_ADD | EV_ENABLE | EV_CLEAR);
}

void CgiHandler::childProcess(void)
{
	dup2(cgiStdinPipe[0], STDIN_FILENO);
	dup2(cgiStdoutPipe[1], STDOUT_FILENO);

	close(cgiStdinPipe[1]);
	close(cgiStdoutPipe[0]);

	if (chdir(scriptDirectory.c_str()) == -1)
	{
		print_log(RED, "[ERROR] chdir");
		exit(1);
	}

	char *argv[] = {(char *)"/usr/bin/python3", (char *)scriptFileName.c_str(), NULL};
	// print_log(BLACK, "[DEBUG] executing cgi");
	execve("/usr/bin/python3", argv, env.data());
	exit(1);
}

// bool CgiHandler::isFinished(ClientState &client)
// {
// 	if (client.cgiPid <= 0)
// 		return true;
// 	int status;
// 	pid_t result = waitpid(client.cgiPid, &status, 0);
// 	if (WIFEXITED(status) && status != 0)
// 		return false;
// 	else if (result == client.cgiPid)
// 		return true;
// 	else
// 	{
// 		print_log(RED, "[ERROR] Something went wrong with waitpid.");
// 		return true;
// 	}
// }

void CgiHandler::writeToCgiStdin(ClientState &client)
{
	// print_log(BLACK, "[FUNC] writeToCgiStdin");
	if (client.cgiInputFd < 0)
		return;

	if (client.request->getBody().empty())
	{
		KqueueManager::registerEvent(client.cgiInputFd, EVFILT_WRITE, EV_DELETE);
		close(client.cgiInputFd);
		client.cgiInputFd = -1;
		return;
	}

	const std::string &body = client.request->getBody();
	ssize_t bytesSent = write(client.cgiInputFd, body.data(), body.size());

	if (bytesSent > 0)
	{
		KqueueManager::registerEvent(client.cgiInputFd, EVFILT_WRITE, EV_DELETE);
		close(client.cgiInputFd);
		client.cgiInputFd = -1;
	}
	else if (bytesSent == -1)
	{
		print_log(RED, "[ERROR] Error Occurred while writing to cgi stdin.");
		KqueueManager::registerEvent(client.cgiInputFd, EVFILT_WRITE, EV_DELETE);
		close(client.cgiInputFd);
		client.cgiInputFd = -1;
	}
}

bool CgiHandler::readFromCgiStdout(ClientState &client)
{
	// print_log(BLACK, "[FUNC] readFromCgiStdout");
	if (client.cgiOutputFd < 0)
		return false;

	char buffer[4096];
	ssize_t bytesRead = read(client.cgiOutputFd, buffer, sizeof(buffer));
	buffer[bytesRead] = '\0';
	std::string response = buffer;

	if (bytesRead > 0) {
		if (client.responseBuffer.empty())
			client.responseBuffer = "HTTP/1.1 200 OK\r\n";
		client.responseBuffer += response;
		return false;
	} else if (bytesRead == 0) {
		KqueueManager::registerEvent(client.cgiOutputFd, EVFILT_READ, EV_DELETE);
		close(client.cgiOutputFd);
		client.cgiOutputFd = -1;
		if (client.cgiPid > 0)
		{
			int status;
			waitpid(client.cgiPid, &status, 0);
			if (WIFEXITED(status) && status != 0) {
				client.responseBuffer = ErrorHandler::createResponse(500, client.serverConfig.error_pages);
				client.statuscode = 500;
			} else if (WIFSIGNALED(status)) {
				client.responseBuffer = ErrorHandler::createResponse(504, client.serverConfig.error_pages);
				client.statuscode = 504;
			}
			KqueueManager::removeTimeout(client.cgiPid);
		}
		KqueueManager::registerEvent(client.fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR);
		return true;
	} else {
		print_log(RED, "[ERROR] readFromCgiStdout");
		KqueueManager::registerEvent(client.cgiOutputFd, EVFILT_READ, EV_DELETE);
		client.responseBuffer.erase();
		client.responseBuffer = ErrorHandler::createResponse(500, client.serverConfig.error_pages);
		client.statuscode = 500;
		KqueueManager::registerEvent(client.fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR);
		close(client.cgiOutputFd);
		client.cgiOutputFd = -1;
		return false;
	}
}

// HOW TO CGI TIMEOUT WITH KQ: (!!!)

// bool CgiHandler::handleCGIWithTimeout(KqueueManager &kqManager, pid_t cgiPid)
// {
// 	// Register the process as a monitored event in kqueue
// 	struct kevent change;
// 	EV_SET(&change, cgiPid, EVFILT_PROC, EV_ADD | EV_ENABLE, NOTE_EXIT, 0, nullptr);

// 	if (kevent(kqManager.kq, &change, 1, nullptr, 0, nullptr) == -1)
// 	{
// 		std::cerr << "[ERROR] Failed to register CGI process in kqueue: " << strerror(errno) << std::endl;
// 		return false;
// 	}

// 	// Wait for either the process to exit or the timeout to occur
// 	struct kevent event;
// 	struct timespec timeout = { CGI_TIMEOUT_SECONDS, 0 }; // Timeout duration (e.g., 10 seconds)

// 	int nev = kevent(kqManager.kq, nullptr, 0, &event, 1, &timeout);
// 	if (nev == -1)
// 	{
// 		std::cerr << "[ERROR] kevent failed: " << strerror(errno) << std::endl;
// 		return false; // Treat as no timeout, handle as normal
// 	}
// 	else if (nev == 0)
// 	{
// 		// Timeout occurred
// 		print_log(RED, "[ERROR] CGI script timed out.");
// 		kill(cgiPid, SIGKILL); // Terminate the stuck process
// 		waitpid(cgiPid, nullptr, 0); // Clean up process resources
// 		return true; // Indicate timeout occurred
// 	}
// 	else if (event.filter == EVFILT_PROC && event.fflags & NOTE_EXIT)
// 	{
// 		// Process exited normally
// 		waitpid(cgiPid, nullptr, 0); // Clean up process resources
// 		return false; // No timeout occurred
// 	}

// 	return false; // Default case, no timeout
// }