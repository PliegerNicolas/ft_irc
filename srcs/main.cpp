/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 03:14:40 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/16 11:51:43 by nicolas          ###   ########.fr       */
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

static int	convertPortToInt(char *strPort)
{
	std::istringstream	iss(strPort);
	int					port;

	if (!(iss >> port))
		throw std::runtime_error("Error: couldn't convert port to string.");
	return (port);
}

// argv[1] == 'port'
// argv[2] == 'password'
int	main(int argc, char **argv)
{
	ASocket::t_soconfig	serverConfig;
	int	port;

	if (verifyArguments(argc, argv))
		return (1);

	try
	{
		port = convertPortToInt(argv[1]);
		serverConfig = ASocket::buildSocketConfig(AF_INET, SOCK_STREAM | SOCK_NONBLOCK,
			IPPROTO_TCP, "127.0.0.1", port);
		Server			serv(serverConfig);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	return (0);
}
