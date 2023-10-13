/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 19:19:49 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/13 19:25:08 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "socket/ServerSocket.hpp"

/* Constructors & Destructors */

	/* Public */
ServerSocket::ServerSocket(void):
	Socket()
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSocket: default constructor called.";
		std::cout << WHITE;
	}
}

ServerSocket::ServerSocket(const ServerSocket &other):
	Socket(other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Socket: copy constructor called.";
		std::cout << WHITE;
	}
	(void)other;
}

ServerSocket	&ServerSocket::operator=(const ServerSocket &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSocket: assignment operator called.";
		std::cout << WHITE;
	}

	if (this != &other)
	{
		Socket::operator=(other);
	}

	return (*this);
}

ServerSocket::~ServerSocket(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSocket: default destructor called.";
		std::cout << WHITE;
	}
}
	/* Protected */
	/* Private */

/* Member functions */

	/* Public */
	/* Protected */
	/* Private */

/* Getters */

	/* Public */
	/* Protected */
	/* Private */

/* Setters */

	/* Public */
	/* Protected */
	/* Private */
