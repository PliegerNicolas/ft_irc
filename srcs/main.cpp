/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 03:14:40 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/14 11:18:04 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include <iostream>
#include <sstream>

#include "colors.hpp"

#include "socket/ServerSocket.hpp"
#include "socket/ClientSocket.hpp"

static void	putExpectedUsage(void)
{
	std::cout << CYAN;
	std::cout << "Expected usage: ./ircserv <port> <password>." << std::endl;
	std::cout << WHITE;
}

static void	putErrorMessage(const std::string str)
{
	std::cerr << RED;
	std::cerr << "Error : " << str << "." << std::endl;
	std::cerr << WHITE;
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
	int	port;

	if (verifyArguments(argc, argv))
		return (1);

	try
	{
		port = convertPortToInt(argv[1]);
		(void)port;
		ServerSocket	s(AF_INET, SOCK_STREAM, IPPROTO_TCP, "0.0.0.0", port);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	return (0);
}

