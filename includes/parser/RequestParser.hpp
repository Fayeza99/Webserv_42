#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <unordered_map>
#include <vector>

class FileUpload
{
	public:
		std::istringstream body_stream;
		std::unordered_map<std::string, std::string> headers;
		std::string content;
		std::string name;
		std::string filename;

		FileUpload(std::string body);
		FileUpload(const FileUpload& other);
		void set_name(void);
		void set_filename(void);
};

class RequestParser
{
	private:
		std::string _request;
		std::string method;
		std::string uri;
		std::string httpVersion;
		std::unordered_map<std::string, std::string> headers;
		std::string body;

		std::vector<FileUpload> _upload;
		std::string _boundary;

		void parseRequest(const std::string& request);

		void parse_headers(std::istringstream& request_stream, std::unordered_map<std::string, std::string>& headers);
		void parseUpload(void);
		void set_boundary(void);

	public:
		bool _isUpload;
		bool isValidMethod(const std::string& methodStr) const;
		bool isCgiRequest(void);
		std::string trim(const std::string& str);

		RequestParser(const std::string& request);
		~RequestParser();
		std::string const& getRequest() const;
		std::string const& getMethod() const;
		std::string const& getUri() const;
		std::string const& getHttpVersion() const;
		std::string const& getBody() const;
		std::unordered_map<std::string, std::string> const& getHeaders() const;
		std::vector<FileUpload> const& getUpload() const;
};
