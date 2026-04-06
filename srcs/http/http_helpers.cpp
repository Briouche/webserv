#include "webserv.hpp"
#include "HttpResponse.hpp"


std::string	toUpperCase(const std::string& str)
{
	std::string	result = str;

	for (size_t i = 0; i < result.size(); i++)
    	result[i] = toupper(result[i]);
	return (result);
}

std::string	defaultErrorPage(unsigned int status, std::string phrase)
{
	std::ostringstream html;
    html <<
        "<!DOCTYPE html>\n"
        "<html lang=\"fr\">\n"
        "<head>\n"
        "    <meta charset=\"UTF-8\">\n"
        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "    <title>" << status << " " << phrase << "</title>\n"
        "    <style>\n"
        "        body {\n"
        "            font-family: Arial, sans-serif;\n"
        "            text-align: center;\n"
        "            padding: 50px;\n"
        "            background-color: #f8f9fa;\n"
        "            color: #333;\n"
        "        }\n"
        "        h1 {\n"
        "            color: #dc3545;\n"
        "            font-size: 50px;\n"
        "        }\n"
        "        p {\n"
        "            font-size: 20px;\n"
        "        }\n"
        "        a {\n"
        "            color: #007bff;\n"
        "            text-decoration: none;\n"
        "        }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "    <h1>" << status << " " << phrase << "</h1>\n"
        "    <p>Une erreur est survenue.</p>\n"
        "    <p>Veuillez réessayer plus tard ou <a href=\"/\">retourner à l’accueil</a>.</p>\n"
        "</body>\n"
        "</html>\n";
    return (html.str());
}

HttpResponse&	returnResponse(HttpResponse& response, unsigned int status, std::string phrase, const ServerConfig& conf)
{
	std::string	body;

	if (status == 405)
		response.addHeader("allow", "GET, POST, DELETE");
	response.setStatus(status);
	response.setPhrase(phrase);

	if (!conf.error_pages.empty())
	{
		std::map<int, std::string>::const_iterator	it = conf.error_pages.find(status);
		if (it != conf.error_pages.end())
		{
			std::ifstream file(it->second.c_str());
			if (file.is_open())
			{
				std::ostringstream buff;
				buff << file.rdbuf();
				body = buff.str();
				file.close();
			}
		}
	}
	if (body.empty() && status != 204 && status != 200)
		body = defaultErrorPage(status, phrase);
	response.setBody(body);
	std::ostringstream	bodySize;
	bodySize << body.size();
	response.addHeader("content-length", bodySize.str());
	response.addHeader("content-type", "text/html");
	return (response);
}

std::string getMimeType(const std::string& filepath)
{
    std::map<std::string, std::string> mime_types;

    mime_types[".html"] = "text/html";
    mime_types[".htm"]  = "text/html";
    mime_types[".css"]  = "text/css";
    mime_types[".js"]   = "application/javascript";
    mime_types[".json"] = "application/json";
    mime_types[".png"]  = "image/png";
    mime_types[".jpg"]  = "image/jpeg";
    mime_types[".jpeg"] = "image/jpeg";
    mime_types[".gif"]  = "image/gif";
    mime_types[".ico"]  = "image/x-icon";
    mime_types[".txt"]  = "text/plain";
    mime_types[".pdf"]  = "application/pdf";

    size_t dot = filepath.rfind('.');
    if (dot == std::string::npos)
        return ("text/html");

    std::map<std::string, std::string>::const_iterator it;
    it = mime_types.find(filepath.substr(dot));
    if (it == mime_types.end())
        return ("text/html");
    return (it->second);
}

bool	isCgi(const std::string& filepath)
{
	size_t	i = filepath.rfind('.');

	if (i == std::string::npos)
		return (false);

	std::string extention = filepath.substr(i);
	if (extention == ".php" || extention == ".py")
		return (true);

	return (false);
}

std::string	getExtention(std::string& filepath, const LocationConfig* location)
{
	size_t	i = filepath.rfind('.');

	if (i == std::string::npos)
		return ("");

	std::string extention = filepath.substr(i);
	std::map<std::string, std::string>::const_iterator it = location->cgi_handlers.find(extention);
	if (it == location->cgi_handlers.end())
		return ("");
	return (it->second);
}

// get extention and check for permissions
std::string	validateCgi(httpInfo& httpinfo)
{
	struct stat fileinfo;
	if (stat(httpinfo.filepath.c_str(), &fileinfo) == -1)
	{
		returnResponse(httpinfo.res, 404, "Not Found", httpinfo.conf);
		return ("");
	}	
	if (!(fileinfo.st_mode & S_IXUSR)) //permission to execute?
	{
		returnResponse(httpinfo.res, 403, "Forbidden: can't execute", httpinfo.conf);
		return ("");
	}

	std::string extention = getExtention(httpinfo.filepath, httpinfo.loc); //python? php? other?
	if (extention == "")
	{
		returnResponse(httpinfo.res, 500, "Internal Server Error: cgi-extention not found", httpinfo.conf);
		return ("");
	}
	struct stat interpinfo;
	if (stat(extention.c_str(), &interpinfo) == -1) //is extention usable
	{
		returnResponse(httpinfo.res, 500, "Internal Server Error: cgi-extention unusable", httpinfo.conf);
		return ("");
	}
	return (extention);
}
