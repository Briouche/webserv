/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_setup.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brcoppie <brcoppie@student.42belgium.be    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2026/04/01 14:41:26 by brcoppie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "color.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdexcept> 

void server::setupSockets()
{
	size_t serverCounter = 0;

	for (size_t i = 0; i < _configs.size(); ++i)
	{
		const ServerConfig &conf = _configs[i];
		for (size_t j = 0; j < conf.ports.size(); j++)
		{
			int port = conf.ports[j];
			int server_fd = socket(AF_INET, SOCK_STREAM, 0);
			if (server_fd < 0)
			{
				std::cerr << "Failed to create socket for server " << i << std::endl;
				continue;
			}

			int opt = 1;
			setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

			struct sockaddr_in address;
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = inet_addr(_configs[i].host.c_str());
			address.sin_port = htons(port);

			if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
			{
				std::cerr << "Failed to bind socket for port " << port << std::endl;
				close(server_fd);
				continue;
			}

			if (listen(server_fd, 128) < 0)
			{
				std::cerr << "Failed to listen on port " << port << std::endl;
				close(server_fd);
				continue;
			}

			serverCounter += 1;
			fcntl(server_fd, F_SETFL, O_NONBLOCK);
			_listen_fds.push_back(server_fd);
			_clients_per_server[server_fd] = std::vector<client>();
			_config_per_listen_fd[server_fd] = &(_configs[i]);

			std::cout << GREEN "Server listening on " << _configs[i].host << ":"
					  << port << RESET << std::endl;
		}
	}
	if (serverCounter == 0)
		throw std::runtime_error("All ports already used");
}

void server::setupEpoll()
{
	_epoll_fd = epoll_create1(0);
	if (_epoll_fd < 0)
	{
		std::cerr << "Failed to create epoll" << std::endl;
		return;
	}

	for (size_t i = 0; i < _listen_fds.size(); ++i)
	{
		epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = _listen_fds[i];
		epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _listen_fds[i], &ev);
	}
}

void server::init()
{
	setupSockets();
	setupEpoll();
}

bool server::isListenSocket(int fd)
{
	for (size_t i = 0; i < _listen_fds.size(); i++)
	{
		if (fd == _listen_fds[i])
			return true;
	}
	return false;
}

void server::run()
{
	epoll_event events[MAX_EVENTS];

	extern volatile sig_atomic_t g_running;

	while (g_running)
	{
		int n = epoll_wait(_epoll_fd, events, MAX_EVENTS, 1000);

		if (n < 0)
			continue;
		time_t now = time(NULL);
		for (int i = 0; i < n; ++i)
		{
			int fd = events[i].data.fd;
			if (isListenSocket(fd))
				acceptClient(fd);
			else if (isCgiPipe(fd) && (events[i].events & (EPOLLIN | EPOLLHUP)))
				handleCgiRead(fd);
			else if (isCgiPipe(fd) && (events[i].events & EPOLLOUT))
				handleCgiWrite(fd);
			else if (events[i].events & EPOLLIN)
				handleRead(fd);
			else if (events[i].events & EPOLLOUT)
				handleWrite(fd);
		}
		checkTimeouts(now);
	}
	cleanup();
}
