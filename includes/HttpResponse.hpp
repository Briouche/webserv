#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include <iostream>
# include <string>
# include <map>
# include <vector>
# include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <cstdio>
# include "HttpRequest.hpp"
# include "config.hpp"

class client;

class HttpResponse
{
	private:
		std::string							_version;
		int									_status;
		std::string 						_phrase;
		std::map<std::string, std::string>  _headers;
		std::vector<std::string>            _setCookies;
		std::string							_body;

	public:
	    HttpResponse(void);
        HttpResponse(const HttpResponse& other);
        HttpResponse &operator=(const HttpResponse &other);
        ~HttpResponse();

        const std::string&                          getVersion() const;
		int			                        		getStatus() const;
        const std::string&                          getPhrase() const;
        const std::map<std::string, std::string>&   getHeaders() const;
		const std::string							getHeaderValue(std::string key) const;
		const std::vector<std::string>&             getSetCookies() const;
        const std::string&                          getBody() const;


        void    setVersion(const std::string& version);
		void    setStatus(const int status);
        void    setPhrase(const std::string& phrase);
        void    setBody(const std::string& body);
        void    addHeader(const std::string& key, const std::string& value);
		void    addSetCookie(const std::string& cookie);


        std::string    			buildResponse() const;
		static HttpResponse 	redirect(const std::string &url);
};

struct httpInfo
{
	const HttpRequest&		req;
	HttpResponse&			res;
	const ServerConfig&		conf;
	const LocationConfig*	loc;
	std::string				filepath;
	client*					cli;
	int						epollFd;
    std::map<int, int>*		cgiPipeToClientFd;

	httpInfo(const HttpRequest& req, HttpResponse& res, const ServerConfig& conf)
        : req(req), res(res), conf(conf), loc(NULL) {}
};


HttpResponse			handleRequest(HttpRequest& request, const ServerConfig &config, client* cli, int epollFd, std::map<int, int>* cgiPipeToClientFd);
HttpResponse&			returnResponse(HttpResponse& response, unsigned int status, std::string phrase, const ServerConfig& conf);
HttpResponse&			postHandler(httpInfo& httpInfo);
HttpResponse&			getHandler(httpInfo& httpInfo);
HttpResponse&   		deleteHandler(httpInfo& httpInfo);
std::string				getMimeType(const std::string& filepath);
std::string				getExtention(std::string& filepath, const LocationConfig* location);
HttpResponse&			execCgi(httpInfo& httpinfo, std::string& extention);
bool					isCgi(const std::string& filepath);
std::string				validateCgi(httpInfo& httpinfo);
HttpResponse&			parseCGIoutput(HttpResponse& response, std::string& output, httpInfo& httpInfo);
std::string				toLowerCase(const std::string& str);
std::string				toUpperCase(const std::string& str);

#endif