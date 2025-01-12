#include "StaticHandler.hpp"

StaticHandler::StaticHandler(ClientState& client) : AResponseHandler(client) {
	setFilePath();
}

StaticHandler::~StaticHandler(void) {}

void StaticHandler::getResponse(void) const {
	std::ostringstream response;
	if (_location.redirect && getUri() != _location.redirect_uri) {
		response << handleRedir();
		// send
	}
	// other options
}//todo

void StaticHandler::setFilePath(void) {
	if (getUri() == _location.uri) {
		if (_location.default_files.size() > 0)
			_filePath = _location.document_root + "/" + _location.default_files[0];
	} else {
		std::string uri = getUri().substr(_location.uri.length());
		_filePath = _location.document_root + "/" + uri;
	}
}

bool StaticHandler::autoIndex(void) const {return _location.autoIndex;}

void StaticHandler::listDir(void) const {}//todo

std::string StaticHandler::handleRedir(void) const {
	std::ostringstream response;
	response << getHttpVersion()
			 << " 302 Found\r\n"
			 << "Location: " << _location.redirect_uri << "\r\n"
			 << "Connection: close\r\n\r\n";
	return response.str();
}