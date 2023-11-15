/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ASocket.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nicolas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 18:43:54 by nicolas           #+#    #+#             */
/*   Updated: 2023/11/15 10:58:37 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "socket/ASocket.hpp"

/* Constructors & Destructors */

	/* Public */
ASocket::ASocket(void):
	_addrInfo(NULL)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ASocket: Default constructor called.";
		std::cout << WHITE << std::endl;
	}

	memset(&_hints, 0, sizeof(_hints));
}

ASocket::ASocket(const ASocket &other):
	_addrInfo(addrInfoDeepCopy(other)),
	_hints(other._hints)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ASocket: Copy constructor called.";
		std::cout << WHITE << std::endl;
	}
}

ASocket	&ASocket::operator=(const ASocket &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ASocket: Assignment operator called.";
		std::cout << WHITE << std::endl;
	}

	if (this != &other)
	{
		_addrInfo = addrInfoDeepCopy(other);
		_hints = other._hints;
	}

	return (*this);
}

ASocket::~ASocket(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "ASocket: Default destructor called.";
		std::cout << WHITE << std::endl;
	}

	if (_addrInfo)
		freeaddrinfo(_addrInfo);
}
	/* Protected */
	/* Private */

/* Member functions */

	/* Public */
	/* Protected */
	/* Private */

void	ASocket::handleErrors(const int &statusCode)
{
	if (statusCode >= 0)
		return ;

	int					errCode = errno;
	std::ostringstream	errorMessage;

	if (_addrInfo)
		freeaddrinfo(_addrInfo);

	errorMessage << "Error: " << strerror(errCode) << " (socket).";
	throw std::runtime_error(errorMessage.str());
}

/* Getters */

	/* Public */

struct addrinfo	*ASocket::addrInfoDeepCopy(const ASocket &other)
{
	struct addrinfo	*cpy = NULL;

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

	/* Protected */
	/* Private */

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
