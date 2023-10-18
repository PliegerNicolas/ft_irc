/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:49:23 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/18 11:46:54 by nplieger         ###   ########.fr       */
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
		{
			struct pollfd	newPollFd;

			newPollFd.fd = it->fd;
			newPollFd.events = POLLIN | POLLHUP | POLLERR;

			_pollFds.push_back(newPollFd);
		}
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
		const ServerSockets::Sockets	&_sockets = _serverSockets.getSockets();

		for (ServerSockets::SocketsConstIt it = _sockets.begin(); it != _sockets.end(); it++)
		{
			struct pollfd	newPollFd;

			newPollFd.fd = it->fd;
			newPollFd.events = POLLIN | POLLHUP | POLLERR;

			_pollFds.push_back(newPollFd);
		}
	}
}

/* Member functions */

	/* Public */
	/* Protected */
	/* Private */

void	Server::eventLoop(void)
{
	const ServerSockets::Sockets	&serverSockets = _serverSockets.getSockets();

	while (true)
	{
		// Await requests all stored file descriptors (server and clients).
		int		activity = poll(_pollFds.data(), _pollFds.size(), -1);

		if (activity < 0)
			throw	std::runtime_error(std::string("Error: ") + strerror(errno) + " (server).");

		size_t	i = 0;

		// Watch events on server pollFds
		for (; i < serverSockets.size(); i++)
		{

			const ASocket::t_socket	&serverSocket = serverSockets[i];
			struct pollfd			&pollFd = _pollFds[i];

			if (pollFd.revents & POLLIN)
			{
				pollFd.revents &= ~POLLIN;

				// Client connection
				Client			*client = new Client(serverSocket);
				struct	pollfd	clientPollFd = client->generatePollFd();

				_clients.push_back(client);
				_pollFds.push_back(clientPollFd);
			}
			else if (pollFd.revents & POLLHUP)
			{
				pollFd.revents &= ~POLLHUP;

				// socket disconnection ???
			}
			else if (pollFd.revents & POLLERR)
			{
				pollFd.revents &= ~POLLERR;
			}
			(void)serverSocket;
		}

		// Watch events on client pollFds
		for (; i < _pollFds.size(); i++)
		{
			Client			*client = _clients[i - serverSockets.size()];
			struct pollfd	&pollFd = _pollFds[i];

			if (pollFd.revents & POLLIN)
			{
				pollFd.revents &= ~POLLIN;

				// Data received from client.
				// Clear revent flag.
				pollFd.revents &= ~POLLIN;

				const char	delimiter = '\n';
				char		buffer[MSG_BUFFER_SIZE];
				int			readBytes = -1;

				memset(buffer, 0, sizeof(buffer));
				readBytes = recv(pollFd.fd, buffer, sizeof(buffer), 0);

				if (readBytes <= 0)
				{
					// Force disconnection.
					pollFd.revents |= POLLHUP;
					if (readBytes < 0)
						throw std::runtime_error(std::string("Error: ") + strerror(errno)
							+ " (server).");
				}
				else
				{
					// Fill buffer string.
					client->addToBuffer(buffer, readBytes);
					// Extract message if delimiter found.
					std::string	message = client->getMessage(delimiter);

					if (message.empty())
						return ;

					// Print message into server.
					std::cout << "client n*: ";
					std::cout << message << std::endl;
				}
			}
			else if (pollFd.revents & POLLHUP)
			{
				pollFd.revents &= ~POLLHUP;

				// Client disconnection.
			}
			else if (pollFd.revents & POLLERR)
			{
				pollFd.revents &= ~POLLERR;
			}
			(void)client;
		}
	}
}

/*
// Set as front element of _pollFds, server pollfd.
_pollFds.push_back(getSocket().getPoll());

while (true)
{
	// Wait for event on any socket (revent on pollfd).
	int activity = poll(_pollFds.data(), _pollFds.size(), -1);

	if (activity < 0)
		throw std::runtime_error("Error: poll error (server).");

	handleClientConnections(_pollFds.front());

	for (size_t i = 0; i < _clients.size(); i++)
	{
		struct pollfd	&pollFd = _pollFds[i + 1];
		Client			*client = *(_clients.begin() + i);

		try
		{
			handleClientDataReception(client, pollFd);
		}
		catch (const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
		handleClientDisconnections(pollFd, i);
	}
}
*/

/*
void	Server::handleClientConnections(struct pollfd &pollFd)
{
	if (!(pollFd.revents & POLLIN))
		return ;

	// Clear revent flag.
	pollFd.revents &= ~POLLIN;

	_clients.push_back(new Client(*this));
	_pollFds.push_back(_clients.back()->getSocket().getPoll());
}

void	Server::handleClientDataReception(Client *client, struct pollfd &pollFd)
{
	if (!(pollFd.revents & POLLIN))
		return ;

	// Clear revent flag.
	pollFd.revents &= ~POLLIN;

	const char	delimiter = '\n';
	char		buffer[MSG_BUFFER_SIZE];
	int			readBytes = -1;

	memset(buffer, 0, sizeof(buffer));
	readBytes = recv(pollFd.fd, buffer, sizeof(buffer), 0);

	if (readBytes <= 0)
	{
		// Force disconnection.
		pollFd.revents |= POLLHUP;
		if (readBytes < 0)
			throw std::runtime_error(std::string("Error: ") + strerror(errno) + " (server).");
	}
	else
	{
		// Fill buffer string.
		client->addToBuffer(buffer, readBytes);
		// Extract message if delimiter found.
		std::string	message = client->getMessage(delimiter);

		if (message.empty())
			return ;

		// Print message into server.
		std::cout << "nº" << client->getSocket().getSocketFd() << ": ";
		std::cout << message << std::endl;
	}
}

void	Server::handleClientDisconnections(struct pollfd &pollFd, size_t &index)
{
	if (!(pollFd.revents & POLLHUP))
		return ;

	// Clear revent flag.
	pollFd.revents &= ~POLLHUP;

	_pollFds.erase(_pollFds.begin() + (index - 1));
	_clients.erase(_clients.begin() + index);
	index--;
}
*/

/* Getters */

	/* Public */
	/* Protected */
	/* Private */

/* Setters */

	/* Public */
	/* Protected */
	/* Private */
