/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brcoppie <brcoppie@student.42belgium.be    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/20 11:27:29 by shokahn           #+#    #+#             */
/*   Updated: 2026/04/01 13:59:31 by brcoppie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <csignal>
#include <sys/wait.h>
#include "color.h"
#include "parser.hpp"
#include "server.hpp"
#include "client.hpp"
#include "config.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

bool earlyValidateRequest(HttpRequest &request, HttpResponse &response, const ServerConfig &conf);

#endif
