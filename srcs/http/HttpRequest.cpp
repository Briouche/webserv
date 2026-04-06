/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stdevis <stdevis@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/20 14:06:39 by stdevis           #+#    #+#             */
/*   Updated: 2026/03/27 14:41:32 by stdevis          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

HttpRequest::HttpRequest(void) : _method(""), _uri(""), _queryString(""), _version(""), _body("") {
}

HttpRequest::HttpRequest(const HttpRequest &other) : _method(other._method), _uri(other._uri), 
		_queryString(other._queryString), _version(other._version), _body(other._body),  _headers(other._headers), _cookies(other._cookies) {
}

HttpRequest &HttpRequest::operator=(const HttpRequest &other)
{
    if (this == &other)
        return (*this);
    _method = other._method;
    _uri = other._uri;
	_queryString = other._queryString;
    _version = other._version;
    _headers = other._headers;
    _cookies = other._cookies;
    _body = other._body;
    return (*this);
}

HttpRequest::~HttpRequest(void) {
}

const std::string&  HttpRequest::getMethod(void) const
{
    return (_method);
}

const std::string&  HttpRequest::getUri() const
{
    return (_uri);
}

const std::string&  HttpRequest::getQuery() const
{
    return (_queryString);
}

const std::string&  HttpRequest::getVersion() const
{
    return (_version);
}

const std::map<std::string, std::string>&   HttpRequest::getHeaders() const
{
    return (_headers);
}

const std::map<std::string, std::string>&   HttpRequest::getCookies() const
{
    return (_cookies);
}

const std::string	HttpRequest::getHeaderValue(std::string key) const
{
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	if (it == _headers.end())
		return ("");
	return (it->second);
}

const std::string	HttpRequest::getCookieValue(std::string key) const
{
	std::map<std::string, std::string>::const_iterator it = _cookies.find(key);
	if (it == _cookies.end())
		return ("");
	return (it->second);
}

const std::string&  HttpRequest::getBody() const
{
    return (_body);
}

void    HttpRequest::setMethod(const std::string& method)
{
    _method = method;
}

void    HttpRequest::setUri(const std::string& uri)
{
    _uri = uri;
}

void    HttpRequest::setQuery(const std::string& queryString)
{
    _queryString = queryString;
}

void    HttpRequest::setVersion(const std::string& version)
{
    _version = version;
}

void    HttpRequest::setBody(const std::string& body)
{
    _body = body;
}

void    HttpRequest::addHeader(const std::string& key, const std::string& value)
{
    _headers[key] = value;
}

void	HttpRequest::addCookie(const std::string& key, const std::string& value)
{
	_cookies[key] = value;
}

void HttpRequest::setFilepath(const std::string &path)
{
	_filepath = path;
}

const std::string &HttpRequest::getFilepath() const
{
	return (_filepath);
}

void    HttpRequest::printRequest() const
{
    std::cout << "Method: " << _method << std::endl;
    std::cout << "URI: " << _uri << std::endl;
    std::cout << "Version: " << _version << std::endl;
    std::cout << "Headers:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator i = _headers.begin(); i != _headers.end(); ++i)
        std::cout << "  " << i->first << ": " << i->second << std::endl;
    std::cout << "Body: " << (_body.empty() ? "(empty)" : _body) << std::endl;
}
