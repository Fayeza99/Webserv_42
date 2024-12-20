#pragma once

#include "RequestParser.hpp"
#include "ClientState.hpp"
#include "KqueueManager.hpp"

#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

class CgiHandler {
	private:
		RequestParser& request;
		ClientState& clientState;
		KqueueManager& kqManager;

		pid_t cgiPid;
		int cgiStdinPipe[2];
		int cgiStdoutPipe[2];

		std::map<std::string, std::string> envVars;
		std::vector<std::string> envStrings;
		std::vector<char*> env;
		char** envp;

		std::string scriptFileName;
		std::string scriptDirectoryPath;

	public:
		CgiHandler(RequestParser& req, ClientState& cs, KqueueManager& kq);
		~CgiHandler();

		void prepareEnvironment();
		void executeCgi();
		void cgiChildProcess();
		void cgiParentProcess();

		void cleanup();
};

void writeToCgiStdin(ClientState& clientState, KqueueManager& kqManager);
void readFromCgiStdout(ClientState& clientState, KqueueManager& kqManager);
bool isCgiFinished(ClientState& clientState);
