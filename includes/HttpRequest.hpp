#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <iostream>
# include <string>
# include <map>

class HttpRequest
{
    private:
        std::string                         _method;
        std::string                         _uri;
		std::string						 	_queryString;
        std::string                         _version;
        std::string                         _body;
        std::map<std::string, std::string>  _headers;
        std::map<std::string, std::string>  _cookies;
		std::string							_filepath;

	public:
        HttpRequest(void);
        HttpRequest(const HttpRequest& other);
        HttpRequest &operator=(const HttpRequest &other);
        ~HttpRequest();
		
        const std::string&                          getMethod() const;
        const std::string&                          getUri() const;
		const std::string&							getQuery() const;
        const std::string&                          getVersion() const;
        const std::map<std::string, std::string>&   getHeaders() const;
		const std::string							getHeaderValue(std::string key) const;
        const std::map<std::string, std::string>&   getCookies() const;
		const std::string							getCookieValue(std::string key) const;
        const std::string&                          getBody() const;
		const std::string&							getFilepath() const;


		void    setMethod(const std::string& method);
        void    setUri(const std::string& uri);
		void	setQuery(const std::string& queryString);
        void    setVersion(const std::string& version);
        void    setBody(const std::string& body);
        void    addHeader(const std::string& key, const std::string& value);
		void	addCookie(const std::string& key, const std::string& value);
		void 	setFilepath(const std::string &path);

        void    printRequest() const;
};

#endif
