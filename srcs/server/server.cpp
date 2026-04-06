/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brcoppie <brcoppie@student.42belgium.be    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2026/03/31 17:05:35 by brcoppie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include <unistd.h>

server::server(const std::vector<ServerConfig> &configs)
	: _epoll_fd(-1), _configs(configs) {}

server::~server()
{
	if (_epoll_fd != -1)
		close(_epoll_fd);
	for (size_t i = 0; i < _listen_fds.size(); ++i)
	{
		if (_listen_fds[i] != -1)
			close(_listen_fds[i]);
	}
	for (std::map<int, std::vector<client> >::iterator it = _clients_per_server.begin();
		 it != _clients_per_server.end(); ++it)
	{
		for (std::vector<client>::iterator cli = it->second.begin(); cli != it->second.end(); ++cli)
		{
			close(cli->getFd());
		}
	}
}
