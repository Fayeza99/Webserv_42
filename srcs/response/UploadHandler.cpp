#include "UploadHandler.hpp"

UploadHandler::UploadHandler(ClientState& client) : AResponseHandler(client), _boundary("") {
	setFilePath();
}

UploadHandler::~UploadHandler(void) {}

void UploadHandler::getResponse(void) const {}

void UploadHandler::parseBody(void) {}

void UploadHandler::writeFiles(void) {}

void UploadHandler::setBoundary(void) {
	auto it = getHeaders().find("Content-Type");
	if (it == getHeaders().end())
		return ;
	_boundary = it->second;
	_boundary = _boundary.substr(_boundary.find("boundary=") + 9);
	_boundary = _request.trim(_boundary);
}

void UploadHandler::setFilePath(void) {
	if (getUri() == _location.uri) {
		if (_location.default_files.size() < 1)
			_filePath = _location.document_root;
	} else {
		std::string uri = getUri().substr(_location.uri.length());
		_filePath = _location.document_root + "/" + uri;
	}
}
