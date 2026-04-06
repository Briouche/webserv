#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <sys/types.h>
#include <dirent.h>

std::string	generateAutoindex(std::string& filepath, const std::string& uri)
{
	DIR*			dir = opendir(filepath.c_str());
	struct dirent*	entry;
	std::string		name;
	std::string		html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Index of " + uri + "</title>"
        "<style>"
        "body { font-family: Arial, sans-serif; background: #1e1e1e; color: #f0f0f0; padding: 40px; }"
        "h1 { color: #6cf; }"
        "a { color: #6cf; text-decoration: none; display: block; margin: 8px 0; }"
        "a:hover { text-decoration: underline; }"
        "</style></head><body>"
        "<h1>Index of " + uri + "</h1>";

	while ((entry = readdir(dir)) != 0)
	{
		name = entry->d_name;
		if (name[0] == '.')
			continue;
		std::string fullPath = filepath + "/" + name;
        struct stat fileinfo;
        if (stat(fullPath.c_str(), &fileinfo) == -1)
            continue ;

        if (S_ISDIR(fileinfo.st_mode))
            name += "/";

        html += "<a href='" + name + "'>" + name + "</a>";
	}
	closedir(dir);
	return (html);
}

HttpResponse&	getAutoindex(httpInfo& httpinfo)
{
	std::string body = generateAutoindex(httpinfo.filepath, httpinfo.req.getUri());
    if (body.empty())
        return (returnResponse(httpinfo.res, 500, "Internal Server Error: autoindex failed", httpinfo.conf));
    httpinfo.res.setStatus(200);
    httpinfo.res.setPhrase("OK");
    httpinfo.res.setBody(body);
    std::ostringstream ss;
    ss << body.size();
    httpinfo.res.addHeader("content-type", "text/html");
    httpinfo.res.addHeader("content-length", ss.str());
	return (httpinfo.res);
}

HttpResponse&	findFile(httpInfo& httpinfo)
{
	//test root + uri
	struct stat	fileinfo;
	if (stat(httpinfo.filepath.c_str(), &fileinfo) == -1)
		return (returnResponse(httpinfo.res, 404, "Not Found", httpinfo.conf));

	//is it a directory?
	else if (S_ISDIR(fileinfo.st_mode))
	{
		if (httpinfo.filepath[httpinfo.filepath.size() - 1] != '/')
		{
			httpinfo.res.setStatus(301);
			httpinfo.res.setPhrase("Moved Permanently");
			httpinfo.res.addHeader("Location", httpinfo.req.getUri() + "/");
			return (httpinfo.res);
		}
		//try index file 
		std::string	indexPath = httpinfo.filepath + httpinfo.loc->index;
		if (stat(indexPath.c_str(), &fileinfo) == 0 && S_ISREG(fileinfo.st_mode))
			httpinfo.filepath = indexPath;
		else if (httpinfo.loc->autoindex)
			return (getAutoindex(httpinfo)); //if autoindex is on, create autoindex
		else
			return (returnResponse(httpinfo.res, 403, "Forbidden: directory listing disabled", httpinfo.conf));
	}
	//is user authorised to read file?
	if (access(httpinfo.filepath.c_str(), R_OK) == -1)
		return (returnResponse(httpinfo.res, 403, "Forbidden: unauthorized to read file", httpinfo.conf));
	return (httpinfo.res);
}

std::string	readFile(const std::string& path)
{
	std::ifstream		file(path.c_str(), std::ios::in | std::ios::binary);
	std::ostringstream	ss;

	if (!file)
		return ("");

	ss << file.rdbuf();
	return (ss.str());
}

HttpResponse&	getHandler(httpInfo& httpinfo)
{
	if (isCgi(httpinfo.filepath))
	{
		std::string extention = validateCgi(httpinfo);				// get extention and check if its usable
		if (extention.empty())
			return (httpinfo.res);
		return (execCgi(httpinfo, extention));
	}
	else if (findFile(httpinfo).getStatus() != 0)
		return (httpinfo.res);

	httpinfo.res.setBody(readFile(httpinfo.filepath));
	std::ostringstream	bodySize;
	bodySize << httpinfo.res.getBody().size();
	httpinfo.res.addHeader("content-length", bodySize.str());
	httpinfo.res.addHeader("content-type", getMimeType(httpinfo.filepath));
	httpinfo.res.setPhrase("OK");
	httpinfo.res.setStatus(200);

	return (httpinfo.res);
}
