#pragma once

#include "RequestParser.hpp"
#include "ClientState.hpp"

#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <sys/types.h>

class CgiHandler {
	private:
		RequestParser& request;
		ClientState& clientState;

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
		CgiHandler(RequestParser& req, ClientState& cs);
		~CgiHandler();

		void prepareEnvironment();
		void executeCgi();
		void cgiChildProcess();
		void cgiParentProcess();
		void writeToCgiStdin();
		void readFromCgiStdout();
		bool isCgiFinished();
		void cleanup();
};
