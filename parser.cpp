#include <iostream>
#include <cstring>
#include <string>
#include <unordered_map>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080

class HttpRequestParser {
public:
    std::string method;
    std::string uri;
    std::string http_version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    // Constructor that takes the raw request string and parses it
    HttpRequestParser(const std::string& request) {
        parseRequest(request);
    }

private:
    void parseRequest(const std::string& request) {
        std::istringstream request_stream(request);
        std::string line;

        // Parse the request line
        if (std::getline(request_stream, line)) {
            std::istringstream line_stream(line);
            line_stream >> method >> uri >> http_version;
        }

        // Parse the headers
        while (std::getline(request_stream, line) && line != "\r") {
            if (line.empty() || line == "\r")
                break;
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string header_name = line.substr(0, colon);
                std::string header_value = line.substr(colon + 1);
                // Remove possible leading whitespaces
                header_value.erase(0, header_value.find_first_not_of(" \t"));
                // Remove possible trailing carriage return
                if (!header_value.empty() && header_value.back() == '\r') {
                    header_value.pop_back();
                }
                headers[header_name] = header_value;
            }
        }

        // Parse the body (if any)
        if (request_stream.peek() != EOF) {
            body = std::string(std::istreambuf_iterator<char>(request_stream), {});
        }
    }
};

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[2048] = {0};

    const char* http_response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 46\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"
        "<html><body><h1>Hello from server!</h1></body></html>";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        return -1;
    }

    // Attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        close(server_fd);
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Binding socket to the address
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return -1;
    }

    // Listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        return -1;
    }

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(new_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            std::cout << "Client disconnected or read error." << std::endl;
            close(new_socket);
            continue;
        }

        // Convert buffer to std::string for easier handling
        std::string request(buffer);

        // Parse the HTTP Request using the HttpRequestParser class
        HttpRequestParser httpRequest(request);

        // Output parsed request details
        std::cout << "Method: " << httpRequest.method << std::endl;
        std::cout << "URI: " << httpRequest.uri << std::endl;
        std::cout << "HTTP Version: " << httpRequest.http_version << std::endl;

        std::cout << "Headers:" << std::endl;
        for (const auto& header : httpRequest.headers) {
            std::cout << header.first << ": " << header.second << std::endl;
        }

        if (!httpRequest.body.empty()) {
            std::cout << "Body: " << httpRequest.body << std::endl;
        }

        // Sending a response to the client
        send(new_socket, http_response, strlen(http_response), 0);
        std::cout << "HTTP response sent." << std::endl;

        // Handling quit command within the request
        if (httpRequest.uri.find("/quit") != std::string::npos) {
            std::cout << "Quit command received, shutting down." << std::endl;
            close(new_socket);
            break;
        }

        close(new_socket);
    }

    close(server_fd);
    return 0;
}
