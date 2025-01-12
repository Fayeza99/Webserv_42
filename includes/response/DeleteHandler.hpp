#pragma once

#include "AResponseHandler.hpp"

// DELETE Requests, no html
class DeleteHandler : public AResponseHandler
{
public:
	DeleteHandler(ClientState& client);
	~DeleteHandler(void);

	void getResponse(void) const;

private:
	void setFilePath(void);
	void deleteFile(void);
};
