#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

struct cgiState
{
	bool running;
	pid_t pid;
	int pipeIn;
	int pipeOut;
	std::string output;
	std::string bodyToWrite;
	size_t bodyWritten;
	time_t start;
	int serverFd;
};

class client
{
public:
	client();
	~client();

	cgiState cgi;

	int getFd() const;
	size_t getContentLengh() const;
	size_t getWriteOffset() const;
	std::string &getBuffer();
	std::string &getResponseBuffer();
	std::string &getBody();
	HttpRequest &getRequest();
	HttpResponse &getResponse();
	bool getChunkedFinished() const;
	time_t getLastActive() const;
	bool getIsChunked() const;
	const std::string &getSessionId() const;
	bool getHasExistingSession() const;

	void setFd(int fd);
	void setHeadersParsed(bool parsed);
	void setContentLengh(size_t contentLengh);
	void setWriteOffset(size_t writeOffset);
	void setResponseBuffer(const std::string &responseBuffer);
	void setResponse(const HttpResponse &response);
	void setChunkedFinished(bool value);
	void setBody(const std::string &body);
	void setLastActive(time_t lastActive);
	void setIsChunked(bool isChunked);
	void setSessionId(const std::string &id);
	void setHasExistingSession(bool hasExisting);

	void addWriteOffset(size_t written);
	void appendBuffer(const char *data, size_t size);
	bool isHeadersParsed() const;
	void reset();

private:
	int _fd;
	std::string _sessionId;
	bool _hasExistingSession;

	bool _headersParsed;
	bool _chunkedFinished;
	bool _isChunked;
	size_t _contentLengh;
	size_t _writeOffset;
	time_t _lastActive;
	std::string _buffer;
	std::string _responseBuffer;
	std::string _body;
	HttpRequest _request;
	HttpResponse _response;
};

#endif
