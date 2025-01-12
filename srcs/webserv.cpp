#include "server.hpp"

int main (int argc, char **argv) {
	if (argc > 2 ) {
		std::cerr << "Usage: " << argv[0] << " <config_file_path>" << std::endl;
		return 1;
	}
	else {
		std::string configFilePath;
		if (argc == 2) {
			configFilePath = argv[1];
		} else {
			configFilePath = "config/default.conf";
		}
		Server server;
		try {
			server.configure(configFilePath);
			server.setup();
			server.run();

		} catch (const std::exception &e) {
			std::cerr << "Opps! Something went wrong!" << e.what() << std::endl;
			return 1;
		}
	}
}

// response testing ------------------------------------------------------------------

// int	main(void) {
// 	// setup with config
// 	Parser parser(readConfigFile("config/default.conf"));
// 	ServerConfig serverconf = parser.parseServer();//this means only one serverconfig is used from the file
// 	ClientState clientstate(serverconf);

// 	// test_1(clientstate);
// 	// test_2(clientstate);
// 	// test_3(clientstate);
// 	// test_4(clientstate);
// 	test_5(clientstate);
// 	return 0;
// }
