/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_client.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brcoppie <brcoppie@student.42belgium.be    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2026/03/31 17:05:35 by brcoppie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

void server::acceptClient(int listen_fd)
{
	int client_fd = accept(listen_fd, NULL, NULL);
	if (client_fd < 0)
		return;

	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = client_fd;
	epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

	client c;
	c.setFd(client_fd);
	c.setLastActive(time(NULL));

	c.cgi.running = false;
	c.cgi.pid = -1;
	c.cgi.pipeIn = -1;
	c.cgi.pipeOut = -1;
	c.cgi.output = "";
	c.cgi.bodyToWrite = "";
	c.cgi.bodyWritten = 0;
	c.cgi.start = 0;
	c.cgi.serverFd = listen_fd;

	_clients_per_server[listen_fd].push_back(c);
}

client *server::getClient(int fd, int &server_idx)
{
	for (std::map<int, std::vector<client> >::iterator it = _clients_per_server.begin();
		 it != _clients_per_server.end(); ++it)
	{
		for (std::vector<client>::iterator cli = it->second.begin(); cli != it->second.end(); ++cli)
		{
			if (cli->getFd() == fd)
			{
				server_idx = it->first;
				return &(*cli);
			}
		}
	}
	return NULL;
}

void server::removeClient(int fd, int server_idx)
{
	client *cli = getClient(fd, server_idx);
	if (cli && cli->cgi.running)
	{
		kill(cli->cgi.pid, SIGKILL);
		waitpid(cli->cgi.pid, NULL, 0);
		cleanupCgi(cli);
	}

	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);

	if (_clients_per_server.find(server_idx) != _clients_per_server.end())
	{
		std::vector<client> &clients = _clients_per_server[server_idx];
		for (std::vector<client>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			if (it->getFd() == fd)
			{
				clients.erase(it);
				break;
			}
		}
	}
}

void server::cleanup()
{
	for (std::map<int, std::vector<client> >::iterator it = _clients_per_server.begin();
		 it != _clients_per_server.end(); ++it)
	{
		for (std::vector<client>::iterator cli = it->second.begin(); cli != it->second.end(); ++cli)
		{
			if (cli->cgi.running)
				cleanupCgi(&(*cli));
			close(cli->getFd());
		}
	}
	_clients_per_server.clear();
	for (size_t i = 0; i < _listen_fds.size(); ++i)
	{
		close(_listen_fds[i]);
	}
	_listen_fds.clear();
	if (_epoll_fd != -1)
		close(_epoll_fd);
}
