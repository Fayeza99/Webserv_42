#include <iostream>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <GlobalConfig.hpp>

#define RESET   "\033[0m"
#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

std::string readConfigFile(const std::string& configFilePath);
void printGlobalConfig(const GlobalConfig& config, int indent);
std::string getDocumentRoot(const ServerConfig &serverConfig, const std::string &uri);
LocationConfig getLocation(const ServerConfig &serverConfig, const std::string &uri);

void print_log(const char *color, std::string msg);
