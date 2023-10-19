/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 03:14:40 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/19 15:36:55 by nplieger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "Server.hpp"

volatile sig_atomic_t g_serverExit = false;

static void	putExpectedUsage(void)
{
	std::cout << "Expected usage: ./ircserv <port> <password>." << std::endl;
}

static void	putErrorMessage(const std::string str)
{
	std::cerr << "Error : " << str << "." << std::endl;
}

static bool	verifyArguments(int argc, char **argv)
{
	if (argc != 3)
	{
		std::ostringstream	errorMsg;
		errorMsg << "2 arguments expected, " << (argc - 1) << " received";
		putErrorMessage(errorMsg.str());
		putExpectedUsage();
		return (true);
	}
	else if (!*argv[1] || !*argv[2])
	{
		putErrorMessage("empty arguments aren't permitted");
		putExpectedUsage();
		return (true);
	}
	return (false);
}

// argv[1] == 'port'
// argv[2] == 'password'
int	main(int argc, char **argv)
{
	if (verifyArguments(argc, argv))
		return (1);

	signal(SIGINT, sigintHandler);

	try
	{
		ServerSockets::t_serverconfig	serverConfig;
		serverConfig = ServerSockets::buildServerConfig(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP,
			"localhost", argv[1]);

		Server	serv(serverConfig);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	signal(SIGINT, SIG_DFL);

	return (0);
}
