/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 11:49:32 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/17 18:48:44 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "Client.hpp"

/* Constructors & Destructors */

	/* Public */

Client::Client(const ASocket::t_socket &serverSocket):
	_socket(ClientSockets(serverSocket))
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Client: parameter constructor called.";
		std::cout << WHITE;
	}
}

Client::Client(const Client &other):
	_socket(other._socket)
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
		_socket = other._socket;
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
	_socket(ClientSockets())
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
	/* Protected */
	/* Private */

/* Getters */

	/* Public */

std::string	Client::getMessage(const char delimiter)
{
	size_t		pos = _buffer.find(delimiter);
	std::string	message;

	if (pos != std::string::npos)
	{
		message = _buffer.substr(0, pos);
		_buffer.erase(0, pos + 1);
	}
	return (message);
}

	/* Protected */
	/* Private */

/* Setters */

	/* Public */

void	Client::addToBuffer(const char *buffer, const size_t readBytes)
{
		_buffer.append(buffer, readBytes);
}

	/* Protected */
	/* Private */
