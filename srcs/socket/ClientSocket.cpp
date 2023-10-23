/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 19:21:43 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/23 10:32:31 by nplieger         ###   ########.fr       */
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

ClientSocket::ClientSocket(const ASocket::t_socket &serverSocket):
	ASocket()
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ClientSocket: parameter constructor called.";
		std::cout << WHITE;
	}

	_socket.fd = accept(serverSocket.fd, serverSocket.info->ai_addr,
		const_cast<socklen_t*>(&serverSocket.info->ai_addrlen));
	handleErrors(_socket.fd);
	_socket.info = NULL;
}

ClientSocket::ClientSocket(const ClientSocket &other):
	ASocket(other),
	_socket(other._socket)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ClientSocket: copy constructor called.";
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
		_socket = other._socket;
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

	for (i = 0; i < SERVOPTSIZE; i++)
	{
		if (setsockopt(_socket.fd, socketOptions[i].level, socketOptions[i].option,
			&socketOptions[i].value, sizeof(socketOptions[i].value)) < 0)
			throw std::runtime_error("Error: couldn't set socket option (client).");
		if (fcntl(_socket.fd, F_SETFL, O_NONBLOCK))
			throw std::runtime_error("Error: couldn't set socket option (client).");
	}
}

void	ClientSocket::handleErrors(const int &statusCode)
{
	if (statusCode >= 0)
		return ;

	int					errCode = errno;
	std::ostringstream	errorMessage;

	errorMessage << "Error: " << strerror(errCode) << " (socket).";
	throw std::runtime_error(errorMessage.str());
}

/* Getters */

	/* Public */

const ASocket::t_socket	ClientSocket::getSocket(void) const
{
	return (_socket);
}

	/* Protected */
	/* Private */

/* Setters */

	/* Public */
	/* Protected */
	/* Private */
