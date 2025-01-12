#pragma once

#include "AResponseHandler.hpp"
#include "ErrorHandler.hpp"

// DELETE Requests, no html
class DeleteHandler : public AResponseHandler
{
public:
	DeleteHandler(ClientState &client);
	~DeleteHandler(void);

	void getResponse(void);

private:
	void setFilePath(void);
	int deleteFile(void) const;
};
