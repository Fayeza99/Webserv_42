#include "DeleteHandler.hpp"

DeleteHandler::DeleteHandler(ClientState &client) : AResponseHandler(client)
{
	// print_log(BLUE, "DeleteHandler Constructor");
	setFilePath();
}

DeleteHandler::~DeleteHandler(void)
{
	// print_log(BLUE, "DeleteHandler Destructor");
}

void DeleteHandler::getResponse(void)
{
	int code = deleteFile();
	if (code == 204)
		_client.responseBuffer = getHttpVersion() + " 204 No Content\r\n\r\n";
	else
		_client.responseBuffer = ErrorHandler::createResponse(code, getErrorPages());
	KqueueManager::registerEvent(_client.fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR);
}

int DeleteHandler::deleteFile(void) const
{
	if (_filePath.empty())
		return 404;
	char realPath[PATH_MAX];
	if (realpath(_filePath.c_str(), realPath) == NULL)
		return 404;
	if (!std::remove(_filePath.c_str()))
		return 204;
	return 500;
}

void DeleteHandler::setFilePath(void)
{
	if (getUri() != _location.uri)
	{
		std::string uri = getUri().substr(_location.uri.length());
		_filePath = _location.document_root + "/" + uri;
	}
}
