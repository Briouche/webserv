#include "webserv.hpp"


static bool	splitOutput(std::string& headers, std::string& body, std::string& output)
{
	size_t endHead = output.find("\r\n\r\n");

    if (endHead == std::string::npos) {
    	endHead = output.find("\n\n");
	}
	if (endHead == std::string::npos)
		return (false);

    headers = output.substr(0, endHead);
	if (output[endHead] == '\r')
    	body = output.substr(endHead + 4);
	else
		body = output.substr(endHead + 2);
	return (true);
}

static bool	parseHeaders(HttpResponse& response, bool& statusFound, std::string& headers)
{
	size_t      len = headers.size();
    size_t      pos = 0;
    size_t      endline;
    std::string line;

    while (pos < len)
    {
		// find next line
        endline = headers.find("\r\n", pos);
		if (endline == std::string::npos)
			endline = headers.find("\n", pos);
        if (endline == std::string::npos)
            endline = len;
        line = headers.substr(pos, endline - pos);
        if (line.empty())
            break ;

		// sep key and value
        size_t colonPos = line.find(":");
        if (colonPos == std::string::npos)
            return (false);

        std::string key = toLowerCase(line.substr(0, colonPos));
		std::string value = line.substr(colonPos + 1);
        while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
            value.erase(0, 1);

		// check for status
		if (key == "status")
		{
			size_t space = value.find(' ');
            if (space != std::string::npos)
            {
                int code = std::atoi(value.substr(0, space).c_str());
                std::string phrase = value.substr(space + 1);
                response.setStatus(code);
                response.setPhrase(phrase);
            }
            else
            {
                response.setStatus(std::atoi(value.c_str()));
                response.setPhrase("");
            }
            statusFound = true;
		}
		else if (key == "set-cookie")
			response.addSetCookie(value);
        else
	        response.addHeader(key, value);
        pos = endline + (headers[endline] == '\r' ? 2 : 1);		// if the lines end with \r\n --> +2, if they end with \n --> +1
    }
	return (true);
}

HttpResponse&	parseCGIoutput(HttpResponse& response, std::string& output, httpInfo& info)
{
	// separate body and headers
	std::string	headers;
	std::string	body;
	bool		statusFound = false;

	if (!splitOutput(headers, body, output))
		return (returnResponse(info.res, 500, "Internal Server Error: cgi parsing", info.conf));

	// parse headers while keeping eye out for 'status: code phrase'
	if (!parseHeaders(response, statusFound, headers))
		return (returnResponse(info.res, 500, "Internal Server Error: cgi parsing 2.0", info.conf));

	// fill response (headers are already in response)
	response.setBody(body);

	std::ostringstream len;
	len << body.size();
	const std::map<std::string, std::string>&	h = response.getHeaders();
	std::map<std::string, std::string>::const_iterator it = h.find("content-length");
	if (it == h.end())									
		response.addHeader("content-length", len.str());

	if (!statusFound)
	{
		response.setStatus(200);
		response.setPhrase("OK");
	}
	return (response);
}
