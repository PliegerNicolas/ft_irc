/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:49:32 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/18 15:36:07 by nplieger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "Client.hpp"

/* Constructors & Destructors */

	/* Public */

Client::Client(const ASocket::t_socket &serverSocket):
	_clientSocket(ClientSocket(serverSocket))
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Client: parameter constructor called.";
		std::cout << WHITE;
	}
}

Client::Client(const Client &other):
	_clientSocket(other._clientSocket)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Client: copy constructor called.";
		std::cout << WHITE;
	}
}

Client	&Client::operator=(const Client &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Client: assignment operator called.";
		std::cout << WHITE;
	}

	if (this != &other)
	{
		_clientSocket = other._clientSocket;
	}

	return (*this);
}

Client::~Client(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Client: default destructor called.";
		std::cout << WHITE;
	}
}
	/* Protected */
	/* Private */

Client::Client(void):
	_clientSocket(ClientSocket())
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Client: default constructor called.";
		std::cout << WHITE;
	}
}

/* Member functions */

	/* Public */

const struct pollfd	Client::generatePollFd(void)
{
	struct pollfd	pollFd;

	pollFd.fd = _clientSocket.getSocket().fd;
	pollFd.events = POLLIN | POLLHUP | POLLERR;

	return (pollFd);
}

	/* Protected */
	/* Private */

/* Getters */

	/* Public */

std::string	&Client::getBuffer(void)
{
	return (_buffer);
}

	/* Protected */
	/* Private */

/* Setters */

	/* Public */
	/* Protected */
	/* Private */
