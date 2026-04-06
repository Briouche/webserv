/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stdevis <stdevis@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 15:28:09 by lecartuy          #+#    #+#             */
/*   Updated: 2026/03/20 11:26:48 by stdevis          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSING_HPP
#define PARSING_HPP

#include "config.hpp"

#include <iostream>
#include <fstream>    
#include <sstream>      
#include <string>       
#include <stdexcept>    
#include <cctype> 
#include <vector>
#include <cstdlib>


enum TokenType {
    WORD,
    NUMBER,
    LBRACE,
    RBRACE,
    SEMICOLON,
    TOKEN_END
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    Token(TokenType t, const std::string& v, int l)
        : type(t), value(v), line(l) {}
};


class FileLoader {
    private:
        std::string _filename;

        std::string readFile();
        std::string removeComments(const std::string& text);
        std::string normalizeWhitespace(const std::string& text);
        
    public :
        FileLoader(const std::string& filename);
        std::string load();
};

class Tokenizer {
    private:
        std::string _input;
        size_t _pos;
        int _line;

        char peek() const;
        char advance();
        void skipWhitespace();

        Token readWord();
        Token readNumber();
    
    public:
        Tokenizer(const std::string& input);
        std::vector<Token> tokenize();
};

class Parser 
{
	private:
		const std::vector<Token>& _tokens;
		size_t _pos;

		typedef void (Parser::*DirectiveHandler)(ServerConfig&);
		typedef void (Parser::*LocationDirectiveHandler)(LocationConfig&);

		std::map<std::string, DirectiveHandler> _directives;
		std::map<std::string, LocationDirectiveHandler> _locationDirectives;

		const Token& peek() const;
		const Token& advance();
		bool match(TokenType type);
		Token expect(TokenType type, const std::string& msg);

		ServerConfig parseServer();
		LocationConfig parseLocation();
		void parseDirective(ServerConfig& server);
		void parseLocationDirective(LocationConfig& loc);

	public:
		Parser(const std::vector<Token>& tokens);

		Config parse();

		// Server handlers
		void handleListen(ServerConfig& server);
		void handleServerName(ServerConfig& server);
		void handleRoot(ServerConfig& server);
		void handleHost(ServerConfig& server);
		void handleClientMaxBodySize(ServerConfig& server);
		void handleErrorPage(ServerConfig& server);

		// Location handlers
		void handleAutoIndex(LocationConfig& loc);
		void handleLocationRoot(LocationConfig& loc);
		void handleIndex(LocationConfig& loc);
		void handleUploadPath(LocationConfig& loc);
		void handleAllowedMethods(LocationConfig& loc);
		void handleCgiPass(LocationConfig& loc);
		void handleRedirect(LocationConfig& loc);
};

#endif