/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ASocket.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 18:43:54 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/16 12:39:47 by nicolas          ###   ########.fr       */
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

	// initialize struct addrinfo _hints, _socketInfo && struct pollfd
	memset(&_poll, 0, sizeof(_poll));
	memset(&_hints, 0, sizeof(_hints));
	memset(&_socketInfo, 0, sizeof(_socketInfo));
}

ASocket::ASocket(const ASocket &other):
	_poll(other._poll),
	_hints(other._hints)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ASocket: copy constructor called.";
		std::cout << WHITE;
	}

	socketInfoDeepCopy(other);
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
		_poll = other._poll;
		_hints = other._hints;
		socketInfoDeepCopy(other);
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

	if (_socketInfo)
		freeaddrinfo(_socketInfo);
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
	if (_socketInfo != NULL)
		return (_socketInfo->ai_addr);
	return (NULL);
	//return (reinterpret_cast<struct sockaddr*>(&_address));
}

const int	&ASocket::getSocketFd(void) const
{
	return (_poll.fd);
}

const std::string	ASocket::getIP(void) const
{
	char		ipString[INET6_ADDRSTRLEN];
	memset(ipString, 0, sizeof(ipString));

	if (_socketInfo != NULL)
	{
		void	*addr = NULL;

		if (_socketInfo->ai_family == AF_INET
			&& _socketInfo->ai_addrlen >= sizeof(struct sockaddr_in))
		{
			// IPv4
			struct sockaddr_in	*ipv4 = reinterpret_cast<struct sockaddr_in*>(_socketInfo->ai_addr);
			addr = &(ipv4->sin_addr);
		}
		else if (_socketInfo->ai_family == AF_INET6
			&& _socketInfo->ai_addrlen >= sizeof(struct sockaddr_in6))
		{
			//IPv6
			struct sockaddr_in6 *ipv6 = reinterpret_cast<struct sockaddr_in6*>(_socketInfo->ai_addr);
			addr = &(ipv6->sin6_addr);
		}

		if (addr != NULL)
			inet_ntop(_socketInfo->ai_family, addr, ipString, sizeof(ipString));
	}

	return (ipString);
}

uint16_t	ASocket::getPort(void) const
{
	if (_socketInfo != NULL)
	{
		void	*port = NULL;

		if (_socketInfo->ai_family == AF_INET
			&& _socketInfo->ai_addrlen >= sizeof(struct sockaddr_in))
		{
			struct sockaddr_in *ipv4 = reinterpret_cast<struct sockaddr_in*>(_socketInfo->ai_addr);
			port = &(ipv4->sin_port);
		}
		else if (_socketInfo->ai_family == AF_INET6
			&& _socketInfo->ai_addrlen >= sizeof(struct sockaddr_in6))
		{
			struct sockaddr_in6 *ipv6 = reinterpret_cast<struct sockaddr_in6*>(_socketInfo->ai_addr);
			port = &(ipv6->sin6_port);
		}

		if (port != NULL)
		{
			uint16_t	portValue;
			memcpy(&portValue, port, sizeof(uint16_t));
			return (ntohs(portValue));
		}
	}
	return (0);
}

	/* Protected */
	/* Private */

void	ASocket::socketInfoDeepCopy(const ASocket &other)
{
	if (_socketInfo != NULL)
	{
		freeaddrinfo(_socketInfo);
		_socketInfo = NULL;
	}

	if (other._socketInfo != NULL)
	{
		struct addrinfo *current = other._socketInfo;
		struct addrinfo *previous = NULL;

		while (current != NULL)
		{
			struct addrinfo *newAddrInfo = new struct addrinfo;
			memcpy(newAddrInfo, current, sizeof(struct addrinfo));
			newAddrInfo->ai_next = NULL;

			if (previous == NULL)
				_socketInfo = newAddrInfo;
			else
				previous->ai_next = newAddrInfo;

			previous = newAddrInfo;
			current = current->ai_next;
		}
	}
}

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
