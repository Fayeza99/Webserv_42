#include "webserv.hpp"

int main (int argc, char **argv) {
	if (argc == 2) {
		Server server;
		try {
			server.configure(argv[1]);
			server.setup();
			server.run();
		} catch (const std::exception &e) {
			std::cerr << "Opps! Something went wrong!" << e.what() << std::endl;
			return 1;
		}
	} else {
		std::cerr << "Usage: " << argv[0] << " <config_file_path>" << std::endl;
		return 1;
	}
}