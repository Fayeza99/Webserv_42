#pragma once

#include <string>
#include <iostream>
#include <map>
#include <sstream>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include "utils.hpp"

class ErrorHandler {
	private:
		static const std::map<int, std::string> defaultErrorMessages;
		static const std::map<int, std::string> defaultErrorFilePaths;

		 static std::string buildStatusLine(int errorCode);
		 static std::string readFileContents(const std::string& filePath);

	public:
		static std::string createResponse(int errorCode);
		static std::string createResponse(int errorCode, const std::map<int, std::string>& customErrorPages);
};