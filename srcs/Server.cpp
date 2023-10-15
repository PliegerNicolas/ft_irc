/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:49:23 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/15 12:51:03 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "Server.hpp"

/* Constructors & Destructors */

	/* Public */

Server::Server(const ASocket::t_soconfig &config):
	_socket(ServerSocket(config))
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Server: parameter constructor called.";
		std::cout << WHITE;
	}

	Server::eventLoop();
}

Server::Server(const Server &other):
	_socket(other._socket),
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
		_socket = other._socket;
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
}
	/* Protected */
	/* Private */

Server::Server(void):
	_socket(ServerSocket())
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Server: default constructor called.";
		std::cout << WHITE;
	}

	_pollFds.push_back(getSocket().getPoll());
}

/* Member functions */

	/* Public */
	/* Protected */
	/* Private */

void	Server::eventLoop(void)
{
	bool	end = false;
	// Set as front element of _pollFds, server pollfd.
	_pollFds.push_back(getSocket().getPoll());

	while (end == false)
	{
		// Wait for event on any socket (revent on pollfd).
		int activity = poll(_pollFds.data(), _pollFds.size(), -1);

		if (activity < 0)
			throw std::runtime_error("Error: poll error (server).");

		for (size_t i = 0; i < _clients.size(); i++)
		{
			struct pollfd	&pollFd = _pollFds[i + 1];
			Client			*client = *(_clients.begin() + i);

			end |= handleClientDataTransfers(client, pollFd);
		}

		handleClientConnections(_pollFds.front());
	}
}

void	Server::handleClientConnections(struct pollfd &pollFd)
{
	if (!(pollFd.revents & POLLIN))
		return ;

	// Clear revent flag.
	pollFd.revents &= ~ POLLIN;

	_clients.push_back(new Client(*this));
	_pollFds.push_back(_clients.back()->getSocket().getPoll());
}

bool	Server::handleClientDataTransfers(Client *client, struct pollfd &pollFd)
{
	if (pollFd.revents & POLLIN)
	{
		// Clear revent flag.
		pollFd.revents &= ~ POLLIN;

		char		buffer[1024];
		const char	delimiter = '\n';

		try
		{
			int	readBytes = recv(pollFd.fd, buffer, sizeof(buffer), 0);

			client->addToBuffer(buffer, readBytes);

			const std::string	message = client->getMessage(delimiter);

			if (!message.empty())
			{
				// TEMP
				if (message == "EXIT")
					return (true);
				// END_TEMP

				std::cout << "nº" << client->getSocket().getSocketFd() << ": ";
				std::cout << message << std::endl;
			}
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error: " << e.what() << "(server)." << std::endl;
			return (true);
		}
	}
	return (false);
}

void	Server::handleClientDisconnections(struct pollfd &pollFd, size_t &index)
{
	if (!(pollFd.revents & POLLHUP))
		return ;

	// Clear revent flag.
	pollFd.revents &= ~ POLLHUP;

	_pollFds.erase(_pollFds.begin() + (index - 1));
	_clients.erase(_clients.begin() + index);
	index--;
}

/* Getters */

	/* Public */

ServerSocket	&Server::getSocket(void)
{
	return (_socket);
}

	/* Protected */
	/* Private */

/* Setters */

	/* Public */
	/* Protected */
	/* Private */
