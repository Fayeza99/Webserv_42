#pragma once

#include "AResponseHandler.hpp"

// GET Requests
class StaticHandler : public AResponseHandler
{
public:
	StaticHandler(ClientState& client);
	~StaticHandler(void);

	void getResponse(void) const;

private:
	void setFilePath(void);
	std::string handleRedir(void) const;
	void listDir(void) const;
	bool autoIndex(void) const;
};
