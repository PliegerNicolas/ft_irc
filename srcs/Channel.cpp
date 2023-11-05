/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hania <hania@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/20 14:50:37 by nplieger          #+#    #+#             */
/*   Updated: 2023/11/05 04:37:42 by nicolas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Channel.hpp"

/* Constructors & Destructors */

	/* Public */

Channel::Channel(const std::string &name):
	_name(truncate(name, MAX_CHANNELNAME_LEN)),
	_userLimit(-1),
	_modesMask(NO_EXTERNAL_MESSAGES | TOPIC_LOCK)
{
	if (DEBUG)
	{
		std::cout << GRAY;
		std::cout << "Channel: default constructor called.";
		std::cout << WHITE;
	}
}

Channel::Channel(const Channel &other):
	_name(other._name),
	_topic(other._topic),
	_users(other._users),
	_invitedClients(other._invitedClients),
	_userLimit(other._userLimit),
	_modesMask(other._modesMask)
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
		_name = other._name;
		_topic = other._topic;
		_users = other._users;
		_invitedClients = other._invitedClients;
		_userLimit = other._userLimit;
		_modesMask = other._modesMask;
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
	_userLimit(-1),
	_modesMask(0)
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

void	Channel::addUser(Client* client, const int &permissionsMask)
{
	_users.push_back(createUser(client, permissionsMask));
}

void	Channel::addInvitation(Client *client)
{
	_invitedClients.push_back(client);
}

void	Channel::removeUser(const Client* client)
{
	UsersIterator	it = _users.begin();

	for (; it != _users.end() && it->client != client; it++);

	if (it != _users.end())
		_users.erase(it);
}

void	Channel::removeInvitation(Client *client)
{
	ClientsIterator	it = _invitedClients.begin();

	for (; it != _invitedClients.end() && client != *it; it++);

	if (it != _invitedClients.end())
		_invitedClients.erase(it);
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

bool	Channel::isInvited(Client *client)
{
	ClientsIterator	it = _invitedClients.begin();

	for (; it != _invitedClients.end() && *it != client; it++);

	if (it != _invitedClients.end())
		return (true);
	return (false);
}

bool	Channel::isClientRegistered(const Client* client) const
{
	for (UsersConstIterator it = _users.begin(); it != _users.end(); ++it)
	{
		if (client == it->client)
			return (true);
	}

	return (false);
}

bool	Channel::canKick(const Client *client)
{
	const User	*user = Channel::getUser(client->getNickname());

	if (!user)
		return (false);
	if (areBitsSet(user->modesMask, OPERATOR | HALF_OPERATOR | OWNER))
		return (true);
	return (false);
}

bool	Channel::canInvite(const Client *client)
{
	const User	*user = Channel::getUser(client->getNickname());

	if (!user)
		return (false);
	if (areBitsSet(user->modesMask, OPERATOR | HALF_OPERATOR | OWNER))
		return (true);
	return (false);
}

bool	Channel::canChangeTopic(const Client *client)
{
	const User	*user = Channel::getUser(client->getNickname());

	if (!user)
		return (false);
	if (areBitsNotSet(_modesMask, TOPIC_LOCK))
		return (true);
	else if (areBitsSet(user->modesMask, OPERATOR | OWNER))
		return (true);
	return (false);
}

void	Channel::addChannelModes(const std::string &modes, std::string &invalidChars)
{
	setBits(_modesMask, Channel::channelModesToMask(modes, invalidChars));
}

void	Channel::removeChannelModes(const std::string &modes, std::string &invalidChars)
{
	removeBits(_modesMask, Channel::channelModesToMask(modes, invalidChars));
}

void	Channel::addUserModes(User *targetUser, const std::string &modes,
	std::string &invalidChars)
{
	if (!targetUser)
		return ;

	setBits(targetUser->modesMask, Channel::userModesToMask(modes, invalidChars));
}

void	Channel::removeUserModes(User *targetUser, const std::string &modes,
	std::string &invalidChars)
{
	if (!targetUser)
		return ;

	removeBits(targetUser->modesMask, Channel::userModesToMask(modes, invalidChars));
}
	/* Protected */
	/* Private */

Channel::t_user	Channel::createUser(Client* client, const int  &modesMask)
{
	t_user	user;

	user.client = client;
	user.modesMask = modesMask;

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

const Channel::Users	&Channel::getUsers(void) const
{
	return (_users);
}




const std::string	Channel::getChannelModes(void) const
{
	return (Channel::channelMaskToModes(_modesMask));
}

int	Channel::getChannelModesMask(void) const
{
	return (_modesMask);
}

const std::string	Channel::getUserModes(const Client *client)
{
	const User	*user = getUser(client->getNickname());

	if (!user)
		return ("");
	return (Channel::userMaskToModes(user->modesMask));
}

int	Channel::getUserModesMask(const Client *client)
{
	const User	*user = getUser(client->getNickname());

	if (!user)
		return (0);
	return (user->modesMask);
}

	/* Protected */
	/* Private */

/* Setters */

	/* Public */

void	Channel::setTopic(const std::string &topic)
{
	_topic = truncate(topic, MAX_TOPIC_LEN);
}



void	Channel::setChannelModesMask(const int &mask)
{
	setBits(_modesMask, mask);
}

void	Channel::setUserModesMask(User *targetUser, const int &mask)
{
	if (!targetUser)
		return ;

	setBits(targetUser->modesMask, mask);
}

	/* Protected */
	/* Private */

/* Static functions */

int	Channel::defaultUserPerms(void)
{
	return (INVISIBLE | SERVER_NOTICE);
}

int	Channel::defaultHalfOpsPerms(void)
{
	return (INVISIBLE | SERVER_NOTICE | HALF_OPERATOR);
}

int	Channel::defaultOpsPerms(void)
{
	return (INVISIBLE | SERVER_NOTICE | VOICE | OPERATOR);
}

int	Channel::defaultAdminPerms(void)
{
	return (INVISIBLE | SERVER_NOTICE | VOICE | ADMIN);
}

int	Channel::defaultOwnerPerms(void)
{
	return (INVISIBLE | SERVER_NOTICE | VOICE | ADMIN | OWNER);
}

int	Channel::channelModesToMask(const std::string &modes, std::string &invalidChars)
{
	int	mask = 0;

	for (size_t i = 0; i < modes.length(); i++)
	{
		switch (modes[i])
		{
			case 't':
				setBits(mask, TOPIC_LOCK);
				break ;
			case 'i':
				setBits(mask, INVITE_ONLY);
				break ;
			case 'n':
				setBits(mask, NO_EXTERNAL_MESSAGES);
				break ;
			case 'm':
				setBits(mask, MODERATED);
				break ;
			case 'l':
				setBits(mask, USER_LIMIT);
				break ;
			case 'k':
				setBits(mask, KEY_PASS);
				break ;
			case 'p':
				setBits(mask, PRIVATE);
				break ;
			case 's':
				setBits(mask, SECRET);
				break ;
			default:
				invalidChars += modes[i];
				break ;
		}
	}
	return (mask);
}

std::string	Channel::channelMaskToModes(const int &mask)
{
	std::string	modes = "+";
	const char	bitToChar[] = {'t', 'i', 'n', 'm', 'l', 'k', 'p', 's'};

	for (size_t shift = 0; shift < sizeof(bitToChar) / sizeof(*bitToChar); shift++)
	{
		if((mask >> shift) & 1)
			modes += bitToChar[shift];
	}

	return (modes);
}

int	Channel::userModesToMask(const std::string &modes, std::string &invalidChars)
{
	int	mask = 0;

	for (size_t i = 0; i < modes.length(); i++)
	{
		switch (modes[i])
		{
			case 's':
				setBits(mask, SERVER_NOTICE);
				break ;
			case 'x':
				setBits(mask, SSL_TLS); // not used
				break ;
			case 'i':
				setBits(mask, INVISIBLE);
				break ;
			case 'h':
				setBits(mask, HALF_OPERATOR);
				break ;
			case 'o':
				setBits(mask, OPERATOR);
				break ;
			case 'a':
				setBits(mask, ADMIN);
				break ;
			case 'q':
				setBits(mask, OWNER);
				break ;
			default:
				invalidChars += modes[i];
				break ;
		}
	}
	return (mask);
}

std::string	Channel::userMaskToModes(const int &mask)
{
	std::string	modes = "+";
	const char	bitToChar[] = {'s', 'x', 'i', 'h', 'o', 'a', 'q'};

	for (size_t shift = 0; shift < sizeof(bitToChar) / sizeof(*bitToChar); shift++)
	{
		if((mask >> shift) & 1)
			modes += bitToChar[shift];
	}

	return (modes);
}
