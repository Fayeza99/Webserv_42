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

// cgi testing ------------------------------------------------------------------

//normal request from chrome for http://localhost/
void	test_1(void) {
	std::cout << "-----------------------------------TEST_1\n";
	std::string request_string = "GET / HTTP/1.1\nHost: localhost:8080\nConnection: keep-alive\nCache-Control: max-age=0\nsec-ch-ua: \"Chromium\";v=\"128\", \"Not;A=Brand\";v=\"24\", \"Opera GX\";v=\"114\"\nsec-ch-ua-mobile: ?0\nsec-ch-ua-platform: \"Windows\"\nUpgrade-Insecure-Requests: 1\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36 OPR/114.0.0.0 (Edition std-1)\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\nSec-Fetch-Site: none\nSec-Fetch-Mode: navigate\nSec-Fetch-User: ?1\nSec-Fetch-Dest: document\nAccept-Encoding: gzip, deflate, br, zstd\nAccept-Language: en-GB,en-US;q=0.9,en;q=0.8,de;q=0.7";
	RequestParser request(request_string);

	Response response(request, "/");
	std::string response_string = response.get_response();

	std::cout << "--RESPONSE--\n" << response_string << "----END-----\n"
		<< "status\t" << response.get_status() << std::endl;
	std::cout << "-----------------------------------------\n";
}

// int	main(void) {
// 	std::cout << "\n";
// 	test_1();
// 	return (0);
// }
