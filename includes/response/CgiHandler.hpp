#pragma once

#include "AResponseHandler.hpp"

// POST or GET Cgi Requests (.py)
class CgiHandler : public AResponseHandler
{
public:
	CgiHandler(ClientState& client);
	~CgiHandler(void);

	void getResponse(void) const;

private:
	pid_t cgiPid;
	int cgiStdinPipe[2];
	int cgiStdoutPipe[2];
	std::map<std::string, std::string> envVars;
	std::vector<std::string> envStrings;
	std::vector<char*> env;
	char** envp;

	std::string scriptFileName;
	std::string scriptDirectoryPath;

	void setFilePath(void);
	void executeCgi(void);
	void parentProcess(void);
	void childProcess(void);
	bool isFinished(void);
};
