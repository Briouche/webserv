/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lecartuy <lecartuy@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 17:54:17 by lecartuy          #+#    #+#             */
/*   Updated: 2026/03/04 11:54:11 by lecartuy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/parser.hpp"

Tokenizer::Tokenizer(const std::string& input) : _input(input), _pos(0), _line(1) {}

char Tokenizer::peek() const {
    if (_pos >= _input.size())
        return '\0';
    return _input[_pos];
}

char Tokenizer::advance() {
    if (_pos >= _input.size())
        return '\0';

    char c = _input[_pos++];
    if (c == '\n')
        _line++;
    return c;
}

void Tokenizer::skipWhitespace() {
    while (_pos < _input.size() && std::isspace(peek()))
        advance();
}

Token Tokenizer::readWord()
{
    std::string value;
    int startLine = _line;

    while (_pos < _input.size() &&
           !std::isspace(static_cast<unsigned char>(peek())) &&
           peek() != '{' &&
           peek() != '}' &&
           peek() != ';')
    {
        value += advance();
    }
    return Token(WORD, value, startLine);
}

Token Tokenizer::readNumber() {
    std::string value;
    int startLine = _line;

    while (_pos < _input.size() && std::isdigit(peek()))
        value += advance();

    return Token(NUMBER, value, startLine);
}

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;

    while (_pos < _input.size()) {

        skipWhitespace();
        if (_pos >= _input.size())
            break;

        char c = peek();

        if (c == '{') {
            tokens.push_back(Token(LBRACE, "{", _line));
            advance();
        }
        else if (c == '}') {
            tokens.push_back(Token(RBRACE, "}", _line));
            advance();
        }
        else if (c == ';') {
            tokens.push_back(Token(SEMICOLON, ";", _line));
            advance();
        }
        else {
            tokens.push_back(readWord());
        }
    }

    tokens.push_back(Token(TOKEN_END, "", _line));
    return tokens;
}

