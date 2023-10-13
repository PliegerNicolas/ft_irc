/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 18:43:54 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/13 19:13:33 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "socket/Socket.hpp"

/* Constructors & Destructors */

	/* Public */
Socket::Socket(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Socket: default constructor called.";
		std::cout << WHITE;
	}

	// initialize struct sockaddr_in _address && struct pollfd
	memset(&_address, 0, sizeof(_address));
	memset(&_poll, 0, sizeof(_poll));
}

Socket::Socket(const Socket &other):
	_address(other._address),
	_poll(other._poll)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Socket: copy constructor called.";
		std::cout << WHITE;
	}
}

Socket	&Socket::operator=(const Socket &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Socket: assignment operator called.";
		std::cout << WHITE;
	}

	if (this != &other)
	{
		_address = other._address;
		_poll = other._poll;
	}

	return (*this);
}

Socket::~Socket(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Socket: default destructor called.";
		std::cout << WHITE;
	}

	close(_poll.fd);
}
	/* Protected */
	/* Private */

/* Member functions */

	/* Public */
	/* Protected */

void	Socket::handleSocketErrors(const int &statusCode)
{
	if (statusCode >= 0)
		return ;

	std::ostringstream	errorMessage;

	errorMessage << "Error: " << strerror(statusCode) << " (socket).";
	throw std::runtime_error(errorMessage.str());
}

	/* Private */

/* Getters */

	/* Public */

const struct pollfd	&Socket::getPoll(void) const
{
	return (_poll);
}

const struct sockaddr	*Socket::getAddress(void) const
{
	return (reinterpret_cast<const struct sockaddr*>(&_address));
}

const int	&Socket::getSocketFd(void) const
{
	return (_poll.fd);
}

std::string	Socket::getIP(void) const
{
	char		ipString[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &(_address.sin_addr), ipString, INET_ADDRSTRLEN);
	return (ipString);
}

uint16_t	Socket::getPort(void) const
{
	return (ntohs(_address.sin_port));
}

	/* Protected */
	/* Private */

/* Setters */

	/* Public */
	/* Protected */
	/* Private */

const Socket::t_soconfig
Socket::buildSocketConfig(const int &domain, const int &service, const int &protocol,
	const std::string &interface, const int &port)
{
	t_soconfig	socketConfig;

	socketConfig.domain = domain;
	socketConfig.service = service;
	socketConfig.protocol = protocol;
	socketConfig.interface = interface;
	socketConfig.port = port;

	return (socketConfig);
}
