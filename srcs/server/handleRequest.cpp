
#include "HttpResponse.hpp"
#include "config.hpp"
#include "webserv.hpp"
#include <algorithm>

const LocationConfig *getLocation(const ServerConfig &config, const std::string &uri)
{
	const LocationConfig *best = NULL;
	size_t best_len = 0;

	for (std::vector<LocationConfig>::const_iterator it = config.locations.begin();
		 it != config.locations.end(); ++it)
	{
		if (uri.find(it->path) == 0)
		{
			size_t len = it->path.size();
			if (uri.size() == len || uri[len] == '/' || it->path == "/")
			{
				if (len > best_len)
				{
					best_len = len;
					best = &(*it);
				}
			}
		}
	}
	return (best);
}

// Early validation of basic HTTP protocol (before auth check)
// Returns true if valid, false if error (and sets error response)
bool earlyValidateRequest(HttpRequest &request, HttpResponse &response, const ServerConfig &conf)
{
	// Check HTTP version
	if (request.getVersion() != "HTTP/1.1")
	{
		returnResponse(response, 505, "Version not supported", conf);
		return false;
	}

	// Check Host header
	const std::map<std::string, std::string> &headers = request.getHeaders();
	std::map<std::string, std::string>::const_iterator it = headers.find("host");
	if (it == headers.end())
	{
		returnResponse(response, 400, "Bad Request", conf);
		return false;
	}

	// Check path traversal
	std::string uri = request.getUri();
	if (uri.empty() || uri[0] != '/')
	{
		returnResponse(response, 400, "Bad Request", conf);
		return false;
	}
	else if (uri.find("/..") != std::string::npos)
	{
		returnResponse(response, 403, "Forbidden: no traversal '/../'", conf);
		return false;
	}

	return true;
}

HttpResponse &verify_request(HttpRequest &request, httpInfo &info)
{
	if (request.getVersion() != "HTTP/1.1")
		return (returnResponse(info.res, 505, "Version not supported", info.conf));

	// headers
	const std::map<std::string, std::string> &headers = request.getHeaders();
	std::map<std::string, std::string>::const_iterator it = headers.find("host");
	if (it == headers.end())
		return (returnResponse(info.res, 400, "Bad Request", info.conf));
	bool isChunked = false;
	it = headers.find("transfer-encoding");
	if (it != headers.end() && it->second == "chunked")
	{
		isChunked = true;
	}

	it = headers.find("content-length");
	if (!request.getBody().empty() && !isChunked)
	{
		if (request.getBody().size() > info.conf.client_max_body_size)
			return (returnResponse(info.res, 413, "Request entity too large", info.conf));
		if (it == headers.end())
			return (returnResponse(info.res, 411, "Length Required", info.conf));
		if (request.getBody().size() > info.conf.client_max_body_size)
			return (returnResponse(info.res, 413, "Request entity too large", info.conf));
		if (it == headers.end())
			return (returnResponse(info.res, 411, "Length Required", info.conf));
		else
		{
			char *endptr;
			unsigned long contentLength = strtoul(it->second.c_str(), &endptr, 10);
			if (*endptr != '\0' || contentLength != request.getBody().size())
				return (returnResponse(info.res, 400, "Bad Request: content-length incorrect", info.conf));
		}
	}

	// uri
	std::string uri = request.getUri();
	if (uri.empty() || uri[0] != '/')
		return (returnResponse(info.res, 400, "Bad Request", info.conf));
	else if (uri.find("/..") != std::string::npos)
		return (returnResponse(info.res, 403, "Forbidden: no traversal '/../'", info.conf));
	size_t i = uri.find("?");
	if (i != std::string::npos)
	{
		request.setUri(uri.substr(0, i));
		request.setQuery(uri.substr(i + 1));
	}

	// method
	info.loc = getLocation(info.conf, request.getUri());
	if (!info.loc)
		return (returnResponse(info.res, 404, "Not Found: location not found", info.conf));
	std::vector<std::string> allowed_methods = info.loc->allowed_methods;
	if (std::find(allowed_methods.begin(), allowed_methods.end(), request.getMethod()) == allowed_methods.end()) // if no methods --> nothing allowed (to invert, just add !empty check)
		return (returnResponse(info.res, 405, "Method Not Allowed", info.conf));

	return (info.res);
}

HttpResponse handleRequest(HttpRequest &request, const ServerConfig &config, client *cli, int epollFd, std::map<int, int> *cgiPipeToClientFd)
{
	HttpResponse response;
	httpInfo httpinfo(request, response, config);

	if (verify_request(request, httpinfo).getStatus() != 0)
		return (response);

	// redirect when needed
	if (httpinfo.loc && httpinfo.loc->has_redirect)
	{
		response.setStatus(httpinfo.loc->redirect_code);
		response.setPhrase("Moved Permanently");
		response.addHeader("Location", httpinfo.loc->redirect_url);
		response.addHeader("Content-Length", "0");
		return (response);
	}

	httpinfo.filepath = config.root + request.getUri();
	request.setFilepath(httpinfo.filepath);
	httpinfo.cli = cli;
	httpinfo.cgiPipeToClientFd = cgiPipeToClientFd;
	httpinfo.epollFd = epollFd;

	if (request.getMethod() == "GET")
		return (getHandler(httpinfo));

	else if (request.getMethod() == "DELETE")
		return (deleteHandler(httpinfo));

	else if (request.getMethod() == "POST")
		return (postHandler(httpinfo));

	else
		return (returnResponse(httpinfo.res, 405, "Method not implemented... yet", httpinfo.conf));
}
