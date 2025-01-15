#include "ResponseControl.hpp"

ResponseControl::ResponseControl(ClientState &client) : _location(), _handler(nullptr), _client(client), _request(*client.request)
{
	print_log(BLUE, "Response Constructor");
	_location.getLocation(_client.serverConfig, _request.getUri());
	setHandler();
}

ResponseControl::~ResponseControl(void)
{
	print_log(BLUE, "Response Destructor");
	delete _handler;
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
	else if (_request.isCgiRequest())
		_handler = new CgiHandler(_client);
	else
		_handler = new StaticHandler(_client);
}

void ResponseControl::getResponse(void)
{
	_handler->getResponse();
}
