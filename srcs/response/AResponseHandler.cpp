#include "AResponseHandler.hpp"

std::string const& AResponseHandler::getRequest() const {return _request.getRequest();}
std::string const& AResponseHandler::getMethod() const {return _request.getMethod();}
std::string const& AResponseHandler::getUri() const {return _request.getUri();}
std::string const& AResponseHandler::getHttpVersion() const {return _request.getHttpVersion();}
std::string const& AResponseHandler::getBody() const {return _request.getBody();}
std::unordered_map<std::string, std::string> const& AResponseHandler::getHeaders() const {return _request.getHeaders();}
std::map<int, std::string> const& AResponseHandler::getErrorPages() const {return _client.serverConfig.error_pages;};
bool AResponseHandler::methodAllowed(const std::string& methodStr) const {
	bool isSupported = std::find(
		_location.supported_methods.begin(),
		_location.supported_methods.end(),
		getMethod()
	) != _location.supported_methods.end();
	return isSupported;
}

AResponseHandler::AResponseHandler(ClientState& client) : _client(client), _request(*client.request), _filePath(""), _location() {
	_location.getLocation(_client.serverConfig, _request.getUri());
}

AResponseHandler::~AResponseHandler(void) {}
