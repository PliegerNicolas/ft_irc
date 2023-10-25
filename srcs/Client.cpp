/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:49:32 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/25 02:43:55 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "Client.hpp"

/* Constructors & Destructors */

	/* Public */

Client::Client(const ASocket::t_socket &serverSocket):
	_clientSocket(ClientSocket(serverSocket)),
	_activeChannel(NULL),
	_serverPermissions(0),
	_connectionRetries(1)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Client: parameter constructor called.";
		std::cout << WHITE;
	}
}

Client::Client(const Client &other):
	_clientSocket(other._clientSocket),
	_activeChannel(other._activeChannel),
	_serverPermissions(other._serverPermissions),
	_connectionRetries(other._connectionRetries)
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
		_activeChannel = other._activeChannel;
		_serverPermissions = other._serverPermissions;
		_connectionRetries = other._connectionRetries;
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
	_clientSocket(ClientSocket()),
	_activeChannel(NULL),
	_serverPermissions(0),
	_connectionRetries(1)
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

	memset(&pollFd, 0, sizeof(pollFd));

	pollFd.fd = _clientSocket.getSocket().fd;
	pollFd.events = POLLIN | POLLHUP | POLLERR;

	return (pollFd);
}

void	Client::closeSocketFd(void)
{
	close(_clientSocket.getSocket().fd);
}

void	Client::incrementConnectionRetries(void)
{
	_connectionRetries++;
}

void	Client::receiveMessage(const std::string &message) const
{
	send(getSocketFd(), message.c_str(), message.length(), 0);
}

void	Client::broadcastMessageToChannel(const std::string &message) const
{
	if (!_activeChannel)
		return ;

	Channel::Users	users = _activeChannel->getUsers();

	for (Channel::UsersIterator it = users.begin(); it != users.end(); it++)
	{
		if (this != it->client)
			(it->client)->receiveMessage(message);
	}
}

	/* Protected */
	/* Private */

/* Getters */

	/* Public */

std::string	&Client::getBuffer(void)
{
	return (_messageBuffer);
}

int	Client::getSocketFd(void) const
{
	return (_clientSocket.getSocket().fd);
}

const std::string	&Client::getNickname(void) const
{
	return (_nickname);
}

short	&Client::getConnectionRetries(void)
{
	return (_connectionRetries);
}

int	Client::getServerPermissions(void) const
{
	return (_serverPermissions);
}

Channel	*Client::getActiveChannel(void)
{
	return (_activeChannel);
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
		const int			&getSocketFd(void);
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


void	Client::setNickname(const std::string &nickname)
{
	_nickname = nickname;
}

void	Client::setServerPermissions(const int &mask)
{
	_serverPermissions |= mask;
}

void	Client::setActiveChannel(Channel *channel)
{
	_activeChannel = channel;
}

	/* Protected */
	/* Private */
