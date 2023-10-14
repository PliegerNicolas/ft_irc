/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ASocket.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 18:43:54 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/14 11:43:39 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "socket/ASocket.hpp"

/* Constructors & Destructors */

	/* Public */
ASocket::ASocket(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ASocket: default constructor called.";
		std::cout << WHITE;
	}

	// initialize struct sockaddr_in _address && struct pollfd
	memset(&_address, 0, sizeof(_address));
	memset(&_poll, 0, sizeof(_poll));
}

ASocket::ASocket(const ASocket &other):
	_address(other._address),
	_poll(other._poll)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ASocket: copy constructor called.";
		std::cout << WHITE;
	}
}

ASocket	&ASocket::operator=(const ASocket &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ASocket: assignment operator called.";
		std::cout << WHITE;
	}

	if (this != &other)
	{
		_address = other._address;
		_poll = other._poll;
	}

	return (*this);
}

ASocket::~ASocket(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ASocket: default destructor called.";
		std::cout << WHITE;
	}

	close(_poll.fd);
}
	/* Protected */
	/* Private */

/* Member functions */

	/* Public */
	/* Protected */

void	ASocket::handleSocketErrors(const int &statusCode)
{
	if (statusCode >= 0)
		return ;

	int					errCode = errno;
	std::ostringstream	errorMessage;

	errorMessage << "Error: " << strerror(errCode) << " (socket).";
	throw std::runtime_error(errorMessage.str());
}

	/* Private */

/* Getters */

	/* Public */

const struct pollfd	&ASocket::getPoll(void) const
{
	return (_poll);
}

struct sockaddr	*ASocket::getAddress(void)
{
	return (reinterpret_cast<struct sockaddr*>(&_address));
}

const int	&ASocket::getSocketFd(void) const
{
	return (_poll.fd);
}

std::string	ASocket::getIP(void) const
{
	char		ipString[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &(_address.sin_addr), ipString, INET_ADDRSTRLEN);
	return (ipString);
}

uint16_t	ASocket::getPort(void) const
{
	return (ntohs(_address.sin_port));
}

	/* Protected */
	/* Private */

/* Setters */

	/* Public */
	/* Protected */
	/* Private */

/* Static */

	/* Public */

const ASocket::t_soconfig
ASocket::buildSocketConfig(const int &domain, const int &service, const int &protocol,
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

const ASocket::t_sooption
ASocket::buildSocketOption(const int &level, const int &option, const int &value)
{
	t_sooption	socketOption;

	socketOption.level = level;
	socketOption.option = option;
	socketOption.value = value;

	return (socketOption);
}

	/* Protected */
	/* Private */
