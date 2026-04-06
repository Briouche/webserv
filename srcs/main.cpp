/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brcoppie <brcoppie@student.42belgium.be    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/24 11:20:33 by lecartuy          #+#    #+#             */
/*   Updated: 2026/04/01 15:03:26 by brcoppie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include <unistd.h>

volatile sig_atomic_t g_running = 1;

void handle_sigint(int sig)
{
    (void)sig;
    g_running = 0;
    std::cout << YELLOW "\nCTRL+C received, stopping server..." RESET << std::endl;
}


int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./webserv config.conf" << std::endl;
        return 1;
    }

    try
    {
        FileLoader loader(argv[1]);
        std::string configText = loader.load();

        std::string configPath(argv[1]);
        std::string::size_type slash = configPath.find_last_of('/');
        if (slash != std::string::npos)
        {
            std::string configDir = configPath.substr(0, slash);
            if (!configDir.empty())
                chdir(configDir.c_str());
        }

        Tokenizer tokenizer(configText);
        std::vector<Token> tokens = tokenizer.tokenize();

        Parser parser(tokens);
        Config config = parser.parse();

        server srv(config.servers);
        srv.init();
        signal(SIGINT, handle_sigint);
        srv.run();

        std::cout << GREEN BOLD "Server stopped correctly." RESET << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << RED "Erreur: " << e.what() << RESET << std::endl;
        return 1;
    }
    return 0;
}
