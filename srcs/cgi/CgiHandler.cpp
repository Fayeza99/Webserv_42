#include "CgiHandler.hpp"

CgiHandler::CgiHandler(RequestParser& req, ClientState& cs, KqueueManager& kq) : request(req), clientState(cs), kqManager(kq), cgiPid(-1) {
	cgiStdinPipe[0] = -1;
	cgiStdinPipe[1] = -1;
	cgiStdoutPipe[0] = -1;
	cgiStdoutPipe[1] = -1;

	scriptFileName = "hello.py";
	scriptDirectoryPath = "www/cgi-bin/";
}

void CgiHandler::prepareEnvironment(void) {
	std::string uri = request.getUri();
	size_t queryPos = uri.find("?");

	envVars["GATEWAY_INTERFACE"] = "CGI/1.1";
	envVars["SERVER_PROTOCOL"] = "HTTP/1.1";
	envVars["REQUEST_METHOD"] = request.getMethod();
	envVars["CONTENT_LENGTH"] = std::to_string(request.getBody().length());

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

	envVars["SERVER_PORT"] = clientState.serverConfig.listen_port;
	envVars["REMOTE_PORT"] = clientState.clientPort;
	envVars["REMOTE_ADDR"] = clientState.clientIPAddress;

	auto headers = request.getHeaders();
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

void CgiHandler::executeCgi() {
	if (pipe(cgiStdinPipe) == -1 || pipe(cgiStdoutPipe) == -1) {
		throw std::runtime_error("Falied to create CGI pipes");
	}

	cgiPid = fork();
	if (cgiPid == -1) {
		throw std::runtime_error("Failed to fork CGI process");
	}

	if (cgiPid == 0) {
		cgiChildProcess();
	} else {
		cgiParentProcess();
	}
}

void CgiHandler::cgiChildProcess() {
	dup2(cgiStdinPipe[0], STDIN_FILENO);
	dup2(cgiStdoutPipe[1], STDOUT_FILENO);

	close(cgiStdinPipe[1]);
	close(cgiStdoutPipe[0]);

	if (chdir(scriptDirectoryPath.c_str()) == -1)
		exit(1);

	char *argv[] = {(char *)"/usr/bin/python3", (char *)scriptFileName.c_str()};
	execve(scriptFileName.c_str(), argv, envp);

	exit(1);
}

void CgiHandler::cgiParentProcess() {
	close(cgiStdinPipe[0]);
	close(cgiStdoutPipe[1]);

	fcntl(cgiStdinPipe[1], F_SETFL, O_NONBLOCK);
	fcntl(cgiStdoutPipe[0], F_SETFL, O_NONBLOCK);

	clientState.cgiInputFd = cgiStdinPipe[1];
	clientState.cgiOutputFd = cgiStdoutPipe[0];

	if (!request.getBody().empty()) {
		kqManager.registerEvent(clientState.cgiInputFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR);
	}
	kqManager.registerEvent(clientState.cgiOutputFd, EVFILT_READ, EV_ADD | EV_ENABLE | EV_CLEAR);
}