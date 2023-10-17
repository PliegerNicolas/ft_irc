/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ASocket.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 18:43:54 by nicolas           #+#    #+#             */
/*   Updated: 2023/10/17 16:09:52 by nicolas          ###   ########.fr       */
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
}

ASocket::ASocket(const ASocket &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ASocket: Copy constructor called.";
		std::cout << WHITE;
	}

	(void)other;
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
		(void)other;
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

/*
const std::string	ASocket::getIP(const struct addrinfo &addrInfo) const
{
	char		ipString[INET6_ADDRSTRLEN];
	memset(ipString, 0, sizeof(ipString));

	void	*addr = NULL;

	if (addrInfo.ai_family == AF_INET
		&& addrInfo.ai_addrlen >= sizeof(struct sockaddr_in))
	{
		// IPv4
		struct sockaddr_in	*ipv4 = reinterpret_cast<struct sockaddr_in*>(addrInfo.ai_addr);
		addr = &(ipv4->sin_addr);
	}
	else if (addrInfo.ai_family == AF_INET6
		&& addrInfo.ai_addrlen >= sizeof(struct sockaddr_in6))
	{
		//IPv6
		struct sockaddr_in6 *ipv6 = reinterpret_cast<struct sockaddr_in6*>(addrInfo.ai_addr);
		addr = &(ipv6->sin6_addr);
	}

	if (addr != NULL)
	{
		// retrieve IP address
		inet_ntoa(ipString, *reinterpret_cast<struct in_addr*>(addr));
	}

	return (ipString);
}

const std::string	ASocket::getPort(const struct addrinfo &addrInfo) const
{
	std::string	portString;

	void	*port = NULL;

	if (addrInfo.ai_family == AF_INET
		&& addrInfo.ai_addrlen >= sizeof(struct sockaddr_in))
	{
		struct sockaddr_in *ipv4 = reinterpret_cast<struct sockaddr_in*>(addrInfo.ai_addr);
		port = &(ipv4->sin_port);
	}
	else if (addrInfo.ai_family == AF_INET6
		&& addrInfo.ai_addrlen >= sizeof(struct sockaddr_in6))
	{
		struct sockaddr_in6 *ipv6 = reinterpret_cast<struct sockaddr_in6*>(addrInfo.ai_addr);
		port = &(ipv6->sin6_port);
	}

	if (port != NULL)
	{
		uint16_t portValue = ntohs(*reinterpret_cast<uint16_t*>(portPtr));
		std::stringstream ss;
		ss << portValue;
		portString = ss.str();
	}

	return (portString);
}
*/

	/* Protected */
	/* Private */

/*
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
*/

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
