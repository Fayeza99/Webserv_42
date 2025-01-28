#include "ResponseControl.hpp"

ResponseControl::ResponseControl(ClientState &client) : _location(), _handler(nullptr), _client(client), _request(*client.request)
{
	// print_log(BLUE, "Response Constructor");
	_location.getLocation(_client.serverConfig, _request.getUri());
	setHandler();
}

ResponseControl::~ResponseControl(void)
{
	// print_log(BLUE, "Response Destructor");
	delete _handler;
}

bool ResponseControl::isCgiRequest(void)
{
	if (_location.cgi_ext.empty())
		return false;
	if (_request.getUri().find(_location.cgi_ext) != std::string::npos)
		return true;
	return false;
}

AResponseHandler *ResponseControl::getHandler(void) { return _handler; }

void ResponseControl::setHandler(void)
{
	if (_location.redirect == true)
		_handler = new StaticHandler(_client);
	else if (_request.getMethod() == "DELETE")
		_handler = new DeleteHandler(_client);
	else if (_request._isUpload)
		_handler = new UploadHandler(_client);
	else if (isCgiRequest())
		_handler = new CgiHandler(_client);
	else
		_handler = new StaticHandler(_client);
}

void ResponseControl::getResponse(void)
{
	_handler->getResponse();
}

ClientState &ResponseControl::getClient(void) {return _client;}
