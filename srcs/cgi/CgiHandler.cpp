#include "CgiHandler.hpp"

CgiHandler::CgiHandler(RequestParser& req, ClientState& cs) : request(req), clientState(cs), cgiPid(-1) {
	cgiStdinPipe[0] = -1;
	cgiStdinPipe[1] = -1;
	cgiStdoutPipe[0] = -1;
	cgiStdoutPipe[1] = -1;
}

void CgiHandler::prepareEnvironment(void) {
	std::string uri = _request.getUri();
	size_t queryPos = uri.find("?");

	envVars["GATEWAY_INTERFACE"] = "CGI/1.1";
	envVars["SSERVER_PROTOCOL"] = "HTTP/1.1";
	envVars["REQUEST_METHOD"] = request.getMethod();
	envVars["CONTENT_LENGTH"] = std::to_string(request.getBody().length());

	std::string scriptName = uri.substr(0, (uri.find(".py") + 3));
	scriptName = scriptName.substr(name.find_last_of("/") + 1);
	envVars["SCRIPT_NAME"] = scriptName;

	if (querryPos != std::string::npos) {
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
		envVars[header.first] = header.second;
	}
}
