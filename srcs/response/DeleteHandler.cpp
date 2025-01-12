#include "DeleteHandler.hpp"

DeleteHandler::DeleteHandler(ClientState& client) : AResponseHandler(client) {
	setFilePath();
}

DeleteHandler::~DeleteHandler(void) {}

void DeleteHandler::getResponse(void) const {}

void DeleteHandler::deleteFile(void) {
	if (_filePath.empty())
		return ;
	char realPath[PATH_MAX];
	if (realpath(_filePath.c_str(), realPath) == NULL)
		return ;
	std::remove(_filePath.c_str());
}

void DeleteHandler::setFilePath(void) {
	if (getUri() != _location.uri) {
		std::string uri = getUri().substr(_location.uri.length());
		_filePath = _location.document_root + "/" + uri;
	}
}
