/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ASocket.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 18:43:54 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/16 20:09:49 by nicolas          ###   ########.fr       */
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
		std::cout << "ASocket: Default constructor called.";
		std::cout << WHITE;
	}

	// initialize struct addrinfo _hints, _addrInfo && struct pollfd
	memset(&_hints, 0, sizeof(_hints));
	memset(&_addrInfo, 0, sizeof(_addrInfo));
}

ASocket::ASocket(const ASocket &other):
	_pollFds(other._pollFds),
	_hints(other._hints),
	_addrInfo(addrInfoDeepCopy(other))
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ASocket: Copy constructor called.";
		std::cout << WHITE;
	}
}

ASocket	&ASocket::operator=(const ASocket &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ASocket: Assignment operator called.";
		std::cout << WHITE;
	}

	if (this != &other)
	{
		_pollFds = other._pollFds;
		_hints = other._hints;
		_addrInfo = addrInfoDeepCopy(other);
	}

	return (*this);
}

ASocket::~ASocket(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ASocket: Default destructor called.";
		std::cout << WHITE;
	}

	if (_addrInfo)
	{
		freeaddrinfo(_addrInfo);
		_addrInfo = NULL;
	}

	for (PollFdsIt it = _pollFds.begin(); it != _pollFds.end(); it++)
		close(it->fd);
	_pollFds.clear();
}
	/* Protected */
	/* Private */

/* Member functions */

	/* Public */
	/* Protected */
	/* Private */

/* Getters */

	/* Public */

const std::string	ASocket::getIP(void) const
{
	char		ipString[INET6_ADDRSTRLEN];
	memset(ipString, 0, sizeof(ipString));

	if (_addrInfo != NULL)
	{
		void	*addr = NULL;

		if (_addrInfo->ai_family == AF_INET
			&& _addrInfo->ai_addrlen >= sizeof(struct sockaddr_in))
		{
			// IPv4
			struct sockaddr_in	*ipv4 = reinterpret_cast<struct sockaddr_in*>(_addrInfo->ai_addr);
			addr = &(ipv4->sin_addr);
		}
		else if (_addrInfo->ai_family == AF_INET6
			&& _addrInfo->ai_addrlen >= sizeof(struct sockaddr_in6))
		{
			//IPv6
			struct sockaddr_in6 *ipv6 = reinterpret_cast<struct sockaddr_in6*>(_addrInfo->ai_addr);
			addr = &(ipv6->sin6_addr);
		}

		if (addr != NULL)
			inet_ntop(_addrInfo->ai_family, addr, ipString, sizeof(ipString));
	}

	return (ipString);
}

uint16_t	ASocket::getPort(void) const
{
	if (_addrInfo != NULL)
	{
		void	*port = NULL;

		if (_addrInfo->ai_family == AF_INET
			&& _addrInfo->ai_addrlen >= sizeof(struct sockaddr_in))
		{
			struct sockaddr_in *ipv4 = reinterpret_cast<struct sockaddr_in*>(_addrInfo->ai_addr);
			port = &(ipv4->sin_port);
		}
		else if (_addrInfo->ai_family == AF_INET6
			&& _addrInfo->ai_addrlen >= sizeof(struct sockaddr_in6))
		{
			struct sockaddr_in6 *ipv6 = reinterpret_cast<struct sockaddr_in6*>(_addrInfo->ai_addr);
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

void	ASocket::handleSocketErrors(const int &statusCode)
{
	if (statusCode >= 0)
		return ;

	int					errCode = errno;
	std::ostringstream	errorMessage;

	errorMessage << "Error: " << strerror(errCode) << " (socket).";

	if (_addrInfo)
	{
		freeaddrinfo(_addrInfo);
		_addrInfo = NULL;
	}

	throw std::runtime_error(errorMessage.str());
}

	/* Protected */
	/* Private */

struct addrinfo	*ASocket::addrInfoDeepCopy(const ASocket &other)
{
	struct addrinfo	*cpy;

	if (_addrInfo != NULL)
	{
		freeaddrinfo(_addrInfo);
		_addrInfo = NULL;
	}

	if (other._addrInfo != NULL)
	{
		struct addrinfo *current = other._addrInfo;
		struct addrinfo *previous = NULL;

		while (current != NULL)
		{
			struct addrinfo *newAddrInfo = new struct addrinfo;
			memcpy(newAddrInfo, current, sizeof(struct addrinfo));
			newAddrInfo->ai_next = NULL;

			if (previous == NULL)
				cpy = newAddrInfo;
			else
				previous->ai_next = newAddrInfo;

			previous = newAddrInfo;
			current = current->ai_next;
		}
	}
	return (cpy);
}

/* Setters */

	/* Public */
	/* Protected */
	/* Private */

/* Static */

	/* Public */

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
