/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nplieger <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/20 14:50:37 by nplieger          #+#    #+#             */
/*   Updated: 2023/10/24 16:49:47 by nplieger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

/* Constructors & Destructors */

	/* Public */

Channel::Channel(const Client* channelCreator):
	_userLimit(-1)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Channel: default constructor called.";
		std::cout << WHITE;
	}

	_users.push_back(createUser(channelCreator, getAdminPerms()));
}

Channel::Channel(const Channel &other):
	_userLimit(other._userLimit)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Channel: copy constructor called.";
		std::cout << WHITE;
	}
}

Channel	&Channel::operator=(const Channel &other)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Channel: assignment operator called.";
		std::cout << WHITE;
	}

	if (this != &other)
	{
		_userLimit = other._userLimit;
	}

	return (*this);
}

Channel::~Channel(void)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Channel: default destructor called.";
		std::cout << WHITE;
	}
}
	/* Protected */
	/* Private */

Channel::Channel(void):
	_userLimit(-1)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Channel: default constructor called.";
		std::cout << WHITE;
	}
}

/* Member functions */

	/* Public */

bool	Channel::isUserRegistered(const Client* client) const
{
	for (UsersConstIterator it = _users.begin(); it != _users.end(); ++it)
	{
		if (client == it->client)
			return (true);
	}

	return (false);
}

	/* Protected */
	/* Private */

Channel::t_user	Channel::createUser(const Client* client, const size_t &permissionsMask)
{
	t_user	user;

	user.client = client;
	user.permissionsMask = permissionsMask;

	return (user);
}

/* Getters */

	/* Public */

int	Channel::getUserPerms(void)
{
	return (0);
}

int	Channel::getHalfOpsPerms(void)
{
	return (KICK | INVITE);
}

int	Channel::getOpsPerms(void)
{
	return (KICK | BAN | INVITE | TOPIC);
}

int	Channel::getAdminPerms(void)
{
	return (KICK | BAN | INVITE | TOPIC | MODE);
}

	/* Protected */
	/* Private */

void	Channel::addUser(const Client* client, const int &mask)
{
	t_user	newUser;

	newUser.client = client;
	newUser.permissionsMask = mask;
	_users.push_back(newUser);
}

/* Setters */

	/* Public */
	/* Protected */
	/* Private */
