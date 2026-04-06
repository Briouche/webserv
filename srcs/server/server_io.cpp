/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_io.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stdevis <stdevis@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2026/04/06 10:55:47 by stdevis          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include <unistd.h>
#include <iostream>

bool server::readFromSocket(int fd, client *cli, int server_idx)
{
	char buf[4096];
	int bytes = read(fd, buf, sizeof(buf));

	if (bytes == 0)
		return false;
	else if (bytes == -1)
	{
		removeClient(fd, server_idx);
		cli->setResponse(returnResponse(cli->getResponse(), 500, "Internal Server Error: socket read failed", *_config_per_listen_fd[server_idx]));
		return false;
	}

	cli->appendBuffer(buf, bytes);
	cli->setLastActive(time(NULL));
	return true;
}

bool server::tryParseHeaders(client *cli, int server_idx)
{
	size_t pos = cli->getBuffer().find("\r\n\r\n");
	if (pos == std::string::npos)
		return false;

	std::string headerStr = cli->getBuffer().substr(0, pos + 4);
	if (parseRequestHeaders(headerStr, cli->getRequest()) != 0)
	{
		cli->setResponse(returnResponse(cli->getResponse(), 400, "Bad Request", *_config_per_listen_fd[server_idx]));
		prepareResponseForWrite(cli, server_idx);
		return false;
	}
	cli->setHeadersParsed(true);
	//cli->getRequest().printRequest(); 

	const std::string contentLen = cli->getRequest().getHeaderValue("content-length");
	size_t contentLength = 0;
	if (!contentLen.empty())
		contentLength = std::strtoul(contentLen.c_str(), NULL, 10);
	cli->setContentLengh(contentLength);

	const std::string transferEnc = cli->getRequest().getHeaderValue("transfer-encoding");
	bool isChunked = (transferEnc == "chunked");
	cli->setIsChunked(isChunked);
	cli->setChunkedFinished(!isChunked);
	return true;
}

bool server::tryParseBody(client *cli)
{
	if (cli->getIsChunked())
		return parseRequestBodyChunked(cli);
	return parseRequestBodyContentLength(cli);
}

void server::prepareResponseForWrite(client *cli, int server_idx)
{
	cli->setResponseBuffer(cli->getResponse().buildResponse());
	cli->setWriteOffset(0);

	ssize_t written = write(cli->getFd(), cli->getResponseBuffer().c_str(), cli->getResponseBuffer().size());

	if (written == -1)
	{
		cli->setResponse(returnResponse(cli->getResponse(), 500, "Internal Server Error: socket write failed", *_config_per_listen_fd[server_idx]));
		removeClient(cli->getFd(), server_idx);
		return;
	}
	else if (written < (ssize_t)cli->getResponseBuffer().size())
	{
		cli->setWriteOffset(written);

		epoll_event ev;
		ev.events = EPOLLIN | EPOLLOUT;
		ev.data.fd = cli->getFd();
		epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, cli->getFd(), &ev);
	}
	if (shouldCloseAfterResponse(cli->getResponse()))
	{
		removeClient(cli->getFd(), server_idx);
	}
	else
	{
		cli->reset();
	}
}

void server::handleRead(int fd)
{
	int server_idx;
	client *cli = getClient(fd, server_idx);
	if (!cli)
		return;

	if (!readFromSocket(fd, cli, server_idx))
		return;

	if (!cli->isHeadersParsed() && !tryParseHeaders(cli, server_idx))
		return;

	if (!tryParseBody(cli))
		return;

	if (cli->getSessionId().empty())
	{
		std::string cookieHeader = cli->getRequest().getHeaderValue("cookie");
		std::string existingSession = extractSessionId(cookieHeader);

		//std::cout << "cookieheader = " << cookieHeader << "\n\texisting session = " << existingSession << std::endl;
		if (!existingSession.empty())
		{
			cli->setSessionId(existingSession);
			cli->setHasExistingSession(true);
		}
		else
		{
			std::string newSession = generateSessionId(cli->getFd());
			cli->setSessionId(newSession);
			cli->setHasExistingSession(false);
		}
	}

	std::string url = cli->getRequest().getUri();
	std::string session = cli->getSessionId();

	if (url == "/cgi-bin/login.php" && cli->getRequest().getMethod() == "POST")
	{
		handleLoginPost(cli, server_idx, session);
		return;
	}

	const ServerConfig *config = _config_per_listen_fd[server_idx];
	HttpResponse earlyResponse;
	if (!earlyValidateRequest(cli->getRequest(), earlyResponse, *config))
	{
		earlyResponse.addSetCookie("session_id=" + session + "; Path=/; HttpOnly");
		cli->setResponse(earlyResponse);
		prepareResponseForWrite(cli, server_idx);
		return;
	}
	
	if (!isPublicRoute(url) && !isAuthenticated(session) && !cli->getHasExistingSession())
	{
		handleAuthRedirect(cli, server_idx, session);
		return;
	}

	HttpResponse response = handleRequest(cli->getRequest(), *config, cli, _epoll_fd, &_cgiPipeToClientFd);

	response.addSetCookie("session_id=" + session + "; Path=/; HttpOnly");
	if (cli->getRequest().getVersion() == "HTTP/1.1" && !shouldCloseConnection(cli->getRequest()))
		response.addHeader("Connection", "keep-alive");

	cli->setResponse(response);

	if (cli->cgi.running)
		return;

	prepareResponseForWrite(cli, server_idx);
}

void server::handleWrite(int fd)
{
	int server_idx;
	client *cli = getClient(fd, server_idx);
	if (!cli)
		return;

	ssize_t written = write(fd, cli->getResponseBuffer().c_str() + cli->getWriteOffset(),
							cli->getResponseBuffer().size() - cli->getWriteOffset());
	if (written > 0)
		cli->addWriteOffset(written);	
	if (cli->getWriteOffset() == cli->getResponseBuffer().size())
	{
		if (shouldCloseConnection(cli->getRequest()) || shouldCloseAfterResponse(cli->getResponse()))
			removeClient(fd, server_idx);
		else
			cli->reset();
	}
}
