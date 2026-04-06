/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   file_reader.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lecartuy <lecartuy@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 15:33:21 by lecartuy          #+#    #+#             */
/*   Updated: 2026/02/24 11:19:51 by lecartuy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/parser.hpp"

FileLoader::FileLoader(const std::string& filename) : _filename(filename) {
    if (_filename.empty())
        throw std::runtime_error("Filename cannot be empty");
}

std::string FileLoader::readFile() {
    std::ifstream file(_filename.c_str());
    if (!file.is_open())
        throw std::runtime_error("Cannot open config file");
    if (file.peek() == std::ifstream::traits_type::eof())
        throw std::runtime_error("Config file is empty");
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string FileLoader::removeComments(const std::string& text) {
    std::string result;
    std::string line;
    std::istringstream stream(text);
    
    while (std::getline(stream, line)) {
        std::size_t commentPos = line.find('#');
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);
        result += line + '\n';
    }
    return result;
}

std::string FileLoader::normalizeWhitespace(const std::string& text) {
    std::string result;
    bool inWhitespace = false;

    for (std::size_t i = 0; i < text.size(); ++i)
    {
        if (text[i] == '\n') {
            result += '\n';
            inWhitespace = false;
        }
        else if (std::isspace(static_cast<unsigned char>(text[i]))) {
            if (!inWhitespace) {
                result += ' ';
                inWhitespace = true;
            }
        }
        else {
            result += text[i];
            inWhitespace = false;
        }
    }
    return result;
}

std::string FileLoader::load() {
    std::string text = readFile();
    text = removeComments(text);
    text = normalizeWhitespace(text);
    return text;
}