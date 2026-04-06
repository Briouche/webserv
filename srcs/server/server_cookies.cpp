/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_cookies.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brcoppie <brcoppie@student.42belgium.be    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2026/03/31 17:05:35 by brcoppie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include <cstdlib>

// ─── Connection helpers ──────────────────────────────────────────────────────

bool server::shouldCloseConnection(const HttpRequest &req)
{
	std::string conn = req.getHeaderValue("connection");
	return (conn == "close");
}

bool server::shouldCloseAfterResponse(const HttpResponse &res)
{
	std::string conn = res.getHeaderValue("Connection");
	if (conn.empty())
		conn = res.getHeaderValue("connection");
	if (conn == "close")
		return true;
	if (res.getStatus() == 408 || res.getStatus() == 504)
		return true;
	return false;
}

// ─── Session / cookie helpers ────────────────────────────────────────────────

std::string server::generateSessionId(int fd)
{
	unsigned int seed = time(NULL) ^ fd;
	char buf[17];
	const char *chars = "0123456789abcdef";

	for (int i = 0; i < 16; i++)
		buf[i] = chars[rand_r(&seed) % 16];

	buf[16] = '\0';
	return std::string(buf);
}

std::string server::extractSessionId(const std::string &cookieHeader)
{
	std::string key = "session_id=";
	size_t pos = cookieHeader.find(key);
	if (pos == std::string::npos)
		return "";
	pos += key.size();
	size_t end = cookieHeader.find(';', pos);
	if (end == std::string::npos)
		return cookieHeader.substr(pos);
	return cookieHeader.substr(pos, end - pos);
}

bool server::isPublicRoute(const std::string &url)
{
	return (url == "/login.html" || url.find("/css/") == 0 || url.find("/js/") == 0);
}

bool server::isAuthenticated(const std::string &session)
{
	return (session_authenticated.count(session) && session_authenticated[session]);
}

// ─── Auth helpers ────────────────────────────────────────────────────────────

static bool parseCredentials(const std::string &body, const std::string &user, const std::string &pass)
{
	return (body.find("username=" + user) != std::string::npos &&
			body.find("password=" + pass) != std::string::npos);
}

bool server::handleLoginPost(client *cli, int server_idx, const std::string &session)
{
	std::string body = cli->getRequest().getBody();
	HttpResponse redirectResp;
	redirectResp.setStatus(302);

	if (parseCredentials(body, "admin", "42"))
	{
		session_authenticated[session] = true;
		redirectResp.addHeader("Location", "/index.html");
	}
	else
		redirectResp.addHeader("Location", "/login.html");

	redirectResp.addSetCookie("session_id=" + session + "; Path=/; HttpOnly");
	cli->setResponse(redirectResp);
	prepareResponseForWrite(cli, server_idx);
	return true;
}

bool server::handleAuthRedirect(client *cli, int server_idx, const std::string &session)
{
	HttpResponse redirectResp;
	redirectResp.setStatus(302);
	redirectResp.addHeader("Location", "/login.html");
	redirectResp.addSetCookie("session_id=" + session + "; Path=/; HttpOnly");
	cli->setResponse(redirectResp);
	prepareResponseForWrite(cli, server_idx);
	return true;
}
