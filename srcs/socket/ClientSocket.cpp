/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 19:21:43 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/14 11:47:13 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "socket/ClientSocket.hpp"

/* Constructors & Destructors */

	/* Public */

ClientSocket::ClientSocket(const ServerSocket &server):
	ASocket()
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ClientSocket: parameter constructor called.";
		std::cout << WHITE;
	}

	socklen_t	addrLen = sizeof(*this->getAddress());
	_poll.fd = accept(server.getSocketFd(), this->getAddress(), &addrLen);
	_poll.events = POLLIN;
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

ClientSocket::ClientSocket(void):
	ASocket()
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ClientSocket: default constructor called.";
		std::cout << WHITE;
	}
	_poll.events = POLLIN;
}

/* Member functions */

	/* Public */
	/* Protected */
	/* Private */

void	ClientSocket::setSocketOptions(void)
{
	t_sooption	socketOptions[CLIENTOPTSIZE];
	size_t		i = 0;

	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_REUSEADDR, 1);
	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_RCVBUF, 8192);
	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_SNDBUF, 8192);
	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_KEEPALIVE, 1);
	socketOptions[i++] = ASocket::buildSocketOption(IPPROTO_TCP, TCP_QUICKACK, 1);
	socketOptions[i++] = ASocket::buildSocketOption(IPPROTO_TCP, TCP_NODELAY, 1);

	for (i = 0; i < CLIENTOPTSIZE; i++)
		if (setsockopt(getSocketFd(), socketOptions[i].level, socketOptions[i].value,
			&socketOptions[i].value, sizeof(socketOptions[i].value)) < 0)
			throw std::runtime_error("Error: couldn't set socket option (socket).");
}

/* Getters */

	/* Public */
	/* Protected */
	/* Private */

/* Setters */

	/* Public */
	/* Protected */
	/* Private */
