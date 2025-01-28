#pragma once

#include "AResponseHandler.hpp"
#include "ErrorHandler.hpp"

// POST or GET Cgi Requests (.py)
class CgiHandler : public AResponseHandler
{
public:
	CgiHandler(ClientState &client);
	~CgiHandler(void);

	void getResponse(void);
	static void writeToCgiStdin(ClientState &client);
	static bool readFromCgiStdout(ClientState &client);

private:
	pid_t cgiPid;
	int cgiStdinPipe[2];
	int cgiStdoutPipe[2];
	std::map<std::string, std::string> envVars;
	std::vector<std::string> envStrings;
	std::vector<char *> env;

	std::string scriptFileName;
	std::string scriptDirectory;

	void setFilePath(void);
	void prepareEnvironment(void);
	void executeCgi(void);
	void parentProcess(void);
	void childProcess(void);
};
