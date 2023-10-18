/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:49:23 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/18 14:28:05 by nplieger         ###   ########.fr       */
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

	for (ClientsIterator it = _clients.begin(); it < _clients.end(); it++)
		delete *it;

	_clients.clear();
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
			throw	std::runtime_error(std::string("Error: ") + strerror(errno) + " (server).");

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
				if (handleClientDataReception(client, pollFd) == CLIENT_ONLINE)
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

bool	Server::handleClientDataReception(Client *client, struct pollfd &pollFd)
{
	// TEMP
	const char	delimiter = '\n';
	char		buffer[MSG_BUFFER_SIZE];
	int			readBytes = -1;

	memset(buffer, 0, sizeof(buffer));
	readBytes = recv(pollFd.fd, buffer, sizeof(buffer), 0);

	if (readBytes < 0)
		throw std::runtime_error(std::string("Error: ") + strerror(errno) + " (server).");
	else if (readBytes == 0)
		return (CLIENT_DISCONNECTED);
	else
	{
		client->addToBuffer(buffer, readBytes);
		std::string	message = client->getMessage(delimiter);

		if (message.empty())
			return (CLIENT_ONLINE);

		std::cout << "client n*: ";
		std::cout << message << std::endl;
	}
	return (CLIENT_ONLINE);
}

void	Server::handleClientDisconnections(const ServerSockets::Sockets &serverSockets, size_t &i)
{
	_pollFds.erase(_pollFds.begin() + i);
	_clients.erase(_clients.begin() + (i - serverSockets.size()));
	i--;
}

/* Getters */

	/* Public */
	/* Protected */
	/* Private */

/* Setters */

	/* Public */
	/* Protected */
	/* Private */
