#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "GlobalConfig.hpp"  // Assuming this is the header file for your config classes
#include "Parser.hpp"        // Assuming this is the header file where your Parser class is defined
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>



int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }
    std::string configFile = argv[1];
    std::ifstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << configFile << std::endl;
        return 1;
    }
    std::string configContent((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());

    try {
        Parser parser(configContent);
        GlobalConfig config = parser.parse();
        std::cout << "Configuration parsed successfully." << std::endl;
        // Further processing with `config`...
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse configuration: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
