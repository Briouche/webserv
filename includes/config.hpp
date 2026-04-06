/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lecartuy <lecartuy@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/24 19:44:38 by lecartuy          #+#    #+#             */
/*   Updated: 2026/03/17 16:03:57 by lecartuy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <vector>
#include <string> 
#include <map>

struct LocationConfig
{
    LocationConfig() : autoindex(false), has_redirect(false), redirect_code(0) {}

    std::string path;
    bool autoindex;

    std::vector<std::string> allowed_methods;

    std::string root;
    std::string index;

    std::string upload_path;

    std::map<std::string, std::string> cgi_handlers;

    bool has_redirect;
    int redirect_code;
    std::string redirect_url;
};

struct ServerConfig
{
    ServerConfig() : host("0.0.0.0"), client_max_body_size(1000000) {}

    std::vector<int> ports;

    std::string root;

    std::vector<LocationConfig> locations;

    std::string host;
    std::string server_name;

    size_t client_max_body_size;

    std::map<int, std::string> error_pages;
};

struct Config
{
    std::vector<ServerConfig> servers;
};

#endif