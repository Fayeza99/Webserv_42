#include "UploadHandler.hpp"
#include "RequestParser.hpp"
#include "utils.hpp"

UploadHandler::UploadHandler(ClientState &client) : AResponseHandler(client), _boundary("")
{
	// print_log(BLUE, "UploadHandler Constructor");
	setFilePath();
	setBoundary();
	parseBody();
}

UploadHandler::~UploadHandler(void)
{
	// print_log(BLUE, "UploadHandler Destructor");
}

void UploadHandler::getResponse(void)
{
	std::ostringstream response;
	if (!methodAllowed())
		_client.responseBuffer = ErrorHandler::createResponse(405, getErrorPages());
	else if (!RequestParser::isDirectory(_filePath))
		_client.responseBuffer = ErrorHandler::createResponse(404, getErrorPages());
	else if (writeFiles())
	{
		response << getHttpVersion() << " 201 Created\r\n";
		if (getErrorPages().find(201) != getErrorPages().end())
			response << ErrorHandler::createResponse(201, getErrorPages());
		else
			response << "Content-Length: 0\r\nConnection: close\r\n\r\n";
		_client.responseBuffer = response.str();
	}
	else
	{
		print_log(RED, "[ERROR] handleUpload");
		_client.responseBuffer = ErrorHandler::createResponse(500, getErrorPages());
	}
	KqueueManager::registerEvent(_client.fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR);
}

void UploadHandler::parseBody(void)
{
	std::string line;
	std::string b(getBody());
	std::vector<std::string> parts;
	while (!b.empty())
	{
		size_t next = b.find(_boundary);
		parts.push_back(b.substr(0, next));
		if (next != std::string::npos)
			b = b.substr(next + _boundary.length());
		else
			b = "";
	}
	for (std::string p : parts)
	{
		p = RequestParser::trim(p);
		if (p == "--")
			continue;
		FileUpload upload(p);
		RequestParser::parseHeaders(upload.body_stream, upload.headers);
		upload.set_name();
		upload.set_filename();
		while (std::getline(upload.body_stream, line))
			upload.content += line + '\n';
		size_t pos = upload.content.find_last_of("--");
		if (pos != std::string::npos)
			upload.content = upload.content.substr(0, pos - 3);
		_uploads.push_back(upload);
	}
}

bool UploadHandler::writeFiles(void) const
{
	bool success = false;
	for (const FileUpload &file : _uploads)
	{
		if (file.filename.empty())
			continue;
		std::string path = _filePath + "/" + file.filename;
		std::ofstream outfile(path);
		outfile << file.content;
		outfile.close();
		success = true;
	}
	return success;
}

void UploadHandler::setBoundary(void)
{
	auto it = getHeaders().find("Content-Type");
	if (it == getHeaders().end())
		return;
	_boundary = it->second;
	_boundary = _boundary.substr(_boundary.find("boundary=") + 9);
	_boundary = _request.trim(_boundary);
}

void UploadHandler::setFilePath(void)
{
	if (getUri() == _location.uri)
	{
		if (_location.default_files.size() < 1)
			_filePath = _location.document_root;
	}
	else
	{
		std::string uri = getUri().substr(_location.uri.length());
		_filePath = _location.document_root + "/" + uri;
	}
}

// FileUpload

FileUpload::FileUpload(std::string body) : body_stream(body) {}

FileUpload::FileUpload(const FileUpload &other)
{
	body_stream = std::istringstream(other.body_stream.str());
	content = other.content;
	name = other.name;
	filename = other.filename;
}

void FileUpload::set_name(void)
{
	name = "";
	size_t pos = headers["Content-Disposition"].find(" name=\"");
	if (pos != std::string::npos)
	{
		name = headers["Content-Disposition"].substr(pos + 7);
		name = name.substr(0, name.find("\""));
	}
}

void FileUpload::set_filename(void)
{
	filename = "";
	size_t pos = headers["Content-Disposition"].find(" filename=\"");
	if (pos != std::string::npos)
	{
		filename = headers["Content-Disposition"].substr(pos + 11);
		filename = filename.substr(0, filename.find("\""));
	}
}
