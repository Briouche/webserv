#include "server.hpp"
#include <cstdlib>

std::string	toLowerCase(const std::string& str)
{
	std::string	result = str;

	for (size_t i = 0; i < result.size(); i++)
    	result[i] = tolower(result[i]);
	return (result);
}

std::string	decode(std::string uri)
{
	std::string	decoded;
	size_t		len = uri.size();
	size_t		i = 0;

	while (i < len)
	{
		if (uri[i] == '%' && i + 2 < len)
		{
			std::string	hex = uri.substr(i + 1, 2);
			char c = (char)strtol(hex.c_str(), NULL, 16);
			decoded += c;
			i += 2;
		}
		else
			decoded += uri[i];
		i++;
	}
	return (decoded);
}

static std::string trim(const std::string& s)
{
	size_t start = 0;
	while (start < s.size() && (s[start] == ' ' || s[start] == '\t'))
		start++;
	size_t end = s.size();
	while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t'))
		end--;
	return s.substr(start, end - start);
}

static void parseCookieHeader(const std::string& value, HttpRequest& request)
{
	// Cookie: a=1; b=hello
	size_t pos = 0;
	while (pos < value.size())
	{
		size_t semi = value.find(';', pos);
		std::string part = (semi == std::string::npos) ? value.substr(pos) : value.substr(pos, semi - pos);
		part = trim(part);
		if (!part.empty())
		{
			size_t eq = part.find('=');
			if (eq != std::string::npos && eq != 0)
			{
				std::string name = trim(part.substr(0, eq));
				std::string val = trim(part.substr(eq + 1));
				if (!name.empty())
					request.addCookie(name, val);
			}
		}
		if (semi == std::string::npos)
			break;
		pos = semi + 1;
	}
}

static int parseFirstLine(std::string& line, HttpRequest& request)
{
    size_t firstSpace = line.find(" ");
    if (firstSpace == std::string::npos)
        return (1);
    size_t secondSpace = line.find(" ", firstSpace + 1);
    if (secondSpace == std::string::npos)
        return (2);

    request.setMethod(line.substr(0, firstSpace));
    request.setUri(decode(line.substr(firstSpace + 1, secondSpace - firstSpace - 1)));

    std::string version = line.substr(secondSpace + 1);
    if (!version.empty() && version[version.size() - 1] == '\r')
        version.erase(version.size() - 1);
    request.setVersion(version);
    return (0);
}

static int  parseHeaders(std::string& headers, HttpRequest& request)
{
    size_t      len = headers.size();
    size_t      pos = 0;
    size_t      endline;
    std::string line;

    while (pos < len)
    {
        endline = headers.find("\r\n", pos);
        if (endline == std::string::npos)
            return (1);
        line = headers.substr(pos, endline - pos);
        if (line.empty())
            break ;

        size_t colonPos = line.find(":");
        if (colonPos == std::string::npos)
            return (1);

        std::string key = toLowerCase(line.substr(0, colonPos));
        std::string value = line.substr(colonPos + 1);
        while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
            value.erase(0, 1);
		if (key == "cookie")
			parseCookieHeader(value, request);
        request.addHeader(key, value);
        pos = endline + 2;
    }
    return (0);
}

int	parseRequestBodyContentLength(client *cli)
{
	size_t bodyPos = cli->getBuffer().find("\r\n\r\n");
	if (bodyPos == std::string::npos)
		return (0);
	size_t bodyStart = bodyPos + 4;
	size_t bodyReceived = cli->getBuffer().size() - bodyStart;

	if (bodyReceived < cli->getContentLengh())
		return 0;
	std::string body = cli->getBuffer().substr(bodyStart, cli->getContentLengh());
	cli->getRequest().setBody(body);
	return 1;
}

int parseRequestBodyChunked(client *cli)
{
	size_t bodyStart = cli->getBuffer().find("\r\n\r\n") + 4;
	if (bodyStart == std::string::npos)
		return 0;

	while(true)
	{
		size_t lineEnd = cli->getBuffer().find("\r\n", bodyStart);
		if (lineEnd == std::string::npos)
			return 0;
		std::string sizeStr = cli->getBuffer().substr(bodyStart, lineEnd - bodyStart);
		size_t chunkSize = std::strtoul(sizeStr.c_str(), NULL, 16);
		if (chunkSize == 0)
		{
			if (cli->getBuffer().size() < lineEnd + 4)
				return 0;
			cli->getRequest().setBody(cli->getBody());
			cli->setChunkedFinished(true);
			return 1;
		}
		size_t dataStart = lineEnd + 2;
		size_t dataEnd = dataStart + chunkSize;
		if (cli->getBuffer().size() < dataEnd + 2)
			return 0;
		cli->getBody().append(cli->getBuffer().substr(dataStart, chunkSize));

		bodyStart = dataEnd + 2;
	}
	return 0;
}

int parseRequestHeaders(const std::string& rawRequest, HttpRequest& request)
{
   size_t endHead = rawRequest.find("\r\n\r\n");
    if (endHead == std::string::npos)
        return (1);

    std::string head = rawRequest.substr(0, endHead + 2);

    size_t endFirstLine = head.find("\r\n");
    std::string firstLine = head.substr(0, endFirstLine);
    if (parseFirstLine(firstLine, request) != 0)
        return (2);
    std::string headers = head.substr(endFirstLine + 2);
    if (parseHeaders(headers, request) != 0)
        return (3);

    return (0);
}

