# Webserv
 Webserv is a high-performance, event-driven HTTP server written in C++ that utilizes non-blocking I/O and kqueue for efficient connection management. The server is designed to mimic features found in popular servers like Nginx, including the ability to serve static files and execute CGI scripts configurable —all via Nginx-like configuration files.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [File Structure](#file-structure)
- [Requirements](#requirements)
- [Installation and Build Instructions](#installation-and-build-instructions)
- [Configuration](#configuration)
- [Usage](#usage)
- [Testing](#testing)



## Overview

Webserv is built with modern C++ and POSIX APIs and leverages an event-driven architecture using `kqueue` for managing thousands of concurrent connections efficiently. It supports both static file serving and dynamic content generation via CGI, while also providing robust error handling.


## Features

- **Non-Blocking I/O:** Uses POSIX sockets in non-blocking mode with kqueue for scalable event notification.
- **Static File Serving:** Serves files (HTML, CSS, JavaScript, images, videos, etc.) from a dedicated static directory.
- **CGI Support:** Executes CGI scripts (e.g., Python scripts) to generate dynamic content.
- **Configuration Driven:** Reads configuration from Nginx-like config files that define server, location, and global settings.
- **Keep-Alive Connections:** Supports persistent connections to reduce latency and resource usage.
- **Detailed Logging & Error Handling:** Logs critical errors and handles unexpected situations gracefully.

## File Structure

```plaintext
Webserv_42/
├── config
│   └── default.conf
├── includes
│   ├── parser
│   │   ├── GlobalConfig.hpp
│   │   ├── Lexer.hpp
│   │   ├── Parser.hpp
│   │   └── RequestParser.hpp
│   ├── response
│   │   ├── AResponseHandler.hpp
│   │   ├── CgiHandler.hpp
│   │   ├── DeleteHandler.hpp
│   │   ├── ErrorHandler.hpp
│   │   ├── ResponseControl.hpp
│   │   ├── StaticHandler.hpp
│   │   └── UploadHandler.hpp
│   ├── server
│   │   ├── ClientState.hpp
│   │   ├── KqueueManager.hpp
│   │   └── server.hpp
│   └── utils
│       └── utils.hpp
├── Makefile
├── readme.md
├── siege.siegerc
├── srcs
│   ├── parsing
│   │   ├── Config.cpp
│   │   ├── Lexer.cpp
│   │   ├── Parser.cpp
│   │   └── RequestParser.cpp
│   ├── response
│   │   ├── AResponseHandler.cpp
│   │   ├── CgiHandler.cpp
│   │   ├── DeleteHandler.cpp
│   │   ├── ErrorHandler.cpp
│   │   ├── ResponseControl.cpp
│   │   ├── StaticHandler.cpp
│   │   └── UploadHandler.cpp
│   ├── server
│   │   ├── KqueueManager.cpp
│   │   └── server.cpp
│   ├── utils
│   │   └── utils.cpp
│   └── webserv.cpp
├── tests
│   ├── fileUpload.txt
│   ├── siege.sh
│   ├── siege.siegerc
│   └── tests.py
└── www
    ├── assets
    │   ├── css
    │   │   └── styles.css
    │   ├── img/
    │   ├── js
    │   │   └── main.js
    │   └── scss
    │       └── styles.scss
    ├── cgi-bin
    │   ├── environment.py
    │   ├── loop.py
    │   └── post_test.py
    ├── error/
    ├── error_custom/
    ├── form.html
    ├── index.html
    ├── post_test.html
    ├── upload
    │   ├── bigUpload.txt
    │   └── empty.txt
    └── upload.html
```

**Notes:**
- **`www`**: Contains all static files and assests required for the static website. In addtion to that, it contains the CGI scripts.
- **`config`**: Contains default configuration file.
- **`includes`**: Contains header files grouped by functionality (parsing, response, server, utils).
- **`srcs`**: Contains the corresponding source files for the headers.


## Requirements

- **Operating System:** FreeBSD or macOS (OS supporting Kqueue)
- **Compiler:** C++ compiler with C++17 support (e.g., g++, clang++)
- **Build Tools:** Make
- **Dependencies:** POSIX libraries for sockets, kqueue, etc.

## Installation and Build Instructions

### Building Locally

1. **Clone the Repository:**

   ```bash
   git clone https://github.com/yourusername/webserv.git
   cd webserv
   ```

2. **Build the Project:**

   Use the provided `Makefile`:

   ```bash
   make
   ```

3. **Run the Server:**

   ```bash
   ./build/webserv config/nginx.conf
   ```


## Configuration

The server reads its configuration from files located in the `config` directory (e.g., `nginx.conf`). The configuration files follow an Nginx-like syntax and define:
  
- **Server Settings:**  
  - Listening port.
  - Hostnames (for virtual hosting).
  - Custom error pages (e.g., mapping error codes like 404 to a file path).
  - Directory Listing
  
- **Location Settings:**  
  - URI mappings.
  - Document root for static files.
  - Default files (e.g., `index.html`).
  - Supported HTTP methods.
  - CGI extensions for dynamic content.
  - Directory Listing using autoindex
  - Return for redirections

Example configuration snippet:
```nginx
server {
	listen 80;
	client_max_body_size 100000;
	# change this path to check custom error pages (example uri: /pagenotfound)
	error_page 404 www/error_custom/404.html;
	error_page 201 www/error_custom/201.html;

	# fully static site
	location / {
		root www;
		autoindex on;
		index form.html;
		allow GET POST;
	}
	# redirect
	location /redirect {
		return /assets;
	}
	# uploads
	# big uploads
	# delete request
	location /my_files {
		root www/upload;
		allow POST DELETE;
	}
	# cgi:
	# cgi errors/infinite loop
	# post to cgi
	# cgi extension
	# get to cgi
	location /cgi {
		root www/cgi-bin;
		autoindex on;
		# index start.py;
		allow GET POST;
		cgi_extension .py;
	}
}

```

## Usage

Once built, run the server by specifying a configuration file:
```bash
./build/webserv config/nginx.conf
```

Your server will:
- Listen on the port specified in the configuration.
- Serve static files from the `static` directory.
- Execute CGI scripts located in `cgi-bin` when requested.
- Return custom error pages for error conditions (e.g., 404 Not Found).


## Testing

### Basic Testing

1. **Using a Web Browser:**
   - Navigate to `http://localhost:80/` to load the default page.
   - Try accessing various static files (e.g., `http://localhost:80/index.html`, `http://localhost:80/css/styles.css`).

2. **Using Curl:**
   ```bash
   curl -v http://localhost:80/
   curl -I http://localhost:8080/index.html
   ```

### Load Testing with Siege

To simulate concurrent users and stress test your server:
```bash
siege -c 50 -t 1M http://localhost:80/
```

### Testing CGI Functionality

Access a CGI script, for example:
```bash
http://localhost:8080/cgi-bin/environment.py
```
Ensure the script executes correctly and the output is returned.

## Acknowledgements

- A project at 42 coding school
- Inspired by the functionality of Nginx.
- Built with modern C++ and POSIX APIs.
- Utilizes kqueue for efficient event-driven I/O.

