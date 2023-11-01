/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/20 14:50:37 by nplieger          #+#    #+#             */
/*   Updated: 2023/10/31 22:08:08 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

/* Constructors & Destructors */

	/* Public */

Channel::Channel(const std::string &name):
	_name(truncate(name, MAX_CHANNELNAME_LEN)),
	_userLimit(-1)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Channel: default constructor called.";
		std::cout << WHITE;
	}
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
	_name(""),
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

bool	Channel::isClientRegistered(const Client* client) const
{
	for (UsersConstIterator it = _users.begin(); it != _users.end(); ++it)
	{
		if (client == it->client)
			return (true);
	}

	return (false);
}

bool	Channel::isFull(void) const
{
	if (_userLimit == -1 || _userLimit > static_cast<int>(_users.size()))
		return (false);
	return (true);
}

bool	Channel::isEmpty(void) const
{
	if (_users.size() == 0)
		return (true);
	return (false);
}

void	Channel::addUser(Client* client, const int &mask)
{
	_users.push_back(createUser(client, mask));
}

void	Channel::removeUser(const Client* client)
{
	UsersIterator	it = _users.begin();

	for (; it != _users.end() && it->client != client; it++);

	if (it != _users.end())
		_users.erase(it);
}

	/* Protected */
	/* Private */

Channel::t_user	Channel::createUser(Client* client, const int  &permissionsMask)
{
	t_user	user;

	user.client = client;
	user.permissionsMask = permissionsMask;

	return (user);
}

/* Getters */

	/* Public */

const std::string	&Channel::getName(void) const
{
	return (_name);
}

const std::string	&Channel::getTopic(void) const
{
	return (_topic);
}

Channel::User	*Channel::getUser(const std::string &nickname)
{
	UsersIterator	it = _users.begin();

	for (; it != _users.end() && it->client->getNickname() != nickname; it++);

	if (it != _users.end())
		return (&(*it));
	return (NULL);
}

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

/* Setters */

	/* Public */

void	Channel::setTopic(const std::string &topic)
{
	_topic = truncate(topic, MAX_TOPIC_LEN);
}

	/* Protected */
	/* Private */
