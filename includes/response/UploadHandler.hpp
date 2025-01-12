#pragma once

#include "AResponseHandler.hpp"

// POST Upload Requests, no html
class UploadHandler : public AResponseHandler
{
public:
	UploadHandler(ClientState& client);
	~UploadHandler(void);

	void getResponse(void) const;

private:
	std::string _boundary;
	std::vector<FileUpload> _uploads;

	void setFilePath(void);
	void parseBody(void);
	void setBoundary(void);
	void writeFiles(void);
};
