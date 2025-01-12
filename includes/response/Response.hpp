#pragma once

# include <string>
# include <cstdio>
# include <iostream>
# include <unistd.h>
# include <fcntl.h>
# include <sys/socket.h>
# include <sys/wait.h>
# include <vector>
# include <algorithm>
# include <fstream>
# include <map>

# include "RequestParser.hpp"
# include "ClientState.hpp"
# include "ErrorHandler.hpp"
# include "utils.hpp"

# define PATH_MAX 1024
# define BUFFER_SIZE 1024

class Response {
public:
	Response(ClientState& clientState);
	~Response();

	std::string get_response(void);
	RequestParser& get_request(void);
	int get_status(void);
	void executeCgi();
	void writeToCgiStdin(ClientState& clientState);
	bool readFromCgiStdout(ClientState& clientState);

private:
	std::string serve_static_file(void);
	std::string handle_delete(void);
	std::string handle_redir(void);
	std::string handle_upload(void);
	std::string get_content_type(const std::string& path) const;
	bool method_allowed(void);
	void setFilePath();

	RequestParser* _request;
	ClientState& _clientState;
	LocationConfig _location;
	std::string _response;
	std::string _documentRoot;
	std::string _filePath;

	void prepareEnvironment();
	void cgiChildProcess();
	void cgiParentProcess();

	pid_t cgiPid;
	int cgiStdinPipe[2];
	int cgiStdoutPipe[2];

	std::map<std::string, std::string> envVars;
	std::vector<std::string> envStrings;
	std::vector<char*> env;
	char** envp;

	std::string scriptFileName;
	std::string scriptDirectoryPath;

	std::map<std::string, std::string> _headers;
	std::string _body;
	int _statuscode;
};

bool isCgiFinished(ClientState& clientState);


// void	test_1(ClientState& clientstate);
// void	test_2(ClientState& clientstate);
// void	test_3(ClientState& clientstate);
// void	test_4(ClientState& clientstate);
// void	test_5(ClientState& clientstate);
