#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <map>
#include <csignal>
#include <sys/epoll.h>
#include "client.hpp"
#include "config.hpp"

#define MAX_EVENTS 64
#define TIMEOUT_SECONDS 5

class server
{
public:
	server(const std::vector<ServerConfig> &configs);
	~server();

	void init();
	void run();

private:
	int _epoll_fd;
	std::vector<ServerConfig> _configs;
	std::vector<int> _listen_fds;
	std::map<int, std::vector<client> > _clients_per_server;
	std::map<int, const ServerConfig *> _config_per_listen_fd;
	std::map<int, int> _cgiPipeToClientFd;
	std::map<std::string, bool> session_authenticated;

	void setupSockets();
	void setupEpoll();
	void acceptClient(int listen_fd);
	void handleRead(int fd);
	void handleWrite(int fd);
	bool isListenSocket(int fd);
	void cleanup();
	void prepareResponseForWrite(client *cli, int server_idx);
	client *getClient(int fd, int &server_idx);
	void removeClient(int fd, int server_idx);
	bool shouldCloseConnection(const HttpRequest &req);

	void handleCgiRead(int fd);
	void handleCgiWrite(int fd);
	void checkTimeouts(time_t now);
	void cleanupCgi(client *cli);
	bool isCgiPipe(int fd);
	client *getClientByPipe(int pipe_fd, int &server_idx);
	bool handleLoginPost(client *cli, int server_idx, const std::string &session);
	bool handleAuthRedirect(client *cli, int server_idx, const std::string &session);
	bool isAuthenticated(const std::string &session);
	std::string extractSessionId(const std::string &cookieHeader);
	std::string generateSessionId(int fd);
	bool isPublicRoute(const std::string &url);
	bool shouldCloseAfterResponse(const HttpResponse &res);

	bool tryParseBody(client *cli);
	bool tryParseHeaders(client *cli, int server_idx);
	bool readFromSocket(int fd, client *cli, int server_idx);
};

int parseRequestHeaders(const std::string &rawRequest, HttpRequest &request);
int parseRequestBodyChunked(client *cli);
int parseRequestBodyContentLength(client *cli);

#endif
