/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:49:23 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/14 18:26:10 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "Server.hpp"

/* Constructors & Destructors */

	/* Public */

Server::Server(const int &domain, const int &service, const int &protocol,
	const std::string &interface, const int &port):
	_socket(ServerSocket(domain, service, protocol, interface, port))
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
	_pollFds(other._pollFds),
	_clients(other._clients)
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
		_pollFds = other._pollFds;
		_clients = other._clients;
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

	_pollFds.push_back(_socket.getPoll());
}

/* Member functions */

	/* Public */
	/* Protected */
	/* Private */

void	Server::eventLoop(void)
{
	int	activity;

	// Insert the server's socketFd at the beginning of the _pollFds list.
	_pollFds.push_back(_socket.getPoll());

	while (true)
	{
		// Await updates on any pollFd of the list.
		activity = poll(&_pollFds[0], _pollFds.size(), -1);

		if (activity < 0)
			throw std::runtime_error("Error: poll error (server).");

		// By only using .push_back(), serverPollFd will always be at front.
		if (_pollFds.front().revents & POLLIN)
		{
			// On user connection
			_clients.push_back(Client(*this));
			_pollFds.push_back(_clients.back().getSocket().getPoll());
		}

		for (size_t i = 1; i < _pollFds.size(); i++)
		{
			struct pollfd	&pollFd = _pollFds[i];

			if (pollFd.revents & POLLIN)
			{
				// on client data reception
				std::cout << "A client is doing something, youhou !" << std::endl;

				pollFd.revents = 0;
			}
			else if (pollFd.revents & POLLHUP)
			{
				// On client disconnection.
				_clients.erase(_clients.begin() + i);
				_pollFds.erase(_pollFds.begin() + i);
				i--;
			}
		}
	}
}

/* Getters */

	/* Public */

const ServerSocket	&Server::getSocket(void) const
{
	return (_socket);
}

	/* Protected */
	/* Private */

/* Setters */

	/* Public */
	/* Protected */
	/* Private */
