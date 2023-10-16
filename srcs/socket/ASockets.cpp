/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ASockets.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 18:43:54 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/16 22:33:24 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "socket/ASockets.hpp"

/* Constructors & Destructors */

	/* Public */
ASockets::ASockets(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ASockets: Default constructor called.";
		std::cout << WHITE;
	}
}

ASockets::ASockets(const ASockets &other):
	_sockets(other._sockets)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ASockets: Copy constructor called.";
		std::cout << WHITE;
	}
}

ASockets	&ASockets::operator=(const ASockets &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ASockets: Assignment operator called.";
		std::cout << WHITE;
	}

	if (this != &other)
	{
		_sockets = other._sockets;
	}

	return (*this);
}

ASockets::~ASockets(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ASockets: Default destructor called.";
		std::cout << WHITE;
	}

	for (SocketsIt it = _sockets.begin(); it != _sockets.end(); it++)
		close(it->first);
	_sockets.clear();
}
	/* Protected */
	/* Private */

/* Member functions */

	/* Public */
	/* Protected */

void	ASockets::handleSocketErrors(const int &statusCode)
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

const std::string	ASockets::getIP(const int &fd) const
{
	(void)fd;
	return ("");
}

const std::string	ASockets::getPort(const int &fd) const
{
	(void)fd;
	return ("");
}

/*
const std::string	ASockets::getIP(void) const
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

uint16_t	ASockets::getPort(void) const
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
*/

	/* Protected */
	/* Private */

/*
struct addrinfo	*ASockets::addrInfoDeepCopy(const ASockets &other)
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
*/

/* Setters */

	/* Public */
	/* Protected */
	/* Private */

/* Static */

	/* Public */

const ASockets::t_sooption
ASockets::buildSocketOption(const int &level, const int &option, const int &value)
{
	t_sooption	socketOption;

	socketOption.level = level;
	socketOption.option = option;
	socketOption.value = value;

	return (socketOption);
}

	/* Protected */
	/* Private */
