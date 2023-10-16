/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 03:14:40 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/17 00:50:32 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "Server.hpp"

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

	try
	{
		ServerSockets::t_serverconfig	serverConfig1;
		ServerSockets::t_serverconfig	serverConfig2;
		serverConfig1 = ServerSockets::buildServerConfig(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP,
			"127.0.0.1", "6667");
		serverConfig2 = ServerSockets::buildServerConfig(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP,
			"::1", "6667");

		(void)serverConfig1;
		(void)serverConfig2;

		ServerSockets		serv(serverConfig2);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	return (0);
}
