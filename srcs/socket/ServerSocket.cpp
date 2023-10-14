/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 19:19:49 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/14 01:59:59 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "socket/ServerSocket.hpp"

/* Constructors & Destructors */

	/* Public */
ServerSocket::ServerSocket(void):
	ASocket()
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ServerSocket: default constructor called.";
		std::cout << WHITE;
	}

	// Set socket options
	setSocketOptions();

	// Set event to which Server listens too in poll
	_poll.events = POLLIN;

	// bind to network
	ASocket::handleSocketErrors(bindToNetwork());

	// listen to network communication
	ASocket::handleSocketErrors(listenToNetwork());
}

ServerSocket::ServerSocket(const ServerSocket &other):
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
		ASocket::operator=(other);
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

void	ServerSocket::setSocketOptions(void)
{
	t_sooption	socketOptions[SERVOPTSIZE];
	size_t		i = 0;

	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_REUSEADDR, 1);
	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_RCVBUF, 8192);
	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_SNDBUF, 8192);
	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_KEEPALIVE, 1);
	socketOptions[i++] = ASocket::buildSocketOption(IPPROTO_TCP, TCP_QUICKACK, 1);
	socketOptions[i++] = ASocket::buildSocketOption(IPPROTO_TCP, TCP_NODELAY, 1);

	for (i = 0; i < SERVOPTSIZE; i++)
		if (setsockopt(getSocketFd(), socketOptions[i].level, socketOptions[i].value,
			&socketOptions[i].value, sizeof(socketOptions[i].value)) < 0)
			throw std::runtime_error("Error: couldn't set socket option (socket).");
}

int	ServerSocket::bindToNetwork(void)
{
	return (bind(this->getSocketFd(), this->getAddress(), sizeof(*this->getAddress())));
}

int	ServerSocket::listenToNetwork(void)
{
	return (listen(this->getSocketFd(), SERVBACKLOG));
}

/* Getters */

	/* Public */
	/* Protected */
	/* Private */

/* Setters */

	/* Public */
	/* Protected */
	/* Private */
