/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSockets.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 19:21:43 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/17 18:47:29 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "socket/ClientSockets.hpp"

/* Constructors & Destructors */

	/* Public */
ClientSockets::ClientSockets(void):
	ASocket()
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ClientSockets: default constructor called.";
		std::cout << WHITE;
	}
}

ClientSockets::ClientSockets(const ASocket::t_socket &serverSocket):
	ASocket()
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ClientSockets: parameter constructor called.";
		std::cout << WHITE;
	}

	 _socket.fd = accept(serverSocket.fd, serverSocket.info.ai_addr,
		const_cast<socklen_t*>(&serverSocket.info.ai_addrlen));
}

ClientSockets::ClientSockets(const ClientSockets &other):
	ASocket(other),
	_socket(other._socket)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ClientSockets: copy constructor called.";
		std::cout << WHITE;
	}
}

ClientSockets	&ClientSockets::operator=(const ClientSockets &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ClientSockets: assignment operator called.";
		std::cout << WHITE;
	}

	if (this != &other)
	{
		ASocket::operator=(other);
		_socket = other._socket;
	}

	return (*this);
}

ClientSockets::~ClientSockets(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ClientSockets: default destructor called.";
		std::cout << WHITE;
	}
}

	/* Protected */
	/* Private */

/* Member functions */

	/* Public */
	/* Protected */
	/* Private */

void	ClientSockets::setSocketOptions(void)
{
	t_sooption	socketOptions[CLIENTOPTSIZE];
	size_t		i = 0;

	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_REUSEADDR, 1);
	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_RCVBUF, 8192);
	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_SNDBUF, 8192);
	socketOptions[i++] = ASocket::buildSocketOption(SOL_SOCKET, SO_KEEPALIVE, 1);
	socketOptions[i++] = ASocket::buildSocketOption(IPPROTO_TCP, TCP_QUICKACK, 1);
	socketOptions[i++] = ASocket::buildSocketOption(IPPROTO_TCP, TCP_NODELAY, 1);

	for (i = 0; i < SERVOPTSIZE; i++)
		if (setsockopt(_socket.fd, socketOptions[i].level, socketOptions[i].option,
			&socketOptions[i].value, sizeof(socketOptions[i].value)) < 0)
			throw std::runtime_error("Error: couldn't set socket option (client).");
}

/* Getters */

	/* Public */
	/* Protected */
	/* Private */

/* Setters */

	/* Public */
	/* Protected */
	/* Private */
