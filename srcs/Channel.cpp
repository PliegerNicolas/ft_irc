/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mfaucheu <mfaucheu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/20 14:50:37 by nplieger          #+#    #+#             */
/*   Updated: 2023/10/25 15:52:38 by mfaucheu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

/* Constructors & Destructors */

	/* Public */

Channel::Channel(Client* channelCreator):
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

Channel::t_user	Channel::createUser(Client* client, const size_t &permissionsMask)
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

const Channel::Users	&Channel::getUsers(void) const
{
	return (_users);
}

	/* Protected */
	/* Private */

void	Channel::addUser(Client* client, const int &mask)
{
	t_user	newUser;

	newUser.client = client;
	newUser.permissionsMask = mask;
	_users.push_back(newUser);
}

void	Channel::removeUser(const Client* client, const int mask)
{
	UsersIterator it;

	if (areBitsSet(mask, KICK)
		&& areBitsNotSet(client->getServerPermissions(), KICK))
		std::cerr << "Error: permission" << std::endl;
	else
	{
		for (it = _users.begin(); it != _users.end(); ++it)
		{
			if (it->client == client)
			{
				std::cerr << "J'enleve un user\n";
				std::cerr << "nick = " << it->client->getNickname() << std::endl;
				_users.erase(it);
				return ;
			}
		}
	}
}

void	Channel::removeUser(std::string _nickname)
{
	for (UsersIterator it = _users.begin(); it != _users.end(); ++it)
	{
		if (it->client->getNickname() == _nickname)
		{
			_users.erase(it);
			return ;
		}
	}
}

/* Setters */

	/* Public */
	/* Protected */
	/* Private */
