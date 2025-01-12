#pragma once

#include "AResponseHandler.hpp"
#include "ErrorHandler.hpp"

class FileUpload
{
public:
	std::istringstream body_stream;
	std::unordered_map<std::string, std::string> headers;
	std::string content;
	std::string name;
	std::string filename;

	FileUpload(std::string body);
	FileUpload(const FileUpload &other);
	void set_name(void);
	void set_filename(void);
};

// POST Upload Requests, no html
class UploadHandler : public AResponseHandler
{
public:
	UploadHandler(ClientState &client);
	~UploadHandler(void);

	void getResponse(void);

private:
	std::string _boundary;
	std::vector<FileUpload> _uploads;

	void setFilePath(void);
	void parseBody(void);
	void setBoundary(void);
	bool writeFiles(void) const;
};
