/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:49:23 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/19 11:10:11 by nplieger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "Server.hpp"

/* Constructors & Destructors */

	/* Public */

Server::Server(const ServerSockets::t_serverconfig &serverConfig):
	_serverSockets(ServerSockets(serverConfig))
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Server: parameter constructor called.";
		std::cout << WHITE;
	}

	{
		const ServerSockets::Sockets	&sockets = _serverSockets.getSockets();

		for (ServerSockets::SocketsConstIt it = sockets.begin(); it != sockets.end(); it++)
			_pollFds.push_back(generatePollFd(*it));
	}

	eventLoop();
}

Server::Server(const Server &other):
	_serverSockets(other._serverSockets),
	_clients(other._clients),
	_pollFds(other._pollFds)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Server: copy constructor called.";
		std::cout << WHITE;
	}
}

Server	&Server::operator=(const Server &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Server: assignment operator called.";
		std::cout << WHITE;
	}

	if (this != &other)
	{
		_serverSockets = other._serverSockets;
		_clients = other._clients;
		_pollFds = other._pollFds;
	}

	return (*this);
}

Server::~Server(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Server: default destructor called.";
		std::cout << WHITE;
	}

	deleteClients(_clients);
	_pollFds.clear();
}
	/* Protected */
	/* Private */

Server::Server(void):
	_serverSockets(ServerSockets())
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Server: default constructor called.";
		std::cout << WHITE;
	}

	{
		const ServerSockets::Sockets	&sockets = _serverSockets.getSockets();

		for (ServerSockets::SocketsConstIt it = sockets.begin(); it != sockets.end(); it++)
			_pollFds.push_back(generatePollFd(*it));
	}
}

/* Member functions */

	/* Public */
const struct pollfd	Server::generatePollFd(const ASocket::t_socket	&serverSocket)
{
	struct pollfd	pollFd;

	pollFd.fd = serverSocket.fd;
	pollFd.events = POLLIN;

	return (pollFd);
}

	/* Protected */
	/* Private */

void	Server::eventLoop(void)
{
	const ServerSockets::Sockets	&serverSockets = _serverSockets.getSockets();

	while (true)
	{
		int		activity = poll(_pollFds.data(), _pollFds.size(), -1);

		if (activity < 0)
		{
			deleteClients(_clients);
			throw	std::runtime_error(std::string("Error: ") + strerror(errno) + " (server).");
		}

		size_t	i = 0;

		handleServerPollFds(serverSockets, i);
		handleClientsPollFds(serverSockets, i);
	}
}

void	Server::handleServerPollFds(const ServerSockets::Sockets &serverSockets, size_t &i)
{
	for (; i < serverSockets.size(); i++)
	{
		const ASocket::t_socket	&serverSocket = serverSockets[i];
		struct pollfd			&pollFd = _pollFds[i];

		switch (pollFd.revents)
		{
			case POLLIN:
				pollFd.revents &= ~POLLIN;
				handleClientConnections(serverSocket);
				break ;
			default:
				break ;
		}
	}
}

void	Server::handleClientsPollFds(const ServerSockets::Sockets &serverSockets, size_t &i)
{
	for (; i < _pollFds.size(); i++)
	{
		Client			*client = _clients[i - serverSockets.size()];
		struct pollfd	&pollFd = _pollFds[i];

		switch (pollFd.revents)
		{
			case POLLIN:
				pollFd.revents &= ~POLLIN;
				if (handleClientDataReception(client, pollFd) == CLIENT_CONNECTED)
					break ;
			case POLLHUP:
				pollFd.revents &= ~POLLHUP;
				handleClientDisconnections(serverSockets, i);
				break ;
			case POLLERR:
				break ;
			default:
				break ;
		}
	}
}

// ACT ON POLL EVENTS.

void	Server::handleClientConnections(const ServerSockets::t_socket &serverSocket)
{
	Client			*client = new Client(serverSocket);
	struct	pollfd	clientPollFd = client->generatePollFd();

	_clients.push_back(client);
	_pollFds.push_back(clientPollFd);
}

/**
 *	With a TCP socket, excess data isn't discarded but is buffered
 *	by the operating system. The subsequent recv calls will read that data.
**/
bool	Server::handleClientDataReception(Client *client, struct pollfd &pollFd)
{
	std::string		&clientBuffer = client->getBuffer();
	std::string		delimiter = DELIMITER;

	char			recvBuffer[MSG_BUFFER_SIZE];
	int				readBytes = -1;


	memset(recvBuffer, 0, sizeof(recvBuffer));

	readBytes = recv(pollFd.fd, recvBuffer, sizeof(recvBuffer), 0);
	if (readBytes < 0)
	{
		deleteClients(_clients);
		throw std::runtime_error(std::string("Error: ") + strerror(errno) + " (server).");
	}
	else if (readBytes == 0)
		return (CLIENT_DISCONNECTED);

	clientBuffer.append(recvBuffer, readBytes);

	std::string		message;
	size_t			pos;

	while ((pos = clientBuffer.find(delimiter)) != std::string::npos)
	{
		if (pos >= (MSG_BUFFER_SIZE - delimiter.length()))
		{
			pos = MSG_BUFFER_SIZE - delimiter.length();
			message = clientBuffer.substr(0, pos);
			clientBuffer.erase(0, pos);
			message += delimiter;
		}
		else
		{
			pos += delimiter.length();
			message = clientBuffer.substr(0, pos);
			clientBuffer.erase(0, pos);
		}

		if (message != delimiter)
			std::cout << "Client n°" << "x" << ": " << message;
	}

	return (CLIENT_CONNECTED);
}

void	Server::handleClientDisconnections(const ServerSockets::Sockets &serverSockets, size_t &i)
{
	_pollFds.erase(_pollFds.begin() + i);
	_clients.erase(_clients.begin() + (i - serverSockets.size()));
	i--;
}

// Utilities

void	Server::deleteClients(Server::Clients &clients)
{
	for (ClientsIterator it = clients.begin(); it < clients.end(); it++)
		delete *it;
	clients.clear();
}

/* Getters */

	/* Public */
	/* Protected */
	/* Private */

/* Setters */

	/* Public */
	/* Protected */
	/* Private */
