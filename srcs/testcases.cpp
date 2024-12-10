
#include "ClientState.hpp"
#include "Response.hpp"
#include "utils.hpp"

// // normal request from chrome for http://localhost/
// void	test_1(ClientState& clientstate, std::ofstream& file) {
// 	file << "-----------------------------------TEST_1\n";
// 	std::string request_string = "GET / HTTP/1.1\nHost: localhost:8080\nConnection: keep-alive\nCache-Control: max-age=0\nsec-ch-ua: \"Chromium\";v=\"128\", \"Not;A=Brand\";v=\"24\", \"Opera GX\";v=\"114\"\nsec-ch-ua-mobile: ?0\nsec-ch-ua-platform: \"Windows\"\nUpgrade-Insecure-Requests: 1\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36 OPR/114.0.0.0 (Edition std-1)\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\nSec-Fetch-Site: none\nSec-Fetch-Mode: navigate\nSec-Fetch-User: ?1\nSec-Fetch-Dest: document\nAccept-Encoding: gzip, deflate, br, zstd\nAccept-Language: en-GB,en-US;q=0.9,en;q=0.8,de;q=0.7";
// 	RequestParser request(request_string);

// 	Response r(request, clientstate);
// 	std::string response_string = r.get_response();

// 	file << GREEN << "--REQUEST---\n" << RESET
// 		<< request_string
// 		<< GREEN << "\n----END-----\n" << RESET
// 		<< GREEN << "--RESPONSE--\n" << RESET
// 		<< response_string
// 		<< GREEN << "\n----END-----\n" << RESET
// 		<< "status\t" << r.get_status()
// 		<< "\n-----------------------------------------\n";
// }

void print_response(Response& r, std::ofstream& file) {
	file << "--REQUEST---\n"
		<< r.get_request().getRequest()
		<< "\n----END-----\n\n"
		<< "--RESPONSE--\n"
		<< r.get_response()
		<< "\n----END-----\n\n"
		<< "status\t" << r.get_status()
		<< "\n-----------------------------------------\n";
}

// get request from chrome for http://localhost/
void	test_1(ClientState& clientstate) {
	std::ofstream file("logfiles/log1.txt");
	file << "-----------------------------------TEST_1\n";
	std::string request_string = "GET / HTTP/1.1\nHost: localhost:8080\nConnection: keep-alive\nCache-Control: max-age=0\nsec-ch-ua: \"Chromium\";v=\"128\", \"Not;A=Brand\";v=\"24\", \"Opera GX\";v=\"114\"\nsec-ch-ua-mobile: ?0\nsec-ch-ua-platform: \"Windows\"\nUpgrade-Insecure-Requests: 1\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36 OPR/114.0.0.0 (Edition std-1)\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\nSec-Fetch-Site: none\nSec-Fetch-Mode: navigate\nSec-Fetch-User: ?1\nSec-Fetch-Dest: document\nAccept-Encoding: gzip, deflate, br, zstd\nAccept-Language: en-GB,en-US;q=0.9,en;q=0.8,de;q=0.7";
	RequestParser request(request_string);

	Response r(request, clientstate);
	print_response(r, file);
	file.close();
}

// get request from chrome for http://localhost/images/EDTH2.jpeg
void	test_2(ClientState& clientstate) {
	std::ofstream file("logfiles/log2.txt");
	file << "-----------------------------------TEST_2\n";
	std::string request_string = "GET /images/EDTH2.jpeg HTTP/1.1\nHost: localhost:8080\nConnection: keep-alive\nsec-ch-ua: \"Chromium\";v=\"128\", \"Not;A=Brand\";v=\"24\", \"Opera GX\";v=\"114\"\nsec-ch-ua-mobile: ?0\nsec-ch-ua-platform: \"Windows\"\nUpgrade-Insecure-Requests: 1\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36 OPR/114.0.0.0 (Edition std-1)\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\nSec-Fetch-Site: none\nSec-Fetch-Mode: navigate\nSec-Fetch-User: ?1\nSec-Fetch-Dest: document\nAccept-Encoding: gzip, deflate, br, zstd\nAccept-Language: en-GB,en-US;q=0.9,en;q=0.8,de;q=0.7";
	RequestParser request(request_string);

	Response r(request, clientstate);
	print_response(r, file);
	file.close();
}

// get request from chrome for http://localhost/form.html
void	test_3(ClientState& clientstate) {
	std::ofstream file("logfiles/log3.txt");
	file << "-----------------------------------TEST_3\n";
	std::string request_string = "GET /cgi-bin/hello.py HTTP/1.1\nHost: localhost:8080\nConnection: keep-alive\nsec-ch-ua: \"Google Chrome\";v=\"131\", \"Chromium\";v=\"131\", \"Not_A Brand\";v=\"24\"\nsec-ch-ua-mobile: ?0\nsec-ch-ua-platform: \"Windows\"\nUpgrade-Insecure-Requests: 1\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\nSec-Fetch-Site: none\nSec-Fetch-Mode: navigate\nSec-Fetch-User: ?1\nSec-Fetch-Dest: document\nAccept-Encoding: gzip, deflate, br, zstd\nAccept-Language: en-US,en;q=0.9\n";
	RequestParser request(request_string);

	Response r(request, clientstate);
	print_response(r, file);
	file.close();
}

// get request from chrome for http://localhost/form.html then post to cgi
void	test_4(ClientState& clientstate) {
	std::ofstream file("logfiles/log4.txt");
	file << "-----------------------------------TEST_4.1\n";
	{
		std::string request_string = "GET /form.html HTTP/1.1\nHost: localhost:8080\nConnection: keep-alive\nsec-ch-ua: \"Google Chrome\";v=\"131\", \"Chromium\";v=\"131\", \"Not_A Brand\";v=\"24\"\nsec-ch-ua-mobile: ?0\nsec-ch-ua-platform: \"Windows\"\nUpgrade-Insecure-Requests: 1\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\nSec-Fetch-Site: none\nSec-Fetch-Mode: navigate\nSec-Fetch-User: ?1\nSec-Fetch-Dest: document\nAccept-Encoding: gzip, deflate, br, zstd\nAccept-Language: en-US,en;q=0.9\n";
		RequestParser request(request_string);
		Response r(request, clientstate);
		print_response(r, file);
	}
	file << "\n\n-----------------------------------TEST_4.2\n";
	{
		std::string request_string = "POST /cgi-bin/post_test.py HTTP/1.1\nHost: localhost\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/117.0.0.0 Safari/537.36\nContent-Type: application/x-www-form-urlencoded\nContent-Length: 36\n\nname=Alice&message=Hello%2C+world%21";
		RequestParser request(request_string);
		Response r(request, clientstate);
		print_response(r, file);
	}
	file.close();
}
