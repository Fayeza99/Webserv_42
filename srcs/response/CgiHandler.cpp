#include "CgiHandler.hpp"

CgiHandler::CgiHandler(ClientState& client) : AResponseHandler(client) {
	setFilePath();
}

CgiHandler::~CgiHandler(void) {}

void CgiHandler::getResponse(void) const {}

void CgiHandler::setFilePath(void) {
	if (getUri() == _location.uri) {
		if (_location.default_files.size() > 0)
			_filePath = _location.document_root + "/" + _location.default_files[0];
	} else {
		std::string uri = getUri().substr(_location.uri.length());
		_filePath = _location.document_root + "/" + uri;
	}
}

void CgiHandler::executeCgi(void) {}
void CgiHandler::parentProcess(void) {}
void CgiHandler::childProcess(void) {}
bool CgiHandler::isFinished(void) {return false;}