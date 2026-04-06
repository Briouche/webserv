#include "webserv.hpp"
#include <cstring>
#include <unistd.h>
#include <ctime>

void    freeEnv(char** envp)
{
    for (int i = 0; envp[i] != NULL; ++i)
        delete[] envp[i];
    delete[] envp;
}

char**	buildEnv(httpInfo& httpinfo)
{
	std::vector<std::string>	env;
	const HttpRequest&			request = httpinfo.req;

	env.push_back("REDIRECT_STATUS=200");
	env.push_back("SERVER_SOFTWARE=webserv/1.0");
	env.push_back("SERVER_PROTOCOL=HTTP/1.1");
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");

	std::ostringstream port;
    port << httpinfo.conf.ports[0];
    env.push_back("SERVER_PORT=" + port.str());
	env.push_back("SERVER_NAME=" + httpinfo.conf.host);

	env.push_back("REQUEST_METHOD=" + request.getMethod());
	env.push_back("SCRIPT_NAME=" + request.getUri());
	env.push_back("PATH_INFO=" + request.getUri());
	env.push_back("SCRIPT_FILENAME=" + httpinfo.filepath);
	env.push_back("QUERY_STRING=" + request.getQuery());

	std::ostringstream	bodySize;
	bodySize << httpinfo.req.getBody().size();
	env.push_back("CONTENT_LENGTH=" + bodySize.str());
	env.push_back("CONTENT_TYPE=" + request.getHeaderValue("content-type"));

	//add other headers with HTTP_ prefix (cookies etc)
	const std::map<std::string, std::string>& 			headers = request.getHeaders();
	std::map<std::string, std::string>::const_iterator	it = headers.begin();
	while (it != headers.end())
	{
		if (it->first == "content-type" || it->first == "content-length") {
			it++;
			continue ;
		}
		std::string key = "HTTP_" + toUpperCase(it->first);
		for (size_t i = 0; i < key.size(); ++i)
		{
			if (key[i] == '-')
				key[i] = '_';
		}
		env.push_back(key + "=" + it->second);
		it++;
	}

	char**	envp = new char*[env.size() + 1];
	for (size_t i = 0; i < env.size(); ++i)
	{
		envp[i] = new char[env[i].size() + 1];
		std::strcpy(envp[i], env[i].c_str());
	}
	envp[env.size()] = NULL;
	return (envp);
}

void	handleChild(httpInfo& httpinfo, int pipe_req[2], int pipe_res[2], std::string& extention)
{
	//setup pipes
	close(pipe_req[1]);
	close(pipe_res[0]);
	if (dup2(pipe_req[0], STDIN_FILENO) == -1 || dup2(pipe_res[1], STDOUT_FILENO) == -1)
	{
		close(pipe_req[0]);
		close(pipe_res[1]);
		exit(1);
	}
	close(pipe_req[0]);
	close(pipe_res[1]);

	//build arguments
	char**	env = buildEnv(httpinfo);
	char*	av[3];
	av[0] = const_cast<char*>(extention.c_str());
	av[1] = const_cast<char*>(httpinfo.filepath.c_str());
	av[2] = NULL;

	execve(extention.c_str(), av, env);
	perror("execve failed");
	freeEnv(env);
	exit(1);
}


bool server::isCgiPipe(int fd)
{
	return (_cgiPipeToClientFd.find(fd) != _cgiPipeToClientFd.end());
}

client* server::getClientByPipe(int pipe_fd, int &server_idx)
{
    std::map<int, int>::iterator it = _cgiPipeToClientFd.find(pipe_fd);
    if (it == _cgiPipeToClientFd.end())
		return NULL;
    return (getClient(it->second, server_idx));
}

void server::handleCgiRead(int fd)
{
    int     server_idx;
    client* cli = getClientByPipe(fd, server_idx);
    if (!cli)
        return ;

    char    buffer[2048];
    long    bytes = read(fd, buffer, sizeof(buffer));
    if (bytes > 0)
    {
        cli->cgi.output += std::string(buffer, bytes);
        return ;										// more data might come, wait for next epoll event
    }
    													// bytes == 0 or -1 --> CGI is done
    epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    _cgiPipeToClientFd.erase(fd);
    cli->cgi.pipeOut = -1;

	if (bytes == -1)
	{
		cli->getResponse() = returnResponse(cli->getResponse(), 500, "Internal Server Error: cgi error: read fail", *_config_per_listen_fd[cli->cgi.serverFd]);
		return ;
	}
	
    int status;
    waitpid(cli->cgi.pid, &status, 0);
    cli->cgi.running = false;

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        cli->getResponse() = returnResponse(cli->getResponse(), 500, "Internal Server Error: cgi error: exit status not 0", *_config_per_listen_fd[cli->cgi.serverFd]);
    else if (WIFSIGNALED(status))
        cli->getResponse() = returnResponse(cli->getResponse(), 500, "Internal Server Error: cgi killed", *_config_per_listen_fd[cli->cgi.serverFd]);
    else
	{
		httpInfo info(cli->getRequest(), cli->getResponse(), *_config_per_listen_fd[cli->cgi.serverFd]);
        cli->getResponse() = parseCGIoutput(cli->getResponse(), cli->cgi.output, info);
    }
	prepareResponseForWrite(&(*cli), cli->cgi.serverFd);
}

void server::handleCgiWrite(int fd)
{
    int		server_idx;
    client*	cli = getClientByPipe(fd, server_idx);
    if (!cli)
        return ;

    const	std::string& body = cli->cgi.bodyToWrite;
    long	written = write(fd, body.c_str() + cli->cgi.bodyWritten, \
                         body.size() - cli->cgi.bodyWritten);
	if (written == -1)
	{
		cli->setResponse(returnResponse(cli->getResponse(), 500, "Internal Server Error: cgi write failed", *_config_per_listen_fd[server_idx]));
		removeClient(cli->getFd(), server_idx);
	}
	
	else if (written == -1 || cli->cgi.bodyWritten >= body.size())
	{
		epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
		_cgiPipeToClientFd.erase(fd);
		cli->cgi.pipeIn = -1;
	}

    else if (written > 0)
		cli->cgi.bodyWritten += written;

}

void server::cleanupCgi(client* cli)
{
    if (cli->cgi.pipeIn != -1)
    {
        epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, cli->cgi.pipeIn, NULL);
        close(cli->cgi.pipeIn);
        _cgiPipeToClientFd.erase(cli->cgi.pipeIn);
        cli->cgi.pipeIn = -1;
    }
    if (cli->cgi.pipeOut != -1)
    {
        epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, cli->cgi.pipeOut, NULL);
        close(cli->cgi.pipeOut);
        _cgiPipeToClientFd.erase(cli->cgi.pipeOut);
        cli->cgi.pipeOut = -1;
    }
    cli->cgi.running = false;
}

void server::checkTimeouts(time_t now)
{
    for (std::map<int, std::vector<client> >::iterator it = _clients_per_server.begin(); 
         it != _clients_per_server.end(); ++it)
    {
        for (std::vector<client>::iterator cli = it->second.begin(); 
             cli != it->second.end(); ++cli)
        {
            if (cli->cgi.running && (now - cli->cgi.start >= TIMEOUT_SECONDS))
            {
                kill(cli->cgi.pid, SIGKILL);
                waitpid(cli->cgi.pid, NULL, 0);
                cleanupCgi(&(*cli));
                cli->getResponse() = returnResponse(cli->getResponse(), 504, "CGI Timeout", 
                                                     *_config_per_listen_fd[cli->cgi.serverFd]);
                cli->getResponse().addHeader("Connection", "close");
                prepareResponseForWrite(&(*cli), cli->cgi.serverFd);
                break;
            }
            
            if (!cli->cgi.running && (now - cli->getLastActive() > TIMEOUT_SECONDS))
            {
                /* If the client never sent any data, just drop the connection (avoid 408 spam on idle accepts). */
                if (!cli->isHeadersParsed() && cli->getBuffer().empty())
                {
                    removeClient(cli->getFd(), it->first);
                    break;
                }
                cli->getResponse() = returnResponse(cli->getResponse(), 408, "Request Timeout", 
                                                     *_config_per_listen_fd[it->first]);
                cli->getResponse().addHeader("Connection", "close");
                prepareResponseForWrite(&(*cli), it->first);
				break;
        	}
    	}
	}
}

HttpResponse&	execCgi(httpInfo& httpinfo, std::string& extention)
{
	int		pipe_req[2];															// parent writes request to child
	int		pipe_res[2];															// child writes response to parent

	if (pipe(pipe_req) == -1)
		return (returnResponse(httpinfo.res, 500, "Internal Server Error: pipe fail", httpinfo.conf));
	else if (pipe(pipe_res) == -1)
	{
		close(pipe_req[0]);
		close(pipe_req[1]);
		return (returnResponse(httpinfo.res, 500, "Internal Server Error: pipe fail", httpinfo.conf));
	}

	pid_t	pid = fork();
	if (pid == -1)
	{
		close(pipe_req[0]);
		close(pipe_req[1]);
		close(pipe_res[0]);
		close(pipe_res[1]);
		return (returnResponse(httpinfo.res, 500, "Internal Server Error: fork fail", httpinfo.conf));
	}
	else if (pid == 0)
		handleChild(httpinfo, pipe_req, pipe_res, extention);

	close(pipe_req[0]);
	close(pipe_res[1]);

	// set child info in the client and set cgi.running -> true
	httpinfo.cli->cgi.running = true;
	httpinfo.cli->cgi.pid = pid;
	httpinfo.cli->cgi.pipeIn = pipe_req[1];
	httpinfo.cli->cgi.pipeOut = pipe_res[0];
	httpinfo.cli->cgi.bodyToWrite = httpinfo.req.getBody();
	httpinfo.cli->cgi.bodyWritten = 0;
	httpinfo.cli->cgi.start = time(NULL);

	epoll_event	ev;
	ev.data.fd = pipe_res[0];
	ev.events = EPOLLIN;
	epoll_ctl(httpinfo.epollFd, EPOLL_CTL_ADD, pipe_res[0], &ev);
	(*httpinfo.cgiPipeToClientFd)[pipe_res[0]] = httpinfo.cli->getFd();
	if (!httpinfo.cli->cgi.bodyToWrite.empty())
	{
		ev.data.fd = pipe_req[1];
		ev.events = EPOLLOUT;
		epoll_ctl(httpinfo.epollFd, EPOLL_CTL_ADD, pipe_req[1], &ev);
		(*httpinfo.cgiPipeToClientFd)[pipe_req[1]] = httpinfo.cli->getFd();
	}
	else
	{
		close(pipe_req[1]);
		httpinfo.cli->cgi.pipeIn = -1;
	}

	return (httpinfo.res);
}
