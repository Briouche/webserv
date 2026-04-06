/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   directives.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lecartuy <lecartuy@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 15:35:17 by lecartuy          #+#    #+#             */
/*   Updated: 2026/04/06 11:30:26 by lecartuy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/parser.hpp"


// server directives


void Parser::handleListen(ServerConfig& server)
{
    Token port = expect(WORD, "listen expects a port number");

    for (size_t i = 0; i < port.value.size(); ++i)
    {
        if (!std::isdigit(port.value[i]))
            throw std::runtime_error("listen expects a numeric port");
    }

    int p = atoi(port.value.c_str());

    if (p <= 0 || p > 65535)
        throw std::runtime_error("Invalid port range");

    for (size_t i = 0; i < server.ports.size(); ++i)
    {
        if (server.ports[i] == p)
            throw std::runtime_error("Duplicate listen port");
    }

    server.ports.push_back(p);
}

void Parser::handleServerName(ServerConfig& server)
{
    Token name = expect(WORD, "server_name expects a value");
    server.server_name = name.value;
}

void Parser::handleRoot(ServerConfig& server)
{
    Token path = expect(WORD, "root expects a path");
    server.root = path.value;
}

static bool isValidIP(const std::string& s)
{
    int dots = 0;
    int octet = 0;
    bool hasDigit = false;

    for (size_t i = 0; i < s.size(); ++i)
    {
        char c = s[i];
        if (c == '.')
        {
            if (!hasDigit || octet > 255)
                return false;
            ++dots;
            octet = 0;
            hasDigit = false;
        }
        else if (isdigit((unsigned char)c))
        {
            octet = octet * 10 + (c - '0');
            hasDigit = true;
        }
        else
            return false;
    }
    return dots == 3 && hasDigit && octet <= 255;
}

void Parser::handleHost(ServerConfig& server)
{
    Token host = expect(WORD, "host expects an IP address");
    if (!isValidIP(host.value))
        throw std::runtime_error("Invalid host IP: " + host.value);
    server.host = host.value;
}

void Parser::handleClientMaxBodySize(ServerConfig& server)
{
    Token tok = expect(WORD, "client_max_body_size expects a number");

    for (size_t i = 0; i < tok.value.size(); ++i)
    {
        if (!std::isdigit(tok.value[i]))
            throw std::runtime_error("client_max_body_size must be numeric");
    }

    int size = atoi(tok.value.c_str());
    if (size <= 0)
        throw std::runtime_error("client_max_body_size must be positive");

    server.client_max_body_size = size;
}

void Parser::handleErrorPage(ServerConfig& server)
{
    Token code = expect(WORD, "error_page expects an error code");
    Token path = expect(WORD, "error_page expects a path");

    for (size_t i = 0; i < code.value.size(); ++i)
    {
        if (!std::isdigit(code.value[i]))
            throw std::runtime_error("error_page expects a numeric error code");
    }

    int c = atoi(code.value.c_str());
    if (c < 400 || c > 599)
        throw std::runtime_error("Invalid HTTP error code");

    server.error_pages[c] = path.value;
}


// Location directives


void Parser::handleAutoIndex(LocationConfig& loc)
{
    Token val = expect(WORD, "autoindex expects 'on' or 'off'");

    if (val.value == "on")
        loc.autoindex = true;
    else if (val.value == "off")
        loc.autoindex = false;
    else
        throw std::runtime_error("autoindex must be 'on' or 'off'");
}

void Parser::handleLocationRoot(LocationConfig& loc)
{
    Token path = expect(WORD, "root expects a path");
    loc.root = path.value;
}

void Parser::handleIndex(LocationConfig& loc)
{
    Token file = expect(WORD, "index expects a filename");
    loc.index = file.value;
}

void Parser::handleUploadPath(LocationConfig& loc)
{
    Token path = expect(WORD, "upload_path expects a path");
    loc.upload_path = path.value;
}

void Parser::handleAllowedMethods(LocationConfig& loc)
{
    while (peek().type == WORD)
    {
        Token method = expect(WORD, "Expected HTTP method");

        if (method.value != "GET" &&
            method.value != "POST" &&
            method.value != "DELETE")
            throw std::runtime_error("Invalid HTTP method: " + method.value);

        loc.allowed_methods.push_back(method.value);
    }

    if (loc.allowed_methods.empty())
        throw std::runtime_error("allowed_methods requires at least one method");
}

void Parser::handleCgiPass(LocationConfig& loc)
{
    Token ext = expect(WORD, "cgi_pass expects an extension");
    Token path = expect(WORD, "cgi_pass expects a CGI executable path");

    if (ext.value[0] != '.')
        throw std::runtime_error("CGI extension must start with '.'");

    loc.cgi_handlers[ext.value] = path.value;
}

void Parser::handleRedirect(LocationConfig& loc)
{
    Token code = expect(WORD, "redirect expects a status code");
    Token url = expect(WORD, "redirect expects a URL");

    for (size_t i = 0; i < code.value.size(); ++i)
    {
        if (!std::isdigit(code.value[i]))
            throw std::runtime_error("redirect status code must be a number");
    }

    int status = atoi(code.value.c_str());

    if (status < 300 || status > 399)
        throw std::runtime_error("redirect code must be between 300 and 399");

    loc.has_redirect = true;
    loc.redirect_code = status;
    loc.redirect_url = url.value;
}