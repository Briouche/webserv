#include "HttpResponse.hpp"


HttpResponse::HttpResponse(void) : _version("HTTP/1.1"), _status(0), _phrase(""), _body("") {
}

HttpResponse::HttpResponse(const HttpResponse &other) : _version(other._version), _status(other._status),
									_phrase(other._phrase), _headers(other._headers), _setCookies(other._setCookies), _body(other._body) {
}

HttpResponse &HttpResponse::operator=(const HttpResponse &other)
{
    if (this == &other)
        return (*this);
    _version = other._version;
	_status = other._status;
    _phrase = other._phrase;
	_headers = other._headers;
	_setCookies = other._setCookies;
    _body = other._body;
    return (*this);
}

HttpResponse::~HttpResponse(void) {
}

const std::string&  HttpResponse::getVersion() const
{
    return (_version);
}

int  HttpResponse::getStatus(void) const
{
    return (_status);
}

const std::string&  HttpResponse::getPhrase() const
{
    return (_phrase);
}


const std::map<std::string, std::string>&   HttpResponse::getHeaders() const
{
    return (_headers);
}

const std::string	HttpResponse::getHeaderValue(std::string key) const
{
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	if (it == _headers.end())
		return ("");
	return (it->second);
}

const std::vector<std::string>& HttpResponse::getSetCookies() const
{
	return (_setCookies);
}

const std::string&  HttpResponse::getBody() const
{
    return (_body);
}

void    HttpResponse::setVersion(const std::string& version)
{
    _version = version;
}

void    HttpResponse::setStatus(int status)
{
    _status = status;
}

void    HttpResponse::setPhrase(const std::string& phrase)
{
    _phrase = phrase;
}


void    HttpResponse::setBody(const std::string& body)
{
    _body = body;
}

void    HttpResponse::addHeader(const std::string& key, const std::string& value)
{
    _headers[key] = value;
}

void HttpResponse::addSetCookie(const std::string& cookie)
{
	_setCookies.push_back(cookie);
}


std::string    HttpResponse::buildResponse() const
{
    std::ostringstream response;

    response << _version << " " << _status << " " << _phrase << "\r\n";
    for (std::map<std::string, std::string>::const_iterator i = _headers.begin(); i != _headers.end(); ++i)
    {
        response << i->first << ": " << i->second << "\r\n";
    }
    for (std::vector<std::string>::const_iterator i = _setCookies.begin(); i != _setCookies.end(); ++i)
    {
        response << "Set-Cookie: " << *i << "\r\n";
    }
    response << "\r\n";

    if (!_body.empty())
        response << _body;
    return response.str();
}

HttpResponse HttpResponse::redirect(const std::string &url)
{
    HttpResponse res;
    res.setStatus(302);               
    res.setPhrase("Found");          
    res.addHeader("Location", url);       
    res.addHeader("Content-Length", "0");  
    return res;
}

