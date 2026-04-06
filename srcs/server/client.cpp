#include "client.hpp"

client::client()
	: _fd(-1), _sessionId(""), _hasExistingSession(false), _headersParsed(false), _contentLengh(0), _writeOffset(0), _buffer(""), _responseBuffer("") {}

client::~client()
{
}

int client::getFd() const
{
	return _fd;
}

void client::setFd(int fd)
{
	_fd = fd;
}

bool client::isHeadersParsed() const
{
	return _headersParsed;
}

void client::setHeadersParsed(bool parsed)
{
	_headersParsed = parsed;
}

size_t client::getContentLengh() const
{
	return _contentLengh;
}

void client::setContentLengh(size_t contentLengh)
{
	_contentLengh = contentLengh;
}

size_t client::getWriteOffset() const
{
	return _writeOffset;
}

void client::setWriteOffset(size_t writeOffset)
{
	_writeOffset = writeOffset;
}

void client::addWriteOffset(size_t written)
{
	_writeOffset += written;
}

std::string &client::getBuffer()
{
	return _buffer;
}

void client::appendBuffer(const char *data, size_t size)
{
	_buffer.append(data, size);
}

std::string &client::getResponseBuffer()
{
	return _responseBuffer;
}

void client::setResponseBuffer(const std::string &responseBuffer)
{
	_responseBuffer = responseBuffer;
}

HttpRequest &client::getRequest()
{
	return _request;
}

HttpResponse &client::getResponse()
{
	return _response;
}

void client::setResponse(const HttpResponse &response)
{
	_response = response;
}

bool client::getChunkedFinished() const
{
	return _chunkedFinished;
}

void client::setChunkedFinished(bool value)
{
	_chunkedFinished = value;
}

std::string &client::getBody()
{
	return _body;
}

void client::setBody(const std::string &body)
{
	_body = body;
}

time_t client::getLastActive() const
{
	return _lastActive;
}

void client::setLastActive(time_t lastActive)
{
	_lastActive = lastActive;
}

bool client::getIsChunked() const
{
	return _isChunked;
}

void client::setIsChunked(bool isChunked)
{
	this->_isChunked = isChunked;
}

const std::string &client::getSessionId() const
{
	return _sessionId;
}

void client::setSessionId(const std::string &id)
{
	_sessionId = id;
}

bool client::getHasExistingSession() const
{
	return _hasExistingSession;
}

void client::setHasExistingSession(bool hasExisting)
{
	_hasExistingSession = hasExisting;
}

void client::reset()
{
	_buffer.clear();
	_body.clear();

	_headersParsed = false;
	_isChunked = false;
	_chunkedFinished = false;

	_contentLengh = 0;
	_writeOffset = 0;
	_lastActive = 0;
	_responseBuffer.clear();

	_request = HttpRequest();
	_response = HttpResponse();
}
