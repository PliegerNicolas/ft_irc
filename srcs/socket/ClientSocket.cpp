/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 19:21:43 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/14 01:23:59 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "socket/ClientSocket.hpp"

/* Constructors & Destructors */

	/* Public */
ClientSocket::ClientSocket(void):
	ASocket()
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ClientSocket: default constructor called.";
		std::cout << WHITE;
	}
}

ClientSocket::ClientSocket(const ClientSocket &other):
	ASocket(other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Socket: copy constructor called.";
		std::cout << WHITE;
	}
	(void)other;
}

ClientSocket	&ClientSocket::operator=(const ClientSocket &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ClientSocket: assignment operator called.";
		std::cout << WHITE;
	}

	if (this != &other)
	{
		ASocket::operator=(other);
	}

	return (*this);
}

ClientSocket::~ClientSocket(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ClientSocket: default destructor called.";
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
