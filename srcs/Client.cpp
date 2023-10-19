/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:49:32 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/19 13:43:38 by nplieger         ###   ########.fr       */
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
	return (_messageBuffer);
}

	/* Protected */
	/* Private */

/* Setters */

	/* Public */

int	Client::readAndStoreFdBuffer(Server &server, const struct pollfd &pollFd)
{
	int				readBytes = -1;
	char			recvBuffer[MSG_BUFFER_SIZE];
	memset(recvBuffer, 0, sizeof(recvBuffer));

	readBytes = recv(pollFd.fd, recvBuffer, sizeof(recvBuffer), 0);
	if (readBytes < 0)
	{
		server.deleteClients();
		throw std::runtime_error(std::string("Error: ") + strerror(errno) + " (server).");
	}
	else if (readBytes == 0)
		return (CLIENT_DISCONNECTED);

	_messageBuffer.append(recvBuffer, readBytes);
	return (CLIENT_CONNECTED);
}

	/* Protected */
	/* Private */
