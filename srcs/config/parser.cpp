/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lecartuy <lecartuy@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/24 19:48:10 by lecartuy          #+#    #+#             */
/*   Updated: 2026/03/17 12:38:31 by lecartuy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/parser.hpp"

Parser::Parser(const std::vector<Token>& tokens) : _tokens(tokens), _pos(0) {
    
    // server directives 
    
    _directives["listen"] = &Parser::handleListen;
    _directives["root"]   = &Parser::handleRoot;
    _directives["server_name"] = &Parser::handleServerName;
    _directives["host"] = &Parser::handleHost;
    _directives["client_max_body_size"] = &Parser::handleClientMaxBodySize;
    _directives["error_page"] = &Parser::handleErrorPage;
    
    // location directives
    
    _locationDirectives["autoindex"] = &Parser::handleAutoIndex;
    _locationDirectives["root"] = &Parser::handleLocationRoot;
    _locationDirectives["index"] = &Parser::handleIndex;
    _locationDirectives["upload_path"] = &Parser::handleUploadPath;
    _locationDirectives["allowed_methods"] = &Parser::handleAllowedMethods;
    _locationDirectives["cgi_pass"] = &Parser::handleCgiPass;
    _locationDirectives["redirect"] = &Parser::handleRedirect;
    
}

const Token& Parser::peek() const {
    return _tokens[_pos];
}

const Token& Parser::advance() {
    return _tokens[_pos++];
}

bool Parser::match(TokenType type) {
    if (peek().type == type) {
        _pos++;
        return true;
    }
    return false;
}

Token Parser::expect(TokenType type, const std::string& msg)
{
    if (peek().type != type)
        throw std::runtime_error(msg);
    return _tokens[_pos++];
}

//...........................................................................

Config Parser::parse() 
{
    Config config;

    while (peek().type != TOKEN_END) {
        config.servers.push_back(parseServer());
    }
    return config;
}

ServerConfig Parser::parseServer()
{
    Token keyword = expect(WORD, "Expected 'server'");
    if (keyword.value != "server")
        throw std::runtime_error("Expected 'server' keyword");

    expect(LBRACE, "Expected '{' after server");

    ServerConfig server;

    while (!match(RBRACE))
    {
        if (peek().type == WORD && peek().value == "location")
            server.locations.push_back(parseLocation());
        else
            parseDirective(server);
    }
    return server;
}

void Parser::parseDirective(ServerConfig& server)
{
    Token name = expect(WORD, "Expected directive name");

    std::map<std::string, DirectiveHandler>::iterator it =
        _directives.find(name.value);

    if (it == _directives.end())
        throw std::runtime_error("Unknown directive: " + name.value);

    DirectiveHandler handler = it->second;
    (this->*handler)(server);

    expect(SEMICOLON, "Missing ';' after directive");
}

LocationConfig Parser::parseLocation()
{
    expect(WORD, "Expected 'location'");

    Token path = expect(WORD, "Expected location path");

    LocationConfig loc;
    loc.path = path.value;

    expect(LBRACE, "Expected '{' after location path");

    while (!match(RBRACE))
        parseLocationDirective(loc);

    return loc;
}

void Parser::parseLocationDirective(LocationConfig& loc)
{
    Token name = expect(WORD, "Expected location directive name");

    std::map<std::string, LocationDirectiveHandler>::iterator it =
        _locationDirectives.find(name.value);

    if (it == _locationDirectives.end())
        throw std::runtime_error("Unknown location directive: " + name.value);

    LocationDirectiveHandler handler = it->second;
    (this->*handler)(loc);

    expect(SEMICOLON, "Missing ';' after location directive");
}
