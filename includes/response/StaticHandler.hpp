#pragma once

#include "AResponseHandler.hpp"
#include "ErrorHandler.hpp"

// GET Requests
class StaticHandler : public AResponseHandler
{
public:
	StaticHandler(ClientState &client);
	~StaticHandler(void);

	void getResponse(void);

private:
	void setFilePath(void);
	std::string responseString(void) const;
	std::string handleRedir(void) const;
	std::string listDir(void) const;
	std::string listDirHtml(void) const;
	bool autoIndex(void) const;
};
